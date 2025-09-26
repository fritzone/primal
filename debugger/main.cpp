#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <map>
#include <memory>
#include <algorithm>
#include <cmath>

#include <hal.h>
#include <numeric_decl.h>
#include <function.h>
#include <types.h>
#include <opcodes.h>
#include <util.h>
#include <vm.h>

const char* FONT_FILENAME = "VGA9.F16";
const int FONT_CHAR_WIDTH = 8;
const int FONT_CHAR_HEIGHT = 16;
const int FONT_NUM_CHARS = 256;
const int FONT_BYTES_PER_CHAR = 16;
const int MAIN_VIEW_COLS = 100;
const int MAIN_VIEW_ROWS = 40;
const int REGS_VIEW_COLS = 40;
const int REGS_VIEW_ROWS = 30;
const int MEM_VIEW_COLS = 75;
const int MEM_VIEW_ROWS = 30;

// --- Turbo Debugger Colors ---
#define TD_BLUE          SDL_Color{0x00, 0x00, 0xA8, 0xFF}
#define TD_LIGHT_BLUE    SDL_Color{0x58, 0x58, 0xFC, 0xFF}
#define TD_CYAN          SDL_Color{0x00, 0xA8, 0xA8, 0xFF}
#define TD_WHITE         SDL_Color{0xF8, 0xF8, 0xF8, 0xFF}
#define TD_BLACK         SDL_Color{0x00, 0x00, 0x00, 0xFF}
#define TD_BRIGHT_WHITE  SDL_Color{0xFF, 0xFF, 0xFF, 0xFF}
#define TD_MAGENTA       SDL_Color{0xA8, 0x00, 0xA8, 0xFF}
#define TD_YELLOW        SDL_Color{0xF8, 0xF8, 0x54, 0xFF}
#define TD_GRAY          SDL_Color{0xA8, 0xA8, 0xA8, 0xFF}

// --- Data Structures ---
struct VGAChar {
    unsigned char character_code;
    SDL_Color fg_color;
    SDL_Color bg_color;
};

struct TextScreen {
    int cols;
    int rows;
    std::vector<VGAChar> buffer;
};

struct DisassembledInstruction {
    uint32_t address;
    std::string mnemonic;
    std::vector<std::string> params;
    std::vector<uint8_t> bytes;
};


struct DebuggerWindow {
    SDL_Window* sdl_window = nullptr;
    SDL_Renderer* sdl_renderer = nullptr;
    Uint32 window_id = 0;
    TextScreen screen;
    bool is_visible = true;
};


// --- Global State ---
enum class WindowState {
    MAIN_DEBUG = 0,
    FUNCTION_TABLE,
    STRING_TABLE,
    FILE_INFO,
    SETTINGS,
    HELP
};
WindowState g_current_view = WindowState::MAIN_DEBUG;

// --- Data Loaded from .pric file ---
std::vector<uint8_t> g_bytecode;
std::vector<DisassembledInstruction> g_disassembly;
std::vector<std::string> g_function_table_lines;
std::vector<std::string> g_string_table_lines;
std::vector<std::string> g_file_info_lines;
int g_scroll_y = 0;
int g_cursor_y = 0;
int g_regs_scroll_y = 0;
int g_mem_scroll_y = 0;

// --- Helper Functions from primdis ---
template<typename T>
T read_value(const std::vector<uint8_t>& bytecode, word_t& offset) {
    if (offset + sizeof(T) > bytecode.size()) {
        throw std::runtime_error("Unexpected end of file while reading value.");
    }
    T value;
    memcpy(&value, &bytecode[offset], sizeof(T));
    offset += sizeof(T);
    return value;
}

std::string read_lp_string(const std::vector<uint8_t>& bytecode, word_t& offset) {
    uint8_t len = read_value<uint8_t>(bytecode, offset);
    if (offset + len > bytecode.size()) {
        throw std::runtime_error("Unexpected end of file while reading string.");
    }
    std::string value(reinterpret_cast<const char*>(&bytecode[offset]), len);
    offset += len;
    return value;
}

using OpcodeMap = std::map<uint8_t, std::shared_ptr<primal::opcodes::opcode>>;

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

