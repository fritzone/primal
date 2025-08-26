#ifndef HAL_H
#define HAL_H

#include "numeric_decl.h"

/* Holds the number of registers in the virtual machine */
static const int VM_REG_COUNT = 256;

/* The size of the free memory region, 1MB initially can be increased if you feel like */
static const word_t VM_MEM_SEGMENT_SIZE = 1000 /** 1024*/;

// the size of the header
const word_t PRIMAL_HEADER_SIZE = 4 + 3 * word_size;

// where the string table copy starts in the memory
static const word_t STRING_TABLE_INDEX_IN_MEM = 500;

#endif //HAL_H
