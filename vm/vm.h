#ifndef VM_H
#define VM_H

#include <hal.h>

#include <vector>
#include <map>
#include <functional>
#include <memory>

class vm final
{

public:

    vm();

    static std::shared_ptr<vm> create();

    bool run(const std::vector<uint8_t>& app);

    // access function for the IP. You can use this to modify it though for faster access
    std::size_t& ip()      {return m_ip;}
    std::size_t ip() const {return m_ip;}

    // access function for the SP. You can use this to modify it though for faster access
    numeric_t& sp()      {return m_sp;}
    numeric_t sp() const {return m_sp;}

    // access function for the given regstser. You can use this to modify it though for faster access
    reg& r(uint8_t i)             { return m_r[i];}
    const reg& r(uint8_t i) const { return m_r[i];}

    type_destination fetch_type_dest();
    uint8_t fetch_register_index();
    numeric_t fetch_immediate();

    template<class OPC, class EXECUTOR>
    static void register_opcode(OPC&& o, EXECUTOR&& ex)
    {
        auto f = [&](vm* machina) -> bool {return ex(machina);};
        executor t;
        t.opcode_runner = std::function<bool(vm*)>(f);
        vm_runner[o.bin()] = t;
    };

    void panic();

private:

    struct executor
    {
        std::function<bool(vm*)> opcode_runner;
    };

    static std::map<uint8_t, executor> vm_runner;

    reg m_r[VM_REG_COUNT];              // the registers of the machine
    std::size_t m_ip = 0;               // the instructions pointer
    numeric_t& m_sp;                    // the stack pointer

    /* the memory segment */
    std::unique_ptr<uint8_t[]> ms;
};

#endif
