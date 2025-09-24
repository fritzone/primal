#ifndef TYPE_DESTINATION_DECL_H
#define TYPE_DESTINATION_DECL_H

#include <cstdint>
#include <string>

/* represents the type of a piece of binary data */
enum class type_destination : uint8_t
{
    TYPE_MOD_IMM                    = 0x00,     // what follows is a 32 bit integer value
    TYPE_MOD_REG                    = 0x01,     // what follows is the index of a register, 1 byte
    TYPE_MOD_MEM_REG_IDX            = 0x02,     // The number from the memory at the value of the the given register
    TYPE_MOD_MEM_REG_BYTE           = 0x03,     // The byte value from the memory at the given registers' value
    TYPE_MOD_REG_BYTE               = 0x04,     // The 0th byte of a register
    TYPE_MOD_REG_BYTE0              = 0x40,     // The 0th byte of a register
    TYPE_MOD_REG_BYTE1              = 0x41,     // The 1st byte of a register
    TYPE_MOD_REG_BYTE2              = 0x42,     // The 2nd byte of a register
    TYPE_MOD_REG_BYTE3              = 0x43,     // The 3rd byte of a register
#if TARGET_ARCH == 64
    TYPE_MOD_REG_BYTE4              = 0x44,     // The 0th byte of a register
    TYPE_MOD_REG_BYTE5              = 0x45,     // The 1st byte of a register
    TYPE_MOD_REG_BYTE6              = 0x46,     // The 2nd byte of a register
    TYPE_MOD_REG_BYTE7              = 0x47,     // The 3rd byte of a register
#endif
    TYPE_MOD_MEM_IMM                = 0x05,     // what follows is the address of a memory cell, numeric_t value
    TYPE_MOD_MEM_IMM_BYTE           = 0x06,     // what follows is the address of a memory cell, numeric_t value
    TYPE_MOD_MEM_REG_IDX_OFFS       = 0x07,     // The number from the memory at the value of the the given register + an offset
    TYPE_MOD_MEM_REG_IDX_REG_OFFS   = 0x08,     // The number from the memory at the value of the the given register + an offset with the value of another register
    TYPE_MOD_IMM_BYTE               = 0x09,     // what follows is a 8 bit integer value

    TYPE_MOD_UNKNOWN                = 0xFF,     // Unknownd for now

};

std::string to_string(type_destination dst);

#endif
