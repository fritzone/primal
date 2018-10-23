#ifndef OPERATORS_H
#define OPERATORS_H

#include "op_priv.h"
#include "util.h"

#include <map>
#include <memory>

namespace primal
{
// All the operators in the system
    static std::map<std::string, std::unique_ptr<ops>> operators = util::create_map<std::string, std::unique_ptr<ops>>
            ("+",  util::make_unique<ops> ("+", PRIO_10, new opcodes::ADD))
            ("-",  util::make_unique<ops> ("-", PRIO_10, new opcodes::SUB))
            ("*",  util::make_unique<ops> ("*", PRIO_20, new opcodes::MUL))
            ("/",  util::make_unique<ops> ("/", PRIO_20, new opcodes::DIV))
            ("%",  util::make_unique<ops> ("%", PRIO_20, new opcodes::MOD))
            ("|",  util::make_unique<ops> ("|", PRIO_40, new opcodes::OR ))
            ("^",  util::make_unique<ops> ("^", PRIO_40, new opcodes::XOR))
            ("&",  util::make_unique<ops> ("&", PRIO_30, new opcodes::AND))
            ("==", util::make_unique<comp>("==",PRIO_50, new opcodes::EQ, new opcodes::DJT ))
            ("!=", util::make_unique<comp>("!=",PRIO_50, new opcodes::EQ, new opcodes::DJNT))
            ("<",  util::make_unique<comp>("<", PRIO_50, new opcodes::LT, new opcodes::DJT ))
            (">",  util::make_unique<comp>(">", PRIO_50, new opcodes::GT, new opcodes::DJT ))
            ("<=", util::make_unique<comp>("<=",PRIO_50, new opcodes::LTE,new opcodes::DJT ))
            (">=", util::make_unique<comp>(">=",PRIO_50, new opcodes::GTE,new opcodes::DJT ))
    ;

}

#endif
