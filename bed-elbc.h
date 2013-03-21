
/**
 * @file
 *
 * @ingroup BEDImplELBCFCM
 *
 * @brief BED eLBC FCM API.
 */

/*
 * Copyright (c) 2013 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#ifndef BED_ELBC_H
#define BED_ELBC_H

#include "bed-nand.h"
#include "bed-bits.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDImplELBCFCM BED eLBC FCM
 *
 * @ingroup BEDImplNAND
  *
 * @{
 */

typedef struct {
	uint32_t br;
	uint32_t or;
} bed_elbc_bank;

typedef struct {
        bed_elbc_bank banks[8];
#define ELBC_BR_BA(val) BED_BFLD32(val, 0, 16)
#define ELBC_BR_BA_GET(reg) BED_BFLD32GET(reg, 0, 16)
#define ELBC_BR_BA_SET(reg, val) BED_BFLD32SET(reg, val, 0, 16)
#define ELBC_BR_BA_MASK BED_BMSK32(0, 16)
#define ELBC_BR_PS(val) BED_BFLD32(val, 19, 20)
#define ELBC_BR_PS_GET(reg) BED_BFLD32GET(reg, 19, 20)
#define ELBC_BR_PS_SET(reg, val) BED_BFLD32SET(reg, val, 19, 20)
#define ELBC_BR_DECC(val) BED_BFLD32(val, 21, 22)
#define ELBC_BR_DECC_GET(reg) BED_BFLD32GET(reg, 21, 22)
#define ELBC_BR_DECC_SET(reg, val) BED_BFLD32SET(reg, val, 21, 22)
#define ELBC_BR_WP BED_BBIT32(23)
#define ELBC_BR_MSEL(val) BED_BFLD32(val, 24, 26)
#define ELBC_BR_MSEL_GET(reg) BED_BFLD32GET(reg, 24, 26)
#define ELBC_BR_MSEL_SET(reg, val) BED_BFLD32SET(reg, val, 24, 26)
#define ELBC_BR_V BED_BBIT32(31)
#define ELBC_OR_AM(val) BED_BFLD32(val, 0, 16)
#define ELBC_OR_AM_GET(reg) BED_BFLD32GET(reg, 0, 16)
#define ELBC_OR_AM_SET(reg, val) BED_BFLD32SET(reg, val, 0, 16)
#define ELBC_OR_BCTLD BED_BBIT32(19)
#define ELBC_OR_TRLX BED_BBIT32(29)
#define ELBC_OR_EHTR BED_BBIT32(30)
#define ELBC_OR_EAD BED_BBIT32(31)
#define ELBC_OR_GPCM_CSNT BED_BBIT32(20)
#define ELBC_OR_GPCM_ACS(val) BED_BFLD32(val, 21, 22)
#define ELBC_OR_GPCM_ACS_GET(reg) BED_BFLD32GET(reg, 21, 22)
#define ELBC_OR_GPCM_ACS_SET(reg, val) BED_BFLD32SET(reg, val, 21, 22)
#define ELBC_OR_GPCM_XACS BED_BBIT32(23)
#define ELBC_OR_GPCM_SCY(val) BED_BFLD32(val, 24, 27)
#define ELBC_OR_GPCM_SCY_GET(reg) BED_BFLD32GET(reg, 24, 27)
#define ELBC_OR_GPCM_SCY_SET(reg, val) BED_BFLD32SET(reg, val, 24, 27)
#define ELBC_OR_GPCM_SETA BED_BBIT32(28)
#define ELBC_OR_FCM_PGS BED_BBIT32(21)
#define ELBC_OR_FCM_CSCT BED_BBIT32(22)
#define ELBC_OR_FCM_CST BED_BBIT32(23)
#define ELBC_OR_FCM_CHT BED_BBIT32(24)
#define ELBC_OR_FCM_SCY(val) BED_BFLD32(val, 25, 27)
#define ELBC_OR_FCM_SCY_GET(reg) BED_BFLD32GET(reg, 25, 27)
#define ELBC_OR_FCM_SCY_SET(reg, val) BED_BFLD32SET(reg, val, 25, 27)
#define ELBC_OR_FCM_RST BED_BBIT32(28)
#define ELBC_OR_UPM_BI BED_BBIT32(23)
        uint32_t reserved_40[10];
        uint32_t mar;
        uint32_t reserved_6c;
        uint32_t mamr;
        uint32_t mbmr;
        uint32_t mcmr;
#define ELBC_MXMR_RFEN BED_BBIT32(1)
#define ELBC_MXMR_OP(val) BED_BFLD32(val, 2, 3)
#define ELBC_MXMR_OP_GET(reg) BED_BFLD32GET(reg, 2, 3)
#define ELBC_MXMR_OP_SET(reg, val) BED_BFLD32SET(reg, val, 2, 3)
#define ELBC_MXMR_UWPL BED_BBIT32(4)
#define ELBC_MXMR_AM(val) BED_BFLD32(val, 5, 7)
#define ELBC_MXMR_AM_GET(reg) BED_BFLD32GET(reg, 5, 7)
#define ELBC_MXMR_AM_SET(reg, val) BED_BFLD32SET(reg, val, 5, 7)
#define ELBC_MXMR_DS(val) BED_BFLD32(val, 8, 9)
#define ELBC_MXMR_DS_GET(reg) BED_BFLD32GET(reg, 8, 9)
#define ELBC_MXMR_DS_SET(reg, val) BED_BFLD32SET(reg, val, 8, 9)
#define ELBC_MXMR_G0CL(val) BED_BFLD32(val, 10, 12)
#define ELBC_MXMR_G0CL_GET(reg) BED_BFLD32GET(reg, 10, 12)
#define ELBC_MXMR_G0CL_SET(reg, val) BED_BFLD32SET(reg, val, 10, 12)
#define ELBC_MXMR_GPL4 BED_BBIT32(13)
#define ELBC_MXMR_RLF(val) BED_BFLD32(val, 14, 17)
#define ELBC_MXMR_RLF_GET(reg) BED_BFLD32GET(reg, 14, 17)
#define ELBC_MXMR_RLF_SET(reg, val) BED_BFLD32SET(reg, val, 14, 17)
#define ELBC_MXMR_WLF(val) BED_BFLD32(val, 18, 21)
#define ELBC_MXMR_WLF_GET(reg) BED_BFLD32GET(reg, 18, 21)
#define ELBC_MXMR_WLF_SET(reg, val) BED_BFLD32SET(reg, val, 18, 21)
#define ELBC_MXMR_TLF(val) BED_BFLD32(val, 22, 25)
#define ELBC_MXMR_TLF_GET(reg) BED_BFLD32GET(reg, 22, 25)
#define ELBC_MXMR_TLF_SET(reg, val) BED_BFLD32SET(reg, val, 22, 25)
#define ELBC_MXMR_MAD(val) BED_BFLD32(val, 26, 31)
#define ELBC_MXMR_MAD_GET(reg) BED_BFLD32GET(reg, 26, 31)
#define ELBC_MXMR_MAD_SET(reg, val) BED_BFLD32SET(reg, val, 26, 31)
        uint32_t reserved_7c[2];
        uint32_t mrtpr;
#define ELBC_MRTPR_PTP(val) BED_BFLD32(val, 0, 7)
#define ELBC_MRTPR_PTP_GET(reg) BED_BFLD32GET(reg, 0, 7)
#define ELBC_MRTPR_PTP_SET(reg, val) BED_BFLD32SET(reg, val, 0, 7)
        uint32_t mdr;
#define ELBC_MDR_AS3(val) BED_BFLD32(val, 0, 7)
#define ELBC_MDR_AS3_GET(reg) BED_BFLD32GET(reg, 0, 7)
#define ELBC_MDR_AS3_SET(reg, val) BED_BFLD32SET(reg, val, 0, 7)
#define ELBC_MDR_AS2(val) BED_BFLD32(val, 8, 15)
#define ELBC_MDR_AS2_GET(reg) BED_BFLD32GET(reg, 8, 15)
#define ELBC_MDR_AS2_SET(reg, val) BED_BFLD32SET(reg, val, 8, 15)
#define ELBC_MDR_AS1(val) BED_BFLD32(val, 16, 23)
#define ELBC_MDR_AS1_GET(reg) BED_BFLD32GET(reg, 16, 23)
#define ELBC_MDR_AS1_SET(reg, val) BED_BFLD32SET(reg, val, 16, 23)
#define ELBC_MDR_AS0(val) BED_BFLD32(val, 24, 31)
#define ELBC_MDR_AS0_GET(reg) BED_BFLD32GET(reg, 24, 31)
#define ELBC_MDR_AS0_SET(reg, val) BED_BFLD32SET(reg, val, 24, 31)
        uint32_t reserved_8c;
        uint32_t lsor;
#define ELBC_LSOR_BANK(val) BED_BFLD32(val, 29, 31)
#define ELBC_LSOR_BANK_GET(reg) BED_BFLD32GET(reg, 29, 31)
#define ELBC_LSOR_BANK_SET(reg, val) BED_BFLD32SET(reg, val, 29, 31)
        uint32_t reserved_94[3];
        uint32_t lurt;
#define ELBC_LURT_LURT(val) BED_BFLD32(val, 0, 7)
#define ELBC_LURT_LURT_GET(reg) BED_BFLD32GET(reg, 0, 7)
#define ELBC_LURT_LURT_SET(reg, val) BED_BFLD32SET(reg, val, 0, 7)
        uint32_t reserved_a4[3];
        uint32_t ltesr;
        uint32_t ltedr;
        uint32_t lteir;
#define ELBC_LTE_BM BED_BBIT32(0)
#define ELBC_LTE_FCT BED_BBIT32(1)
#define ELBC_LTE_PAR BED_BBIT32(2)
#define ELBC_LTE_WP BED_BBIT32(5)
#define ELBC_LTE_CS BED_BBIT32(12)
#define ELBC_LTE_CC BED_BBIT32(31)
        uint32_t lteatr;
#define ELBC_LTEATR_RWB BED_BBIT32(2)
#define ELBC_LTEATR_SRCID(val) BED_BFLD32(val, 11, 15)
#define ELBC_LTEATR_SRCID_GET(reg) BED_BFLD32GET(reg, 11, 15)
#define ELBC_LTEATR_SRCID_SET(reg, val) BED_BFLD32SET(reg, val, 11, 15)
#define ELBC_LTEATR_PB(val) BED_BFLD32(val, 16, 19)
#define ELBC_LTEATR_PB_GET(reg) BED_BFLD32GET(reg, 16, 19)
#define ELBC_LTEATR_PB_SET(reg, val) BED_BFLD32SET(reg, val, 16, 19)
#define ELBC_LTEATR_BNK(val) BED_BFLD32(val, 20, 23)
#define ELBC_LTEATR_BNK_GET(reg) BED_BFLD32GET(reg, 20, 23)
#define ELBC_LTEATR_BNK_SET(reg, val) BED_BFLD32SET(reg, val, 20, 23)
#define ELBC_LTEATR_V BED_BBIT32(31)
        uint32_t ltear;
        uint32_t reserved_c4[3];
        uint32_t lbcr;
#define ELBC_LBCR_LDIS BED_BBIT32(0)
#define ELBC_LBCR_BCTLC(val) BED_BFLD32(val, 8, 9)
#define ELBC_LBCR_BCTLC_GET(reg) BED_BFLD32GET(reg, 8, 9)
#define ELBC_LBCR_BCTLC_SET(reg, val) BED_BFLD32SET(reg, val, 8, 9)
#define ELBC_LBCR_AHD BED_BBIT32(10)
#define ELBC_LBCR_BMT(val) BED_BFLD32(val, 16, 23)
#define ELBC_LBCR_BMT_GET(reg) BED_BFLD32GET(reg, 16, 23)
#define ELBC_LBCR_BMT_SET(reg, val) BED_BFLD32SET(reg, val, 16, 23)
#define ELBC_LBCR_BMTPS(val) BED_BFLD32(val, 28, 31)
#define ELBC_LBCR_BMTPS_GET(reg) BED_BFLD32GET(reg, 28, 31)
#define ELBC_LBCR_BMTPS_SET(reg, val) BED_BFLD32SET(reg, val, 28, 31)
        uint32_t lcrr;
#define ELBC_LCRR_EADC(val) BED_BFLD32(val, 14, 15)
#define ELBC_LCRR_EADC_GET(reg) BED_BFLD32GET(reg, 14, 15)
#define ELBC_LCRR_EADC_SET(reg, val) BED_BFLD32SET(reg, val, 14, 15)
#define ELBC_LCRR_CLKDIV(val) BED_BFLD32(val, 27, 31)
#define ELBC_LCRR_CLKDIV_GET(reg) BED_BFLD32GET(reg, 27, 31)
#define ELBC_LCRR_CLKDIV_SET(reg, val) BED_BFLD32SET(reg, val, 27, 31)
        uint32_t reserved_d8[2];
        uint32_t fmr;
#define ELBC_FMR_CWTO(val) BED_BFLD32(val, 16, 19)
#define ELBC_FMR_CWTO_GET(reg) BED_BFLD32GET(reg, 16, 19)
#define ELBC_FMR_CWTO_SET(reg, val) BED_BFLD32SET(reg, val, 16, 19)
#define ELBC_FMR_BOOT BED_BBIT32(20)
#define ELBC_FMR_ECCM BED_BBIT32(23)
#define ELBC_FMR_AL(val) BED_BFLD32(val, 26, 27)
#define ELBC_FMR_AL_GET(reg) BED_BFLD32GET(reg, 26, 27)
#define ELBC_FMR_AL_SET(reg, val) BED_BFLD32SET(reg, val, 26, 27)
#define ELBC_FMR_OP(val) BED_BFLD32(val, 30, 31)
#define ELBC_FMR_OP_GET(reg) BED_BFLD32GET(reg, 30, 31)
#define ELBC_FMR_OP_SET(reg, val) BED_BFLD32SET(reg, val, 30, 31)
        uint32_t fir;
#define ELBC_FIR_OP_SHIFT_INC (-4)
#define ELBC_FIR_OP0_SHIFT 28
#define ELBC_FIR_OP1_SHIFT 24
#define ELBC_FIR_OP2_SHIFT 20
#define ELBC_FIR_OP3_SHIFT 16
#define ELBC_FIR_OP4_SHIFT 12
#define ELBC_FIR_OP5_SHIFT 8
#define ELBC_FIR_OP6_SHIFT 4
#define ELBC_FIR_OP7_SHIFT 0
#define ELBC_FIR_OP_NOP 0x0U
#define ELBC_FIR_OP_CA 0x1U
#define ELBC_FIR_OP_PA 0x2U
#define ELBC_FIR_OP_UA 0x3U
#define ELBC_FIR_OP_CM0 0x4U
#define ELBC_FIR_OP_CM1 0x5U
#define ELBC_FIR_OP_CM2 0x6U
#define ELBC_FIR_OP_CM3 0x7U
#define ELBC_FIR_OP_WB 0x8U
#define ELBC_FIR_OP_WS 0x9U
#define ELBC_FIR_OP_RB 0xaU
#define ELBC_FIR_OP_RS 0xbU
#define ELBC_FIR_OP_CW0 0xcU
#define ELBC_FIR_OP_CW1 0xdU
#define ELBC_FIR_OP_RBW 0xeU
#define ELBC_FIR_OP_RSW 0xfU
        uint32_t fcr;
#define ELBC_FCR_CMD_SHIFT_INC (-8)
#define ELBC_FCR_CMD0_SHIFT 24
#define ELBC_FCR_CMD1_SHIFT 16
#define ELBC_FCR_CMD2_SHIFT 8
#define ELBC_FCR_CMD3_SHIFT 0
        uint32_t fbar;
#define ELBC_FBAR_BLK(val) BED_BFLD32(val, 8, 31)
#define ELBC_FBAR_BLK_GET(reg) BED_BFLD32GET(reg, 8, 31)
#define ELBC_FBAR_BLK_SET(reg, val) BED_BFLD32SET(reg, val, 8, 31)
        uint32_t fpar;
#define ELBC_FPAR_SP_PI(val) BED_BFLD32(val, 17, 21)
#define ELBC_FPAR_SP_PI_GET(reg) BED_BFLD32GET(reg, 17, 21)
#define ELBC_FPAR_SP_PI_SET(reg, val) BED_BFLD32SET(reg, val, 17, 21)
#define ELBC_FPAR_SP_MS BED_BBIT32(22)
#define ELBC_FPAR_SP_CI(val) BED_BFLD32(val, 23, 31)
#define ELBC_FPAR_SP_CI_GET(reg) BED_BFLD32GET(reg, 23, 31)
#define ELBC_FPAR_SP_CI_SET(reg, val) BED_BFLD32SET(reg, val, 23, 31)
#define ELBC_FPAR_LP_PI(val) BED_BFLD32(val, 14, 19)
#define ELBC_FPAR_LP_PI_GET(reg) BED_BFLD32GET(reg, 14, 19)
#define ELBC_FPAR_LP_PI_SET(reg, val) BED_BFLD32SET(reg, val, 14, 19)
#define ELBC_FPAR_LP_MS BED_BBIT32(20)
#define ELBC_FPAR_LP_CI(val) BED_BFLD32(val, 21, 31)
#define ELBC_FPAR_LP_CI_GET(reg) BED_BFLD32GET(reg, 21, 31)
#define ELBC_FPAR_LP_CI_SET(reg, val) BED_BFLD32SET(reg, val, 21, 31)
        uint32_t fbcr;
#define ELBC_FBCR_BC(val) BED_BFLD32(val, 20, 31)
#define ELBC_FBCR_BC_GET(reg) BED_BFLD32GET(reg, 20, 31)
#define ELBC_FBCR_BC_SET(reg, val) BED_BFLD32SET(reg, val, 20, 31)
} bed_elbc;

