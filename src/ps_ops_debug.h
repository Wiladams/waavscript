#pragma once

#include <iostream>

#include "pscore.h"
#include "psvm.h"
#include "ps_print.h"


namespace waavs {





    // ==
    static bool op_eqeq(PSVirtualMachine& vm) {
        if (vm.opStack().empty()) return false;
        PSObject obj;
        vm.opStack().pop(obj);
        writeObjectDeep(obj, std::cout);
        std::cout << std::endl;
        return true;
    }

    // =
    static bool op_eq(PSVirtualMachine& vm) {
        if (vm.opStack().empty()) return false;
        PSObject obj;
        if (!vm.opStack().pop(obj)) return false;
        writeObjectShallow(obj, std::cout);
        std::cout << std::endl;
        return true;
    }

    // =only
    static bool op_eqonly(PSVirtualMachine& vm) {
        if (vm.opStack().empty()) return false;
        PSObject obj;
        if (!vm.opStack().pop(obj)) return false;
        writeObjectShallow(obj, std::cout);

        return true;
    }

    // print
    static bool op_print(PSVirtualMachine& vm) {
        if (vm.opStack().empty())
            return vm.error("op_print: stackunderflow");

        PSObject obj;
        vm.opStack().pop(obj);

        if (!obj.isString())
            return vm.error("op_print: typecheck, only prints strings");
        
        std::cout << obj.asString().toString();
        
        return true;
    }



    static bool op_stack(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        for (const auto& obj : s) {
            writeObjectShallow(obj, std::cout);
            std::cout << " ";
        }
        std::cout << std::endl;
        return true;
    }

    static bool op_pstack(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        std::printf("<< pstack BEGIN <<\n");
        for (const auto& obj : s) {
            writeObjectDeep(obj, std::cout);
            std::cout << std::endl;
        }
        std::printf(">> pstack END >>\n");
        return true;
    }

    static bool op_errordict(PSVirtualMachine& vm) {
        auto dict = PSDictionary::create();
        vm.opStack().push(PSObject::fromDictionary(dict));
        return true;
    }

    static bool op_handleerror(PSVirtualMachine&) {
        std::cerr << "An error occurred." << std::endl;
        return true;
    }



    // --- Debug Operator Table ---

    inline const PSOperatorFuncMap& getDebugOps() {
        static const PSOperatorFuncMap table = {
            { "==",         op_eqeq },
            { "=",          op_eq },
            { "=only",      op_eqonly },
            { "print",      op_print },
            { "stack",      op_stack },
            { "pstack",     op_pstack },
            { "errordict",  op_errordict },
            { "handleerror",op_handleerror },
        };
        return table;
    }

} // namespace waavs
