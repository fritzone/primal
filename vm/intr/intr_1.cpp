#include <vm.h>
#include <numeric_decl.h>

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
    if(v->r(0) == 0)
    {
        std::cout << v->r(2).value();
    }
    else
    {
        numeric_t addr = v->r(2).value();
        if(!v->address_is_valid(addr))
        {
            return false;
        }

        numeric_t len = v->r(1).value();
        if(len < 0 || len > VM_MEM_SEGMENT_SIZE)
        {
            return false;
        }
        while(len)
        {
            std::cout << v->get_mem_byte(addr);
            addr ++;
            len --;
        }
    }
}


}
