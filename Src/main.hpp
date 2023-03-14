//
// Created by treys on 2023/10/03 20:51
//

#ifndef MAIN_HPP
#define MAIN_HPP

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

#include "FMT/FMTToUART.hpp"

#include "hardware/clocks.h"

#include "TypedUnits.hpp"

/**
 * @brief A constexpr version of pwm_gpio_to_slice_num.
 * @param gpio
 * @returns
 */
inline constexpr auto PWMGPIOToSliceNum(uint gpio) {
  return (gpio >> 1u) & 7u;
}

inline auto CPUFrequency() -> units::frequency::hertz_u32_t {
  return units::frequency::hertz_u32_t{clock_get_hz(clk_sys)};
}

struct PWMManager {
//  explicit PWMManager(uint8_t slice_number)
//      : slice_number_(slice_number) {}

  constexpr PWMManager(uint8_t a_pin, uint8_t b_pin)
      : slice_number_(PWMGPIOToSliceNum(a_pin)), a_pin_(a_pin), b_pin_(b_pin) {}

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

  auto SMPSFrequency(units::frequency::hertz_u32_t frequency) {
    auto top = (CPUFrequency() / frequency / 2) - 1;
    pwm_set_wrap(slice_number_, top);
  }

//  auto SMPSFrequencyKHz() -> uint16_t {
//    constexpr auto f_sys_khz = 125000000 / 1000;
//    return 1 / ((Top() + 1) * 2 / f_sys_khz);
//  }

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

  /**
   * @brief Changes the duty cycle of the A and B channels of the PWM channel.
   * @param duty_cycle The duty cycle represented as a values between 0 and 1.
   */
  auto SMPSDuty(float duty_cycle) const -> void {
    //    duty = duty_cycle / 100;
    auto top = Top();
    uint16_t a_level;
    uint16_t b_level;
    if (duty_cycle < 0.01) {
      a_level = 0;
      b_level = 0;
    } else if (duty_cycle > 0.99) {
      a_level = top;
      b_level = top;
    } else {
      a_level = std::floor(static_cast<float>(top) * duty_cycle);
      b_level = a_level;
      a_level -= deadband_;
      b_level += deadband_;
    }

    pwm_set_both_levels(slice_number_, a_level, b_level);
  }

  auto Initialize() -> void {
    if (PWMGPIOToSliceNum(a_pin_) != PWMGPIOToSliceNum(b_pin_)) {
      FMTDebug("A and B PWM pins are not part of the same slice, this means they will not switch synchronously\n");
    }

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(a_pin_, GPIO_FUNC_PWM);
    gpio_set_function(b_pin_, GPIO_FUNC_PWM);

    gpio_disable_pulls(a_pin_);
    gpio_disable_pulls(b_pin_);

    gpio_set_slew_rate(a_pin_, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(b_pin_, GPIO_SLEW_RATE_FAST);

    gpio_set_drive_strength(a_pin_, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(b_pin_, GPIO_DRIVE_STRENGTH_12MA);

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
  const uint8_t a_pin_;
  const uint8_t b_pin_;
  uint16_t deadband_ = 24;
};

inline PWMManager pwm_manager(0, 1);

inline queue_t v_in_samples_queue;
inline queue_t i_in_samples_queue;
inline queue_t v_out_samples_queue;
inline queue_t i_out_samples_queue;

inline queue_t tune_values_queue;
inline queue_t smps_parameters_queue;
inline queue_t sensor_data_queue;
inline queue_t core_1_misc_data_queue;

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

struct SensorData {
  units::voltage::volt_t v_in;
  units::current::ampere_t i_in;
  units::voltage::volt_t v_out;
  units::current::ampere_t i_out;
};

struct Core1MiscData {
  float duty;
  units::temperature::celsius_t cpu_temperature;
};

inline constexpr auto rate = 50_Hz;                       // Whatever the PI loop call rate will be
inline constexpr units::time::second_t period = 1 / rate;  // Convert the Hz to period
inline constexpr auto MONITOR_PIN = 2;

void main1();

#define EXTRACT_FUNCTION(func_name, struct_type, variable_type, member_name)              \
  inline auto func_name(std::optional<struct_type> tune_struct, variable_type& variable)->bool { \
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

#endif  // MAIN_HPP
