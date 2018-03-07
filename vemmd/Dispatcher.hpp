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
#ifndef VEOS_VEMM_DISPATCHER_HPP_
#define VEOS_VEMM_DISPATCHER_HPP_
/**
 * @file Dispatcher.hpp
 * @brief definition of Dispatcher class
 */

#include <set>
#include <map>
#include <vector>
#include <string>
#include <csignal>

#include <ev++.h>

#include <boost/noncopyable.hpp>

namespace dispatcher {

/**
 * @brief TmplDispatcher
 *
 * defined as a templated class for easier testing:
 * apply mocks to this template at tests
 */
template <class OP, class UR> class TmplDispatcher: private boost::noncopyable {
private:
  ev::dynamic_loop loop_;
  int accept_socket_;
  std::string accept_socket_name_;
  ev::io watcher_;
  std::map<int, ev::sig> sigwatchers_;
  std::set<OP *> nodes_;
  std::vector<UR *> clients_;

  static int create_accept_socket(const char *);

  static void device_cb(ev::io &, int);
  static void proxy_cb(ev::io &, int);
  virtual void accept_cb(ev::io &, int);
  virtual void signal_cb(ev::sig &, int);

  virtual bool handle_request(UR *c);
  virtual void terminate_all();

public:
  TmplDispatcher(const std::string &sv_sock_name,
             const std::vector<std::string> &devices);
  virtual ~TmplDispatcher();
  virtual void run() {
    this->loop_.run();
  }
};
} // namespace dispatcher

// Instantiation
class OSDaemonProxy;
class UpcallReceiver;
typedef dispatcher::TmplDispatcher<OSDaemonProxy, UpcallReceiver> Dispatcher;
#include "Dispatcher.ipp"
#endif
