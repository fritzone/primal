#include "vm_impl.h"

#include <exceptions.h>

#include <cstring>
#include <iostream>
#include <iomanip>

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

bool vm_impl::run(const std::vector<uint8_t> &app, vm* v)
{
    // firstly set up the memory segment for this machine and initialize it to 0x00
    app_size = static_cast<word_t>(app.size());
    ms = std::make_unique<uint8_t[]>(static_cast<size_t>(app_size + VM_MEM_SEGMENT_SIZE));
    std::fill(ms.get(), ms.get() + VM_MEM_SEGMENT_SIZE + app_size, 0x00);

    // then copy over the data from app to the end of the memory segment
    std::copy(app.begin(), app.end(), ms.get() + VM_MEM_SEGMENT_SIZE);
    // set the IP and SP to point to the correct location
    m_ip = VM_MEM_SEGMENT_SIZE + PRIMAL_HEADER_SIZE; // will grow upwards, will skip .P10 and the stringtable loc entry
    stack_offset = *reinterpret_cast<word_t*>(ms.get() + VM_MEM_SEGMENT_SIZE + 8);
    sp = stack_offset * word_size;   // will grow upwards

    // set the size of the memory into reg 251
    m_r[251] = VM_MEM_SEGMENT_SIZE;
    m_r[252] = sp;

    // then start running it
    while(opcode_runners.count(ms[static_cast<size_t>(m_ip)]))
    {
        // read in an opcode
        uint8_t opc = ms[static_cast<size_t>(m_ip++)];

        try
        {
            // is there a registered opcode runner for the given opcode?
            if(!opcode_runners[ opc ].runner(v))
            {
                panic();
            }
        }
        catch (const primal::vm_panic& p)
        {
            throw;
        }
        catch(...)
        {
            panic();
        }

        // is the opcode after the current one 0xFF meaning: halt the machine?
        if(m_ip < 0)
        {
            panic();
        }
        if(ms[static_cast<size_t>(m_ip)] == 0xFF)
        {
            //bindump();
            return true;
        }
    }
    // theoretically we never should end up here, so let's just panic
    panic();
}

void vm_impl::panic()
{
    std::cout << "VM PANIC ☹ - instruction dump:\n---------------------------------------------------\n";
    word_t start = std::max<word_t>(VM_MEM_SEGMENT_SIZE, m_ip - 64);
    word_t end = m_ip + std::min<word_t>(64, app_size);
    bindump("PANIC", start, end, true);

    std::cout << "VM PANIC ☹ - memory dump:\n---------------------------------------------------\n";
    memdump(m_ip - 10, m_ip + 10);

    throw primal::vm_panic("PANIC");
}

void vm_impl::memdump(word_t start, word_t end, bool insert_addr)
{
    std::string s;
    std::stringstream ss;

    for(word_t i = start; i <  end; i++)
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
        ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase  << static_cast<int>(ms[static_cast<size_t>(i)]) ;

        if(ms[static_cast<size_t>(i)] > 32 && ms[static_cast<size_t>(i)] < 255)
        {
            s += static_cast<char>(ms[static_cast<size_t>(i)]);
        }
        else
        {
            s += '.';
        }

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
            std::cout << ss.str() << " | " << s << std::endl;
            s = "";
            ss.clear();
            ss.str(std::string());
            insert_addr = true;
        }
    }

    std::string sttrs = ss.str();
    while(sttrs.length() < 80) sttrs += " ";

    std::cout << sttrs << "    | " << s << std::endl;
}

