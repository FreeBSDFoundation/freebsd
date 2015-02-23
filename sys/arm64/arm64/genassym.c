/*-
 * Copyright (c) 2004 Olivier Houchard
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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");
#include <sys/param.h>
#include <sys/assym.h>
#include <sys/pcpu.h>
#include <sys/proc.h>

#include <machine/frame.h>
#include <machine/pcb.h>
#include <machine/vmparam.h>

ASSYM(KERNBASE, KERNBASE);
ASSYM(TDF_ASTPENDING, TDF_ASTPENDING);
ASSYM(TDF_NEEDRESCHED, TDF_NEEDRESCHED);

ASSYM(PCPU_SIZE, sizeof(struct pcpu));
ASSYM(PC_CURPCB, offsetof(struct pcpu, pc_curpcb));
ASSYM(PC_CURTHREAD, offsetof(struct pcpu, pc_curthread));

ASSYM(PCB_REGS, offsetof(struct pcb, pcb_x));
ASSYM(PCB_SP, offsetof(struct pcb, pcb_sp));
ASSYM(PCB_L1ADDR, offsetof(struct pcb, pcb_l1addr));
ASSYM(PCB_ONFAULT, offsetof(struct pcb, pcb_onfault));

ASSYM(CF_SETTTB, offsetof(struct cpu_functions, cf_setttb));
ASSYM(CF_CONTROL, offsetof(struct cpu_functions, cf_control));
ASSYM(CF_CONTEXT_SWITCH, offsetof(struct cpu_functions, cf_context_switch));
ASSYM(CF_DCACHE_WB_RANGE, offsetof(struct cpu_functions, cf_dcache_wb_range));
ASSYM(CF_L2CACHE_WB_RANGE, offsetof(struct cpu_functions, cf_l2cache_wb_range));
ASSYM(CF_TLB_FLUSHID_SE, offsetof(struct cpu_functions, cf_tlb_flushID_SE));

ASSYM(SF_UC, offsetof(struct sigframe, sf_uc));

ASSYM(TD_PCB, offsetof(struct thread, td_pcb));
ASSYM(TD_FLAGS, offsetof(struct thread, td_flags));
ASSYM(TD_LOCK, offsetof(struct thread, td_lock));

ASSYM(PC_CPU, offsetof(struct pcpu, pc_cpu));
ASSYM(PC_CURPMAP, offsetof(struct pcpu, pc_curpmap));

ASSYM(TF_X, offsetof(struct trapframe, tf_x));
