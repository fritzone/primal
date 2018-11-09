#include "catch.hpp"

#include <vm.h>
#include <compiler.h>
#include <options.h>
#include <iostream>




TEST_CASE("Compiler compiles, functions with params - 3rd", "[compiler]")
{

    primal::options::instance().generate_assembly(true);

    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   fun some(integer a)
                       var b
                       let b = a
                   endf
                   var b
                   let b = 77
                   let a = 88
                   some (b)

               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 77);
    REQUIRE(vm->get_mem(4) == 88);
}

/*
TEST_CASE("Compiler compiles, functions with params - 2nd", "[compiler]")
{

    primal::options::instance().generate_assembly(true);

    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   fun some(integer a)
                       var b
                       let b = 55
                       let a = 44
                   endf
                   var b
                   let b = 77
                   let a = 88
                   some (b)

               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 77);
    REQUIRE(vm->get_mem(4) == 88);
}

/*

TEST_CASE("Compiler compiles, functions with params - 1st", "[compiler]")
{

    primal::options::instance().generate_assembly(true);

    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   fun some(integer a)
                       let a = 44
                   endf
                   let a = 88
                   some (a)
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 88);
}

TEST_CASE("Compiler compiles, functions with params", "[compiler]")
{

    primal::options::instance().generate_assembly(true);

    auto c = primal::compiler::create();

    c->compile(R"code(
                   fun some(integer a)
                       var b
                       let b = 55
                       let a = 44
                   endf
                   some (4)
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
}

TEST_CASE("Compiler compiles, functions 1", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a,t
                   let a = 5
                   fun some(...)
                       var z
                       let z = 53
                       let a = 44
                   endf
                   some (4)
                   let t = 66
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 44);
    REQUIRE(vm->get_mem(4) == 66);
}

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
    REQUIRE(static_cast<int>(vm->get_mem_byte(1)) == 0);
    REQUIRE(static_cast<int>(vm->get_mem_byte(2)) == 0);
    REQUIRE(vm->get_mem_byte(3) == 20);
    REQUIRE(vm->r(3).value() == 20);
}

TEST_CASE("Asm compiler - JUMP test", "[asm-compiler]")
{
    // ASM code below will jump over the MOV $r1, 43. Please note, there is added 16 bytes for the header!
    std::shared_ptr<primal::compiler> c = primal::compiler::create();
    c->compile(R"code(
                      asm MOV $r1 42
                      asm JMP 1048614
                      asm MOV $r1 43
                      asm SUB $r1 1
                )code");

    std::shared_ptr<primal::vm> vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->r(1).value() == 41);
}

TEST_CASE("Asm compiler - stack operatons", "[asm-compiler")
{
    std::shared_ptr<primal::compiler> c = primal::compiler::create();
    c->compile(R"code(
                      asm MOV $r1 42
                      asm PUSH $r1
                      asm POP $r2
                      asm EQ $r2 42
                )code");

    std::shared_ptr<primal::vm> vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->r(2).value() == 42);
    REQUIRE(vm->flag() == true);
}

TEST_CASE("Compiler compiles, Simple write", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   fun some(...)
                       var int y
                       let y = 2
                   endf
                   var x,z
                   let x = 12
                   some (4)
                   let z = 55
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 12);
}

TEST_CASE("Script compiler - NOT operations", "[script-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      var x,y,z
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

TEST_CASE("Compiler compiles, IF test", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var x
                   let x = 1
                   if x == 1 then
                      let x = 3
                   endif
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 3);
}

TEST_CASE("Script compiler - 1 NOT operation", "[script-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      var x,y,z
                      let x = !1
                      let y = !0
                      let z = !(1+0)
                )code"
    );
    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 0);
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
                      var integer x
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
                      var x
                      let x = 40
                )code"
    );

    std::shared_ptr<primal::vm> vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 40);
}

TEST_CASE("Asm compiler - COPY test", "[asm-compiler]")
{
    std::shared_ptr<primal::compiler> c = primal::compiler::create();
    c->compile(R"code(
                     var x
                     let x = 313249263
                     asm COPY 4 0 4
                     asm MOV $r1@0 [@4]
                     asm MOV $r1@1 [@5]
                     asm MOV $r1@2 [@6]
                     asm MOV $r1@3 [@7]
                )code");

    std::shared_ptr<primal::vm> vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(4) == 313249263);
    REQUIRE(vm->r(1).value() == 313249263);
}

TEST_CASE("Asm compiler - EQ/JT test", "[asm-compiler]")
{
    std::shared_ptr<primal::compiler> c = primal::compiler::create();
    c->compile(R"code(
                      asm MOV $r1 42
               :ok
                      asm EQ $r1 42
                )code");

    std::shared_ptr<primal::vm> vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->r(1).value() == 42);
    REQUIRE(vm->flag() == true);
}

/**/
