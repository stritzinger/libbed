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

#define MLC_CHUNK_SIZE 528
#define MLC_CHUNK_DATA_SIZE 512
#define MLC_CHUNK_OOB_SIZE 16
#define MLC_CHUNK_OOB_FREE_SIZE 5
#define MLC_CHUNK_OOB_USER_SIZE 6
#define MLC_CHUNK_OOB_ECC_SIZE 10
#define MLC_CHUNK_DATA_WORD_COUNT (MLC_CHUNK_DATA_SIZE / 4)
#define MLC_CHUNK_OOB_WORD_COUNT (MLC_CHUNK_OOB_SIZE / 4)

/**
 * @name MLC NAND Lock Protection Register (MLC_LOCK_PR)
 *
 * @{
 */

#define MLC_UNLOCK_PROT 0xa25e

/** @} */

/**
 * @name MLC NAND Status Register (MLC_ISR)
 *
 * @{
 */

#define MLC_ISR_DECODER_FAILURE BED_BIT32(6)
#define MLC_ISR_SYMBOL_ERRORS(reg) BED_FLD32GET(reg, 4, 5)
#define MLC_ISR_ERRORS_DETECTED BED_BIT32(3)
#define MLC_ISR_ECC_READY BED_BIT32(2)
#define MLC_ISR_CONTROLLER_READY BED_BIT32(1)
#define MLC_ISR_NAND_READY BED_BIT32(0)

/** @} */

/**
 * @name MLC NAND Controller Configuration Register (MLC_ICR)
 *
 * @{
 */

#define MLC_ICR_SOFT_WRITE_PROT BED_BIT32(3)
#define MLC_ICR_LARGE_PAGES BED_BIT32(2)
#define MLC_ICR_ADDR_WORD_COUNT_4_5 BED_BIT32(1)
#define MLC_ICR_IO_BUS_16 BED_BIT32(0)

/** @} */

/**
 * @name MLC NAND Auto Encode Register (MLC_ECC_AUTO_ENC)
 *
 * @{
 */

#define MLC_ECC_AUTO_ENC_PROGRAM BED_BIT32(8)

/** @} */

/**
 * @name MLC NAND Chip-Enable Host Control Register (MLC_CEH)
 *
 * @{
 */

#define MLC_CEH_NORMAL BED_BIT32(0)

/** @} */

static void mlc_wait(volatile bed_lpc32xx_mlc *mlc, uint32_t flags)
{
	while ((mlc->isr & flags) != flags) {
		/* Wait */
	}
}

static void mlc_control(bed_device *bed, int data, int ctrl)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;

	if (bed_nand_is_real_data(data)) {
		if ((ctrl & BED_NAND_CTRL_CLE) != 0) {
			mlc->cmd = (uint8_t) data;
		} else {
			mlc->addr = (uint8_t) data;
		}
	}
}

static bool mlc_is_ready(bed_device *bed)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;
	uint32_t flags = MLC_ISR_CONTROLLER_READY | MLC_ISR_NAND_READY;

	return (mlc->isr & flags) == flags;
}

static bool mlc_is_word_aligned(const uint8_t *data, size_t n)
{
	return (((uintptr_t) data) | n) % 4 == 0;
}

static void mlc_read(volatile bed_lpc32xx_mlc_port *port, uint8_t *data, size_t n)
{
	size_t i;

	if (mlc_is_word_aligned(data, n)) {
		uint32_t *data_32 = (uint32_t *) data;

		for (i = 0; i < n / 4; ++i) {
			data_32 [i] = port->w32;
		}
	} else {
		for (i = 0; i < n; ++i) {
			data [i] = port->w8;
		}
	}
}

static void mlc_read_buffer(bed_device *bed, uint8_t *data, size_t n)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;

	mlc_read(&mlc->data, data, n);
}

static uint8_t mlc_read_8(bed_device *bed)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;

	return mlc->data.w8;
}

static uint16_t mlc_read_16(bed_device *bed)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;

	return mlc->data.w16;
}

static bool mlc_is_fixed(
	const bed_nand_context *nand,
	uint32_t isr
)
{
	return MLC_ISR_SYMBOL_ERRORS(isr) > 2;
}

