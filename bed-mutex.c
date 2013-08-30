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

#ifdef __rtems__

#include <assert.h>
#include <rtems.h>

bed_status bed_mutex_initialize(bed_device *bed)
{
	rtems_status_code sc = rtems_semaphore_create(
		rtems_build_name('B', 'E', 'D', 'M'),
		1,
		RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY,
		0,
		&bed->mutex_id
	);

	return sc == RTEMS_SUCCESSFUL ? BED_SUCCESS : BED_ERROR_SYSTEM;
}

void bed_mutex_obtain(bed_device *bed)
{
	rtems_status_code sc = rtems_semaphore_obtain(
		bed->mutex_id,
		RTEMS_WAIT,
		RTEMS_NO_TIMEOUT
	);
	(void) sc;
	assert(sc == RTEMS_SUCCESSFUL);
}

void bed_mutex_release(bed_device *bed)
{
	rtems_status_code sc = rtems_semaphore_release(bed->mutex_id);
	(void) sc;
	assert(sc == RTEMS_SUCCESSFUL);
}

#endif /* __rtems__ */