void vm_impl::bindump(const char *title, word_t start, word_t end, bool insert_addr)
{

    if(start == -1) start = VM_MEM_SEGMENT_SIZE; //std::max<word_t>(VM_MEM_SEGMENT_SIZE, m_ip - 64);
    if(end == -1) end = VM_MEM_SEGMENT_SIZE + app_size;

    std::stringstream ss;

    if(title)
    {
        std::cout << "----" << title << "----" << std::endl;
    }

    memdump(start, end, insert_addr);

    // memory dump
    for(word_t i=0; i<stack_offset * word_size; i += word_size)
    {
        ss << std::right << " [" << std::setfill('0') << std::setw(8) << std::dec << i << "] = ";
        ss << get_mem(i) << std::endl;
    }
    ss << "-"; // indicates the stack start
    for(word_t i = stack_offset * word_size; i<max_used_sp + 4*word_size; i += word_size)
    {
        if(i == sp.value())
        {
            ss <<">" ; // current stack position
        }
        else
        {
            if (i > stack_offset * word_size) ss << " ";
        }

        ss << std::right << "[" << std::setfill('0') << std::setw(8) << std::dec << i << "] = ";
        ss << std::setfill('0') << std::setw(16) << std::dec << get_mem(i) << std::endl;
    }
    std::cout << ss.str();

    std::cout << "IP=" << std::dec << m_ip << "[:" << m_ip - VM_MEM_SEGMENT_SIZE << "] (" << std::hex << m_ip << ")" << std::endl;
    std::cout << "SP=" << std::dec << sp.value() << " (" << std::hex << sp.value() << ")" << std::endl;
    std::cout << std::endl;
}


reg_subbyte* vm_impl::rsb(uint8_t ridx, uint8_t bidx)
{
    t1.m_r = &m_r[ridx];
    t1.m_bidx = bidx;
    return &t1;
}

void vm_impl::set_mem(word_t address, word_t new_value)
{
    if(address > VM_MEM_SEGMENT_SIZE + app_size || address < 0)
    {
        panic();
    }

    std::memcpy( &ms[0] + address, &new_value, sizeof(new_value));
}

word_t vm_impl::get_mem(word_t address)
{
    if(address > VM_MEM_SEGMENT_SIZE + app_size || address < 0)
    {
        panic();
    }

    word_t v = 0;
    std::memcpy(&v, &ms[0] + address, sizeof(v));
    return v;
}

void vm_impl::set_mem_byte(word_t address, uint8_t b)
{
    if(address > VM_MEM_SEGMENT_SIZE + app_size || address < 0)
    {
        panic();
    }
    ms[static_cast<size_t>(address)] = b;
}

uint8_t vm_impl::get_mem_byte(word_t address)
{
    if(address > VM_MEM_SEGMENT_SIZE + app_size || address < 0)
    {
        panic();
    }
    return ms[static_cast<size_t>(address)];
}

memaddress* vm_impl::mem(word_t address)
{
    auto setter = [&](word_t a, word_t v) -> void
    {
        set_mem(a,v);
    };
    auto getter = [&](word_t a) -> word_t
    {
        return get_mem(a);
    };

    ma_i = !ma_i;
    ma[ma_i].m_address = address;
    ma[ma_i].m_setter = setter;
    ma[ma_i].m_getter = getter;
    return &ma[ma_i];
}

memaddress_byte_ref* vm_impl::mem_byte(word_t address)
{
    auto setter = [&](word_t a, uint8_t v) -> void
    {
        set_mem_byte(a,v);
    };
    auto getter = [&](word_t a) -> uint8_t
    {
        return get_mem_byte(a);
    };

    mb_i = !mb_i;
    mb[mb_i].m_address = address;
    mb[mb_i].m_setter = setter;
    mb[mb_i].m_getter = getter;
    return &mb[mb_i];
}


immediate *vm_impl::imm(word_t v)
{
    mi_i++; if(mi_i == 3) mi_i = 0;

    imv[mi_i].m_value = v;
    return &imv[mi_i];
}


type_destination vm_impl::fetch_type_dest()
{
    return static_cast<type_destination>(ms[static_cast<size_t>(m_ip ++)]) ;
}


uint8_t vm_impl::fetch_register_index()
{
    return ms[static_cast<size_t>(m_ip ++)];
}

