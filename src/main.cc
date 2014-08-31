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
#include <iostream>

#include "compressor.h"

#define FAKE_ARGS

int main(int argc, char *argv[])
{
#if defined(FAKE_ARGS)
  int fake_argc = 0;
  char* fake_argv[] = {
    "lz4tool.exe",
    //"-i", "c:/windows/cursors/*",
    "-i", "c:/windows/system32/*",
    "-o", "./out",
    "-e", "lz4",
    "-j", "0",
    nullptr
  };
  while (fake_argv[fake_argc] != nullptr)
    fake_argc++;
  return Compressor::run(fake_argc, fake_argv) ? 0 : -1;
#else //- defined(FAKE_ARGS)
  return Compressor::run(argc, argv) ? 0 : -1;
#endif //- defined(FAKE_ARGS)
}
