/**
 * @file
 *
 * @ingroup BEDYAFFS
 *
 * @brief YAFFS Support Implementation
 */

/*
 * Copyright (c) 2012-2013 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "bed-yaffs.h"

#include <yaffs/rtems_yaffs.h>
#include <yaffs/yaffs_guts.h>
#include <yaffs/yaffs_packedtags2.h>

#include <rtems/imfs.h>

#define BED_YAFFS_STATIC_ASSERT(cond, msg) \
	typedef int bed_yaffs_static_assert_ ## msg [(cond) ? 1 : -1]

BED_YAFFS_STATIC_ASSERT(
	YAFFS_ECC_RESULT_UNKNOWN < YAFFS_ECC_RESULT_NO_ERROR,
	ecc_result_0
);

BED_YAFFS_STATIC_ASSERT(
	YAFFS_ECC_RESULT_NO_ERROR < YAFFS_ECC_RESULT_FIXED,
	ecc_result_1
);

BED_YAFFS_STATIC_ASSERT(
	YAFFS_ECC_RESULT_FIXED < YAFFS_ECC_RESULT_UNFIXED,
	ecc_result_2
);

typedef struct {
	const bed_partition *part;
	int pages_per_chunk;
	int nand_chunk_shift;
	int oob_size;
	rtems_yaffs_os_context os_context;
} bed_yaffs_context;

static void bed_yaffs_lock(struct yaffs_dev *dev, void *arg)
{
        const bed_yaffs_context *self = dev->driver_context;

        bed_obtain(self->part);
}

static void bed_yaffs_unlock(struct yaffs_dev *dev, void *arg)
{
        const bed_yaffs_context *self = dev->driver_context;

        bed_release(self->part);
}

static void bed_yaffs_unmount(struct yaffs_dev *dev, void *arg)
{
        bed_yaffs_context *self = dev->driver_context;

	free(self);
}

static bed_address bed_yaffs_nand_chunk_to_address(
	const bed_yaffs_context *self,
	int nand_chunk
)
{
	return ((bed_address) nand_chunk) << self->nand_chunk_shift;
}

static int bed_yaffs_write_chunk_tags(
	struct yaffs_dev *dev,
	int nand_chunk,
	const u8 *data,
	const struct yaffs_ext_tags *tags
)
{
	int yc = YAFFS_OK;
	bed_yaffs_context *self = dev->driver_context;
	const bed_partition *part = self->part;
	size_t page_size = bed_page_size(part);
	int oob_free_size = bed_oob_free_size(part);
	bed_address addr = bed_yaffs_nand_chunk_to_address(self, nand_chunk);
	struct yaffs_packed_tags2 packed_tags;
	uint8_t *oob_data = (uint8_t *) &packed_tags;
	int oob_size = self->oob_size;
	int i;

	assert(data != NULL && tags != NULL);

	yaffs_pack_tags2(&packed_tags, tags, !dev->param.no_tags_ecc);

	for (i = 0; yc == YAFFS_OK && i < self->pages_per_chunk; ++i) {
		bed_status status;
		int current_oob_size = oob_size < oob_free_size ?
			oob_size : oob_free_size;
		bed_oob_request oob = {
			.mode = BED_OOB_MODE_AUTO,
			.offset = 0,
			.size = (uint16_t) current_oob_size,
			.data = oob_data
		};

		status = bed_write_oob(part, addr, data, page_size, &oob);
		if (status != BED_SUCCESS) {
			yc = YAFFS_FAIL;
		}

		addr += page_size;
		data += page_size;
		oob_data += current_oob_size;
		oob_size -= current_oob_size;
	}

	return yc;
}

static int bed_yaffs_read_chunk_tags(
	struct yaffs_dev *dev,
	int nand_chunk,
	u8 *data,
	struct yaffs_ext_tags *tags
)
{
	enum yaffs_ecc_result ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
	bed_yaffs_context *self = dev->driver_context;
	const bed_partition *part = self->part;
	size_t page_size = bed_page_size(part);
	size_t data_size = data != NULL ? page_size : 0;
	int oob_free_size = bed_oob_free_size(part);
	bed_address addr = bed_yaffs_nand_chunk_to_address(self, nand_chunk);
	struct yaffs_packed_tags2 packed_tags;
	uint8_t *oob_data = (uint8_t *) &packed_tags;
	int oob_size = self->oob_size;
	int i;

	assert(tags != NULL);

	for (i = 0; i < self->pages_per_chunk; ++i) {
		bed_status status;
		int current_oob_size = oob_size < oob_free_size ?
			oob_size : oob_free_size;
		bed_oob_request oob = {
			.mode = BED_OOB_MODE_AUTO,
			.offset = 0,
			.size = (uint16_t) current_oob_size,
			.data = oob_data
		};

		status = bed_read_oob(part, addr, data, data_size, &oob);
		if (status == BED_ERROR_ECC_FIXED) {
			if (ecc_result == YAFFS_ECC_RESULT_NO_ERROR) {
				ecc_result = YAFFS_ECC_RESULT_FIXED;
			}
		} else if (status != BED_SUCCESS) {
			ecc_result = YAFFS_ECC_RESULT_UNFIXED;
		}

		addr += page_size;
		data += data_size;
		oob_data += current_oob_size;
		oob_size -= current_oob_size;
	}

	if (tags != NULL) {
		yaffs_unpack_tags2(tags, &packed_tags, !dev->param.no_tags_ecc);

		if (tags->ecc_result > ecc_result) {
			ecc_result = tags->ecc_result;
		}

		tags->ecc_result = ecc_result;
	}

	return ecc_result == YAFFS_ECC_RESULT_NO_ERROR ? YAFFS_OK : YAFFS_FAIL;
}

static int bed_yaffs_bad_block(struct yaffs_dev *dev, int blockId)
{
	bed_status status;
	bed_yaffs_context *self = dev->driver_context;
	const bed_partition *part = self->part;
	bed_address addr = bed_block_to_address(part, (bed_address) blockId);

	status = bed_mark_block_bad(part, addr);

	return status == BED_SUCCESS ? YAFFS_OK : YAFFS_FAIL;
}

static int bed_yaffs_query_block(
	struct yaffs_dev *dev,
	int blockId,
	enum yaffs_block_state *state, u32 *seq_number
)
{
	int yc = YAFFS_OK;
	bed_status status;
	bed_yaffs_context *self = dev->driver_context;
	const bed_partition *part = self->part;
	bed_address addr = bed_block_to_address(part, (bed_address) blockId);

	*state = YAFFS_BLOCK_STATE_UNKNOWN;
	*seq_number = 0;

	status = bed_is_block_valid(part, addr);
	if (status == BED_SUCCESS) {
		int nand_chunk = blockId * dev->param.chunks_per_block;
		struct yaffs_ext_tags tags;

		yc = bed_yaffs_read_chunk_tags(dev, nand_chunk, NULL, &tags);
		if (yc == YAFFS_OK) {
			if (tags.chunk_used) {
				*state = YAFFS_BLOCK_STATE_NEEDS_SCAN;
				*seq_number = tags.seq_number;
			} else {
				*state = YAFFS_BLOCK_STATE_EMPTY;
			}
		}
	} else {
		assert(status == BED_ERROR_BLOCK_IS_BAD);
		*state = YAFFS_BLOCK_STATE_DEAD;
	}

	return yc;
}

static int bed_yaffs_erase(struct yaffs_dev *dev, int blockId)
{
	bed_status status;
	bed_yaffs_context *self = dev->driver_context;
	const bed_partition *part = self->part;
	bed_address addr = bed_block_to_address(part, (bed_address) blockId);

	status = bed_erase(part, addr, BED_ERASE_MARK_BAD_ON_ERROR);

	return status == BED_SUCCESS ? YAFFS_OK : YAFFS_FAIL;
}

static int bed_yaffs_initialise(struct yaffs_dev *dev)
{
	return YAFFS_OK;
}

bed_status bed_yaffs_initialize_device(
	const bed_partition *part,
	struct yaffs_dev *dev
)
{
	bed_status status = BED_SUCCESS;
	bed_yaffs_context *self = NULL;

	self = malloc(sizeof(*self));
	if (self != NULL) {
		struct yaffs_param *param = &dev->param;
		int oob_size = bed_ecc_covers_oob(part) ?
			sizeof(struct yaffs_packed_tags2_tags_only)
				: sizeof(struct yaffs_packed_tags2);
		int oob_free_size = bed_oob_free_size(part);
		int pages_per_chunk = 1;
		int nand_chunk_shift = bed_page_shift(part);

		memset(dev, 0, sizeof(*dev));

		dev->os_context = &self->os_context;
		dev->driver_context = self;

		while (oob_free_size < oob_size) {
			oob_free_size *= 2;
			pages_per_chunk *= 2;
			++nand_chunk_shift;
		}

		self->os_context.lock = bed_yaffs_lock;
		self->os_context.unlock = bed_yaffs_unlock;
		self->os_context.unmount = bed_yaffs_unmount;
		self->os_context.dev = rtems_filesystem_make_dev_t(
			IMFS_GENERIC_DEVICE_MAJOR_NUMBER,
			(rtems_device_minor_number) part
		);

		self->part = part;
		self->pages_per_chunk = pages_per_chunk;
		self->nand_chunk_shift = nand_chunk_shift;
		self->oob_size = oob_size;

		param->name = "NAND";
		param->start_block = 0;
		param->end_block =
			(int) bed_address_to_block(part, bed_size(part)) - 1;
		param->total_bytes_per_chunk =
			(u32) (pages_per_chunk * bed_page_size(part));
		param->spare_bytes_per_chunk =
			pages_per_chunk * bed_oob_size(part);
		param->chunks_per_block = (int) (bed_block_size(part)
				/ param->total_bytes_per_chunk);
		param->n_reserved_blocks = 5;
		param->n_caches = 15;
		param->inband_tags = 0;
		param->is_yaffs2 = 1;
		param->no_tags_ecc = bed_ecc_covers_oob(part);

		param->write_chunk_tags_fn = bed_yaffs_write_chunk_tags;
		param->read_chunk_tags_fn = bed_yaffs_read_chunk_tags;
		param->bad_block_fn = bed_yaffs_bad_block;
		param->query_block_fn = bed_yaffs_query_block;
		param->erase_fn = bed_yaffs_erase;
		param->initialise_flash_fn = bed_yaffs_initialise;
	} else {
		status = BED_ERROR_SYSTEM;
	}

	return status;
}
