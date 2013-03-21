/**
 * @file
 *
 * @ingroup BEDImplBits
 *
 * @brief BED Bit Fields API.
 */

/*
 * Copyright (c) 2008-2012 embedded brains GmbH.  All rights reserved.
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

#ifndef BED_BITS_H
#define BED_BITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup BEDImplBits BED Bit Fields
 *
 * @ingroup BEDImpl
 *
 * @{
 */

#define BED_BIT8(bit) \
  ((uint8_t) (((unsigned int) 1) << (bit)))

#define BED_MSK8(first_bit, last_bit) \
  ((uint8_t) ((BED_BIT8((last_bit) - (first_bit) + 1) - 1) << (first_bit)))

#define BED_FLD8(val, first_bit, last_bit) \
  ((uint8_t) \
    ((((unsigned int) (val)) << (first_bit)) & BED_MSK8(first_bit, last_bit)))

#define BED_FLD8GET(reg, first_bit, last_bit) \
  ((uint8_t) (((reg) & BED_MSK8(first_bit, last_bit)) >> (first_bit)))

#define BED_FLD8SET(reg, val, first_bit, last_bit) \
  ((uint8_t) (((reg) & ~BED_MSK8(first_bit, last_bit)) \
    | BED_FLD8(val, first_bit, last_bit)))

#define BED_BIT16(bit) \
  ((uint16_t) (((unsigned int) 1) << (bit)))

#define BED_MSK16(first_bit, last_bit) \
  ((uint16_t) ((BED_BIT16((last_bit) - (first_bit) + 1) - 1) << (first_bit)))

#define BED_FLD16(val, first_bit, last_bit) \
  ((uint16_t) \
    ((((unsigned int) (val)) << (first_bit)) & BED_MSK16(first_bit, last_bit)))

#define BED_FLD16GET(reg, first_bit, last_bit) \
  ((uint16_t) (((reg) & BED_MSK16(first_bit, last_bit)) >> (first_bit)))

#define BED_FLD16SET(reg, val, first_bit, last_bit) \
  ((uint16_t) (((reg) & ~BED_MSK16(first_bit, last_bit)) \
    | BED_FLD16(val, first_bit, last_bit)))

#define BED_BIT32(bit) \
  ((uint32_t) (((uint32_t) 1) << (bit)))

#define BED_MSK32(first_bit, last_bit) \
  ((uint32_t) ((BED_BIT32((last_bit) - (first_bit) + 1) - 1) << (first_bit)))

#define BED_FLD32(val, first_bit, last_bit) \
  ((uint32_t) \
    ((((uint32_t) (val)) << (first_bit)) & BED_MSK32(first_bit, last_bit)))

#define BED_FLD32GET(reg, first_bit, last_bit) \
  ((uint32_t) (((reg) & BED_MSK32(first_bit, last_bit)) >> (first_bit)))

#define BED_FLD32SET(reg, val, first_bit, last_bit) \
  ((uint32_t) (((reg) & ~BED_MSK32(first_bit, last_bit)) \
    | BED_FLD32(val, first_bit, last_bit)))

#define BED_BIT64(bit) \
  ((uint64_t) (((uint64_t) 1) << (bit)))

#define BED_MSK64(first_bit, last_bit) \
  ((uint64_t) ((BED_BIT64((last_bit) - (first_bit) + 1) - 1) << (first_bit)))

#define BED_FLD64(val, first_bit, last_bit) \
  ((uint64_t) \
    ((((uint64_t) (val)) << (first_bit)) & BED_MSK64(first_bit, last_bit)))

#define BED_FLD64GET(reg, first_bit, last_bit) \
  ((uint64_t) (((reg) & BED_MSK64(first_bit, last_bit)) >> (first_bit)))

#define BED_FLD64SET(reg, val, first_bit, last_bit) \
  ((uint64_t) (((reg) & ~BED_MSK64(first_bit, last_bit)) \
    | BED_FLD64(val, first_bit, last_bit)))

#define BED_BBIT8(bit) \
  BED_BIT8(7 - (bit))

#define BED_BMSK8(first_bit, last_bit) \
  BED_MSK8(7 - (last_bit), 7 - (first_bit))

#define BED_BFLD8(val, first_bit, last_bit) \
  BED_FLD8(val, 7 - (last_bit), 7 - (first_bit))

#define BED_BFLD8GET(reg, first_bit, last_bit) \
  BED_FLD8GET(reg, 7 - (last_bit), 7 - (first_bit))

#define BED_BFLD8SET(reg, val, first_bit, last_bit) \
  BED_FLD8SET(reg, val, 7 - (last_bit), 7 - (first_bit))

#define BED_BBIT16(bit) \
  BED_BIT16(15 - (bit))

#define BED_BMSK16(first_bit, last_bit) \
  BED_MSK16(15 - (last_bit), 15 - (first_bit))

#define BED_BFLD16(val, first_bit, last_bit) \
  BED_FLD16(val, 15 - (last_bit), 15 - (first_bit))

#define BED_BFLD16GET(reg, first_bit, last_bit) \
  BED_FLD16GET(reg, 15 - (last_bit), 15 - (first_bit))

#define BED_BFLD16SET(reg, val, first_bit, last_bit) \
  BED_FLD16SET(reg, val, 15 - (last_bit), 15 - (first_bit))

#define BED_BBIT32(bit) \
  BED_BIT32(31 - (bit))

#define BED_BMSK32(first_bit, last_bit) \
  BED_MSK32(31 - (last_bit), 31 - (first_bit))

#define BED_BFLD32(val, first_bit, last_bit) \
  BED_FLD32(val, 31 - (last_bit), 31 - (first_bit))

#define BED_BFLD32GET(reg, first_bit, last_bit) \
  BED_FLD32GET(reg, 31 - (last_bit), 31 - (first_bit))

#define BED_BFLD32SET(reg, val, first_bit, last_bit) \
  BED_FLD32SET(reg, val, 31 - (last_bit), 31 - (first_bit))

#define BED_BBIT64(bit) \
  BED_BIT64(63 - (bit))

#define BED_BMSK64(first_bit, last_bit) \
  BED_MSK64(63 - (last_bit), 63 - (first_bit))

#define BED_BFLD64(val, first_bit, last_bit) \
  BED_FLD64(val, 63 - (last_bit), 63 - (first_bit))

#define BED_BFLD64GET(reg, first_bit, last_bit) \
  BED_FLD64GET(reg, 63 - (last_bit), 63 - (first_bit))

#define BED_BFLD64SET(reg, val, first_bit, last_bit) \
  BED_FLD64SET(reg, val, 63 - (last_bit), 63 - (first_bit))

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BED_BITS_H */
