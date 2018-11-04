#include "vm.h"

#include <exceptions.h>
#include <opcodes.h>
#include <hal.h>
#include <types.h>
#include <util.h>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <cstring>
#include <sstream>

using namespace primal;

std::map<uint8_t, vm::executor> vm::vm_runner;

vm::vm()
{
    for(uint8_t i = 0; i<255; i++)
    {
        m_r[i].set_idx(i);
    }
}


bool vm::run(const std::vector<uint8_t> &app)
{
    // firstly set up the memory segment for this machine and initialize it to 0xFF
    app_size = app.size();
    ms = std::make_unique<uint8_t[]>(app_size + VM_MEM_SEGMENT_SIZE);
    std::fill(ms.get(), ms.get() + VM_MEM_SEGMENT_SIZE + app_size, 0xFF);

    // then copy over the data from app to the end of the memory segment
    std::copy(app.begin(), app.end(), ms.get() + VM_MEM_SEGMENT_SIZE);

    // set the IP and SP to point to the correct location
    m_ip = VM_MEM_SEGMENT_SIZE;     // will grow upwards
    r(255) = VM_MEM_SEGMENT_SIZE;   // will grow downwards

    // then start running it
    while(vm_runner.count(ms[m_ip]))
    {
        // read in an opcode
        uint8_t opc = ms[m_ip ++];

        // is there a registered opcode runner for the given opcode?
        if(!vm_runner[ opc ].opcode_runner(this))
        {
            panic();
        }

        // is the opcode after the current one 0xFF meaning: halt the machine?
        if(ms[m_ip] == 0xFF)
        {
            return true;
        }
    }
    panic();
}

std::shared_ptr<vm> vm::create()
{
    // this is autogenerated by cmake
    register_opcodes();

    return std::make_shared<vm>();
}

type_destination vm::fetch_type_dest()
{
    return static_cast<type_destination>(ms[m_ip ++]) ;
}

void vm::panic()
{
    std::cout << "VM PANIC ☹ - instruction dump:\n---------------------------------------------------\n";
    std::stringstream ss;
    numeric_t start = std::max(VM_MEM_SEGMENT_SIZE, m_ip - 64);
    bool insert_addr = true;
    for(numeric_t i = start; i < m_ip + std::min(64, app_size); i++)
    {
        if(insert_addr)
        {
            ss << std::setfill(' ') << std::setw(9) << std::dec << std::right <<  i << "[:" << std::setw(3) << i - VM_MEM_SEGMENT_SIZE << "]";
            insert_addr = false;
        }
        if(i == m_ip)
        {
            ss << ">";
        }
        else
        {
            ss << " ";
        }
        ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase  << static_cast<int>(ms[i]) ;
        if(i == m_ip)
        {
            ss << "<";
        }
        else
        {
            ss << " ";
        }
        if(ss.str().length() > 80)
        {
            std::cout << ss.str() << std::endl;
            ss.clear();
            ss.str(std::string());
            insert_addr = true;
        }
    }
    std::cout << ss.str() << std::endl;
    std::cout << "IP=" << std::dec << m_ip << "[:" << m_ip - VM_MEM_SEGMENT_SIZE << "] (" << std::hex << m_ip << ")" << std::endl;
    std::cout << "SP=" << std::dec << r(255).value() << " (" << std::hex << r(255).value() << ")" << std::endl;
    std::cout << std::endl;
    throw primal::vm_panic("PANIC");
}

uint8_t vm::fetch_register_index()
{
    return ms[m_ip ++];
}

numeric_t vm::fetch_immediate()
{
    numeric_t retv = htovm(*(reinterpret_cast<numeric_t*>(ms.get() + m_ip)));
    m_ip += sizeof(numeric_t);
    return retv;
}

void vm::set_mem(numeric_t address, numeric_t new_value)
{
    if(address > VM_MEM_SEGMENT_SIZE)
    {
        panic();
    }

    std::memcpy( &ms[0] + address, &new_value, sizeof(new_value));
}

numeric_t vm::get_mem(numeric_t address)
{
    if(address > VM_MEM_SEGMENT_SIZE)
    {
        panic();
    }

    numeric_t v = 0;
    std::memcpy(&v, &ms[0] + address, sizeof(v));
    return v;
}

void vm::set_mem_byte(numeric_t address, uint8_t b)
{
    if(address > VM_MEM_SEGMENT_SIZE)
    {
        panic();
    }
    ms[address] = b;
}

