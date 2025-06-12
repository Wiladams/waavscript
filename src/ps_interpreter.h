#pragma once


#include "psvm.h"
#include "ps_scanner.h"


namespace waavs 
{
    // Unrolls a procedure (array) onto the execution stack.
// The arguments are pushed in reverse order, so the first argument is on top of the stack.
// Returns false on error (e.g., if the array is not a procedure).
    static bool pushProcedureToExecStack(PSVirtualMachine & vm, const PSObject& proc) 
    {
        if (!proc.isArray())
            return vm.error("pushProcedureToExecStack - typecheck, NOT ARRAY");

        auto arr = proc.asArray();
        auto& elems = arr->elements;

        // start by pushing a marker, which is used to properly unwind 
        // the execution stack when the procedure is done or stopped
        vm.execStack().push(PSObject::fromMark(PSMark("pushArray")));
        for (auto it = elems.rbegin(); it != elems.rend(); ++it)
        {
            if (!vm.execStack().push(*it))
                return false;
        }

        return true;
    }

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

        bool execArray(PSObject &obj)
        {
            // push to execution stack then run
            pushProcedureToExecStack(fVM, obj);

            return fVM.run();
        }

        // This is meant to run executable names
        bool execName(PSObject& obj)
        {
            // Lookup the thing in the dictionary stack
            const char* name = obj.asName();
            PSObject resolved;

            if (!fVM.dictionaryStack.load(name, resolved)) {
                return fVM.error("undefined name", name);
            }

            // 2. If it's an operator?  run it immediately
            if (resolved.isOperator()) {
                auto op = resolved.asOperator();

                // Run the operator if it's valid, otherwise return false
                if (op.isValid()) {
                    return op.func(fVM);
                } return false;
            }

            // 3. Name resolves to a procedure?  auto-exec
            if (resolved.isExecutable() && resolved.isArray()) {
                return execArray(resolved);
            }

            // 4. Otherwise, it's a literal value, push to operand stack
            fVM.opStack().push(resolved);

            return true;
        }


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
                    execName(obj);

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