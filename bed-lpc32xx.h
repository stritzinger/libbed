/**
 * @file
 *
 * @ingroup BEDImplLPC32XX
 *
 * @brief BED LPC32XX MLC and SLC API.
 */

/*
 * Copyright (c) 2012-2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#ifndef BED_LPC32XX_H
#define BED_LPC32XX_H

#include "bed-nand.h"
#include "bed-bits.h"

#include <bsp/lpc-dma.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDImplLPC32XX BED LPC32XX MLC and SLC
 *
 * @ingroup BEDImplNAND
  *
 * @{
 */

/**
 * @name SLC NAND Flash Timing Arcs Register (SLC_TAC)
 *
 * @{
 */

#define SLC_TAC_W_RDY(val) BED_FLD32(val, 28, 31)
#define SLC_TAC_W_WIDTH(val) BED_FLD32(val, 24, 27)
#define SLC_TAC_W_HOLD(val) BED_FLD32(val, 20, 23)
#define SLC_TAC_W_SETUP(val) BED_FLD32(val, 16, 19)
#define SLC_TAC_R_RDY(val) BED_FLD32(val, 12, 15)
#define SLC_TAC_R_WIDTH(val) BED_FLD32(val, 8, 11)
#define SLC_TAC_R_HOLD(val) BED_FLD32(val, 4, 7)
#define SLC_TAC_R_SETUP(val) BED_FLD32(val, 0, 3)

/** @} */

#define BED_LPC32XX_SLC_CHUNK_DATA_SIZE 256U

#define BED_LPC32XX_SLC_CHUNK_COUNT_MAX 16

typedef struct {
	uint32_t data;
	uint32_t addr;
	uint32_t cmd;
	uint32_t stop;
	uint32_t ctrl;
	uint32_t cfg;
	uint32_t stat;
	uint32_t int_stat;
	uint32_t ien;
	uint32_t isr;
	uint32_t icr;
	uint32_t tac;
	uint32_t tc;
	uint32_t ecc;
	uint32_t dma_data;
} bed_lpc32xx_slc;

typedef struct {
	bed_partition part;
	bed_device bed;
	bed_nand_context nand;
	volatile bed_lpc32xx_slc *slc;
	size_t chunk_count;
	volatile lpc_dma *dma;
	size_t dma_channel;
	uint32_t ecc_buffer [BED_LPC32XX_SLC_CHUNK_COUNT_MAX];
	uint8_t chunk_buffer_0 [BED_LPC32XX_SLC_CHUNK_DATA_SIZE] __attribute__((aligned(32)));
	uint8_t chunk_buffer_1 [BED_LPC32XX_SLC_CHUNK_DATA_SIZE] __attribute__((aligned(32)));
	bed_area mtd_ecc_area_in_pages;
	void *context;
} bed_lpc32xx_slc_context;

typedef struct {
	const bed_nand_device_info *info;
	volatile bed_lpc32xx_slc *slc;
	uint32_t tac;
	uint16_t chip_count;
	bed_obtain_method obtain;
	bed_release_method release;
	bed_select_chip_method select_chip;
	volatile lpc_dma *dma;
	size_t dma_channel;
	void *context;
} bed_lpc32xx_slc_config;

bed_status bed_lpc32xx_slc_init(
	bed_lpc32xx_slc_context *self,
	const bed_lpc32xx_slc_config *config
);

/**
 * @brief Specifies the area in which the MTD ECC layout will be used.
 *
 * The SLC uses a three byte ECC per 256 data bytes (Hamming code).  The BED
 * library stores these three bytes as [A, B, C].  The MTD (used by U-Boot)
 * stores them as [C, B, A].
 *
 * @param[in] part The partition defining device and area.
 *
 * @retval BED_SUCCESS Successful operation.
 * @retval BED_ERROR_OP_NOT_SUPPORTED Invalid device.
 */
bed_status bed_lpc32xx_slc_set_mtd_ecc_area(
	const bed_partition *part
);

/**
 * @name MLC NAND Timing Register (MLC_TIME_REG)
 *
 * @{
 */

#define MLC_TIME_WR_LOW(val) BED_FLD32(val, 0, 3)
#define MLC_TIME_WR_HIGH(val) BED_FLD32(val, 4, 7)
#define MLC_TIME_RD_LOW(val) BED_FLD32(val, 8, 11)
#define MLC_TIME_RD_HIGH(val) BED_FLD32(val, 12, 15)
#define MLC_TIME_NAND_TA(val) BED_FLD32(val, 16, 18)
#define MLC_TIME_BUSY_DELAY(val) BED_FLD32(val, 19, 23)
#define MLC_TIME_TCEA_DELAY(val) BED_FLD32(val, 24, 25)

/** @} */

typedef union {
	uint32_t w32;
	uint16_t w16;
	uint8_t w8;
} bed_lpc32xx_mlc_port;

typedef struct {
	bed_lpc32xx_mlc_port buff;
	uint32_t reserved_0 [8191];
	bed_lpc32xx_mlc_port data;
	uint32_t reserved_1 [8191];
	uint32_t cmd;
	uint32_t addr;
	uint32_t ecc_enc;
	uint32_t ecc_dec;
	uint32_t ecc_auto_enc;
	uint32_t ecc_auto_dec;
	uint32_t rpr;
	uint32_t wpr;
	uint32_t rubp;
	uint32_t robp;
	uint32_t sw_wp_add_low;
	uint32_t sw_wp_add_hig;
	uint32_t icr;
	uint32_t time;
	uint32_t irq_mr;
	uint32_t irq_sr;
	uint32_t reserved_2;
	uint32_t lock_pr;
	uint32_t isr;
	uint32_t ceh;
} bed_lpc32xx_mlc;

typedef struct {
	bed_partition part;
	bed_device bed;
	bed_nand_context nand;
	volatile bed_lpc32xx_mlc *mlc;
	int chunk_count;
	void *context;
} bed_lpc32xx_mlc_context;

typedef struct {
	const bed_nand_device_info *info;
	volatile bed_lpc32xx_mlc *mlc;
	uint32_t time;
	uint16_t chip_count;
	bed_obtain_method obtain;
	bed_release_method release;
	bed_select_chip_method select_chip;
	void *context;
} bed_lpc32xx_mlc_config;

bed_status bed_lpc32xx_mlc_init(
	bed_lpc32xx_mlc_context *self,
	const bed_lpc32xx_mlc_config *config
);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_LPC32XX_H */
