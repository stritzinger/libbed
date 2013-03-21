/**
 * @file
 *
 * @ingroup BEDImpl
 *
 * @brief BED Implementation API.
 */

/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
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

#ifndef BED_IMPL_H
#define BED_IMPL_H

#include "bed.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDImpl Block Erasable Devices Implementation
 *
 * @{
 */

static inline void bed_select_chip(bed_device *bed, uint16_t chip)
{
	if (bed->current_chip != chip) {
		bed->current_chip = chip;
		(*bed->select_chip)(bed, chip);
	}
}

bed_status bed_device_erase(
	bed_device *bed,
	bed_address addr,
	bed_erase_mode mode
);

void *bed_trash_buffer(size_t n);

extern const char bed_ones_per_byte_table [];

static inline int bed_ones_per_byte(uint8_t byte)
{
	return bed_ones_per_byte_table [byte];
}

static inline bool bed_is_power_of_two(bed_address addr)
{
	return addr != 0 && (addr & (addr - 1)) == 0;
}

static inline int bed_power_of_two(bed_address addr)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
	return __builtin_ffsl(addr) - 1;
#pragma GCC diagnostic pop
}

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_LITTLE_ENDIAN__)
	#error "endianess not defined"
#endif

static inline uint16_t bed_cpu_to_le16(uint16_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return val;
#else
	return __builtin_bswap16(val);
#endif
}

static inline uint32_t bed_cpu_to_le32(uint32_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return val;
#else
	return __builtin_bswap32(val);
#endif
}

static inline uint16_t bed_le16_to_cpu(uint16_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return val;
#else
	return __builtin_bswap16(val);
#endif
}

static inline uint32_t bed_le32_to_cpu(uint32_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return val;
#else
	return __builtin_bswap32(val);
#endif
}

static inline bool bed_is_block_aligned(
	const bed_device *bed,
	bed_address addr
)
{
	return (addr & (bed->block_size - 1)) == 0;
}

static inline bool bed_is_address_valid(
	const bed_partition *part,
	bed_address addr
)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
	return addr >= 0 && addr < part->size;
#pragma GCC diagnostic pop
}

static inline bool bed_is_range_valid(
	const bed_partition *part,
	bed_address addr,
	bed_address size
)
{
	return bed_is_address_valid(part, addr) && size <= part->size - addr;
}

static inline bool bed_is_oob_request_valid(
	const bed_device *bed,
	const bed_oob_request *oob
)
{
	uint16_t oob_free_size = (uint16_t) (oob->mode == BED_OOB_MODE_AUTO ?
		bed->oob_free_size : bed->oob_size);

	return oob->offset + oob->size <= oob_free_size;
}

static inline void bed_set_geometry_parameters(bed_device *bed)
{
	bed->page_shift = (uint8_t) bed_power_of_two(bed->page_size);
	assert(1U << bed->page_shift == bed->page_size);
	bed->block_shift = (uint8_t) bed_power_of_two(bed->block_size);
	assert(1U << bed->block_shift == bed->block_size);
	bed->chip_shift = (uint8_t) bed_power_of_two(bed->blocks_per_chip * bed->block_size);
	assert(1U << bed->chip_shift == bed->blocks_per_chip * bed->block_size);

	bed->pages_per_block = (uint16_t) (1U << (bed->block_shift - bed->page_shift));
	bed->page_mask = (1U << (bed->chip_shift - bed->page_shift)) - 1;

	bed->size = bed->block_size;
	bed->size *= bed->blocks_per_chip;
	bed->size *= bed->chip_count;
}

/** @} */

/**
 * @defgroup BEDImplDefaultMethods BED Default Methods
 *
 * @ingroup BEDImpl
 *
 * @{
 */

bed_status bed_op_not_supported(void);

void bed_default_select_chip(bed_device *bed, uint16_t chip);

#define bed_default_release \
	((bed_release_method) bed_op_not_supported)

#define bed_default_obtain \
	((bed_obtain_method) bed_op_not_supported)

#define bed_read_not_supported \
	((bed_read_method) bed_op_not_supported)

#define bed_read_oob_not_supported \
	((bed_read_oob_method) bed_op_not_supported)

#define bed_write_not_supported \
	((bed_write_method) bed_op_not_supported)

#define bed_write_oob_not_supported \
	((bed_write_oob_method) bed_op_not_supported)

#define bed_erase_not_supported \
	((bed_erase_method) bed_op_not_supported)

#define bed_is_block_valid_not_supported \
	((bed_is_block_valid_method) bed_op_not_supported)

#define bed_mark_block_bad_not_supported \
	((bed_mark_block_bad_method) bed_op_not_supported)

/** @} */

/**
 * @defgroup BEDECCHamming256 BED ECC Hamming 256
 *
 * @ingroup BEDImpl
 *
 * ECC layout:
 * <table>
 *   <tr>
 *     <th></th><th>Bit 7</th><th>Bit 6</th><th>Bit 5</th><th>Bit 4</th>
 *     <th>Bit 3</th><th>Bit 2</th><th>Bit 1</th><th>Bit 0</th>
 *   </tr>
 *   <tr>
 *     <td>ECC[0]</td><td>RP7</td><td>RP6</td><td>RP5</td><td>RP4</td>
 *     <td>RP3</td><td>RP2</td><td>RP1</td><td>RP0</td>
 *   </tr>
 *   <tr>
 *     <td>ECC[1]</td><td>RP15</td><td>RP14</td><td>RP13</td><td>RP12</td>
 *     <td>RP11</td><td>RP10</td><td>RP9</td><td>RP8</td>
 *   </tr>
 *   <tr>
 *     <td>ECC[2]</td><td>CP5</td><td>CP4</td><td>CP3</td><td>CP2</td>
 *     <td>CP1</td><td>CP0</td><td>1</td><td>1</td>
 *   </tr>
 * </table>
 *
 * @{
 */

#define BED_ECC_HAMMING_256_SIZE 3

void bed_ecc_hamming_256_calculate(const void *data, uint8_t *ecc);

bed_status bed_ecc_hamming_256_correct(
	void *data,
	const uint8_t *read_ecc,
	const uint8_t *calc_ecc
);

/** @} */

/**
 * @defgroup BEDMutex BED Mutex Support
 *
 * @ingroup BEDImpl
 *
 * @{
 */

bed_status bed_mutex_initialize(bed_device *bed);

void bed_mutex_obtain(bed_device *bed);

void bed_mutex_release(bed_device *bed);

/** @} */ 

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_IMPL_H */
