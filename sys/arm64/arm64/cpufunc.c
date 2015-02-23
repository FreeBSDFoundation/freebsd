/*-
 * Copyright (c) 2014 Robin Randhawa
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
 */

/*
 * This cpufuncs implementation is based on the ARM v7 implementation.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <machine/cpu.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/uma.h>

#include <machine/cpufunc.h>

/* PRIMARY CACHE VARIABLES */
int	arm64_picache_size;
int	arm64_picache_line_size;
int	arm64_picache_ways;

int	arm64_pdcache_size;	/* and unified */
int	arm64_pdcache_line_size;
int	arm64_pdcache_ways;

int	arm64_pcache_type;
int	arm64_pcache_unified;

int	arm64_dcache_align;
int	arm64_dcache_align_mask;

u_int	arm64_cache_level;
u_int	arm64_cache_type[14];
u_int	arm64_cache_loc;

/* 1 == use cpu_sleep(), 0 == don't */
int cpu_do_powersave;
int ctrl;

struct cpu_functions arm64_cpufuncs = {
	/* CPU functions */
	
	arm64_id,                     /* id                   */
	arm64_nullop,                 /* cpwait               */
	
	/* MMU functions */
	
	arm64_control,                /* control              */
	arm64_setttb,                 /* Setttb               */
	arm64_faultstatus,            /* Faultstatus          */
	arm64_faultaddress,           /* Faultaddress         */
	
	/* 
	 * TLB functions.  ARMv7 does all TLB ops based on a unified TLB model
	 * whether the hardware implements separate I+D or not, so we use the
	 * same 'ID' functions for all 3 variations.
	 */
	
	arm64_tlb_flushID,              /* tlb_flushID          */
	arm64_tlb_flushID_SE,           /* tlb_flushID_SE       */
	arm64_tlb_flushID,              /* tlb_flushI           */
	arm64_tlb_flushID_SE,           /* tlb_flushI_SE        */
	arm64_tlb_flushID,              /* tlb_flushD           */
	arm64_tlb_flushID_SE,           /* tlb_flushD_SE        */
	
	/* Cache operations */
	
	arm64_icache_sync_range,        /* icache_sync_range    */
	
	arm64_dcache_wbinv_range,       /* dcache_wbinv_range   */
	arm64_dcache_inv_range,         /* dcache_inv_range     */
	arm64_dcache_wb_range,          /* dcache_wb_range      */
	
	arm64_idcache_wbinv_range,      /* idcache_wbinv_range  */
	
	/* 
	 * Note: For CPUs using the PL310 the L2 ops are filled in when the
	 * L2 cache controller is actually enabled.
	 */
	(void *)arm64_nullop,         /* l2cache_wbinv_range  */
	(void *)arm64_nullop,         /* l2cache_inv_range    */
	(void *)arm64_nullop,         /* l2cache_wb_range     */
	(void *)arm64_nullop,         /* l2cache_drain_writebuf */
	
	/* Other functions */
	
	arm64_nullop,                 /* flush_prefetchbuf    */
	arm64_drain_writebuf,           /* drain_writebuf       */
	arm64_nullop,                 /* flush_brnchtgt_C     */
	(void *)arm64_nullop,         /* flush_brnchtgt_E     */
	
	arm64_sleep,                    /* sleep                */
	
	/* Soft functions */
	
	arm64_null_fixup,             /* dataabt_fixup        */
	arm64_null_fixup,             /* prefetchabt_fixup    */
	
	arm64_context_switch,           /* context_switch       */
	
	arm64_setup                     /* cpu setup            */
};

/*
 * Global constants also used by locore.s
 */

struct cpu_functions cpufuncs;
u_int cputype;

static void get_cachetype(void);

/* Additional cache information local to this file.  Log2 of some of the
   above numbers.  */
static int	arm64_dcache_l2_nsets;
static int	arm64_dcache_l2_assoc;
static int	arm64_dcache_l2_linesize;

static void
get_cachetype()
{
	arm64_dcache_l2_nsets = 0;
	arm64_dcache_l2_assoc = 0;
	arm64_dcache_l2_linesize = 0;
}

/*
 * Cannot panic here as we may not have a console yet ...
 */

int
set_cpufuncs()
{

	/*
	 * NOTE: cpu_do_powersave defaults to off.  If we encounter a
	 * CPU type where we want to use it by default, then we set it.
	 */

	cpufuncs = arm64_cpufuncs;
	get_cachetype();

	/* Use powersave on this CPU. */
	cpu_do_powersave = 1;

	uma_set_align(arm64_dcache_align_mask);
	return (0);
}

/*
 * Fixup routines for data and prefetch aborts.
 *
 * Several compile time symbols are used
 *
 * DEBUG_FAULT_CORRECTION - Print debugging information during the
 * correction of registers after a fault.
 */

/*
 * Null abort fixup routine.
 * For use when no fixup is required.
 */
int
arm64_null_fixup(arg)
	void *arg;
{
	return(ABORT_FIXUP_OK);
}

/*
 * CPU Setup code
 */
void
arm64_setup(char *args)
{

	uint32_t linesz;

	/*
	 * Record I and D minimum line sizes - may need this sooner.
	 */
	__asm __volatile("mrs	%0, ctr_el0\n" : "=r" (linesz) : );
	arm64_pdcache_line_size = (linesz & 0xF) >> 16;
	arm64_picache_line_size = (linesz & 0xF);
}
