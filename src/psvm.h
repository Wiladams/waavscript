#pragma once


#include <vector>
#include <memory>


#include "pscore.h"
#include "dictionarystack.h"
#include "ocspan.h"
#include "psstack.h"


namespace waavs
{
    // Forward declaration for operators
    struct PSVirtualMachine;

    using PSOperatorImpl = bool (*)(PSVirtualMachine&, const PSOperatorArgs& args);
/*
    struct PSOperatorEntry {
        PSOperatorSignature signature{};
        PSOperatorImpl function{ nullptr > ;
        };

        using PSOperatorEntryMap = std::unordered_map<const char*, PSOperatorEntry>;
*/

    struct PSOperatorTable
    {
        std::unordered_map<const char*, PSOperator> ops;

        void add(const char* name, PSOperatorFunc fn) 
        {
            ops[name] = PSOperator(name, fn);
        }

        const PSOperator* lookup(const char* name) const 
        {
            auto it = ops.find(name);
            if (it != ops.end())
                return &it->second;
            return nullptr;
        }

        bool contains(const char* name) const
        {
            return ops.find(name) != ops.end();
        }

        void clear() {
            ops.clear();
        }
    };

    /*
    struct PSOperatorEntryTable
    {
        std::unordered_map<const char*, PSOperatorEntry> ops;

        void add(const char* name, PSOperatorEntry entry) {
            ops[name] = entry;
        }

        PSOperatorEntry* lookup(const char* name) {
            auto it = ops.find(name);
            if (it != ops.end())
                return &it->second;
            return nullptr;
        }

        bool contains(const char* name) const
        {
            return ops.find(name) != ops.end();
        }

        void clear() {
            ops.clear();
        }
    };

    */

    using PSExecutionStack = PSStack<PSObject>;

    struct PSVirtualMachine
    {
    private:
        PSOperandStack operandStack_;
        PSExecutionStack executionStack_;
		bool stopRequested = false;
        bool exitRequested = false;

    public:


        PSDictionaryStack dictionaryStack;
        PSOperatorTable operatorTable;
		//PSOperatorEntryTable operatorEntryTable;

        std::shared_ptr<PSDictionary> systemdict;
        std::shared_ptr<PSDictionary> userdict;

        int buildProcDepth = 0;
        int32_t randSeed = 1;

    public:
        PSVirtualMachine()
        {
            systemdict = std::make_shared<PSDictionary>();
            userdict = std::make_shared<PSDictionary>();

            // Set up dictionary stack (bottom to top):
            dictionaryStack.push(systemdict); // Lowest priority
            dictionaryStack.push(userdict);   // Highest priority

        }

        // stack access
        inline PSOperandStack& opStack() { return operandStack_; }
        inline const PSOperandStack& opStack() const { return operandStack_; }

        inline PSExecutionStack& execStack() { return executionStack_; }
        inline const PSExecutionStack& execStack() const { return executionStack_; }


        /*
        //#define PS_OP(NAME, SIG, FN) { { NAME, SIG }, FN }

        void registerBuiltin(const PSOperatorEntry& entry) 
        {
            const char* interned = PSNameTable::INTERN(entry.signature.name);
			operatorEntryTable.add(interned, entry);
        }

        void registerBuiltins(PSOperatorEntryMap& entries)
        {
            for (const auto& entry : entries) {
                registerBuiltin(entry.second);
            }
		}
        */

		//=====================================================================
		// OLD WAY OF REGISTERING OPERATORS
		//======================================================================
        // Mass registration of builtin operators.  This is NOT how user
        // defined operators are registered.
        void registerBuiltin(const char* name, PSOperatorFunc fn)
        {
            const char* interned = PSNameTable::INTERN(name);
            PSOperator op(interned, fn);
            operatorTable.ops[interned] = op;
            systemdict->put(interned, PSObject::fromOperator(&operatorTable.ops[interned]));
        }

        void registerOps(const PSOperatorFuncMap& ops)
        {
            for (const auto& entry : ops) {
                registerBuiltin(entry.first, entry.second);
            }
        }

        const PSOperator * lookupOperator(const char * name) const
        {
			return operatorTable.lookup(name);
		}

        bool lookupName(const char* name, PSObject& out) const
        {
            return dictionaryStack.load(name, out);
        }



        // managing execution stack
        bool error(const char* message) const {
            printf("%% Error: %s\n", message);
            return false;
        }

        bool error(const char* message, const char* detail) const {
            printf("%% Error: %s (%s)\n", message, detail);
            return false;
        }


