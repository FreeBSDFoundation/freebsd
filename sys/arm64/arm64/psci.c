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
 * This implements support for ARM's Power State Co-ordination Interface
 * [PSCI]. The implementation adheres to version 0.2 of the PSCI
 * specification. PSCI standardizes operations such as system reset, CPU
 * on/off/suspend. PSCI requires a compliant firmware implementation.
 *
 * The PSCI specification used for this implementation is available at:
 *
 * <http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.den0022b/index.html>.
 *
 * Note that only v0.2 of the specification is supported at present.
 *
 * TODO:
 * 1. Add support for v0.1.
 *
 * 2. Add support for AAarch32 callers [this implementation assumes only
 *    AArch64 callers].
 * 3. Add support for remaining PSCI calls [this implementation only
 *    supports get_version, system_reset and cpu_on].
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <machine/bus.h>
#include <machine/psci.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#ifdef	DEBUG
#define	dprintf(fmt, args...) printf(fmt, ##args)
#else
#define	dprintf(fmt, args...)
#endif

struct psci_softc {
	device_t        dev;
};

static int psci_v0_1_init(device_t dev);
static int psci_v0_2_init(device_t dev);

static psci_callfn_t psci_call;
static psci_initfn_t psci_init;

int psci_present;

static struct ofw_compat_data compat_data[] = {
	{"arm,psci",            (uintptr_t)psci_v0_1_init},
	{"arm,psci-0.2",        (uintptr_t)psci_v0_2_init},
	{NULL,                  0}
};

static int psci_probe(device_t dev);
static int psci_attach(device_t dev);

static device_method_t psci_methods[] = {
	DEVMETHOD(device_probe,     psci_probe),
	DEVMETHOD(device_attach,    psci_attach),

	DEVMETHOD_END
};

static driver_t psci_driver = {
	"psci",
	psci_methods,
	sizeof(struct psci_softc),
};

static devclass_t psci_devclass;

EARLY_DRIVER_MODULE(psci, simplebus, psci_driver, psci_devclass, 0, 0,
    BUS_PASS_CPU + BUS_PASS_ORDER_MIDDLE);
EARLY_DRIVER_MODULE(psci, ofwbus, psci_driver, psci_devclass, 0, 0,
    BUS_PASS_CPU + BUS_PASS_ORDER_MIDDLE);

static int
psci_probe(device_t dev)
{
	struct ofw_compat_data *ocd;

	if (!ofw_bus_status_okay(dev))
		return (ENXIO);

	for (ocd = &compat_data[0]; ocd->ocd_str != NULL; ocd++) {
		if (ofw_bus_is_compatible_strict(dev, ocd->ocd_str)) {
			psci_init = (psci_initfn_t)ocd->ocd_data;
			break;
		}
	}

	if (ocd->ocd_str == NULL)
		return (ENXIO);

	device_set_desc(dev, "ARM Power State Co-ordination Interface Driver");
	return (BUS_PROBE_SPECIFIC);
}

static int
psci_attach(device_t dev)
{
	phandle_t node;
	char method[16];

	node = ofw_bus_get_node(dev);
	if ((OF_getprop(node, "method", method, sizeof(method))) > 0) {
		if (strcmp(method, "hvc") == 0) {
			psci_call = psci_hvc_despatch;
			goto out;
		}

		if (strcmp(method, "smc") == 0) {
			psci_call = psci_smc_despatch;
			goto out;
		}

		device_printf(dev, "%s: PSCI conduit in the DT is invalid\n",
		    __FUNCTION__);
		return (ENXIO);
	} else {
		device_printf(dev, "%s: PSCI conduit not supplied in the DT\n",
		    __FUNCTION__);
		return (ENXIO);
	}

out:
	if(psci_init(dev))
		return (ENXIO);

	return (0);
}

static int
psci_get_version(void)
{

	return (psci_call(PSCI_FNID_VERSION, 0, 0, 0));
}

void
psci_system_reset(void)
{

	psci_call(PSCI_FNID_SYSTEM_RESET, 0, 0, 0);
}

int
psci_cpu_on(unsigned long cpu, unsigned long entry, unsigned long context_id)
{

	return(psci_call(PSCI_FNID_CPU_ON, cpu, entry, context_id));
}

static int
psci_v0_1_init(device_t dev)
{

	/*
	 * PSCI v0.1 is currently unsupported.
	 */
	device_printf(dev, "%s: PSCI v0.1 is currently unsupported\n",
	    __FUNCTION__);
	return (FAILURE);
}

static int
psci_v0_2_init(device_t dev)
{
	int version;

	version = psci_get_version();

	if (version == PSCI_RETVAL_NOT_SUPPORTED)
		return (FAILURE);

	if ((PSCI_VER_MAJOR(version) == 0) && (PSCI_VER_MINOR(version) == 2)) {
		psci_present = 1;
	}
	else {
		device_printf(dev, "PSCI version number mismatched with DT\n");
		return (FAILURE);
	}

	if (bootverbose)
		device_printf(dev, "PSCI version 0.2 available\n");

	return (SUCCESS);
}
