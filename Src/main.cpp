#include <cstdint>
#include <cstdio>
#include <numeric>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "json.hpp"

#include "GetLine.hpp"

constexpr auto LED_PIN = PICO_DEFAULT_LED_PIN;
//constexpr auto LED_PIN = 13;

using namespace nlohmann;

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

float v_in_offset = 0;
float v_in_multiplier = 1.0F;

float v_out_offset = 0;
float v_out_multiplier = 1.0F;

float i_in_offset = 1.65F;
float i_in_gain = 1.0F;

float i_out_offset = 1.65F;
float i_out_gain = 1.0F;

struct PWMManager {
  explicit PWMManager(uint8_t slice_number) : slice_number_(slice_number) {}

  [[nodiscard]]
  auto Top() const -> uint32_t {
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

    a_level -= deadband_/2;
    b_level += deadband_/2;

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
      pwm_set_enabled(slice_number_, true); // Start PWM running
  }

  const uint8_t slice_number_;
  uint16_t deadband_ = 24;
};

enum class ADC_Channels : uint8_t {
  voltage_in = 0,
  current_in = 1,
  voltage_out = 2,
  current_out = 3,
  cpu_temperature = 4
};

auto ADCVoltage(ADC_Channels channel) -> float {
  constexpr float conversion_factor = 3.3f / (1 << 12); // 12-bit conversion, assume max value == ADC_VREF == 3.3 V

  adc_select_input(static_cast<uint>(channel));
  auto adc_value = adc_read();
  return static_cast<float>(adc_value) * conversion_factor;
}

constexpr auto RawToVoltage(uint16_t adc_raw) -> float {
  constexpr float conversion_factor = 3.3f / (1 << 12); // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
  return static_cast<float>(adc_raw) * conversion_factor;
}


auto ADCRaw(ADC_Channels channel) -> uint16_t {
  adc_select_input(static_cast<uint>(channel));
  return adc_read();
}

auto ADCCurrent(ADC_Channels channel, uint16_t shunt_mohm, float offset, float gain) -> float {
  auto raw = ADCVoltage(channel);
  auto shunt_voltage = (raw-offset) * gain;
  return shunt_voltage / (static_cast<float>(shunt_mohm) / 1000);
}

float ADCTemperatureC() {
  constexpr float conversion_factor = 3.3f / (1 << 12); // 12-bit conversion, assume max value == ADC_VREF == 3.3 V

  adc_select_input(static_cast<uint>(ADC_Channels::cpu_temperature));
  float adc = (float)adc_read() * conversion_factor;
  return 27.0F - (adc - 0.706F) / 0.001721F;
}

constexpr size_t averaging_array_size = 300;
std::array<uint16_t, averaging_array_size> v_in_readings{};
std::array<uint16_t, averaging_array_size> v_out_readings{};
std::array<uint16_t, averaging_array_size> i_in_readings{};
std::array<uint16_t, averaging_array_size> i_out_readings{};

PWMManager pwm_manager(pwm_gpio_to_slice_num(0));

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

  pwm_manager.Initialize();

  std::array<char, 256> buf{};

  json j;

  while(true) {
    auto in_str = GetLine(buf, '\r');
    if (!in_str.empty()) {
      j = json::parse(in_str);
      if (j.contains("SMPS")) {
        if (j["SMPS"].contains("Duty")) pwm_manager.SMPSDuty(j["SMPS"]["Duty"]);
        if (j["SMPS"].contains("DeadBand")) pwm_manager.DeadBand(j["SMPS"]["DeadBand"]);
        if (j["SMPS"].contains("Frequency")) pwm_manager.SMPSFrequencyKHz(j["SMPS"]["Frequency"]);
      }
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

    j["SMPS"]["In"]["VVolts"] = v_in_average;
    j["SMPS"]["In"]["IVolts"] = v_out_average;
    j["SMPS"]["Out"]["VVolts"] = i_in_average;
    j["SMPS"]["Out"]["IVolts"] = i_out_average;

    j["System"]["Temp"] = ADCTemperatureC();
    std::string json_str = j.dump();
    puts(json_str.data());
    j.clear();

    sleep_ms(10);
  }
}