        void exit() { 
            exitRequested = true; 
        }
        bool isExitRequested() const { return exitRequested; }
        void clearExitRequest() { exitRequested = false; }

        void stop() { stopRequested = true; }
        bool isStopRequested() const { return stopRequested; }
        void clearStopRequest() { stopRequested = false; }

        bool run() {
            while (!execStack().empty()) {
                PSObject obj = execStack().pop();
                if (!execute(obj)) return false;

                if (isExitRequested()) break;

                if (isStopRequested()) {
                    //printf("Execution stopped.\n");
                    clearStopRequest();
                    return true; // Stop is not an error, just a pause
				}
            }
            return true;
        }

        bool execute(const PSObject& obj);
        bool executeName(const char* name);


        // --- Execute elements of a procedure array
        /*
        inline bool execArray(PSArray* arr)
        {
            if (!arr) return false;

            for (size_t i = 0; i < arr->size(); ++i) {
                PSObject obj;
                arr->get(static_cast<int>(i), obj);

                // Canonicalize procedures
                if (obj.isArray() && obj.asArray()->isProcedure() && !obj.isExecutable()) {
                    obj.setExecutable(true);
                }

                if (obj.isExecutable()) {
                    execStack().push(obj);
                }
                else {
                    opStack().push(obj);
                }
            }

            return run();
        }
        */

        
        inline bool execArray(PSArray* arr) 
        {
            if (!arr) return false;

            for (int i = static_cast<int>(arr->size()) - 1; i >= 0; --i) {
                PSObject obj;
                arr->get(i, obj);
                execStack().push(obj);
            }

            // Now run the execution loop
            return run();
        }
        

        void bindArray(PSArray* proc);
        void bind();

    };

    //====================================================================
// New execution model coming online.  runOperator()
//====================================================================
/*
    bool PSVirtualMachine::runOperator(const PSOperatorEntry& entry)
    {
        const auto& sig = entry.signature;
        auto& s = this->opStack();

        if (s.size() < sig.arity)
            return false;

        PSOperatorArgs args{ sig, {}, sig.arity };

        for (size_t i = 0; i < sig.arity; ++i)
            if (!s.pop(args.values[i]))
                return false;

		// reverse direction of arguments if needed
        for (size_t i = 0; i < sig.arity / 2; ++i)
            std::swap(args.values[i], args.values[sig.arity - 1 - i]);

		// Check types of arguments
        for (size_t i = 0; i < sig.arity; ++i)
            if (!args.values[i].is(sig.kinds[i]))
                return false;

		// Call the operator's function, passing the VM and arguments
        return entry.function(*this, args);
    }
    */

    bool PSVirtualMachine::executeName(const char* name)
    {
        PSObject obj;
        if (!dictionaryStack.load(name, obj)) {
            // Name not found � treat as literal
            opStack().push(PSObject::fromName(name));
            return true;
        }

        if (obj.isExecutable()) {
            return execute(obj);
        }

        // Literal value � push it
        opStack().push(obj);
        return true;
    }


    // --- Execute a single PSObject
    bool PSVirtualMachine::execute(const PSObject& obj)
    {
        // If it's a name (possibly executable), try resolving it
        if (obj.isName()) {
            const char* name = obj.asName();

            // 1. Try operator table (built-in)
            const PSOperator* op = lookupOperator(name);
            if (op) {
                return op->func(*this);  // Execute the operator directly
            }

            // 2. Try dictionary stack
            PSObject resolved;
            if (lookupName(name, resolved)) {
                return execute(resolved);
            }

            // 3. Not found — undefined error
            printf("Error: undefined name '%s'\n", name);
            return false;
        }

        // If it's an executable array, execute it
        if (obj.isArray() && obj.isExecutable()) {
            return execArray(obj.asArray());
        }

        // Any other object — push to operand stack
        opStack().push(obj);
        return true;
    }




    // --- Replace names in procedure with operator pointers
    inline void PSVirtualMachine::bindArray(PSArray* proc) {
        if (!proc) return;

        for (auto& obj : proc->elements) {
            if (obj.type == PSObjectType::Name) {
                const PSOperator* op = operatorTable.lookup(obj.data.name);
                if (op) {
                    obj = PSObject::fromOperator(op);
                }
            }
        }
    }

    // --- Bind a procedure (array on stack)
    inline void PSVirtualMachine::bind() {
        if (opStack().empty()) return;

        PSObject obj;
        opStack().pop(obj);


        if (obj.isArray() && obj.asArray()) {
            bindArray(obj.asArray());
            opStack().push(obj); // push updated array
        }
    }


}
