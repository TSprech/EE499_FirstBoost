//
// Created by treys on 2023/02/09 15:55:40.
//
#ifndef GETLINE_HPP
#define GETLINE_HPP

#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

/**
 * @brief Reads a line from the USB serial port without blocking.
 * @warning This function does not guarantee a line has been read as it can timeout. Check the returned iterator against the beginning of the buffer to see if valid data has been read.
 * @code {.cpp}
 * auto str = GetLine(buf);
 *   if (!in_str.empty()) {
 *     // Code dealing with the string here
 *   }
 * @endcode
 * @param buffer A char buffer which will be used to read in the characters and which the returned string_view is based on.
 * @param eol_indicator The character that indicates the end of a line, data is read until this is reached.
 * @param timeout_us The amount of time to wait for a character, in µs, before concluding no data is available and moving on.
 * @param echo If true, the characters read in will be printed back to the console as they are processed.
 * @returns A string view representing the data that has been read in.
 */
inline auto GetLine(std::span<char> buffer, char eol_indicator = '\n', uint32_t timeout_us = 50, bool echo = false) -> std::string_view {
  size_t end_index = 0;  // Keeps track of the index to the next available buffer element to write received data into

  while (true) {
    auto c = getchar_timeout_us(timeout_us);          // Nonblocking timeout based get char, if no serial data is available, pass
    if (c == EOF || c == eol_indicator) [[unlikely]]  // The end of the line is either EOL or the user provided delimiter
      break;
    else [[likely]]  // If the end of the line hasn't been reached, place the data in the buffer
      buffer[end_index++] = c;

    if (echo) putchar(c);  // Echo the char received if needed
  }

  buffer[end_index] = '\0';           // Once this is reached, a full line has been read but still needs its null terminator, so it is added
  return {buffer.data(), end_index};  // Return the iterator to the end of the received data (may not be end of buffer)
}

#endif  // GETLINE_HPP
