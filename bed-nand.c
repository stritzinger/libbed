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

#include "bed-nand.h"

#include <assert.h>
#include <string.h>

static const bed_oob_request null_oob = {
	.mode = BED_OOB_MODE_AUTO,
	.offset = 0,
	.size = 0,
	.data = NULL
};

void bed_nand_wait_for_ready(bed_device *bed)
{
	bed_nand_context *nand = bed->context;

	while (!(*nand->is_ready)(bed)) {
		/* Wait */
	}
}

void bed_nand_command(bed_device *bed, bed_nand_cmd cmd, uint32_t row_or_byte, uint16_t column)
{
	bed_nand_context *nand = bed->context;
	int cmd_ctrl = BED_NAND_CTRL_CLE;
	int addr_ctrl = BED_NAND_CTRL_ALE;

	if (cmd == BED_NAND_CMD_PROGRAM_PAGE) {
		if (column >= bed->page_size) {
			(*nand->control)(bed, BED_NAND_CMD_POINTER_OOB, cmd_ctrl);
			column = (uint16_t) (column - bed->page_size);
		} else if (column >= 256) {
			assert(0);
		}
	}

	(*nand->control)(bed, cmd, cmd_ctrl);

	if ((cmd & BED_NAND_CMD_ADDR_COLUMN) != 0) {
		if ((nand->flags & BED_NAND_FLG_BUS_WIDTH_16) != 0) {
			column >>= 1;
		}

		(*nand->control)(bed, column, addr_ctrl);
	}

	if ((cmd & BED_NAND_CMD_ADDR_ROW) != 0) {
		(*nand->control)(bed, (int) row_or_byte, addr_ctrl);
		(*nand->control)(bed, (int) (row_or_byte >> 8), addr_ctrl);
		if (bed_nand_needs_3_page_cycles(bed)) {
			(*nand->control)(bed, (int) (row_or_byte >> 16), addr_ctrl);
		}
	} else if ((cmd & BED_NAND_CMD_ADDR_BYTE) != 0) {
		(*nand->control)(bed, (int) row_or_byte, addr_ctrl);
	}

	(*nand->control)(bed, BED_NAND_CMD_NONE, 0);

	if ((cmd & BED_NAND_CMD_WAIT_FOR_READY) != 0) {
		bed_nand_wait_for_ready(bed);
	}
}

void bed_nand_command_large_pages(bed_device *bed, bed_nand_cmd cmd, uint32_t row_or_byte, uint16_t column)
{
	bed_nand_context *nand = bed->context;
	int addr_ctrl = BED_NAND_CTRL_ALE;

	if (cmd == BED_NAND_CMD_READ_OOB) {
		column = (uint16_t) (column + bed->page_size);
		cmd = BED_NAND_CMD_READ_PAGE;
	}

	(*nand->control)(bed, cmd, BED_NAND_CTRL_CLE);

	if ((cmd & BED_NAND_CMD_ADDR_COLUMN) != 0) {
		if ((nand->flags & BED_NAND_FLG_BUS_WIDTH_16) != 0) {
			column >>= 1;
		}

		(*nand->control)(bed, column, addr_ctrl);
		(*nand->control)(bed, column >> 8, addr_ctrl);
	}

	if ((cmd & BED_NAND_CMD_ADDR_ROW) != 0) {
		(*nand->control)(bed, (int) row_or_byte, addr_ctrl);
		(*nand->control)(bed, (int) (row_or_byte >> 8), addr_ctrl);
		if (bed_nand_needs_3_page_cycles(bed)) {
			(*nand->control)(bed, (int) (row_or_byte >> 16), addr_ctrl);
		}
	} else if ((cmd & BED_NAND_CMD_ADDR_BYTE) != 0) {
		(*nand->control)(bed, (int) row_or_byte, addr_ctrl);
	}

	(*nand->control)(bed, BED_NAND_CMD_NONE, 0);

	if (cmd == BED_NAND_CMD_READ_PAGE) {
		(*nand->control)(
			bed,
			BED_NAND_CMD_READ_PAGE_2,
			BED_NAND_CTRL_CLE
		);
		(*nand->control)(bed, BED_NAND_CMD_NONE, 0);
	}

	if ((cmd & BED_NAND_CMD_WAIT_FOR_READY) != 0) {
		bed_nand_wait_for_ready(bed);
	}
}

