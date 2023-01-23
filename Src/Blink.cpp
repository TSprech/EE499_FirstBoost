#include "pico/stdlib.h"
#include <cstdint>
#include <cstdio>

int main() {
  stdio_init_all();
  const uint32_t LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  while (true) {
    printf("Hello, World 2!\n");
    gpio_put(LED_PIN, true);
    sleep_ms(250);
    gpio_put(LED_PIN, false);
    sleep_ms(250);
  }
}
