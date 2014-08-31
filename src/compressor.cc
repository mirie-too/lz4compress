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
#include "compressor.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <thread>

#include <tclap/CmdLine.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#endif //- defined(WIN32)

#include "fs.h"
#include "utils.h"

//- uncomment to enable logging
//#define COMPRESSOR_LOGGING

using std::cout;
using std::cerr;
using std::endl;

const char Compressor::kVersion[] = "0.1";

Compressor::Compressor()
  : m_input("*.*"),
    m_outDir(),
    m_newExt(),
    m_numJobs(0)
{
}

bool Compressor::run(int argc, char *argv[])
{
  cout << "LZ4 compress tool version " << kVersion << endl;

#if defined(COMPRESSOR_LOGGING)
  cerr << "Max input size = " << beautify_number(kMaxInputSize) << endl;
  cerr << "Working directory = " << std::current_path<std::path>() << endl;
#endif //- defined(COMPRESSOR_LOGGING)

  Compressor c;
  return
    c.parseCommandLine(argc, argv) &&
    c.prepareJobs() &&
    c.executeJobs();
}

bool Compressor::parseCommandLine(int argc, char *argv[])
{
  TCLAP::CmdLine cmd("LZ4 compressor", ' ', kVersion);

  TCLAP::ValueArg<std::string> iArg("i", "input",
    "Input files (wildcards are allowed)", true, "*.*", "files");
  TCLAP::ValueArg<std::string> oArg("o", "output",
    "Output directory", true, ".", "dir");
  TCLAP::ValueArg<std::string> eArg("e", "extension",
    "New extension of output files", false, "", "extension");
  TCLAP::ValueArg<unsigned> jArg("j", "jobs",
    "Number of parallel jobs", false, 0, "jobs");

  cmd.add(iArg);
  cmd.add(oArg);
  cmd.add(eArg);
  cmd.add(jArg);
  cmd.parse(argc, argv);

  m_input = iArg.getValue();
  m_outDir = oArg.getValue();
  m_newExt = eArg.getValue();
  m_numJobs = jArg.getValue();

  return true;  //- in the case of any exceptions they will be
                //- handled by the TCLAP::CmdLine itself
}

bool Compressor::prepareJobs()
{
  unsigned numCPUs = (std::max)(1u, std::thread::hardware_concurrency());
  if (m_numJobs == 0 || m_numJobs > 4 * numCPUs)
    m_numJobs = numCPUs + 1;
  cout << numCPUs << " CPU(s) detected, "
       << m_numJobs <<  " jobs started" << endl;

  std::path input_path(m_input);
  std::path output_path(m_outDir);
  std::regex filter(glob_to_regex(input_path.filename()));

  //- enumerate all input files, create jobs
  for (std::directory_iterator i(std::path(input_path.parent_path()));
    i != std::directory_iterator(); ++i) {
    if (std::is_regular_file(i->status()) &&
      std::regex_match(i->path().string(), filter)) {
      std::path input = i->path();
      std::path output(output_path);
      output /= input.filename();
      if (!m_newExt.empty())
        output.replace_extension(m_newExt);
      Job job(input, output);
      if (job.inputSize() > kMaxInputSize) {
        //! @todo provide more information (file size, limit etc)
        std::cerr << input << ": file is too big, skipped" << std::endl;
      } else {
        m_jobs.push_back(job);
      }
    }
  }

  /*-
   * Split the jobs to nearly equal size parts.
   * The simplest way: sort the jobs, split them to parts starting from the
   * largest one. Not the best algorithm, but it works great in the case
   * of input files with comparable sizes. Rigorous solution at glance seems
   * to be a kind of knapsack problem, brainstorm it later, i hope :).
   */

  //- sort
  std::sort(m_jobs.begin(), m_jobs.end(), [](const Job &lhs, const Job &rhs) {
    return lhs.inputSize() > rhs.inputSize();
  });

  //- calculate input size per worker
  Job::Size totalSize = std::accumulate(m_jobs.begin(), m_jobs.end(),
    static_cast<Job::Size>(0), [](Job::Size sum, const Job &job) {
    return sum + job.inputSize();
  });
  Job::Size partSize = totalSize / m_numJobs;

  //- split to parts
  m_bounds.push_back(0);
  for (unsigned i = 0; i < m_jobs.size();) {
    Job::Size s = 0;
    while (i < m_jobs.size() && s < partSize)
      s += m_jobs[i++].inputSize();
    m_bounds.push_back(i);
  }

  /*-
   * Splitted, parts are:
   * part 0: m_bounds[0]..m_bounds[1]
   * part 1: m_bounds[1]..m_bounds[2]
   * part i: m_bounds[i]..m_bounds[i + 1]
   */

#if defined(COMPRESSOR_LOGGING)
  for (unsigned i = 1; i < m_bounds.size(); i++) {
    unsigned begin = m_bounds[i - 1];
    unsigned end = m_bounds[i];
    Job::Size size = 0;
    for (unsigned j = begin; j < end; j++) {
      size += m_jobs[j].inputSize();
    }
    cerr << "Part #" << i << ": [" << begin << ", " << end << "), size = " << beautify_number(size) << endl;
  }
#endif //- defined(COMPRESSOR_LOGGING)

  return true;
}