uint8_t vm::get_mem_byte(numeric_t address)
{
    if(address > VM_MEM_SEGMENT_SIZE)
    {
        panic();
    }
    return ms[address];
}

reg_subbyte* vm::rsb(uint8_t ridx, uint8_t bidx)
{
    t1.m_r = &r(ridx);
    t1.m_bidx = bidx;
    return &t1;
}

memaddress* vm::mem(numeric_t address)
{
    auto setter = [&](numeric_t a, numeric_t v) -> void
        {
            set_mem(a,v);
        };
    auto getter = [&](numeric_t a) -> numeric_t
        {
            return get_mem(a);
        };

    ma_i = !ma_i;
    ma[ma_i].m_address = address;
    ma[ma_i].m_setter = setter;
    ma[ma_i].m_getter = getter;
    return &ma[ma_i];
}

memaddress_byte_ref* vm::mem_byte(numeric_t address)
{
    auto setter = [&](numeric_t a, uint8_t v) -> void
    {
        set_mem_byte(a,v);
    };
    auto getter = [&](numeric_t a) -> uint8_t
    {
        return get_mem_byte(a);
    };

    mb_i = !mb_i;
    mb[mb_i].m_address = address;
    mb[mb_i].m_setter = setter;
    mb[mb_i].m_getter = getter;
    return &mb[mb_i];
}


immediate *vm::imm(numeric_t v)
{
    mi_i++; if(mi_i == 3) mi_i = 1;

    imv[mi_i].m_value = v;
    return &imv[mi_i];
}

valued *vm::fetch()
{
    auto dst = fetch_type_dest();
    switch(dst)
    {
        case type_destination::TYPE_MOD_IMM:
        {
            return imm(fetch_immediate());
        }

        case type_destination::TYPE_MOD_REG_BYTE:
        {
            uint8_t ridx = fetch_register_index();
            return rsb(ridx, 0);
        }

        case type_destination::TYPE_MOD_REG_BYTE0:  // [[fallthrough]]
        case type_destination::TYPE_MOD_REG_BYTE1:
        case type_destination::TYPE_MOD_REG_BYTE2:
        case type_destination::TYPE_MOD_REG_BYTE3:
        {
            uint8_t ridx = fetch_register_index();
            return rsb(ridx, static_cast<uint8_t>(dst) - static_cast<uint8_t>(type_destination::TYPE_MOD_REG_BYTE0));
        }

            // are we moving something into a register?
        case type_destination::TYPE_MOD_REG:
        {
            uint8_t ridx = fetch_register_index();
            return &(r(ridx));
        }

            // are we moving something into an immediate memory address?
        case type_destination::TYPE_MOD_MEM_IMM:
        {
            numeric_t vaddr = fetch_immediate();
            return mem(vaddr);
        }

        case type_destination::TYPE_MOD_MEM_REG_IDX:
        {
            uint8_t ridx = fetch_register_index();
            return mem(r(ridx).value());
        }

        case type_destination::TYPE_MOD_MEM_REG_BYTE:
        {
            uint8_t ridx = fetch_register_index();
            return mem_byte(r(ridx).value());
        }

        case type_destination::TYPE_MOD_MEM_IMM_BYTE:
        {
            numeric_t vaddr = fetch_immediate();
            return mem_byte(vaddr);
        }
    }

    return nullptr;

}

bool vm::copy(numeric_t dest, numeric_t src, numeric_t cnt)
{
    if(dest + cnt > VM_MEM_SEGMENT_SIZE || src < 0 || dest < 0 || src > VM_MEM_SEGMENT_SIZE)
    {
        return false;
    }
    std::memmove(&ms[dest], &ms[src], cnt);
    return true;
}

bool vm::push(const valued* v)
{
    set_mem(r(255).value() - 4, v->value());
    r(255) -= 4;
    if(r(255).value() < 0)
    {
        panic();
    }
    set_mem_byte(r(255).value() - 1, util::to_integral(v->get_type()));
    r(255) -= 1;
    if (r(255).value() < 0)
    {
        panic();
    }

    return true;
}

numeric_t vm::pop()
{
    numeric_t v = get_mem(r(255).value());
    r(255) += 4;
    if(r(255).value() > VM_MEM_SEGMENT_SIZE)
    {
        panic();
    }
    return v;
}
