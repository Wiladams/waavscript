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


        bool interpret(OctetCursor& input) 
        {
            PSTokenGenerator tokGen(input);
            PSToken tok;

            while (tokGen.next(tok)) 
            {
                if (tok.type == PSTokenType::PS_TOKEN_Invalid)
                    continue;

                PSObject obj;
                if (!transformTokenToPSObject(tok, obj)) {
                    printf("Error: failed to convert token\n");
                    return false;
                }

                // Handle executable objects (e.g., names and arrays)
                if (!fVM.execute(obj)) {
                    printf("Error: VM execution failed\n");
                    return false;
                }

                if (fVM.isExitRequested())
                    break;
            }

            return true;
        }


    };
}