#define ELBC_LTE_NAND_STATUS (ELBC_LTE_FCT | ELBC_LTE_PAR | ELBC_LTE_CC)

#define ELBC_ECC_CORRECTABLE_BITS_PER_512_BYTES 1

#define ELBC_LARGE_PAGE_BR(base_address) \
	(((base_address) & ELBC_BR_BA_MASK) \
		| ELBC_BR_PS(0x1) \
		| ELBC_BR_MSEL(0x1) \
		| ELBC_BR_V)

#define ELBC_SLOW_TIMING_OR \
	(ELBC_OR_FCM_CSCT \
		| ELBC_OR_FCM_CST \
		| ELBC_OR_FCM_CHT \
		| ELBC_OR_FCM_SCY(0x7) \
		| ELBC_OR_FCM_RST \
		| ELBC_OR_TRLX \
		| ELBC_OR_EHTR)

typedef enum {
	ELBC_ECC_MODE_NONE = 0x0,
	ELBC_ECC_MODE_CHECK = 0x1,
	ELBC_ECC_MODE_CHECK_AND_GENERATE = 0x2
} bed_elbc_ecc_mode;

typedef struct {
	bed_device bed;
	bed_nand_context nand;
	bed_partition part;
	volatile bed_elbc *elbc;
	uint8_t *base_address;
	uint8_t *current_buffer;
	uint32_t bank;
	size_t current_buffer_offset;
	uint32_t ltesr;
	uint32_t fmr;
	bed_elbc_ecc_mode ecc_mode_auto;
	bool execute_done;
} bed_elbc_context;

