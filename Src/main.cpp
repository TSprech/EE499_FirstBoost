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

#define DISABLE_PREDEFINED_UNITS
#define ENABLE_PREDEFINED_TEMPERATURE_UNITS
#define ENABLE_PREDEFINED_FREQUENCY_UNITS
#define ENABLE_PREDEFINED_CURRENT_UNITS
#define ENABLE_PREDEFINED_CHARGE_UNITS
#define ENABLE_PREDEFINED_TIME_UNITS
#define ENABLE_PREDEFINED_ENERGY_UNITS
#define ENABLE_PREDEFINED_POWER_UNITS
#define ENABLE_PREDEFINED_VOLTAGE_UNITS
#define ENABLE_PREDEFINED_IMPEDANCE_UNITS
#define UNIT_LIB_DISABLE_IOSTREAM
#define UNIT_LIB_DEFAULT_TYPE float
#include "units.h"
using namespace units::literals;

#include "fmt/core.h"

constexpr auto LED_PIN = PICO_DEFAULT_LED_PIN;
constexpr auto MONITOR_PIN = 2;
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

constexpr auto RawToVoltage(uint16_t adc_raw) -> units::voltage::volt_t {
  constexpr float conversion_factor = 3.3f / (1 << 12);  // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
  return units::voltage::volt_t{static_cast<float>(adc_raw) * conversion_factor};
}

constexpr auto RawToVoltage(uint16_t adc_raw, units::voltage::volt_t offset, float multiplier) -> units::voltage::volt_t {
  auto voltage = RawToVoltage(adc_raw);
  return (voltage - offset) * multiplier;
}

auto VoltageToCurrent(units::voltage::volt_t shunt_voltage, units::impedance::ohm_t shunt, units::voltage::volt_t offset, float gain) -> units::current::ampere_t {
  auto adjusted_shunt_voltage = (shunt_voltage - offset) * gain;
  return adjusted_shunt_voltage / shunt;
}

auto RawToCurrent(uint16_t adc_raw, units::impedance::ohm_t shunt, units::voltage::volt_t offset, float gain) -> units::current::ampere_t {
  return VoltageToCurrent(RawToVoltage(adc_raw), shunt, offset, gain);
}

auto CorrectVoltage(units::voltage::volt_t voltage, units::voltage::volt_t offset, float multiplier) -> units::voltage::volt_t {
  return (voltage - offset) * multiplier;
}

auto ADCVoltage(ADC_Channels channel) -> units::voltage::volt_t {
  return RawToVoltage(ADCRaw(channel));
}

auto ADCCurrent(ADC_Channels channel, units::impedance::ohm_t shunt, units::voltage::volt_t offset, float gain) -> units::current::ampere_t {
  auto raw = ADCVoltage(channel);
  auto shunt_voltage = (raw - offset) * gain;
  return shunt_voltage / shunt;
}

auto CPUTemperature() -> units::temperature::celsius_t {
  auto temp_voltage = ADCVoltage(ADC_Channels::cpu_temperature);
  return units::temperature::celsius_t{27.0F - (temp_voltage.to<float>() - 0.706F) / 0.001721F};
}

struct PWMManager {
  explicit PWMManager(uint8_t slice_number)
      : slice_number_(slice_number) {}

  [[nodiscard]] auto Top() const -> uint32_t {
    return pwm_hw->slice[slice_number_].top;
  }

