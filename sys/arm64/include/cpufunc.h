/*-
 * Copyright (c) 2014 Andrew Turner
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

#ifndef _MACHINE_CPUFUNC_H_
#define	_MACHINE_CPUFUNC_H_

#ifdef _KERNEL

#include <machine/armreg.h>

static __inline void
breakpoint(void)
{

	__asm("brk #0");
}

static __inline register_t
intr_disable(void)
{
	/* DAIF is a 32-bit register */
	uint32_t ret;

	__asm __volatile(
	    "mrs %x0, daif   \n"
	    "msr daifset, #2 \n"
	    : "=&r" (ret));

	return (ret);
}

static __inline void
intr_restore(register_t s)
{

	WRITE_SPECIALREG(daif, s);
}

static __inline void
intr_enable(void)
{

	__asm __volatile("msr daifclr, #2");
}

static __inline register_t
get_midr(void)
{
	uint64_t midr;

	midr = READ_SPECIALREG(midr_el1);

	return (midr);
}

static __inline register_t
get_mpidr(void)
{
	uint64_t mpidr;

	mpidr = READ_SPECIALREG(mpidr_el1);

	return (mpidr);
}

struct cpu_functions {

	/* CPU functions */

	u_int	(*cf_id)		(void);
	void	(*cf_cpwait)		(void);

	/* MMU functions */

	u_int	(*cf_control)		(u_int bic, u_int eor);
	void	(*cf_setttb)		(u_int ttb);
	u_int	(*cf_faultstatus)	(void);
	u_int	(*cf_faultaddress)	(void);

	/* TLB functions */

	void	(*cf_tlb_flushID)	(void);
	void	(*cf_tlb_flushID_SE)	(u_int va);
	void	(*cf_tlb_flushI)	(void);
	void	(*cf_tlb_flushI_SE)	(u_int va);
	void	(*cf_tlb_flushD)	(void);
	void	(*cf_tlb_flushD_SE)	(u_int va);

	/*
	 * Cache operations:
	 *
	 * We define the following primitives:
	 *
	 *	icache_sync_all		Synchronize I-cache
	 *	icache_sync_range	Synchronize I-cache range
	 *
	 *      FIXME: Not implementing this for the moment. Do we need to ?
	 *	dcache_wbinv_all	Write-back and Invalidate D-cache
	 *	dcache_wbinv_range	Write-back and Invalidate D-cache range
	 *	dcache_inv_range	Invalidate D-cache range
	 *	dcache_wb_range		Write-back D-cache range
	 *
	 *      FIXME: Not implementing this for the moment. Do we need to ?
	 *	idcache_wbinv_all	Write-back and Invalidate D-cache,
	 *				Invalidate I-cache
	 *	idcache_wbinv_range	Write-back and Invalidate D-cache,
	 *				Invalidate I-cache range
	 *
	 * Note that the ARM term for "write-back" is "clean".  We use
	 * the term "write-back" since it's a more common way to describe
	 * the operation.
	 *
	 * There are some rules that must be followed:
	 *
	 *	ID-cache Invalidate All:
	 *		Unlike other functions, this one must never write back.
	 *		It is used to intialize the MMU when it is in an unknown
	 *		state (such as when it may have lines tagged as valid
	 *		that belong to a previous set of mappings).
	 * 
	 *	I-cache Synch (all or range):
	 *		The goal is to synchronize the instruction stream,
	 *		so you may beed to write-back dirty D-cache blocks
	 *		first.  If a range is requested, and you can't
	 *		synchronize just a range, you have to hit the whole
	 *		thing.
	 *
	 *	D-cache Write-Back and Invalidate range:
	 *		If you can't WB-Inv a range, you must WB-Inv the
	 *		entire D-cache.
	 *
	 *	D-cache Invalidate:
	 *		If you can't Inv the D-cache, you must Write-Back
	 *		and Invalidate.  Code that uses this operation
	 *		MUST NOT assume that the D-cache will not be written
	 *		back to memory.
	 *
	 *	D-cache Write-Back:
	 *		If you can't Write-back without doing an Inv,
	 *		that's fine.  Then treat this as a WB-Inv.
	 *		Skipping the invalidate is merely an optimization.
	 *
	 *	All operations:
	 *		Valid virtual addresses must be passed to each
	 *		cache operation.
	 */
	void	(*cf_icache_sync_range)	(vm_offset_t, vm_size_t);

