/**
 * @file
 *
 * @ingroup BEDImplNOR
 *
 * @brief BED NOR API.
 */

/*
 * Copyright (c) 2013 embedded brains GmbH.  All rights reserved.
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

#ifndef BED_NOR_H
#define BED_NOR_H

#include "bed.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDImplNOR BED NOR
 *
 * @ingroup BEDImpl
 *
 * @{
 */

bed_partition *bed_nor_simulator_create(
	uint32_t block_count,
	uint32_t block_size
);

void bed_nor_simulator_destroy(bed_partition *part);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_NOR_H */
