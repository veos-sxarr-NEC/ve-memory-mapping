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
#ifndef VEOS_VEPM_H_INCLUDE
#define VEOS_VEPM_H_INCLUDE

#ifdef __KERNEL__
#include <linux/ioctl.h>
#define __BEGIN_DECLS /* kernel is in C */
#define __END_DECLS /* kernel is in C */
#else
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#endif

__BEGIN_DECLS

struct vepm_atb_request_v1 {
	int event;
	pid_t pid;
	long vaddr;
	long size;
	long result;
	unsigned char veshm_id[16];
	int write;
	uid_t euid;
};

struct vepm_atb_request {
        int event;
        pid_t pid;
        long vaddr;
        long size;
        long result;
        unsigned char veshm_id[16];
        int write;
        uid_t euid;
	int veos_pid;
};

#define VEPM_ATB_REQUEST_V1 _IOWR('v', 0xe2, struct vepm_atb_request_v1)
#define VEPM_ATB_RESPONSE_V1 _IOWR('v', 0xe3, struct vepm_atb_request_v1)
#define VEPM_ATB_REQUEST _IOWR('v', 0xe4, struct vepm_atb_request)
#define VEPM_ATB_RESPONSE _IOWR('v', 0xe5, struct vepm_atb_request)

enum {
	VEPM_ACQUIRE = 0,
	VEPM_GET_PAGES,
	VEPM_PUT_PAGES,
	VEPM_DMAATTACH,
	VEPM_DMADETACH,
};

__END_DECLS

#endif /* VEOS_VEPM_H_INCLUDE */
