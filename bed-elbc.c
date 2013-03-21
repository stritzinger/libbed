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

#include "bed-elbc.h"

#include <assert.h>
#include <string.h>

static bed_elbc_context *elbc_get_context(bed_device *bed)
{
	return (bed_elbc_context *) bed;
}

static void elbc_control(bed_device *bed, int data, int ctrl)
{
	assert(0);
}

static bool elbc_is_ready(bed_device *bed)
{
	return true;
}

static void elbc_execute(bed_elbc_context *self)
{
	if (!self->execute_done) {
		self->execute_done = true;

		self->ltesr = bed_elbc_execute(self->elbc, self->bank, self->fmr);
	}
}

static void elbc_read_buffer(bed_device *bed, uint8_t *data, size_t n)
{
	bed_elbc_context *self = elbc_get_context(bed);
	uint8_t *buf = &self->current_buffer[self->current_buffer_offset];
	volatile bed_elbc *elbc = self->elbc;

	assert(n > 0);
	assert(self->current_buffer_offset <= 2048 + 64);

	if (elbc->fbcr == 0) {
		elbc->fbcr = n;
	}

	elbc_execute(self);

	self->current_buffer_offset += n;

	memcpy(data, buf, n);
}

static uint8_t elbc_read_8(bed_device *bed)
{
	uint8_t buf[1];

	elbc_read_buffer(bed, &buf[0], sizeof(buf));

	return buf[0];
}

static uint16_t elbc_read_16(bed_device *bed)
{
	uint8_t buf[2];

	elbc_read_buffer(bed, &buf[0], sizeof(buf));

	return (uint16_t) ((buf[1] << 8) | buf[0]);
}

static void elbc_write_buffer(bed_device *bed, const uint8_t *data, size_t n)
{
	bed_elbc_context *self = elbc_get_context(bed);
	uint8_t *buf = &self->current_buffer[self->current_buffer_offset];
	volatile bed_elbc *elbc = self->elbc;

	assert(n > 0);
	assert(self->current_buffer_offset <= 2048 + 64);

	self->current_buffer_offset += n;

	memcpy(buf, data, n);

	bed_elbc_write_workaround(&buf[n - 1]);
}

static void elbc_set_ecc_mode(const bed_elbc_context *self, bed_oob_mode mode)
{
	bed_elbc_ecc_mode ecc_mode = mode == BED_OOB_MODE_AUTO ?
		self->ecc_mode_auto : ELBC_ECC_MODE_NONE;

	bed_elbc_set_ecc_mode(self->elbc, self->bank, ecc_mode);
}

static bed_status elbc_read_page(bed_device *bed, uint8_t *data, bed_oob_mode mode)
{
	bed_status status = BED_SUCCESS;
	bed_elbc_context *self = elbc_get_context(bed);
	bed_nand_context *nand = &self->nand;
	uint8_t *buf = &self->current_buffer[0];
	uint8_t *oob = nand->oob_buffer;

	assert(self->current_buffer_offset == 0);

	elbc_set_ecc_mode(self, mode);
	elbc_execute(self);

	memcpy(data, buf, bed->page_size);
	buf += bed->page_size;
	memcpy(oob, buf, bed->oob_size);

	if (mode == BED_OOB_MODE_AUTO && (self->ltesr & ELBC_LTE_PAR) != 0) {
		status = BED_ERROR_ECC_UNCORRECTABLE;
	} else if ((self->ltesr & ELBC_LTE_FCT) != 0) {
		status = BED_ERROR_NO_DEVICE;
	}

	return status;
}

#ifndef BED_CONFIG_READ_ONLY
static bed_status elbc_write_page(bed_device *bed, const uint8_t *data, bed_oob_mode mode)
{
	bed_status status = BED_SUCCESS;
	bed_elbc_context *self = elbc_get_context(bed);
	bed_nand_context *nand = &self->nand;
	uint8_t *buf = &self->current_buffer[0];
	uint8_t *oob = nand->oob_buffer;

	assert(self->current_buffer_offset == 0);

	elbc_set_ecc_mode(self, mode);

	memcpy(buf, data, bed->page_size);
	buf += bed->page_size;
	memcpy(buf, oob, bed->oob_size);

	bed_elbc_write_workaround(&buf[bed->oob_size - 1]);

	return status;
}
#endif /* BED_CONFIG_READ_ONLY */