typedef struct {
	const bed_nand_device_info *info;
	volatile bed_elbc *elbc;
	uint8_t *base_address;
	uint32_t bank;
	bed_obtain_method obtain;
	bed_release_method release;
	bed_select_chip_method select_chip;
} bed_elbc_config;

bed_status bed_elbc_init(
	bed_elbc_context *self,
	const bed_elbc_config *config
);

void bed_elbc_init_with_no_chip_detection(
	bed_elbc_context *self,
	const bed_elbc_config *config,
	uint32_t blocks_per_chip,
	uint32_t block_size,
	uint16_t page_size
);

/** @} */

/* Internal functions, do not use */

static inline void bed_elbc_write_workaround(volatile uint8_t *last)
{
	/* Workaround to prevent eLBC hangs during NAND write operations */
	__asm__ volatile("" : : : "memory");
	*last;
}

static inline void bed_elbc_init_module(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t br,
	uint32_t or_timing
)
{
	elbc->banks[bank].br = br;
	elbc->banks[bank].or = ELBC_OR_AM(0x1ffff) | or_timing;
}

static inline void bed_elbc_set_ecc_mode(
	volatile bed_elbc *elbc,
	uint32_t bank,
	bed_elbc_ecc_mode mode
)
{
	volatile uint32_t *br = &elbc->banks[bank].br;

	*br = ELBC_BR_DECC_SET(*br, mode);
}

