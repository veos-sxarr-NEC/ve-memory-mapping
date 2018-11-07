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
 * @file log.cpp
 * @brief Logger for VEMM daemon
 */
#include "log.hpp"

extern "C" {
  int LOG4C_INIT();
  int LOG4C_FINI();
}

namespace log {
namespace impl {
  class Log {
  private:
    log4c_category_t *category_;
  public:
    Log (){
      LOG4C_INIT();
      this->category_ = log4c_category_get(VEMMD_LOG_CATEGORY);
      if (category_ == 0)
        throw std::runtime_error("log4c_category_gat() failed.");
    }
    ~Log() {
      LOG4C_FINI();
    }
    log4c_category_t *getcategory() { return this->category_; }
  } log_;
} // namespace impl
} // namespace log

/**
 * @brief Get log4c category for VEMMD_LOG().
 * @return Log4c category for VEMM daemon
 */
const log4c_category_t *vemmd_log_category_() {
  // log_ is constructed in static initializer.
  return log::impl::log_.getcategory();
}
