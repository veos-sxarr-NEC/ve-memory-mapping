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
 * @file vemmagent.cpp
 * @brief implementation of VEMM agent
 */
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <google/protobuf/text_format.h>

#include "vemm.pb.h"

#include <vepm.h>

#include "vemmagent.h"
#include "VEMMAgent.hpp"

namespace {
  /**
   * @brief convert a message to string
   *
   * @param m a message in a protobuf message type
   * @return human-readable string describing content of m
   */
  std::string message_to_string(const google::protobuf::Message &m) {
    std::string str;
    google::protobuf::TextFormat::PrintToString(m, &str);
    return str;
  }
}
#define PRINT_TRACE_MESSAGE(name, message) do { \
  auto m_ = message_to_string(message); \
  AGENT_DEBUG("%s(%s, _)", name, m_.c_str()); \
} while (0)

/**
 * @brief handler for VEPM_ACQUIRE request
 *
 * @param[in] req request from VEMM daemon
 * @param[out] res response for reply to VEMM daemon
 *
 * @return 0 upon success; negative upon failure.
 */
int64_t VEMMAgent::handle_acquire(const vemm_atb_request &req,
                              vemm_atb_response &res) {
  PRINT_TRACE_MESSAGE("VEMMAgent::handle_acquire", req);
  auto pid = req.pid();
  uint64_t vaddr_start = req.vaddr();
  size_t size = req.size();

  if (!this->ops_.check_pid(pid)) {
    AGENT_DEBUG("VE OS does not server process %d.", static_cast<int>(pid));
    return -ESRCH;
  }
  // check whether the virtual memory region resides on the process.
  return this->ops_.acquire(pid, vaddr_start, size);
}

/**
 * @brief handler for VEPM_GET_PAGES request
 *
 * @param[in] req request from VEMM daemon
 * @param[out] res response for reply to VEMM daemon
 *
 * @return the number of pages mapped from BAR01 window
 */
int64_t VEMMAgent::handle_get_pages(const vemm_atb_request &req,
                                    vemm_atb_response &res) {
  PRINT_TRACE_MESSAGE("VEMMAgent::handle_get_pages", req);
  auto euid = req.euid();
  auto pid = req.pid();
  uint64_t vaddr = req.vaddr();
  size_t size = req.size();
  bool writable = req.write();
  uint64_t *addr_arrayp;
  size_t addr_array_size;
  unsigned char veshm_id[16];
  if (!this->ops_.check_pid(pid)) {
    AGENT_DEBUG("VE OS does not server process %d.", static_cast<int>(pid));
    return -ESRCH;
  }
  int rv = this->ops_.get_pages(euid, pid, vaddr, size, writable,
                                    &addr_arrayp, &addr_array_size, veshm_id);
  if (rv <= 0) {
    AGENT_ERR("VE OS failed to handle the request (%d)", rv);
    return rv;
  }
  for (size_t i = 0; i < addr_array_size; ++i) {
    res.add_additional(addr_arrayp[i]);
  }
  res.set_veshm_id(reinterpret_cast<char *>(veshm_id), sizeof(veshm_id));
  std::free(addr_arrayp);
  return rv;
}

/**
 * @brief handler for VEPM_PUT_PAGES request
 *
 * @param[in] req request from VEMM daemon
 * @param[out] res response for reply to VEMM daemon
 *
 * @return 0 upon success; negative upon failure
 */
int64_t VEMMAgent::handle_put_pages(const vemm_atb_request &req,
                                    vemm_atb_response &res) {
  PRINT_TRACE_MESSAGE("VEMMAgent::handle_put_pages", req);
  auto euid = req.euid();
  auto pid = req.pid();
  unsigned char veshm_id[16];
  // do not check PID because the process can be dead.
  memcpy(veshm_id, req.veshm_id().data(), sizeof(veshm_id));
  return this->ops_.put_pages(euid, pid, veshm_id);
}

/**
 * @brief handler for VEPM_DMAATTACH request
 *
 * @param[in] req request from VEMM daemon
 * @param[out] res response for reply to VEMM daemon
 *
 * @return VEHVA attached upon success; negative upon failure
 */
int64_t VEMMAgent::handle_dmaattach(const vemm_atb_request &req,
                                    vemm_atb_response &res) {
  PRINT_TRACE_MESSAGE("VEMMAgent::handle_dmaattach", req);
  auto euid = req.euid();
  auto pid = req.pid();
  auto vhva = req.vaddr();
  auto writable = req.write();
  uint64_t vehva;
  if (!this->ops_.check_pid(pid)) {
    AGENT_DEBUG("VE OS does not server process %d.", static_cast<int>(pid));
    return -ESRCH;
  }
  int64_t rv = this->ops_.dmaattach(euid, pid, vhva, writable, &vehva);
  return rv == 0 ? vehva : rv;
}

/**
 * @brief handler for VEPM_DMADETACH request
 *
 * @param[in] req request from VEMM daemon
 * @param[out] res response for reply to VEMM daemon
 *
 * @return zero upon success; negative upon failure
 */
int64_t VEMMAgent::handle_dmadetach(const vemm_atb_request &req,
                                    vemm_atb_response &res) {
  PRINT_TRACE_MESSAGE("VEMMAgent::handle_dmadetach", req);
  auto euid = req.euid();
  auto pid = req.pid();
  auto vehva = req.vaddr();
  if (!this->ops_.check_pid(pid)) {
    AGENT_DEBUG("VE OS does not server process %d.", static_cast<int>(pid));
    return -ESRCH;
  }
  return this->ops_.dmadetach(euid, pid, vehva);
}