static bed_status mlc_check_ecc_status(
	bed_status status,
	const bed_nand_context *nand,
	uint32_t isr
)
{
	if ((isr & MLC_ISR_ERRORS_DETECTED) != 0) {
		if ((isr & MLC_ISR_DECODER_FAILURE) == 0) {
			if (status == BED_SUCCESS && mlc_is_fixed(nand, isr)) {
				status = BED_ERROR_ECC_FIXED;
			}
		} else {
			status = BED_ERROR_ECC_UNCORRECTABLE;
		}
	}

	return status;
}

static bed_status mlc_read_page(bed_device *bed, uint8_t *data, bool use_ecc)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;
	uint8_t *oob = nand->oob_buffer;
	int i;

	if (use_ecc) {
		for (i = 0; i < self->chunk_count; ++i) {
			mlc->ecc_auto_dec = 0;

			mlc_wait(mlc, MLC_ISR_CONTROLLER_READY | MLC_ISR_NAND_READY);

			status = mlc_check_ecc_status(status, nand, mlc->isr);

			mlc_read(&mlc->buff, data, MLC_CHUNK_DATA_SIZE);
			mlc_read(&mlc->buff, oob, MLC_CHUNK_OOB_SIZE);

			data += MLC_CHUNK_DATA_SIZE;
			oob += MLC_CHUNK_OOB_SIZE;
		}
	} else {
		for (i = 0; i < self->chunk_count; ++i) {
			mlc_wait(mlc, MLC_ISR_NAND_READY);

			mlc_read(&mlc->data, data, MLC_CHUNK_DATA_SIZE);
			mlc_read(&mlc->data, oob, MLC_CHUNK_OOB_SIZE);

			data += MLC_CHUNK_DATA_SIZE;
			oob += MLC_CHUNK_OOB_SIZE;
		}
	}

	return status;
}

static void mlc_write(volatile bed_lpc32xx_mlc_port *port, const uint8_t *data, size_t n)
{
	size_t i;

	if (mlc_is_word_aligned(data, n)) {
		const uint32_t *data_32 = (const uint32_t *) data;

		for (i = 0; i < n / 4; ++i) {
			port->w32 = data_32 [i];
		}
	} else {
		for (i = 0; i < n; ++i) {
			port->w8 = data [i];
		}
	}
}

static void mlc_write_buffer(bed_device *bed, const uint8_t *data, size_t n)
{
	const bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;

	mlc_write(&mlc->data, data, n);
}

#ifndef BED_CONFIG_READ_ONLY
static bed_status mlc_write_page(bed_device *bed, const uint8_t *data, bool use_ecc)
{
	bed_status status = BED_SUCCESS;

	if (use_ecc) {
		bed_nand_context *nand = bed->context;
		const bed_lpc32xx_mlc_context *self = nand->context;
		volatile bed_lpc32xx_mlc *mlc = self->mlc;
		const uint8_t *oob = nand->oob_buffer;
		int i;

		for (i = 0; i < self->chunk_count; ++i) {
			mlc->ecc_enc = 0;

			mlc_write(&mlc->buff, data, MLC_CHUNK_DATA_SIZE);
			mlc_write(&mlc->buff, oob, MLC_CHUNK_OOB_USER_SIZE);

			mlc->ecc_auto_enc = 0;

			mlc_wait(mlc, MLC_ISR_CONTROLLER_READY);

			data += MLC_CHUNK_DATA_SIZE;
			oob += MLC_CHUNK_OOB_SIZE;
		}
	} else {
		status = BED_ERROR_OOB_MODE;
	}

	return status;
}

static bed_status mlc_mark_page_bad(bed_device *bed, uint32_t page)
{
	bed_nand_context *nand = bed->context;
	const bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;
	const uint8_t *oob = nand->oob_buffer;
	int i;

	(*nand->command)(bed, BED_NAND_CMD_PROGRAM_PAGE, page, 0);

	mlc->ecc_enc = 0;

	for (i = 0; i < MLC_CHUNK_DATA_WORD_COUNT; ++i) {
		mlc->buff.w32 = 0;
	}

	mlc_write(&mlc->buff, oob, MLC_CHUNK_OOB_USER_SIZE);

	mlc->ecc_auto_enc = 0;

	mlc_wait(mlc, MLC_ISR_CONTROLLER_READY);

	(*nand->command)(bed, BED_NAND_CMD_PROGRAM_PAGE_2, 0, 0);

	return bed_nand_check_status(bed, BED_ERROR_WRITE);
}
#endif /* BED_CONFIG_READ_ONLY */