static void elbc_command_large_pages(bed_device *bed, bed_nand_cmd cmd, uint32_t row_or_byte, uint16_t column)
{
	bed_elbc_context *self = elbc_get_context(bed);
	volatile bed_elbc *elbc = self->elbc;
	bool execute = false;
	bool prepare = false;
	int op_shift = ELBC_FIR_OP0_SHIFT;
	int cmd_shift = ELBC_FCR_CMD0_SHIFT;
	uint32_t fbar = 0;
	uint32_t fpar = 0;
	uint32_t fir = 0;
	uint32_t fcr = 0;
	uint32_t fbcr = 0;

	if (cmd == BED_NAND_CMD_READ_OOB) {
		column = (uint16_t) (column + bed->page_size);
		cmd = BED_NAND_CMD_READ_PAGE;
	}

	fir |= ELBC_FIR_OP_CM0 << op_shift;
	op_shift += ELBC_FIR_OP_SHIFT_INC;
	fcr |= (cmd & 0xffU) << cmd_shift;
	cmd_shift += ELBC_FCR_CMD_SHIFT_INC;

	if ((cmd & (BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW | BED_NAND_CMD_ADDR_BYTE)) != 0) {
		uint32_t buffer_index = 0;

		if ((cmd & BED_NAND_CMD_ADDR_COLUMN) != 0) {
			fpar |= column;

			fir |= ELBC_FIR_OP_CA << op_shift;
			op_shift += ELBC_FIR_OP_SHIFT_INC;
		}

		if ((cmd & BED_NAND_CMD_ADDR_ROW) != 0) {
			if (bed->page_size == 512) {
				fbar |= row_or_byte >> 5;
				fpar |= ELBC_FPAR_SP_PI(row_or_byte);

				buffer_index = row_or_byte & 0x7;
			} else {
				fbar |= row_or_byte >> 6;
				fpar |= ELBC_FPAR_LP_PI(row_or_byte);

				buffer_index = (row_or_byte & 0x1) << 2;
			}

			fir |= ELBC_FIR_OP_PA << op_shift;
			op_shift += ELBC_FIR_OP_SHIFT_INC;
		} else if ((cmd & BED_NAND_CMD_ADDR_BYTE) != 0) {
			elbc->mdr = ELBC_MDR_AS0(row_or_byte);

			fir |= ELBC_FIR_OP_UA << op_shift;
			op_shift += ELBC_FIR_OP_SHIFT_INC;
		}

		self->current_buffer = self->base_address + buffer_index * 1024;
		self->current_buffer_offset = column;
	} else if (cmd == BED_NAND_CMD_READ_STATUS) {
		self->current_buffer = self->base_address;
		self->current_buffer_offset = 0;
	}

	switch (cmd) {
		case BED_NAND_CMD_READ_PAGE:
			fir |= ELBC_FIR_OP_CW1 << op_shift;
			op_shift += ELBC_FIR_OP_SHIFT_INC;
			fcr |= (BED_NAND_CMD_READ_PAGE_2 & 0xffU) << cmd_shift;
			/* Fall through */
		case BED_NAND_CMD_READ_ID:
		case BED_NAND_CMD_READ_PARAMETER_PAGE:
		case BED_NAND_CMD_READ_STATUS:
			fir |= ELBC_FIR_OP_RBW << op_shift;
			break;
		case BED_NAND_CMD_PROGRAM_PAGE:
			fir |= ELBC_FIR_OP_WB << op_shift;
			op_shift += ELBC_FIR_OP_SHIFT_INC;

			fir |= ELBC_FIR_OP_CW1 << op_shift;
			fcr |= (BED_NAND_CMD_PROGRAM_PAGE_2 & 0xffU) << cmd_shift;
			break;
		case BED_NAND_CMD_ERASE_BLOCK:
			fir |= ELBC_FIR_OP_CW1 << op_shift;
			fcr |= (BED_NAND_CMD_ERASE_BLOCK_2 & 0xffU) << cmd_shift;
			break;
		case BED_NAND_CMD_SET_FEATURES:
			fir |= ELBC_FIR_OP_WB << op_shift;
			break;
		default:
			/* Do nothing */
			break;
	}

	switch (cmd) {
		case BED_NAND_CMD_ERASE_BLOCK:
		case BED_NAND_CMD_PROGRAM_PAGE:
		case BED_NAND_CMD_READ_ID:
		case BED_NAND_CMD_READ_OOB:
		case BED_NAND_CMD_READ_PAGE:
		case BED_NAND_CMD_READ_STATUS:
		case BED_NAND_CMD_SET_FEATURES:
			prepare = true;
			break;
		case BED_NAND_CMD_ERASE_BLOCK_2:
		case BED_NAND_CMD_PROGRAM_PAGE_2:
			execute = true;
			break;
		case BED_NAND_CMD_SET_FEATURES_2:
			fbcr = self->current_buffer_offset;
			execute = true;
			break;
		case BED_NAND_CMD_READ_MODE:
		case BED_NAND_CMD_RESET:
			prepare = true;
			execute = true;
			break;
		case BED_NAND_CMD_READ_PARAMETER_PAGE:
			fbcr = 3 * sizeof(bed_nand_onfi);
			prepare = true;
			break;
		default:
			/* Do nothing */
			break;
	}

	if (prepare) {
		elbc->fbar = fbar;
		elbc->fpar = fpar;
		elbc->fir = fir;
		elbc->fcr = fcr;
		elbc->fbcr = fbcr;

		self->execute_done = false;
	}

	if (execute) {
		elbc_execute(self);
	}
}

