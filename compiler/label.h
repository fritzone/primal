#ifndef LABEL_H
#define LABEL_H

#include <string>

/* Simple class to represent a label in the assembly output */
class label
{
public:
    static label create();
    std::string name() const;

private:
    label();

private:
    static uint32_t label_counter;

private:
    std::string lblname = "";
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

#endif //PRIMITIVE_LABEL_H
