/**
 * @file
 *
 * @ingroup BEDImplNAND
 *
 * @brief BED NAND API.
 */

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

#ifndef BED_NAND_H
#define BED_NAND_H

#include "bed-impl.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDImplNAND BED NAND
 *
 * @ingroup BEDImpl
 *
 * @{
 */

#define BED_NAND_STATUS_FAIL 0x01
#define BED_NAND_STATUS_FAIL_N_MINUS_1 0x02
#define BED_NAND_STATUS_MICRON_REWRITE_RECOMMENDED 0x04
#define BED_NAND_STATUS_READY 0x40
#define BED_NAND_STATUS_WP 0x80

/* Command latch enable */
#define BED_NAND_CTRL_CLE 0x02

/* Address latch enable */
#define BED_NAND_CTRL_ALE 0x04

#define BED_NAND_CMD_MASK 0xff

#define BED_NAND_CMD_WAIT_FOR_READY 0x8000

#define BED_NAND_CMD_ADDR_BYTE 0x4000

#define BED_NAND_CMD_ADDR_COLUMN 0x2000

#define BED_NAND_CMD_ADDR_ROW 0x1000

#define BED_NAND_CMD_VIRTUAL 0x80000000

typedef enum {
	BED_NAND_CMD_NONE = BED_NAND_CMD_VIRTUAL,
	BED_NAND_CMD_RESET = (0xff | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_READ_ID = (0x90 | BED_NAND_CMD_ADDR_BYTE | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_READ_PARAMETER_PAGE = (0xec | BED_NAND_CMD_ADDR_BYTE | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_READ_UNIQUE_ID = (0xed | BED_NAND_CMD_ADDR_BYTE | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_GET_FEATURES = (0xee | BED_NAND_CMD_ADDR_BYTE | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_SET_FEATURES = (0xef | BED_NAND_CMD_ADDR_BYTE),
	BED_NAND_CMD_SET_FEATURES_2 = (0xef | BED_NAND_CMD_VIRTUAL | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_READ_STATUS = (0x70 | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_RANDOM_DATA_READ = (0x05 | BED_NAND_CMD_ADDR_COLUMN),
	BED_NAND_CMD_RANDOM_DATA_READ_2 = 0xe0,
	BED_NAND_CMD_RANDOM_DATA_INPUT = (0x85 | BED_NAND_CMD_ADDR_COLUMN),
	BED_NAND_CMD_READ_MODE = 0x00,
	BED_NAND_CMD_READ_OOB = (0x50 | BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_READ_PAGE = (0x00 | BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW | BED_NAND_CMD_WAIT_FOR_READY),
	BED_NAND_CMD_READ_PAGE_2 = 0x30,
	BED_NAND_CMD_READ_PAGE_CACHE_SEQUENTIAL = 0x31,
	BED_NAND_CMD_READ_PAGE_CACHE_RANDOM = (0x00 | BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW),
	BED_NAND_CMD_READ_PAGE_CACHE_RANDOM_2 = 0x31,
	BED_NAND_CMD_READ_PAGE_CACHE_LAST = 0x3f,
	BED_NAND_CMD_POINTER_OOB = 0x50,
	BED_NAND_CMD_PROGRAM_PAGE = (0x80 | BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW),
	BED_NAND_CMD_PROGRAM_PAGE_2 = 0x10,
	BED_NAND_CMD_PROGRAM_PAGE_CACHE = (0x80 | BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW),
	BED_NAND_CMD_PROGRAM_PAGE_CACHE_2 = 0x15,
	BED_NAND_CMD_ERASE_BLOCK = (0x60 | BED_NAND_CMD_ADDR_ROW),
	BED_NAND_CMD_ERASE_BLOCK_2 = 0xd0,
	BED_NAND_CMD_READ_FOR_INTERNAL_DATA_MOVE = (0x00 | BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW),
	BED_NAND_CMD_READ_FOR_INTERNAL_DATA_MOVE_2 = 0x35,
	BED_NAND_CMD_PROGRAM_FOR_INTERNAL_DATA_MOVE = (0x85 | BED_NAND_CMD_ADDR_COLUMN | BED_NAND_CMD_ADDR_ROW),
	BED_NAND_CMD_PROGRAM_FOR_INTERNAL_DATA_MOVE_2 = 0x10
} bed_nand_cmd;

static inline bool bed_nand_is_real_data(int data)
{
	return ((unsigned) data & BED_NAND_CMD_VIRTUAL) == 0;
}

#define BED_NAND_MFR_SPANSION 0x01
#define BED_NAND_MFR_RAYTEON 0x92
#define BED_NAND_MFR_FUJITSU 0x04
#define BED_NAND_MFR_HYNIX 0xad
#define BED_NAND_MFR_MACRONIX 0xc2
#define BED_NAND_MFR_MICRON 0x2c
#define BED_NAND_MFR_NATIONAL 0x8f
#define BED_NAND_MFR_RENESAS 0x07
#define BED_NAND_MFR_SAMSUNG 0xec
#define BED_NAND_MFR_ST 0x20
#define BED_NAND_MFR_TOSHIBA 0x98

typedef struct bed_nand_context bed_nand_context;

typedef void (*bed_nand_command_method)(bed_device *bed, bed_nand_cmd cmd, uint32_t row_or_byte, uint16_t column);

typedef void (*bed_nand_control_method)(bed_device *bed, int data, int ctrl);

typedef bool (*bed_nand_is_ready_method)(bed_device *bed);

typedef uint8_t (*bed_nand_read_8_method)(bed_device *bed);

typedef uint16_t (*bed_nand_read_16_method)(bed_device *bed);

typedef void (*bed_nand_read_buffer_method)(bed_device *bed, uint8_t *data, size_t n);

typedef bed_status (*bed_nand_read_oob_only_method)(bed_device *bed, uint32_t page);

typedef bed_status (*bed_nand_read_page_method)(bed_device *bed, uint8_t *data, bool use_ecc);

typedef void (*bed_nand_write_buffer_method)(bed_device *bed, const uint8_t *data, size_t n);

typedef bed_status (*bed_nand_write_page_method)(bed_device *bed, const uint8_t *data, bool use_ecc);

typedef bed_status (*bed_nand_mark_page_bad_method)(bed_device *bed, uint32_t page);

#define BED_NAND_BBC_CHECK_SECOND_PAGE 0x1

#define BED_NAND_BBC_CHECK_LAST_PAGE 0x2

typedef struct {
	uint16_t flags;
	uint16_t marker_position;
} bed_nand_bad_block_control;

#define BED_NAND_FLG_BUS_WIDTH_16 0x1

#define BED_NAND_MAX_OOB_SIZE 576

#define BED_NAND_MAX_PAGE_SIZE 8192

typedef struct {
	uint16_t offset;
	uint16_t size;
} bed_nand_range;

extern const bed_nand_range bed_nand_oob_free_ranges_16 [];

extern const bed_nand_range bed_nand_oob_ecc_ranges_16 [];

extern const bed_nand_range bed_nand_oob_free_ranges_64 [];

extern const bed_nand_range bed_nand_oob_ecc_ranges_64 [];

extern const bed_nand_range bed_nand_oob_free_ranges_128 [];

extern const bed_nand_range bed_nand_oob_ecc_ranges_128 [];

extern const bed_nand_range bed_micron_oob_free_ranges_64 [];

typedef struct {
	/* Revision information and features block */
	uint8_t signature [4];
	uint16_t revision;
	uint16_t features;
	uint16_t optional_commands;
	uint8_t reserved_10 [2];
	uint16_t extended_parameter_page_length;
	uint16_t parameter_pages;
	uint8_t reserved_15 [16];

	/* Manufacturer information block */
	char device_manufacturer [12];
	char device_model [20];
	uint8_t jedec_manufacturer_id;
	uint16_t date_code;
	uint8_t reserved_67 [13];

	/* Memory organization block */
	uint32_t data_bytes_per_page;
	uint16_t spare_bytes_per_page;
	uint32_t data_bytes_per_partial_page;
	uint16_t spare_bytes_per_partial_page;
	uint32_t pages_per_block;
	uint32_t blocks_per_lun;
	uint8_t lun_count;
	uint8_t address_cycles;
	uint8_t bits_per_cell;
	uint16_t max_bad_blocks_per_lun;
	uint16_t block_endurance;
	uint8_t guaranteed_valid_blocks;
	uint16_t block_endurance_for_guaranteed_blocks;
	uint8_t programs_per_page;
	uint8_t partial_programming_attributes;
	uint8_t bits_ecc_correctability;
	uint8_t interleaved_address_bits;
	uint8_t interleaved_operation_attributes;
	uint8_t reserved_115 [13];

	/* Electrical parameters block */
	uint8_t max_io_pin_capacitance;
	uint16_t async_timing_mode;
	uint16_t async_program_cache_timing_mode;
	uint16_t t_prog;
	uint16_t t_bers;
	uint16_t t_r;
	uint16_t t_ccs;
	uint16_t src_sync_timing_mode;
	uint16_t src_sync_features;
	uint16_t typical_clk_pin_capacitance;
	uint16_t typical_io_pin_capacitance;
	uint16_t typical_input_pin_capacitance;
	uint8_t max_input_pin_capacitance;
	uint8_t driver_strength_support;
	uint16_t t_r_interleaved;
	uint16_t t_ald;
	uint8_t reserved_156 [7];

	/* Vendor block */
	uint16_t vendor_specific_revision;
	uint8_t reserved_166 [88];
	uint16_t crc;
} __attribute__((packed)) bed_nand_onfi;

struct bed_nand_context {
	uint8_t oob_buffer [BED_NAND_MAX_OOB_SIZE] __attribute__((aligned(8)));
	bed_nand_command_method command;
	bed_nand_control_method control;
	bed_nand_is_ready_method is_ready;
	bed_nand_read_8_method read_8;
	bed_nand_read_16_method read_16;
	bed_nand_read_buffer_method read_buffer;
	bed_nand_write_buffer_method write_buffer;
	bed_nand_read_oob_only_method read_oob_only;
	bed_nand_read_page_method read_page;
#ifndef BED_CONFIG_READ_ONLY
	bed_nand_write_page_method write_page;
	bed_nand_mark_page_bad_method mark_page_bad;
#endif /* BED_CONFIG_READ_ONLY */
        void *context;
	uint32_t flags;
	const bed_nand_range *oob_free_ranges;
	const bed_nand_range *oob_ecc_ranges;
	bed_nand_bad_block_control bbc;
	uint8_t id [8];
	bed_nand_onfi onfi;
	uint8_t ecc_correctable_bits_per_512_bytes;
	bed_nand_read_page_method boxed_read_page;
#ifndef BED_CONFIG_READ_ONLY
	bed_nand_write_page_method boxed_write_page;
#endif /* BED_CONFIG_READ_ONLY */
};

void bed_nand_set_default_oob_layout(bed_device *bed);

void bed_nand_wait_for_ready(bed_device *bed);

bed_status bed_nand_check_status(bed_device *bed, bed_status error_status);

uint16_t bed_nand_onfi_crc(const bed_nand_onfi *onfi);

void bed_nand_command(bed_device *bed, bed_nand_cmd cmd, uint32_t row_or_byte, uint16_t column);

void bed_nand_command_large_pages(bed_device *bed, bed_nand_cmd cmd, uint32_t row_or_byte, uint16_t column);

bed_status bed_nand_read_oob_only(bed_device *bed, uint32_t page);

bed_status bed_nand_read_oob_only_with_trash(bed_device *bed, uint32_t page);

bed_status bed_nand_mark_page_bad(bed_device *bed, uint32_t page);

#define bed_nand_mark_page_bad_not_supported \
	((bed_nand_mark_page_bad_method) bed_op_not_supported)

bed_status bed_nand_read(
	bed_device *bed,
	bed_address addr,
	void *data,
	size_t n
);

bed_status bed_nand_read_oob(
	bed_device *bed,
	bed_address addr,
	void *data,
	size_t n,
	const bed_oob_request *oob
);

bed_status bed_nand_write(
	bed_device *bed,
	bed_address addr,
	const void *data,
	size_t n
);

bed_status bed_nand_write_oob(
	bed_device *bed,
	bed_address addr,
	const void *data,
	size_t n,
	const bed_oob_request *oob
);

bed_status bed_nand_erase(bed_device *bed, bed_address addr);

bed_status bed_nand_is_block_valid(bed_device *bed, bed_address addr);

bed_status bed_nand_mark_block_bad(bed_device *bed, bed_address addr);

typedef struct {
	uint32_t id : 8;

	/**
	 * page_size = 2 ** (page_size_code + 7), 0 = invalid
	 */
	uint32_t page_size_code : 5;

	/**
	 * block_size = 2 ** (block_size_code + 11), 0 = invalid
	 */
	uint32_t block_size_code : 5;

	/**
	 * chip_size = 2 ** (chip_size_code + 19), 0 = invalid
	 */
	uint32_t chip_size_code : 5;

	uint32_t bus_width_16 : 1;
} bed_nand_device_info;

extern const bed_nand_device_info bed_nand_device_info_8_bit_1_8_V [];

extern const bed_nand_device_info bed_nand_device_info_8_bit_3_3_V [];

extern const bed_nand_device_info bed_nand_device_info_16_bit_1_8_V [];

extern const bed_nand_device_info bed_nand_device_info_16_bit_3_3_V [];

extern const bed_nand_device_info bed_nand_device_info_all [];

static inline uint16_t bed_nand_device_page_size(
	const bed_nand_device_info *info
)
{
	int page_size_code = info->page_size_code;

	return (uint16_t) (page_size_code != 0 ? (1 << (page_size_code + 7)) : 0);
}

static inline uint32_t bed_nand_device_block_size(
	const bed_nand_device_info *info
)
{
	int block_size_code = info->block_size_code;
	uint32_t one = 1;

	return block_size_code != 0 ? (one << (block_size_code + 11)) : 0;
}

static inline bed_address bed_nand_device_chip_size(
	const bed_nand_device_info *info
)
{
	int chip_size_code = info->chip_size_code;
	bed_address one = 1;

	return chip_size_code != 0 ? (one << (chip_size_code + 19)) : 0;
}

static inline bool bed_nand_device_is_terminal(
	const bed_nand_device_info *info
)
{
	static const bed_nand_device_info terminal = { 0, 0, 0, 0 , 0 };

	return memcmp(info, &terminal, sizeof(terminal)) == 0;
}

bed_status bed_nand_detect(
	bed_device *bed,
	uint16_t chip_count,
	const bed_nand_device_info *device_info
);

bed_status bed_nand_detect_finalize(bed_device *bed);

bed_partition *bed_nand_simulator_create(
	uint16_t chip_count,
	uint32_t blocks_per_chip,
	uint32_t block_size,
	uint16_t page_size
);

void bed_nand_simulator_destroy(bed_partition *part);

static inline bool bed_nand_has_large_pages(const bed_device *bed)
{
	return bed->page_size > 512;
}

static inline void bed_nand_set_bad_block_marker_position(bed_device *bed)
{
	bed_nand_context *nand = (bed_nand_context *) bed->context;

	if (bed_nand_has_large_pages(bed) || (nand->flags & BED_NAND_FLG_BUS_WIDTH_16) != 0) {
		nand->bbc.marker_position = 0;
	} else {
		nand->bbc.marker_position = 5;
	}
}

static inline bool bed_nand_needs_3_page_cycles(const bed_device *bed)
{
	return bed->chip_shift - bed->page_shift > 16;
}

static inline uint8_t bed_nand_manufacturer_id(const bed_partition *part)
{
	const bed_device *bed = (const bed_device *) part->bed;
	const bed_nand_context *nand = (const bed_nand_context *) bed->context;

	return nand->id [0];
}

static inline uint8_t bed_nand_model_id(const bed_partition *part)
{
	const bed_device *bed = (const bed_device *) part->bed;
	const bed_nand_context *nand = (const bed_nand_context *) bed->context;

	return nand->id [1];
}

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_NAND_H */
