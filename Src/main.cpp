#include <concepts>
#include <cstdint>
#include <cstdio>

#include "GetLine.hpp"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "json.hpp"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "main.hpp"

#include "FMT/FMTToUART.hpp"

constexpr auto LED_PIN = PICO_DEFAULT_LED_PIN;

using namespace nlohmann;

// If you need to brush up on concepts in function signatures: https://www.sandordargo.com/blog/2021/02/17/cpp-concepts-4-ways-to-use-them
template <typename T>
concept string_convertable = std::convertible_to<T, std::string_view>;  // Requires that the argument can be converted to a string view

// If you need to brush up on variadic recursion: https://stackoverflow.com/questions/1657883/variable-number-of-arguments-in-c
auto CheckValue(json& j, string_convertable auto current_key) -> bool {
  return j.contains(current_key);
}

auto CheckValue(json& j, string_convertable auto current_key, string_convertable auto... remaining_keys) -> bool {
  if (j.contains(current_key)) {
    return CheckValue(j[current_key], remaining_keys...);
  } else {
    return false;
  }
}

auto FetchValue(json& j, string_convertable auto current_key) {
  return j[current_key];
}

auto FetchValue(json& j, string_convertable auto current_key, string_convertable auto... remaining_keys) {
  return FetchValue(j[current_key], remaining_keys...);
}

/**
 * @brief Reads in a variable from the JSON doc or does nothing.
 * @param j The JSON doc to read from.
 * @param variable The variable reference, will either be updated if the value is available or unmodified if not found.
 * @param keys The set of keys which represent the path to the value.
 * @returns True if found (variable updated), false if not found.
 */
template <typename T>
auto FetchJSONValue(T& variable, json& j, string_convertable auto... keys) -> bool {
  if (CheckValue(j, keys...)) {
    variable = T{FetchValue(j, keys...)};
    return true;
  } else {
    return false;
  }
}

inline auto CheckKeyThenCall(json& json_doc, std::string_view key, auto callback) -> void {
  if (json_doc.contains(key)) callback(json_doc[key]);
}

inline auto BuiltInLED(bool state) -> void {
  gpio_put(LED_PIN, state);
}

inline auto ParseEStop(json& j) -> void {
  if (j["SMPS"]["EStop"]) {
  }
}

