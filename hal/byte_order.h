#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

/**
 * @file byte_order.h
 * @brief Byte-order conversion utilities for the VM.
 *
 * Provides functions to convert 16-, 32-, and 64-bit integers
 * between host byte order and the VM’s expected byte order.
 *
 * If compiled with `COMPILED_ON_LITTLE_ENDIAN`, the VM is assumed to run
 * on a little-endian host and optimized functions are provided.
 * Otherwise, unions are used for explicit byte manipulation.
 */

/// @def COMPILED_ON_LITTLE_ENDIAN
/// @brief Defined when the code is compiled on a little-endian host system.
#define COMPILED_ON_LITTLE_ENDIAN

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef COMPILED_ON_LITTLE_ENDIAN

/**
 * @brief Convert a 16-bit value from host to VM byte order.
 *
 * On little-endian systems, this may be a no-op.
 *
 * @param s 16-bit unsigned integer in host order.
 * @return Value converted to VM order.
 */
uint16_t htovm_16(uint16_t s);

/**
 * @brief Convert a 32-bit value from host to VM byte order.
 *
 * On little-endian systems, this may be a no-op.
 *
 * @param l 32-bit unsigned integer in host order.
 * @return Value converted to VM order.
 */
uint32_t htovm_32(uint32_t l);

/**
 * @brief Convert a 64-bit value from host to VM byte order.
 *
 * On little-endian systems, this may be a no-op.
 *
 * @param ll 64-bit unsigned integer in host order.
 * @return Value converted to VM order.
 */
uint64_t htovm_64(uint64_t ll);

#else /* Big-endian case */

/**
 * @brief Union for 32-bit value reinterpretation.
 *
 * Provides access to the same 32-bit integer as:
 * - An array of 4 bytes,
 * - An array of 2 16-bit integers,
 * - A single 32-bit integer.
 */
typedef union
{
    uint8_t c[4];   ///< Access as 4 bytes.
    uint16_t s[2];  ///< Access as 2 16-bit words.
    uint32_t l;     ///< Access as a 32-bit integer.
} U4;

/**
 * @brief Union for 64-bit value reinterpretation.
 *
 * Provides access to the same 64-bit integer as:
 * - An array of 8 bytes,
 * - A 2D array of 2×4 bytes,
 * - An array of 4 16-bit integers,
 * - An array of 2 32-bit integers,
 * - A single 64-bit integer.
 */
typedef union
{
    uint8_t c[8];     ///< Access as 8 bytes.
    uint8_t c2[2][4]; ///< Access as 2×4 byte chunks.
    uint16_t s[4];    ///< Access as 4 16-bit words.
    uint32_t l[2];    ///< Access as 2 32-bit integers.
    uint64_t ll;      ///< Access as a 64-bit integer.
} U8;

/**
 * @brief Union for 16-bit value reinterpretation.
 *
 * Provides access to the same 16-bit integer as:
 * - An array of 2 bytes,
 * - A single 16-bit integer.
 */
typedef union
{
    uint8_t c[2];   ///< Access as 2 bytes.
    uint16_t s;     ///< Access as 16-bit integer.
} U2;

/**
 * @brief Convert a 16-bit value from host to VM byte order.
 *
 * On big-endian systems, this swaps the byte order.
 *
 * @param s 16-bit unsigned integer in host order.
 * @return Value converted to VM order.
 */
uint16_t htovm_16(uint16_t s);

/**
 * @brief Convert a 32-bit value from host to VM byte order.
 *
 * On big-endian systems, this swaps the byte order.
 *
 * @param l 32-bit unsigned integer in host order.
 * @return Value converted to VM order.
 */
uint32_t htovm_32(uint32_t l);

/**
 * @brief Convert a 64-bit value from host to VM byte order.
 *
 * On big-endian systems, this swaps the byte order.
 *
 * @param ll 64-bit unsigned integer in host order.
 * @return Value converted to VM order.
 */
uint64_t htovm_64(uint64_t ll);

#endif // COMPILED_ON_LITTLE_ENDIAN

#ifdef __cplusplus
}
#endif

#endif // BYTE_ORDER_H
