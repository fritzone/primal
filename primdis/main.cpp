#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdint>
#include <cstring>

// Primal-specific headers
#include <hal.h>
#include <numeric_decl.h>
#include <function.h> // For function_summary_pod and entity_type
#include <types.h>    // For to_string(entity_type)
#include <opcodes.h> // Includes all opcode definition headers

#include <util.h>

std::string xml_escape(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.length());
    for (char c : str) {
        switch (c) {
        case '&':  escaped += "&amp;"; break;
        case '<':  escaped += "&lt;"; break;
        case '>':  escaped += "&gt;"; break;
        case '"': escaped += "&quot;"; break;
        case '\'': escaped += "&apos;"; break;
        default:   escaped += c; break;
        }
    }
    return escaped;
}

void print_header(const std::vector<uint8_t>& bytecode) {
    std::cout << "--- Header Information ---" << std::endl;
    word_t offset = 0;

    // 1. Signature
    std::string signature(reinterpret_cast<const char*>(&bytecode[offset]), 4);
    offset += 4;
    std::cout << "Signature:            " << signature << std::endl;
    if (signature != ".P10") {
        std::cerr << "Warning: Invalid or unknown file signature." << std::endl;
    }
    // 2. Function Table Offset
    word_t func_table_offset = (util::read_value<word_t>(bytecode, offset));
    std::cout << "Function Table Offset:0x" << std::hex << func_table_offset << std::dec << std::endl;

    // 3. Stack Start Offset (in words)
    word_t stack_start_words = (util::read_value<word_t>(bytecode, offset));
    std::cout << "Stack Start (words):  " << stack_start_words << " (at " << stack_start_words * word_size << " bytes)" << std::endl;

    // 4. String Table Offset
    word_t string_table_offset = (util::read_value<word_t>(bytecode, offset));
    std::cout << "String Table Offset:  0x" << std::hex << string_table_offset << std::dec << std::endl;


}

// Map to hold instances of all opcode objects, indexed by their binary code.
using OpcodeMap = std::map<uint8_t, std::shared_ptr<primal::opcodes::opcode>>;

// Function to create and populate the opcode map.
OpcodeMap create_opcode_map() {
    OpcodeMap opcodes;
    opcodes[primal::opcodes::MOV().bin()] = std::make_shared<primal::opcodes::MOV>();
    opcodes[primal::opcodes::ADD().bin()] = std::make_shared<primal::opcodes::ADD>();
    opcodes[primal::opcodes::SUB().bin()] = std::make_shared<primal::opcodes::SUB>();
    opcodes[primal::opcodes::MUL().bin()] = std::make_shared<primal::opcodes::MUL>();
    opcodes[primal::opcodes::DIV().bin()] = std::make_shared<primal::opcodes::DIV>();
    opcodes[primal::opcodes::MOD().bin()] = std::make_shared<primal::opcodes::MOD>();
    opcodes[primal::opcodes::OR().bin()] = std::make_shared<primal::opcodes::OR>();
    opcodes[primal::opcodes::AND().bin()] = std::make_shared<primal::opcodes::AND>();
    opcodes[primal::opcodes::EQ().bin()] = std::make_shared<primal::opcodes::EQ>();
    opcodes[primal::opcodes::NEQ().bin()] = std::make_shared<primal::opcodes::NEQ>();
    opcodes[primal::opcodes::JT().bin()] = std::make_shared<primal::opcodes::JT>();
    opcodes[primal::opcodes::JNT().bin()] = std::make_shared<primal::opcodes::JNT>();
    opcodes[primal::opcodes::JMP().bin()] = std::make_shared<primal::opcodes::JMP>();
    opcodes[primal::opcodes::LT().bin()] = std::make_shared<primal::opcodes::LT>();
    opcodes[primal::opcodes::GT().bin()] = std::make_shared<primal::opcodes::GT>();
    opcodes[primal::opcodes::LTE().bin()] = std::make_shared<primal::opcodes::LTE>();
    opcodes[primal::opcodes::GTE().bin()] = std::make_shared<primal::opcodes::GTE>();
    opcodes[primal::opcodes::PUSH().bin()] = std::make_shared<primal::opcodes::PUSH>();
    opcodes[primal::opcodes::POP().bin()] = std::make_shared<primal::opcodes::POP>();
    opcodes[primal::opcodes::COPY().bin()] = std::make_shared<primal::opcodes::COPY>();
    opcodes[primal::opcodes::DJT().bin()] = std::make_shared<primal::opcodes::DJT>();
    opcodes[primal::opcodes::DJNT().bin()] = std::make_shared<primal::opcodes::DJNT>();
    opcodes[primal::opcodes::DJMP().bin()] = std::make_shared<primal::opcodes::DJMP>();
    opcodes[primal::opcodes::XOR().bin()] = std::make_shared<primal::opcodes::XOR>();
    opcodes[primal::opcodes::NOT().bin()] = std::make_shared<primal::opcodes::NOT>();
    opcodes[primal::opcodes::CALL().bin()] = std::make_shared<primal::opcodes::CALL>();
    opcodes[primal::opcodes::RET().bin()] = std::make_shared<primal::opcodes::RET>();
    opcodes[primal::opcodes::INTR().bin()] = std::make_shared<primal::opcodes::INTR>();
    opcodes[primal::opcodes::INC().bin()] = std::make_shared<primal::opcodes::INC>();
    return opcodes;
}

