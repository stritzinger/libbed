/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
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

#include "bed-lpc32xx.h"

#include <assert.h>

#include <libcpu/arm-cp15.h>

#include <bsp.h>
#include <bsp/lpc32xx.h>

#define SLC_CHUNK_ECC_SIZE 3U
#define SLC_CHUNK_DATA_WORD_COUNT (BED_LPC32XX_SLC_CHUNK_DATA_SIZE / 4U)

/**
 * @name SLC NAND Flash Control Register (SLC_CTRL)
 *
 * @{
 */

#define SLC_CTRL_SW_RESET BED_BIT32(2)
#define SLC_CTRL_ECC_CLEAR BED_BIT32(1)
#define SLC_CTRL_DMA_START BED_BIT32(0)

/** @} */

/**
 * @name SLC NAND Flash Configuration Register (SLC_CFG)
 *
 * @{
 */

#define SLC_CFG_CE_LOW BED_BIT32(5)
#define SLC_CFG_DMA_ECC BED_BIT32(4)
#define SLC_CFG_ECC_EN BED_BIT32(3)
#define SLC_CFG_DMA_BURST BED_BIT32(2)
#define SLC_CFG_DMA_DIR BED_BIT32(1)
#define SLC_CFG_WIDTH BED_BIT32(0)

/** @} */

/**
 * @name SLC NAND Flash Status Register (SLC_STAT)
 *
 * @{
 */

#define SLC_STAT_DMA_ACTIVE BED_BIT32(2)
#define SLC_STAT_SLC_ACTIVE BED_BIT32(1)
#define SLC_STAT_READY BED_BIT32(0)

/** @} */

/**
 * @name SLC NAND Flash Interrupt Status, Enable, Set, and Clear Register
 * (SLC_INT_STAT, SLC_IEN, SLC_ISR, and SLC_ISC)
 *
 * @{
 */

#define SLC_INT_TC BED_BIT32(1)
#define SLC_INT_RDY BED_BIT32(0)

/** @} */

/**
 * @name SLC NAND Flash Error Correction Code Register (SLC_ECC)
 *
 * @{
 */

#define SLC_ECC_LP(val) BED_FLD32(val, 6, 21)
#define SLC_ECC_CP(val) BED_FLD32(val, 0, 5)

/** @} */

static void slc_control(bed_device *bed, int data, int ctrl)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_slc_context *self = nand->context;
	volatile bed_lpc32xx_slc *slc = self->slc;

	if (bed_nand_is_real_data(data)) {
		if ((ctrl & BED_NAND_CTRL_CLE) != 0) {
			slc->cmd = (uint8_t) data;
		} else {
			slc->addr = (uint8_t) data;
		}
	}
}

static bool slc_is_ready(bed_device *bed)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_slc_context *self = nand->context;
	volatile bed_lpc32xx_slc *slc = self->slc;

	return (slc->stat & SLC_STAT_READY) != 0;
}

static void slc_read_buffer(bed_device *bed, uint8_t *data, size_t n)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_slc_context *self = nand->context;
	volatile bed_lpc32xx_slc *slc = self->slc;
	size_t i;

	for (i = 0; i < n; ++i) {
		data [i] = (uint8_t) slc->data;
	}
}

static uint8_t slc_read_8(bed_device *bed)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_slc_context *self = nand->context;
	volatile bed_lpc32xx_slc *slc = self->slc;

	return (uint8_t) slc->data;
}

static uint16_t slc_read_16(bed_device *bed)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_slc_context *self = nand->context;
	volatile bed_lpc32xx_slc *slc = self->slc;
	uint32_t d_0 = slc->data;
	uint32_t d_1 = slc->data;

	return (uint16_t) ((d_1 << 8) | d_0);
}

static void slc_write_buffer(bed_device *bed, const uint8_t *data, size_t n)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_slc_context *self = nand->context;
	volatile bed_lpc32xx_slc *slc = self->slc;
	size_t i;

	for (i = 0; i < n; ++i) {
		slc->data = data [i];
	}
}