static inline uint32_t bed_elbc_finalize_module(
	volatile bed_elbc *elbc,
	uint32_t bank,
	bool has_large_pages,
	bool needs_3_page_cycles,
	bed_elbc_ecc_mode mode
)
{
	volatile uint32_t *or = &elbc->banks[bank].or;
	uint32_t fmr = ELBC_FMR_CWTO(0xf);

	if (has_large_pages) {
		*or |= ELBC_OR_FCM_PGS;
		fmr |= ELBC_FMR_ECCM;
	} else {
		*or &= ~ELBC_OR_FCM_PGS;
	}

	if (needs_3_page_cycles) {
		fmr |= ELBC_FMR_AL(0x1);
	}

	bed_elbc_set_ecc_mode(elbc, bank, mode);

	return fmr;
}

static inline uint32_t bed_elbc_wait(volatile bed_elbc *elbc, uint32_t flags)
{
	uint32_t ltesr;

	do {
		ltesr = elbc->ltesr;
	} while ((ltesr & flags) != flags);

	return ltesr;
}

static uint32_t bed_elbc_execute(volatile bed_elbc *elbc, uint32_t bank, uint32_t fmr)
{
	elbc->fmr = fmr | ELBC_FMR_OP(0x3);
	elbc->ltesr = ELBC_LTE_NAND_STATUS;
	elbc->lteatr = 0;
	elbc->lsor = bank;

	return bed_elbc_wait(elbc, ELBC_LTE_CC);
}

