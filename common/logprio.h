/*
 * Copyright (C) 2017-2018 NEC Corporation
 * This file is part of VE memory mapping
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
#ifndef VEMM_COMMON_LOGPRIO_H
#define VEMM_COMMON_LOGPRIO_H
/**
 * aliases for priority names in logging library
 */
enum {
  VEMM_LOG_ALERT = LOG4C_PRIORITY_ALERT,
  VEMM_LOG_CRIT = LOG4C_PRIORITY_CRIT,
  VEMM_LOG_ERR = LOG4C_PRIORITY_ERROR,
  VEMM_LOG_INFO = LOG4C_PRIORITY_INFO,
  VEMM_LOG_DEBUG = LOG4C_PRIORITY_DEBUG,
  VEMM_LOG_TRACE = LOG4C_PRIORITY_TRACE,
};
#endif
