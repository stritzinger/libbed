/**
 * @file
 *
 * @ingroup BED
 *
 * @brief BED API.
 */

/*
 * Copyright (c) 2012-2014 embedded brains GmbH.  All rights reserved.
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

#ifndef BED_H
#define BED_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct bed_device bed_device;

/**
 * @defgroup BED Block Erasable Devices
 *
 * @{
 */

/**
 * @brief Byte address of a block erasable device.
 */
typedef uint32_t bed_address;

/**
 * @brief Area of a block erasable device.
 */
typedef struct {
	/**
	 * @brief Byte address of the area begin.
	 */
	bed_address begin;

	/**
	 * @brief Size of the area in bytes.
	 */
	bed_address size;
} bed_area;

/**
 * @brief Block erasable device status code.
 */
typedef enum {
	BED_SUCCESS,
	BED_ERROR_BLOCK_IS_BAD,
	BED_ERROR_INVALID_ADDRESS,
	BED_ERROR_WRITE,
	BED_ERROR_ERASE,
	BED_ERROR_ECC_FIXED,
	BED_ERROR_ECC_UNCORRECTABLE,
	BED_ERROR_ECC_MICRON_INTERNAL,
	BED_ERROR_NO_DEVICE,
	BED_ERROR_NO_ONFI_DEVICE,
	BED_ERROR_ONFI_CRC,
	BED_ERROR_ONFI_REVISION,
	BED_ERROR_BUS_WIDTH,
	BED_ERROR_SYSTEM,
	BED_ERROR_UNSATISFIED,
	BED_ERROR_OP_NOT_SUPPORTED,
	BED_ERROR_READ_ONLY,
	BED_ERROR_OOB_MODE,
	BED_ERROR_STOPPED
} bed_status;

/**
 * @brief Out-of-bounds (OOB) mode.
 */
typedef enum {
	/**
	 * @brief Automatically place OOB data.
	 *
	 * This generates also the ECC for the OOB data.
	 */
	BED_OOB_MODE_AUTO,

	/**
	 * @brief Place out-of-bounds data according to offset and size.
	 *
	 * This may not generate the ECC for the OOB data.
	 */
	BED_OOB_MODE_RAW
} bed_oob_mode;

/**
 * @brief Out-of-bounds (OOB) request.
 */
typedef struct {
	/**
	 * @brief OOB mode.
	 */
	bed_oob_mode mode;

	/**
	 * @brief Byte offset in the OOB area for the data.
	 */
	uint16_t offset;

	/**
	 * @brief Size of the data in bytes.
	 */
	uint16_t size;

	/**
	 * @brief Data for OOB area.
	 */
	uint8_t *data;
} bed_oob_request;

/**
 * @brief Erase mode.
 */
typedef enum {
	/**
	 * @brief Do not erase bad blocks and do not mark block bad on erase
	 * error.
	 */
	BED_ERASE_NORMAL,

	/**
	 * @brief Erase independent of the current block state and do not mark
	 * block bad on erase error.
	 */
	BED_ERASE_FORCE,

	/**
	 * @brief Do not erase bad blocks and mark block bad on erase error.
	 */
	BED_ERASE_MARK_BAD_ON_ERROR
} bed_erase_mode;

/**
 * @brief Block erasable device partition.
 */
typedef struct {
	/**
	 * @brief Block erasable device for this partition.
	 */
	bed_device *bed;

	/**
	 * @brief Begin byte address of this partition.
	 */
	bed_address begin;

	/**
	 * @brief Size of this partition in bytes.
	 */
	bed_address size;
} bed_partition;

/**
 * @brief Creates a new partition.
 *
 * The new partition will not reference the parent partition afterwards.
 *
 * @param[out] child The new partition.
 * @param[in] parent The parent partition.  The new partition will inherit its
 * block erasable device.
 * @param[in] begin The byte address of the new partition begin relative to the
 * parent partition.
 * @param[in] size The new partition size in bytes.
 *
 * @retval BED_SUCCESS Successful operation.
 * @retval BED_ERROR_INVALID_ADDRESS The new partition area is invalid.
 */
bed_status bed_partition_create(
	bed_partition *child,
	const bed_partition *parent,
	bed_address begin,
	bed_address size
);

void bed_obtain(const bed_partition *part);

