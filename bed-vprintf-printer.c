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

#include "bed.h"

#include <stdarg.h>

int bed_vprintf_printer(void *arg, const char *fmt, ...)
{
	int (*printer)(const char *fmt, va_list ap) = arg;
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = (*printer)(fmt, ap);
	va_end(ap);

	return rv;
}
