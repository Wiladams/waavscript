#pragma once


#include "psvm.h"

/*

namespace waavs 
{

    //
	// The PSInterpreter takes a strea of tokens and runs them through the PSVirtualMachine.
    // This is the main entry point when you're running a PS script
    // or running an interactive interface.
    struct PSInterpreter 
    {
    private:
        PSVirtualMachine& fVM;

    public:
        PSInterpreter(PSVirtualMachine& vm)
            : fVM(vm) 
        {
        }

        // The interpreter is working at the highest level of input.  
        // The behavior here is to execute executable names immediately
        // everything else goes onto the operand stack of the vm, and
        // that's all it does.  Takes the stream of objects, and executes
        // them one by one.
        //
        // We're not concerned with creating procedure bodies.  That either
        // happens at the scanner level, or in the case of an array, as
        // regular operators.
        //
        bool interpret(PSObjectGenerator &objGen)
        {

            while (true)
            {
                PSObject obj;

                // return the next object from the object generator
                if (!objGen.next(obj)) return false;

                    
                // The only things we try to execute directly are
				// executable names.  Everything else is pushed onto the stack.
                if (obj.isExecutableName()) 
                {
                    if (!fVM.execName(obj)) break;

                    if (fVM.isExitRequested()) {
                        break;
                    }

                    if (fVM.isStopRequested()) {
                        fVM.clearStopRequest();
                        continue;
                    }
                }
                else {
                    fVM.opStack().push(obj);
                }
            }

            return true;
        }
        
        bool interpret(const OctetCursor input) {
            PSObjectGenerator objGen(input);
        
			return interpret(objGen);
        }

    };
}

*/