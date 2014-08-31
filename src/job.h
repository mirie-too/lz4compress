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

#include <vector>

#include "fs.h"

class Job {
public:
  typedef decltype(std::file_size(std::path())) Size;

  Job(const std::path &input, const std::path &output);

  std::string input() const { return m_input.string(); }
  
  Size inputSize() const { return m_inputSize; }
  Size outputSize() const { return m_outputSize; }

  bool run(std::vector<char> &ibuffer, std::vector<char> &obuffer);

  void dump() const;

private:
  bool error() const;

  std::path m_input, m_output;
  Size m_inputSize, m_outputSize;
};