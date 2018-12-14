#include "catch.hpp"
#include "numeric_decl.h"

#include <vm.h>
#include <vm_impl.h>
#include <compiler.h>
#include <options.h>
#include <iostream>
/*
TEST_CASE("Compiler compiles, string indexed assignment", "[compiler]")
{
    primal::options::instance().generate_assembly(true);
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var string a
                   let a = "ABCDEF"
                   let a[2] = "X"
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));

    vm->impl->bindump();

    REQUIRE(vm->get_mem(0) == 4);
    REQUIRE(vm->get_mem_byte(4) == 6);
}

/**/


TEST_CASE("Compiler compiles, string assignment", "[compiler]")
{

    auto c = primal::compiler::create();

    c->compile(R"code(
                   import write

                   var string a
                   let a = "ABCDEF"

                   write(a)
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));

    REQUIRE(vm->get_mem(0) == 4100);
    REQUIRE(vm->get_mem_byte(4100) == 6);
}


TEST_CASE("Compiler fibonacci", "[compiler]")
{
    std::shared_ptr<primal::compiler> c = primal::compiler::create();
    c->compile(R"code(
                   import write

                   var t1, t2, nextTerm, n
                   let n = 100

                   let t1 = 0
                   let t2 = 1

                   :again

                   let nextTerm = t1 + t2

                   let t1 = t2
                   let t2 = nextTerm
                   if nextTerm < n then
                       write(nextTerm, " --> ")
                       goto again
                   end
                )code");

    std::shared_ptr<primal::vm> vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(12) == 144);

}

TEST_CASE("Compiler compiles, simple if else", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a,b
                   let a = 3
                   let b = 3
                   if b == 4 then
                       let a = 9
                   else
                       let a = 6
                   end
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 6);
}

//     primal::options::instance().generate_assembly(true);

#if TARGET_ARCH == 32

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

#endif

TEST_CASE("Compiler compiles, while test", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a,b
                   let a = 5
                   let b = 0
                   while a > 0
                      let a = a - 1
                      let b = b + 1
                   end
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 0);
    REQUIRE(vm->get_mem(word_size) == 5);
}

TEST_CASE("Compiler compiles, function with variable args", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   fun some(...)
                       var int y
                       let y = 2
                   end
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

TEST_CASE("Compiler compiles, write function", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
               fun write(...)

                    asm MOV $r249 $r255
                    # Decrease the stack pointer to skip the pushed R254 and the return address. This is for 32 bit builds
                    asm SUB $r255 8

                    # First: the number of parameters that came in
                    asm POP $r10

                :next_var

                    # fetch the value that needs to be printed
                    asm POP $r2

                    # This $r1 will contain the type of the variable: 1 for string, 0 for number
                    asm POP $r1

                    # Is this a numeric value we want to print?
                    asm EQ $r1 0

                    # If yes, goto the print number location
                    asm JT print_number

                    # else goto the print string location
                    asm JMP print_string

                 :print_number

                    # print it out
                    asm INTR 1

                    # Move to the next variable
                    asm SUB $r10 1

                    # JT is logically equivalent to JNZ
                    asm JT next_var

                    # Done here, just return
                    asm MOV $r255 $r249
                    asm JMP leave

                 :print_string

                    # Here $r2 contains the address of the string, first character is the length

                    # Initialize $r1 with the length
                    asm MOV $r1 0
                    asm MOV $r1@0 [$r2]

                    # Get the address of the actual character data
                    asm ADD $r2 1

                    # Print it
                    asm INTR 1

                    # Move to the next variable
                    asm SUB $r10 1

                    # JT is logically equivalent to JNZ
                    asm JT next_var

                    # Done here, just return
                    asm MOV $r255 $r249
               :leave
               end

               write(5678, "abc", "def", 1234)
               )code"
             );

    auto vm = primal::vm::create();

    REQUIRE(vm->run(c->bytecode()));

}

TEST_CASE("Compiler compiles, simple goto", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   let a = 5
                   goto skip
                   let a = 6
                   :skip
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 5);
}


