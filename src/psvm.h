#pragma once
#pragma once

#include <vector>
#include <memory>


#include "pscore.h"
#include "dictionarystack.h"
#include "ocspan.h"


namespace waavs
{
// Forward declaration for operators
struct PSVirtualMachine;

struct PSOperatorTable 
{
	std::unordered_map<const char *, PSOperator> ops;

    void add(const char * name, PSOperatorFunc fn) {
        ops[name] = PSOperator(name, fn);
    }

    PSOperator* lookup(const char * name) {
        auto it = ops.find(name);
        if (it != ops.end())
            return &it->second;
        return nullptr;
    }

    bool contains(const char * name) const 
    {
        return ops.find(name) != ops.end();
    }

    void clear() {
        ops.clear();
    }
};

struct PSVirtualMachine 
{
public:
    bool exitRequested = false;
    bool stopRequested = false;

public:
    std::vector<PSObject> operandStack;
    std::vector<PSObject> executionStack;
    PSDictionaryStack dictionaryStack;
    PSOperatorTable operatorTable;

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

    /*
    void registerOperator(const char* name, PSOperator op) {
        operatorTable.ops[name] = std::move(op);
        systemdict->put(name, PSObject::fromOperator(&operatorTable.ops[name]));
    }

	// When we register a builtin operator, we also add it to the system dictionary
    void defineOperator(const OctetCursor &nameSpan, PSOperatorFunc fn) 
    {
        const char* internedName = PSNameTable::intern(nameSpan);
		registerOperator(internedName, PSOperator(internedName, fn));
    }
    */

    // Mass registration of builtin operators.  This is NOT how user
    // defined operators are registered.
    void registerBuiltin(const char* name, PSOperatorFunc fn)
    {
        const char* interned = PSNameTable::INTERN(name);
        PSOperator op(interned, fn);
        operatorTable.ops[interned] = op;
        systemdict->put(interned, PSObject::fromOperator(&operatorTable.ops[interned]));
    }

    void registerOps(const PSOperatorFuncMap &ops)
    {
        for (const auto& entry  : ops) {
            registerBuiltin(entry.first, entry.second);
        }
    }

    // managing operand stack
    bool peek(PSObject& out) const {
        if (operandStack.empty()) return false;
        out = operandStack.back();
        return true;
    }

    bool pop(PSObject& out) {
        if (operandStack.empty()) return false;
        out = operandStack.back();
        operandStack.pop_back();
        return true;
    }

    bool popNumber(double& out) {
        if (operandStack.empty()) return false;
        const PSObject& top = operandStack.back();
        if (!top.isNumber()) return false;
        out = top.asReal();
        operandStack.pop_back();
        return true;
    }

    void push(const PSObject& obj) {
        operandStack.push_back(obj);
    }

    void pushOperator(PSOperator* op) {
        operandStack.push_back(PSObject::fromOperator(op));
    }

    void pushInt(int32_t val) {
        operandStack.push_back(PSObject::fromInt(val));
    }





	// managing execution stack
    void exit() { exitRequested = true; }
    bool isExitRequested() const { return exitRequested; }
    void clearExitRequest() { exitRequested = false; }

    void stop() { stopRequested = true; }
    bool isStopRequested() const { return stopRequested; }
    void clearStop() { stopRequested = false; }


    bool execute(const PSObject& obj);
    bool executeName(const char * name);

    void execArray(PSArray* proc);
    void bindArray(PSArray* proc);
    void bind();

    bool isBuildingProc() const {
        return buildProcDepth > 0;
    }

    void beginProc() {
        operandStack.push_back(PSObject::fromMark());
        buildProcDepth++;
    }

    PSArray* endProc() {
        // Pop everything to mark, build an array
        std::vector<PSObject> content;

        while (!operandStack.empty()) {
            PSObject obj = operandStack.back();
            operandStack.pop_back();
            if (obj.type == PSObjectType::Mark)
                break;
            content.insert(content.begin(), obj);
        }

        buildProcDepth--;

        auto* proc = new PSArray();
        proc->elements = std::move(content);
        proc->setExecutable(true);

        return proc;
    }

};

bool PSVirtualMachine::executeName(const char * name) 
{
    PSObject obj;
    if (!dictionaryStack.load(name, obj)) {
        // Name not found � treat as literal
        push(PSObject::fromName(name));
        return true;
    }

    if (obj.isExecutable()) {
        return execute(obj);
    }

    // Literal value � push it
    push(obj);
    return true;
}


// --- Execute a single PSObject
inline bool PSVirtualMachine::execute(const PSObject& obj) {
    switch (obj.type) {
    case PSObjectType::Operator:
        if (obj.data.op && obj.data.op->func)
            return obj.data.op->func(*this);
        return false;

    case PSObjectType::Array:
        if (obj.data.arr && obj.data.arr->isExecutable()) {
            execArray(obj.data.arr);
            return true;
        }
        operandStack.push_back(obj);
        return true;

    case PSObjectType::Name: {
        PSObject resolved;
        if (dictionaryStack.load(obj.data.name, resolved)) {
            return execute(resolved);
        }
        // undefined name: treat as literal
        operandStack.push_back(obj);
        return true;
    }

    default:
        operandStack.push_back(obj);
        return true;
    }
}

// --- Execute elements of a procedure array
inline void PSVirtualMachine::execArray(PSArray* proc) {
    if (!proc) return;

    for (const auto& item : proc->elements) {
        execute(item);
    }
}

// --- Replace names in procedure with operator pointers
inline void PSVirtualMachine::bindArray(PSArray* proc) {
    if (!proc) return;

    for (auto& obj : proc->elements) {
        if (obj.type == PSObjectType::Name) {
            PSOperator* op = operatorTable.lookup(obj.data.name);
            if (op) {
                obj = PSObject::fromOperator(op);
            }
        }
    }
}

// --- Bind a procedure (array on stack)
inline void PSVirtualMachine::bind() {
    if (operandStack.empty()) return;

    PSObject obj = operandStack.back();
    operandStack.pop_back();

    if (obj.type == PSObjectType::Array && obj.data.arr) {
        bindArray(obj.data.arr);
        operandStack.push_back(obj); // push updated array
    }
}
}
