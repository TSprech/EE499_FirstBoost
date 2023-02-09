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

constexpr auto startLineLength = 8;

/*
 *  read a line of any  length from stdio (grows)
 *
 *  @param fullDuplex input will echo on entry (terminal mode) when false
 *  @param linebreak defaults to "\n", but "\r" may be needed for terminals
 *  @return entered line on heap - don't forget calling free() to get memory back
 */
static char * getLine(bool fullDuplex = true, char lineBreak = '\n') {
  // th line buffer
  // will allocated by pico_malloc module if <cstdlib> gets included
  char * pStart = (char*)malloc(startLineLength);
  char * pPos = pStart;  // next character position
  size_t maxLen = startLineLength; // current max buffer size
  size_t len = maxLen; // current max length
  int c;

  if(!pStart) {
    return NULL; // out of memory or dysfunctional heap
  }

  while(1) {
    c = getchar(); // expect next character entry
    if(c == EOF || c == lineBreak) {
      break;     // non blocking exit
    }

    if (fullDuplex) {
      putchar(c); // echo for fullDuplex terminals
    }

    if(--len == 0) { // allow larger buffer
      len = maxLen;
      // double the current line buffer size
      char *pNew  = (char*)realloc(pStart, maxLen *= 2);
      if(!pNew) {
        free(pStart);
        return NULL; // out of memory abort
      }
      // fix pointer for new buffer
      pPos = pNew + (pPos - pStart);
      pStart = pNew;
    }

    // stop reading if lineBreak character entered
    if((*pPos++ = c) == lineBreak) {
      break;
    }
  }

  *pPos = '\0';   // set string end mark
  return pStart;
}

int main() {
  stdio_init_all();

//  adc_init();
//  adc_gpio_init(26);
//  adc_gpio_init(27);
//  adc_gpio_init(28);
//  adc_gpio_init(29);
//
//  adc_select_input(0);
//
//  // Tell GPIO 0 and 1 they are allocated to the PWM
//  gpio_set_function(0, GPIO_FUNC_PWM);
//  gpio_set_function(1, GPIO_FUNC_PWM);
//
//  gpio_disable_pulls(0);
//  gpio_disable_pulls(1);
//
//  gpio_set_slew_rate(0, GPIO_SLEW_RATE_FAST);
//  gpio_set_slew_rate(1, GPIO_SLEW_RATE_FAST);
//
//  gpio_set_drive_strength(0, GPIO_DRIVE_STRENGTH_12MA);
//  gpio_set_drive_strength(1, GPIO_DRIVE_STRENGTH_12MA);
//
//  // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
//  auto slice = pwm_gpio_to_slice_num(0);
//
//  pwm_set_wrap(slice, 625 - 1);
////  pwm_set_both_levels(slice, 5000, 1250);
//  pwm_set_both_levels(slice, 295, 335);
//  pwm_set_phase_correct(slice, true);
//  pwm_set_output_polarity(slice, false, false);
//  pwm_set_enabled(slice, true); // Start PWM running

  const uint32_t LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  json j;
//  std::array<char, 200> s{};

  while(true) {
    char *pLine = getLine(true, '\r');
    if (!*pLine) {
      printf("returned empty - nothing blocked");
    } else {
      std::string_view s = std::string_view(pLine, strlen(pLine));
      printf("entered: %s\r\n\r\n", pLine);
      j = json::parse(s);
      if (j["LED"] == "1") {
        gpio_put(LED_PIN, true);
      } else {
        gpio_put(LED_PIN, false);
      }
      free(pLine); // dont forget freeing buffer !!
    }

//    j["in"]["volts"] = 5.218852;
//    j["in"]["amps"] = 0.391453;
//    std::string json_str = j.dump();
//
//    puts(json_str.data());
//
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


//#include <stdio.h>
//#include <cstdlib>          // need this include, if you need heap management (malloc, realloc, free)
//#include <pico/stdlib.h>    // needed for Pico SDK support
//
//const uint startLineLength = 8; // the linebuffer will automatically grow for longer lines
//const char eof = 255;           // EOF in stdio.h -is -1, but getchar returns int 255 to avoid blocking
//
///*
// *  read a line of any  length from stdio (grows)
// *
// *  @param fullDuplex input will echo on entry (terminal mode) when false
// *  @param linebreak defaults to "\n", but "\r" may be needed for terminals
// *  @return entered line on heap - don't forget calling free() to get memory back
// */
//static char * getLine(bool fullDuplex = true, char lineBreak = '\n') {
//  // th line buffer
//  // will allocated by pico_malloc module if <cstdlib> gets included
//  char * pStart = (char*)malloc(startLineLength);
//  char * pPos = pStart;  // next character position
//  size_t maxLen = startLineLength; // current max buffer size
//  size_t len = maxLen; // current max length
//  int c;
//
//  if(!pStart) {
//    return NULL; // out of memory or dysfunctional heap
//  }
//
//  while(1) {
//    c = getchar(); // expect next character entry
//    if(c == eof || c == lineBreak) {
//      break;     // non blocking exit
//    }
//
//    if (fullDuplex) {
//      putchar(c); // echo for fullDuplex terminals
//    }
//
//    if(--len == 0) { // allow larger buffer
//      len = maxLen;
//      // double the current line buffer size
//      char *pNew  = (char*)realloc(pStart, maxLen *= 2);
//      if(!pNew) {
//        free(pStart);
//        return NULL; // out of memory abort
//      }
//      // fix pointer for new buffer
//      pPos = pNew + (pPos - pStart);
//      pStart = pNew;
//    }
//
//    // stop reading if lineBreak character entered
//    if((*pPos++ = c) == lineBreak) {
//      break;
//    }
//  }
//
//  *pPos = '\0';   // set string end mark
//  return pStart;
//}
//
//int main() {
//    stdio_init_all(); // needed for redirect stdin/stdout to Pico's USB or UART ports
//
//    while(1) {
//        char *pLine = getLine(true, '\r');
//        if (!*pLine) {
//            printf("returned empty - nothing blocked");
//        } else {
//            printf("entered: %s\r\n\r\n", pLine);
//            free(pLine); // dont forget freeing buffer !!
//        }
//    }
//}