std::string format_parameter(const std::vector<uint8_t>& bytecode, word_t& ip) {
    std::stringstream ss;
    auto type = static_cast<type_destination>(read_value<uint8_t>(bytecode, ip));
    switch (type) {
    case type_destination::TYPE_MOD_IMM:
        ss << "0x" << std::hex << htovm(read_value<word_t>(bytecode, ip));
        break;
    case type_destination::TYPE_MOD_REG:
        ss << "$r" << static_cast<int>(read_value<uint8_t>(bytecode, ip));
        break;
    case type_destination::TYPE_MOD_MEM_IMM:
        ss << "[0x" << std::hex << htovm(read_value<word_t>(bytecode, ip)) << "]";
        break;
    case type_destination::TYPE_MOD_MEM_REG_IDX:
        ss << "[$r" << static_cast<int>(read_value<uint8_t>(bytecode, ip)) << "]";
        break;
    case type_destination::TYPE_MOD_MEM_REG_BYTE:
        ss << "[@$r" << static_cast<int>(read_value<uint8_t>(bytecode, ip)) << "]";
        break;
    case type_destination::TYPE_MOD_MEM_REG_IDX_OFFS: {
        uint8_t reg_idx = read_value<uint8_t>(bytecode, ip);
        char op = read_value<char>(bytecode, ip);
        word_t offset = htovm(read_value<word_t>(bytecode, ip));
        ss << "[$r" << static_cast<int>(reg_idx) << op << "0x" << std::hex << offset << "]";
        break;
    }
    case type_destination::TYPE_MOD_MEM_REG_IDX_REG_OFFS: {
        uint8_t reg1_idx = read_value<uint8_t>(bytecode, ip);
        char op = read_value<char>(bytecode, ip);
        uint8_t reg2_idx = read_value<uint8_t>(bytecode, ip);
        ss << "[$r" << static_cast<int>(reg1_idx) << op << "$r" << static_cast<int>(reg2_idx) << "]";
        break;
    }
    default:
        ss << "<?>";
        break;
    }
    return ss.str();
}

void disassemble_bytecode(const std::vector<uint8_t>& bytecode, std::vector<DisassembledInstruction>& out_instructions) {
    const auto opcode_map = create_opcode_map();
    word_t header_offset = 4 + word_size + word_size + word_size + word_size;
    word_t func_table_offset = htovm(read_value<word_t>(bytecode, header_offset));

    word_t ip = PRIMAL_HEADER_SIZE;
    while (ip < func_table_offset) {
        DisassembledInstruction instr;
        instr.address = static_cast<uint32_t>(ip - PRIMAL_HEADER_SIZE);
        size_t line_start_ip = ip;
        uint8_t opcode_val = read_value<uint8_t>(bytecode, ip);

        if (opcode_val == 0xFF) {
            instr.mnemonic = "EXIT";
            instr.bytes.push_back(opcode_val);
            out_instructions.push_back(instr);
            break;
        }

        auto it = opcode_map.find(opcode_val);
        if (it == opcode_map.end()) {
            instr.mnemonic = "DB";
            std::stringstream ss;
            ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode_val);
            instr.params.push_back(ss.str());
            instr.bytes.push_back(opcode_val);
            out_instructions.push_back(instr);
            continue;
        }

        const auto& opcode_obj = it->second;
        instr.mnemonic = opcode_obj->name();

        size_t params_ip_start = ip;
        for (size_t i = 0; i < opcode_obj->paramcount(); ++i) {
            instr.params.push_back(format_parameter(bytecode, ip));
        }
        size_t params_ip_end = ip;

        for (size_t i = line_start_ip; i < params_ip_end; ++i) {
            instr.bytes.push_back(bytecode[i]);
        }
        out_instructions.push_back(instr);
    }
}