static bool slc_is_cache_aligned(uintptr_t data)
{
	return ((uintptr_t) data) % 32 == 0;
}

static void slc_cache_invalidate(const void *addr, size_t n)
{
	const uint8_t *p = ARM_CP15_CACHE_PREPARE_MVA(addr);
	size_t i;

	n += (size_t) addr - (size_t) p;

	for (i = 0; i < n; i += 32) {
		arm_cp15_data_cache_invalidate_line(p);
		p += 32;
	}
}

#ifndef BED_CONFIG_READ_ONLY
static void slc_cache_clean(const void *addr, size_t n)
{
	const uint8_t *p = ARM_CP15_CACHE_PREPARE_MVA(addr);
	size_t i;

	n += (size_t) addr - (size_t) p;

	for (i = 0; i < n; i += 32) {
		arm_cp15_data_cache_clean_line(p);
		p += 32;
	}
}
#endif /* BED_CONFIG_READ_ONLY */

static void slc_dma_wait(volatile lpc_dma *dma, uint32_t channel_bit)
{
	while ((dma->int_tc_stat & channel_bit) == 0) {
		/* Wait */
	}
}

static void slc_dma_transfer(bed_lpc32xx_slc_context *self, uintptr_t data, bool read)
{
	volatile bed_lpc32xx_slc *slc = self->slc;
	volatile lpc_dma *dma = self->dma;
	volatile lpc_dma_channel *channel = &dma->channels [self->dma_channel];
	uint32_t channel_bit = 1U << self->dma_channel;
	size_t chunk_count = self->chunk_count;
	uint32_t slc_cfg = slc->cfg;
	uint32_t chunk = data;
	size_t i;

	slc_cfg |= SLC_CFG_ECC_EN | SLC_CFG_DMA_ECC | SLC_CFG_DMA_BURST;

	if (read) {
		slc_cfg |= SLC_CFG_DMA_DIR;
	} else {
#ifndef BED_CONFIG_READ_ONLY
		slc_cfg &= ~SLC_CFG_DMA_DIR;
#endif /* BED_CONFIG_READ_ONLY */
	}

	channel->cfg = 0;
	channel->desc.lli = 0;

	slc->cfg = slc_cfg;
	slc->ctrl = SLC_CTRL_ECC_CLEAR;
	slc->tc = chunk_count * BED_LPC32XX_SLC_CHUNK_DATA_SIZE;
	slc->ctrl = SLC_CTRL_DMA_START;

	if (read) {
		bool cache_aligned = slc_is_cache_aligned(data);

		for (i = 0; i < chunk_count; ++i) {
			uint32_t dest;

			if (cache_aligned) {
				dest = chunk;
			} else {
				if (i == 0) {
					dest = (uint32_t) self->chunk_buffer_0;
				} else if (i == chunk_count - 1) {
					dest = (uint32_t) self->chunk_buffer_1;
				} else {
					dest = chunk;
				}
			}

			slc_cache_invalidate(
				(const void *) dest,
				BED_LPC32XX_SLC_CHUNK_DATA_SIZE
			);

			dma->int_tc_clear = channel_bit;

			channel->desc.src = (uint32_t) &slc->dma_data;
			channel->desc.dest = dest;
			channel->desc.ctrl = DMA_CH_CTRL_TSZ(SLC_CHUNK_DATA_WORD_COUNT)
				| DMA_CH_CTRL_DI
				| DMA_CH_CTRL_SB_4
				| DMA_CH_CTRL_DB_4
				| DMA_CH_CTRL_SW_32
				| DMA_CH_CTRL_DW_8
				| DMA_CH_CTRL_I;
			channel->cfg = DMA_CH_CFG_E
				| DMA_CH_CFG_SPER(LPC32XX_DMA_PER_NAND_0)
				| DMA_CH_CFG_FLOW_PER_TO_MEM_DMA
				| DMA_CH_CFG_IE
				| DMA_CH_CFG_ITC;

			slc_dma_wait(dma, channel_bit);

			lpc32xx_micro_seconds_delay(10);
			self->ecc_buffer [i] = slc->ecc;

			chunk += BED_LPC32XX_SLC_CHUNK_DATA_SIZE;
		}

		if (!cache_aligned) {
			memcpy(
				(void *) data,
				self->chunk_buffer_0,
				BED_LPC32XX_SLC_CHUNK_DATA_SIZE
			);
			memcpy(
				(void *) (data + (chunk_count - 1) * BED_LPC32XX_SLC_CHUNK_DATA_SIZE),
				self->chunk_buffer_1,
				BED_LPC32XX_SLC_CHUNK_DATA_SIZE
			);
		}
	} else {
#ifndef BED_CONFIG_READ_ONLY
		for (i = 0; i < chunk_count; ++i) {
			slc_cache_clean(
				(const void *) chunk,
				BED_LPC32XX_SLC_CHUNK_DATA_SIZE
			);

			dma->int_tc_clear = channel_bit;

			channel->desc.src = chunk;
			channel->desc.dest = (uint32_t) &slc->dma_data;
			channel->desc.ctrl = DMA_CH_CTRL_TSZ(BED_LPC32XX_SLC_CHUNK_DATA_SIZE)
				| DMA_CH_CTRL_SI
				| DMA_CH_CTRL_SB_4
				| DMA_CH_CTRL_DB_4
				| DMA_CH_CTRL_SW_8
				| DMA_CH_CTRL_DW_32
				| DMA_CH_CTRL_I;
			channel->cfg = DMA_CH_CFG_E
				| DMA_CH_CFG_DPER(LPC32XX_DMA_PER_NAND_0)
				| DMA_CH_CFG_FLOW_MEM_TO_PER_DMA
				| DMA_CH_CFG_IE
				| DMA_CH_CFG_ITC;

			slc_dma_wait(dma, channel_bit);

			lpc32xx_micro_seconds_delay(10);
			self->ecc_buffer [i] = slc->ecc;

			chunk += BED_LPC32XX_SLC_CHUNK_DATA_SIZE;
		}
#endif /* BED_CONFIG_READ_ONLY */
	}

	slc_cfg &= ~(SLC_CFG_ECC_EN | SLC_CFG_DMA_ECC | SLC_CFG_DMA_BURST | SLC_CFG_DMA_DIR);
	slc->cfg = slc_cfg;
}