static inline void bed_elbc_enable_micron_ecc(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr,
	volatile uint8_t *base_address,
	bool enable
)
{
	elbc->fir = (ELBC_FIR_OP_CW0 << ELBC_FIR_OP0_SHIFT)
		| (ELBC_FIR_OP_UA << ELBC_FIR_OP1_SHIFT)
		| (ELBC_FIR_OP_WB << ELBC_FIR_OP2_SHIFT);
	elbc->fcr = (BED_NAND_CMD_SET_FEATURES & 0xffU) << ELBC_FCR_CMD0_SHIFT;
	elbc->mdr = ELBC_MDR_AS0(0x90);
	elbc->fbar = 0;
	elbc->fpar = 0;
	elbc->fbcr = 4;

	base_address[0] = enable ? 0x8 : 0x0;
	base_address[1] = 0x0;
	base_address[2] = 0x0;
	base_address[3] = 0x0;

	bed_elbc_write_workaround(&base_address[3]);

	bed_elbc_execute(elbc, bank, fmr);
}

static inline bool bed_elbc_is_micron_ecc_enabled(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr,
	volatile uint8_t *base_address
)
{
	elbc->fir = (ELBC_FIR_OP_CW0 << ELBC_FIR_OP0_SHIFT)
		| (ELBC_FIR_OP_UA << ELBC_FIR_OP1_SHIFT)
		| (ELBC_FIR_OP_RBW << ELBC_FIR_OP2_SHIFT);
	elbc->fcr = (BED_NAND_CMD_READ_ID & 0xffU) << ELBC_FCR_CMD0_SHIFT;
	elbc->mdr = ELBC_MDR_AS0(0x0);
	elbc->fbar = 0;
	elbc->fpar = 0;
	elbc->fbcr = 5;

	bed_elbc_execute(elbc, bank, fmr);

	return (base_address[4] & 0x80) != 0;
}