void populate_function_table(const std::vector<uint8_t>& bytecode, std::vector<std::string>& out_lines) {
    word_t header_offset = 0;
    // Read offsets from header to find the function table
    header_offset += 4; // Skip to function table offset, jump over .P10
    word_t func_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));
    std::cout << "--- Function Symbol Table  @ " << func_table_offset << std::endl;
    word_t funcs_offs = func_table_offset + 4;
    word_t func_table_count = htovm(util::read_value<word_t>(bytecode, funcs_offs));

    std::cout << "--- Function Symbol Table (" << func_table_count << " entries) ---" << std::endl;
    std::stringstream ss;
    word_t current_offset = funcs_offs;
    for (word_t i = 0; i < func_table_count; ++i) {
        ss.str(""); ss.clear();
        ss << "Function #" << i + 1 << ":";
        out_lines.push_back(ss.str());

        std::string name = read_lp_string(bytecode, current_offset);
        word_t address = htovm(read_value<word_t>(bytecode, current_offset));
        bool is_extern = read_value<uint8_t>(bytecode, current_offset);
        auto return_type = static_cast<primal::entity_type>(read_value<uint8_t>(bytecode, current_offset));
        uint8_t param_count = read_value<uint8_t>(bytecode, current_offset);

        ss.str(""); ss.clear(); ss << "  Name:    " << name; out_lines.push_back(ss.str());
        ss.str(""); ss.clear(); ss << "  Address: 0x" << std::hex << address << std::dec; out_lines.push_back(ss.str());
        ss.str(""); ss.clear(); ss << "  Extern:  " << (is_extern ? "Yes" : "No"); out_lines.push_back(ss.str());
        ss.str(""); ss.clear(); ss << "  Return:  " << primal::to_string(return_type); out_lines.push_back(ss.str());

        ss.str(""); ss.clear(); ss << "  Params (" << (int)param_count << "):";
        if (param_count > 0) {
            ss << " ";
            for (uint8_t p = 0; p < param_count; ++p) {
                auto param_type = static_cast<primal::entity_type>(read_value<uint8_t>(bytecode, current_offset));
                ss << primal::to_string(param_type) << (p == param_count - 1 ? "" : ", ");
            }
        }
        out_lines.push_back(ss.str());
        out_lines.push_back("");
    }
}
void populate_string_table(const std::vector<uint8_t>& bytecode, std::vector<std::string>& out_lines) {
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
        std::stringstream ss;
        word_t str_addr = current_offset;
        std::string str = read_lp_string(bytecode, current_offset);
        ss << std::setw(4) << str_index++ << " [0x" << std::hex << str_addr << std::dec << "]: \"" << str << "\"";
        out_lines.push_back(ss.str());
    }
}
void populate_header_info(const std::vector<uint8_t>& bytecode, std::vector<std::string>& out_lines) {
    std::stringstream ss;
    word_t offset = 0;
    std::string signature(reinterpret_cast<const char*>(&bytecode[offset]), 4);
    offset += 4;
    ss << "Signature:            " << signature; out_lines.push_back(ss.str()); ss.str("");
    offset += word_size; // Skip reserved
    word_t stack_start_words = htovm(read_value<word_t>(bytecode, offset));
    ss << "Stack Start (words):  " << stack_start_words << " (at " << stack_start_words * word_size << " bytes)"; out_lines.push_back(ss.str()); ss.str("");
    word_t string_table_offset = htovm(read_value<word_t>(bytecode, offset));
    ss << "String Table Offset:  0x" << std::hex << string_table_offset << std::dec; out_lines.push_back(ss.str()); ss.str("");
    word_t func_table_offset = htovm(read_value<word_t>(bytecode, offset));
    ss << "Function Table Offset:0x" << std::hex << func_table_offset << std::dec; out_lines.push_back(ss.str()); ss.str("");
    word_t func_table_count = htovm(read_value<word_t>(bytecode, offset));
    ss << "Function Count:       " << func_table_count; out_lines.push_back(ss.str()); ss.str("");
}


// --- Text UI Functions ---
void screen_clear(TextScreen* screen, SDL_Color fg, SDL_Color bg) {
    for (int i = 0; i < screen->cols * screen->rows; ++i) {
        screen->buffer[i] = { ' ', fg, bg };
    }
}

void screen_put_char(TextScreen* screen, int x, int y, char c, SDL_Color fg, SDL_Color bg) {
    if (x >= 0 && x < screen->cols && y >= 0 && y < screen->rows) {
        screen->buffer[y * screen->cols + x] = { (unsigned char)c, fg, bg };
    }
}

void screen_print(TextScreen* screen, int x, int y, const std::string& str, SDL_Color fg, SDL_Color bg) {
    for (size_t i = 0; i < str.length(); ++i) {
        if (x + i < (size_t)screen->cols) {
            screen_put_char(screen, x + i, y, str[i], fg, bg);
        }
    }
}

void draw_char_on_renderer(SDL_Renderer* renderer, const unsigned char* font_data, unsigned char char_code, int x, int y, SDL_Color fg, SDL_Color bg, float cell_w, float cell_h) {
    // Calculate cell boundaries precisely to avoid gaps due to floating point inaccuracies.
    int x_start = (int)round(x * cell_w);
    int y_start = (int)round(y * cell_h);
    int x_end = (int)round((x + 1) * cell_w);
    int y_end = (int)round((y + 1) * cell_h);
    int width = x_end - x_start;
    int height = y_end - y_start;

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_Rect bg_rect = { x_start, y_start, width, height };
    SDL_RenderFillRect(renderer, &bg_rect);

    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, fg.a);
    const unsigned char* glyph = font_data + (int)char_code * FONT_BYTES_PER_CHAR;

    // The pixel drawing logic is now relative to the precise cell boundaries.
    float pixel_w = (float)width / FONT_CHAR_WIDTH;
    float pixel_h = (float)height / FONT_CHAR_HEIGHT;

    for (int row = 0; row < FONT_CHAR_HEIGHT; ++row) {
        for (int col = 0; col < FONT_CHAR_WIDTH; ++col) {
            if ((glyph[row] >> (7 - col)) & 1) {
                SDL_FRect pixel_rect = {
                    x_start + col * pixel_w,
                    y_start + row * pixel_h,
                    pixel_w,
                    pixel_h
                };
                SDL_RenderFillRectF(renderer, &pixel_rect);
            }
        }
    }
}