static bed_status is_page_valid(bed_device *bed, uint32_t page)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;

	status = (*nand->read_oob_only)(bed, page);
	if (status == BED_SUCCESS) {
		status = nand->oob_buffer [nand->bbc.marker_position] == 0xff ?
			BED_SUCCESS : BED_ERROR_BLOCK_IS_BAD;
	}

	return status;
}

bed_status bed_nand_is_block_valid(bed_device *bed, bed_address addr)
{
	bed_status status = BED_SUCCESS;

	if (bed_is_block_aligned(bed, addr)) {
		bed_nand_context *nand = bed->context;
		uint16_t chip = (uint16_t) (addr >> bed->chip_shift);
		uint32_t page = ((uint32_t) (addr >> bed->page_shift)) & bed->page_mask;
		int i = 0;

		bed_select_chip(bed, chip);

		if ((nand->bbc.flags & BED_NAND_BBC_CHECK_LAST_PAGE) != 0) {
			page = bed->pages_per_block - 1U;
		}

		do {
			status = is_page_valid(bed, page);
			++i;
			page += bed->pages_per_block;
		} while ((nand->bbc.flags & BED_NAND_BBC_CHECK_SECOND_PAGE) != 0 && status == BED_SUCCESS && i < 2);
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}

static void copy_oob(
	const bed_nand_context *nand,
	const bed_oob_request *oob
)
{
	const uint8_t *oob_buffer = nand->oob_buffer;
	uint8_t *data = oob->data;
	size_t offset = oob->offset;
	size_t size = oob->size;

	switch (oob->mode) {
		case BED_OOB_MODE_RAW:
			memcpy(data, oob_buffer + offset, size);
			break;
		case BED_OOB_MODE_AUTO: {
			const bed_nand_range *range = nand->oob_free_ranges;

			while (range->size > 0 && size > 0) {
				if (offset < range->size) {
					size_t s = range->size - offset;
					size_t k = s < size ? s : size;

					memcpy(data, oob_buffer + range->offset, k);
					++range;
					size -= k;
					data += k;
				} else {
					offset -= range->size;
				}
			}

			break;
		}
		default:
			assert(0);
			break;
	}
}

bed_status bed_nand_read_oob_only(bed_device *bed, uint32_t page)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;

	(*nand->command)(bed, BED_NAND_CMD_READ_OOB, page, 0);
	(*nand->read_buffer)(bed, nand->oob_buffer, bed->oob_size);

	return status;
}

bed_status bed_nand_read_oob_only_with_trash(bed_device *bed, uint32_t page)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;

	(*nand->command)(bed, BED_NAND_CMD_READ_PAGE, page, 0);
	status = (*nand->read_page)(bed, bed_trash_buffer(bed->page_size), BED_OOB_MODE_RAW);

	return status;
}

bed_status bed_nand_read(
	bed_device *bed,
	bed_address addr,
	void *data,
	size_t n
)
{
	return (bed->read_oob)(bed, addr, data, n, &null_oob);
}