static void slc_ecc_copy(uint8_t *ecc, const uint32_t *slc_ecc, size_t count)
{
	size_t i;

	for (i = 0; i < count; ++i) {
		uint32_t e = slc_ecc [i];

		e = ~(e << 2) & 0xffffff;

		*ecc = (uint8_t) e;
		++ecc;

		*ecc = (uint8_t) (e >> 8);
		++ecc;

		*ecc = (uint8_t) (e >> 16);
		++ecc;
	}
}

static bed_status slc_read_page(bed_device *bed, uint8_t *data, bed_oob_mode mode)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	bed_lpc32xx_slc_context *self = nand->context;
	uint8_t *oob = nand->oob_buffer;

	slc_dma_transfer(self, (uintptr_t) data, true);
	slc_read_buffer(bed, oob, bed->oob_size);

	if (mode == BED_OOB_MODE_AUTO) {
		uint8_t calc_ecc_buf [BED_LPC32XX_SLC_CHUNK_COUNT_MAX * BED_ECC_HAMMING_256_SIZE];
		const uint8_t *read_ecc = nand->oob_buffer + nand->oob_ecc_ranges->offset;
		uint8_t *calc_ecc = calc_ecc_buf;
		uint8_t *chunk = data;
		size_t i;

		slc_ecc_copy(calc_ecc, self->ecc_buffer, self->chunk_count);

		for (i = 0; i < self->chunk_count; ++i) {
			bed_status ecc_status = bed_ecc_hamming_256_correct(chunk, read_ecc, calc_ecc);

			if (ecc_status != BED_SUCCESS && status == BED_SUCCESS) {
				status = ecc_status;
			}

			chunk += BED_LPC32XX_SLC_CHUNK_DATA_SIZE;
			read_ecc += BED_ECC_HAMMING_256_SIZE;
			calc_ecc += BED_ECC_HAMMING_256_SIZE;
		}
	}

	return status;
}

