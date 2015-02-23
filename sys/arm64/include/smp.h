/*-
 * Copyright (c) 2014 Andre Turner <andrew@FreeBSD.org>
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

#ifndef	_MACHINE_SMP_H_
#define	_MACHINE_SMP_H_

#include <sys/_cpuset.h>
#include <machine/pcb.h>

#define IPI_AST		0
#define IPI_PREEMPT	2
#define IPI_RENDEZVOUS	3
#define IPI_STOP	4
#define IPI_STOP_HARD	4
#define IPI_HARDCLOCK	6
#define IPI_TLB		7

void	init_secondary(int cpu);
void	mpentry(void);

void	ipi_all_but_self(u_int ipi);
void	ipi_cpu(int cpu, u_int ipi);
void	ipi_selected(cpuset_t cpus, u_int ipi);

/* PIC interface */
void	pic_ipi_send(cpuset_t cpus, u_int ipi);
void	pic_ipi_clear(int ipi);
int	pic_ipi_read(int arg);

/* Platform interface */
void	platform_mp_setmaxid(void);
int	platform_mp_probe(void);
void	platform_mp_start_ap(void);
void	platform_mp_init_secondary(void);

void	platform_ipi_send(cpuset_t cpus, u_int ipi);

/* global data in mp_machdep.c */
extern struct pcb               stoppcbs[];
#endif /* !_MACHINE_SMP_H_ */