namespace asio = boost::asio;
typedef struct vemm_agent_operations ops_t;

/**
 * @brief constructor
 *
 * @param socketname path name to the socket of VEMM daemon
 * @param ops a set of functions in VE OS to handle requests
 */
VEMMAgent::VEMMAgent(const char *socketname, const ops_t *ops,
                     log4c_category_t *logcat):
  socket_(this->io_service_), category_(logcat), ops_(*ops), cmdmap_({
    {VEPM_ACQUIRE, &VEMMAgent::handle_acquire},
    {VEPM_GET_PAGES, &VEMMAgent::handle_get_pages},
    {VEPM_PUT_PAGES, &VEMMAgent::handle_put_pages},
    {VEPM_DMAATTACH, &VEMMAgent::handle_dmaattach},
    {VEPM_DMADETACH, &VEMMAgent::handle_dmadetach},
  }) {
  AGENT_DEBUG("Initializing VEMM agent (vemmd socket = %s)", socketname);
  this->socket_.connect(asio::local::stream_protocol::endpoint(socketname));
  this->socket_.async_read_some(asio::buffer(this->recvbuf_),
                                boost::bind(&VEMMAgent::handle_receive,
                                this, asio::placeholders::error,
                                asio::placeholders::bytes_transferred));
  AGENT_DEBUG("VEMM agent %p is initialized.", this);
}

/**
 * @brief destructor
 */
VEMMAgent::~VEMMAgent() {
}

/**
 * @brief event handler on receiving a request form VEMM daemon
 *
 * @param[out] result of operation
 * @param size the number of bytes read from the socket in VEMMAgent
 */
void VEMMAgent::handle_receive(const boost::system::error_code &e, size_t size) {
  AGENT_DEBUG("VEMMAgent::handle_receive(size=%lu)", size);
  if (e) {
    auto msgstr = e.message();
    AGENT_ERR("receive failed: %s", msgstr.c_str());
    throw VEMMAgentRuntimeError(msgstr);
  }
  vemm_atb_request req;
  if (!req.ParseFromArray(this->recvbuf_, size)) {
    AGENT_ERR("Parse failed.");
    throw VEMMAgentRuntimeError("handle_receive: ParseFromArray() failed.");
  }

  // invoke a handler corresponding to the request
  auto cmd = req.event();

  vemm_atb_response res;

  auto rv = (this->*cmdmap_[cmd])(req, res);
  res.set_result(rv);
  AGENT_DEBUG("send response.");
  // send response via socket
  std::string sendbuf = res.SerializeAsString();
  {
    auto msgstr = message_to_string(res);
    AGENT_DEBUG("response message (length = %u):\n%s",
      static_cast<unsigned>(sendbuf.size()), msgstr.c_str());
  }
  asio::write(this->socket_, asio::buffer(sendbuf));
  AGENT_DEBUG("complete sending response.");
  // set this event handler again
  this->socket_.async_read_some(asio::buffer(this->recvbuf_),
                                boost::bind(&VEMMAgent::handle_receive,
                                this, asio::placeholders::error,
                                asio::placeholders::bytes_transferred));
}

/**
 * @brief Run VEMM agent
 */
void VEMMAgent::run() {
  this->io_service_.run();
}

/**
 * @brief request this VEMM agent to stop
 */
void VEMMAgent::request_stop() {
  this->io_service_.stop();
}

/* C I/F */
/**
 * @brief create VEMM agent
 *
 * @param s path name to the socket of VEMM daemon
 * @param ops a set of functions in VE OS to handle requests
 *
 * @return pointer to a new VEMM agent
 */
vemm_agent *vemm_agent__create(const char *s, const ops_t *ops) {
  log4c_init();
  vemm_agent *rv = 0;
  auto cat = log4c_category_get(VEMMAGENT_LOG_CATEGORY);
  if (cat == 0) {
    return 0;
  }
  try {
    rv = reinterpret_cast<vemm_agent *>(new VEMMAgent(s, ops, cat));
  } catch (std::exception &e) {
    VE_LOG(cat, VEMM_LOG_ERR, "failed to create VEMM agent: %s", e.what());
  } catch (...) {
    VE_LOG(cat, VEMM_LOG_ERR, "caused an exception in creating VEMM agent.");
  }
  return rv;
}

/**
 * @brief delete VEMM agent
 *
 * @param a pointer to VEMM agent
 */
void vemm_agent__destroy(vemm_agent *a) {
  VEMMAgent *ap = reinterpret_cast<VEMMAgent *>(a);
  delete ap;
}

/**
 * @brief run VEMM agent on the current thread
 *
 * @param a VEMM agent
 * @return zero upon success; negative upon failure.
 */
int vemm_agent__exec(vemm_agent *a) {
  VEMMAgent *ap = reinterpret_cast<VEMMAgent *>(a);
  try {
    ap->run();
  } catch (...) {
    return -1;
  }
  return 0;
}

/**
 * @brief request VEMM agent to stop
 *
 * @param a VEMM agent to stop
 */
void vemm_agent__request_stop(vemm_agent *a) {
  VEMMAgent *ap = reinterpret_cast<VEMMAgent *>(a);
  try {
    ap->request_stop();
  } catch (...) {
    //Catch all exceptions here to prevent exceptions from going to C code.
  }
}
