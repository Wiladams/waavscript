#pragma once


#include <vector>
#include <memory>


#include "pscore.h"
#include "dictionarystack.h"
#include "psstack.h"
#include "psgraphicscontext.h"
#include "ps_scanner.h"
#include "psfile.h"


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
		int fLanguageLevel = 2; // Default language level
        std::unique_ptr<PSGraphicsContext> graphicsContext_;
        PSOperandStack operandStack_;
        PSExecutionStack executionStack_;
		bool stopRequested = false;
        bool exitRequested = false;

        PSDictionaryHandle systemdict;
        PSDictionaryHandle userdict;

        std::shared_ptr<PSFile> fCurrentFile; // Current file being processed, if any

    public:
        PSDictionaryStack dictionaryStack;


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

        // Meta Information
        int languageLevel() const { return fLanguageLevel; }
		void setLanguageLevel(int level) { fLanguageLevel = level; }

        // File Handling
        std::shared_ptr<PSFile> getCurrentFile() const { return fCurrentFile; }
        void setCurrentFile(std::shared_ptr<PSFile> file) { fCurrentFile = file; }

        // stack access
        inline PSOperandStack& opStack() { return operandStack_; }
        inline const PSOperandStack& opStack() const { return operandStack_; }

        inline PSExecutionStack& execStack() { return executionStack_; }
        inline const PSExecutionStack& execStack() const { return executionStack_; }

        // Graphics context access
        PSGraphicsContext* graphics() { return graphicsContext_.get(); }
        inline void setGraphicsContext(std::unique_ptr<PSGraphicsContext> ctx) { graphicsContext_ = std::move(ctx);}

        PSDictionaryHandle getSystemDict() const { return systemdict; }
        PSDictionaryHandle getUserDict() const { return userdict; }
        PSDictionaryHandle setUserDict(PSDictionaryHandle dict)
        {
            userdict = dict;
           // dictionaryStack.setTop(dict);
            return userdict;
        }

		//=====================================================================
		// REGISTERING OPERATORS
		//======================================================================
        // Mass registration of builtin operators.  This is NOT how user
        // defined operators are registered.
        bool registerBuiltin(const PSName & name, PSOperatorFunc fn)
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
        bool run()
        {
            while (!execStack().empty()) {
                PSObject obj = execStack().pop();

                // Treat executable arrays (procedures) specially
                if (obj.isExecutable() && obj.isArray()) {
                    auto arr = obj.asArray();

                    // Push elements of procedure onto the exec stack, back to front
                    for (auto it = arr->elements.rbegin(); it != arr->elements.rend(); ++it)
                        execStack().push(*it);

                    // Continue loop — next thing to execute is now top element of proc
                    continue;
                }

                // Otherwise, just execute the object normally
                if (!execObject(obj))
                    return false;

                if (isExitRequested())
                    break;
                if (isStopRequested())
                    break;
            }

            return true;
        }


        // This is a critical function, as it dictates how procedures are executed,
        // whether we get tail recursion, etc.
        bool execProc(PSObject& proc)
        {
            if (!proc.isArray())
                return error("execProc - typecheck, NOT ARRAY");

            // Push the procedure frame (visible to execstack/countexecstack)
            execStack().push(proc);  // Optionally wrap in a frame marker

            auto arr = proc.asArray();
            for (const auto& obj : arr->elements) {
                if (!execObject(obj)) return false;

                if (isExitRequested()) break;
                if (isStopRequested()) break;
            }

            execStack().pop();  // Remove procedure frame
            return true;
        }

 
        // This is meant to run executable names
        bool execName(const PSObject& obj)
        {
            // Lookup the thing in the dictionary stack
            auto name = obj.asName();
            PSObject resolved;

            if (!dictionaryStack.load(name, resolved)) {
                return error("undefined name", name.c_str());
            }

            // 2. If it's an operator?  run it immediately
            if (resolved.isOperator()) {
                auto op = resolved.asOperator();

                // Run the operator if it's valid, otherwise return false
                if (op.isValid()) {
                    return op.func(*this);
                } return false;
            }

            // 3. Name resolves to a procedure?  auto-exec
            if (resolved.isExecutable() && resolved.isArray()) {
                return execProc(resolved);
            }

            // 4. Otherwise, it's a literal value, push to operand stack
            opStack().push(resolved);

            return true;
        }

        // --- Execute a single PSObject
        bool execObject(const PSObject& obj)
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
                case PSObjectType::Save:
                case PSObjectType::File:
                case PSObjectType::Font:
                case PSObjectType::FontFace:
                default:
                    opStack().push(obj);
                    return true;

                case PSObjectType::Operator: {
                    auto op = obj.asOperator();
                    if (op.isValid()) {
                        return op.func(*this);
                    }
                    else {
                        return error("PSVirtualMachine::ececute - invalid operator", op.name.c_str());
                    }
                }

                case PSObjectType::Name: {

                    // 1. If It's a literal name= ("/foo"), push to opStack as literal
                    if (obj.isLiteralName())
                        return opStack().push(obj);

                    // If it's not a literal name, then it's something we should lookup
                    // It should resolve to either an executable thing (operator,procedure)
                    // or it will be another literal, which can just be put on the opStack
                    auto name = obj.asName();
                    PSObject resolved;


                    if (!dictionaryStack.load(name, resolved)) {
                        return error("undefined name", name.c_str());
                    }


                    // 2. If it's an operator?  Execute it
                    if (resolved.isOperator()) {
                        return execObject(resolved);
                    }

                    // 3. Name resolves to a procedure?  auto-exec
                    if (resolved.isArray() && resolved.asArray()->isProcedure()) {
                        return execProc(resolved);
                    }

                    // 4. Otherwise, it's a literal value, push to operand stack
                    opStack().push(resolved);
                    return true;
                }

                case PSObjectType::Array: {
                    auto arr = obj.asArray();
                    if (!arr)
                        return error("execute::Array null array");


                    opStack().push(obj); // treat it as a literal, for later execution
                        return true;
                }

            }

            return true;
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
        bool interpret(const PSObject& obj)
        {
            // The only things we try to execute directly are
            // executable names.  Everything else is pushed onto the stack.
            if (obj.isExecutableName())
            {
                if (!execName(obj)) return false;
            }
            else {
                opStack().push(obj);
            }

            return true;
        }

        bool interpret(PSObjectGenerator& objGen)
        {

            while (true)
            {
                PSObject obj;

                // return the next object from the object generator
                if (!objGen.next(obj)) 
                    return false;

				if (!interpret(obj)) 
                    return false;

                if (isExitRequested()) {
                    break;
                }

                if (isStopRequested()) {
                    clearStopRequest();
                    continue;
                }
            }

            return true;
        }

        bool interpret(const OctetCursor input) {
            PSObjectGenerator objGen(input);

            return interpret(objGen);
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

}
