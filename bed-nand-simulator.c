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
#include <stdlib.h>
#include <inttypes.h>

#define ECC_CHUNK_SIZE 256

#define OOB_CHUNK_SIZE 8

typedef enum {
	IDLE,
	EXPECT_NONE,
	ADDR_BYTE,
	ADDR_COL_0,
	ADDR_COL_1,
	ADDR_ROW_0,
	ADDR_ROW_1,
	ADDR_ROW_2,
	READ_ID_CHECK_ADDR,
	READ_PAGE_2,
	PROGRAM_PAGE,
	PROGRAM_PAGE_2,
	ERASE_BLOCK_2
} nand_sim_state;

typedef enum {
	SIM_IO_DATA,
	SIM_IO_ID,
	SIM_IO_ID_ONFI,
	SIM_IO_PARAM,
	SIM_IO_STATUS,
	SIM_IO_UNDEFINED
} nand_sim_io_mode;

typedef struct {
	nand_sim_state state;
	nand_sim_state next_state;
	nand_sim_io_mode io_mode;
	uint32_t page;
	uint16_t column;
	uint16_t page_with_oob_size;
	size_t pages_per_chip;
	size_t ecc_chunks;
	uint8_t id [8];
	bed_nand_onfi onfi;
	uint8_t *area;
} nand_sim_context;

#ifndef NDEBUG
static bool is_cmd(int ctrl)
{
	return (ctrl & (BED_NAND_CTRL_ALE | BED_NAND_CTRL_CLE))
		== (BED_NAND_CTRL_CLE);
}

static bool is_addr(int ctrl)
{
	return (ctrl & (BED_NAND_CTRL_ALE | BED_NAND_CTRL_CLE))
		== (BED_NAND_CTRL_ALE);
}
#endif

static void expect_none_state(nand_sim_context *sim, nand_sim_state next)
{
	sim->state = EXPECT_NONE;
	sim->next_state = next;
}

static void start_sequence(bed_device *bed, int data, int ctrl)
{
	bed_nand_context *nand = bed->context;
	nand_sim_context *sim = nand->context;

	if (bed_nand_is_real_data(data)) {
		assert(is_cmd(ctrl));

		sim->io_mode = SIM_IO_UNDEFINED;
		sim->column = 0;
		sim->page = 0;

		switch (data) {
			case BED_NAND_CMD_READ_OOB:
			case BED_NAND_CMD_READ_PAGE:
				if (data == BED_NAND_CMD_READ_OOB) {
					sim->column = 512;
				}
				sim->io_mode = SIM_IO_DATA;
				sim->state = ADDR_COL_0;
				if (bed_nand_has_large_pages(bed)) {
					sim->next_state = READ_PAGE_2;
				} else {
					sim->next_state = IDLE;
				}
				break;
			case BED_NAND_CMD_POINTER_OOB:
				sim->column = 512;
				sim->io_mode = SIM_IO_DATA;
				sim->state = PROGRAM_PAGE;
				sim->next_state = PROGRAM_PAGE_2;
				break;
			case BED_NAND_CMD_PROGRAM_PAGE:
				sim->io_mode = SIM_IO_DATA;
				sim->state = ADDR_COL_0;
				sim->next_state = PROGRAM_PAGE_2;
				break;
			case BED_NAND_CMD_ERASE_BLOCK:
				sim->state = ADDR_ROW_0;
				sim->next_state = ERASE_BLOCK_2;
				break;
			case BED_NAND_CMD_READ_ID:
				sim->state = ADDR_BYTE;
				sim->next_state = READ_ID_CHECK_ADDR;
				break;
			case BED_NAND_CMD_READ_PARAMETER_PAGE:
				sim->io_mode = SIM_IO_PARAM;
				sim->state = ADDR_BYTE;
				sim->next_state = IDLE;
				break;
			case BED_NAND_CMD_READ_STATUS:
				sim->io_mode = SIM_IO_STATUS;
				expect_none_state(sim, IDLE);
				break;
			case BED_NAND_CMD_RESET:
				expect_none_state(sim, IDLE);
				break;
			default:
				assert(0);
				break;
		}
	} else {
		sim->state = IDLE;
	}
}

static uint32_t get_page(const bed_device *bed, const nand_sim_context *sim)
{
	return bed->current_chip * sim->pages_per_chip + sim->page;
}

static uint8_t *get_page_data(const bed_device *bed, const nand_sim_context *sim, uint32_t page)
{
	return sim->area + page * sim->page_with_oob_size;
}

static uint8_t *get_page_oob(const bed_device *bed, const nand_sim_context *sim, uint32_t page)
{
	return get_page_data(bed, sim, page) + bed->page_size;
}