void bed_release(const bed_partition *part);

bed_status bed_read(const bed_partition *part, bed_address addr, void *data, size_t n);

bed_status bed_read_oob(const bed_partition *part, bed_address addr, void *data, size_t n, const bed_oob_request *oob);

bed_status bed_write(const bed_partition *part, bed_address addr, const void *data, size_t n);

bed_status bed_write_oob(const bed_partition *part, bed_address addr, const void *data, size_t n, const bed_oob_request *oob);

bed_status bed_erase(
	const bed_partition *part,
	bed_address addr,
	bed_erase_mode mode
);

bed_status bed_erase_all(const bed_partition *part, bed_erase_mode mode);

bed_status bed_is_block_valid(const bed_partition *part, bed_address addr);

bed_status bed_mark_block_bad(const bed_partition *part, bed_address addr);

bed_status bed_write_with_skip(
	const bed_partition *part,
	const void *data,
	size_t n,
	void *page_buffer
);

/**
 * @brief Read with skip process function.
 *
 * @see bed_read_with_skip().
 *
 * @retval false Continue processing.
 * @retval true Stop processing.
 */
typedef bool (*bed_read_process)(
  void *process_arg,
  bed_address addr,
  void *data,
  size_t n,
  void *oob,
  size_t m
);

/**
 * @brief Reads the valid pages of a partition and skips bad blocks.
 *
 * The pages are read with ECC correction turned on (BED_OOB_MODE_AUTO).
 *
 * @param[in] part The partition.
 * @param[in] process The page process function.
 * @param[in] process_arg The argument for the page process function.
 * @param[in] page_buffer Buffer to store the page content.  It must be large
 * enough for the pages of this partition.
 * @param[in] oob_buffer Buffer to store the OOB content.  It must be large
 * enough for the OOB areas of this partition.
 *
 * @retval BED_SUCCESS Successful operation.
 * @retval BED_ERROR_STOPPED The process function requested a stop.
 * @retval BED_ERROR_ECC_UNCORRECTABLE Uncorrectable ECC error during a page
 * read.
 * @retval other Other error status codes depending on the driver.
 */
bed_status bed_read_with_skip(
	const bed_partition *part,
	bed_read_process process,
	void *process_arg,
	void *page_buffer,
	void *oob_buffer
);

/**
 * @brief Read all process function.
 *
 * @see bed_read_all().
 *
 * @retval false Continue processing.
 * @retval true Stop processing.
 */
typedef bool (*bed_read_all_process)(
  void *process_arg,
  bed_address addr,
  bed_status is_block_valid_status,
  bed_status page_read_status,
  void *data,
  size_t n,
  void *oob,
  size_t m
);

/**
 * @brief Reads all pages of a partition.
 *
 * @param[in] part The partition.
 * @param[in] oob_mode The OOB mode used to read the pages.
 * @param[in] process The page process function.
 * @param[in] process_arg The argument for the page process function.
 * @param[in] page_buffer Buffer to store the page content.  It must be large
 * enough for the pages of this partition.
 * @param[in] oob_buffer Buffer to store the OOB content.  It must be large
 * enough for the OOB areas of this partition.
 *
 * @retval BED_SUCCESS Successful operation.
 * @retval BED_ERROR_STOPPED The process function requested a stop.
 */
bed_status bed_read_all(
	const bed_partition *part,
	bed_oob_mode oob_mode,
	bed_read_all_process process,
	void *process_arg,
	void *page_buffer,
	void *oob_buffer
);

extern const bed_partition bed_null_partition;

typedef int (*bed_printer)(void *arg, const char *fmt, ...)
	__attribute__((__format__(__printf__, 2, 3)));

int bed_vprintf_printer(void *arg, const char *fmt, ...);

void bed_print_bad_blocks(const bed_partition *part, bed_printer printer, void *printer_arg);

/** @} */

