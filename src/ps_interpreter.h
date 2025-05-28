#pragma once


#include "psvm.h"
#include "ps_lexer.h"
#include "ps_scanner.h"


namespace waavs 
{

    struct PSInterpreter 
    {
    private:
        PSVirtualMachine& fVM;

    public:
        PSInterpreter(PSVirtualMachine& vm)
            : fVM(vm) 
        {
        }


        bool interpret(OctetCursor& input) {
            PSTokenGenerator tokGen(input);
            PSObject obj;

            while (parseObject(tokGen, obj)) {
                // Only execute names and operators at the top level.
                if (obj.isName() || obj.isOperator()) {
                    if (!fVM.execute(obj)) {
                        printf("Error: VM execution failed\n");
                        return false;
                    }
                }
                else {
                    // Push all other values (numbers, strings, arrays, etc.)
                    fVM.opStack().push(obj);
                }

                if (fVM.isExitRequested())
                    break;
            }

            return true;
        }




    };
}