/**
 * @file
 *
 * @ingroup BEDImplTest
 *
 * @brief BED Test API.
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

#ifndef BED_TEST_H
#define BED_TEST_H

#include "bed.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDImplTest BED Test
 *
 * @ingroup BEDImpl
 *
 * @{
 */

bed_status bed_test_if_erased(
	const bed_partition *part,
	uint32_t *page_buffer,
	uint8_t *oob_buffer
);

bed_status bed_test_write_and_read(
	const bed_partition *part,
	uint32_t *page_value,
	uint8_t *oob_value,
	uint32_t *page_buffer,
	uint8_t *oob_buffer
);

typedef struct {
	uint32_t erase_counter;
	uint32_t erase_errors;
	uint32_t write_counter;
	uint32_t write_errors;
	uint32_t read_ecc_fixed;
	uint32_t read_ecc_uncorrectable;
	uint32_t read_other_errors;
	uint32_t read_data_errors;
	uint32_t read_oob_errors;
} bed_test_mark_block_bad_stats;

/**
 * @brief Makes a block bad.
 *
 * This test erases and writes block 0 of the partition until a write or erase
 * error occurs.
 */
void bed_test_make_block_bad(
	const bed_partition *part,
	uint32_t *page_buffer_0,
	uint8_t *oob_buffer_0,
	uint32_t *page_buffer_1,
	uint8_t *oob_buffer_1,
	bed_test_mark_block_bad_stats *stats
);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_TEST_H */
