#include <vm.h>
#include <numeric_decl.h>
#include <vm_impl.h>
#include <iostream>

namespace primal
{

/* Intr1 does the following: Prints out the content of REG(2) of the virtual machine depending on the
 * value of REG1:
 *  - if REG1 is 0, the actual value of REG2 is printed out
 *  - if REG1 is not 0, the content of the memory cell pointed to by REG2 is printed out, the number of characters
 *    to be printed is REG1
 */
bool intr_1(vm* v)
{
    if(v->r(1) == 0)
    {
        std::cout << v->r(2).value();
    }
    else
    {
        word_t addr = v->r(2).value();
        word_t len = v->r(1).value();
        while(len)
        {
            char c = static_cast<char>(v->get_mem_byte(addr));
            if(c != '\\')
            {
                std::cout << c;
                addr ++;
                len --;
            }
            else
            {
                addr ++;
                len --;
                if(len)
                {
                    c = static_cast<char>(v->get_mem_byte(addr));
                    addr ++;
                    len --;
                    switch(c)
                    {
                    case 'n': std::cout << std::endl;
                        break;
                    }
                }
            }
        }
    }
    return true;
}


}
