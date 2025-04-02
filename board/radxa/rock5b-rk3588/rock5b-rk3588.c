// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023-2024 Collabora Ltd.
 */

#include <fdtdec.h>
#include <fdt_support.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <env.h>
#include <asm/io.h>

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (IS_ENABLED(CONFIG_TYPEC_FUSB302))
		fdt_status_okay_by_compatible(blob, "fcs,fusb302");
	return 0;
}
#endif

enum {
	ROCKCHIP_DDR4 = 0,
	ROCKCHIP_DDR2 = 2,
	ROCKCHIP_DDR3 = 3,
	ROCKCHIP_LPDDR2 = 5,
	ROCKCHIP_LPDDR3 = 6,
	ROCKCHIP_LPDDR4 = 7,
	ROCKCHIP_LPDDR4X = 8,
	ROCKCHIP_LPDDR5 = 9,
	ROCKCHIP_DDR5 = 10,
	ROCKCHIP_UNUSED = 0xFF
};

#define RK3588_PMUGRF_BASE_ADDR			0xfd58a000
#define RK3588_PMUGRF_OS_REG2			0x208
#define RK3588_PMUGRF_OS_REG2_DRAMTYPE_INFO	GENMASK(15, 13)
#define RK3588_PMUGRF_OS_REG3			0x20c
#define RK3588_PMUGRF_OS_REG3_SYSREG_VERSION	GENMASK(31, 28)
#define RK3588_PMUGRF_OS_REG3_DRAMTYPE_INFO_V3	GENMASK(13, 12)

static int rk3588_get_memory_type(void)
{
	u32 reg2 = readl(RK3588_PMUGRF_BASE_ADDR + RK3588_PMUGRF_OS_REG2);
	u32 reg3 = readl(RK3588_PMUGRF_BASE_ADDR + RK3588_PMUGRF_OS_REG3);
	u32 ddr_type;

	/* lower 3 bits of the DDR type */
	ddr_type = FIELD_GET(RK3588_PMUGRF_OS_REG2_DRAMTYPE_INFO, reg2);

	/*
	 * For version three and higher the upper two bits of the DDR type are
	 * in RK3588_PMUGRF_OS_REG3
	 */
	if (FIELD_GET(RK3588_PMUGRF_OS_REG3_SYSREG_VERSION, reg3) >= 0x3)
		ddr_type |= FIELD_GET(RK3588_PMUGRF_OS_REG3_DRAMTYPE_INFO_V3, reg3) << 3;

	return ddr_type;
}

static const char* rock5_get_model_devicetree(void)
{
	switch (rk3588_get_memory_type()) {
		case ROCKCHIP_LPDDR5:
			return "rockchip/rk3588-rock-5b-plus.dtb";
		case ROCKCHIP_LPDDR4:
		default:
			return "rockchip/rk3588-rock-5b.dtb";
	}
}

int board_fit_config_name_match(const char *name)
{
	return strcmp(name, rock5_get_model_devicetree());
}

int rk_board_late_init(void)
{
	env_set("fdtfile", rock5_get_model_devicetree());
	return 0;
}
