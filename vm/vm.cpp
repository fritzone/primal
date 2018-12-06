#include "vm.h"
#include "vm_impl.h"

#include <interrupts.h>

#include <exceptions.h>
#include <opcodes.h>
#include <hal.h>
#include <types.h>
#include <util.h>

#include <algorithm>
#include <algorithm>
#include <memory>
#include <sstream>

using namespace primal;


vm::vm() : impl(new vm_impl)
{
}

bool vm::run(const std::vector<uint8_t> &app)
{
    return impl->run(app, this);
}

word_t &vm::ip()      {return impl->ip();}

word_t vm::ip() const {return impl->ip();}

void vm::set_mem(word_t address, word_t new_value)
{
    impl->set_mem(address, new_value);
}

word_t vm::get_mem(word_t address)
{
    return impl->get_mem(address);
}

void vm::set_mem_byte(word_t address, uint8_t b)
{
    impl->set_mem_byte(address, b);
}

uint8_t vm::get_mem_byte(word_t address)
{
    return impl->get_mem_byte(address);
}

std::shared_ptr<vm> vm::create()
{
    // these functions are autogenerated by cmake in their own files
    register_opcodes();
    register_interrupts();

    return std::make_shared<vm>();
}


valued *vm::fetch()
{
    return impl->fetch();
}

reg &vm::r(uint8_t i)             { return impl->m_r[i];}
const reg &vm::r(uint8_t i) const { return impl->m_r[i];}

bool vm::copy(word_t dest, word_t src, word_t cnt)
{
    return impl->copy(dest, src, cnt);
}

bool vm::push(const valued *v)
{
    return impl->push(v);
}

bool vm::push(const word_t v)
{
    return impl->push(v);
}

word_t vm::pop()
{
    return impl->pop();
}


word_t vm::flag() const {return impl->flag();}

void vm::set_flag(word_t v)
{
    impl->m_r[253].set_value( v );
}

bool vm::jump(word_t v)
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
    else
    {
        panic();
    }
}

bool vm::address_is_valid(word_t addr)
{
    return addr <= impl->app_size + VM_MEM_SEGMENT_SIZE && addr >= 0;
}


void vm::panic()
{
    impl->panic();
}
