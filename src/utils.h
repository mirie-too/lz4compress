/**************************************************************************
Copyright (c) 2014, Sergey Kulishenko <serkul(at)ukr(dot)net>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**************************************************************************/
#pragma once

#include <regex>

#include <stdint.h>

std::regex glob_to_regex(const std::string &glob);

template<class T>
std::string beautify_number(T value){
  struct comma : public std::numpunct<char>
  {
  protected:
    virtual char do_thousands_sep() const override { return ','; }
    virtual std::string do_grouping() const override { return "\03"; }
  };

  std::stringstream ss;
  ss.imbue(std::locale(std::locale(), new comma()));
  ss << std::setprecision(2) << std::fixed << value;
  return ss.str();
}

#if defined(_MSC_VER)
#define swap32 _byteswap_ulong
#else //- defined(_MSC_VER)
inline uint32_t swap32(uint32_t value)
{
  return
      ((value << 24) & 0xff000000) |
      ((value <<  8) & 0x00ff0000) |
      ((value >>  8) & 0x0000ff00) |
      ((value >> 24) & 0x000000ff);
}
#endif //- defined(_MSC_VER)

inline uint32_t uint32_le(uint32_t value)
{
  static union {
    bool isBigEndian() const { return c == 0; }

    uint32_t i;
    char c;
  } byteOrder = { 0x00112233 };

  return byteOrder.isBigEndian() ? swap32(value) : value;
}