void render_text_screen(SDL_Renderer* renderer, TextScreen* screen, const unsigned char* font_data) {
    int window_w, window_h;
    SDL_GetRendererOutputSize(renderer, &window_w, &window_h);

    float cell_w = (float)window_w / screen->cols;
    float cell_h = (float)window_h / screen->rows;
    if (cell_w < 1.0f || cell_h < 1.0f) return;

    for (int y = 0; y < screen->rows; ++y) {
        for (int x = 0; x < screen->cols; ++x) {
            VGAChar vga_char = screen->buffer[y * screen->cols + x];
            draw_char_on_renderer(renderer, font_data, vga_char.character_code, x, y, vga_char.fg_color, vga_char.bg_color, cell_w, cell_h);
        }
    }
}


void draw_window_frame(TextScreen* screen, const std::string& title) {
    SDL_Color frame_fg = TD_BRIGHT_WHITE;
    SDL_Color frame_bg = TD_CYAN;
    SDL_Color title_fg = TD_BLACK;
    SDL_Color shadow_bg = {0,0,0,128};

    /*for (int y = 1; y < screen->rows; ++y) {
        screen_put_char(screen, screen->cols - 1, y, 219, shadow_bg, shadow_bg);
    }
    for (int x = 1; x <= screen->cols; ++x) {
        screen_put_char(screen, x, screen->rows - 2, 220, shadow_bg, shadow_bg);
    }*/

    for (int x = 0; x <= screen->cols - 1; ++x) {
        screen_put_char(screen, x, 0, 205, frame_fg, frame_bg);
        screen_put_char(screen, x, screen->rows - 2, 205, frame_fg, frame_bg);
    }
    for (int y = 1; y < screen->rows - 2; ++y) {
        screen_put_char(screen, 0, y, 186, frame_fg, frame_bg);
        screen_put_char(screen, screen->cols - 1, y, 186, frame_fg, frame_bg);
    }
    screen_put_char(screen, 0, 0, 201, frame_fg, frame_bg);
    screen_put_char(screen, screen->cols - 1, 0, 187, frame_fg, frame_bg);
    screen_put_char(screen, 0, screen->rows - 2, 200, frame_fg, frame_bg);
    screen_put_char(screen, screen->cols - 1, screen->rows - 2, 188, frame_fg, frame_bg);

    if(title.size() == 0) return;

    std::string display_title = " " + title + " ";
    int title_start_x = (screen->cols - 2 - display_title.length()) / 2;
    screen_print(screen, title_start_x, 0, display_title, title_fg, frame_bg);
}

void draw_status_bar(TextScreen* screen) {
    for (int x = 0; x < screen->cols; ++x) {
        screen_put_char(screen, x, screen->rows - 1, ' ', TD_BLACK, TD_CYAN);
    }

    const char* bar_text[] = { "F1-Help", "F2-Main", "F3-Funcs", "F4-Strs", "F5-Info", "F6-Set", "F7-Step", "F8-Regs", "F9-Mem", "F10-Exit" };
    int x = 1;
    for (int i = 0; i < 10; ++i) {
        std::string part = bar_text[i];
        if (part.empty()) { x += 8; continue; }
        screen_print(screen, x, screen->rows - 1, part.substr(0, 2), TD_BLACK, TD_CYAN);
        screen_print(screen, x + 2, screen->rows - 1, part.substr(2), TD_BRIGHT_WHITE, TD_CYAN);
        x += part.length() + 3;
    }
}

void populate_screen_from_lines(TextScreen* screen, const std::vector<std::string>& lines, int scroll_y) {
    int max_visible_lines = screen->rows - 3;
    for (int i = 0; i < max_visible_lines; ++i) {
        int line_idx = i + scroll_y;
        if (line_idx < (int)lines.size()) {
            screen_print(screen, 2, i + 1, lines[line_idx], TD_WHITE, TD_BLUE);
        }
    }
}