static void mlc_unlock(volatile bed_lpc32xx_mlc *mlc)
{
	mlc->lock_pr = MLC_UNLOCK_PROT;
}

static void mlc_init(volatile bed_lpc32xx_mlc *mlc, uint32_t time)
{
	if (time != 0) {
		mlc_unlock(mlc);
		mlc->time = time;
	}

	mlc_unlock(mlc);
	mlc->icr = 0;

	mlc->ceh = MLC_CEH_NORMAL;
}

static void mlc_finalize(
	const bed_device *bed,
	volatile bed_lpc32xx_mlc *mlc
)
{
	uint32_t icr = 0;

	if (bed_nand_has_large_pages(bed)) {
		icr |= MLC_ICR_LARGE_PAGES;
	}

	if (bed_nand_needs_3_page_cycles(bed)) {
		icr |= MLC_ICR_ADDR_WORD_COUNT_4_5;
	}

	if (icr != 0) {
		mlc_unlock(mlc);
		mlc->icr = icr;
	}
}

static const bed_nand_range mlc_oob_free_ranges_16 [] = {
	{ 0, 4 },
	{ 0, 0 }
};

static const bed_nand_range mlc_oob_free_ranges_64 [] = {
	{ 0, 6 },
	{ 16, 6 },
	{ 32, 6 },
	{ 48, 6 },
	{ 0, 0 }
};

static void mlc_chip_detected(bed_device *bed)
{
	bed_nand_context *nand = bed->context;
	bed_lpc32xx_mlc_context *self = nand->context;
	volatile bed_lpc32xx_mlc *mlc = self->mlc;

	mlc_finalize(bed, mlc);

	self->chunk_count = bed->page_size / MLC_CHUNK_DATA_SIZE;

	nand->oob_ecc_ranges = NULL;

	if (bed_nand_has_large_pages(bed)) {
		bed->oob_free_size = 24;
		nand->oob_free_ranges = mlc_oob_free_ranges_64;
	} else {
		bed->is_block_valid = bed_nand_is_block_valid;
		bed->oob_free_size = 4;
		nand->oob_free_ranges = mlc_oob_free_ranges_16;
#ifndef BED_CONFIG_READ_ONLY
		nand->mark_page_bad = mlc_mark_page_bad;
#endif /* BED_CONFIG_READ_ONLY */
	}
}

bed_status bed_lpc32xx_mlc_init(
	bed_lpc32xx_mlc_context *self,
	const bed_lpc32xx_mlc_config *config
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
	bed->is_block_valid = bed_is_block_valid_not_supported;
	bed->read = bed_nand_read;
	bed->read_oob = bed_nand_read_oob;
#ifndef BED_CONFIG_READ_ONLY
	bed->write = bed_nand_write;
	bed->write_oob = bed_nand_write_oob;
	bed->erase = bed_nand_erase;
	bed->mark_block_bad = bed_nand_mark_block_bad;
#endif /* BED_CONFIG_READ_ONLY */
	bed->chip_count = config->chip_count;
	bed->ecc_covers_oob = 1;

	nand->context = self;
	nand->command = bed_nand_command;
	nand->control = mlc_control;
	nand->is_ready = mlc_is_ready;
	nand->read_8 = mlc_read_8;
	nand->read_16 = mlc_read_16;
	nand->read_buffer = mlc_read_buffer;
	nand->write_buffer = mlc_write_buffer;
	nand->read_oob_only = bed_nand_read_oob_only_with_trash;
	nand->read_page = mlc_read_page;
#ifndef BED_CONFIG_READ_ONLY
	nand->write_page = mlc_write_page;
	nand->mark_page_bad = bed_nand_mark_page_bad_not_supported;
#endif /* BED_CONFIG_READ_ONLY */
	nand->ecc_correctable_bits_per_512_bytes = 4;

	self->mlc = config->mlc;

	if (status == BED_SUCCESS) {
		mlc_init(config->mlc, config->time);

		status = bed_nand_detect(bed, config->chip_count, config->info);
	}

	if (status == BED_SUCCESS) {
		mlc_chip_detected(bed);

		status = bed_nand_detect_finalize(bed);

		part->bed = bed;
		part->size = bed->size;
	}

	return status;
}
