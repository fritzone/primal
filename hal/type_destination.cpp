#include "type_destination_decl.h"

std::string to_string(type_destination dst)
{
    switch(dst)
    {
    case type_destination::TYPE_MOD_IMM:                  return "TYPE_MOD_IMM";
    case type_destination::TYPE_MOD_REG:                  return "TYPE_MOD_REG";
    case type_destination::TYPE_MOD_MEM_REG_IDX:          return "TYPE_MOD_MEM_REG_IDX";
    case type_destination::TYPE_MOD_MEM_REG_BYTE:         return "TYPE_MOD_MEM_REG_BYTE";
    case type_destination::TYPE_MOD_REG_BYTE:             return "TYPE_MOD_REG_BYTE";
    case type_destination::TYPE_MOD_REG_BYTE0:            return "TYPE_MOD_REG_BYTE0";
    case type_destination::TYPE_MOD_REG_BYTE1:            return "TYPE_MOD_REG_BYTE1";
    case type_destination::TYPE_MOD_REG_BYTE2:            return "TYPE_MOD_REG_BYTE2";
    case type_destination::TYPE_MOD_REG_BYTE3:            return "TYPE_MOD_REG_BYTE3";
#if TARGET_ARCH == 64
    case type_destination::TYPE_MOD_REG_BYTE4:            return "TYPE_MOD_REG_BYTE4";
    case type_destination::TYPE_MOD_REG_BYTE5:            return "TYPE_MOD_REG_BYTE5";
    case type_destination::TYPE_MOD_REG_BYTE6:            return "TYPE_MOD_REG_BYTE6";
    case type_destination::TYPE_MOD_REG_BYTE7:            return "TYPE_MOD_REG_BYTE7";
#endif
    case type_destination::TYPE_MOD_MEM_IMM:              return "TYPE_MOD_MEM_IMM";
    case type_destination::TYPE_MOD_MEM_IMM_BYTE:         return "TYPE_MOD_MEM_IMM_BYTE";
    case type_destination::TYPE_MOD_MEM_REG_IDX_OFFS:     return "TYPE_MOD_MEM_REG_IDX_OFFS";
    case type_destination::TYPE_MOD_MEM_REG_IDX_REG_OFFS: return "TYPE_MOD_MEM_REG_IDX_REG_OFFS";
    }
    return "UNKNOWN";
}
