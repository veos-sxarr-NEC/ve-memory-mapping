/*
 * Copyright (C) 2017-2018 NEC Corporation
 * This file is part of VE memory mapping.
 *
 * VE memory mapping is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * VE memory mapping is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with VE memory mapping; if not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file main.cpp
 * @brief main routine of VEMM daemon
 **/

#include <cstdio>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <csignal>
#include <iostream>

#include <vepm.h>
#include <systemd/sd-daemon.h>

#include <boost/program_options.hpp>

#include "log.hpp"
#include "Dispatcher.hpp"
#include "config.h"

namespace vemmdmain {
namespace impl {
/**
 * @brief Initialize signal handlers
 * Ignore SIGPIPE and block all signals except SIGINT and SIGTERM.
 */
void init_signal_handler(void) {
  /* register a signal handler */
  signal(SIGPIPE, SIG_IGN);
  /* block all signals */
  sigset_t ss;
  sigfillset(&ss);
  sigdelset(&ss, SIGINT);
  sigdelset(&ss, SIGTERM);

  sigprocmask(SIG_BLOCK, &ss, NULL);
}
} // namespace impl

int vemmd_main(int argc, const char *argv[])
{
  namespace po = boost::program_options;
  using std::string;
  using std::vector;
  
  po::options_description desc("Usage: vemmd -s socket upcalldevice...");
  desc.add_options()
    ("socket,s", po::value<string>(), "socket path");
  desc.add_options()
    ("version,V", "output version information");
  po::options_description hidden("hidden");
  hidden.add_options()
    ("upcallfiles", po::value< vector<string> >(), "upcall device file(s)");
  po::positional_options_description pos_op;
  pos_op.add("upcallfiles", -1);

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::variables_map vm;

  try {
    auto parsed = po::command_line_parser(argc, argv).options(cmdline_options)
                    .positional(pos_op).run();
    po::store(parsed, vm);
  } catch (po::error &e) {
    std::cerr << desc << std::endl;
    exit(1);
  }

  if (vm.count("version") > 0) {
    // print version
    std::cout << "vemmd (veos) " VERSION << std::endl;
    exit(0);
  }
  if (vm.count("socket") != 1 || vm.count("upcallfiles") < 1) {
    std::cerr << desc << std::endl;
    exit(1);
  }

  impl::init_signal_handler();

  string sock(vm["socket"].as<string>());
  vector<string> devices(vm["upcallfiles"].as< vector<string> >());

  Dispatcher d(sock, devices);
  sd_notify(0, "READY=1");
  d.run();
  sd_notify(0, "STOPPING=1");
  return 0;
}
} // namespace vemmdmain