bool Compressor::executeJobs()
{
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;

  high_resolution_clock::time_point timer = high_resolution_clock::now();

  //- spawn workers
  std::vector<std::thread> workers;
  std::atomic<bool> error(false);
  std::atomic<Job::Size> inputBytes(0), outputBytes(0);
  for (unsigned i = 1; i < m_bounds.size(); i++) {
    unsigned begin = m_bounds[i - 1];
    unsigned end = m_bounds[i];
    workers.push_back(std::thread(
      [&error, &inputBytes, &outputBytes](Job *begin, Job *end) {
      Job::Size ibytes = 0, obytes = 0;
      std::vector<char> ibuffer, obuffer;
      while (!error && begin < end) {
        ibytes += begin->inputSize();
        if (!begin->run(ibuffer, obuffer)) {
          error = true;
          cerr << "Error compressing " << begin->input() << "!" << endl;
        }
        obytes += begin->outputSize();
        begin++;
      }
      inputBytes += ibytes;
      outputBytes += obytes;
    }, m_jobs.data() + begin, m_jobs.data() + end));
  }

  //- wait till job is done
  for (auto &w : workers) w.join();

#if defined(WIN32)
  FILETIME CreationTime, ExitTime, KernelTime, UserTime;
  if (GetProcessTimes(GetCurrentProcess(),
                      &CreationTime,
                      &ExitTime,
                      &KernelTime,
                      &UserTime)) {
    TCHAR szBuf[MAX_PATH];
    ULONGLONG qwTime;

    cout << std::string(40, '-') << endl;
    qwTime = (((ULONGLONG) KernelTime.dwHighDateTime) << 32)
      + KernelTime.dwLowDateTime;
    StringCchPrintf(szBuf, MAX_PATH, 
        TEXT("Kernel time: %0.3f sec\n"), 1e-7 * qwTime);
    cout << szBuf;
    qwTime = (((ULONGLONG) UserTime.dwHighDateTime) << 32)
      + UserTime.dwLowDateTime;
    StringCchPrintf(szBuf, MAX_PATH, 
        TEXT("User time  : %0.3f sec\n"), 1e-7 * qwTime);
    cout << szBuf;
    cout << std::string(40, '-') << endl;
  }
#endif //- defined(WIN32)

  milliseconds elapsedTime =  std::chrono::duration_cast<milliseconds>
    (high_resolution_clock::now() - timer);
  double ratio = static_cast<double>(outputBytes.load()) / inputBytes.load();

  cout << "Time  : " << beautify_number(0.001 * elapsedTime.count()) << " sec\n";
  cout << "Input : " << beautify_number(inputBytes.load()) << " bytes\n";
  cout << "Output: " << beautify_number(outputBytes.load()) << " bytes\n";
  cout << "Ratio : " << beautify_number(100.0 * ratio) << '%' << endl;

  return !error;
}
