// psvm.h
#pragma once


#include <vector>
#include <memory>


#include "pscore.h"
#include "ps_execution_frame.h"
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

    using PSExecutionStack = PSStack<PSExecutionItem>;


    struct PSVirtualMachine
    {
    private:
		int fLanguageLevel = 2; // Default language level
        std::unique_ptr<PSGraphicsContext> graphicsContext_;
        
        PSObjectStack operandStack_;
        PSExecutionStack executionStack_;
        PSObjectStack fileStack;


        PSDictionaryHandle systemdict;
        PSDictionaryHandle userdict;
        PSDictionaryHandle systemResourceDirectory;


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

        // Resource management
        PSDictionaryHandle getSystemResourceDirectory() const { return systemResourceDirectory; }
        PSDictionaryStack& getResourceStack() { return fResourceStack; }
        const PSDictionaryStack& getResourceStack() const { return fResourceStack; }

        bool findResource(const PSName& key, const PSName& category, PSObject& out)
        {
            PSObject categoryObj;

            if (!fResourceStack.load(category, categoryObj))
                return false;

            if (!categoryObj.isDictionary())
                return false;

            return categoryObj.asDictionary()->get(key, out);
        }

        bool defineResource(const PSName& key, const PSName& category, const PSObject& value)
        {
            PSObject categoryObj;

            if (!fResourceStack.currentdict()->get(category, categoryObj))
            {
                auto categoryDict = PSDictionary::create();
                fResourceStack.currentdict()->put(category, PSObject::fromDictionary(categoryDict));
                categoryObj = PSObject::fromDictionary(categoryDict);
            }

            if (!categoryObj.isDictionary())
                return false;

            return categoryObj.asDictionary()->put(key, value);
        }


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


        bool exit()
        {
            PSExecutionItem item;

            while (execStack().pop(item))
            {
                if (std::holds_alternative<PSRepeatFrame>(item) ||
                    std::holds_alternative<PSLoopFrame>(item) ||
                    std::holds_alternative<PSForFrame>(item) ||
                    std::holds_alternative<PSForAllFrame>(item) ||
                    std::holds_alternative<PSResourceForAllFrame>(item))
                    return true;
            }

            return error("exit: invalidexit");
        }


        // request a stop to the currently executing loop
        bool stop()
        {
            PSExecutionItem item;

            while (execStack().pop(item))
            {
                if (std::holds_alternative<PSStoppedFrame>(item))
                    return opStack().pushBool(true);
            }

            return error("stop: no enclosing stopped context");
        }


        


 public:
        // Scheduling things
        bool schedule(const PSObject& obj)
        {
            return execStack().push(PSScheduledObject(obj));
        }

        bool scheduleProcedure(const PSObject& proc)
        {
            if (!proc.isArray())
                return error("scheduleProcedure: typecheck, NOT ARRAY");

            return execStack().push(PSProcedureFrame{ proc.asArray(), 0 });
        }

        bool scheduleForAll(const PSObject& container, const PSObject& proc)
        {
            return execStack().push(PSForAllFrame{ container, proc, 0 });
        }

        bool scheduleLoop(const PSObject& proc)
        {
            if (!proc.isArray() || !proc.isExecutable())
                return error("scheduleLoop: typecheck");

            return execStack().push(PSLoopFrame{ proc });
        }

        bool scheduleResourceForAll(const PSName& category, const PSObject& proc)
        {
            return execStack().push(PSResourceForAllFrame{ category, proc, 0, 0 });
        }

        bool scheduleRepeat(const PSObject& proc, int32_t count)
        {
            if (!proc.isArray() || !proc.isExecutable())
                return error("scheduleRepeat: typecheck");

            if (count < 0)
                return error("scheduleRepeat: rangecheck");

            if (count == 0)
                return true;

            return execStack().push(PSRepeatFrame{ proc, count });
        }

        bool scheduleFor(double initial, double increment, double limit, const PSObject& proc)
        {
            if (!proc.isArray() || !proc.isExecutable())
                return error("scheduleFor: typecheck");

            if (increment == 0)
                return error("scheduleFor: rangecheck");

            return execStack().push(PSForFrame{ initial, increment, limit, proc });
        }

        bool scheduleStopped(const PSObject& proc)
        {
            if (!proc.isArray() || !proc.isExecutable())
                return error("scheduleStopped: typecheck");

            if (!execStack().push(PSStoppedFrame{}))
                return false;

            return scheduleProcedure(proc);
        }





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

            if (resolved.isOperator()) 
            {
                return execOperator(resolved);
            }

            // 3. Name resolves to a procedure?  auto-exec
            if (resolved.isArray() && resolved.isExecutable()) 
            {
                return scheduleProcedure(resolved);
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
        // This is the primary execution loop of the virtual machine.
        //
        bool run()
        {
            while (!execStack().empty())
            {

                PSExecutionItem item;
                if (!execStack().pop(item))
                    return error("run(): stackunderflow");

                if (auto scheduled = std::get_if<PSScheduledObject>(&item))
                {
                    const PSObject& obj = scheduled->object;

                    if (obj.isExecutable())
                    {
                        if (obj.isArray()) {
                            opStack().push(obj);
                        }
                        else if (obj.isName() || obj.isOperator()) {
                            if (!execObject(obj))
                                return false;
                        }
                        else {
                            return error("run(): typecheck, unknown executable type");
                        }
                    }
                    else {
                        opStack().push(obj);
                    }
                }
                else if (auto proc = std::get_if<PSProcedureFrame>(&item))
                {
                    if (proc->index >= proc->procedure->elements.size())
                        continue;

                    PSObject obj = proc->procedure->elements[proc->index++];

                    if (proc->index < proc->procedure->elements.size())
                        execStack().push(*proc);

                    if (!schedule(obj))
                        return false;
                }
                else if (auto frame = std::get_if<PSForAllFrame>(&item))
                {
                    switch (frame->container.type)
                    {
                    case PSObjectType::Array:
                    {
                        auto arr = frame->container.asArray();

                        if (frame->index >= arr->elements.size())
                            break;

                        PSObject value = arr->elements[frame->index++];

                        if (frame->index < arr->elements.size())
                            execStack().push(*frame);

                        opStack().push(value);

                        if (!scheduleProcedure(frame->procedure))
                            return false;

                        break;
                    }

                    case PSObjectType::String:
                    {
                        auto str = frame->container.asString();

                        if (frame->index >= static_cast<size_t>(str.length()))
                            break;

                        uint8_t byte;
                        if (!str.get(static_cast<int>(frame->index++), byte))
                            return error("run(): forall string access failed");

                        if (frame->index < static_cast<size_t>(str.length()))
                            execStack().push(*frame);

                        opStack().pushInt(static_cast<int32_t>(byte));

                        if (!scheduleProcedure(frame->procedure))
                            return false;

                        break;
                    }

                    case PSObjectType::Dictionary:
                    {
                        auto dict = frame->container.asDictionary();

                        PSName key;
                        PSObject value;

                        if (!dict->nextEntry(frame->index, key, value))
                            break;

                        execStack().push(*frame);

                        opStack().pushLiteralName(key);
                        opStack().push(value);

                        if (!scheduleProcedure(frame->procedure))
                            return false;

                        break;
                    }

                    default:
                        return error("run(): invalid forall container");
                    }
                }
                else if (auto frame = std::get_if<PSResourceForAllFrame>(&item))
                {
                    auto& rs = getResourceStack();

                    while (frame->resourceIndex < rs.size())
                    {
                        PSDictionaryHandle resourceDir;
                        if (!rs.getFromTop(frame->resourceIndex, resourceDir))
                            return error("run(): invalid resourceforall resource index");

                        PSObject categoryObj;
                        if (!resourceDir->get(frame->category, categoryObj) || !categoryObj.isDictionary())
                        {
                            frame->resourceIndex++;
                            frame->entryIndex = 0;
                            continue;
                        }

                        auto categoryDict = categoryObj.asDictionary();

                        PSName key;
                        PSObject value;

                        if (!categoryDict->nextEntry(frame->entryIndex, key, value))
                        {
                            frame->resourceIndex++;
                            frame->entryIndex = 0;
                            continue;
                        }

                        execStack().push(*frame);

                        opStack().pushLiteralName(key);
                        opStack().push(value);

                        if (!scheduleProcedure(frame->procedure))
                            return false;

                        break;
                    }
                }
                else if (auto frame = std::get_if<PSRepeatFrame>(&item))
                {
                    if (frame->remaining <= 0)
                        continue;

                    frame->remaining--;

                    if (frame->remaining > 0)
                        execStack().push(*frame);

                    if (!scheduleProcedure(frame->procedure))
                        return false;
                }
                else if (auto frame = std::get_if<PSLoopFrame>(&item))
                {
                    execStack().push(*frame);

                    if (!scheduleProcedure(frame->procedure))
                        return false;
                }
                else if (auto frame = std::get_if<PSForFrame>(&item))
                {
                    bool inRange = frame->increment > 0 ? frame->current <= frame->limit : frame->current >= frame->limit;

                    if (!inRange)
                        continue;

                    double value = frame->current;
                    frame->current += frame->increment;

                    execStack().push(*frame);
                    opStack().pushReal(value);

                    if (!scheduleProcedure(frame->procedure))
                        return false;
                }
                else if (std::holds_alternative<PSStoppedFrame>(item))
                {
                    if (!opStack().pushBool(false))
                        return false;
                }
                else {
                    return error("run(): unsupported execution item");
                }

            }

            return true;
        }

        // BUGBUG - TEMPORARY compatibility shim.
        // This no longer executes the procedure synchronously.
        // Remaining callers must be converted to scheduling semantics.
        bool runProc(PSObject& proc)
        {
            return scheduleProcedure(proc);
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
                        schedule(obj); // Push executable objects onto the execution stack
                        if (!run())
                            return error("interpreter: run failed on executable object");
                    }
                }
                else {
                    if (!opStack().push(obj))
                        return error("interpreter: stack overflow while pushing non-executable object");
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
