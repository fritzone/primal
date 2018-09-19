#ifndef VARIABLE_H
#define VARIABLE_H

#include <types.h>

#include <string>
#include <memory>
#include <map>


/* This class will hold the variables and their associated memory adresses */
class variable
{
public:

    explicit variable(const std::string& name);

    numeric_t location() const;

public:

    /* Tells us if the variable with the given name was created or not */
    static bool has_variable(const std::string& name);

    /*Creates a variable with the given name (if it does not exist) */
    static std::shared_ptr<variable> create(const std::string& name);

    static std::map<std::string, std::shared_ptr<variable>> variables;

private:

    std::string m_name;
    numeric_t m_location;

};

#endif

