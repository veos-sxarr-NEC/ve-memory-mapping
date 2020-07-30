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
 * @file OSDaemonProxy.cpp
 * @brief implementation of OSDaemonProxy
 */
#include <sstream>
#include <memory>

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "vemm.pb.h"

#include "OSDaemonProxy.hpp"
#include "log.hpp"


/**
 * @brief constructor
 *
 * @param accept_sock socket to accept a connection
 * @param d pointer to Dispatcher
 */
OSDaemonProxy::OSDaemonProxy(int accept_socket, Dispatcher *d): dispatcher_(d) {
  this->fd_ = accept(accept_socket, NULL, NULL);
  if (this->fd_ == -1) {
    VEMMD_THROW("accept: %s", strerror(errno));
  }
  this->watcher_.set(this->fd_, ev::READ);
  struct ucred cred;
  socklen_t ucred_size=sizeof(struct ucred);
  if (getsockopt(this->fd_, SOL_SOCKET, SO_PEERCRED,
      &cred, &ucred_size) == -1) {
    VEMMD_THROW("getsockopt: %s", strerror(errno));
  }
  this->pid_ = cred.pid;
}

OSDaemonProxy::~OSDaemonProxy() {
  shutdown(this->fd_, SHUT_RDWR);
  close(this->fd_);
}

/*
 * @brief Delegate the specified request to VE OS daemon
 *
 * @param[in] req request from PeerDirect, dispatched by Dispatcher.
 * @param[out] res response from VE OS
 * @return 1 upon success. Negative upon failure.
 *         0 when the OS daemon does not serve the specified area.
 */
int OSDaemonProxy::command(vemm_atb_request &req,
                           vemm_atb_response &res) {
  VEMMD_DEBUG("invoked: send message to fd %d", this->fd_);
  if (!req.SerializeToFileDescriptor(this->fd_)) {
    VEMMD_ERR("failed to send message to fd %d", this->fd_);
    return -1;
  }
  // sizeof(uint64_t) * 2048 (= the number of PCIATB entry) = 16 KiB.
  // allocate a buffer enough to store the reply from VEMM agent.
  constexpr size_t buffer_size = 20 * 1024;
  std::unique_ptr<char[]> recvbuf(new char[buffer_size]);
  auto rv = read(this->fd_, recvbuf.get(), buffer_size);
  if (rv < 0) {
    VEMMD_ERR("failed to receive message from fd %d (errno=%d)",
              this->fd_, errno);
    res.set_result(-errno);
    return -errno;
  } else if (rv == 0) {
    VEMMD_ERR("connection (fd %d) is unexpectedly closed.", this->fd_);
    res.set_result(-EPIPE);
    return -EPIPE;
  }
  vemm_atb_response tmpres;
  if (!tmpres.ParseFromArray(recvbuf.get(), rv)) {
    VEMMD_THROW("failed to parse message from fd %d", this->fd_);
  }
  auto result = tmpres.result();
  VEMMD_DEBUG("result = %ld", result);
  if (result == -ESRCH) {
    VEMMD_DEBUG("VEOS does not respond to the request (pid = %d)",
      req.pid());
    return 0;
  }
  res = tmpres;
  if (result < 0) {
    //some error
    VEMMD_ERR("VEOS returns an error (%ld)", result);
    return result;
  }
  return 1;
}

/**
 * @brief Test a socket to VE OS is closed
 *
 * @return true when a socket is closed.
 *
 * This function is used as a callback when the socket gets readable.
 */
bool OSDaemonProxy::test_closed() {
  char buffer[256];
  auto rv = read(this->fd_, buffer, sizeof(buffer));
  if (rv != 0) {
    int e = errno;
    VEMMD_ERR("read() returned unexpected result (%d)", (int)rv);
    if (rv > 0) {
      std::ostringstream o;
      for (auto &&c: buffer) {
        o << std::hex << c << ' ';
      }
      std::string str = o.str();
      VEMMD_ERR("buffer: %s", str.c_str());
    } else if (e != EINTR) {
      VEMMD_THROW("read() failed unexpectedly (errno = %d)", e);
    } else { // EINTR
      VEMMD_DEBUG("interrupted on testing closed.");
    }
    return false;
  }
  VEMMD_DEBUG("fd %d is closed", this->fd_);
  return true;
}
