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

#define CONFIGURE_MINIMUM_TASK_STACK_SIZE (32 * 1024)

#define CONFIGURE_MINIMUM_POSIX_THREAD_STACK_SIZE (32 * 1024)

#include <rtems/gtest-main.h>
