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
#ifndef VEOS_VEMMAGENT_HPP_
#define VEOS_VEMMAGENT_HPP_
/**
 * @file VEMMAgent.hpp
 * @brief definition of VEMM agent class
 */

#include <string>
#include <map>
#include <stdexcept>

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#include <log4c.h>
#include <velayout.h>

#include "vemmagent.h"
#include "logprio.h"

class vemm_atb_request;
class vemm_atb_response;
class VEMMAgent;

/**
 * @brief VEMM agent class
 */
class VEMMAgent: private boost::noncopyable {
private:
  boost::asio::io_service io_service_;
  boost::asio::local::stream_protocol::socket socket_;
  char recvbuf_[256];
  log4c_category_t *category_;
  struct vemm_agent_operations ops_;
  std::map<int, int64_t(VEMMAgent::*)(const vemm_atb_request &,
                                      vemm_atb_response &)> cmdmap_;
  int64_t handle_acquire(const vemm_atb_request &, vemm_atb_response &);
  int64_t handle_get_pages(const vemm_atb_request &, vemm_atb_response &);
  int64_t handle_put_pages(const vemm_atb_request &, vemm_atb_response &);
  int64_t handle_dmaattach(const vemm_atb_request &, vemm_atb_response &);
  int64_t handle_dmadetach(const vemm_atb_request &, vemm_atb_response &);
  void handle_receive(const boost::system::error_code &, size_t);
public:
  explicit VEMMAgent(const char *, const struct vemm_agent_operations *,
                     log4c_category_t *);
  ~VEMMAgent();
  void run();
  void request_stop();
};

class VEMMAgentRuntimeError: public std::runtime_error {
public:
  VEMMAgentRuntimeError(const std::string &msg): std::runtime_error(msg) {}
};

#define AGENT_LOG(prio, ...) VE_LOG(this->category_, prio, __VA_ARGS__)
#define AGENT_DEBUG(...) AGENT_LOG(VEMM_LOG_DEBUG, __VA_ARGS__)
#define AGENT_ERR(...) AGENT_LOG(VEMM_LOG_ERR, __VA_ARGS__)
#define AGENT_CRIT(...) AGENT_LOG(VEMM_LOG_CRIT, __VA_ARGS__)

#endif
