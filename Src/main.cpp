#include <concepts>
#include <cstdint>
#include <cstdio>
#include <numeric>

#include "GetLine.hpp"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "json.hpp"
#include "pico/stdlib.h"

constexpr auto LED_PIN = PICO_DEFAULT_LED_PIN;
//constexpr auto LED_PIN = 13;

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
auto FetchJSONValue(auto& variable, json& j, string_convertable auto... keys) -> bool {
  if (CheckValue(j, keys...)) {
    variable = FetchValue(j, keys...);
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

uint16_t pwm_a = 0;
uint16_t pwm_b = 0;
uint16_t deadband = 0;
float duty = 0;

#include <atomic>

std::atomic<float> v_in_offset = 0.0F;
std::atomic<float> v_in_multiplier = 1.0F;

std::atomic<float> v_out_offset = 0.0F;
std::atomic<float> v_out_multiplier = 1.0F;

std::atomic<float> i_in_offset = 1.65F;
std::atomic<float> i_in_gain = 1.0F;

std::atomic<float> i_out_offset = 1.65F;
std::atomic<float> i_out_gain = 1.0F;

enum class ADC_Channels : uint8_t {
  voltage_in = 0,
  current_in = 1,
  voltage_out = 2,
  current_out = 3,
  cpu_temperature = 4
};

auto ADCRaw(ADC_Channels channel) -> uint16_t {
  adc_select_input(static_cast<uint>(channel));
  return adc_read();
}

constexpr auto RawToVoltage(uint16_t adc_raw) -> float {
  constexpr float conversion_factor = 3.3f / (1 << 12);  // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
  return static_cast<float>(adc_raw) * conversion_factor;
}

auto ADCVoltage(ADC_Channels channel) -> float {
  return RawToVoltage(ADCRaw(channel));
}

auto ADCCurrent(ADC_Channels channel, uint16_t shunt_mohm, float offset, float gain) -> float {
  auto raw = ADCVoltage(channel);
  auto shunt_voltage = (raw - offset) * gain;
  return shunt_voltage / (static_cast<float>(shunt_mohm) / 1000);
}

float CPUTemperatureC() {
  auto temp_voltage = ADCVoltage(ADC_Channels::cpu_temperature);
  return 27.0F - (temp_voltage - 0.706F) / 0.001721F;
}

struct PWMManager {
  explicit PWMManager(uint8_t slice_number)
      : slice_number_(slice_number) {}

  [[nodiscard]] auto Top() const -> uint32_t {
    return pwm_hw->slice[slice_number_].top;
  }

  auto DeadBand(uint16_t count) {
    deadband = count;
    deadband_ = count;
  }

  //  auto DeadBandUs() {
  //    return ;
  //  }

  auto SMPSFrequencyKHz(uint16_t freq_khz) {
    constexpr auto f_sys_khz = 125000000 / 1000;
    auto top = (f_sys_khz / freq_khz / 2) - 1;
    pwm_set_wrap(slice_number_, top);
  }

  auto SMPSFrequencyKHz() -> uint16_t {
    constexpr auto f_sys_khz = 125000000 / 1000;
    return 1 / ((Top() + 1) * 2 / f_sys_khz);
  }

  auto SimpleDuty(uint16_t duty_cycle) {
    auto top = Top();
    uint16_t a_level = 0;
    uint16_t b_level = 0;
    if (duty_cycle == 0) {
      a_level = 0;
      b_level = top;
    }

    a_level -= deadband_ / 2;
    b_level += deadband_ / 2;

    pwm_set_both_levels(slice_number_, a_level, b_level);
  }

  auto SMPSDuty(float duty_cycle_percentage) {
    duty = duty_cycle_percentage / 100;
    auto top = Top();
    uint16_t a_level = 0;
    uint16_t b_level = 0;
    if (duty_cycle_percentage < 1) {
      a_level = 0;
      b_level = 0;
    } else if (duty_cycle_percentage > 99) {
      a_level = top;
      b_level = top;
    } else {
      a_level = std::floor(top * (duty_cycle_percentage / 100));
      b_level = a_level;
      a_level -= deadband_;
      b_level += deadband_;
    }

    pwm_a = a_level;
    pwm_b = b_level;

    pwm_set_both_levels(slice_number_, a_level, b_level);
  }

  auto Initialize() -> void {
    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    gpio_disable_pulls(0);
    gpio_disable_pulls(1);

    gpio_set_slew_rate(0, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(1, GPIO_SLEW_RATE_FAST);

    gpio_set_drive_strength(0, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(1, GPIO_DRIVE_STRENGTH_12MA);

    pwm_set_wrap(slice_number_, 625 - 1);
    //    //  pwm_set_both_levels(slice, 5000, 1250);
    pwm_set_both_levels(slice_number_, 295, 335);
    pwm_set_phase_correct(slice_number_, true);
    pwm_set_output_polarity(slice_number_, false, false);
    //      SMPSFrequencyKHz(100);
    //      SMPSDuty(50);
    pwm_set_enabled(slice_number_, true);  // Start PWM running
  }

  const uint8_t slice_number_;
  uint16_t deadband_ = 24;
};

constexpr size_t averaging_array_size = 300;
std::array<uint16_t, averaging_array_size> v_in_readings{};
std::array<uint16_t, averaging_array_size> v_out_readings{};
std::array<uint16_t, averaging_array_size> i_in_readings{};
std::array<uint16_t, averaging_array_size> i_out_readings{};

PWMManager pwm_manager(pwm_gpio_to_slice_num(0));

#include <hardware/structs/systick.h>

#include "pico/multicore.h"

auto get_cycle_count() -> uint32_t {
  return systick_hw->cvr;
}

void main1() {
  systick_hw->csr = 0x5;
  systick_hw->rvr = 0x00FFFFFF;

  constexpr size_t feedback_average_number = 5;
  std::array<uint16_t, feedback_average_number> v_in_feedback{};

  while (true) {
//    for (auto& voltage : v_in_feedback) {
//      voltage = ADCRaw(ADC_Channels::voltage_in);
//    }
//
//    auto v_in_feedback_average = RawToVoltage(std::accumulate(v_in_feedback.begin(), v_in_feedback.end(), 0) / v_in_feedback.size());


  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, true);
  stdio_init_all();

  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);
  adc_gpio_init(28);
  adc_gpio_init(29);
  adc_set_temp_sensor_enabled(true);

  multicore_launch_core1(main1);

  pwm_manager.Initialize();

  std::array<char, 256> buf{};

  json j;

  while (true) {
    auto in_str = GetLine(buf, '\r');
    if (!in_str.empty()) {
      j = json::parse(in_str);
      if (j.contains("SMPS")) {
        if (j["SMPS"].contains("Duty")) pwm_manager.SMPSDuty(j["SMPS"]["Duty"]);
        if (j["SMPS"].contains("DeadBand")) pwm_manager.DeadBand(j["SMPS"]["DeadBand"]);
        if (j["SMPS"].contains("Frequency")) pwm_manager.SMPSFrequencyKHz(j["SMPS"]["Frequency"]);
      }

      FetchJSONValue(v_in_offset, j, "Sensor", "Vin", "Offset");
      FetchJSONValue(v_in_multiplier, j, "Sensor", "Vin", "Multiplier");

      FetchJSONValue(v_out_offset, j, "Sensor", "Vout", "Offset");
      FetchJSONValue(v_out_multiplier, j, "Sensor", "Vout", "Multiplier");

      FetchJSONValue(i_in_offset, j, "Sensor", "Iin", "Offset");
      FetchJSONValue(i_in_gain, j, "Sensor", "Iin", "Gain");

      FetchJSONValue(i_out_offset, j, "Sensor", "Iout", "Offset");
      FetchJSONValue(i_out_gain, j, "Sensor", "Iout", "Gain");

      //      if (j.contains("Sensor")) {
      //        if (j["Sensor"].contains("Vin")) {
      //          if (j["Sensor"]["Vin"].contains("Offset")) v_in_offset = j["Sensor"]["Vin"]["Offset"];
      //          if (j["Sensor"]["Vin"].contains("Multiplier")) v_in_multiplier = j["Sensor"]["Vin"]["Multiplier"];
      //        }
      //
      //        if (j["Sensor"].contains("Vout")) {
      //          if (j["Sensor"]["Vout"].contains("Offset")) v_out_offset = j["Sensor"]["Vout"]["Offset"];
      //          if (j["Sensor"]["Vout"].contains("Multiplier")) v_out_multiplier = j["Sensor"]["Vout"]["Multiplier"];
      //        }
      //
      //        if (j["Sensor"].contains("Iin")) {
      //          if (j["Sensor"]["Iin"].contains("Offset")) i_in_offset = j["Sensor"]["Iin"]["Offset"];
      //          if (j["Sensor"]["Iin"].contains("Gain")) i_in_gain = j["Sensor"]["Iin"]["Gain"];
      //        }
      //
      //        if (j["Sensor"].contains("Iout")) {
      //          if (j["Sensor"]["Iout"].contains("Offset")) i_out_offset = j["Sensor"]["Iout"]["Offset"];
      //          if (j["Sensor"]["Iout"].contains("Gain")) i_out_gain = j["Sensor"]["Iout"]["Gain"];
      //        }
      //      }
      if (j.contains("Control")) {
        CheckKeyThenCall(j["Control"], "LED", BuiltInLED);
      }
    }
    j.clear();

    //    j["SMPS"]["In"]["Volts"] = ADCVoltage(ADC_Channels::voltage_in);
    //    j["SMPS"]["Out"]["Volts"] = ADCVoltage(ADC_Channels::voltage_out);
    //    j["SMPS"]["In"]["Amps"] = ADCCurrent(ADC_Channels::current_in, 100, i_in_offset, i_in_gain);
    //    j["SMPS"]["Out"]["Amps"] = ADCCurrent(ADC_Channels::current_out, 100, i_out_offset, i_out_gain);

    for (size_t index = 0; index < averaging_array_size; ++index) {
      v_in_readings[index] = ADCRaw(ADC_Channels::voltage_in);
      v_out_readings[index] = ADCRaw(ADC_Channels::voltage_out);
      i_in_readings[index] = ADCRaw(ADC_Channels::current_in);
      i_out_readings[index] = ADCRaw(ADC_Channels::current_out);
    }

    auto v_in_average = RawToVoltage(std::accumulate(v_in_readings.begin(), v_in_readings.end(), 0) / averaging_array_size);
    auto v_out_average = RawToVoltage(std::accumulate(v_out_readings.begin(), v_out_readings.end(), 0) / averaging_array_size);
    auto i_in_average = RawToVoltage(std::accumulate(i_in_readings.begin(), i_in_readings.end(), 0) / averaging_array_size);
    auto i_out_average = RawToVoltage(std::accumulate(i_out_readings.begin(), i_out_readings.end(), 0) / averaging_array_size);

    //    auto raw = 0.0F;
    //    auto filtered = 0.0F;
    //    filtered = 0.99*filtered + 0.01*raw;

    j["SMPS"]["In"]["VVolts"] = v_in_average;
    j["SMPS"]["In"]["IVolts"] = v_out_average;
    j["SMPS"]["Out"]["VVolts"] = i_in_average;
    j["SMPS"]["Out"]["IVolts"] = i_out_average;

    j["System"]["Temp"] = CPUTemperatureC();
    std::string json_str = j.dump();
    puts(json_str.data());
    j.clear();

    sleep_ms(10);
  }
}
