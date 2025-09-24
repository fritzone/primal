#include "registers.h"
#include "util.h"
namespace primal {

std::string reg::debug() const {
    std::string res = "reg" + util::to_string(m_reg_idx) + "=(" + util::to_string(m_value) + ")";
    return res;
}

std::string memaddress::debug() const {
    return std::string("[") + util::to_string(m_address) + "]";
}

std::string memaddress_byte_ref::debug() const {
    return std::string("[@") + util::to_string(m_address) + "]";
}

std::string reg_subbyte::debug() const  {
    return  std::string("@") + util::to_string((int)m_bidx) + "/" + m_r->debug();
}

std::string immediate::debug() const {
    return util::to_string(m_value);
}

std::string immediate_byte::debug() const {
    return util::to_string(m_value);
}


}
