#include <MOV.h>
#include <vm.h>
#include <types.h>

#include <iostream>

bool impl_MOV(vm* v)
{
    // IP points to the type choose byte
    auto dst = v->fetch_type_dest();
    switch(dst)
    {
        // are we moving something into a register?
        case type_destination::TYPE_MOD_REG:
        {
            uint8_t ridx = v->fetch_register_index();

            // fetch what are we moving in the register
            auto src = v->fetch_type_dest();
            switch(src)
            {
                // we are moving a number into the register
                case type_destination::TYPE_MOD_IMM: { v->r(ridx).set_value(v->fetch_immediate()); return true; }
                case type_destination::TYPE_MOD_REG: { v->r(ridx).set_value(v->r(v->fetch_register_index())); return true; }
            }

            break;
        }

        // are we moving something into an immediate memory address?
        case type_destination ::TYPE_MOD_MEM_IMM:
        {
            numeric_t addr = v->fetch_immediate();
            // fetch what we are moving into the memory addres.
            auto src = v->fetch_type_dest();
            switch(src)
            {
                case type_destination::TYPE_MOD_REG: { v->set_mem(addr, v->r(v->fetch_register_index()).value()); return true; }
            }
        }
    }
    return false;
}

