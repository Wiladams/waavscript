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
    // Forward declarations
    struct PSVirtualMachine;
    

    inline bool pushProcedureToExecStack(PSVirtualMachine& vm, const PSObject& proc);
    inline bool runArray(PSVirtualMachine& vm, const PSObject& proc);

    // contains the mapping of an interned name to a function pointer
    //
    struct PSOperatorTable
    {
    private:
        std::unordered_map<const char*, PSOperator> ops;

    public:
        void add(const char* name, PSOperatorFunc fn) 
        {
            ops[name] = PSOperator(name, fn);
        }

        PSOperator lookup(const char* name) const 
        {
            auto it = ops.find(name);
            if (it != ops.end())
                return it->second;

			// Return a default invalid operator if not found
            return PSOperator();
        }

        bool contains(const char* name) const
        {
            return ops.find(name) != ops.end();
        }

        void clear() {
            ops.clear();
        }
    };



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
        PSOperatorTable operatorTable;

        PSDictionaryHandle systemdict;
        PSDictionaryHandle userdict;

        int buildProcDepth = 0;
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
        inline PSGraphicsContext* graphics() { 
            return graphicsContext_.get(); 
        }

        inline void setGraphicsContext(std::unique_ptr<PSGraphicsContext> ctx) {
            graphicsContext_ = std::move(ctx);
        }


		//=====================================================================
		// REGISTERING OPERATORS
		//======================================================================
        // Mass registration of builtin operators.  This is NOT how user
        // defined operators are registered.
        bool registerBuiltin(const char* name, PSOperatorFunc fn)
        {
            const char* interned = PSNameTable::INTERN(name);

            PSOperator op(interned, fn);
            operatorTable.add(interned, fn);  // Store internally

            // Instead of getting a pointer (now unnecessary), use local copy
            systemdict->put(interned, PSObject::fromOperator(op));

            return true;
        }


        void registerOps(const PSOperatorFuncMap& ops)
        {
            for (const auto& entry : ops) {
                registerBuiltin(entry.first, entry.second);
            }
        }

        PSOperator lookupOperator(const char * name) const
        {
			return operatorTable.lookup(name);
		}

        bool lookupName(const char* name, PSObject& out) const
        {
            return dictionaryStack.load(name, out);
        }



        // managing execution stack



        void exit() { 
            exitRequested = true; 
        }
        bool isExitRequested() const { return exitRequested; }
        void clearExitRequest() { exitRequested = false; }

        void stop() 
        { 
            execStack().clearToMark();  // clear the rest of currently executing procedure
            stopRequested = true; 
        }
        bool isStopRequested() const { return stopRequested; }
        void clearStopRequest() { stopRequested = false; }

        bool run() {
            while (!execStack().empty()) {
                PSObject obj = execStack().pop();

                // skip over structural markers
                if (obj.isMark()) {
                    // debug: skipping execStack mark
                    continue;
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

        // --- Execute a single PSObject
        bool PSVirtualMachine::execute(const PSObject& obj, bool fromExecStack = false)
        {

            switch (obj.type) {
            case PSObjectType::Operator: {
                auto op = obj.asOperator();
                if (op.isValid()) {
                    return op.func(*this);
                }
                else {
                    return error("invalid operator");
                }
            }

            case PSObjectType::Name: {

                // 1. If It's a literal name= ("/foo"), push to opStack as leteral
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
                    return pushProcedureToExecStack(*this, resolved);
                }

                // 4. Otherwise, it's a literal value, push to operand stack
                opStack().push(resolved);
                return true;
            }

            case PSObjectType::Array: {
                auto arr = obj.asArray();
                if (!arr)
                    return error("execute::Array null array");

                if (arr->isProcedure()) {
                    //if (fromExecStack) {
                    //    return pushProcedureToExecStack(*this, obj);
                    //}
                    //else {
                    opStack().push(obj); // treat it as a literal, for later execution
                    return true;
                    //}
                }

                opStack().push(obj); // Non-executable array, treat as literal
                return true;
            }

            default:
                // Literal value (number, string, etc.) — push onto operand stack
                opStack().push(obj);
                return true;
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
    };






	//======================================================================
    // Helpers
	//======================================================================

	// Unrolls a procedure (array) onto the execution stack.
	// The arguments are pushed in reverse order, so the first argument is on top of the stack.
	// Returns false on error (e.g., if the array is not a procedure).
    inline bool pushProcedureToExecStack(PSVirtualMachine& vm, const PSObject& proc) {
        if (!proc.isArray()) 
            return vm.error("typecheck");
        
        auto arr = proc.asArray();
        if (!arr || !arr->isProcedure()) 
            return vm.error("pushProcedureToExecStack::typecheck");

        // start by pushing a marker, which is used to properly unwind 
		// the execution stack when the procedure is done or stopped
        vm.execStack().push(PSObject::fromMark());
        for (auto it = arr->elements.rbegin(); it != arr->elements.rend(); ++it)
        {
            if (!vm.execStack().push(*it))
                return false;
        }

        return true;
    }

    inline bool runArray(PSVirtualMachine& vm, const PSObject& proc) 
    {
        return pushProcedureToExecStack(vm, proc) && vm.run();
    }

}