// Decodes and returns a string for a single parameter from the bytecode stream.
std::string disassemble_parameter(const std::vector<uint8_t>& bytecode, word_t& ip) {
    std::stringstream ss;
    auto type = static_cast<type_destination>(util::read_value<uint8_t>(bytecode, ip));
    switch (type) {
    case type_destination::TYPE_MOD_IMM: {
        word_t val = htovm(util::read_value<word_t>(bytecode, ip));
        ss << "0x" << std::hex << val << " (" << std::dec << val << ")";
        break;
    }
    case type_destination::TYPE_MOD_REG:
        ss << "$r" << static_cast<int>(util::read_value<uint8_t>(bytecode, ip));
        break;
    case type_destination::TYPE_MOD_MEM_IMM: {
        word_t val = htovm(util::read_value<word_t>(bytecode, ip));
        ss << "[0x" << std::hex << val << " (" << std::dec << val << ")]";
        break;
    }
    case type_destination::TYPE_MOD_MEM_REG_IDX:
        ss << "[$r" << static_cast<int>(util::read_value<uint8_t>(bytecode, ip)) << "]";
        break;
    case type_destination::TYPE_MOD_MEM_REG_BYTE:
        ss << "[@$r" << static_cast<int>(util::read_value<uint8_t>(bytecode, ip)) << "]";
        break;
    case type_destination::TYPE_MOD_MEM_REG_IDX_OFFS: {
        uint8_t reg_idx = util::read_value<uint8_t>(bytecode, ip);
        char op = util::read_value<char>(bytecode, ip);
        word_t offset = htovm(util::read_value<word_t>(bytecode, ip));
        ss << "[$r" << static_cast<int>(reg_idx) << op << "0x" << std::hex << offset << " (" << std::dec << offset << ")]";
        break;
    }
    case type_destination::TYPE_MOD_MEM_REG_IDX_REG_OFFS: {
        uint8_t reg1_idx = util::read_value<uint8_t>(bytecode, ip);
        char op = util::read_value<char>(bytecode, ip);
        uint8_t reg2_idx = util::read_value<uint8_t>(bytecode, ip);
        ss << "[$r" << static_cast<int>(reg1_idx) << op << "$r" << static_cast<int>(reg2_idx) << "]";
        break;
    }
    default:
        ss << "<?>";
        break;
    }
    return ss.str();
}


