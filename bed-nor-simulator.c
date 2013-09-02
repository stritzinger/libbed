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

#include "bed-nor.h"
#include "bed-impl.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	bed_partition part;
	bed_device device;
	uint8_t area[];
} nor_sim_context;

static nor_sim_context *nor_sim_get_context(const bed_device *bed)
{
	return bed->context;
}

static bed_status nor_sim_is_block_valid(bed_device *bed, bed_address addr)
{
	return BED_SUCCESS;
}

static bed_status nor_sim_read(bed_device *bed, bed_address addr, void *data, size_t n)
{
	nor_sim_context *ctx = nor_sim_get_context(bed);

	memcpy(data, &ctx->area[addr], n);

	return BED_SUCCESS;
}

#ifndef BED_CONFIG_READ_ONLY
static bed_status nor_sim_write(bed_device *bed, bed_address addr, const void *data, size_t n)
{
	nor_sim_context *ctx = nor_sim_get_context(bed);
	const uint8_t *in = data;
	uint8_t *out = &ctx->area[addr];
	size_t i;

	for (i = 0; i < n; ++i) {
		out[i] &= in[i];
	}

	return BED_SUCCESS;
}

static bed_status nor_sim_erase(bed_device *bed, bed_address addr)
{
	bed_status status = BED_SUCCESS;

	if (bed_is_block_aligned(bed, addr)) {
		nor_sim_context *ctx = nor_sim_get_context(bed);

		memset(&ctx->area[addr], 0xff, bed->block_size);
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}
#endif /* BED_CONFIG_READ_ONLY */

bed_partition *bed_nor_simulator_create(
	uint32_t block_count,
	uint32_t block_size
)
{
	bed_partition *part = NULL;
	uint32_t area_size = block_count * block_size;
	nor_sim_context *sim;

	assert(bed_is_power_of_two(block_count));
	assert(bed_is_power_of_two(block_size));

	sim = malloc(sizeof(*sim) + area_size);
	if (sim != NULL) {
		bed_device *bed;

		memset(sim, 0, sizeof(*sim));
		memset(&sim->area[0], 0xff, area_size);

		bed = &sim->device;
		bed->context = sim;
		bed->obtain = bed_default_obtain;
		bed->release = bed_default_release;
		bed->select_chip = bed_default_select_chip;
		bed->is_block_valid = nor_sim_is_block_valid;
		bed->read = nor_sim_read;
		bed->read_oob = bed_read_oob_not_supported;
#ifndef BED_CONFIG_READ_ONLY
		bed->write = nor_sim_write;
		bed->write_oob = bed_write_oob_not_supported;
		bed->erase = nor_sim_erase;
		bed->mark_block_bad = bed_mark_block_bad_not_supported;
#endif /* BED_CONFIG_READ_ONLY */
		bed->page_size = 1;
		bed->block_size = block_size;
		bed->blocks_per_chip = block_count;
		bed->chip_count = 1;

		bed_set_geometry_parameters(bed);

		part = &sim->part;
		part->bed = bed;
		part->size = bed->size;
	}

	return part;
}

void bed_nor_simulator_destroy(bed_partition *part)
{
	free(part);
}