///*
// * This file is part of the MicroPython project, http://micropython.org/
// *
// * The MIT License (MIT)
// *
// * Copyright (c) 2020-2021 Damien P. George
// *
// * Permission is hereby granted, free of charge, to any person obtaining a copy
// * of this software and associated documentation files (the "Software"), to deal
// * in the Software without restriction, including without limitation the rights
// * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// * copies of the Software, and to permit persons to whom the Software is
// * furnished to do so, subject to the following conditions:
// *
// * The above copyright notice and this permission notice shall be included in
// * all copies or substantial portions of the Software.
// *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// * THE SOFTWARE.
// */
//
//#include "py/runtime.h"
//#include "py/mphal.h"
//#include "modmachine.h"
//
//#include "hardware/clocks.h"
//#include "hardware/pwm.h"
//
///******************************************************************************/
//// MicroPython bindings for machine.PWM
//
//typedef struct _machine_pwm_obj_t {
//  mp_obj_base_t base;
//  uint8_t slice;
//  uint8_t channel;
//  uint8_t duty_type;
//  mp_int_t duty;
//} machine_pwm_obj_t;
//
//enum {
//  DUTY_NOT_SET = 0,
//  DUTY_U16,
//  DUTY_NS
//};
//
//machine_pwm_obj_t machine_pwm_obj[] = {
//      {{&machine_pwm_type}, 0, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 0, PWM_CHAN_B, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 1, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 1, PWM_CHAN_B, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 2, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 2, PWM_CHAN_B, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 3, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 3, PWM_CHAN_B, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 4, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 4, PWM_CHAN_B, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 5, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 5, PWM_CHAN_B, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 6, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 6, PWM_CHAN_B, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 7, PWM_CHAN_A, DUTY_NOT_SET, 0},
//      {{&machine_pwm_type}, 7, PWM_CHAN_B, DUTY_NOT_SET, 0},
//};
//
//void mp_machine_pwm_duty_set_u16(machine_pwm_obj_t *self, mp_int_t duty_u16);
//
//void mp_machine_pwm_duty_set_ns(machine_pwm_obj_t *self, mp_int_t duty_ns);
//
//void mp_machine_pwm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
//  machine_pwm_obj_t *self = MP_OBJ_TO_PTR(self_in);
//  mp_printf(print, "<PWM slice=%u channel=%u>", self->slice, self->channel);
//}
//
//// PWM(pin)
//mp_obj_t mp_machine_pwm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
//  // Check number of arguments
//  mp_arg_check_num(n_args, n_kw, 1, 1, false);
//
//  // Get GPIO to connect to PWM.
//  uint32_t gpio = mp_hal_get_pin_obj(all_args[0]);
//
//  // Get static peripheral object.
//  uint slice = pwm_gpio_to_slice_num(gpio);
//  uint8_t channel = pwm_gpio_to_channel(gpio);
//  machine_pwm_obj_t *self = &machine_pwm_obj[slice * 2 + channel];
//  self->duty_type = DUTY_NOT_SET;
//
//  // Select PWM function for given GPIO.
//  gpio_set_function(gpio, GPIO_FUNC_PWM);
//
//  return MP_OBJ_FROM_PTR(self);
//}
//
//void mp_machine_pwm_deinit(machine_pwm_obj_t *self) {
//  self->duty_type = DUTY_NOT_SET;
//  pwm_set_enabled(self->slice, false);
//}
//
//// Returns: floor((16*F + offset) / div16)
//// Avoids overflow in the numerator that would occur if
////   16*F + offset > 2**32
////   F + offset/16 > 2**28 = 268435456 (approximately, due to flooring)
//uint32_t get_slice_hz(uint32_t offset, uint32_t div16) {
//  uint32_t source_hz = clock_get_hz(clk_sys);
//  if (source_hz + offset / 16 > 268000000) {
//    return (16 * (uint64_t) source_hz + offset) / div16;
//  } else {
//    return (16 * source_hz + offset) / div16;
//  }
//}
//
//// Returns 16*F / denom, rounded.
//uint32_t get_slice_hz_round(uint32_t div16) {
//  return get_slice_hz(div16 / 2, div16);
//}
//
//// Returns ceil(16*F / denom).
//uint32_t get_slice_hz_ceil(uint32_t div16) {
//  return get_slice_hz(div16 - 1, div16);
//}
//
//mp_obj_t mp_machine_pwm_freq_get(machine_pwm_obj_t *self) {
//  uint32_t div16 = pwm_hw->slice[self->slice].div;
//  uint32_t top = pwm_hw->slice[self->slice].top;
//  uint32_t pwm_freq = get_slice_hz_round(div16 * (top + 1));
//  return MP_OBJ_NEW_SMALL_INT(pwm_freq);
//}
//
//void mp_machine_pwm_freq_set(machine_pwm_obj_t *self, mp_int_t freq) {
//// Set the frequency, making "top" as large as possible for maximum resolution.
//// Maximum "top" is set at 65534 to be able to achieve 100% duty with 65535.
//#define TOP_MAX 65534
//  uint32_t source_hz = clock_get_hz(clk_sys);
//  uint32_t div16;
//  uint32_t top;
//
//  if ((source_hz + freq / 2) / freq < TOP_MAX) {
//// If possible (based on the formula for TOP below), use a DIV of 1.
//// This also prevents overflow in the DIV calculation.
//    div16 = 16;
//
//// Same as get_slice_hz_round() below but canceling the 16s
//// to avoid overflow for high freq.
//    top = (source_hz + freq / 2) / freq - 1;
//  } else {
//// Otherwise, choose the smallest possible DIV for maximum
//// duty cycle resolution.
//// Constraint: 16*F/(div16*freq) < TOP_MAX
//// So:
//    div16 = get_slice_hz_ceil(TOP_MAX * freq);
//
//// Set TOP as accurately as possible using rounding.
//    top = get_slice_hz_round(div16 * freq) - 1;
//  }
//
//
//  if (div16 < 16) {
//    mp_raise_ValueError(MP_ERROR_TEXT("freq too large"));
//  } else if (div16 >= 256 * 16) {
//    mp_raise_ValueError(MP_ERROR_TEXT("freq too small"));
//  }
//  pwm_hw->slice[self->slice].div = div16;
//  pwm_hw->slice[self->slice].top = top;
//  if (self->duty_type == DUTY_U16) {
//    mp_machine_pwm_duty_set_u16(self, self->duty);
//  } else if (self->duty_type == DUTY_NS) {
//    mp_machine_pwm_duty_set_ns(self, self->duty);
//  }
//}
//
//mp_obj_t mp_machine_pwm_duty_get_u16(machine_pwm_obj_t *self) {
//  uint32_t top = pwm_hw->slice[self->slice].top;
//  uint32_t cc = pwm_hw->slice[self->slice].cc;
//  cc = (cc >> (self->channel ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB)) & 0xffff;
//
//// Use rounding (instead of flooring) here to give as accurate an
//// estimate as possible.
//  return MP_OBJ_NEW_SMALL_INT((cc * 65535 + (top + 1) / 2) / (top + 1));
//}
//
//void mp_machine_pwm_duty_set_u16(machine_pwm_obj_t *self, mp_int_t duty_u16) {
//  uint32_t top = pwm_hw->slice[self->slice].top;
//
//// Use rounding here to set it as accurately as possible.
//  uint32_t cc = (duty_u16 * (top + 1) + 65535 / 2) / 65535;
//  pwm_set_chan_level(self->slice, self->channel, cc);
//  pwm_set_enabled(self->slice, true);
//  self->duty = duty_u16;
//  self->duty_type = DUTY_U16;
//}
//
//mp_obj_t mp_machine_pwm_duty_get_ns(machine_pwm_obj_t *self) {
//  uint32_t slice_hz = get_slice_hz_round(pwm_hw->slice[self->slice].div);
//  uint32_t cc = pwm_hw->slice[self->slice].cc;
//  cc = (cc >> (self->channel ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB)) & 0xffff;
//  return MP_OBJ_NEW_SMALL_INT(((uint64_t) cc * 1000000000ULL + slice_hz / 2) / slice_hz);
//}
//
//void mp_machine_pwm_duty_set_ns(machine_pwm_obj_t *self, mp_int_t duty_ns) {
//  uint32_t slice_hz = get_slice_hz_round(pwm_hw->slice[self->slice].div);
//  uint32_t cc = ((uint64_t) duty_ns * slice_hz + 500000000ULL) / 1000000000ULL;
//  if (cc > 65535) {
//    mp_raise_ValueError(MP_ERROR_TEXT("duty larger than period"));
//  }
//  pwm_set_chan_level(self->slice, self->channel, cc);
//  pwm_set_enabled(self->slice, true);
//  self->duty = duty_ns;
//  self->duty_type = DUTY_NS;
//}
