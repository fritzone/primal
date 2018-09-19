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

            }

            break;
        }
    }
    return false;
}

