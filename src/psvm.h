// psvm.h
#pragma once


#include "pscore.h"
#include "ps_execution_frame.h"
#include "dictionarystack.h"
#include "ps_type_stack.h"
#include "ps_type_graphicscontext.h"
#include "ps_type_file.h"
#include "ps_scanner.h"
#include "ps_print.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <chrono>


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
        using Clock = std::chrono::steady_clock;
        Clock::time_point fStartTime = Clock::now();

		int fLanguageLevel = 2; // Default language level
        std::unique_ptr<PSGraphicsContext> graphicsContext_;
        
        PSObjectStack operandStack_;
        PSExecutionStack executionStack_;
        PSObjectStack fileStack;


        PSDictionaryHandle systemdict;
        PSDictionaryHandle statusdict;
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
            statusdict = PSDictionary::create();
            userdict = PSDictionary::create();
            systemResourceDirectory = PSDictionary::create();

            // setup for status dictionary
            // some product specific information
            systemdict->put(PSName("statusdict"), PSObject::fromDictionary(statusdict));
            statusdict->put(PSName("product"), PSObject::fromString(PSString("WaavScript")));

            // Set up dictionary stack (bottom to top):
            dictionaryStack.push(systemdict); // Lowest priority
            dictionaryStack.push(userdict);   // Highest priority

            fResourceStack.push(systemResourceDirectory);
        }

        // Access to properties and state
        int32_t realtime() const noexcept
        {
            using namespace std::chrono;

            const auto elapsed = duration_cast<milliseconds>(Clock::now() - fStartTime).count();
            return static_cast<int32_t>(static_cast<uint32_t>(elapsed));
        }

        PSDictionaryStack& getDictionaryStack() { return dictionaryStack; }
        const PSDictionaryStack& getDictionaryStack() const { return dictionaryStack; }
        // Convenience for accessing various dictionaries
        PSDictionaryHandle getSystemDict() const { return systemdict; }
        PSDictionaryHandle getUserDict() const { return userdict; }
        PSDictionaryHandle getStatusDict() const { return statusdict; }

        PSDictionaryHandle setUserDict(PSDictionaryHandle dict)
        {
            userdict = dict;
            return userdict;
        }
        
        PSDictionaryHandle currentDictionary() noexcept
        {
            return dictionaryStack.currentdict();
        }

        PSDictionaryHandle currentDictionary() const noexcept
        {
            return dictionaryStack.currentdict();
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
            return systemdict->put(name, PSObject::fromOperator(op));
        }

		// quickly register a set of operators
        bool registerOps(const PSOperatorFuncMap& ops)
        {
            bool success = true;

            for (const auto& entry : ops) 
            {
                if (!registerBuiltin(entry.first, entry.second))
                {
                    // report an error if registration fails
                    // but continue to attempt to register the rest of the operators
                    error("registerOps: failed to register operator: ", entry.first.c_str());
                    success = false;
                }
                
            }

            return success;
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
                    std::holds_alternative<PSPathForAllFrame>(item) ||
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

        bool scheduleFile(const PSFileHandle& file)
        {
            if (!file || !file->isValid())
                return error("scheduleFile: invalid file");

            if (!file->hasCursor())
                return error("scheduleFile: file has no cursor");

            auto generator = std::make_shared<PSObjectGenerator>(file);

            if (!pushCurrentFile(file))
                return false;

            if (!execStack().push(PSFileFrame{ file, generator }))
            {
                PSFileHandle ignored;
                popCurrentFile(ignored);
                return false;
            }

            return true;
        }

        bool scheduleEexec(const PSFileHandle& file)
        {
            if (!dictionaryStack.push(systemdict))
                return error("scheduleEexec: dictstackoverflow");

            if (!execStack().push(PSEexecFrame{}))
                return false;

            return scheduleFile(file);
        }

        bool scheduleFor(double initial, double increment, double limit, const PSObject& proc)
        {
            if (!proc.isArray() || !proc.isExecutable())
                return error("scheduleFor: typecheck");

            if (increment == 0)
                return error("scheduleFor: rangecheck");

            return execStack().push(PSForFrame{ initial, increment, limit, proc });
        }

        bool scheduleForAll(const PSObject& container, const PSObject& proc)
        {
            return execStack().push(PSForAllFrame{ container, proc, 0 });
        }

        bool PSVirtualMachine::schedulePathForAll(
            const PSObject& path,
            const PSObject& moveProc,
            const PSObject& lineProc,
            const PSObject& curveProc,
            const PSObject& closeProc)
        {
            PSMatrix inverseCTM;

            if (!graphics()->getCTM().inverse(inverseCTM))
                return error("pathforall: undefinedresult");

            PSPathForAllFrame frame;
            frame.path = path;
            frame.moveProc = moveProc;
            frame.lineProc = lineProc;
            frame.curveProc = curveProc;
            frame.closeProc = closeProc;
            frame.opIndex = 0;
            frame.argIndex = 0;
            frame.inverseCTM = inverseCTM;

            return execStack().push(frame);
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



        bool scheduleStopped(const PSObject& proc)
        {
            if (!proc.isArray() || !proc.isExecutable())
                return error("scheduleStopped: typecheck");

            if (!execStack().push(PSStoppedFrame{}))
                return false;

            return scheduleProcedure(proc);
        }

        bool scheduleImage(int32_t width, int32_t height, int32_t bitsPerComponent, const PSObject& matrix, const PSObject& procedure)
        {
            if (!procedure.isArray() || !procedure.isExecutable())
                return error("scheduleImage: typecheck; expected procedure");

            size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height);
            auto data = std::make_shared<PSString>(byteCount);

            if (!execStack().push(PSImageFrame{ width, height, bitsPerComponent, matrix, procedure, data, 0 }))
                return false;

            return scheduleProcedure(procedure);
        }

        bool scheduleKShow(const PSObject& string, const PSObject& procedure)
        {
            if (!string.isString())
                return error("scheduleKShow: typecheck; expected string");

            if (!procedure.isArray() || !procedure.isExecutable())
                return error("scheduleKShow: typecheck; expected procedure");

            return execStack().push(PSKShowFrame{ string, procedure, 0 });
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
 
        // =============================================================
        // run
        // 
        // Run items off the execution stack until it is empty or an exit/stop request is made.
        // This is the primary execution loop of the virtual machine.
        //
        // =============================================================

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
                    if (proc->index >= proc->procedure->size())
                        continue;

                    PSObject obj = (*proc->procedure)[proc->index++];

                    if (proc->index < proc->procedure->size())
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

                        if (frame->index >= arr->size())
                            break;

                        PSObject value = (*arr)[frame->index++];

                        if (frame->index < arr->size())
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
                else if (auto frame = std::get_if<PSPathForAllFrame>(&item))
                {
                    const PSPath& path = frame->path.asPath();
                    const PathProgram& prog = path.program();

                    while (frame->opIndex < prog.ops.size())
                    {
                        const PathOp op = static_cast<PathOp>(prog.ops[frame->opIndex++]);
                        const uint8_t arity = pathOpArity(op);

                        if (arity == kPathOpInvalidArity)
                            return error("pathforall: invalid path opcode");

                        if (frame->argIndex + arity > prog.args.size())
                            return error("pathforall: malformed path program");

                        const float* args = prog.args.data() + frame->argIndex;
                        frame->argIndex += arity;

                        PSObject proc;

                        switch (op)
                        {
                        case OP_END:
                            return true;

                        case OP_MOVETO:
                        {
                            double x;
                            double y;
                            frame->inverseCTM.transformPoint(args[0], args[1], x, y);

                            opStack().pushReal(x);
                            opStack().pushReal(y);

                            proc = frame->moveProc;
                            break;
                        }

                        case OP_LINETO:
                        {
                            double x;
                            double y;
                            frame->inverseCTM.transformPoint(args[0], args[1], x, y);

                            opStack().pushReal(x);
                            opStack().pushReal(y);

                            proc = frame->lineProc;
                            break;
                        }

                        case OP_CUBICTO:
                        {
                            double x1;
                            double y1;
                            double x2;
                            double y2;
                            double x3;
                            double y3;

                            frame->inverseCTM.transformPoint(args[0], args[1], x1, y1);
                            frame->inverseCTM.transformPoint(args[2], args[3], x2, y2);
                            frame->inverseCTM.transformPoint(args[4], args[5], x3, y3);

                            opStack().pushReal(x1);
                            opStack().pushReal(y1);
                            opStack().pushReal(x2);
                            opStack().pushReal(y2);
                            opStack().pushReal(x3);
                            opStack().pushReal(y3);

                            proc = frame->curveProc;
                            break;
                        }

                        case OP_CLOSE:
                            proc = frame->closeProc;
                            break;

                        case OP_QUADTO:
                        case OP_ARCTO:
                            return error("pathforall: non-PostScript path opcode");

                        default:
                            return error("pathforall: invalid path opcode");
                        }

                        if (frame->opIndex < prog.ops.size())
                            execStack().push(*frame);

                        if (!scheduleProcedure(proc))
                            return false;

                        break;
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
                else if (auto frame = std::get_if<PSImageFrame>(&item))
                {
                    if (opStack().empty())
                        return error("run(): image procedure returned no data");

                    PSObject result;
                    if (!opStack().pop(result))
                        return error("run(): image procedure returned no data");

                    if (!result.isString())
                        return error("run(): image procedure must return a string");

                    const PSString& chunk = result.asString();

                    if (chunk.length() == 0)
                        return error("run(): image procedure returned an empty string");

                    size_t expected = static_cast<size_t>(frame->width) * static_cast<size_t>(frame->height);
                    size_t remaining = expected - frame->written;
                    size_t count = std::min(remaining, static_cast<size_t>(chunk.length()));

                    for (size_t i = 0; i < count; ++i)
                    {
                        if (!frame->data->put(static_cast<uint32_t>(frame->written + i), chunk.data()[i]))
                            return error("run(): image data write failed");
                    }

                    frame->written += count;

                    if (frame->written < expected)
                    {
                        execStack().push(*frame);

                        if (!scheduleProcedure(frame->procedure))
                            return false;

                        continue;
                    }

                    frame->data->setLength(static_cast<uint32_t>(expected));

                    PSMatrix matrix;
                    if (!extractMatrix(frame->matrix, matrix))
                        return error("run(): invalid image matrix");

                    PSImage img;
                    img.width = frame->width;
                    img.height = frame->height;
                    img.bitsPerComponent = frame->bitsPerComponent;
                    img.transform = matrix;

                    auto source = PSMemoryFile::create(OctetCursor(frame->data->data(), expected));

                    if (!source)
                        return error("run(): failed to create image data source");

                    if (!graphics()->image(img, source))
                        return false;
                }

                else if (auto frame = std::get_if<PSFileFrame>(&item))
                {
                    PSObject obj;

                    if (!genNextObject(*frame->generator, obj))
                    {
                        PSFileHandle finishedFile;
                        if (!popCurrentFile(finishedFile))
                            return error("run(): file stack underflow");

                        continue;
                    }

                    if (!execStack().push(*frame))
                        return false;

                    if (!schedule(obj))
                        return false;
                }
                else if (std::holds_alternative<PSStoppedFrame>(item))
                {
                    if (!opStack().pushBool(false))
                        return false;
                }
                else if (auto frame = std::get_if<PSKShowFrame>(&item))
                {
                    const PSString& str = frame->string.asString();

                    if (frame->index >= str.length())
                        continue;

                    auto* grph = graphics();
                    if (!grph)
                        return error("run(): kshow has no graphics context");

                    const size_t currentIndex = frame->index;
                    const uint8_t ch = str.data()[currentIndex];

                    if (!grph->showText(grph->getCTM(), &ch, 1))
                        return error("run(): kshow failed to show character");

                    frame->index++;

                    // Last character has been shown. There is no following pair,
                    // so kshow is complete.
                    if (frame->index >= str.length())
                        continue;

                    const uint8_t nextCh = str.data()[frame->index];

                    // Schedule the continuation first, then the user procedure.
                    // Since the execution stack is LIFO, the procedure executes
                    // before the next character is shown.
                    execStack().push(*frame);

                    opStack().pushInt(static_cast<int32_t>(ch));
                    opStack().pushInt(static_cast<int32_t>(nextCh));

                    if (!scheduleProcedure(frame->procedure))
                        return false;
                }
                else if (std::holds_alternative<PSEexecFrame>(item))
                {
                    if (!dictionaryStack.pop())
                        return error("run(): eexec dictionary stack underflow");
                }
                // -------------------- END OF EXECUTION ITEM TYPES --------------------
                else {
                    return error("run(): unsupported execution item");
                }

            }

            return true;
        }

        // BUGBUG - TEMPORARY compatibility shim.
        // This no longer executes the procedure synchronously.
        // Remaining callers must be converted to scheduling semantics.
        bool runProc(PSObject& proc) = delete;
        //{
        //    return scheduleProcedure(proc);
        //}

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
            if (!scheduleFile(file))
                return false;

            return run();
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
