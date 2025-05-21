#pragma once

#include "ps_scanner.h"
#include "psvm.h"

namespace waavsps {

    struct PSInterpreter {
        PSVirtualMachine& vm;

        explicit PSInterpreter(PSVirtualMachine& v) : vm(v) {}

        // Interpret a stream of tokens from input
        bool run(ByteSpan input) {
            PSScanner scanner(input, &vm);
            while (true) {
                PSToken tok = scanner.nextToken();

                if (tok.type == PSTokenType::EOI)
                    return true;

                if (tok.type == PSTokenType::Invalid)
                    return false;

                // Execute the token
                if (tok.type == PSTokenType::ExecutableName) {
                    ByteSpan name = tok.span;
                    std::string nstr(reinterpret_cast<const char*>(name.data()), name.size());
                    vm.execute(nstr);  // Looks up operator or value
                }
                else if (tok.type == PSTokenType::Operator) {
                    PSOperator* op = static_cast<PSOperator*>(tok.ptr);
                    if (!op->func(vm)) return false;
                }
                else {
                    // Literal token — push as PSObject
                    vm.push(tok.toObject());
                }
            }
        }
    };

}

