#include <MOV.h>
#include <vm.h>
#include <types.h>

#include <iostream>

bool impl_MOV(vm* v)
{
    // IP points to the type choose byte
    switch(v->fetch_type_dest())
    {
        // are we moving something into a register?
        case type_destination::TYPE_MOD_REG:
        {
            uint8_t ridx = v->fetch_register_index();

            // fetch what are we moving in the register
            switch(v->fetch_type_dest())
            {
                // we are moving a number into the register
                case type_destination::TYPE_MOD_IMM:
                {
                    v->r(ridx) = v->fetch_immediate();;
                    return true;
                }

                case type_destination::TYPE_MOD_REG:
                {
                    auto idx2 = v->fetch_register_index();
                    v->r(ridx) = v->r(idx2);
                    return true;
                }
                case type_destination::TYPE_MOD_VAR:break;
                case type_destination::TYPE_MOD_MEM_IMM:break;
                case type_destination::TYPE_MOD_STR_IMM:break;
            }

            break;
        }
    }
    return false;
}

