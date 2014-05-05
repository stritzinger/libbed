/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this fileay be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include "bed-impl.h"

static bed_device bed_null_device = {
#ifndef BED_CONFIG_READ_ONLY
	.write = bed_write_not_supported,
	.write_oob = bed_write_oob_not_supported,
	.erase = bed_erase_not_supported,
	.mark_block_bad = bed_mark_block_bad_not_supported,
#endif /* BED_CONFIG_READ_ONLY */
	.obtain = bed_default_obtain,
	.release = bed_default_release,
	.select_chip = bed_default_select_chip,
	.is_block_valid = bed_is_block_valid_not_supported,
	.read = bed_read_not_supported,
	.read_oob = bed_read_oob_not_supported
};

const bed_partition bed_null_partition = {
	.bed = &bed_null_device
};
