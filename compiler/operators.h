#ifndef OPERATORS_H
#define OPERATORS_H

#include "util.h"
#include <opcodes.h>

#include <map>
#include <memory>

// operator precedence
static constexpr int PRIO_10 = 10;   // addition and subtraction
static constexpr int PRIO_20 = 20;   // multiplication, division and modulo
static constexpr int PRIO_30 = 30;   // and
static constexpr int PRIO_40 = 40;   // or
static constexpr int PRIO_50 = 50;   // comparison

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

// All the operators in the system
static std::map<std::string, std::unique_ptr<ops>> operators = util::create_map<std::string, std::unique_ptr<ops>>
    ("+",  util::make_unique<ops> ("+", PRIO_10, new opcodes::ADD))
    ("-",  util::make_unique<ops> ("-", PRIO_10, new opcodes::SUB))
    ("*",  util::make_unique<ops> ("*", PRIO_20, new opcodes::MUL))
    ("/",  util::make_unique<ops> ("/", PRIO_20, new opcodes::DIV))
    ("%",  util::make_unique<ops> ("%", PRIO_20, new opcodes::MOD))
    ("|",  util::make_unique<ops> ("|", PRIO_40, new opcodes::OR ))
    ("&",  util::make_unique<ops> ("&", PRIO_30, new opcodes::AND))
    ("==", util::make_unique<comp>("==",PRIO_50, new opcodes::EQ, new opcodes::DJT ))
    ("!=", util::make_unique<comp>("!=",PRIO_50, new opcodes::EQ, new opcodes::DJNT))
    ("<",  util::make_unique<comp>("<", PRIO_50, new opcodes::LT, new opcodes::DJT ))
    (">",  util::make_unique<comp>(">", PRIO_50, new opcodes::GT, new opcodes::DJT ))
    ("<=", util::make_unique<comp>("<=",PRIO_50, new opcodes::LTE,new opcodes::DJT ))
    (">=", util::make_unique<comp>(">=",PRIO_50, new opcodes::GTE,new opcodes::DJT ))
;

#endif
