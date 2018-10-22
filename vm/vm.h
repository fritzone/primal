#ifndef VM_H
#define VM_H

#include <hal.h>

#include <vector>
#include <map>
#include <functional>
#include <memory>

using namespace std::placeholders;

class vm final
{

public:

    vm();

    static std::shared_ptr<vm> create();

    // will run the given application
    bool run(const std::vector<uint8_t>& app);

    // access function for the IP. You can use this to modify it though for faster access
    std::size_t& ip()      {return m_ip;}
    std::size_t ip() const {return m_ip;}

    // will set the memory at the given addres to the new value
    void set_mem(size_t address, numeric_t new_value);
    numeric_t get_mem(size_t address);

    void set_mem_byte(size_t address, uint8_t b);
    uint8_t get_mem_byte(size_t address);

    // access function for the given register. You can use this to modify it though for faster access
    reg& r(uint8_t i)             { return m_r[i];}
    const reg& r(uint8_t i) const { return m_r[i];}

    // access function for a given sub reg byte
    reg_subbyte* rsb(uint8_t ridx, uint8_t bidx);

    // access the memory at the given location
    memaddress* mem(numeric_t address);
    memaddress_byte_ref* mem_byte(numeric_t address);
    immediate* imm(numeric_t v);

    type_destination fetch_type_dest();
    uint8_t fetch_register_index();
    numeric_t fetch_immediate();

    bool copy(numeric_t dest, numeric_t src, numeric_t cnt);

    template<class OPC, class EXECUTOR>
    static void register_opcode(OPC&& o, EXECUTOR&& ex)
    {
        auto f = [&](vm* machina) -> bool {return ex(machina);};
        executor t;
        t.opcode_runner = std::function<bool(vm*)>(f);
        vm_runner[o.bin()] = t;
    };

    [[noreturn]] void panic() ;

    valued* fetch();

    // the flag of the last operation
    bool flag() const {return m_lbo;}
    bool& flag() {return m_lbo;}

private:

    struct executor
    {
        std::function<bool(vm*)> opcode_runner;
    };

    static std::map<uint8_t, executor> vm_runner;

    reg m_r[VM_REG_COUNT];              // the registers of the machine
    std::size_t m_ip = 0;               // the instructions pointer
    std::unique_ptr<uint8_t[]> ms;      // the memory segment

    reg_subbyte t1 {&r(0), 0};
    immediate imv {-1};
    memaddress_byte_ref mb[2];
    memaddress ma[2];
    int mb_i = 0;
    int ma_i = 0;
    bool m_lbo = false; // the last operations' result. This is set to false if the result was 0


};

#endif
