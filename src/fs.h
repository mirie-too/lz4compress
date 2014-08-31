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

#if (_MSC_VER == 1700) || (_MSC_VER == 1800)   //- MSVC2012, MSVS2013
#include <filesystem>

//- put them in their normal c++14 place
namespace std
{
  using tr2::sys::path;
  using tr2::sys::directory_iterator;
  using tr2::sys::is_regular_file;
  using tr2::sys::file_size;
  using tr2::sys::current_path;
}
#else
//- need boost's filesystem module or similar?
#error "Not supported yet!"
#endif