// The main disassembler function.
void disassemble_bytecode(const std::vector<uint8_t>& bytecode) {
    const auto opcode_map = create_opcode_map();

    word_t header_offset = 4 + word_size + word_size; // Skip to string table offset
    word_t string_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));

    std::cout << "--- Disassembly ---" << std::endl;

    word_t ip = PRIMAL_HEADER_SIZE;
    while (ip < string_table_offset) {
        size_t line_start_ip = ip;
        uint8_t opcode_val = util::read_value<uint8_t>(bytecode, ip);

        // Print Address
        std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex << (line_start_ip - PRIMAL_HEADER_SIZE) << ":  ";

        if (opcode_val == 0xFF) {
            std::cout << "EXIT                                            # ff" << std::endl;
            continue;
        }

        auto it = opcode_map.find(opcode_val);
        if (it == opcode_map.end()) {
            std::cout << "DB 0x" << std::hex << static_cast<int>(opcode_val) << " (UNKNOWN)" << std::dec << std::endl;
            continue;
        }

        const auto& opcode_obj = it->second;

        // Get parameters as a string
        std::stringstream params_ss;
        for (word_t i = 0; i < opcode_obj->paramcount(); ++i) {
            params_ss << disassemble_parameter(bytecode, ip) << " ";
        }
        size_t line_end_ip = ip;

        // Print Mnemonic and Operands, aligned
        std::cout << std::setw(8) << std::setfill(' ') << std::left << opcode_obj->name()
                  << std::setw(40) << std::left << params_ss.str();

        // Print raw bytes for the instruction
        std::cout << "# ";
        for (size_t i = line_start_ip; i < line_end_ip; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytecode[i]) << " ";
        }
        std::cout << std::dec << std::endl;
    }
    std::cout << "-------------------\n" << std::endl;
}


void print_function_table(const std::vector<uint8_t>& bytecode) {
    word_t header_offset = 0;
    // Read offsets from header to find the function table
    header_offset += 4; // Skip to function table offset, jump over .P10
    word_t func_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));
    std::cout << "--- Function Symbol Table  @ " << func_table_offset << std::endl;
    word_t funcs_offs = func_table_offset + 4;
    word_t func_table_count = htovm(util::read_value<word_t>(bytecode, funcs_offs));

    std::cout << "--- Function Symbol Table (" << func_table_count << " entries) ---" << std::endl;

    word_t current_offset = funcs_offs;
    for (word_t i = 0; i < func_table_count; ++i) {
        std::cout << "Function #" << i + 1 << ":" << std::endl;

        // 1. Name
        std::string name = util::read_lp_string(bytecode, current_offset);
        std::cout << "  Name:    " << name << std::endl;

        // 2. Address
        word_t address = htovm(util::read_value<word_t>(bytecode, current_offset));
        std::cout << "  Address: 0x" << std::hex << address << std::dec << "(" << address << ")" << std::endl;

        // 3. Is Extern
        bool is_extern = util::read_value<uint8_t>(bytecode, current_offset);
        std::cout << "  Extern:  " << (is_extern ? "Yes" : "No") << std::endl;

        // 4. Return Type
        auto return_type = static_cast<primal::entity_type>(util::read_value<uint8_t>(bytecode, current_offset));
        std::cout << "  Return:  " << primal::to_string(return_type) << std::endl;

        // 5. Parameters
        uint8_t param_count = util::read_value<uint8_t>(bytecode, current_offset);
        std::cout << "  Params (" << (int)param_count << "):";
        if (param_count > 0) {
            std::cout << " ";
            for (uint8_t p = 0; p < param_count; ++p) {
                auto param_type = static_cast<primal::entity_type>(util::read_value<uint8_t>(bytecode, current_offset));
                std::cout << primal::to_string(param_type) << (p == param_count - 1 ? "" : ", ");
            }
        }
        std::cout << std::endl << std::endl;
    }
    std::cout << "-------------------------------------------\n" << std::endl;
}

void print_string_table(const std::vector<uint8_t>& bytecode) {
    word_t header_offset = 0;
    // Read offsets from header to find the function table
    header_offset += 4; // Skip to function table offset, jump over .P10
    word_t func_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));
    header_offset += word_size; // Skip to string table offset
    word_t string_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));


    std::cout << "--- String Table ---" << std::endl;

    word_t current_offset = string_table_offset;
    int str_index = 0;
    while (current_offset < func_table_offset) {
        word_t str_addr = current_offset;
        std::string str = util::read_lp_string(bytecode, current_offset);
        std::cout << std::setw(4) << str_index++ << " [0x" << std::hex << str_addr << std::dec << "]: \"" << str << "\"" << std::endl;
    }
    std::cout << "--------------------\n" << std::endl;
}

