//
// Created by treys on 2023/02/09 15:55:40.
//

#include <cstdint>
#include <cstdio>
#include <span>

#ifndef GETLINE_HPP
#define GETLINE_HPP

inline auto GetLine(std::span<char> buffer, char eol_indicator = '\n', uint32_t timeout_us = 25, bool echo = false) -> char* {
  int32_t end_index = 0;

  while (true) {
    auto c = getchar_timeout_us(timeout_us);
//    auto c = getchar();
    if (c == EOF || c == eol_indicator) [[unlikely]]
      break;
    else [[likely]]
      buffer[end_index++] = c;

    if (echo) putchar(c);  // echo for fullDuplex terminals
  }

  buffer[end_index] = '\0';  // set string end mark
  return (buffer.begin() + end_index).base();
}

#endif  // GETLINE_HPP