static const bed_nand_range elbc_oob_free_ranges_16_eccm_0 [] = {
	{ 0, 5 },
	{ 9, 7 },
	{ 0, 0 }
};

static const bed_nand_range elbc_oob_free_ranges_64_eccm_1 [] = {
	{ 1, 7 },
	{ 11, 13 },
	{ 27, 13 },
	{ 43, 13 },
	{ 59, 5 },
	{ 0, 0 }
};

static void elbc_chip_detected(bed_device *bed)
{
	bed_elbc_context *self = elbc_get_context(bed);
	bed_nand_context *nand = &self->nand;
	volatile bed_elbc *elbc = self->elbc;

	if (nand->ecc_correctable_bits_per_512_bytes == ELBC_ECC_CORRECTABLE_BITS_PER_512_BYTES) {
		self->ecc_mode_auto = ELBC_ECC_MODE_CHECK_AND_GENERATE;
	} else {
		self->ecc_mode_auto = ELBC_ECC_MODE_NONE;
	}

	self->fmr = bed_elbc_finalize_module(
		elbc,
		self->bank,
		bed_nand_has_large_pages(bed),
		bed_nand_needs_3_page_cycles(bed),
		ELBC_ECC_MODE_NONE
	);

	nand->oob_ecc_ranges = NULL;
	if (bed_nand_has_large_pages(bed)) {
		bed->oob_free_size = 51;
		nand->oob_free_ranges = elbc_oob_free_ranges_64_eccm_1;
	} else {
		bed->oob_free_size = 12;
		nand->oob_free_ranges = elbc_oob_free_ranges_16_eccm_0;
	}
}

static void elbc_init_context(
	bed_elbc_context *self,
	const bed_elbc_config *config
)
{
	bed_device *bed = &self->bed;
	bed_nand_context *nand = &self->nand;
	uint16_t chip_count = 1;

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
	bed->chip_count = 1;
	bed->ecc_covers_oob = 0;

	nand->context = self;
	nand->command = elbc_command_large_pages;
	nand->control = elbc_control;
	nand->is_ready = elbc_is_ready;
	nand->read_8 = elbc_read_8;
	nand->read_16 = elbc_read_16;
	nand->read_buffer = elbc_read_buffer;
	nand->write_buffer = elbc_write_buffer;
	nand->read_oob_only = bed_nand_read_oob_only;
	nand->read_page = elbc_read_page;
#ifndef BED_CONFIG_READ_ONLY
	nand->write_page = elbc_write_page;
	nand->mark_page_bad = bed_nand_mark_page_bad;
#endif /* BED_CONFIG_READ_ONLY */
	nand->ecc_correctable_bits_per_512_bytes = ELBC_ECC_CORRECTABLE_BITS_PER_512_BYTES;

	self->elbc = config->elbc;
	self->base_address = config->base_address;
	self->bank = config->bank;
}

bed_status bed_elbc_init(
	bed_elbc_context *self,
	const bed_elbc_config *config
)
{
	bed_status status = BED_SUCCESS;
	bed_device *bed = &self->bed;

	elbc_init_context(self, config);

	status = bed_nand_detect(bed, bed->chip_count, config->info);
	if (status == BED_SUCCESS) {
		bed_partition *part = &self->part;

		elbc_chip_detected(bed);

		status = bed_nand_detect_finalize(bed);

		part->bed = bed;
		part->size = bed->size;
	}

	return status;
}

void bed_elbc_init_with_no_chip_detection(
	bed_elbc_context *self,
	const bed_elbc_config *config,
	uint32_t blocks_per_chip,
	uint32_t block_size,
	uint16_t page_size
)
{
	bed_device *bed = &self->bed;
	bed_partition *part = &self->part;

	elbc_init_context(self, config);

	bed->page_size = page_size;
	bed->block_size = block_size;
	bed->blocks_per_chip = blocks_per_chip;
	bed->oob_size = page_size / 32;

	bed_nand_set_bad_block_marker_position(bed);
	bed_set_geometry_parameters(bed);

	elbc_chip_detected(bed);

	part->bed = bed;
	part->size = bed->size;
}