word_t vm_impl::fetch_immediate()
{
    word_t retv = htovm(*(reinterpret_cast<word_t*>(ms.get() + m_ip)));
    m_ip += word_size;
    return retv;
}

uint8_t vm_impl::fetch_byte()
{
    return ms[static_cast<size_t>(m_ip ++)];
}

valued *vm_impl::fetch()
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
        return &(m_r[ridx]);
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
        return mem(m_r[ridx].value());
    }

    case type_destination::TYPE_MOD_MEM_REG_BYTE:
    {
        uint8_t ridx = fetch_register_index();
        return mem_byte(m_r[ridx].value());
    }

    case type_destination::TYPE_MOD_MEM_IMM_BYTE:
    {
        word_t vaddr = fetch_immediate();
        return mem_byte(vaddr);
    }

    case type_destination::TYPE_MOD_MEM_REG_IDX_OFFS:
    {
        uint8_t ridx = fetch_register_index();
        uint8_t op = fetch_byte();

        word_t vaddr = fetch_immediate();

        if(op == '+')
        {
            return mem_byte(m_r[ridx].value() + vaddr);
        }
        else if(op == '-')
        {
            return mem_byte(m_r[ridx].value() - vaddr);
        }
        else if(op == '*')
        {
            return mem_byte(m_r[ridx].value() * vaddr);
        }
        else if(op == '/')
        {
            return mem_byte(m_r[ridx].value() / vaddr);
        }
        break;
    }

    case type_destination::TYPE_MOD_MEM_REG_IDX_REG_OFFS:
    {
        uint8_t ridx = fetch_register_index();
        uint8_t op = fetch_byte();
        uint8_t ridx2 = fetch_register_index();
        if(op == '+')
        {
            return mem_byte( m_r[ridx].value() + m_r[ridx2].value() );
        }
        else if(op == '-')
        {
            return mem_byte( m_r[ridx].value() - m_r[ridx2].value() );
        }
        else if(op == '*')
        {
            return mem_byte( m_r[ridx].value() * m_r[ridx2].value() );
        }
        else if(op == '/')
        {
            return mem_byte( m_r[ridx].value() / m_r[ridx2].value() );
        }
    }
    }

    panic();

}

word_t &vm_impl::ip()      {return m_ip;}

word_t vm_impl::ip() const {return m_ip;}

word_t vm_impl::flag() const {return m_lbo.value();}

bool vm_impl::push(const valued *v)
{
    if(!v) return false;

    // value
    set_mem(sp.value(), v->value());
    sp += word_size;
    if(sp > max_used_sp) max_used_sp = sp.value();
    if(sp.value() > VM_MEM_SEGMENT_SIZE)
    {
        panic();
    }

    return true;
}

bool vm_impl::push(const word_t v)
{
    immediate i(v);
    return push(&i);
}

word_t vm_impl::pop()
{
    if(sp.value() - word_size < 0)
    {
        panic();
    }
    word_t v = get_mem(sp.value() - word_size);
    sp -= word_size;
    return v;
}

bool vm_impl::copy(word_t dest, word_t src, word_t cnt)
{
    if(dest + cnt > VM_MEM_SEGMENT_SIZE || src < 0 || dest < 0)
    {
        return false;
    }
#if 1
    std::cout << "************************************" << std::endl;
    std::cout << "COPY: src=" << src << " dest=" << dest << " cnt=" << cnt << std::endl;
    memdump(src, src + cnt);
    std::cout << "************************************" << std::endl;
#endif
    std::memmove(&ms[static_cast<size_t>(dest)], &ms[static_cast<size_t>(src)], static_cast<size_t>(cnt));

#if 1
    std::cout << "************************************" << std::endl;
    memdump(dest - 4, dest + 10 + cnt);
    std::cout << "************************************" << std::endl;
#endif
    return true;
}