static void erase_block(bed_device *bed)
{
	const bed_nand_context *nand = bed->context;
	const nand_sim_context *sim = nand->context;
	uint32_t page = get_page(bed, sim);
	uint8_t *nand_data_and_oob = get_page_data(bed, sim, page);

	assert(page % bed->pages_per_block == 0);

	memset(nand_data_and_oob, 0xff, (size_t) bed->pages_per_block * sim->page_with_oob_size);
}

static void nand_sim_control(bed_device *bed, int data, int ctrl)
{
	bed_nand_context *nand = bed->context;
	nand_sim_context *sim = nand->context;
	int value = data & BED_NAND_CMD_MASK;

	switch (sim->state) {
		case IDLE:
			start_sequence(bed, data, ctrl);
			break;
		case EXPECT_NONE:
			assert((unsigned) data == BED_NAND_CMD_NONE);
			sim->state = sim->next_state;
			break;
		case ADDR_BYTE:
			assert(is_addr(ctrl));
			sim->column = (uint16_t) (sim->column + value);
			sim->state = sim->next_state;
			break;
		case ADDR_COL_0:
			assert(is_addr(ctrl));
			sim->column = (uint16_t) (sim->column + value);
			if (bed_nand_has_large_pages(bed)) {
				sim->state = ADDR_COL_1;
			} else {
				sim->state = ADDR_ROW_0;
			}
			break;
		case ADDR_COL_1:
			assert(is_addr(ctrl));
			sim->column = (uint16_t) (sim->column + (value << 8));
			sim->state = ADDR_ROW_0;
			break;
		case ADDR_ROW_0:
			assert(is_addr(ctrl));
			sim->page += (uint32_t) value;
			sim->state = ADDR_ROW_1;
			break;
		case ADDR_ROW_1:
			assert(is_addr(ctrl));
			sim->page += ((uint32_t) value) << 8;
			if (bed->chip_shift - bed->page_shift <= 16) {
				expect_none_state(sim, sim->next_state);
			} else {
				sim->state = ADDR_ROW_2;
			}
			break;
		case ADDR_ROW_2:
			assert(is_addr(ctrl));
			sim->page += ((uint32_t) value) << 16;
			expect_none_state(sim, sim->next_state);
			break;
		case READ_ID_CHECK_ADDR:
			assert((unsigned) data == BED_NAND_CMD_NONE);
			switch (sim->column) {
				case 0x00:
					sim->io_mode = SIM_IO_ID;
					break;
				case 0x20:
					sim->io_mode = SIM_IO_ID_ONFI;
					break;
				default:
					assert(0);
					break;
			}
			sim->column = 0;
			sim->state = IDLE;
			break;
		case READ_PAGE_2:
			assert(data == BED_NAND_CMD_READ_PAGE_2);
			expect_none_state(sim, IDLE);
			break;
		case PROGRAM_PAGE:
			assert(data == BED_NAND_CMD_PROGRAM_PAGE);
			sim->state = ADDR_COL_0;
			break;
		case PROGRAM_PAGE_2:
			assert(data == BED_NAND_CMD_PROGRAM_PAGE_2);
			expect_none_state(sim, IDLE);
			break;
		case ERASE_BLOCK_2:
			assert(data == BED_NAND_CMD_ERASE_BLOCK_2);
			erase_block(bed);
			expect_none_state(sim, IDLE);
			break;
		default:
			assert(0);
			break;
	}
}

static bool nand_sim_is_ready(bed_device *bed)
{
	return true;
}

static void nand_sim_read_buffer(bed_device *bed, uint8_t *data, size_t n)
{
	static const uint8_t onfi [] = { 'O', 'N', 'F', 'I' };
	static const uint8_t nand_status [] = { BED_NAND_STATUS_READY };

	bed_nand_context *nand = bed->context;
	nand_sim_context *sim = nand->context;
	const uint8_t *nand_data;
	size_t size_max;

	switch (sim->io_mode) {
		case SIM_IO_DATA: {
			uint32_t page = get_page(bed, sim);

			nand_data = get_page_data(bed, sim, page);
			size_max = sim->page_with_oob_size;

			break;
		}
		case SIM_IO_ID:
			nand_data = sim->id;
			size_max = sizeof(sim->id);
			break;
		case SIM_IO_ID_ONFI:
			nand_data = onfi;
			size_max = sizeof(onfi);
			break;
		case SIM_IO_PARAM:
			if (sim->column == sizeof(sim->onfi)) {
				sim->column = 0;
			}
			nand_data = (const uint8_t *) &sim->onfi;
			size_max = sizeof(sim->onfi);
			break;
		default:
			assert(sim->io_mode == SIM_IO_STATUS);

			sim->column = 0;
			nand_data = nand_status;
			size_max = sizeof(nand_status);
			break;
	}

	(void) size_max;
	assert(sim->column < size_max);
	assert(n <= size_max - sim->column);

	memcpy(data, nand_data + sim->column, n);

	sim->column = (uint16_t) (sim->column + n);
}

