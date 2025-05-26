#include "ps_interpreter.h"
#include "psvmfactory.h"

#include <memory>
#include <cstdio>

using namespace waavs;

// Utility to wrap input and run interpreter
static void runPostScript(const char* sourceText) {
    OctetCursor input(sourceText);
    std::unique_ptr<PSVirtualMachine> vm = PSVMFactory::createVM();

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
    runPostScript("10 5 add =");   // expect: 15
    runPostScript("10 5 sub =");   // expect: 5
    runPostScript("10 5 mul =");   // expect: 50
    runPostScript("10 5 div =");   // expect: 2
}

static void test_stack_ops() {
    printf("\n== Stack Operators ==\n");
    runPostScript("1 2 3 dup = = =");       // expect: 3, 3, 2
    runPostScript("1 2 exch = =");          // expect: 1, 2
    runPostScript("1 2 3 pop = =");         // expect: 2, 1
    runPostScript("1 2 3 4 copy = = = = ="); // expect: 4, 3, 2, 1, 4
    runPostScript("clear");                // just clear the stack
}

static void test_control_flow_stubs() {
    printf("\n== Control Flow (Stubbed) ==\n");
    runPostScript("true { 1 } if");     // should not error if 'if' is implemented
    runPostScript("0 1 2 { = } repeat"); // should not error if 'repeat' is implemented
}

static void test_unimplemented_op() {
    printf("\n== Unimplemented Operator ==\n");
    runPostScript("foobar"); // Should trigger an undefined name or op message
}

// ------------ Entry ------------

int main() {
    test_arithmetic_ops();
    test_stack_ops();
    test_control_flow_stubs();
    test_unimplemented_op();

    return 0;
}