int main() {
  stdio_init_all();
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  queue_init(&tune_values_queue, sizeof(TuneValues), 10);
  queue_init(&smps_parameters_queue, sizeof(SMPSParameters), 10);
  queue_init(&sensor_data_queue, sizeof(SensorData), 2);

  multicore_launch_core1(main1);

  std::array<char, 512> buf{};

  json j;

  while (true) {
    auto in_str = GetLine(buf, '\r');
    if (!in_str.empty()) {
      j = json::parse(in_str);
      if (j.contains("SMPS")) {
        //        if (j["SMPS"].contains("Duty")) pwm_manager.SMPSDuty(j["SMPS"]["Duty"]);
        if (j["SMPS"].contains("DeadBand")) pwm_manager.DeadBand(j["SMPS"]["DeadBand"]);
        //        if (j["SMPS"].contains("Frequency")) pwm_manager.SMPSFrequencyKHz(j["SMPS"]["Frequency"]);
      }

      units::voltage::volt_t v_set_point = 0_V;
      if (FetchJSONValue(v_set_point, j, "SMPS", "SetPoint")) {
        SMPSParameters parameters{.v_out_set_point = v_set_point};
        queue_try_add(&smps_parameters_queue, &parameters);
      }

      float duty = 0;
      if (FetchJSONValue(duty, j, "SMPS", "Duty")) {
        SMPSParameters parameters{.user_duty = duty / 100.0F}; // Divide the duty by 100 to make it in the range of 0 - 1
        queue_try_add(&smps_parameters_queue, &parameters);
      }

      bool enable = false;
      if (FetchJSONValue(enable, j, "SMPS", "Enable")) {
        SMPSParameters parameters{.enable = enable};
        queue_try_add(&smps_parameters_queue, &parameters);
      }

      bool pi_loop_enable = false;
      if (FetchJSONValue(pi_loop_enable, j, "SMPS", "PIEnable")) {
        SMPSParameters parameters{.pi_loop_enable = pi_loop_enable};
        queue_try_add(&smps_parameters_queue, &parameters);
      }

      units::voltage::volt_t v_in_offset = 0_V;
      if (FetchJSONValue(v_in_offset, j, "Sensor", "Tune", "Vin", "Offset")) {
        VTune v_tune{.offset = v_in_offset};
        TuneValues tune{.v_in_tune = v_tune};
        queue_try_add(&tune_values_queue, &tune);
      }

      float v_in_multiplier = 2.0F;
      if (FetchJSONValue(v_in_multiplier, j, "Sensor", "Tune", "Vin", "Multiplier")) {
        VTune v_tune{.multiplier = v_in_multiplier};
        TuneValues tune{.v_in_tune = v_tune};
        queue_try_add(&tune_values_queue, &tune);
      }

      units::voltage::volt_t v_out_offset = 0_V;
      if (FetchJSONValue(v_out_offset, j, "Sensor", "Tune", "Vout", "Offset")) {
        VTune v_tune{.offset = v_out_offset};
        TuneValues tune{.v_out_tune = v_tune};
        queue_try_add(&tune_values_queue, &tune);
      }
      float v_out_multiplier = 10.0F;
      if (FetchJSONValue(v_out_multiplier, j, "Sensor", "Tune", "Vout", "Multiplier")) {
        VTune v_tune{.multiplier = v_out_multiplier};
        TuneValues tune{.v_out_tune = v_tune};
        queue_try_add(&tune_values_queue, &tune);
      }

      units::voltage::volt_t i_in_offset = 1.65_V;
      if (FetchJSONValue(i_in_offset, j, "Sensor", "Tune", "Iin", "Offset")) {
        ITune i_tune{.offset = i_in_offset};
        TuneValues tune{.i_in_tune = i_tune};
        queue_try_add(&tune_values_queue, &tune);
      }
      float i_in_gain = 1.0F;
      if (FetchJSONValue(i_in_gain, j, "Sensor", "Tune", "Iin", "Gain")) {
        ITune i_tune{.gain = i_in_gain};
        TuneValues tune{.i_in_tune = i_tune};
        queue_try_add(&tune_values_queue, &tune);
      }

      units::voltage::volt_t i_out_offset = 1.65_V;
      if (FetchJSONValue(i_out_offset, j, "Sensor", "Tune", "Iout", "Offset")) {
        ITune i_tune{.offset = i_out_offset};
        TuneValues tune{.i_out_tune = i_tune};
        queue_try_add(&tune_values_queue, &tune);
      }
      float i_out_gain = 1.0F;
      if (FetchJSONValue(i_out_gain, j, "Sensor", "Tune", "Iout", "Gain")) {
        ITune i_tune{.gain = i_out_gain};
        TuneValues tune{.i_out_tune = i_tune};
        queue_try_add(&tune_values_queue, &tune);
      }

      units::impedance::ohm_t in_shunt = 1_Ohm;
      if (FetchJSONValue(in_shunt, j, "Sensor", "Parameters", "In", "ShuntOhms")) {
        ITune i_tune{.shunt = in_shunt};
        TuneValues tune{.i_in_tune = i_tune};
        queue_try_add(&tune_values_queue, &tune);
      }
      units::impedance::ohm_t out_shunt = 1_Ohm;
      if (FetchJSONValue(out_shunt, j, "Sensor", "Parameters", "Out", "ShuntOhms")) {
        ITune i_tune{.shunt = out_shunt};
        TuneValues tune{.i_out_tune = i_tune};
        queue_try_add(&tune_values_queue, &tune);
      }

      if (j.contains("Control")) {
        CheckKeyThenCall(j["Control"], "LED", BuiltInLED);
      }
    }
    j.clear();

    SensorData sensor_data{};
    while (queue_try_remove(&sensor_data_queue, &sensor_data)) {
      j["SMPS"]["In"]["Volts"] = sensor_data.v_in.to<float>();
      j["SMPS"]["In"]["Amps"] = sensor_data.i_in.to<float>();
      j["SMPS"]["Out"]["Volts"] = sensor_data.v_out.to<float>();
      j["SMPS"]["Out"]["Amps"] = sensor_data.i_out.to<float>();
//      std::string buf{};
//      fmt::format_to(std::back_inserter(buf), "v_in_samples_queue: {}\n", queue_get_level(&v_in_samples_queue));
//      fmt::format_to(std::back_inserter(buf), "i_in_samples_queue: {}\n", queue_get_level(&i_in_samples_queue));
//      fmt::format_to(std::back_inserter(buf), "v_out_samples_queue: {}\n", queue_get_level(&v_out_samples_queue));
//      fmt::format_to(std::back_inserter(buf), "i_out_samples_queue: {}\n", queue_get_level(&i_out_samples_queue));
//      fmt::format_to(std::back_inserter(buf), "tune_values_queue: {}\n", queue_get_level(&tune_values_queue));
//      fmt::format_to(std::back_inserter(buf), "smps_parameters_queue: {}\n", queue_get_level(&smps_parameters_queue));
//      fmt::format_to(std::back_inserter(buf), "sensor_data_queue: {}\n", queue_get_level(&sensor_data_queue));

      j["System"]["Queues"]["VIn"]["Level"] = queue_get_level(&v_in_samples_queue);
//      j["System"]["Queues"]["VIn"]["MaxSize"] = (&v_in_samples_queue);
      j["System"]["Queues"]["IIn"]["Level"] = queue_get_level(&i_in_samples_queue);
//      j["System"]["Queues"]["IIn"]["MaxSize"] = (&i_in_samples_queue);
      j["System"]["Queues"]["VOut"]["Level"] = queue_get_level(&v_out_samples_queue);
//      j["System"]["Queues"]["VOut"]["MaxSize"] = (&v_out_samples_queue);
      j["System"]["Queues"]["IOut"]["Level"] = queue_get_level(&i_out_samples_queue);
//      j["System"]["Queues"]["IOut"]["MaxSize"] = (&i_out_samples_queue);
      j["System"]["Queues"]["TuneValues"]["Level"] = queue_get_level(&tune_values_queue);
//      j["System"]["Queues"]["TuneValues"]["MaxSize"] = (&tune_values_queue);
      j["System"]["Queues"]["SMPSParameters"]["Level"] = queue_get_level(&smps_parameters_queue);
//      j["System"]["Queues"]["SMPSParameters"]["MaxSize"] = (&smps_parameters_queue);
      j["System"]["Queues"]["SensorData"]["Level"] = queue_get_level(&sensor_data_queue);
//      j["System"]["Queues"]["SensorData"]["MaxSize"] = (&sensor_data_queue);

//      j["System"]["Message"] = buf;
    }

    Core1MiscData misc_data{};
    while (queue_try_remove(&core_1_misc_data_queue, &misc_data)) {
      j["SMPS"]["Duty"] = misc_data.duty;
      j["System"]["TempC"] = misc_data.cpu_temperature.to<float>();
    }

    if (!j.empty()) {
      std::string json_str = j.dump();
      puts(json_str.data());
    }

    j.clear();

    sleep_ms(50);
  }
}
