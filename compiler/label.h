#ifndef LABEL_H
#define LABEL_H

#include "sequence.h"

#include <string>

namespace primal
{
/* Simple class to represent a label in the assembly output */
    class label : public sequence
    {
    public:

        explicit label(source&);
        explicit label(source& src, const std::string& s) : sequence(src), lblname(s), m_absolute(true) {}

        static void reset_counter();

        static label create(source&);
        std::string name() const;
        void set_name(const std::string& name) { lblname = name; }
        bool absolute() const {return m_absolute; }

        prepared_type prepare(std::vector<token>& tokens) override;
        bool compile(compiler*) override;

    private:
        static word_t label_counter;

    private:
        std::string lblname = "";
        bool m_absolute = false;
    };

/* This represents the declaration of a label in the code */
    class declare_label
    {
    public:
        explicit declare_label(label l) : the_label(std::move(l)) {}
        label get_label() const {return the_label;}

    private:

        label the_label;
    };

}

#endif //PRIMITIVE_LABEL_H
