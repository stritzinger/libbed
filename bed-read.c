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

void bed_default_select_chip(bed_device *bed, uint16_t chip)
{
	bed->current_chip = chip;
}

void bed_obtain(const bed_partition *part)
{
	bed_device *bed = part->bed;

	(*bed->obtain)(bed);
}

void bed_release(const bed_partition *part)
{
	bed_device *bed = part->bed;

	(*bed->release)(bed);
}

bed_status bed_read(const bed_partition *part, bed_address addr, void *data, size_t n)
{
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	if (bed_is_range_valid(part, addr, n)) {
		if (n > 0) {
			(*bed->obtain)(bed);
			status = (*bed->read)(bed, part->begin + addr, data, n);
			(*bed->release)(bed);
		}
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}
