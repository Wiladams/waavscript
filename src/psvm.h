#pragma once


#include <vector>
#include <memory>


#include "pscore.h"
#include "dictionarystack.h"
#include "ocspan.h"
#include "psstack.h"
#include "psgraphicscontext.h"

namespace waavs
{

	// PSVirtualMachine
    // 
	// This is the CPU of the PostScript interpreter.
	// It manages the execution stack, operand stack, graphics context,
	// and operator table. It executes operators and handles the PostScript language semantics.
    // You can use this VM completely in the absense of the PSInterpreter.  Mainly it relies
	// on the PSObject and PSOperator classes to handle the execution of operators.
    // 
    struct PSVirtualMachine
    {
    private:
        std::unique_ptr<PSGraphicsContext> graphicsContext_;
        PSOperandStack operandStack_;
        PSExecutionStack executionStack_;
		bool stopRequested = false;
        bool exitRequested = false;

    public:
        PSDictionaryStack dictionaryStack;
        PSDictionaryHandle systemdict;
        PSDictionaryHandle userdict;

        int32_t randSeed = 1;

    public:
        PSVirtualMachine()
        {
            systemdict = PSDictionary::create();
            userdict = PSDictionary::create();

            // Set up dictionary stack (bottom to top):
            dictionaryStack.push(systemdict); // Lowest priority
            dictionaryStack.push(userdict);   // Highest priority

        }

        // stack access
        inline PSOperandStack& opStack() { return operandStack_; }
        inline const PSOperandStack& opStack() const { return operandStack_; }

        inline PSExecutionStack& execStack() { return executionStack_; }
        inline const PSExecutionStack& execStack() const { return executionStack_; }

        // Graphics context access
        PSGraphicsContext* graphics() { return graphicsContext_.get(); }
        inline void setGraphicsContext(std::unique_ptr<PSGraphicsContext> ctx) { graphicsContext_ = std::move(ctx);}


		//=====================================================================
		// REGISTERING OPERATORS
		//======================================================================
        // Mass registration of builtin operators.  This is NOT how user
        // defined operators are registered.
        bool registerBuiltin(const char* name, PSOperatorFunc fn)
        {
            PSOperator op(name, fn);

            // Instead of getting a pointer (now unnecessary), use local copy
            systemdict->put(name, PSObject::fromOperator(op));

            return true;
        }

		// quickly register a set of operators
        void registerOps(const PSOperatorFuncMap& ops)
        {
            for (const auto& entry : ops) {
                registerBuiltin(entry.first, entry.second);
            }
        }



        // request whole program exit
        void exit() { 
            exitRequested = true; 
        }
        bool isExitRequested() const { return exitRequested; }
        void clearExitRequest() { exitRequested = false; }

        // request a stop to the currently executing loop
        void stop() 
        { 
            execStack().clearToMark();  // clear the rest of currently executing procedure
            stopRequested = true; 
        }
        bool isStopRequested() const { return stopRequested; }
        void clearStopRequest() { stopRequested = false; }


        // Start running whatever is currently on the 
        // execution stack
        bool run() {
            while (!execStack().empty()) {
                PSObject obj = execStack().pop();

                // skip over structural markers
                if (obj.isMark()) {
                    // If we've reached a marker, that indicates the end
                    // of a function frame, so we should return.
                    return true;
				}

                if (!execute(obj, false))
                    return false;;

                if (isExitRequested()) 
                    break;
                if (isStopRequested()) {
                    //clearStopRequest();
                    break; // Stop is not an error, just a pause
				}
            }
            return true;
        }


        // runArray()
        // 
        // Pus the items of an array onto the execution stack in reverse order
		// That is, elements[0] ends up on the top of the execution stack.
        bool runArray(const PSObject& proc)
        {
            return pushProcedureToExecStack(proc) && run();
        }

 


        // --- Execute a single PSObject
        bool execute(const PSObject& obj, bool fromExecStack = false)
        {

            switch (obj.type) {
                // Literal value (number, string, etc.) — push onto operand stack

                case PSObjectType::Int:
                case PSObjectType::Real:
                case PSObjectType::Bool:
                case PSObjectType::String:
                case PSObjectType::Matrix:
                case PSObjectType::Mark:
                case PSObjectType::Null:
                    opStack().push(obj);
                    return true;

                case PSObjectType::Operator: {
                    auto op = obj.asOperator();
                    if (op.isValid()) {
                        return op.func(*this);
                    }
                    else {
                        return error("PSVirtualMachine::ececute - invalid operator", op.name);
                    }
                }

            case PSObjectType::Name: {

                // 1. If It's a literal name= ("/foo"), push to opStack as literal
                if (obj.isLiteralName())
                    return opStack().push(obj);

                // If it's not a literal name, then it's something we should lookup
                // It should resolve to either an executable thing (operator,procedure)
                // or it will be another literal, which can just be put on the opStack
                const char* name = obj.asName();
                PSObject resolved;


                if (!dictionaryStack.load(name, resolved)) {
                    return error("undefined name", name);
                }


                // 2. If it's an operator?  put it on the exec stack
                // to be executed later.
                if (resolved.isOperator()) {
                    return execStack().push(resolved);
                }

                // 3. Name resolves to a procedure?  auto-exec
                if (resolved.isArray() && resolved.asArray()->isProcedure()) {
                    return pushProcedureToExecStack(resolved);
                }

                // 4. Otherwise, it's a literal value, push to operand stack
                opStack().push(resolved);
                return true;
            }

            case PSObjectType::Array: {
                auto arr = obj.asArray();
                if (!arr)
                    return error("execute::Array null array");

                //if (arr->isProcedure()) {
                    opStack().push(obj); // treat it as a literal, for later execution
                    return true;
                //}
                //else {
                //    return resolveLiteralArray(obj);
                //}
            }

            //default:
                // Literal value (number, string, etc.) — push onto operand stack
                //opStack().push(obj);
                //return true;
            }

            return true;
        }
 
		//=======================================================================
        // ERROR handling
		//=======================================================================
        bool error(const char* message) const {
            printf("%% Error: %s\n", message);
            return false;
        }

        bool error(const char* message, const char* detail) const {
            printf("%% Error: %s (%s)\n", message, detail);
            return false;
        }

        private:
            // Unrolls a procedure (array) onto the execution stack.
            // The arguments are pushed in reverse order, so the first argument is on top of the stack.
            // Returns false on error (e.g., if the array is not a procedure).
            bool pushProcedureToExecStack(const PSObject& proc) {
                if (!proc.isArray())
                    return error("pushProcedureToExecStack - typecheck, NOT ARRAY");

                auto arr = proc.asArray();
				auto& elems = arr->elements;

                // An array can be pushed to the exec stack
                //if (!arr || !arr->isProcedure())
                //    return error("pushProcedureToExecStack::typecheck, NOT PROC");

                // start by pushing a marker, which is used to properly unwind 
                // the execution stack when the procedure is done or stopped
                execStack().push(PSObject::fromMark(PSMark("pushArray")));
                for (auto it = elems.rbegin(); it !=elems.rend(); ++it)
                {
                    if (!execStack().push(*it))
                        return false;
                }

                return true;
            }
    };

}
