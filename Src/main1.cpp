//
// Created by treys on 2023/03/10 20:50
//

#include <algorithm>
#include <cstdint>
#include <optional>

#include "ADCHelpers.hpp"
#include "FMT/FMTToUART.hpp"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "main.hpp"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/util/queue.h"

/**
 * @brief Manages the PWM duty cycle and runs the PI loop if enabled.
 * @param rt Timer instance, unused.
 * @returns True to indicate that the timer should call the function again (repeat).
 */
bool FeedbackLoop(repeating_timer_t* rt) {
  gpio_put(MONITOR_PIN, true);  // Used to time how long the function takes
//  uart_putc(uart1, '0');

  static VTune v_in_tune = {
      // The tune values for the input voltage sensor
      .offset = 0_V,
      .multiplier = 2,
  };

  static VTune v_out_tune = {
      // The tune values for the output voltage sensor
      .offset = 0_V,
      .multiplier = 10,
  };

  static ITune i_in_tune = {
      // The tune values for the input current sensor
      .shunt = 100_mOhm,
      .offset = 1.65_V,
      .gain = 1,
  };

  static ITune i_out_tune = {
      // The tune values for the output current sensor
      .shunt = 100_mOhm,
      .offset = 1.65_V,
      .gain = 1,
  };

  static SMPSParameters smps_parameters = {
      // Parameters sent by the GUI to control the state of the SMPS
      .v_out_set_point = 0_V,
      .user_duty = 0.25,
      .enable = false,
      .pi_loop_enable = false,
  };

  TuneValues tune{};
  while (queue_try_remove(&tune_values_queue, &tune)) {  // Keep reading tune values off the queue until it is empty
    // All of these follow the basic format as follows
    if (ExtractOffset(tune.v_in_tune, v_in_tune.offset))                             // Try to read the offset value (it is std::optional) into the v_in_tune struct, and if the value was present
//      FMTDebug("V in offset changed: {}V\n", v_in_tune.offset.value().to<float>());  // Print out the new value
;
    if (ExtractMultiplier(tune.v_in_tune, v_in_tune.multiplier))
//      FMTDebug("V in multiplier changed: {}\n", v_in_tune.multiplier.value());
;

    if (ExtractOffset(tune.v_out_tune, v_out_tune.offset))
//      FMTDebug("V out offset changed: {}V\n", v_out_tune.offset.value().to<float>());
;
    if (ExtractMultiplier(tune.v_out_tune, v_out_tune.multiplier))
//      FMTDebug("V out multiplier changed: {}\n", v_out_tune.multiplier.value());
;

    if (ExtractShunt(tune.i_in_tune, i_in_tune.shunt))
//      FMTDebug("I in shunt changed: {}Ω\n", i_in_tune.shunt.value().to<float>());
;
    if (ExtractOffset(tune.i_in_tune, i_in_tune.offset))
//      FMTDebug("I in offset changed: {}V\n", i_in_tune.offset.value().to<float>());
;
    if (ExtractGain(tune.i_in_tune, i_in_tune.gain))
//      FMTDebug("I in gain changed: {}\n", i_in_tune.gain.value());
;

    if (ExtractShunt(tune.i_out_tune, i_out_tune.shunt))
//      FMTDebug("I out shunt changed: {}Ω\n", i_out_tune.shunt.value().to<float>());
;
    if (ExtractOffset(tune.i_out_tune, i_out_tune.offset))
//      FMTDebug("I out offset changed: {}V\n", i_out_tune.offset.value().to<float>());
;
    if (ExtractGain(tune.i_out_tune, i_out_tune.gain))
//      FMTDebug("I out gain changed: {}\n", i_out_tune.gain.value());
;
  }
//  uart_putc(uart1, '1');

  SMPSParameters parameters{};
  while (queue_try_remove(&smps_parameters_queue, &parameters)) {
    if (ExtractVOutSetPoint(parameters, smps_parameters.v_out_set_point))
//      FMTDebug("V out set point changed: {}V\n", smps_parameters.v_out_set_point.value().to<float>());
    ;
    if (ExtractUserDuty(parameters, smps_parameters.user_duty))
//      FMTDebug("User duty cycle changed: {}%\n", smps_parameters.user_duty.value());
    ;
    if (ExtractEnable(parameters, smps_parameters.enable))
//      FMTDebug("SMPS is now {}\n", (smps_parameters.enable.value() ? "enabled" : "disabled"));
    ;
    if (ExtractPILoopEnable(parameters, smps_parameters.pi_loop_enable))
//      FMTDebug("PI Loop is now {}\n", (smps_parameters.pi_loop_enable.value() ? "enabled" : "disabled"));
    ;
  }
//  uart_putc(uart1, '2');

  // Average all the raw values and apply corrective tune values
  auto v_in_average = AverageVoltageFromQueue(v_in_samples_queue, v_in_tune.offset.value(), v_in_tune.multiplier.value());
  auto i_in_average = AverageCurrentFromQueue(i_in_samples_queue, i_in_tune.shunt.value(), i_in_tune.offset.value(), i_in_tune.gain.value());
  auto v_out_average = AverageVoltageFromQueue(v_out_samples_queue, v_out_tune.offset.value(), v_out_tune.multiplier.value());
  auto i_out_average = AverageCurrentFromQueue(i_out_samples_queue, i_out_tune.shunt.value(), i_out_tune.offset.value(), i_out_tune.gain.value());
//  uart_putc(uart1, '3');

  static float duty_integrated = 0.0F;  // Used for the I calculations of the PI loop
  float duty = 0.0F;

  if (smps_parameters.enable.value() && smps_parameters.pi_loop_enable.value()) {  // SMPS enabled and PI loop enabled
    constexpr float duty_max = 0.90F;                                              // The max duty cycle the PI loop will every try to output
    constexpr float duty_min = 0.20F;                                              // The min duty cycle the PI loop will every try to output
    constexpr float k_p = 0.01F;                                                    // KP constant for the PI compensation
    constexpr float k_i = 0.2F;                                                   // KI constant for the PI compensation

    auto error = smps_parameters.v_out_set_point.value() - v_out_average;  // Calculate the error between desired voltage and output voltage
//    uart_putc(uart1, '4');

    duty = k_p * error.to<float>() + duty_integrated;
    duty = std::clamp(duty, duty_min, duty_max);
//    uart_putc(uart1, '5');

    duty_integrated += k_i * error.to<float>() * period.to<float>();
    duty_integrated = std::clamp(duty_integrated, duty_min, duty_max);
//    uart_putc(uart1, '6');

//    static auto last_duty = duty;
//    if (std::abs(duty - last_duty) > 0.05F) {
////      FMTDebug("Setting PI duty cycle: {}%\n", duty);
    ;
//      last_duty = duty;
//    }
  } else {
    duty_integrated = smps_parameters.user_duty.value();
//    uart_putc(uart1, '6');
  }

  if (smps_parameters.enable.value() && !smps_parameters.pi_loop_enable.value()) {
    duty = smps_parameters.user_duty.value();
//    uart_putc(uart1, '7');
//    static auto last_duty = smps_parameters.user_duty.value();
//    if (duty != last_duty) {
////      FMTDebug("Setting user duty cycle: {}%\n", duty);
    ;
//      last_duty = duty;
//    }
  }
  if (!smps_parameters.enable.value() && !smps_parameters.pi_loop_enable.value()) {
    duty = 0;
//    uart_putc(uart1, '8');
  }

  if (v_out_average > 32_V) {
    duty = 0;
//    uart_putc(uart1, '9');
  }

  pwm_manager.SMPSDuty(duty);
//  uart_putc(uart1, 'A');

  SensorData sensor_data{
      .v_in = v_in_average,
      .i_in = i_in_average,
      .v_out = v_out_average,
      .i_out = i_out_average,
  };
  queue_try_add(&sensor_data_queue, &sensor_data);
//  uart_putc(uart1, 'B');

//  Core1MiscData misc_data{
//      .duty = duty,
//      .cpu_temperature = CPUTemperature(),
//  };
//  queue_try_add(&core_1_misc_data_queue, &misc_data);

  gpio_put(MONITOR_PIN, false);
//  uart_putc(uart1, 'C');
//  uart_putc(uart1, '\n');
  return true;  // Return true to repeat the timer
}

