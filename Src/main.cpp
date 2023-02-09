#include <cstdint>
#include <cstdio>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "json.hpp"

using namespace nlohmann;

// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
constexpr float conversion_factor = 3.3f / (1 << 12);

int main() {
  stdio_init_all();

  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);
  adc_gpio_init(28);
  adc_gpio_init(29);

  adc_select_input(0);

  // Tell GPIO 0 and 1 they are allocated to the PWM
  gpio_set_function(0, GPIO_FUNC_PWM);
  gpio_set_function(1, GPIO_FUNC_PWM);

  gpio_disable_pulls(0);
  gpio_disable_pulls(1);

  gpio_set_slew_rate(0, GPIO_SLEW_RATE_FAST);
  gpio_set_slew_rate(1, GPIO_SLEW_RATE_FAST);

  gpio_set_drive_strength(0, GPIO_DRIVE_STRENGTH_12MA);
  gpio_set_drive_strength(1, GPIO_DRIVE_STRENGTH_12MA);

  // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
  auto slice = pwm_gpio_to_slice_num(0);

  pwm_set_wrap(slice, 625 - 1);
//  pwm_set_both_levels(slice, 5000, 1250);
  pwm_set_both_levels(slice, 295, 335);
  pwm_set_phase_correct(slice, true);
  pwm_set_output_polarity(slice, false, false);
  pwm_set_enabled(slice, true); // Start PWM running

  json j;

  while(true) {
    j["in"]["volts"] = 5.218852;
    j["in"]["amps"] = 0.391453;
    std::string json_str = j.dump();

    puts(json_str.data());

    j.clear();
    sleep_ms(2000);

//  uint16_t rec_size = 0;
//
//  if (st_rx.available()) {
//    st_rx.rxObj(rx_s, rec_size);
//  }
//  std::cout << std::endl << "Decoded string: " << rx_s << std::endl;
//  ifile.close();



//    tight_loop_contents();
//    for (auto adc = 0; adc < 4; ++adc) {
//      adc_select_input(adc);
//      uint16_t result = adc_read();
//      printf("ADC #: %i, Raw value: 0x%03x, voltage: %f V\n", adc, result, result * conversion_factor);
//    }
//    sleep_ms(500);
  }
}





//#include "pico/stdlib.h"
//#include <cstdint>
//#include <cstdio>
//
//int main() {
//  stdio_init_all();
//  const uint32_t LED_PIN = PICO_DEFAULT_LED_PIN;
//  gpio_init(LED_PIN);
//  gpio_set_dir(LED_PIN, GPIO_OUT);
//  while (true) {
//    printf("Hello, World 2!\n");
//    gpio_put(LED_PIN, true);
//    sleep_ms(250);
//    gpio_put(LED_PIN, false);
//    sleep_ms(250);
//  }
//}


//#include <cstdint>
//#include <cstdio>
//
//#include "pico/stdlib.h"
//#include "hardware/pwm.h"
//#include "hardware/gpio.h"
//
//#include "json.hpp"
//#include "SerialTransferCpp.hpp"
//
//using namespace nlohmann;
//
//int main() {
//  stdio_init_all();
//  json j;
//
//  const uint32_t LED_PIN = PICO_DEFAULT_LED_PIN;
//  gpio_init(LED_PIN);
//  gpio_set_dir(LED_PIN, GPIO_OUT);
//
//  SerialConfig pico_config = {
//        .put_c = [](auto ch) { putchar_raw(ch); },
//        .get_c = []() -> int { return getchar(); },
//        .peek_c = []() { return getchar_timeout_us(5) == PICO_ERROR_TIMEOUT; }};
//
//  stcpp::SerialTransfer stxfr;
//  stxfr.begin(pico_config);
//
//  while(true) {
//    gpio_put(LED_PIN, true);
//    sleep_ms(1000);
//    j["in"]["volts"] = 5.218852;
//    j["in"]["amps"] = 0.391453;
//    std::string json_str = j.dump();
//
//    puts(json_str.data());
//    size_t send_size = 0;
//    send_size = stxfr.txObj(static_cast<size_t>(json_str.length()), send_size);
//    gpio_put(LED_PIN, false);
//    send_size = stxfr.txObj(json_str.data(), send_size, json_str.length());
//    stxfr.sendData(send_size);
//
//    j.clear();
//    sleep_ms(1000);
//  }
//}