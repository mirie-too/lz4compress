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

#include <string>

#include <limits.h>

#include "job.h"

class Compressor {
public:
  static const char kVersion[];
  static const Job::Size kMaxInputSize =
    static_cast<Job::Size>(2) * (1 << 30) - 1; //- 2GB

  static bool run(int argc, char *argv[]);

private:
  Compressor();

  bool parseCommandLine(int argc, char *argv[]);
  bool prepareJobs();
  bool executeJobs();

  std::string m_input;
  std::string m_outDir;
  std::string m_newExt;
  unsigned m_numJobs;
  std::vector<Job> m_jobs;
  std::vector<unsigned> m_bounds;
};