void print_bytecode_dump(const std::vector<uint8_t>& bytecode) {
    std::cout << "--- Bytecode Hex Dump ---" << std::endl;
    const size_t bytes_per_line = 16;
    for (size_t i = PRIMAL_HEADER_SIZE; i < bytecode.size(); ++i) {
        if ((i - PRIMAL_HEADER_SIZE) % bytes_per_line == 0) {
            if (i != PRIMAL_HEADER_SIZE) {
                std::cout << std::endl;
            }
            std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << (i - PRIMAL_HEADER_SIZE) << ": ";
        }
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytecode[i]) << " ";
    }
    std::cout << std::dec << "\n-------------------------" << std::endl;
}


// XML printers
void print_header_xml(std::ostream& os, const std::vector<uint8_t>& bytecode) {
    os << "  <Header>\n";
    word_t offset = 0;
    std::string signature(reinterpret_cast<const char*>(&bytecode[offset]), 4);
    offset += 4;
    os << "    <Signature>" << xml_escape(signature) << "</Signature>\n";
    offset += word_size; // Skip reserved
    word_t stack_start_words = htovm(util::read_value<word_t>(bytecode, offset));
    os << "    <StackStart words=\"" << stack_start_words << "\" bytes=\"" << stack_start_words * word_size << "\"/>\n";
    word_t string_table_offset = htovm(util::read_value<word_t>(bytecode, offset));
    os << "    <StringTableOffset>0x" << std::hex << string_table_offset << std::dec << "</StringTableOffset>\n";
    word_t func_table_offset = htovm(util::read_value<word_t>(bytecode, offset));
    os << "    <FunctionTableOffset>0x" << std::hex << func_table_offset << std::dec << "</FunctionTableOffset>\n";
    word_t func_table_count = htovm(util::read_value<word_t>(bytecode, offset));
    os << "    <FunctionCount>" << func_table_count << "</FunctionCount>\n";
    os << "  </Header>\n";
}

void print_function_table_xml(std::ostream& os, const std::vector<uint8_t>& bytecode) {

    word_t header_offset = 0;
    // Read offsets from header to find the function table
    header_offset += 4; // Skip to function table offset, jump over .P10
    word_t func_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));
    word_t funcs_offs = func_table_offset + 4;
    word_t func_table_count = htovm(util::read_value<word_t>(bytecode, funcs_offs));
    word_t current_offset = funcs_offs;
    for (word_t i = 0; i < func_table_count; ++i) {
        os << "    <Function id=\"" << i + 1 << "\">\n";
        std::string name = util::read_lp_string(bytecode, current_offset);
        os << "      <Name>" << xml_escape(name) << "</Name>\n";
        word_t address = htovm(util::read_value<word_t>(bytecode, current_offset));
        os << "      <Address>0x" << std::hex << address << std::dec << "</Address>\n";
        bool is_extern = util::read_value<uint8_t>(bytecode, current_offset);
        os << "      <Extern>" << (is_extern ? "true" : "false") << "</Extern>\n";
        auto return_type = static_cast<primal::entity_type>(util::read_value<uint8_t>(bytecode, current_offset));
        os << "      <Return type=\"" << primal::to_string(return_type) << "\"/>\n";
        uint8_t param_count = util::read_value<uint8_t>(bytecode, current_offset);
        os << "      <Parameters count=\"" << (int)param_count << "\">\n";
        for (uint8_t p = 0; p < param_count; ++p) {
            auto param_type = static_cast<primal::entity_type>(util::read_value<uint8_t>(bytecode, current_offset));
            os << "        <Parameter id=\"" << (int)p << "\" type=\"" << primal::to_string(param_type) << "\"/>\n";
        }
        os << "      </Parameters>\n";
        os << "    </Function>\n";
    }
    os << "  </FunctionTable>\n";
}

