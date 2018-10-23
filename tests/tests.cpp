#include "catch.hpp"

#include <vm.h>
#include <compiler.h>
#include <iostream>

TEST_CASE("ASM compiler - Reg byte mem access", "[asm-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      asm MOV [@$r1] 20
                      asm MOV $r2 3
                      asm MOV [@$r2] [@$r1]
                      asm MOV $r3@0 [@0]
                )code"
    );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem_byte(0) == 20);
    REQUIRE(static_cast<int>(vm->get_mem_byte(1)) == 255);
    REQUIRE(static_cast<int>(vm->get_mem_byte(2)) == 255);
    REQUIRE(vm->get_mem_byte(3) == 20);
    REQUIRE(vm->r(3).value() == 20);
}

TEST_CASE("ASM compiler - Reg Indexed mem access", "[asm-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      asm MOV [$r1] 20
                      asm MOV $r2 [$r1]
                      asm MOV [$r2] 32
                )code"
    );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 20);
    REQUIRE(vm->r(2).value() == 20);
    REQUIRE(vm->get_mem(20) == 32);
}




TEST_CASE("Script compiler - NOT operations", "[script-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      let x = !1
                      let y = !0
                      let z = !(1+0)
                )code"
    );
    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 0);
    REQUIRE(vm->get_mem(4) == 1);
    REQUIRE(vm->get_mem(8) == 0);
}

/*
TEST_CASE("Compiler compiles", "[compiler]")
{
    auto c = compiler::create();
    c->compile(":a_label\nlet x = -2\nlet y = 5\n if x != 3 then\n let x = y + 3 \ngoto a_label\nendif\nlet x=5\ngoto a_label");
    auto vm = vm::create();
    vm->run(c->bytecode());
}*/

TEST_CASE("ASM compiler - basic operations", "[asm-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      asm MOV $r1 20
                      asm MOV $r2 $r1
                      asm MOV $r5 9
                      asm MOV [0] $r2
                      asm MOV [4] $r2
                      asm MOV $r3@1 9
                      asm MOV $r4@2 $r5
                      asm ADD $r2 $r1

                      # Comment in here
                      asm ADD $r2 10
                      asm MOV $r7 $r2
                      asm MOD $r7 7
                      asm DIV $r2 2
                      asm MOV $r6 11
                      asm AND $r6 10
                      asm MUL $r6 10
                      asm OR  $r6 1
                      asm SUB $r2 1
                )code"
              );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));

    REQUIRE(vm->r(1) == 20);
    REQUIRE(vm->r(3).value() == 0x0000000900);
    REQUIRE(vm->get_mem(0) == 20);
    REQUIRE(vm->get_mem(4) == 20);
    REQUIRE(vm->get_mem(4) != 21);
    REQUIRE(vm->r(4) == 0x00090000);
    REQUIRE(vm->r(6).value() == 101);
    REQUIRE(vm->r(7).value() == 1);
    REQUIRE(vm->r(2).value() == 24);
}

TEST_CASE("Script compiler - XOR operations", "[script-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      let x = 20
                      let x = x ^ 10
                )code"
    );
    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 30);
}

TEST_CASE("Script compiler - Basic memory access", "[script-compiler]")
{
    std::shared_ptr<primal::compiler> c = primal::compiler::create();
    c->compile(R"code(
                      let x = 40
                )code"
    );

    std::shared_ptr<primal::vm> vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 40);
}


