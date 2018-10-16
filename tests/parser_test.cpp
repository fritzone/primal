#include "catch.hpp"

#include <vm.h>
#include <compiler.h>
#include <iostream>

/*
TEST_CASE("Compiler compiles", "[compiler]")
{
    auto c = compiler::initalize();
    c->compile(":a_label\nlet x = -2\nlet y = 5\n if x != 3 then\n let x = y + 3 \ngoto a_label\nendif\nlet x=5\ngoto a_label");
    auto vm = vm::create();
    vm->run(c->bytecode());
}*/

TEST_CASE("ASM compiler", "[compiler]")
{
    auto c = compiler::initalize();
    c->compile(R"code(
                 asm MOV $r3@1 9
                 asm MOV $r1 20
                 asm MOV $r2 $r1
                 asm MOV [0] $r2
                 asm MOV [4] $r2
                 asm MOV $r5 9
                 asm MOV $r4@2 $r5
                 asm ADD $r2 10
                )code"
              );



    auto vm = vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->r(1) == 20);
    REQUIRE(vm->r(2) == 30);
    REQUIRE(vm->r(3) == 0x0000000900);
    REQUIRE(vm->get_mem(0) == 20);
    REQUIRE(vm->get_mem(4) == 20);
    REQUIRE(vm->get_mem(4) != 21);
    REQUIRE(vm->r(4) == 0x00090000);
}
