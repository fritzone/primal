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
            switch(v->fetch_type_dest())
            {
                // we are moving a number into the register
                case type_destination::TYPE_MOD_IMM: { v->r(ridx) = v->fetch_immediate(); return true; }
                case type_destination::TYPE_MOD_REG: { auto idx2 = v->fetch_register_index(); v->r(ridx) = v->r(idx2); return true; }
            }

            break;
        }

        // are we moving something into an immediate memory address?
        case type_destination ::TYPE_MOD_MEM_IMM:
        {
            numeric_t addr = v->fetch_immediate();
            // fetch what we are moving into the memory addres.
            switch(v->fetch_type_dest())
            {
                case type_destination::TYPE_MOD_REG: { auto idx = v->fetch_register_index(); v->set_mem(addr, v->r(idx).value()); return true; }
            }
        }
    }
    return false;
}

