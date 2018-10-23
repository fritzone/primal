#include <opcodes.h>

// operator precedence
static constexpr int PRIO_10 = 10;   // addition and subtraction
static constexpr int PRIO_20 = 20;   // multiplication, division and modulo
static constexpr int PRIO_30 = 30;   // and
static constexpr int PRIO_40 = 40;   // or
static constexpr int PRIO_50 = 50;   // comparison

namespace primate
{
/* A structure holding an operator */
    struct ops
    {
        ops() = delete;
        virtual ~ops() = default;

        /* The operator as a string, the priority of it and the assigned opcode */
        ops(std::string o, int p, opcodes::opcode* opc) : op(std::move(o)), precedence(p), opcode(opc) {}

        std::string op;
        int precedence = -1;
        std::shared_ptr<opcodes::opcode> opcode;
    };

/* The comparision operators are also operators */
    struct comp : public ops
    {
        /* The operator as a string, the priority of it, the assigned opcode and the jump operation associated with this */
        comp(const std::string& o, int p, opcodes::opcode* opc, opcodes::opcode* jmpr) : ops(o, p, opc), jump(jmpr) {}

        std::shared_ptr<opcodes::opcode> jump;
    };
}
