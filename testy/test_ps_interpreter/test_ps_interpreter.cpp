#include "ps_interpreter.h"
#include "psvmfactory.h"

#include <memory>
#include <cstdio>

using namespace waavs;

// Utility to wrap input and run interpreter
static void runPostscript(const char* sourceText) {
    OctetCursor input(sourceText);
    std::unique_ptr<PSVirtualMachine> vm = PSVMFactory::createVM();
    printf("+-----------------------------------------+\n");
    printf("PS INPUT\n %s\n", sourceText);
    printf("+-----------------------------------------+\n");

    if (!vm) {
        printf("Failed to create virtual machine\n");
        return;
    }

    PSInterpreter interp(*vm);
    interp.interpret(input);

}

// ------------ Test Cases ------------

static void test_arithmetic_ops() {
    printf("== Arithmetic Operators ==\n");
    runPostscript("10 5 add =");   // expect: 15
    runPostscript("10 5 sub =");   // expect: 5
    runPostscript("10 5 mul =");   // expect: 50
    runPostscript("10 5 div =");   // expect: 2
}

static void test_stack_ops() {
    printf("\n== Stack Operators ==\n");
    runPostscript("1 2 3 dup = = =");       // expect: 3, 3, 2
    runPostscript("1 2 exch = =");          // expect: 1, 2
    runPostscript("1 2 3 pop = =");         // expect: 2, 1
    runPostscript("1 2 3 3 copy = = = = = = "); // expect: 3, 2, 1, 3, 2, 1,
    runPostscript("clear");                // just clear the stack
}
const char* stopped_s1 = R"||(
{
  (before stop) =
  stop
  (after stop) =
} stopped
(continued) =
)||";

static void test_op_stopped() {
    runPostscript(stopped_s1); // should print "before stop" and then "continued"
}

static void test_control_flow() {
    printf("\n== Control Flow  ==\n");
    runPostscript("true { 1 } if = ");     // should not error if 'if' is implemented
    runPostscript("0 1 2 { = } repeat  % expect: 1, 0"); // should not error if 'repeat' is implemented,    expect: 1, 0
	runPostscript("0 1 3 {=} for % prints 0 1 2 3"); // expect: 0, 1, 2, 3
	runPostscript("3 -1 0 {=} for % prints 3 2 1 0"); // expect: 3, 2, 1, 0
    runPostscript("0 0.25 3 {=} for % prints 0..3, in increments of 0.25");
}

void test_forall()
{
    printf("\n== ForAll Operator ==\n");
    runPostscript("[ 10 20 30 ] { = } forall");
    runPostscript("(abc) { = } forall");
    runPostscript("[ (hello) 123 true ] { dup type = } forall");
    runPostscript("[ 1 2 3 4 5 ] { dup 3 eq{ exit } if (Value = ) print =} forall");
    runPostscript("[ 1 2 3 4 5 ] { dup mul = } forall");
}

static void test_debug_ops() {
    printf("\n== Debug Operators ==\n");
    runPostscript("[1 2 3 ] << /foo (bar) /nums [4 5 6] >> =="); // expect: 3, 2, 1
    runPostscript("1 2 3 [4 5 6] stack");
    runPostscript("1 2 3 [4 5 6] pstack");
}

const char* loop_s1 = R"||(
0 {
    (start) =
        dup 
        1 add 
        dup 3 gt 
    { (exit triggered) = exit } if
} loop
)||";

const char* loop_s2 = R"||(
0 10 {
    (start) =
        dup

        1 add

        dup

        3 gt

        { (exit triggered) = exit } 
 pstack
        if
} repeat
)||";


static void test_loop_op()
{
        printf("\n== Loop Operator ==\n");
        //runPostscript("1 dup ="); // expect: 1
		//runPostscript("2 1 add ="); // expect: 3
        //runPostscript("3 2 gt ="); // expect true
		//runPostscript("2 3 gt {(true) =} if  %should print nothing"); // expect: nothing
		//runPostscript("4 3 gt {(yes) =} if  %should print 'yes'"); // expect: yes
        //runPostscript("{ (hi) = exit (after-exit) = } loop");
		runPostscript(loop_s1);
        //runPostscript(loop_s2);

		//runPostscript("0 { dup = 1 add dup 3 gt{ exit } if } loop pop"); 
}

static void test_logic()
{
	printf("\n== Logic Operators ==\n");
    static const char* logic_s1 = R"||(
1 2 gt =
1 2 gt { (should not print) = } if
    )||";

	runPostscript(logic_s1);
}


static void test_procedure()
{
    printf("\n== Procedure Operators ==\n");
    runPostscript("true { { (Nested procedure executed) = } exec }  if");
	runPostscript("3 { { (Hello) = } exec } repeat                 % Should print (Hello) 3 times");
    runPostscript("3 { (Hello) = } repeat                          % Should print (Hello) 3 times");

    runPostscript("true { true { (Both conditions met) = } if } if          % Should printf (Both conditions met)");
}

static void test_repeat()
{
    printf("\n== Repeat Operator ==\n");

    runPostscript("0 3 { (inside repeat) = } repeat");
}



static void test_nested()
{
    const char* nested_s1 = R"||(
% Nested execution
/innerProc { (hello from inner) = } def
/outerProc { innerProc  } def
outerProc
)||";
    
    const char* nested_s2 = R"||(
% Nested execution
/outerProc { innerProc exec } def
outerProc
)||";

    const char* nested_s3 = R"||(
/innerProc { (hello from inner) = } def
/outerProc { /innerProc load exec } def
outerProc

)||";

    runPostscript(nested_s1);
    //runPostscript(nested_s2);
    //runPostscript(nested_s3);


}

static void test_exec()
{
    runPostscript("{ 1 2 add } exec =");
}

static void test_operator_def()
{
    printf("\n== Operator Definition ==\n");
    runPostscript("/x 42 def x =");
}

static void test_unimplemented_op() 
{
    printf("\n== Unimplemented Operator ==\n");
    runPostscript("foobar"); // Should trigger an undefined name or op message
}

// ------------ Entry ------------

int main() {
    test_arithmetic_ops();
    test_stack_ops();
    test_control_flow();
    test_debug_ops();
    test_loop_op();
    test_forall();
    test_logic();
    test_procedure();
    test_repeat();
    test_nested();
    test_exec();
    test_op_stopped();
    test_operator_def();

    //test_unimplemented_op();

    return 0;
}

