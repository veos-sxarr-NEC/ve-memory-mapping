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
#ifndef VEOS_VEMM_UPCALLRECEIVER_HPP_
#define VEOS_VEMM_UPCALLRECEIVER_HPP_
/**
 * @file UpcallReceiver.hpp
 * @brief definition of UpcallReceiver
 */
#include <string>
#include <boost/noncopyable.hpp>
#include <ev++.h>

class vemm_atb_request;
class vemm_atb_response;

class OSDaemonProxy;
class UpcallReceiver;
namespace dispatcher {
template <class OP, class UR> class TmplDispatcher;
} // namespace dispatcher

/**
 * @brief Upcall receiver to receive upcall requests from a PeerDirect device
 */
class UpcallReceiver : private boost::noncopyable {
private:
  std::string name_;
  int fd_;
  ev::io watcher_;
  typedef dispatcher::TmplDispatcher<OSDaemonProxy, UpcallReceiver> Dispatcher;
  Dispatcher *dispatcher_;
  friend Dispatcher;

public:
  UpcallReceiver(const std::string &, Dispatcher *);
  virtual ~UpcallReceiver();

  virtual int recv_request(vemm_atb_request &);
  virtual int send_response(const vemm_atb_request &,
                            const vemm_atb_response &);
  const std::string &get_name() { return this->name_; }
};
#endif
