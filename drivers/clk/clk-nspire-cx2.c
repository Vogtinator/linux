// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: 2013 Daniel Tang <tangrs@tangrs.id.au>
// SPDX-FileCopyrightText: 2021 Fabian Vogt <fabian@ritter-vogt.de>

#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>

#define MHZ (1000 * 1000)

static void __init nspire_cx2_clk_setup(struct device_node *node)
{
	void __iomem *io;
	struct clk_hw *hw;
	const char *clk_name = node->name;
	u32 reg020, reg030, reg810;
	u32 baseclk, cpudiv, cpuclk;

	io = of_iomap(node, 0);
	if (!io)
		return;

	reg020 = readl(io + 0x020);
	reg030 = readl(io + 0x030);
	reg810 = readl(io + 0x810);

	iounmap(io);

	if (!(reg810 & BIT(4)))
		cpuclk = 48 * MHZ;
	else if (reg030 & BIT(4))
		cpuclk = 24 * MHZ;
	else {
		baseclk = 24 * MHZ * (reg030 >> 24) / ((reg030 >> 16) & 0x1F);
		cpudiv = 1 + ((reg020 >> 20) & 0xF);
		cpuclk = baseclk / cpudiv;
	}

	of_property_read_string(node, "clock-output-names", &clk_name);

	hw = clk_hw_register_fixed_rate(NULL, clk_name, NULL, 0,
					cpuclk);
	if (IS_ERR(hw))
		return;

	of_clk_add_hw_provider(node, of_clk_hw_simple_get, hw);
	pr_info("TI-NSPIRE CX II Clocks: %u MHz CPU\n", cpuclk / MHZ);
}

CLK_OF_DECLARE(nspire_cx2_clk, "ti,nspire-cx2-clock", nspire_cx2_clk_setup);