typedef void (*bed_obtain_method)(bed_device *bed);
typedef void (*bed_release_method)(bed_device *bed);
typedef void (*bed_select_chip_method)(bed_device *bed, uint16_t chip);
typedef bed_status (*bed_is_block_valid_method)(bed_device *bed, bed_address addr);
typedef bed_status (*bed_read_method)(bed_device *bed, bed_address addr, void *data, size_t n);
typedef bed_status (*bed_read_oob_method)(bed_device *bed, bed_address addr, void *data, size_t n, const bed_oob_request *oob);
typedef bed_status (*bed_write_method)(bed_device *bed, bed_address addr, const void *data, size_t n);
typedef bed_status (*bed_write_oob_method)(bed_device *bed, bed_address addr, const void *data, size_t n, const bed_oob_request *oob);
typedef bed_status (*bed_erase_method)(bed_device *bed, bed_address addr);
typedef bed_status (*bed_mark_block_bad_method)(bed_device *bed, bed_address addr);

/**
 * @brief Block erasable device.
 *
 * @addtogroup BEDImpl
 */
struct bed_device {
	bed_obtain_method obtain;
	bed_release_method release;
	bed_select_chip_method select_chip;
	bed_is_block_valid_method is_block_valid;
	bed_read_method read;
	bed_read_oob_method read_oob;
#ifndef BED_CONFIG_READ_ONLY
	bed_write_method write;
	bed_write_oob_method write_oob;
	bed_erase_method erase;
	bed_mark_block_bad_method mark_block_bad;
#endif /* BED_CONFIG_READ_ONLY */
	void *context;
	uint16_t current_chip;
	uint16_t chip_count;
	uint32_t blocks_per_chip;
	uint32_t block_size;
	uint16_t pages_per_block;
	uint16_t page_size;
	uint16_t oob_size;
	uint16_t oob_free_size;
	uint8_t page_shift;
	uint8_t block_shift;
	uint8_t chip_shift;
	uint32_t page_mask;
	uint32_t ecc_covers_oob : 1;
	bed_address size;
#ifdef __rtems__
	uint32_t mutex_id;
#endif
};

/** @} */

/**
 * @addtogroup BED
 *
 * @{
 */

static inline bool bed_ecc_covers_oob(
	const bed_partition *part
)
{
	const bed_device *bed = part->bed;

	return bed->ecc_covers_oob;
}

static inline uint16_t bed_oob_size(const bed_partition *part)
{
	const bed_device *bed = part->bed;

	return bed->oob_size;
}

static inline uint16_t bed_oob_free_size(const bed_partition *part)
{
	const bed_device *bed = part->bed;

	return bed->oob_free_size;
}

static inline uint16_t bed_page_size(const bed_partition *part)
{
	const bed_device *bed = part->bed;

	return bed->page_size;
}

static inline uint32_t bed_block_size(const bed_partition *part)
{
	const bed_device *bed = part->bed;

	return bed->block_size;
}

static inline uint32_t bed_device_size(const bed_partition *part)
{
	const bed_device *bed = part->bed;

	return bed->size;
}

static inline uint32_t bed_begin(const bed_partition *part)
{
	return part->begin;
}

static inline uint32_t bed_size(const bed_partition *part)
{
	return part->size;
}

static inline uint32_t bed_end(const bed_partition *part)
{
	return part->begin + part->size;
}

static inline int bed_page_shift(
	const bed_partition *part
)
{
	const bed_device *bed = part->bed;

	return bed->page_shift;
}

static inline bed_address bed_page_to_address(
	const bed_partition *part,
	bed_address page
)
{
	return page << bed_page_shift(part);
}

static inline bed_address bed_address_to_page(
	const bed_partition *part,
	bed_address addr
)
{
	return addr >> bed_page_shift(part);
}

static inline int bed_block_shift(
	const bed_partition *part
)
{
	const bed_device *bed = part->bed;

	return bed->block_shift;
}

static inline bed_address bed_block_to_address(
	const bed_partition *part,
	bed_address block
)
{
	return block << bed_block_shift(part);
}

static inline bed_address bed_address_to_block(
	const bed_partition *part,
	bed_address addr
)
{
	return addr >> bed_block_shift(part);
}

static inline int bed_chip_shift(
	const bed_partition *part
)
{
	const bed_device *bed = part->bed;

	return bed->chip_shift;
}

static inline bed_address bed_chip_to_address(
	const bed_partition *part,
	bed_address chip
)
{
	return chip << bed_chip_shift(part);
}

static inline bed_address bed_address_to_chip(
	const bed_partition *part,
	bed_address addr
)
{
	return addr >> bed_chip_shift(part);
}

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_H */
