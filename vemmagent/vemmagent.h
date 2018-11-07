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
#ifndef VEOS_VEMMAGENT_H_
#define VEOS_VEMMAGENT_H_
/**
 * @file vemmagent.h
 * @brief a public header exporting VEMM agent API functions.
 */

#define VEMMAGENT_LOG_CATEGORY "veos.os_module.vemm"

#ifdef __cplusplus
/* C API */
extern "C" {
#endif

struct vemm_agent_operations {
	int (*check_pid)(pid_t);
	int (*acquire)(pid_t, uint64_t, size_t);
	int (*get_pages)(uid_t, pid_t, uint64_t, size_t, int,
			 uint64_t **, size_t *, unsigned char *);
	int (*put_pages)(uid_t, pid_t, unsigned char *);
	int (*dmaattach)(uid_t, pid_t, uint64_t, int, uint64_t *);
	int (*dmadetach)(uid_t, pid_t, uint64_t);
};

struct vemm_agent_struct;
typedef struct vemm_agent_struct vemm_agent;
vemm_agent *vemm_agent__create(const char *,
			       const struct vemm_agent_operations *);
int vemm_agent__exec(vemm_agent *);
void vemm_agent__request_stop(vemm_agent *);
void vemm_agent__destroy(vemm_agent *);

extern int LOG4C_INIT(void);
#ifdef __cplusplus
}
#endif

#endif
