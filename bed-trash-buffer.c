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

#include "bed-nand.h"

#include <assert.h>

#include <bsp.h>

void *bed_trash_buffer(size_t n)
{
#ifdef LIBBSP_ARM_LPC32XX_BSP_H
	assert(n < LPC32XX_SCRATCH_AREA_SIZE);

	return lpc32xx_scratch_area;
#else
	static uint8_t trash_buffer [BED_NAND_MAX_PAGE_SIZE];

	assert(n < BED_NAND_MAX_PAGE_SIZE);

	return trash_buffer;
#endif
}