  auto DeadBand(uint16_t count) {
    //    deadband = count;
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
    //    duty = duty_cycle_percentage / 100;
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

    //    pwm_a = a_level;
    //    pwm_b = b_level;

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

constexpr size_t averaging_array_size = 50;
std::array<uint16_t, averaging_array_size> v_in_readings{};
std::array<uint16_t, averaging_array_size> v_out_readings{};
std::array<uint16_t, averaging_array_size> i_in_readings{};
std::array<uint16_t, averaging_array_size> i_out_readings{};

PWMManager pwm_manager(pwm_gpio_to_slice_num(0));

#include <hardware/structs/systick.h>

#include "pico/multicore.h"

auto get_cycle_count() -> uint32_t {
  // Init lines
  //  systick_hw->csr = 0x5;
  //  systick_hw->rvr = 0x00FFFFFF;
  return systick_hw->cvr;
}

//template <typename T>
//  requires std::same_as<std::underlying_type_t<T>, uint32_t> || std::same_as<T, uint32_t>
//auto FIFO_Push(T value) -> void {
//  multicore_fifo_push_blocking(static_cast<uint32_t>(value));
//}
//
//auto FIFO_Push(float value) -> void {
//  auto value_u32 = *reinterpret_cast<uint32_t*>(&value);  // Convert the 4 bytes of the float to a uint32_t without modifying the bits
//  multicore_fifo_push_blocking(value_u32);
//}
//
//auto FIFO_Pop(MessageTypes mtype) {
//}

#include "fmt/format.h"
#include "pico/mutex.h"
#include "pico/util/queue.h"

mutex_t* adc_mutex;

queue_t v_in_samples_queue;
queue_t i_in_samples_queue;
queue_t v_out_samples_queue;
queue_t i_out_samples_queue;

queue_t tune_values_queue;
queue_t smps_parameters_queue;
queue_t sensor_data_queue;

#include <optional>

struct VTune {
  std::optional<units::voltage::volt_t> offset;
  std::optional<float> multiplier;
};

struct ITune {
  std::optional<units::impedance::ohm_t> shunt;
  std::optional<units::voltage::volt_t> offset;
  std::optional<float> gain;
};

struct TuneValues {
  std::optional<VTune> v_in_tune;
  std::optional<ITune> i_in_tune;
  std::optional<VTune> v_out_tune;
  std::optional<ITune> i_out_tune;
};

struct SMPSParameters {
  std::optional<units::voltage::volt_t> v_out_set_point;
  std::optional<float> user_duty;
  std::optional<bool> enable;
  std::optional<bool> pi_loop_enable;
};

template <typename... T>
auto FMTDebug(fmt::format_string<T...> fmt, T&&... args) {
  fmt::memory_buffer buf;
  auto end = vformat_to(std::back_inserter(buf), fmt, fmt::make_format_args(args...));

//  if (get_core_num() == 0) {  // Check which core called for the debug and output to the appropriate UART
//    //  for (auto* data = buf.begin(); data != end; ++data) {
//    for (auto i = 0; i < buf.size(); ++i) {
//      uart_putc(uart0, buf[i]);
//    }
//  } else {
    for (auto i = 0; i < buf.size(); ++i) {
      uart_putc(uart1, buf[i]);
    }
//  }
}

auto AverageFromQueue(queue_t& queue) {
  uint32_t sum = 0;
  size_t number_of_data_points = 0;
  uint16_t current_data = 0;
  while (queue_try_remove(&queue, &current_data)) {
    sum += current_data;
    ++number_of_data_points;
  }
  return sum / number_of_data_points;
}

auto AverageVoltageFromQueue(queue_t& queue) {
  return RawToVoltage(AverageFromQueue(queue));
}

auto AverageVoltageFromQueue(queue_t& queue, units::voltage::volt_t offset, float multiplier) {
  return RawToVoltage(AverageFromQueue(queue), offset, multiplier);
}

auto AverageCurrentFromQueue(queue_t& queue, units::impedance::ohm_t shunt, units::voltage::volt_t offset, float gain) {
  return RawToCurrent(AverageFromQueue(queue), shunt, offset, gain);
}

#define EXTRACT_FUNCTION(func_name, struct_type, variable_type, member_name)              \
  auto func_name(std::optional<struct_type> tune_struct, variable_type& variable)->bool { \
    if (tune_struct && tune_struct.value().member_name) {                                 \
      variable = tune_struct.value().member_name.value();                                 \
      return true;                                                                        \
    }                                                                                     \
    return false;                                                                         \
  }                                                                                       \
  static_assert(true)

EXTRACT_FUNCTION(ExtractOffset, VTune, std::optional<units::voltage::volt_t>, offset);
EXTRACT_FUNCTION(ExtractMultiplier, VTune, std::optional<float>, multiplier);
EXTRACT_FUNCTION(ExtractOffset, ITune, std::optional<units::voltage::volt_t>, offset);
EXTRACT_FUNCTION(ExtractGain, ITune, std::optional<float>, gain);
EXTRACT_FUNCTION(ExtractShunt, ITune, std::optional<units::impedance::ohm_t>, shunt);

EXTRACT_FUNCTION(ExtractVOutSetPoint, SMPSParameters, std::optional<units::voltage::volt_t>, v_out_set_point);
EXTRACT_FUNCTION(ExtractUserDuty, SMPSParameters, std::optional<float>, user_duty);
EXTRACT_FUNCTION(ExtractEnable, SMPSParameters, std::optional<bool>, enable);
EXTRACT_FUNCTION(ExtractPILoopEnable, SMPSParameters, std::optional<bool>, pi_loop_enable);

constexpr auto rate = 100_Hz;                        // Whatever the PI loop call rate will be
constexpr units::time::second_t period = 1 / rate;  // Convert the Hz to period

struct SensorData {
  units::voltage::volt_t v_in;
  units::current::ampere_t i_in;
  units::voltage::volt_t v_out;
  units::current::ampere_t i_out;
};

bool FeedbackLoop(repeating_timer_t* rt) {
  /*  static auto state = true;
  if (state) {
    gpio_put(LED_PIN, true);
    gpio_put(MONITOR_PIN, true);
  }
  else {
    gpio_put(LED_PIN, false);
    gpio_put(MONITOR_PIN, false);
  }
  state = !state;
  */

  static VTune v_in_tune = {
      .offset = 0_V,
      .multiplier = 2};

  static VTune v_out_tune = {
      .offset = 0_V,
      .multiplier = 10};

  static ITune i_in_tune = {
      .shunt = 100_mOhm,
      .offset = 1.65_V,
      .gain = 1};

  static ITune i_out_tune = {
      .shunt = 100_mOhm,
      .offset = 1.65_V,
      .gain = 1};

  static SMPSParameters smps_parameters = {
      .v_out_set_point = 0_V,
      .user_duty = 0.25,
      .enable = false,
      .pi_loop_enable = false};

  TuneValues tune{};
  while (queue_try_remove(&tune_values_queue, &tune)) {
    if (ExtractOffset(tune.v_in_tune, v_in_tune.offset))
      FMTDebug("V in offset changed: {}V\n", v_in_tune.offset.value().to<float>());
    if (ExtractMultiplier(tune.v_in_tune, v_in_tune.multiplier))
      FMTDebug("V in multiplier changed: {}\n", v_in_tune.multiplier.value());

    if (ExtractOffset(tune.v_out_tune, v_out_tune.offset))
      FMTDebug("V out offset changed: {}V\n", v_out_tune.offset.value().to<float>());
    if (ExtractMultiplier(tune.v_out_tune, v_out_tune.multiplier))
      FMTDebug("V out multiplier changed: {}\n", v_out_tune.multiplier.value());

    if (ExtractShunt(tune.i_in_tune, i_in_tune.shunt))
      FMTDebug("I in shunt changed: {}Ω\n", i_in_tune.shunt.value().to<float>());
    if (ExtractOffset(tune.i_in_tune, i_in_tune.offset))
      FMTDebug("I in offset changed: {}V\n", i_in_tune.offset.value().to<float>());
    if (ExtractGain(tune.i_in_tune, i_in_tune.gain))
      FMTDebug("I in gain changed: {}\n", i_in_tune.gain.value());

    if (ExtractShunt(tune.i_out_tune, i_out_tune.shunt))
      FMTDebug("I out shunt changed: {}Ω\n", i_out_tune.shunt.value().to<float>());
    if (ExtractOffset(tune.i_out_tune, i_out_tune.offset))
      FMTDebug("I out offset changed: {}V\n", i_out_tune.offset.value().to<float>());
    if (ExtractGain(tune.i_out_tune, i_out_tune.gain))
      FMTDebug("I out gain changed: {}\n", i_out_tune.gain.value());
  }

  SMPSParameters parameters{};
  while (queue_try_remove(&smps_parameters_queue, &parameters)) {
    if (ExtractVOutSetPoint(parameters, smps_parameters.v_out_set_point))
      FMTDebug("V out set point changed: {}V\n", smps_parameters.v_out_set_point.value().to<float>());
    if (ExtractUserDuty(parameters, smps_parameters.user_duty))
      FMTDebug("User duty cycle changed: {}%\n", smps_parameters.user_duty.value());
    if (ExtractEnable(parameters, smps_parameters.enable))
      FMTDebug("SMPS is now {}\n", (smps_parameters.enable.value() ? "enabled" : "disabled"));
    if (ExtractPILoopEnable(parameters, smps_parameters.pi_loop_enable))
      FMTDebug("PI Loop is now {}\n", (smps_parameters.pi_loop_enable.value() ? "enabled" : "disabled"));
  }

  auto v_in_average = AverageVoltageFromQueue(v_in_samples_queue, v_in_tune.offset.value(), v_in_tune.multiplier.value());
  auto i_in_average = AverageCurrentFromQueue(i_in_samples_queue, i_in_tune.shunt.value(), i_in_tune.offset.value(), i_in_tune.gain.value());
  auto v_out_average = AverageVoltageFromQueue(v_out_samples_queue, v_out_tune.offset.value(), v_out_tune.multiplier.value());
  auto i_out_average = AverageCurrentFromQueue(i_out_samples_queue, i_out_tune.shunt.value(), i_out_tune.offset.value(), i_out_tune.gain.value());

  constexpr float duty_max = 90.0F;
  constexpr float duty_min = 20.0F;
  constexpr float k_p = 0.1F;
  constexpr float k_i = 20.0F;
  static float duty_integrated = 0.0F;

  if (smps_parameters.enable.value() && smps_parameters.pi_loop_enable.value()) {
    auto error = smps_parameters.v_out_set_point.value() - v_out_average;
//    auto error = smps_parameters.v_out_set_point.value() - CorrectVoltage(ADCVoltage(ADC_Channels::voltage_out), v_in_tune.offset.value(), v_in_tune.multiplier.value());
//
    float duty = k_p * error.to<float>() + duty_integrated;
    duty = std::clamp(duty, duty_min, duty_max);
//
    duty_integrated += k_i * error.to<float>() * period.to<float>();
    duty_integrated = std::clamp(duty_integrated, duty_min, duty_max);

    static auto last_duty = duty;
    pwm_manager.SMPSDuty(duty);
    if (std::abs(duty - last_duty) > 2) {
      FMTDebug("Setting PI duty cycle: {}%\n", duty);
      last_duty = duty;
    }
  } else {
    duty_integrated = smps_parameters.user_duty.value();
  }

  if (smps_parameters.enable.value() && !smps_parameters.pi_loop_enable.value()) {
    static auto last_duty = smps_parameters.user_duty.value();
    pwm_manager.SMPSDuty(smps_parameters.user_duty.value());
    if (smps_parameters.user_duty.value() != last_duty) {
      FMTDebug("Setting user duty cycle: {}%\n", smps_parameters.user_duty.value());
      last_duty = smps_parameters.user_duty.value();
    }
  }
  if (!smps_parameters.enable.value() && !smps_parameters.pi_loop_enable.value()) {
    pwm_manager.SMPSDuty(0);
  }

  SensorData sensor_data {
      .v_in = v_in_average,
      .i_in = i_in_average,
      .v_out = v_out_average,
      .i_out = i_out_average
  };

  queue_try_add(&sensor_data_queue, &sensor_data);

  return true;  // Return true to repeat the timer
}

void main1() {
  constexpr size_t feedback_average_number = 5;
  std::array<uint16_t, feedback_average_number> v_in_feedback{};

  gpio_init(MONITOR_PIN);
  gpio_set_dir(MONITOR_PIN, GPIO_OUT);

  uart_init(uart1, 115200);

  gpio_set_function(4, GPIO_FUNC_UART);
  gpio_set_function(5, GPIO_FUNC_UART);

  repeating_timer_t timer;

  if (!add_repeating_timer_us(units::time::microsecond_t{period}.to<uint32_t>(), FeedbackLoop, nullptr, &timer)) {
    printf("Failed to create timer");
  }

  constexpr size_t queue_length = 200;

  queue_init(&v_in_samples_queue, sizeof(uint16_t), queue_length);
  queue_init(&i_in_samples_queue, sizeof(uint16_t), queue_length);
  queue_init(&v_out_samples_queue, sizeof(uint16_t), queue_length);
  queue_init(&i_out_samples_queue, sizeof(uint16_t), queue_length);

  pwm_manager.Initialize();

  while (true) {
    // Calculate error (ref - actual)
    // d is duty cycle
    // Duty cycle limits (2.5% - 90%)

    // Update the integrator (vkp and vki are constants)
    // Else case prevents duty mismatch when enabling

    auto v_in_raw = ADCRaw(ADC_Channels::voltage_in);
    queue_try_add(&v_in_samples_queue, &v_in_raw);

    auto i_in_raw = ADCRaw(ADC_Channels::current_in);
    queue_try_add(&i_in_samples_queue, &i_in_raw);

    auto v_out_raw = ADCRaw(ADC_Channels::voltage_out);
    queue_try_add(&v_out_samples_queue, &v_out_raw);

    auto i_out_raw = ADCRaw(ADC_Channels::current_out);
    queue_try_add(&i_out_samples_queue, &i_out_raw);
  }
}

int main() {
  stdio_init_all();
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  //  while(true) {
  //    gpio_put(LED_PIN, true);
  //    sleep_ms(1000);
  //    gpio_put(LED_PIN, false);
  //    sleep_ms(1000);
  //  }

  queue_init(&tune_values_queue, sizeof(TuneValues), 10);
  queue_init(&smps_parameters_queue, sizeof(SMPSParameters), 10);
  queue_init(&sensor_data_queue, sizeof(SensorData), 2);

  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);
  adc_gpio_init(28);
  adc_gpio_init(29);
  adc_set_temp_sensor_enabled(true);

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
        SMPSParameters parameters{.user_duty = duty};
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

      float v_in_multiplier = 1.0F;
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
      float v_out_multiplier = 1.0F;
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
      std::string buf{};
      fmt::format_to(std::back_inserter(buf), "v_in_samples_queue: {}\n", queue_get_level(&v_in_samples_queue));
      fmt::format_to(std::back_inserter(buf), "i_in_samples_queue: {}\n", queue_get_level(&i_in_samples_queue));
      fmt::format_to(std::back_inserter(buf), "v_out_samples_queue: {}\n", queue_get_level(&v_out_samples_queue));
      fmt::format_to(std::back_inserter(buf), "i_out_samples_queue: {}\n", queue_get_level(&i_out_samples_queue));
      fmt::format_to(std::back_inserter(buf), "tune_values_queue: {}\n", queue_get_level(&tune_values_queue));
      fmt::format_to(std::back_inserter(buf), "smps_parameters_queue: {}\n", queue_get_level(&smps_parameters_queue));
      fmt::format_to(std::back_inserter(buf), "sensor_data_queue: {}\n", queue_get_level(&sensor_data_queue));

      j["System"]["Message"] = buf;

      std::string json_str = j.dump();
      puts(json_str.data());
    }

//    j["System"]["TempC"] = CPUTemperature().to<float>();
    j.clear();

    sleep_ms(50);
  }
}