bed_status bed_nand_read_oob(
	bed_device *bed,
	bed_address addr,
	void *data,
	size_t n,
	const bed_oob_request *oob
)
{
	bed_status status = BED_SUCCESS;

	if (n == 0 || n == bed->page_size) {
		bed_nand_context *nand = bed->context;
		uint16_t chip = (uint16_t) (addr >> bed->chip_shift);
		uint32_t page = ((uint32_t) (addr >> bed->page_shift)) & bed->page_mask;

		bed_select_chip(bed, chip);

		if (n != 0) {
			(*nand->command)(bed, BED_NAND_CMD_READ_PAGE, page, 0);
			status = (*nand->read_page)(bed, data, oob->mode);
		} else {
			status = (*nand->read_oob_only)(bed, page);
		}

		copy_oob(nand, oob);
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}

static uint8_t read_status(bed_device *bed)
{
	bed_nand_context *nand = bed->context;

	bed_nand_wait_for_ready(bed);
	(*nand->command)(bed, BED_NAND_CMD_READ_STATUS, 0, 0);

	return (*nand->read_8)(bed);
}

bed_status bed_nand_check_status(bed_device *bed, bed_status error_status)
{
	uint8_t nand_status = read_status(bed);

	return (nand_status & (BED_NAND_STATUS_READY | BED_NAND_STATUS_FAIL)) == BED_NAND_STATUS_READY ?
		BED_SUCCESS : error_status;
}

#ifndef BED_CONFIG_READ_ONLY
static void fill_oob(
	const bed_device *bed,
	bed_nand_context *nand,
	const bed_oob_request *oob
)
{
	uint8_t *oob_buffer = nand->oob_buffer;
	const uint8_t *data = oob->data;
	size_t offset = oob->offset;
	size_t size = oob->size;

	memset(oob_buffer, 0xff, bed->oob_size);

	switch (oob->mode) {
		case BED_OOB_MODE_RAW:
			memcpy(oob_buffer + offset, data, size);
			break;
		case BED_OOB_MODE_AUTO: {
			const bed_nand_range *range = nand->oob_free_ranges;

			while (range->size > 0 && size > 0) {
				if (offset < range->size) {
					size_t s = range->size - offset;
					size_t k = s < size ? s : size;

					memcpy(oob_buffer + range->offset, data, k);
					++range;
					size -= k;
					data += k;
				} else {
					offset -= range->size;
				}
			}

			break;
		}
		default:
			assert(0);
			break;
	}
}

bed_status bed_nand_write(
	bed_device *bed,
	bed_address addr,
	const void *data,
	size_t n
)
{
	return (*bed->write_oob)(bed, addr, data, n, &null_oob);
}

bed_status bed_nand_write_oob(
	bed_device *bed,
	bed_address addr,
	const void *data,
	size_t n,
	const bed_oob_request *oob
)
{
	bed_status status = BED_SUCCESS;

	if (n == bed->page_size) {
		bed_nand_context *nand = bed->context;
		uint16_t chip = (uint16_t) (addr >> bed->chip_shift);
		uint32_t page = ((uint32_t) (addr >> bed->page_shift)) & bed->page_mask;

		fill_oob(bed, nand, oob);
		bed_select_chip(bed, chip);
		(*nand->command)(bed, BED_NAND_CMD_PROGRAM_PAGE, page, 0);
		status = (*nand->write_page)(bed, data, oob->mode);
		if (status == BED_SUCCESS) {
			(*nand->command)(bed, BED_NAND_CMD_PROGRAM_PAGE_2, 0, 0);
			status = bed_nand_check_status(bed, BED_ERROR_WRITE);
		}
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}

bed_status bed_nand_erase(bed_device *bed, bed_address addr)
{
	bed_status status = BED_SUCCESS;

	if (bed_is_block_aligned(bed, addr)) {
		bed_nand_context *nand = bed->context;
		uint16_t chip = (uint16_t) (addr >> bed->chip_shift);
		uint32_t page = ((uint32_t) (addr >> bed->page_shift)) & bed->page_mask;

		bed_select_chip(bed, chip);
		(*nand->command)(bed, BED_NAND_CMD_ERASE_BLOCK, page, 0);
		(*nand->command)(bed, BED_NAND_CMD_ERASE_BLOCK_2, 0, 0);
		status = bed_nand_check_status(bed, BED_ERROR_ERASE);
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}

static bed_status write_bad_block_mark(bed_device *bed, bed_address addr)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	uint16_t chip = (uint16_t) (addr >> bed->chip_shift);
	uint32_t page = ((uint32_t) (addr >> bed->page_shift)) & bed->page_mask;
	uint8_t marker [2] = { 0, 0 };
	bed_oob_request oob = {
		.mode = BED_OOB_MODE_RAW,
		.offset = nand->bbc.marker_position,
		.size = 1,
		.data = marker
	};
	int i = 0;

	bed_select_chip(bed, chip);

	if ((nand->flags & BED_NAND_FLG_BUS_WIDTH_16) != 0) {
		oob.offset = (uint16_t) (oob.offset & ~0x1);
		++oob.size;
	}

	if ((nand->bbc.flags & BED_NAND_BBC_CHECK_LAST_PAGE) != 0) {
		page = bed->pages_per_block - 1U;
	}

	fill_oob(bed, nand, &oob);

	do {
		status = (*nand->mark_page_bad)(bed, page);

		++i;
		page += bed->pages_per_block;
	} while ((nand->bbc.flags & BED_NAND_BBC_CHECK_SECOND_PAGE) != 0 && status == BED_SUCCESS && i < 2);

	return status;
}

bed_status bed_nand_mark_block_bad(bed_device *bed, bed_address addr)
{
	bed_status status = BED_SUCCESS;

	if (bed_is_block_aligned(bed, addr)) {
		status = (*bed->is_block_valid)(bed, addr);

		if (status != BED_ERROR_BLOCK_IS_BAD) {
			(*bed->erase)(bed, addr);
			status = write_bad_block_mark(bed, addr);
		}
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}

bed_status bed_nand_mark_page_bad(bed_device *bed, uint32_t page)
{
	bed_nand_context *nand = bed->context;

	(*nand->command)(bed, BED_NAND_CMD_PROGRAM_PAGE, page, bed->page_size);
	(*nand->write_buffer)(bed, nand->oob_buffer, bed->oob_size);
	(*nand->command)(bed, BED_NAND_CMD_PROGRAM_PAGE_2, 0, 0);

	return bed_nand_check_status(bed, BED_ERROR_WRITE);
}
#endif /* BED_CONFIG_READ_ONLY */

static void get_chip_id(
	bed_device *bed,
	uint8_t *id,
	size_t n
)
{
	const bed_nand_context *nand = bed->context;

	(*nand->command)(bed, BED_NAND_CMD_READ_ID, 0, 0);
	(*nand->read_buffer)(bed, id, n);
}

uint16_t bed_nand_onfi_crc(const bed_nand_onfi *onfi)
{
	uint16_t crc = 0x4f4e;
	const uint8_t *bytes = (const uint8_t *) onfi;
	int i;

	for (i = 0; i < 254; ++i) {
		int j;

		crc = (uint16_t) (crc ^ (bytes [i] << 8));

		for (j = 0; j < 8; ++j) {
			crc = (uint16_t) ((crc << 1) ^ ((crc & 0x8000) ? 0x8005 : 0));
		}
	}

	return crc;
}

static bool check_onfi_crc(const bed_nand_onfi *onfi)
{
	return bed_nand_onfi_crc(onfi) == bed_le16_to_cpu(onfi->crc);
}

static bed_status detect_onfi_chip(bed_device *bed)
{
	static const uint8_t onfi_id [] = { 'O', 'N', 'F', 'I' };

	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	uint8_t id [4];

	(*nand->command)(bed, BED_NAND_CMD_READ_ID, 0x20, 0);
	(*nand->read_buffer)(bed, id, sizeof(id));
	if (memcmp(id, onfi_id, sizeof(id)) == 0) {
		bed_nand_onfi *onfi = &nand->onfi;
		bool crc_ok = false;
		int i;

		(*nand->command)(bed, BED_NAND_CMD_READ_PARAMETER_PAGE, 0, 0);

		for (i = 0; !crc_ok && i < 3; ++i) {
			(*nand->read_buffer)(bed, (uint8_t *) onfi, sizeof(*onfi));
			crc_ok = check_onfi_crc(onfi);
		}

		if (crc_ok) {
			uint16_t revision = bed_le16_to_cpu(onfi->revision);

			if ((revision & 0x3e) != 0) {
				uint32_t bus_width_flag = (bed_le16_to_cpu(onfi->features) & 0x1) != 0 ?
					BED_NAND_FLG_BUS_WIDTH_16 : 0;

				bed->blocks_per_chip = bed_le32_to_cpu(onfi->blocks_per_lun) * onfi->lun_count;
				bed->page_size = (uint16_t) bed_le32_to_cpu(onfi->data_bytes_per_page);
				bed->block_size = bed_le32_to_cpu(onfi->pages_per_block) * bed->page_size;
				bed->oob_size = bed_le16_to_cpu(onfi->spare_bytes_per_page);

				if ((nand->flags & BED_NAND_FLG_BUS_WIDTH_16) != bus_width_flag) {
					status = BED_ERROR_BUS_WIDTH;
				}
			} else {
				status = BED_ERROR_ONFI_REVISION;
			}
		} else {
			status = BED_ERROR_ONFI_CRC;
		}
	} else {
		status = BED_ERROR_NO_ONFI_DEVICE;
	}

	return status;
}

static bed_status micron_internal_ecc_read_page(
	bed_device *bed,
	uint8_t *data,
	bed_oob_mode mode
)
{
	bed_status status;
	bed_nand_context *nand = bed->context;

	status = (*nand->boxed_read_page)(bed, data, BED_OOB_MODE_RAW);
	if (status == BED_SUCCESS) {
		uint8_t nand_status = read_status(bed);

		(*nand->command)(bed, BED_NAND_CMD_READ_MODE, 0, 0);

		if (mode == BED_OOB_MODE_AUTO) {
			if ((nand_status & (BED_NAND_STATUS_READY | BED_NAND_STATUS_FAIL)) != BED_NAND_STATUS_READY) {
				status = BED_ERROR_ECC_UNCORRECTABLE;
			} else if ((nand_status & BED_NAND_STATUS_MICRON_REWRITE_RECOMMENDED) != 0) {
				status = BED_ERROR_ECC_FIXED;
			}
		}
	}

	return status;
}

#ifndef BED_CONFIG_READ_ONLY
static bed_status micron_internal_ecc_write_page(
	bed_device *bed,
	const uint8_t *data,
	bed_oob_mode mode
)
{
	bed_status status;

	if (mode == BED_OOB_MODE_AUTO) {
		bed_nand_context *nand = bed->context;

		status = (*nand->boxed_write_page)(bed, data, BED_OOB_MODE_RAW);
	} else {
		status = BED_ERROR_OOB_MODE;
	}

	return status;
}
#endif /* BED_CONFIG_READ_ONLY */

static bool micron_has_internal_ecc(const uint8_t *id)
{
	return (id [4] & 0x3) == 0x2;
}

static bool micron_is_internal_ecc_enabled(const uint8_t *id)
{
	return (id [4] & 0x80) != 0;
}

static bed_status micron_check_for_internal_ecc(bed_device *bed)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;

	if (nand->id [0] == BED_NAND_MFR_MICRON && micron_has_internal_ecc(nand->id)) {
		if (bed->page_size == 2048) {
			bool use_internal_ecc = nand->onfi.bits_ecc_correctability
				> nand->ecc_correctable_bits_per_512_bytes;
			uint8_t buf [5] = { use_internal_ecc ? 0x8 : 0x0, 0x0, 0x0, 0x0 };

			(*nand->command)(bed, BED_NAND_CMD_SET_FEATURES, 0x90, 0);
			(*nand->write_buffer)(bed, buf, 4);
			(*nand->command)(bed, BED_NAND_CMD_SET_FEATURES_2, 0, 0);

			get_chip_id(bed, buf, 5);

			if (use_internal_ecc) {
				if (micron_is_internal_ecc_enabled(buf)) {
					bed->oob_free_size = 16;
					bed->ecc_covers_oob = 1;
					nand->ecc_correctable_bits_per_512_bytes = 4;
					nand->oob_free_ranges = bed_micron_oob_free_ranges_64;
					nand->oob_ecc_ranges = NULL;
					nand->boxed_read_page = nand->read_page;
					nand->read_page = micron_internal_ecc_read_page;
#ifndef BED_CONFIG_READ_ONLY
					nand->boxed_write_page = nand->write_page;
					nand->write_page = micron_internal_ecc_write_page;
#endif /* BED_CONFIG_READ_ONLY */
				} else {
					status = BED_ERROR_ECC_MICRON_INTERNAL;
				}
			} else if (micron_is_internal_ecc_enabled(buf)) {
				status = BED_ERROR_ECC_MICRON_INTERNAL;
			}
		} else {
			status = BED_ERROR_ECC_MICRON_INTERNAL;
		}
	}

	return status;
}

static bed_status finalize_detection(bed_device *bed)
{
	bed_nand_context *nand = bed->context;

	bed_set_geometry_parameters(bed);

	if (bed_nand_has_large_pages(bed) && nand->command == bed_nand_command) {
		nand->command = bed_nand_command_large_pages;
	}

	bed_nand_set_bad_block_marker_position(bed);
	bed_nand_set_default_oob_layout(bed);

	return micron_check_for_internal_ecc(bed);
}

static bed_status evaluate_id(bed_device *bed, const uint8_t *id)
{
	bed_status status = BED_SUCCESS;
	const bed_nand_context *nand = bed->context;
	int cell_info = id [2];
	int layout_info = id [3];
	uint32_t bus_width_flag = 0;

	/* Special case for some Samsung chips */
	if (
		id [0] == BED_NAND_MFR_SAMSUNG
			&& id [0] == id [6]
			&& id [1] == id [7]
			&& id [5] != 0x00
			&& (cell_info & 0x0c) != 0
	) {
		bed->page_size = (uint16_t) (2048U << (layout_info & 0x03));

		switch ((layout_info >> 2) & 0x03) {
			case 1:
				bed->oob_size = 128;
				break;
			case 2:
				bed->oob_size = 218;
				break;
			case 3:
				bed->oob_size = 400;
				break;
			default:
				bed->oob_size = 436;
				break;
		}

		bed->block_size = (128U * 1024U)
			<< (((layout_info >> 5) & 0x04) | ((layout_info >> 4) & 0x03));
	} else {
		bed->page_size = (uint16_t) (1024U << (layout_info & 0x03));
		bed->oob_size = (uint16_t) ((8U << ((layout_info >> 2) & 0x01))
			* (bed->page_size >> 9));
		bed->block_size = (64U * 1024U) << ((layout_info >> 4) & 0x03);
		bus_width_flag = (layout_info & 0x40) != 0 ?  BED_NAND_FLG_BUS_WIDTH_16 : 0;
	}

	if ((nand->flags & BED_NAND_FLG_BUS_WIDTH_16) != bus_width_flag) {
		status = BED_ERROR_BUS_WIDTH;
	}

	return status;
}

static bed_status detect_chip(
	bed_device *bed,
	const bed_nand_device_info *device_info
)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	uint8_t second_try_id [2];

	assert((nand->bbc.flags & (BED_NAND_BBC_CHECK_SECOND_PAGE | BED_NAND_BBC_CHECK_LAST_PAGE))
		!= (BED_NAND_BBC_CHECK_SECOND_PAGE | BED_NAND_BBC_CHECK_LAST_PAGE));

	(*nand->command)(bed, BED_NAND_CMD_RESET, 0, 0);

	get_chip_id(bed, nand->id, sizeof(nand->id));
	get_chip_id(bed, second_try_id, sizeof(second_try_id));

	if (memcmp(nand->id, second_try_id, sizeof(second_try_id)) == 0) {
		bool device_info_available = false;
		uint16_t page_size;

		while (!device_info_available && !bed_nand_device_is_terminal(device_info)) {
			device_info_available = device_info->id == nand->id [1];

			if (!device_info_available) {
				++device_info;
			}
		}

		page_size = bed_nand_device_page_size(device_info);

		if (!device_info_available || page_size == 0) {
			status = detect_onfi_chip(bed);
			if (status != BED_SUCCESS && device_info_available) {
				status = evaluate_id(bed, nand->id);
				bed->blocks_per_chip = bed_nand_device_chip_size(device_info)
					>> bed_power_of_two(bed->block_size);
			}
		} else if (device_info_available) {
			uint32_t block_size = bed_nand_device_block_size(device_info);
			int block_shift = bed_power_of_two(block_size);

			bed->page_size = page_size;
			bed->block_size = block_size;
			bed->blocks_per_chip = bed_nand_device_chip_size(device_info) >> block_shift;
			bed->oob_size = bed->page_size / 32;
		} else {
			status = BED_ERROR_NO_DEVICE;
		}

		if (status == BED_SUCCESS) {
			status = finalize_detection(bed);
		}
	} else {
		status = BED_ERROR_NO_DEVICE;
	}

	return status;
}

bed_status bed_nand_detect(
	bed_device *bed,
	uint16_t chip_count,
	const bed_nand_device_info *device_info
)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;

	bed->current_chip = 0xffff;
	bed_select_chip(bed, 0);

	status = detect_chip(
		bed,
		device_info
	);
	if (status == BED_SUCCESS) {
		uint16_t i;

		for (i = 1; i < chip_count; ++i) {
			uint8_t other_id [2];

			bed_select_chip(bed, i);

			get_chip_id(bed, other_id, sizeof(other_id));

			if (memcmp(nand->id, other_id, sizeof(nand->id)) != 0) {
				break;
			}
		}

		bed->chip_count = i;
	}

	return status;
}

bed_status bed_nand_detect_finalize(bed_device *bed)
{
	bed_status status = BED_SUCCESS;

	return status;
}
