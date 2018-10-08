#include "catch.hpp"

#include <vm.h>
#include <compiler.h>

TEST_CASE("Compiler compiles", "[compiler]")
{
    auto c = compiler::initalize();
    c->compile(":a_label\nlet x = -2\nlet y = 5\n if x != 3 then\n let x = y + 3 \ngoto a_label\nendif\nlet x=5\ngoto a_label");
    auto vm = vm::create();
    vm->run(c->bytecode());
}

/*
TEST_CASE("ASM compiler", "[compiler]")
{
    auto c = compiler::initalize();
    c->compile("asm MOV $r1 20");
    auto vm = vm::create();
    vm->run(c->bytecode());
    REQUIRE(vm->r(1) == 20);

}

*/