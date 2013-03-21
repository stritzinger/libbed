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

#include "bed-impl.h"

bed_status bed_is_block_valid(const bed_partition *part, bed_address addr)
{
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	if (bed_is_address_valid(part, addr)) {
		(*bed->obtain)(bed);
		status = (*bed->is_block_valid)(bed, part->begin + addr);
		(*bed->release)(bed);
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}