void print_string_table_xml(std::ostream& os, const std::vector<uint8_t>& bytecode) {
    word_t header_offset = 0;
    // Read offsets from header to find the function table
    header_offset += 4; // Skip to function table offset, jump over .P10
    word_t func_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));
    header_offset += word_size; // Skip to string table offset
    word_t string_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));
    os << "  <StringTable>\n";


    word_t current_offset = string_table_offset;
    int str_index = 0;
    while (current_offset < func_table_offset) {
        word_t str_addr = current_offset;
        std::string str = util::read_lp_string(bytecode, current_offset);
        os << "    <String id=\"" << str_index++ << "\" address=\"0x" << std::hex << str_addr << std::dec << "\">" << xml_escape(str) << "</String>\n";
    }
    os << "  </StringTable>\n";


}

void print_parameter_xml(std::ostream& os, const std::vector<uint8_t>& bytecode, word_t& ip) {
    word_t start_ip = ip;
    auto type = static_cast<type_destination>(util::read_value<uint8_t>(bytecode, ip));
    std::string value_str = disassemble_parameter(bytecode, start_ip);
    ip = start_ip;

    os << "      <Operand type=\"" << to_string(type) << "\" value=\"" << xml_escape(value_str) << "\"/>\n";
}

void disassemble_bytecode_xml(std::ostream& os, const std::vector<uint8_t>& bytecode) {
    const auto opcode_map = create_opcode_map();
    word_t header_offset = 4 + word_size + word_size;
    word_t string_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));

    os << "  <Disassembly>\n";
    word_t ip = PRIMAL_HEADER_SIZE;
    while (ip < string_table_offset) {
        word_t line_start_ip = ip;
        uint8_t opcode_val = util::read_value<uint8_t>(bytecode, ip);

        std::stringstream bytes_ss;
        bytes_ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode_val);

        if (opcode_val == 0xFF) {
            os << "    <Instruction address=\"0x" << std::hex << (line_start_ip - PRIMAL_HEADER_SIZE) << std::dec << "\" mnemonic=\"EXIT\" bytes=\"" << bytes_ss.str() << "\"/>\n";
            break;
        }

        auto it = opcode_map.find(opcode_val);
        if (it == opcode_map.end()) {
            os << "    <Instruction address=\"0x" << std::hex << (line_start_ip - PRIMAL_HEADER_SIZE) << std::dec << "\" mnemonic=\"UNKNOWN\" bytes=\"" << bytes_ss.str() << "\"/>\n";
            continue;
        }

        const auto& opcode_obj = it->second;
        os << "    <Instruction address=\"0x" << std::hex << (line_start_ip - PRIMAL_HEADER_SIZE) << std::dec << "\" mnemonic=\"" << opcode_obj->name() << "\">\n";

        for (word_t i = 0; i < opcode_obj->paramcount(); ++i) {
            print_parameter_xml(os, bytecode, ip);
        }

        word_t line_end_ip = ip;
        for (word_t i = line_start_ip + 1; i < line_end_ip; ++i) {
            bytes_ss << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytecode[i]);
        }
        os << "      <Bytes>" << bytes_ss.str() << "</Bytes>\n";
        os << "    </Instruction>\n";
    }
    os << "  </Disassembly>\n";
}


int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <input.pric> [--xml]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open input file '" << filename << "'" << std::endl;
        return 1;
    }
    bool xml_output = (argc == 3 && std::string(argv[2]) == "--xml");

    std::vector<uint8_t> bytecode(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (bytecode.size() < PRIMAL_HEADER_SIZE) {
        std::cerr << "Error: File is too small to be a valid Primal binary." << std::endl;
        return 1;
    }

    try {
        if (xml_output) {
            std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            std::cout << "<PrimalFile path=\"" << xml_escape(filename) << "\">\n";
            print_header_xml(std::cout, bytecode);
            print_function_table_xml(std::cout, bytecode);
            print_string_table_xml(std::cout, bytecode);
            disassemble_bytecode_xml(std::cout, bytecode);
            std::cout << "</PrimalFile>\n";
        } else {
            print_header(bytecode);
            print_function_table(bytecode);
            print_string_table(bytecode);
            disassemble_bytecode(bytecode);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred while parsing the file: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
