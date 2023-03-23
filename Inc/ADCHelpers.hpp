//
// Created by treys on 2023/10/03 20:57
//

#ifndef ADCHELPERS_HPP
#define ADCHELPERS_HPP

#include <cstdint>
#include "hardware/adc.h"
#include "pico/util/queue.h"

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

/**
 * @brief Calculates the raw average of all values in a queue, the values represent ADC readings.
 * @note Empties the queue passed to it.
 * @warning Using a 12 bit ADC, this can average ~1M samples before the sum overflows.
 * @param queue The queue which contains the ADC readings.
 * @returns The average of all queue values.
 */
auto ADCAverageFromQueue(queue_t& queue) -> uint16_t {
  uint32_t sum = 0;
  size_t number_of_data_points = 0;
  uint16_t current_data = 0;
  while (queue_try_remove(&queue, &current_data)) {  // Keep going through the queue until no values are left
    sum += current_data;
    ++number_of_data_points;
  }
  return sum / number_of_data_points;  // Calculate the average
}

auto AverageVoltageFromQueue(queue_t& queue) {
  return RawToVoltage(ADCAverageFromQueue(queue));
}

auto AverageVoltageFromQueue(queue_t& queue, units::voltage::volt_t offset, float multiplier) {
  return RawToVoltage(ADCAverageFromQueue(queue), offset, multiplier);
}

auto AverageCurrentFromQueue(queue_t& queue, units::impedance::ohm_t shunt, units::voltage::volt_t offset, float gain) {
  return RawToCurrent(ADCAverageFromQueue(queue), shunt, offset, gain);
}

#endif  // ADCHELPERS_HPP