static inline uint8_t bed_elbc_read_status(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr,
	volatile uint8_t *base_address
)
{
	elbc->fir = (ELBC_FIR_OP_CW0 << ELBC_FIR_OP0_SHIFT)
		| (ELBC_FIR_OP_RBW << ELBC_FIR_OP1_SHIFT);
	elbc->fcr = (BED_NAND_CMD_READ_STATUS & 0xffU) << ELBC_FCR_CMD0_SHIFT;
	elbc->fbar = 0;
	elbc->fpar = 0;
	elbc->fbcr = 1;

	bed_elbc_execute(elbc, bank, fmr);

	return base_address[0];
}

static inline void bed_elbc_read_mode(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr
)
{
	elbc->fir = ELBC_FIR_OP_CW0 << ELBC_FIR_OP0_SHIFT;
	elbc->fcr = (BED_NAND_CMD_READ_MODE & 0xffU) << ELBC_FCR_CMD0_SHIFT;
	elbc->fbar = 0;
	elbc->fpar = 0;
	elbc->fbcr = 0;

	bed_elbc_execute(elbc, bank, fmr);
}

static inline void bed_elbc_reset(
	volatile bed_elbc *elbc,
	uint32_t bank,
	uint32_t fmr
)
{
	elbc->fir = (ELBC_FIR_OP_CM0 << ELBC_FIR_OP0_SHIFT)
		| (ELBC_FIR_OP_RBW << ELBC_FIR_OP1_SHIFT);
	elbc->fcr = (BED_NAND_CMD_RESET & 0xffU) << ELBC_FCR_CMD0_SHIFT;
	elbc->fbar = 0;
	elbc->fpar = 0;
	elbc->fbcr = 1;

	bed_elbc_execute(elbc, bank, fmr);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_ELBC_H */
