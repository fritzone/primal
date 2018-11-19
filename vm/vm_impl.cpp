#include "vm_impl.h"

using namespace primal;

std::map<uint8_t, vm_impl::executor> vm_impl::opcode_runners;
std::map<word_t, vm_impl::executor> vm_impl::interrupts;

vm_impl::vm_impl() : m_lbo(m_r[253]), sp(m_r[255])
{
    for(uint8_t i = 0; i<255; i++)
    {
        m_r[i].set_idx(i);
    }
}

vm_impl::~vm_impl()
{
    std::fill(ms.get(), ms.get() + VM_MEM_SEGMENT_SIZE + app_size, 0xFF);
}
