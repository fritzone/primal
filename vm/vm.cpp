#include "vm.h"
#include "vm_impl.h"

#include <interrupts.h>

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


vm::vm() : impl(new vm_impl)
{
}

bool vm::run(const std::vector<uint8_t> &app)
{
    // firstly set up the memory segment for this machine and initialize it to 0x00
    impl->app_size = static_cast<word_t>(app.size());
    impl->ms = std::make_unique<uint8_t[]>(static_cast<size_t>(impl->app_size + VM_MEM_SEGMENT_SIZE));
    std::fill(impl->ms.get(), impl->ms.get() + VM_MEM_SEGMENT_SIZE + impl->app_size, 0x00);

    // then copy over the data from app to the end of the memory segment
    std::copy(app.begin(), app.end(), impl->ms.get() + VM_MEM_SEGMENT_SIZE);

    // set the IP and SP to point to the correct location
    impl->m_ip = VM_MEM_SEGMENT_SIZE + PRIMAL_HEADER_SIZE; // will grow upwards, will skip .P10 and the stringtable loc entry
    impl->stack_offset = *reinterpret_cast<word_t*>(impl->ms.get() + VM_MEM_SEGMENT_SIZE + 8);
    impl->sp = impl->stack_offset * word_size;   // will grow upwards

    // then start running it
    while(impl->opcode_runners.count(impl->ms[static_cast<size_t>(impl->m_ip)]))
    {
        // read in an opcode
        uint8_t opc = impl->ms[static_cast<size_t>(impl->m_ip++)];

        try
        {
            // is there a registered opcode runner for the given opcode?
            if(!impl->opcode_runners[ opc ].runner(this))
            {
                panic();
            }
        }
        catch (const primal::vm_panic&)
        {
            throw;
        }
        catch(...)
        {
            panic();
        }

        // is the opcode after the current one 0xFF meaning: halt the machine?
        if(impl->m_ip < 0)
        {
            panic();
        }
        if(impl->ms[static_cast<size_t>(impl->m_ip)] == 0xFF)
        {
            //bindump();
            return true;
        }
    }
    panic();
}

word_t &vm::ip()      {return impl->m_ip;}

word_t vm::ip() const {return impl->m_ip;}

std::shared_ptr<vm> vm::create()
{
    // these functions are autogenerated by cmake in their own files
    register_opcodes();
    register_interrupts();

    return std::make_shared<vm>();
}

type_destination vm::fetch_type_dest()
{
    return static_cast<type_destination>(impl->ms[static_cast<size_t>(impl->m_ip ++)]) ;
}

void vm::bindump(word_t start, word_t end, bool insert_addr)
{
    if(start == -1) start = VM_MEM_SEGMENT_SIZE;
    if(end == -1) end = start + impl->app_size;

    std::stringstream ss;
    std::string s;

    for(word_t i = start; i < std::min(end, VM_MEM_SEGMENT_SIZE + impl->app_size); i++)
    {
        if(insert_addr)
        {
            ss << std::setfill(' ') << std::setw(9) << std::dec << std::right <<  i << "[:" << std::setw(3) << i - VM_MEM_SEGMENT_SIZE << "]";
            insert_addr = false;
        }
        if(i == impl->m_ip)
        {
            ss << ">";
        }
        else
        {
            ss << " ";
        }
        ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase  << static_cast<int>(impl->ms[static_cast<size_t>(i)]) ;

        if(impl->ms[static_cast<size_t>(i)] > 32 && impl->ms[static_cast<size_t>(i)] < 255)
        {
            s += static_cast<char>(impl->ms[static_cast<size_t>(i)]);
        }
        else
        {
            s += '.';
        }

        if(i == impl->m_ip)
        {
            ss << "<";
        }
        else
        {
            ss << " ";
        }
        if(ss.str().length() > 80)
        {
            std::cout << ss.str() << " | " << s << std::endl;
            s = "";
            ss.clear();
            ss.str(std::string());
            insert_addr = true;
        }
    }
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str(std::string());

    // memory dump
    for(word_t i=0; i<impl->stack_offset * word_size; i += word_size)
    {
        ss << std::right << " [" << std::setfill('0') << std::setw(8) << std::dec << i << "] = ";
        ss << get_mem(i) << std::endl;
    }
    ss << "-"; // indicates the stack start
    for(word_t i = impl->stack_offset * word_size; i<impl->max_used_sp + 32; i += word_size)
    {
        if(i == impl->sp.value())
        {
            ss <<">" ; // current stack position
        }
        else
        {
            if (i > impl->stack_offset * word_size) ss << " ";
        }

        ss << std::right << "[" << std::setfill('0') << std::setw(8) << std::dec << i << "] = ";
        ss << std::setfill('0') << std::setw(8) << std::dec << get_mem(i) << std::endl;
    }
    std::cout << ss.str();
}

void vm::panic()
{
    std::cout << "VM PANIC ☹ - instruction dump:\n---------------------------------------------------\n";
    word_t start = std::max<word_t>(VM_MEM_SEGMENT_SIZE, impl->m_ip - 64);
    word_t end = impl->m_ip + std::min<word_t>(64, impl->app_size);
    bindump(start, end, true);
    std::cout << "IP=" << std::dec << impl->m_ip << "[:" << impl->m_ip - VM_MEM_SEGMENT_SIZE << "] (" << std::hex << impl->m_ip << ")" << std::endl;
    std::cout << "SP=" << std::dec << impl->sp.value() << " (" << std::hex << impl->sp.value() << ")" << std::endl;
    std::cout << std::endl;
    throw primal::vm_panic("PANIC");
}

