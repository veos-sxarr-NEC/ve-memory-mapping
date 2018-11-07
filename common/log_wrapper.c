/*
 * Copyright (C) 2018 NEC Corporation
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
 * @file log_wrapper.c
 * @brief Wrapper for log4c_init() and log4c_fini().
 *        log4c library v1.2.1 (default of debian 9) has a bug which causes
 *        link error when functions in "log4c/init.h" is used from .cpp files.
 *        This file solve the bug. This is temporary measures.
 */
#include <velayout.h>

int LOG4C_INIT(void)
{
	return log4c_init();
}

int LOG4C_FINI(void)
{
	return log4c_fini();
}