void draw_disassembly_window(TextScreen* screen, int scroll_y, int cursor_y) {
    draw_window_frame(screen, "");
    const int max_visible_lines = screen->rows - 3 - 4; // Space for frame and status window

    // Draw vertical separators that integrate with the frame
    for (int y = 1; y < screen->rows - 3; ++y) {
        screen_put_char(screen, 10, y, 186, TD_BRIGHT_WHITE, TD_CYAN); // After address
        screen_put_char(screen, 38, y, 186, TD_BRIGHT_WHITE, TD_CYAN); // After bytes
    }
    screen_put_char(screen, 10, 0, 203, TD_BRIGHT_WHITE, TD_CYAN); // T-junction top
    screen_put_char(screen, 38, 0, 203, TD_BRIGHT_WHITE, TD_CYAN);
    screen_put_char(screen, 10, screen->rows - 3, 202, TD_BRIGHT_WHITE, TD_CYAN); // T-junction bottom
    screen_put_char(screen, 38, screen->rows - 3, 202, TD_BRIGHT_WHITE, TD_CYAN);


    for (int i = 0; i < max_visible_lines; ++i) {
        int instr_idx = i + scroll_y;
        if (instr_idx >= (int)g_disassembly.size()) break;

        const auto& instr = g_disassembly[instr_idx];
        SDL_Color bg_color = (instr_idx == cursor_y) ? TD_CYAN : TD_BLUE;
        SDL_Color default_fg = (instr_idx == cursor_y) ? TD_BLACK : TD_WHITE;
        SDL_Color mnemonic_fg = (instr_idx == cursor_y) ? TD_BLACK : TD_YELLOW;
        SDL_Color reg_fg = (instr_idx == cursor_y) ? TD_BLACK : TD_BRIGHT_WHITE;

        // 1. Address
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << instr.address;
        screen_print(screen, 1, i + 1, ss.str(), TD_GRAY, bg_color);

        // 2. Raw Bytes
        ss.str(""); ss.clear();
        for(size_t j = 0; j < 8 && j < instr.bytes.size(); ++j) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)instr.bytes[j] << " ";
        }
        if (instr.bytes.size() > 8) { ss << ".."; }
        screen_print(screen, 12, i + 1, ss.str(), default_fg, bg_color);

        // 3. Disassembly
        int current_x = 40;
        screen_print(screen, current_x, i + 1, instr.mnemonic, mnemonic_fg, bg_color);
        current_x += instr.mnemonic.length() + 1;

        for (const auto& param : instr.params) {
            if (param.find('$') != std::string::npos) {
                screen_print(screen, current_x, i + 1, param, reg_fg, bg_color);
            } else {
                screen_print(screen, current_x, i + 1, param, default_fg, bg_color);
            }
            current_x += param.length() + 2; // +2 for comma and space
        }
    }
}

void draw_current_instruction_status(TextScreen* screen, const DisassembledInstruction& instr) {
    int y = screen->rows - 5;
    SDL_Color fg = TD_WHITE;
    SDL_Color bg = TD_BLUE;

    // Clear the 3 lines
    for (int line = 0; line < 3; ++line) {
        for (int x = 1; x < screen->cols - 2; ++x) {
            screen_put_char(screen, x, y + line, ' ', fg, bg);
        }
    }

    // 1. Address
    std::stringstream ss;
    ss << "Address: 0x" << std::hex << std::setw(8) << std::setfill('0') << instr.address;
    screen_print(screen, 2, y++, ss.str(), fg, bg);

    // 2. Disassembly
    ss.str(""); ss.clear();
    ss << "Disassembly: " << instr.mnemonic << " ";
    for(size_t i = 0; i < instr.params.size(); ++i) {
        ss << instr.params[i] << (i == instr.params.size() - 1 ? "" : ", ");
    }
    screen_print(screen, 2, y++, ss.str(), fg, bg);

    // 3. Full Bytes
    ss.str(""); ss.clear();
    ss << "Bytes: ";
    for(uint8_t byte : instr.bytes) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    }
    screen_print(screen, 2, y++, ss.str(), fg, bg);
}

void draw_register_window(TextScreen* screen, int scroll_y) {
    draw_window_frame(screen, "Registers");

    screen_put_char(screen, 0, 8, 204, TD_BRIGHT_WHITE, TD_CYAN);
    for (int x = 1; x < screen->cols - 1; ++x) {
        screen_put_char(screen, x, 8, 205, TD_BRIGHT_WHITE, TD_CYAN);
    }
    screen_put_char(screen, screen->cols - 2, 8, 185, TD_BRIGHT_WHITE, TD_CYAN);

    screen_print(screen, 2, 1, "$r250 (IP): 0x00000000", TD_WHITE, TD_BLUE);
    screen_print(screen, 2, 2, "$r251 (MEMSIZE): 1000", TD_WHITE, TD_BLUE);
    screen_print(screen, 2, 3, "$r252 (STACK_START): 0x0", TD_WHITE, TD_BLUE);
    screen_print(screen, 2, 4, "$r253 (LOF): 0", TD_WHITE, TD_BLUE);
    screen_print(screen, 2, 5, "$r254 (FP): 0x0", TD_WHITE, TD_BLUE);
    screen_print(screen, 2, 6, "$r255 (SP): 0x0", TD_WHITE, TD_BLUE);

    int y = 9;
    int regs_per_row = 2;
    int visible_rows = screen->rows - 3 - y;
    for (int i = 0; i < visible_rows * regs_per_row; ++i) {
        int reg_idx = i + scroll_y * regs_per_row;
        if (reg_idx >= 250) break;

        int row = y + i / regs_per_row;
        int col = 2 + (i % regs_per_row) * (screen->cols / regs_per_row);

        std::stringstream ss;
        ss << "$r" << std::left << std::setw(3) << reg_idx << ": " << std::right << std::setw(8) << 0;
        screen_print(screen, col, row, ss.str(), TD_WHITE, TD_BLUE);
    }
}