	void	(*cf_dcache_wbinv_range) (vm_offset_t, vm_size_t);
	void	(*cf_dcache_inv_range)	(vm_offset_t, vm_size_t);
	void	(*cf_dcache_wb_range)	(vm_offset_t, vm_size_t);

	void	(*cf_idcache_wbinv_range) (vm_offset_t, vm_size_t);

	void	(*cf_l2cache_wbinv_range)	  (vm_offset_t, vm_size_t);
	void	(*cf_l2cache_inv_range)	  (vm_offset_t, vm_size_t);
	void	(*cf_l2cache_wb_range)	  (vm_offset_t, vm_size_t);
	void	(*cf_l2cache_drain_writebuf)	  (void);

	/* Other functions */

	void	(*cf_flush_prefetchbuf)	(void);
	void	(*cf_drain_writebuf)	(void);
	void	(*cf_flush_brnchtgt_C)	(void);
	void	(*cf_flush_brnchtgt_E)	(u_int va);

	void	(*cf_sleep)		(int mode);

	/* Soft functions */

	int	(*cf_dataabt_fixup)	(void *arg);
	int	(*cf_prefetchabt_fixup)	(void *arg);

	void	(*cf_context_switch)	(void);

	void	(*cf_setup)		(char *string);
};

extern struct cpu_functions cpufuncs;
extern u_int cputype;

#define cpu_id()		cpufuncs.cf_id()
#define	cpu_cpwait()		cpufuncs.cf_cpwait()

#define cpu_control(c, e)	cpufuncs.cf_control(c, e)
#define cpu_setttb(t)		cpufuncs.cf_setttb(t)
#define cpu_faultstatus()	cpufuncs.cf_faultstatus()
#define cpu_faultaddress()	cpufuncs.cf_faultaddress()

#ifndef SMP

#define	cpu_tlb_flushID()	cpufuncs.cf_tlb_flushID()
#define	cpu_tlb_flushID_SE(e)	cpufuncs.cf_tlb_flushID_SE(e)
#define	cpu_tlb_flushI()	cpufuncs.cf_tlb_flushI()
#define	cpu_tlb_flushI_SE(e)	cpufuncs.cf_tlb_flushI_SE(e)
#define	cpu_tlb_flushD()	cpufuncs.cf_tlb_flushD()
#define	cpu_tlb_flushD_SE(e)	cpufuncs.cf_tlb_flushD_SE(e)

#else
void tlb_broadcast(int);

#if defined(CPU_CORTEXA)
#define TLB_BROADCAST	/* No need to explicitely send an IPI */
#else
#define TLB_BROADCAST	tlb_broadcast(7)
#endif

#define	cpu_tlb_flushID() do { \
	cpufuncs.cf_tlb_flushID(); \
	TLB_BROADCAST; \
} while(0)

#define	cpu_tlb_flushID_SE(e) do { \
	cpufuncs.cf_tlb_flushID_SE(e); \
	TLB_BROADCAST; \
} while(0)


#define	cpu_tlb_flushI() do { \
	cpufuncs.cf_tlb_flushI(); \
	TLB_BROADCAST; \
} while(0)


#define	cpu_tlb_flushI_SE(e) do { \
	cpufuncs.cf_tlb_flushI_SE(e); \
	TLB_BROADCAST; \
} while(0)


#define	cpu_tlb_flushD() do { \
	cpufuncs.cf_tlb_flushD(); \
	TLB_BROADCAST; \
} while(0)


#define	cpu_tlb_flushD_SE(e) do { \
	cpufuncs.cf_tlb_flushD_SE(e); \
	TLB_BROADCAST; \
} while(0)

#endif

#define	cpu_icache_sync_range(a, s) cpufuncs.cf_icache_sync_range((a), (s))

