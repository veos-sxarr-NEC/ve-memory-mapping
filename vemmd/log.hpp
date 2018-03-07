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
#ifndef VEOS_VEMM_LOG_H
#define VEOS_VEMM_LOG_H
/**
 * @file log.hpp
 * @brief macros using Logger for VEMM daemon
 */
#include <velayout.h>
#include "logprio.h"

#include <stdexcept>

#define VEMMD_LOG_CATEGORY "veos.vemmd"
#define VEMMD_LOG(prio, ...) VE_LOG(vemmd_log_category_(), prio, __VA_ARGS__)

#define VEMMD_DEBUG(...) VEMMD_LOG(VEMM_LOG_DEBUG, __VA_ARGS__)
#define VEMMD_ERR(...) VEMMD_LOG(VEMM_LOG_ERR, __VA_ARGS__)
#define VEMMD_CRIT(...) VEMMD_LOG(VEMM_LOG_CRIT, __VA_ARGS__)

#define VEMMD_THROW(...) do { \
  VEMMD_CRIT(__VA_ARGS__); \
  throw std::runtime_error(__func__); \
} while (0)


extern "C" {
  const log4c_category_t *vemmd_log_category_(void);
}

#endif
