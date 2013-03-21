/**
 * @file
 *
 * @ingroup BEDYAFFS
 *
 * @brief BED YAFFS Support API
 */

/*
 * Copyright (c) 2011-2013 embedded brains GmbH.  All rights reserved.
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

#ifndef BED_YAFFS_H
#define BED_YAFFS_H

#include "bed.h"

struct yaffs_dev;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDYAFFS YAFFS Support
 *
 * @ingroup BED
 *
 * @brief YAFFS support.
 *
 * Spare area usage:
 * <table>
 *   <tr>
 *     <th>byte offset</th>
 *     <td>0</td>
 *     <td>1</td><td>2</td><td>3</td><td>4</td>
 *     <td>5</td>
 *     <td>6 up to 15</td>
 *   </tr>
 *   <tr>
 *     <th>usage</th>
 *     <td>bad page information</td>
 *     <td colspan=4>YAFFS data</td>
 *     <td>bad page information</td>
 *     <td colspan=10>error correction code (ECC)</td>
 *   </tr>
 * </table>
 *
 * @{
 */

bed_status bed_yaffs_initialize_device(
	const bed_partition *part,
	struct yaffs_dev *dev
);

static inline bed_status bed_yaffs_format(const bed_partition *part)
{
	return bed_erase_all(part, BED_ERASE_MARK_BAD_ON_ERROR);
}

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_YAFFS_H */
