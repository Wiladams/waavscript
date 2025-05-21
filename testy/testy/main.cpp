#include <iostream>

#include "pscore.h"
#include "dictionarystack.h"
#include "psvm.h"
#include "psvmfactory.h"
#include "ps_scanner.h"

using namespace waavsps;

static void printObject(const PSObject& obj) {
    switch (obj.type) {
    case PSObjectType::Int:
        std::cout << "Int: " << obj.data.iVal << "\n";
        break;
    case PSObjectType::Real:
        std::cout << "Real: " << obj.data.fVal << "\n";
        break;
    case PSObjectType::Bool:
        std::cout << "Bool: " << (obj.data.bVal ? "true" : "false") << "\n";
        break;
    case PSObjectType::Name:
        std::cout << "Name: " << obj.data.name << "\n";
        break;
    default:
        std::cout << "Other type\n";
        break;
    }
}

static bool dummyOp(PSVirtualMachine &) 
{
    std::cout << "dummyOp called\n";
    return true;
}

static void test_dictionarystack()
{
	printf("==== test_dictionarystack ====\n");

    PSDictionaryStack dictStack;

    // Define in base dictionary
    dictStack.def("x", PSObject::fromInt(42));

    PSObject val;
    if (dictStack.load("x", val)) {
        std::cout << "Found x: ";
        printObject(val);  // Expect: Int: 42
    }

    // Push new scope
    auto newDict = std::make_shared<PSDictionary>();
    dictStack.push(newDict);
    dictStack.def("x", PSObject::fromInt(99));

    if (dictStack.load("x", val)) {
        std::cout << "Found x after push: ";
        printObject(val);  // Expect: Int: 99
    }

    // Pop and confirm previous value
    dictStack.pop();
    if (dictStack.load("x", val)) {
        std::cout << "Found x after pop: ";
        printObject(val);  // Expect: Int: 42
    }
}

static void test_isexecutable()
{
	printf("==== test_isexecutable ====\n");

    // --- Test: PSString executable ---
    PSString* str = new PSString("hello");
    str->isExec = true;
    PSObject strObj = PSObject::fromString(str);

    std::cout << "String is executable: " << (strObj.isExecutable() ? "yes" : "no") << "\n";

    // --- Test: PSArray executable ---
    PSArray* arr = new PSArray();
    arr->append(PSObject::fromInt(42));
    arr->setExecutable(true);
    PSObject arrObj = PSObject::fromArray(arr);

    std::cout << "Array is executable: " << (arrObj.isExecutable() ? "yes" : "no") << "\n";

    // --- Test: PSOperator ---
    PSOperator* op = new PSOperator("dummy", dummyOp);
    PSObject opObj = PSObject::fromOperator(op);

    std::cout << "Operator is executable: " << (opObj.isExecutable() ? "yes" : "no") << "\n";

    // --- Test: Non-executable int ---
    PSObject intObj = PSObject::fromInt(100);
    std::cout << "Integer is executable: " << (intObj.isExecutable() ? "yes" : "no") << "\n";

    // --- Clean up ---
    delete str;
    delete arr;
    delete op;
}

static void test_virtualmachine() 
{
	printf("==== test_virtualmachine ====\n");

	auto vm = PSVMFactory::createVM();

    // Simulate: /x 10 def
    vm->operandStack.push_back(PSObject::fromName("x"));
    vm->operandStack.push_back(PSObject::fromInt(10));
    vm->execute(vm->operatorTable.lookup("def")->func == nullptr ? PSObject::null() : PSObject::fromOperator(vm->operatorTable.lookup("def")));

    // Simulate: x (should resolve to 10)
    PSObject lookup = PSObject::fromName("x");
    vm->execute(lookup);

    if (!vm->operandStack.empty()) {
        PSObject top = vm->operandStack.back();
        std::cout << "Top of stack: " << top.asReal() << "\n"; // Expect 10
    }

}

int main() 
{
	//test_isexecutable();
	//test_dictionarystack();
    test_virtualmachine();

    return 0;
}