static uint8_t nand_sim_read_8(bed_device *bed)
{
	uint8_t value;

	nand_sim_read_buffer(bed, &value, sizeof(value));

	return value;
}

static uint16_t nand_sim_read_16(bed_device *bed)
{
	uint16_t value;

	nand_sim_read_buffer(bed, (uint8_t *) &value, sizeof(value));

	return value;
}

static bed_status nand_sim_read_page(bed_device *bed, uint8_t *data, bed_oob_mode mode)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	nand_sim_context *sim = nand->context;
	uint32_t page = get_page(bed, sim);
	const uint8_t *nand_data = get_page_data(bed, sim, page);
	const uint8_t *nand_oob = get_page_oob(bed, sim, page);
	const uint8_t *nand_ecc = nand_oob + nand->oob_ecc_ranges [0].offset;
	uint8_t *oob = nand->oob_buffer;
	size_t i;

	assert(sim->io_mode == SIM_IO_DATA);

	memcpy(data, nand_data, ECC_CHUNK_SIZE * sim->ecc_chunks);
	memcpy(oob, nand_oob, OOB_CHUNK_SIZE * sim->ecc_chunks);

	if (mode == BED_OOB_MODE_AUTO) {
		for (i = 0; status == BED_SUCCESS && i < sim->ecc_chunks; ++i) {
			uint8_t calc_ecc [BED_ECC_HAMMING_256_SIZE];

			bed_ecc_hamming_256_calculate(data, calc_ecc);
			status = bed_ecc_hamming_256_correct(data, nand_ecc, calc_ecc);

			data += ECC_CHUNK_SIZE;
			nand_ecc += BED_ECC_HAMMING_256_SIZE;
		}
	}

	return status;
}

static void nand_sim_memcpy(uint8_t *dst, const uint8_t *src, size_t n)
{
	size_t i;

	for (i = 0; i < n; ++i) {
		dst [i] &= src [i];
	}
}

static void nand_sim_write_buffer(bed_device *bed, const uint8_t *data, size_t n)
{
	bed_nand_context *nand = bed->context;
	nand_sim_context *sim = nand->context;
	uint32_t page = get_page(bed, sim);
	uint8_t *nand_data = get_page_data(bed, sim, page);
	size_t size_max = sim->page_with_oob_size;

	assert(sim->io_mode == SIM_IO_DATA);

	(void) size_max;
	assert(sim->column < size_max);
	assert(n <= size_max - sim->column);

	nand_sim_memcpy(nand_data + sim->column, data, n);

	sim->column = (uint16_t) (sim->column + n);
}

#ifndef BED_CONFIG_READ_ONLY
static bed_status nand_sim_write_page(bed_device *bed, const uint8_t *data, bed_oob_mode mode)
{
	bed_status status = BED_SUCCESS;
	bed_nand_context *nand = bed->context;
	nand_sim_context *sim = nand->context;
	uint32_t page = get_page(bed, sim);
	uint8_t *nand_data = get_page_data(bed, sim, page);
	uint8_t *nand_oob = get_page_oob(bed, sim, page);
	uint8_t *nand_ecc = nand_oob + nand->oob_ecc_ranges [0].offset;
	uint8_t *oob = nand->oob_buffer;
	size_t i;

	assert(sim->io_mode == SIM_IO_DATA);

	nand_sim_memcpy(nand_data, data, ECC_CHUNK_SIZE * sim->ecc_chunks);
	nand_sim_memcpy(nand_oob, oob, OOB_CHUNK_SIZE * sim->ecc_chunks);

	if (mode == BED_OOB_MODE_AUTO) {
		for (i = 0; status == BED_SUCCESS && i < sim->ecc_chunks; ++i) {
			bed_ecc_hamming_256_calculate(nand_data, nand_ecc);

			nand_data += ECC_CHUNK_SIZE;
			nand_ecc += BED_ECC_HAMMING_256_SIZE;
		}
	}

	return status;
}
#endif /* BED_CONFIG_READ_ONLY */