TEST_CASE("Compiler compiles, functions with params - 3rd", "[compiler]")
{


    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   fun some(integer a x)
                       var b,z
                       let b = a
                       let z = x
                   end
                   var b
                   let b = 77
                   let a = 88
                   some (b a)

               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 77);
    REQUIRE(vm->get_mem(word_size) == 88);
}

TEST_CASE("ASM compiler - Reg offseted Indexed mem access", "[asm-compiler]")
{
    auto c = primal::compiler::create();
    c->compile(R"code(
                      asm MOV [$r1+0] 20
                      asm MOV $r2 [$r1]
                      asm MOV [$r2-0] 32
                )code"
    );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 20);
    REQUIRE(vm->r(2).value() == 20);
    REQUIRE(vm->get_mem(20) == 32);
}


TEST_CASE("ASM compiler - jump in asm statements", "[asm-compiler]")
{
    auto c = primal::compiler::create();


    c->compile(R"code(
                      asm JMP lbl
                      asm MOV $r1 30
                    :lbl
                )code"
    );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->r(1).value() == 0);
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

TEST_CASE("Compiler compiles, simple add", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   let a = 2 + 3
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 5);
}


TEST_CASE("Compiler compiles, simple if 2", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a,b,c
                   let a = 3
                   let b = 4
                   let c = 5
                   if a == 2 or a == 3 and c == 5 then
                         let a = 9
                   end
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 9);
}

TEST_CASE("Compiler compiles, simple if 1", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   let a = 3
                   if a == 2 or a == 3 then
                         let a = 3
                   end
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 3);
}


TEST_CASE("Compiler compiles, if in if", "[compiler]")
{

    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a,b
                   let a = 2
                   let b = 3
                   if a == 2 then
                      if b == 3 then
                         let    a = 5
                      end
                   end
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 5);
}


TEST_CASE("Compiler compiles, interrupts", "[asm-compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                     asm MOV $r1 0
                     asm MOV $r2 42
                     asm INTR 1
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
}

TEST_CASE("Compiler compiles, functions with params - 2nd", "[compiler]")
{

    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   fun some(integer a)
                       var b
                       let b = 55
                       let a = 44
                   end
                   var b
                   let b = 77
                   let a = 88
                   some (b)

               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 77);
    REQUIRE(vm->get_mem(word_size) == 88);
}



TEST_CASE("Compiler compiles, functions with params - 1st", "[compiler]")
{

    auto c = primal::compiler::create();

    c->compile(R"code(
                   var a
                   fun some(integer a)
                       let a = 44
                   end
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

    auto c = primal::compiler::create();

    c->compile(R"code(
                   fun some(integer a)
                       var b
                       let b = 55
                       let a = 44
                   end
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
                   end
                   some (4)
                   let t = 66
               )code"
             );

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));
    REQUIRE(vm->get_mem(0) == 44);
    REQUIRE(vm->get_mem(word_size) == 66);
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
    REQUIRE(vm->flag() != 0);
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
    REQUIRE(vm->get_mem(word_size) == 1);
    REQUIRE(vm->get_mem(word_size * 2) == 0);
}

TEST_CASE("Compiler compiles, IF test", "[compiler]")
{
    auto c = primal::compiler::create();

    c->compile(R"code(
                   var x
                   let x = 1
                   if x == 1 then
                      let x = 3
                   end
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
    std::string cd1(R"code(
              asm MOV $r1 20
              asm MOV $r2 $r1
              asm MOV $r5 9
              asm MOV [0] $r2
              )code"
            );

#if TARGET_ARCH == 32
    cd1 += "asm MOV [4] $r2";
#else
    cd1 += "asm MOV [8] $r2";
#endif

    cd1 += R"code(
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
           )code";

    c->compile(cd1);

    auto vm = primal::vm::create();
    REQUIRE(vm->run(c->bytecode()));

    REQUIRE(vm->r(1) == 20);
    REQUIRE(vm->r(3).value() == 0x0000000900);
    REQUIRE(vm->get_mem(0) == 20);
    REQUIRE(vm->get_mem(word_size) == 20);
    REQUIRE(vm->get_mem(word_size) != 21);
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
    REQUIRE(vm->flag() != 0);
}

/**/
