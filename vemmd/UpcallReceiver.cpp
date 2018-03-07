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
 * @file UpcallReceiver.cpp
 * @brief implementation of UpcallReceiver
 */
#include "UpcallReceiver.hpp"
#include <google/protobuf/text_format.h>
#include <memory>

#include <unistd.h>
#include <fcntl.h>

#include <vepm.h>

#include "vemm.pb.h"

#include "log.hpp"

/**
 * @brief Constructor
 *
 * @param device_name path to a PeerDirect device file
 * @param d pointer to Dispatcher
 */
UpcallReceiver::UpcallReceiver(const std::string &device_name, Dispatcher *d):
  name_(device_name), dispatcher_(d) {
  this->fd_ = open(device_name.c_str(), O_RDWR | O_NONBLOCK);
  if (this->fd_ == -1) {
    VEMMD_THROW("open %s: %s", device_name.c_str(), strerror(errno));
  }
  // watch the device. This class does not have an event handler.
  // Dispatcher registers an event handler regarding this file, instead.
  this->watcher_.set(this->fd_, ev::READ);
}

UpcallReceiver::~UpcallReceiver() {
  close(this->fd_);
}

/**
 * @brief Receive a request from PeerDirect device
 * Dispatcher uses this method to get a new request from Upcall Receiver.
 *
 * @param[out] req a request received from PeerDirect device
 * @return 0 upon success; negative upon failure.
 */
int UpcallReceiver::recv_request(vemm_atb_request &req) {
  int rv;
  struct vepm_atb_request r;
  do {
    rv = ioctl(this->fd_, VEPM_ATB_REQUEST, &r);
  } while (rv == -1 && errno == EINTR);
  if (rv == -1) {
    VEMMD_ERR("ioctl: %s", strerror(errno));
    return -errno;
  }
  VEMMD_DEBUG("event = %d, pid = %d, vaddr = %#lx, size = %#lx, write = %d",
              r.event, (int)r.pid, r.vaddr, r.size, r.write);
  req.set_event(r.event);
  req.set_pid(r.pid);
  req.set_vaddr(r.vaddr);
  req.set_size(r.size);
  req.set_write(r.write);
  req.set_euid(r.euid);
  // specify size since veshm_id can include '\0'.
  std::string veshm_id(reinterpret_cast<char *>(r.veshm_id),
                       sizeof(r.veshm_id));
  req.set_veshm_id(veshm_id);
  return 0;
}

/**
 * @brief send a response to PeerDirect device
 *
 * @param[in] req a request received from PeerDirect device
 * @param[in] res a response to the request specified by req.
 * @return 0 upon success; negative upon failure.
 */
int UpcallReceiver::send_response(const vemm_atb_request &req,
                                  const vemm_atb_response &res) {
  int rv;
  struct vepm_atb_request r;
  r.event = req.event();
  r.pid = req.pid();
  r.vaddr = req.vaddr();
  r.size = req.size();
  std::unique_ptr<uint64_t []> additional_data;
  if (res.additional_size() > 0) {
    additional_data.reset(new uint64_t[res.additional_size()]);
    for (int i = 0; i < res.additional_size(); ++i) {
      additional_data[i] = res.additional(i);
    }
    r.result = reinterpret_cast<intptr_t>(additional_data.get());
  } else {
    r.result = res.result();
  }
  if (res.has_veshm_id()) {
    memcpy(r.veshm_id, res.veshm_id().c_str(), sizeof(r.veshm_id));
  }
  rv = ioctl(this->fd_, VEPM_ATB_RESPONSE, &r);
  if (rv == -1) {
    VEMMD_ERR("ioctl: %s", strerror(errno));
    return -errno;
  }
  return 0;
}
