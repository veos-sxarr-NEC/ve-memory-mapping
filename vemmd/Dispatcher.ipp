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
 * @file Dispatcher.ipp
 * @brief implementation of Dispatcher template class
 *
 * This is included from Dispatch.hpp header since this file implements
 * a part of a template class.
 */
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

#include <stdexcept>
#include <google/protobuf/text_format.h>

#include "vemm.pb.h"
#include "log.hpp"
#include "Dispatcher.hpp"
#include "UpcallReceiver.hpp"
#include "OSDaemonProxy.hpp"

namespace dispatcher {

/**
 * @brief Create a socket accepting connections.
 *
 * @param socket_name path to socket.
 * @return the fd number of the socket created upon success.
 * @throws std::runtime_error upon failure.
 */
template <class OP, class UR> int
TmplDispatcher<OP,UR>::create_accept_socket(const char *socket_name) {
  int fd;
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    VEMMD_THROW("create socket: %s", strerror(errno));
  }
  struct sockaddr_un sa = { .sun_family = AF_UNIX, };
  strncpy(sa.sun_path, socket_name, sizeof(sa.sun_path) - 1);
  sa.sun_path[sizeof(sa.sun_path) - 1] = '\0';
  if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    VEMMD_THROW("failed to bind a socket: %s", strerror(errno));
  }
  if (listen(fd, 5) != 0) {
    VEMMD_THROW("failed to listen: %s", strerror(errno));
  }
  return fd;
}

/**
 * @brief Terminate VEMM daemon service, closing all connections from VE OS.
 */
template <class OP, class UR> void TmplDispatcher<OP, UR>::terminate_all() {
  unlink(this->accept_socket_name_.c_str());
  this->watcher_.stop();
  close(this->accept_socket_);
  
  // destruct all Upcall Receivers
  for (auto itc = this->clients_.begin(); itc != this->clients_.end(); ) {
    delete *itc;
    itc = this->clients_.erase(itc);
  }

  // destruct all OS daemon proxies
  for (auto ito = this->nodes_.begin(); ito != this->nodes_.end(); ) {
    delete *ito;
    ito = this->nodes_.erase(ito);
  }
}

/**
 * @brief Request handler
 *
 * @param c Upcall Receiver corresponding to PeerDirect device requesting.
 * @return true upon success; false upon failure.
 */
template <class OP, class UR>
bool TmplDispatcher<OP, UR>::handle_request(UR *c) {
  using google::protobuf::TextFormat;
  vemm_atb_request req;
  VEMMD_DEBUG("handle a request from %s", c->get_name().c_str());
  if (c->recv_request(req) < 0) {
    VEMMD_ERR("failed to receive a request from %s.", c->get_name().c_str());
    return false;
  }
  /* find an agent responding */
  vemm_atb_response response, result;
  result.set_result(-ESRCH);// necessary because nodes_ can be empty.
  for (auto &n: this->nodes_) {
    auto status = n->command(req, response);
    switch (status) {
    case 1:
      c->send_response(req, response);
      return true;
    case 0:
      // try next without updating result.
      VEMMD_DEBUG("node %p returned zero", n);
      break;
    default:
      VEMMD_ERR("error while requesting to VE OS (%p) (result = %ld).", n,
        response.result());
      result = response;//save the last error.
      std::string req_str;
      TextFormat::PrintToString(req, &req_str);
      VEMMD_ERR("error to handle a request: {\n%s}", req_str.c_str());
    }
  }
  std::string info_error;
  TextFormat::PrintToString(req, &info_error);
  VEMMD_ERR("No OS daemon served the request: {\n%s}",
           info_error.c_str());
  c->send_response(req, result);
  return false;
}

/**
 * @brief Callback function invoked when a peer direct device is ready.
 */
template <class OP, class UR>
void TmplDispatcher<OP, UR>::device_cb(ev::io &w, int revents) {
  auto ur = static_cast<UR *>(w.data);
  auto d = static_cast<TmplDispatcher<OP, UR> *>(ur->dispatcher_);
  d->handle_request(ur);
}

/**
 * @brief callback function invoked when a connection to VE OS is closed.
 */
template <class OP, class UR>
void TmplDispatcher<OP, UR>::proxy_cb(ev::io &w, int revents) {
  auto p = static_cast<OP *>(w.data);
  auto d = p->dispatcher_;
  if (p->test_closed()) {
    d->nodes_.erase(p);
    delete(p);
  }
}

/**
 * @brief Callback function for accepting a connection from VE OS
 */
template <class OP, class UR>
void TmplDispatcher<OP, UR>::accept_cb(ev::io &w, int revents) {
  // add a new OSDaemon proxy for VEOS (VEMM agent) connecting to this vemmd.
  auto p = new OP(this->accept_socket_, this);
  this->nodes_.insert(p);
  ev::io &wp = p->watcher_;
  // register a callback to find the connection from the agent closed.
  wp.set(this->loop_);
  wp.set<proxy_cb>(p);
  wp.start();
}

/**
 * @brief Callback function for signals
 */
template <class OP, class UR>
void TmplDispatcher<OP, UR>::signal_cb(ev::sig &w, int revents) {
  VEMMD_DEBUG("signal %d is received.", w.signum);
  w.loop.break_loop();
}

/**
 * @brief constructor of Dispatcher.
 *
 * @param socket_name a path to a socket accepting connections from VE OS
 * @param devices a vector of PeerDirect devices sending VEMM requests
 */
template <class OP, class UR>
TmplDispatcher<OP, UR>::TmplDispatcher(const std::string &socket_name,
                                       const std::vector<std::string> &devices):
  accept_socket_name_(socket_name) {
  /* open clients */
  typedef typename std::remove_pointer<decltype(this)>::type ThisClass;
  for (auto &d: devices) {
    VEMMD_DEBUG("client %s", d.c_str());
    auto ur = new UR(d, this);
    this->clients_.push_back(ur);
    ev::io &w = ur->watcher_;
    w.set(this->loop_);
    w.set<device_cb>(ur);
    w.start();
  }
  /* accept_socket for agents */
  this->accept_socket_ = create_accept_socket(socket_name.c_str());
  this->watcher_.set(this->loop_);
  this->watcher_.template set<ThisClass, &ThisClass::accept_cb>(this);
  this->watcher_.set(this->accept_socket_, ev::READ);
  this->watcher_.start();
  /* signal */
  for (auto s: { SIGINT, SIGTERM}) {
    VEMMD_DEBUG("register a watcher for signal %d", s);
    this->sigwatchers_[s].set(this->loop_);
    this->sigwatchers_[s].set(s);
    this->sigwatchers_[s].template set<ThisClass, &ThisClass::signal_cb>(this);
    this->sigwatchers_[s].start();
  }
}

/**
 * @brief Destructor
 */
template <class OP, class UR>
TmplDispatcher<OP, UR>::~TmplDispatcher() {
  this->terminate_all();
}
}