#define	cpu_dcache_wbinv_range(a, s) cpufuncs.cf_dcache_wbinv_range((a), (s))
#define	cpu_dcache_inv_range(a, s) cpufuncs.cf_dcache_inv_range((a), (s))
#define	cpu_dcache_wb_range(a, s) cpufuncs.cf_dcache_wb_range((a), (s))

#define	cpu_idcache_wbinv_range(a, s) cpufuncs.cf_idcache_wbinv_range((a), (s))
#define cpu_l2cache_wb_range(a, s) cpufuncs.cf_l2cache_wb_range((a), (s))
#define cpu_l2cache_inv_range(a, s) cpufuncs.cf_l2cache_inv_range((a), (s))
#define cpu_l2cache_wbinv_range(a, s) cpufuncs.cf_l2cache_wbinv_range((a), (s))
#define cpu_l2cache_drain_writebuf() cpufuncs.cf_l2cache_drain_writebuf()

#define	cpu_flush_prefetchbuf()	cpufuncs.cf_flush_prefetchbuf()
#define	cpu_drain_writebuf()	cpufuncs.cf_drain_writebuf()
#define	cpu_flush_brnchtgt_C()	cpufuncs.cf_flush_brnchtgt_C()
#define	cpu_flush_brnchtgt_E(e)	cpufuncs.cf_flush_brnchtgt_E(e)

#define cpu_sleep(m)		cpufuncs.cf_sleep(m)

#define cpu_dataabt_fixup(a)		cpufuncs.cf_dataabt_fixup(a)
#define cpu_prefetchabt_fixup(a)	cpufuncs.cf_prefetchabt_fixup(a)
#define ABORT_FIXUP_OK		0	/* fixup succeeded */
#define ABORT_FIXUP_FAILED	1	/* fixup failed */
#define ABORT_FIXUP_RETURN	2	/* abort handler should return */

#define cpu_setup(a)			cpufuncs.cf_setup(a)

int	set_cpufuncs		(void);
#define ARCHITECTURE_NOT_PRESENT	1	/* known but not configured */
#define ARCHITECTURE_NOT_SUPPORTED	2	/* not known */

void	arm64_nullop		(void);
int	arm64_null_fixup	(void *);
int	early_abort_fixup	(void *);
int	late_abort_fixup	(void *);
u_int	arm64_id		(void);
u_int	arm64_cpuid		(void);
u_int	arm64_control		(u_int clear, u_int bic);
u_int	arm64_faultstatus	(void);
u_int	arm64_faultaddress	(void);
u_int	cpu_pfr			(int);

#define setttb		cpu_setttb
#define drain_writebuf	cpu_drain_writebuf

void	arm64_setttb			(u_int);
void	arm64_tlb_flushID		(void);
void	arm64_tlb_flushID_SE		(u_int);
void	arm64_icache_sync_range		(vm_offset_t, vm_size_t);
void	arm64_idcache_wbinv_range	(vm_offset_t, vm_size_t);
void	arm64_dcache_wbinv_range	(vm_offset_t, vm_size_t);
void	arm64_dcache_inv_range		(vm_offset_t, vm_size_t);
void	arm64_dcache_wb_range		(vm_offset_t, vm_size_t);
void	arm64_cpu_sleep			(int);
void	arm64_setup			(char *string);
void	arm64_context_switch		(void);
void	arm64_drain_writebuf		(void);
void	arm64_sev			(void);
void	arm64_sleep			(int unused);
u_int	arm64_auxctrl			(u_int, u_int);

/*
 * Cache info variables.
 */

/* PRIMARY CACHE VARIABLES */
extern int	arm64_picache_size;
extern int	arm64_picache_line_size;
extern int	arm64_picache_ways;

extern int	arm64_pdcache_size;	/* and unified */
extern int	arm64_pdcache_line_size;
extern int	arm64_pdcache_ways;

extern int	arm64_pcache_type;
extern int	arm64_pcache_unified;

extern int	arm64_dcache_align;
extern int	arm64_dcache_align_mask;

extern u_int	arm64_cache_level;
extern u_int	arm64_cache_loc;
extern u_int	arm64_cache_type[14];

#endif	/* _KERNEL */
#endif	/* _MACHINE_CPUFUNC_H_ */