uint8_t vm::fetch_register_index()
{
    return impl->ms[static_cast<size_t>(impl->m_ip ++)];
}

word_t vm::fetch_immediate()
{
    word_t retv = htovm(*(reinterpret_cast<word_t*>(impl->ms.get() + impl->m_ip)));
    impl->m_ip += word_size;
    return retv;
}

void vm::set_mem(word_t address, word_t new_value)
{
    if(address > VM_MEM_SEGMENT_SIZE || address < 0)
    {
        panic();
    }

    std::memcpy( &impl->ms[0] + address, &new_value, sizeof(new_value));
}

word_t vm::get_mem(word_t address)
{
    if(address > VM_MEM_SEGMENT_SIZE || address < 0)
    {
        panic();
    }

    word_t v = 0;
    std::memcpy(&v, &impl->ms[0] + address, sizeof(v));
    return v;
}

void vm::set_mem_byte(word_t address, uint8_t b)
{
    if(address > VM_MEM_SEGMENT_SIZE || address < 0)
    {
        panic();
    }
    impl->ms[static_cast<size_t>(address)] = b;
}

uint8_t vm::get_mem_byte(word_t address)
{
    if(address > VM_MEM_SEGMENT_SIZE || address < 0)
    {
        panic();
    }
    return impl->ms[static_cast<size_t>(address)];
}

reg &vm::r(uint8_t i)             { return impl->m_r[i];}
const reg &vm::r(uint8_t i) const { return impl->m_r[i];}

reg_subbyte* vm::rsb(uint8_t ridx, uint8_t bidx)
{
    impl->t1.m_r = &r(ridx);
    impl->t1.m_bidx = bidx;
    return &impl->t1;
}

memaddress* vm::mem(word_t address)
{
    auto setter = [&](word_t a, word_t v) -> void
        {
            set_mem(a,v);
        };
    auto getter = [&](word_t a) -> word_t
        {
            return get_mem(a);
        };

    impl->ma_i = !impl->ma_i;
    impl->ma[impl->ma_i].m_address = address;
    impl->ma[impl->ma_i].m_setter = setter;
    impl->ma[impl->ma_i].m_getter = getter;
    return &impl->ma[impl->ma_i];
}

memaddress_byte_ref* vm::mem_byte(word_t address)
{
    auto setter = [&](word_t a, uint8_t v) -> void
    {
        set_mem_byte(a,v);
    };
    auto getter = [&](word_t a) -> uint8_t
    {
        return get_mem_byte(a);
    };

    impl->mb_i = !impl->mb_i;
    impl->mb[impl->mb_i].m_address = address;
    impl->mb[impl->mb_i].m_setter = setter;
    impl->mb[impl->mb_i].m_getter = getter;
    return &impl->mb[impl->mb_i];
}


immediate *vm::imm(word_t v)
{
    impl->mi_i++; if(impl->mi_i == 3) impl->mi_i = 1;

    impl->imv[impl->mi_i].m_value = v;
    return &impl->imv[impl->mi_i];
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
            word_t vaddr = fetch_immediate();
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
            word_t vaddr = fetch_immediate();
            return mem_byte(vaddr);
        }

        case type_destination::TYPE_MOD_MEM_REG_IDX_OFFS:
        {
            uint8_t ridx = fetch_register_index();
            word_t vaddr = fetch_immediate();
            return mem(r(ridx).value() + vaddr);
        }
    }

    panic();

}

word_t vm::flag() const {return impl->m_lbo.value();}

word_t &vm::flag() {return impl->m_lbo.value();}

bool vm::call(word_t v)
{
    impl->m_ip = VM_MEM_SEGMENT_SIZE + v;
    return impl->m_ip < VM_MEM_SEGMENT_SIZE + impl->app_size;
}

bool vm::interrupt(word_t i)
{
    std::function<bool(vm*)> fun;
    if(impl->interrupts.count(i) > 0)
    {
        return impl->interrupts[i].runner(this);
    }
    return false;
}

bool vm::address_is_valid(word_t addr)
{
    return addr < impl->app_size + VM_MEM_SEGMENT_SIZE && addr >= 0;
}

bool vm::copy(word_t dest, word_t src, word_t cnt)
{
    if(dest + cnt > VM_MEM_SEGMENT_SIZE || src < 0 || dest < 0 || src > VM_MEM_SEGMENT_SIZE)
    {
        return false;
    }
    std::memmove(&impl->ms[static_cast<size_t>(dest)], &impl->ms[static_cast<size_t>(src)], static_cast<size_t>(cnt));
    return true;
}

bool vm::push(const valued* v)
{
    if(!v) return false;

    // value
    set_mem(impl->sp.value(), v->value());
    impl->sp += word_size;
    if(impl->sp > impl->max_used_sp) impl->max_used_sp = impl->sp.value();
    if(impl->sp.value() > VM_MEM_SEGMENT_SIZE)
    {
        panic();
    }

    return true;
}

bool vm::push(const word_t v)
{
    immediate i(v);
    return push(&i);
}

word_t vm::pop()
{
    if(impl->sp.value() - word_size < 0)
    {
        panic();
    }
    word_t v = get_mem(impl->sp.value() - word_size);
    impl->sp -= word_size;
    return v;
}
