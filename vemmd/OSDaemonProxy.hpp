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
#ifndef VEOS_VEMM_OSDAEMONPROXY_HPP_
#define VEOS_VEMM_OSDAEMONPROXY_HPP_
/**
 * @file OSDadmonProxy.hpp
 * @brief definition of OSDaemonProxy class
 */

#include <boost/noncopyable.hpp>
#include "Dispatcher.hpp"

#include <ev++.h>

class vemm_atb_request;
class vemm_atb_response;

/**
 * @brief OS daemon proxy
 */
class OSDaemonProxy : private boost::noncopyable {
private:
  int fd_;
  int pid_;
  ev::io watcher_;
  Dispatcher *dispatcher_;
  friend Dispatcher;

public:
  OSDaemonProxy(int, Dispatcher *);
  virtual ~OSDaemonProxy();
  /*
   * @brief Delegate the specified request to VE OS daemon
   * @return 1 upon success. Negative upon failure.
   *         0 when the OS daemon does not serve the specified area.
   */
  virtual int command(vemm_atb_request &, vemm_atb_response &);
  virtual bool test_closed();
  virtual int get_veospid() { return this->pid_; }
};
#endif
