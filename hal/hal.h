#ifndef HAL_H
#define HAL_H

#include "types.h"
#include "registers.h"

/* Holds the number of registers in the virtual machine */
static const int VM_REG_COUNT = 256;

/* The size of the free memory region, 1MB initially can be increased if you feel like */
static const int VM_MEM_SEGMENT_SIZE = 1024 * 1024;

#endif //PRIMITIVE_HAL_H
