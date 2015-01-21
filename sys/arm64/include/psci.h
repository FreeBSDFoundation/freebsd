/*-
 * Copyright (c) 2013, 2014 Robin Randhawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef	_MACHINE_PSCI_H_
#define	_MACHINE_PSCI_H_

#include <sys/types.h>

typedef int (*psci_initfn_t)(device_t dev);
typedef int (*psci_callfn_t)(uint64_t, uint64_t, uint64_t, uint64_t);

extern int psci_present;

void	psci_system_reset(void);
int	psci_cpu_on(unsigned long cpu, unsigned long entry, unsigned long context_id);

/*
 * PSCI return codes.
 */
#define	PSCI_RETVAL_SUCCESS		0
#define	PSCI_RETVAL_NOT_SUPPORTED	-1
#define	PSCI_RETVAL_INVALID_PARAMS	-2
#define	PSCI_RETVAL_DENIED		-3
#define	PSCI_RETVAL_ALREADY_ON		-4
#define	PSCI_RETVAL_ON_PENDING		-5
#define	PSCI_RETVAL_INTERNAL_FAILURE	-6
#define	PSCI_RETVAL_NOT_PRESENT		-7
#define	PSCI_RETVAL_DISABLED		-8

/*
 * PSCI function codes (as per PSCI v0.2).
 *
 * These assume that PSCI function calls are made from AARCH32 context
 * and that PSCI function arguments are uint32_t.
 *
 */
#define	PSCI_FNID_VERSION		0x84000000
#define	PSCI_FNID_CPU_SUSPEND		0x84000001
#define	PSCI_FNID_CPU_OFF		0x84000002
#define	PSCI_FNID_CPU_ON		0x84000003
#define	PSCI_FNID_AFFINITY_INFO		0x84000004
#define	PSCI_FNID_MIGRATE		0x84000005
#define	PSCI_FNID_MIGRATE_INFO_TYPE	0x84000006
#define	PSCI_FNID_MIGRATE_INFO_UP_CPU   0x84000007
#define	PSCI_FNID_SYSTEM_OFF		0x84000008
#define	PSCI_FNID_SYSTEM_RESET		0x84000009

#define	PSCI_VER_MAJOR(v)		((v >> 16) & 0xFF)
#define	PSCI_VER_MINOR(v)		(v & 0xFF)

enum err {
	SUCCESS,
	FAILURE
};

static __inline int
psci_hvc_despatch(uint64_t psci_fnid, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
	__asm volatile(
	    "mov    %x0, %0     \n"
	    "mov    %x1, %1     \n"
	    "mov    %x2, %2     \n"
	    "mov    %x3, %3     \n"
	    "hvc #0             \n"
	    : "+r" (psci_fnid)
	    : "r" (arg0), "r" (arg1), "r" (arg2)
	    );

	return psci_fnid;
}

static __inline int
psci_smc_despatch(uint64_t psci_fnid, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
	__asm volatile(
	    "mov    %x0, %0     \n"
	    "mov    %x1, %1     \n"
	    "mov    %x2, %2     \n"
	    "mov    %x3, %3     \n"
	    "smc #0             \n"
	    : "+r" (psci_fnid)
	    : "r" (arg0), "r" (arg1), "r" (arg2)
	    );

	return psci_fnid;
}

#endif /* _MACHINE_PSCI_H_ */