void draw_memory_window(TextScreen* screen, const std::vector<uint8_t>& mem_data, int scroll_y) {
    draw_window_frame(screen, "Memory View");

    int bytes_per_row = 16;
    int visible_rows = screen->rows - 3; // -2 for frame, -1 for status bar (if it had one)

    for (int i = 0; i < visible_rows; ++i) {
        int row_addr = (i + scroll_y) * bytes_per_row;
        if (row_addr >= (int)mem_data.size()) break;

        // 1. Address Column
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << row_addr;
        screen_print(screen, 1, i + 1, ss.str(), TD_GRAY, TD_BLUE);

        // 2. Hex Column
        ss.str(""); ss.clear();
        for (int j = 0; j < bytes_per_row; ++j) {
            int current_addr = row_addr + j;
            if (current_addr < (int)mem_data.size()) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)mem_data[current_addr] << " ";
            } else {
                ss << "   ";
            }
        }
        screen_print(screen, 10, i + 1, ss.str(), TD_WHITE, TD_BLUE);

        // 3. ASCII Column
        ss.str(""); ss.clear();
        for (int j = 0; j < bytes_per_row; ++j) {
            int current_addr = row_addr + j;
            if (current_addr < (int)mem_data.size()) {
                char c = static_cast<char>(mem_data[current_addr]);
                ss << (c?:'.');
            } else {
                ss << " ";
            }
        }
        screen_print(screen, 10 + bytes_per_row * 3, i + 1, ss.str(), TD_YELLOW, TD_BLUE);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.pric>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open input file '" << filename << "'" << std::endl;
        return 1;
    }
    g_bytecode.assign(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
        );

    try {
        populate_header_info(g_bytecode, g_file_info_lines);
        populate_function_table(g_bytecode, g_function_table_lines);
        populate_string_table(g_bytecode, g_string_table_lines);
        disassemble_bytecode(g_bytecode, g_disassembly);
    } catch (const std::exception& e) {
        std::cerr << "Error processing bytecode: " << e.what() << std::endl;
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);

    DebuggerWindow main_window;
    main_window.screen.cols = MAIN_VIEW_COLS;
    main_window.screen.rows = MAIN_VIEW_ROWS;
    main_window.screen.buffer.resize(MAIN_VIEW_COLS * MAIN_VIEW_ROWS);
    main_window.sdl_window = SDL_CreateWindow("Primal Debugger", 100, 100, MAIN_VIEW_COLS * 9, MAIN_VIEW_ROWS * 18, SDL_WINDOW_RESIZABLE);
    main_window.sdl_renderer = SDL_CreateRenderer(main_window.sdl_window, -1, SDL_RENDERER_ACCELERATED);
    main_window.window_id = SDL_GetWindowID(main_window.sdl_window);

    DebuggerWindow regs_window;
    regs_window.screen.cols = REGS_VIEW_COLS;
    regs_window.screen.rows = REGS_VIEW_ROWS;
    regs_window.screen.buffer.resize(REGS_VIEW_COLS * REGS_VIEW_ROWS);
    regs_window.sdl_window = SDL_CreateWindow("Registers", 100 + MAIN_VIEW_COLS * 9 + 20, 100, REGS_VIEW_COLS * 9, REGS_VIEW_ROWS * 18, SDL_WINDOW_RESIZABLE);
    regs_window.sdl_renderer = SDL_CreateRenderer(regs_window.sdl_window, -1, SDL_RENDERER_ACCELERATED);
    regs_window.window_id = SDL_GetWindowID(regs_window.sdl_window);

    DebuggerWindow mem_window;
    mem_window.screen.cols = MEM_VIEW_COLS;
    mem_window.screen.rows = MEM_VIEW_ROWS;
    mem_window.screen.buffer.resize(MEM_VIEW_COLS * MEM_VIEW_ROWS);
    mem_window.sdl_window = SDL_CreateWindow("Memory", 120, 120, MEM_VIEW_COLS * 9, MEM_VIEW_ROWS * 18, SDL_WINDOW_RESIZABLE);
    mem_window.sdl_renderer = SDL_CreateRenderer(mem_window.sdl_window, -1, SDL_RENDERER_ACCELERATED);
    mem_window.window_id = SDL_GetWindowID(mem_window.sdl_window);
    mem_window.is_visible = true;

    unsigned char font_data[FONT_NUM_CHARS * FONT_BYTES_PER_CHAR];
    FILE* font_file = fopen(FONT_FILENAME, "rb");
    if (!font_file) {
        printf("Could not open font file: %s\n", FONT_FILENAME);
        return 1;
    }
    fread(font_data, 1, FONT_NUM_CHARS * FONT_BYTES_PER_CHAR, font_file);
    fclose(font_file);

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                if (e.window.windowID == main_window.window_id) { quit = true; }
                else if (e.window.windowID == regs_window.window_id) { regs_window.is_visible = false; SDL_HideWindow(regs_window.sdl_window); }
                else if (e.window.windowID == mem_window.window_id) { mem_window.is_visible = false; SDL_HideWindow(mem_window.sdl_window); }
            }
            if (e.type == SDL_KEYDOWN) {
                int max_regs_scroll = 0;
                if(REGS_VIEW_ROWS > 12) {
                    max_regs_scroll = (int)ceil(250.0 / 2.0) - (REGS_VIEW_ROWS - 12);
                }

                switch(e.key.keysym.sym) {
                case SDLK_F1: g_current_view = WindowState::HELP; g_scroll_y = 0; g_cursor_y = 0; break;
                case SDLK_F2: g_current_view = WindowState::MAIN_DEBUG; g_scroll_y = 0; g_cursor_y = 0; break;
                case SDLK_F3: g_current_view = WindowState::FUNCTION_TABLE; g_scroll_y = 0; g_cursor_y = 0; break;
                case SDLK_F4: g_current_view = WindowState::STRING_TABLE; g_scroll_y = 0; g_cursor_y = 0; break;
                case SDLK_F5: g_current_view = WindowState::FILE_INFO; g_scroll_y = 0; g_cursor_y = 0; break;
                case SDLK_F6: g_current_view = WindowState::SETTINGS; g_scroll_y = 0; g_cursor_y = 0; break;
                case SDLK_F8:
                    regs_window.is_visible = !regs_window.is_visible;
                    if (regs_window.is_visible) SDL_ShowWindow(regs_window.sdl_window);
                    else SDL_HideWindow(regs_window.sdl_window);
                    break;
                case SDLK_F9: // NEW
                    mem_window.is_visible = !mem_window.is_visible;
                    if (mem_window.is_visible) SDL_ShowWindow(mem_window.sdl_window);
                    else SDL_HideWindow(mem_window.sdl_window);
                    break;
                case SDLK_F10: quit = true; break;
                case SDLK_UP:
                    if (SDL_GetKeyboardFocus() == regs_window.sdl_window) {
                        g_regs_scroll_y = std::max(0, g_regs_scroll_y - 1);
                    } else if (g_current_view == WindowState::MAIN_DEBUG) {
                        g_cursor_y = std::max(0, g_cursor_y - 1);
                    } else {
                        g_scroll_y = std::max(0, g_scroll_y - 1);
                    }
                    break;
                case SDLK_DOWN:
                    if (SDL_GetKeyboardFocus() == regs_window.sdl_window) {
                        g_regs_scroll_y = std::min(max_regs_scroll, g_regs_scroll_y + 1);
                    } else if (g_current_view == WindowState::MAIN_DEBUG) {
                        if (!g_disassembly.empty()) g_cursor_y = std::min((int)g_disassembly.size() - 1, g_cursor_y + 1);
                    } else g_scroll_y++;
                    break;
                }
            }
            if (e.type == SDL_MOUSEWHEEL) {
                if (e.window.windowID == regs_window.window_id) {
                    int max_regs_scroll = (int)ceil(250.0 / 2.0) - (REGS_VIEW_ROWS - 12);
                    if(e.wheel.y > 0) g_regs_scroll_y = std::max(0, g_regs_scroll_y - 1);
                    else if(e.wheel.y < 0) g_regs_scroll_y = std::min(max_regs_scroll, g_regs_scroll_y + 1);
                } else if (e.window.windowID == main_window.window_id) {
                    if(e.wheel.y > 0) g_scroll_y = std::max(0, g_scroll_y - 1);
                    else if(e.wheel.y < 0) g_scroll_y++;
                } else if (e.window.windowID == mem_window.window_id) {
                    if(e.wheel.y > 0) g_mem_scroll_y = std::max(0, g_mem_scroll_y - 1);
                    else if(e.wheel.y < 0) g_mem_scroll_y++;
                }
            }
        }

        // --- Render Main Window ---
        SDL_SetRenderDrawColor(main_window.sdl_renderer, 0, 0, 0, 255);
        SDL_RenderClear(main_window.sdl_renderer);
        screen_clear(&main_window.screen, TD_WHITE, TD_BLUE);
        draw_status_bar(&main_window.screen);

        const std::vector<std::string>* current_lines = nullptr;
        std::string title;

        switch(g_current_view) {
        case WindowState::MAIN_DEBUG: break; // Handled separately
        case WindowState::FUNCTION_TABLE: title = "Function Table"; current_lines = &g_function_table_lines; break;
        case WindowState::STRING_TABLE: title = "String Table"; current_lines = &g_string_table_lines; break;
        case WindowState::FILE_INFO: title = "File Information"; current_lines = &g_file_info_lines; break;
        case WindowState::SETTINGS: title = "Settings"; break;
        case WindowState::HELP: title = "Help"; break;
        }

        if (g_current_view == WindowState::MAIN_DEBUG) {
            int max_visible_lines = main_window.screen.rows - 3 - 4;
            if (g_cursor_y < g_scroll_y) { g_scroll_y = g_cursor_y; }
            if (g_cursor_y >= g_scroll_y + max_visible_lines) { g_scroll_y = g_cursor_y - max_visible_lines + 1; }
            draw_disassembly_window(&main_window.screen, g_scroll_y, g_cursor_y);
            if (g_cursor_y >= 0 && g_cursor_y < (int)g_disassembly.size()) {
                draw_current_instruction_status(&main_window.screen, g_disassembly[g_cursor_y]);
            }
        } else {
            draw_window_frame(&main_window.screen, title);
            if (current_lines) {
                int max_scroll = std::max(0, (int)current_lines->size() - (main_window.screen.rows - 3));
                g_scroll_y = std::min(g_scroll_y, max_scroll);
                populate_screen_from_lines(&main_window.screen, *current_lines, g_scroll_y);
            } else if (g_current_view == WindowState::HELP) {
                screen_print(&main_window.screen, 2, 2, "This is the Primal Debugger.", TD_BRIGHT_WHITE, TD_BLUE);
                screen_print(&main_window.screen, 2, 4, "Use F-keys to navigate between windows.", TD_WHITE, TD_BLUE);
                screen_print(&main_window.screen, 2, 5, "Use UP/DOWN arrows/mouse to scroll.", TD_WHITE, TD_BLUE);
                screen_print(&main_window.screen, 2, 7, "F7 and F10 will be used for execution control.", TD_GRAY, TD_BLUE);
            }
        }

        render_text_screen(main_window.sdl_renderer, &main_window.screen, font_data);
        SDL_RenderPresent(main_window.sdl_renderer);

        if (regs_window.is_visible) {
            SDL_SetRenderDrawColor(regs_window.sdl_renderer, 0, 0, 0, 255);
            SDL_RenderClear(regs_window.sdl_renderer);
            screen_clear(&regs_window.screen, TD_WHITE, TD_BLUE);
            draw_register_window(&regs_window.screen, g_regs_scroll_y);
            render_text_screen(regs_window.sdl_renderer, &regs_window.screen, font_data);
            SDL_RenderPresent(regs_window.sdl_renderer);
        }

        if (mem_window.is_visible) {
            SDL_SetRenderDrawColor(mem_window.sdl_renderer, 0, 0, 0, 255);
            SDL_RenderClear(mem_window.sdl_renderer);
            screen_clear(&mem_window.screen, TD_WHITE, TD_BLUE);

            int max_mem_scroll = std::max(0, ((int)g_bytecode.size() / 16) - (mem_window.screen.rows - 3));
            g_mem_scroll_y = std::min(g_mem_scroll_y, max_mem_scroll);

            draw_memory_window(&mem_window.screen, g_bytecode, g_mem_scroll_y);
            render_text_screen(mem_window.sdl_renderer, &mem_window.screen, font_data);
            SDL_RenderPresent(mem_window.sdl_renderer);
        }
    }

    SDL_DestroyRenderer(main_window.sdl_renderer);
    SDL_DestroyWindow(main_window.sdl_window);
    SDL_DestroyRenderer(regs_window.sdl_renderer);
    SDL_DestroyWindow(regs_window.sdl_window);
    SDL_DestroyRenderer(mem_window.sdl_renderer);
    SDL_DestroyWindow(mem_window.sdl_window);
    SDL_Quit();

    return 0;
}
