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
#include "job.h"

#include <iostream>
#include <vector>

#include <lz4.h>
#include <lz4hc.h>

#include "file.h"
#include "utils.h"

static const uint32_t kMagic = uint32_le(0xDEADBEEF);

/**
 * Compressed file contains the 12 byte header (three 32 bit unsigned little
 * endian integers), in order:
 * -# magic number (0xDEADBEEF)
 * -# size of uncomressed data in bytes
 * -# size of comressed data in bytes
 */

Job::Job(const std::path &input, const std::path &output)
  : m_input(input),
    m_output(output),
    m_inputSize(std::file_size(input)),
    m_outputSize(0)
{
}

bool Job::run(std::vector<char> &ibuffer, std::vector<char> &obuffer)
{
  ibuffer.resize(static_cast<unsigned>(m_inputSize));

  if (!ibuffer.empty()) {
    File inf;
    if (!(inf.open(m_input.string().c_str(), "rb") &&
        inf.read(ibuffer.data(), ibuffer.size())))
      return error();

    obuffer.resize(LZ4_compressBound(ibuffer.size()));

    m_outputSize = LZ4_compressHC(ibuffer.data(), obuffer.data(), ibuffer.size());
    if (m_outputSize == 0)
      return error();

    obuffer.resize(static_cast<int>(m_outputSize));
  }

  uint32_t uncompressedSize = uint32_le(ibuffer.size());
  uint32_t compressedSize = uint32_le(obuffer.size());

  File outf;
  if (!(outf.open(m_output.string().c_str(), "wb") &&
      outf.write(&kMagic, sizeof(uint32_t)) &&
      outf.write(&uncompressedSize, sizeof(uint32_t)) &&
      outf.write(&compressedSize, sizeof(uint32_t)) &&
      outf.write(obuffer.data(), obuffer.size())))
    return error();

  return true;
}

void Job::dump() const
{
  std::cout << m_input << "(" << m_inputSize << ") -> " << m_output << std::endl;
}

bool Job::error() const
{
  if (errno != 0)
    perror("Error: ");
  return false;
}