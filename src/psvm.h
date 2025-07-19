#pragma once


#include <vector>
#include <memory>


#include "pscore.h"
#include "dictionarystack.h"
#include "ps_type_stack.h"
#include "ps_type_graphicscontext.h"
#include "ps_type_file.h"
#include "ps_scanner.h"
#include "ps_print.h"


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
        PSObjectStack operandStack_;
        PSObjectStack executionStack_;
        PSObjectStack fileStack;

		bool stopRequested = false;
        bool exitRequested = false;

        PSDictionaryHandle systemdict;
        PSDictionaryHandle userdict;
        PSDictionaryHandle systemResourceDirectory;

        //std::shared_ptr<PSFile> fCurrentFile; // Current file being processed, if any

        PSDictionaryStack fResourceStack; // Stack of resource dictionaries, if needed

    public:
        PSDictionaryStack dictionaryStack;


        int32_t randSeed = 1;

    public:
        PSVirtualMachine()
        {
            systemdict = PSDictionary::create();
            userdict = PSDictionary::create();
            systemResourceDirectory = PSDictionary::create();

            // Set up dictionary stack (bottom to top):
            dictionaryStack.push(systemdict); // Lowest priority
            dictionaryStack.push(userdict);   // Highest priority

            fResourceStack.push(systemResourceDirectory);
        }

        // Access to properties and state
        PSDictionaryStack& getDictionaryStack() { return dictionaryStack; }
        const PSDictionaryStack& getDictionaryStack() const { return dictionaryStack; }
        PSDictionaryHandle getSystemDict() const { return systemdict; }
        PSDictionaryHandle getUserDict() const { return userdict; }
        PSDictionaryHandle setUserDict(PSDictionaryHandle dict)
        {
            userdict = dict;
            return userdict;
        }

        PSDictionaryHandle getSystemResourceDirectory() const { return systemResourceDirectory; }
        PSDictionaryStack& getResourceStack() { return fResourceStack; }
        const PSDictionaryStack& getResourceStack() const { return fResourceStack; }


        // Meta Information
        int languageLevel() const { return fLanguageLevel; }
		void setLanguageLevel(int level) { fLanguageLevel = level; }

        // File Handling
        bool getCurrentFile(PSFileHandle &handle) const 
        { 
            // return whatever is on the top of the file stack
            PSObject fileObj;
            if (!fileStack.top(fileObj))
                return error("getCurrentFile; no current top of fileStack");

            if (!fileObj.isFile())
                return error("getCurrentFile; top item is not a file object");

            handle = fileObj.asFile();

            return true; 
        }

        bool popCurrentFile(PSFileHandle &file) 
        { 
            // pop the top file off the stack
            PSObject fileObj;
            if (!fileStack.pop(fileObj))
                return error("popCurrentFile; no current top of fileStack");
            if (!fileObj.isFile())
                return error("popCurrentFile; top item is not a file object");
            file = fileObj.asFile();
        
            return true; 
        }

        bool pushCurrentFile(PSFileHandle file) 
        { 
            return fileStack.pushFile(file);
        }

        // stack access
        inline PSObjectStack& opStack() { return operandStack_; }
        inline const PSObjectStack& opStack() const { return operandStack_; }

        inline PSObjectStack& execStack() { return executionStack_; }
        inline const PSObjectStack& execStack() const { return executionStack_; }

        // Graphics context access
        PSGraphicsContext* graphics() { return graphicsContext_.get(); }
        inline void setGraphicsContext(std::unique_ptr<PSGraphicsContext> ctx) { graphicsContext_ = std::move(ctx);}



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


        


 public:

        bool execOperator(const PSObject& obj)
        {
            auto op = obj.asOperator();

            //printf("DBG: execOperator - executing operator: %s\n", op.name.c_str());

            if (!op.exec(*this))
                return error("execOperator: op.exec() failed; ", op.name().c_str());

            return true;
        }

        // This is meant to run executable names
        bool execName(const PSObject& obj)
        {
            auto name = obj.asName();
            //printf("execName: %s\n", name.c_str());

            // 1. If It's a literal name= ("/foo"), push to opStack as literal
            if (obj.isLiteralName())
                return opStack().push(obj);

            // If it's not a literal name, it should be an exacutable name
            // so lookup the thing with the name
            PSObject resolved;

            // First, check system dictionary, for names that started with '//aname'
            if (obj.isSystemOp()) {
                if (!systemdict->get(name, resolved)) {
                    return error("undefined system name", name.c_str());
                }
            }
            else {
                if (!dictionaryStack.load(name, resolved)) {
                    return error("undefined name", name.c_str());
                }
            }

            // 2. If it's an operator?  run it immediately
            //return execObject(resolved);

            if (resolved.isOperator()) {
                return execOperator(resolved);
            }

            // 3. Name resolves to a procedure?  auto-exec
            if (resolved.isArray() && resolved.isExecutable()) {
                //execProc(resolved);
                return runProc(resolved);
            }

            // 4. Otherwise, it's a literal value, push to operand stack
            return opStack().push(resolved);
        }



        // --- Execute a single PSObject
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

        bool execObject(const PSObject& obj)
        {
            //writeObjectDeep(obj); printf("\n");

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
                case PSObjectType::Array:
                default:
                    opStack().push(obj);
                    return true;

                case PSObjectType::Operator: {
                    return execOperator(obj);
                }

                case PSObjectType::Name: {
                    return execName(obj); // Execute the name directly
                }

            }

            return true;
        }
 
        // run
        // 
        // Run items off the execution stack until it is empty or an exit/stop request is made.
        // As these can be nested, we will also return after hitting a procedure end marker.
        //
        bool run()
        {
            while (!execStack().empty()) 
            {
                if (isExitRequested())
                    break;
                if (isStopRequested())
                    break;

                PSObject obj;
                if (!execStack().pop(obj))
                    return error("run(): stackunderflow");
                //writeObjectDeep(obj); printf("\n");

                // Handle marker for proc end
                if (obj.isMark()) {
                    // If it's a mark, pop the stack to the mark
                    //execStack().clearToMark();
                    break; // Successfully cleared to mark
                }



                // executable names, procedures, operators
                if (obj.isExecutable()) {
                    if (obj.isArray())
                        opStack().push(obj);
                    else if (obj.isName() || obj.isOperator())
                    {
                        execObject(obj);
                    }
                    else
                        return error("run(): typecheck, unknown executable type");
                } else
                    opStack().push(obj); // Otherwise, push it back to the operand stack

                if (isExitRequested())
                    break;
                if (isStopRequested())
                    break;
            }

            return true;
        }

        bool runProc(PSObject& proc)
        {
            // If the procedure is not executable, return an error
            //if (!proc.isArray() || !proc.isExecutable())
            //    return error("runProc: typecheck, NOT ARRAY or NOT EXECUTABLE");
            if (!proc.isArray())
                return error("runProc: typecheck, NOT ARRAY");

            // Push the endProc frame marker onto the execution stack
            execStack().mark();

            // push elements in reverse order
            auto arr = proc.asArray();
            for (auto it = arr->elements.rbegin(); it != arr->elements.rend(); ++it)
            {
                execStack().push(*it);
            }


            return run(); // Run the procedure
        }

        // This is a shim.  Mainly it needs to convert systemNamed objects
        // into system operators, which can be executed.
        bool genNextObject(PSObjectGenerator& objGen, PSObject& obj)
        {
            // return the next object from the object generator
            if (!objGen.next(obj))
                return false;

            // If the object is a system name, resolve it to an operator
            if (obj.isName() && obj.isSystemOp())
            {
                if (!systemdict->get(obj.asName(), obj)) {
                    return error("genNextObject: undefined system name", obj.asName().c_str());
                }

                // If the resolved object is an operator, use it
                if (obj.isOperator()) {
                    return true;
                }
                else {
                    return false; // Error, expected an operator
                    // Otherwise, treat it as a literal value
                    //return opStack().push(resolved);
                }
            }

            return true;
        }

        // interpret
        // deals with the stream of objects from the object generator
        //
        bool interpret(PSObjectGenerator& objGen)
        {
            while (true)
            {
                PSObject obj;

                // return the next object from the object generator
                // getting a 'false' return value means the end of the stream
                // it does not necessarily mean an error
                if (!genNextObject(objGen, obj))
                    break; //  error("END of Object stream");

                if (obj.isExecutable())
                {
                    if (obj.isArray())
                    {
                        if (!opStack().push(obj))
                            return error("interpreter: stack overflow while pushing executable array");
                    }
                    else {
                        execStack().push(obj); // Push executable objects onto the execution stack
                        if (!run())
                            return error("interpreter: run failed on executable object");
                    }
                }
                else {
                    if (!opStack().push(obj))
                        return error("interpreter: stack overflow while pushing non-executable object");
                }

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



        bool interpret(const PSFileHandle& file)
        {
            if (!file || !file->isValid())
                return error("interpretFile: invalid file handle");

            pushCurrentFile(file);

            // Use the file's cursor as the input stream
            if (!file->hasCursor())
                return error("interpretFile: file does not have a cursor");

            //OctetCursor cursor;
            //file->getCursor(cursor);

            PSObjectGenerator objGen(file);

            return interpret(objGen);
        }

        bool interpret(OctetCursor& input)
        {
            auto fileHandle = PSMemoryFile::create(input);
            return interpret(fileHandle);

            //PSObjectGenerator objGen(input);
            //return interpret(objGen);
        }

        bool interpret(const char *input)
        {
            OctetCursor cursor(input);
            return interpret(cursor);
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
