#ifndef CODE_STREAM_H
#define CODE_STREAM_H

#include <memory>
#include <vector>
#include <map>

#include <opcodes.h>
#include <types.h>

namespace primal
{
    namespace opcodes { struct opcode; }

    class variable;
    struct reg;
    class token;
    class label;
    class declare_label;
    class compiler;

    class compiled_code final
    {
    public:
        static compiled_code& instance(compiler*);

        explicit compiled_code(compiler* c) : m_compiler(c) {}

        compiled_code() = default;
        ~compiled_code() = default;

        void append(uint8_t b);
        void append_number(numeric_t v);

        void encountered(const label&);
        void declare_label(const label&);

        void string_encountered(int strtbl_idx);

        numeric_t location() const
        {
            return bytes.size();
        }

        /* This will place all the prelimary labels to the correct value */
        void finalize();

        const std::vector<uint8_t>& bytecode() const { return bytes; }

        /* Removes the instance of the compiler from the internals of the system, since it was destructed */
        void destroy();

    private:
        std::vector<uint8_t> bytes;

        /* holds the map where a specific label was encountered in the code, such as jmp _label_2*/
        std::map<std::string, std::vector<uint32_t>> label_encounters;

        /* holds the locations where a specific string was encountered in the code */
        std::map<int, std::vector<uint32_t>> string_encounters;

        /* holds the map where a specific label was declared in the code, such as :_label_2*/
        std::map<std::string, numeric_t> label_declarations;

        static std::map<const compiler*, std::shared_ptr<compiled_code>> compilers_codes;
        compiler* m_compiler = nullptr;
    };

    class generate final
    {
    public:

        generate(compiler*);
        ~generate();

        generate& operator << (opcodes::opcode&& opc);
        generate& operator << (std::shared_ptr<opcodes::opcode> opc);
        generate& operator << (variable&& var);
        generate& operator << (std::shared_ptr<variable> var);
        generate& operator << (reg&& r);
        generate& operator << (const token& tok);
        generate& operator << (const label& l);
        generate& operator << (declare_label&& dl);
        generate& operator << (type_destination td);
        generate& operator << (numeric_t v);

    private:
        compiler* m_compiler;
        size_t m_current_binseq_start = static_cast<size_t>(-1);
        size_t m_current_binseq_end = static_cast<size_t>(-1);
        int params_sent = 0;
    };
}

#endif //PRIMITIVE_CODE_STREAM_H
