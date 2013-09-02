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

#include "bed-impl.h"

bed_status bed_op_not_supported(void)
{
	return BED_ERROR_OP_NOT_SUPPORTED;
}

bed_status bed_read_oob(
	const bed_partition *part,
	bed_address addr,
	void *data,
	size_t n,
	const bed_oob_request *oob
)
{
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	if (bed_is_range_valid(part, addr, n) && bed_is_oob_request_valid(bed, oob)) {
		if (n > 0) {
			(*bed->obtain)(bed);
			status = (*bed->read_oob)(bed, part->begin + addr, data, n, oob);
			(*bed->release)(bed);
		}
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}