#ifndef BED_CONFIG_READ_ONLY
static bed_status slc_write_page(bed_device *bed, const uint8_t *data, bed_oob_mode mode)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	bed_lpc32xx_slc_context *self = nand->context;
	const uint8_t *oob = nand->oob_buffer;
	uint8_t *oob_ecc = nand->oob_buffer + nand->oob_ecc_ranges->offset;

	slc_dma_transfer(self, (uintptr_t) data, false);

	if (mode == BED_OOB_MODE_AUTO) {
		slc_ecc_copy(oob_ecc, self->ecc_buffer, self->chunk_count);
	}

	slc_write_buffer(bed, oob, bed->oob_size);

	return status;
}
#endif /* BED_CONFIG_READ_ONLY */

static void slc_init(volatile bed_lpc32xx_slc *slc, uint32_t tac)
{
	slc->ctrl = SLC_CTRL_SW_RESET;
	slc->cfg = SLC_CFG_ECC_EN | SLC_CFG_DMA_ECC | SLC_CFG_DMA_BURST;
	slc->ien = 0;

	if (tac != 0) {
		slc->tac = tac;
	}
}

static void slc_dma_init(bed_lpc32xx_slc_context *self)
{
	volatile lpc_dma *dma = self->dma;

	LPC32XX_DMACLK_CTRL = 0x1;
	dma->cfg = DMA_CFG_E;
}

static void slc_chip_detected(bed_device *bed)
{
	bed_nand_context *nand = bed->context;
	bed_lpc32xx_slc_context *self = nand->context;

	self->chunk_count = bed->page_size / BED_LPC32XX_SLC_CHUNK_DATA_SIZE;

	slc_dma_init(self);
}

bed_status bed_lpc32xx_slc_init(
	bed_lpc32xx_slc_context *self,
	const bed_lpc32xx_slc_config *config
)
{
	bed_status status = BED_SUCCESS;
	bed_partition *part = &self->part;
	bed_device *bed = &self->bed;
	bed_nand_context *nand = &self->nand;

	memset(self, 0, sizeof(*self));

	bed->context = nand;
	bed->obtain = config->obtain;
	bed->release = config->release;
	bed->select_chip = config->select_chip;
	bed->is_block_valid = bed_nand_is_block_valid;
	bed->read = bed_nand_read;
	bed->read_oob = bed_nand_read_oob;
#ifndef BED_CONFIG_READ_ONLY
	bed->write = bed_nand_write;
	bed->write_oob = bed_nand_write_oob;
	bed->erase = bed_nand_erase;
	bed->mark_block_bad = bed_nand_mark_block_bad;
#endif /* BED_CONFIG_READ_ONLY */
	bed->chip_count = config->chip_count;

	nand->context = self;
	nand->command = bed_nand_command;
	nand->control = slc_control;
	nand->is_ready = slc_is_ready;
	nand->read_8 = slc_read_8;
	nand->read_16 = slc_read_16;
	nand->read_buffer = slc_read_buffer;
	nand->write_buffer = slc_write_buffer;
	nand->read_oob_only = bed_nand_read_oob_only;
	nand->read_page = slc_read_page;
#ifndef BED_CONFIG_READ_ONLY
	nand->write_page = slc_write_page;
	nand->mark_page_bad = bed_nand_mark_page_bad;
#endif /* BED_CONFIG_READ_ONLY */
	nand->ecc_correctable_bits_per_512_bytes = 2;

	self->slc = config->slc;
	self->dma = config->dma;
	self->dma_channel = config->dma_channel;
	self->context = config->context;

	if (status == BED_SUCCESS) {
		slc_init(config->slc, config->tac);

		status = bed_nand_detect(bed, config->chip_count, config->info);
	}

	if (status == BED_SUCCESS) {
		slc_chip_detected(bed);

		status = bed_nand_detect_finalize(bed);

		part->bed = bed;
		part->size = bed->size;
	}

	return status;
}