static void chip_detected(bed_device *bed)
{
	bed_nand_context *nand = bed->context;
	nand_sim_context *sim = nand->context;

	sim->ecc_chunks = bed->page_size / ECC_CHUNK_SIZE;
	sim->page_with_oob_size = (uint16_t) (bed->page_size + bed->oob_size);
	sim->pages_per_chip = bed->blocks_per_chip * bed->pages_per_block;
}

bed_partition *bed_nand_simulator_create(
	uint16_t chip_count,
	uint32_t blocks_per_chip,
	uint32_t block_size,
	uint16_t page_size
)
{
	bed_partition *part = NULL;
	uint32_t pages_per_block = block_size / page_size;
	uint32_t ecc_chunks = page_size / ECC_CHUNK_SIZE;
	uint32_t oob_size = ecc_chunks * OOB_CHUNK_SIZE;
	uint32_t area_size = chip_count * blocks_per_chip * pages_per_block * (page_size + oob_size);
	bed_device *bed;
	bed_nand_context *nand;
	nand_sim_context *sim;
	uint8_t *chunk;

	assert(bed_is_power_of_two(blocks_per_chip));
	assert(bed_is_power_of_two(page_size));
	assert(page_size % ECC_CHUNK_SIZE == 0);

	chunk = malloc(
		sizeof(*part)
			+ sizeof(*bed)
			+ sizeof(*nand)
			+ sizeof(*sim)
			+ area_size
	);

	if (chunk != NULL) {
		bed_status status = BED_SUCCESS;
		bed_nand_onfi *onfi;

		part = (bed_partition *) chunk;
		memset(part, 0, sizeof(*part));
		chunk += sizeof(*part);

		bed = (bed_device *) chunk;
		memset(bed, 0, sizeof(*bed));
		chunk += sizeof(*bed);

		nand = (bed_nand_context *) chunk;
		memset(nand, 0, sizeof(*nand));
		chunk += sizeof(*nand);
		bed->context = nand;

		sim = (nand_sim_context *) chunk;
		memset(sim, 0, sizeof(*sim));
		chunk += sizeof(*sim);
		nand->context = sim;

		bed->obtain = bed_mutex_obtain;
		bed->release = bed_mutex_release;
		bed->select_chip = bed_default_select_chip;
		bed->is_block_valid = bed_nand_is_block_valid;
		bed->read = bed_nand_read;
		bed->read_oob = bed_nand_read_oob;
#ifndef BED_CONFIG_READ_ONLY
		bed->write = bed_nand_write;
		bed->write_oob = bed_nand_write_oob;
		bed->erase = bed_nand_erase;
		bed->mark_block_bad = bed_nand_mark_block_bad;
#endif /* BED_CONFIG_READ_ONLY */
		bed->ecc_covers_oob = 1;

		bed->chip_count = chip_count;

		onfi = &sim->onfi;
		onfi->revision = 0x3e;
		onfi->data_bytes_per_page = bed_cpu_to_le32(page_size);
		onfi->spare_bytes_per_page = bed_cpu_to_le16((uint16_t) oob_size);
		onfi->pages_per_block = bed_cpu_to_le32(pages_per_block);
		onfi->blocks_per_lun = bed_cpu_to_le32(blocks_per_chip);
		onfi->lun_count = 1;
		onfi->crc = bed_cpu_to_le16(bed_nand_onfi_crc(onfi));

		nand->command = bed_nand_command;
		nand->control = nand_sim_control;
		nand->is_ready = nand_sim_is_ready;
		nand->read_8 = nand_sim_read_8;
		nand->read_16 = nand_sim_read_16;
		nand->read_buffer = nand_sim_read_buffer;
		nand->write_buffer = nand_sim_write_buffer;
		nand->read_oob_only = bed_nand_read_oob_only;
		nand->read_page = nand_sim_read_page;
#ifndef BED_CONFIG_READ_ONLY
		nand->write_page = nand_sim_write_page;
		nand->mark_page_bad = bed_nand_mark_page_bad;
#endif /* BED_CONFIG_READ_ONLY */
		nand->ecc_correctable_bits_per_512_bytes = 2;

		sim->area = chunk;
		memset(sim->area, 0xff, area_size);

		if (status == BED_SUCCESS) {
			status = bed_mutex_initialize(bed);
		}

		if (status == BED_SUCCESS) {
			status = bed_nand_detect(bed, chip_count, bed_nand_device_info_all);
		}

		if (status == BED_SUCCESS) {
			chip_detected(bed);

			status = bed_nand_detect_finalize(bed);

			part->bed = bed;
			part->size = bed->size;
		}

		if (status != BED_SUCCESS) {
			free(part);
			part = NULL;
		}
	}

	return part;
}

void bed_nand_simulator_destroy(bed_partition *part)
{
	free(part);
}
