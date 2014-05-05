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

const bed_nand_range bed_nand_oob_free_ranges_16 [] = {
	{ 0, 4 },
	{ 6, 4 },
	{ 0, 0 }
};

const bed_nand_range bed_nand_oob_ecc_ranges_16 [] = {
	{ 10, 6 },
	{ 0, 0 }
};

const bed_nand_range bed_nand_oob_free_ranges_64 [] = {
	{ 2, 38 },
	{ 0, 0 }
};

const bed_nand_range bed_nand_oob_ecc_ranges_64 [] = {
	{ 40, 24 },
	{ 0, 0 }
};

const bed_nand_range bed_nand_oob_free_ranges_128 [] = {
	{ 2, 78 },
	{ 0, 0 }
};

const bed_nand_range bed_nand_oob_ecc_ranges_128 [] = {
	{ 80, 48 },
	{ 0, 0 }
};

const bed_nand_range bed_micron_oob_free_ranges_64 [] = {
	{ 4, 4 },
	{ 20, 4 },
	{ 36, 4 },
	{ 52, 4 },
	{ 0, 0 }
};

void bed_nand_set_default_oob_layout(bed_device *bed)
{
	bed_nand_context *nand = bed->context;

	switch (bed->oob_size) {
		case 16:
			bed->oob_free_size = 8;
			nand->oob_free_ranges = bed_nand_oob_free_ranges_16;
			nand->oob_ecc_ranges = bed_nand_oob_ecc_ranges_16;
			break;
		case 64:
			bed->oob_free_size = 38;
			nand->oob_free_ranges = bed_nand_oob_free_ranges_64;
			nand->oob_ecc_ranges = bed_nand_oob_ecc_ranges_64;
			break;
		default:
			bed->oob_free_size = 78;
			nand->oob_free_ranges = bed_nand_oob_free_ranges_128;
			nand->oob_ecc_ranges = bed_nand_oob_ecc_ranges_128;
			break;
	}
}
