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

#include <machine/asm.h>
#include <machine/param.h>
__FBSDID("$FreeBSD$");

/*
 * FIXME: 
 * Need big.LITTLE awareness at some point.
 * Using arm64_p[id]cache_line_size may not be the best option.
 * Need better SMP awareness.
 */
	.text
	.align	2

.Lpage_mask:
	.word	PAGE_MASK

ENTRY(arm64_nullop)
	ret
END(arm64_nullop)

ENTRY(arm64_id)
	mrs	x0, midr_el1
	ret
END(arm64_id)

ENTRY(arm64_cpuid)
	mrs	x0, mpidr_el1
	ret
END(arm64_cpuid)

ENTRY(arm64_faultstatus)
	mrs	x0, esr_el1
	ret
END(arm64_faultstatus)

ENTRY(arm64_faultaddress)
	mrs	x0, far_el1
	ret
END(arm64_faultaddress)

/*
 * Generic functions to read/modify/write the internal coprocessor registers
 */

ENTRY(arm64_control)
	mrs	x3, sctlr_el1
	bic	x2, x3, x0
	eor	x2, x2, x1

	cmp	x2, x3
	b.eq	1f
	msr	sctlr_el1, x2
1:
	mov	x0, x3
	ret
END(arm64_control)

ENTRY(arm64_setttb)
	dsb	ish
	msr	ttbr0_el1, x0
	dsb	ish
	isb
	ret
END(arm64_setttb)

ENTRY(arm64_tlb_flushID)
#ifdef SMP
	tlbi	vmalle1is
#else
	tlbi	vmalle1
#endif
	dsb	ish
	isb
	ret
END(arm64_tlb_flushID)

ENTRY(arm64_tlb_flushID_SE)
	ldr	x1, .Lpage_mask
	bic	x0, x0, x1
#ifdef SMP
	tlbi	vae1is, x0
#else
	tlbi	vae1, x0
#endif
	dsb	ish
	isb
	ret
END(arm64_tlb_flushID_SE)

ENTRY(arm64_dcache_wb_range)
	ldr	x3, =arm64_pdcache_line_size
	sub	x3, x3, #1
	and	x2, x0, x3
	add	x1, x1, x2
	bic	x0, x0, x3
.Larm64_dcwb_next:
	dc	cvac, x0
	add	x0, x0, x3
	subs	x1, x1, x3
	b.hi	.Larm64_dcwb_next
	dsb	ish
	ret
END(arm64_dcache_wb_range)

ENTRY(arm64_dcache_wbinv_range)
	ldr	x3, =arm64_pdcache_line_size
	sub	x3, x3, #1
	and	x2, x0, x3
	add	x1, x1, x2
	bic	x0, x0, x3
.Larm64_dcwbi_next:
	dc	civac, x0
	add	x0, x0, x3
	subs	x1, x1, x3
	b.hi	.Larm64_dcwbi_next
	dsb	ish
	ret
END(arm64_dcache_wbinv_range)

/*
 * Note, we must not invalidate everything.  If the range is too big we
 * must use wb-inv of the entire cache.
 */
ENTRY(arm64_dcache_inv_range)
	ldr	x3, =arm64_pdcache_line_size
	sub	x3, x3, #1
	and	x2, x0, x3
	add	x1, x1, x2
	bic	x0, x0, x3
.Larm64_dci_next:
	dc	ivac, x0
	add	x0, x0, x3
	subs	x1, x1, x3
	b.hi	.Larm64_dci_next
	dsb	ish
	ret
END(arm64_dcache_inv_range)

ENTRY(arm64_idcache_wbinv_range)
	ldr	x3, =arm64_pdcache_line_size
	sub	x3, x3, #1
	and	x2, x0, x3
	add	x1, x1, x2
	bic	x0, x0, x3
.Larm64_idcwbi_next:
	ic	ivau, x0
	dc	civac, x0
	add	x0, x0, x3
	subs	x1, x1, x3
	b.hi	.Larm64_idcwbi_next
	isb
	dsb	ish
	ret
END(arm64_idcache_wbinv_range)

ENTRY(arm64_icache_sync_range)
	ldr	x3, =arm64_pdcache_line_size
.Larm64_sync_next:
	ic	ivau,	x0
	dc	cvac,	x0
	add	x0, x0, x3
	subs	x1, x1, x3
	bhi	.Larm64_sync_next
	isb
	dsb	ish
	ret
END(arm64_icache_sync_range)

ENTRY(arm64_cpu_sleep)
	dsb	ish
	wfi
	ret
END(arm64_cpu_sleep)

ENTRY(arm64_context_switch)
	dsb	ish

	msr	ttbr0_el1, x0
	isb
#ifdef SMP
	tlbi	alle1is
#else
	tlbi	alle1
#endif
	dsb	ish
	isb
	ret
END(arm64_context_switch)

ENTRY(arm64_drain_writebuf)
	dsb	ish
	ret
END(arm64_drain_writebuf)

ENTRY(arm64_sev)
	dsb	ish
	sev
	nop
	ret
END(arm64_sev)

ENTRY(arm64_auxctrl)
	ret
END(arm64_auxctrl)

ENTRY(arm64_sleep)
	ret
END(arm64_sleep)