void main1() {
  gpio_init(MONITOR_PIN);
  gpio_set_dir(MONITOR_PIN, GPIO_OUT);

  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);
  adc_gpio_init(28);
  adc_gpio_init(29);
  adc_set_temp_sensor_enabled(true);

  gpio_set_function(4, GPIO_FUNC_UART);  // Set these to whatever pins are used for core 1 UART
  gpio_set_function(5, GPIO_FUNC_UART);
  uart_init(uart1, 115200);  // This UART is used for debug messages from core 1

  repeating_timer_t timer;  // Timer used to manage the feedback loop callback

  // Create a timer which calls the feedback loop at the specified rate
  if (!add_repeating_timer_us(units::time::microsecond_t{period}.to<uint32_t>(), FeedbackLoop, nullptr, &timer)) {
//    FMTDebug("Failed to create feedback loop timer\n");
    ;
  }

  constexpr size_t queue_length = 200;  // Max number of ADC readings that will be averaged, anything sampled after the queue has reached this value is thrown away
  // Initialize all the queues that hold sampled ADC data
  queue_init(&v_in_samples_queue, sizeof(uint16_t), queue_length);
  queue_init(&i_in_samples_queue, sizeof(uint16_t), queue_length);
  queue_init(&v_out_samples_queue, sizeof(uint16_t), queue_length);
  queue_init(&i_out_samples_queue, sizeof(uint16_t), queue_length);

  queue_init(&core_1_misc_data_queue, sizeof(Core1MiscData), 10);

  pwm_manager.Initialize();

  // This is the background task, while the feedback loop function is not running, the core fills the queues with ADC readings
  // The feedback loop function will average all the ADC values pushed into the queues when it runs
  while (true) {
//    uart_putc(uart1, 'V');
    auto v_in_raw = ADCRaw(ADC_Channels::voltage_in);
    queue_try_add(&v_in_samples_queue, &v_in_raw);
//    uart_putc(uart1, 'W');

    auto i_in_raw = ADCRaw(ADC_Channels::current_in);
    queue_try_add(&i_in_samples_queue, &i_in_raw);
//    uart_putc(uart1, 'X');

    auto v_out_raw = ADCRaw(ADC_Channels::voltage_out);
    queue_try_add(&v_out_samples_queue, &v_out_raw);
//    uart_putc(uart1, 'Y');

    auto i_out_raw = ADCRaw(ADC_Channels::current_out);
    queue_try_add(&i_out_samples_queue, &i_out_raw);
//    uart_putc(uart1, 'Z');
//    uart_putc(uart1, '\n');
  }
}
