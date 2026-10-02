// ps_ops_control.h
#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    // ( proc -- ) Executes a procedure
    inline bool op_exec(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_exec: stackunderflow");

        PSObject proc;
        if (!s.pop(proc))
            return vm.error("op_exec: stackunderflow");

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("op_exec: typecheck; expected procedure (array)");

        return vm.scheduleProcedure(proc);
    }


    // ( bool proc -- ) If condition is true, execute procedure
    inline bool op_if(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_if: stackunderflow");

        PSObject proc;
        PSObject cond;

        s.pop(proc);
        s.pop(cond);

        if (!cond.isBool())
            return vm.error("op_if: typecheck; expected boolean");

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("op_if: typecheck; expected procedure (array)");

        if (!cond.asBool())
            return true;

        return vm.scheduleProcedure(proc);
    }

    // ( bool proc_true proc_false -- ) Conditional execution
    inline bool op_ifelse(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 3)
            return vm.error("op_ifelse: stackunderflow");

        PSObject procFalse;
        PSObject procTrue;
        PSObject cond;

        s.pop(procFalse);
        s.pop(procTrue);
        s.pop(cond);

        if (!cond.isBool())
            return vm.error("op_ifelse: typecheck; expected boolean");

        if (!procTrue.isArray() || !procTrue.isExecutable())
            return vm.error("op_ifelse: typecheck; expected true procedure");

        if (!procFalse.isArray() || !procFalse.isExecutable())
            return vm.error("op_ifelse: typecheck; expected false procedure");

        return vm.scheduleProcedure(cond.asBool() ? procTrue : procFalse);
    }

    // ( count proc -- ) Repeat execution
    inline bool op_repeat(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("repeat: stackunderflow");

        PSObject proc;
        int32_t count;

        s.pop(proc);

        if (!s.popInt(count))
            return vm.error("repeat: typecheck; expected integer");

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("repeat: typecheck; expected procedure");

        return vm.scheduleRepeat(proc, count);
    }

    // ( proc -- ) Infinite loop execution
    inline bool op_loop(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_loop: stackunderflow");

        PSObject proc;
        s.pop(proc);

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("op_loop: typecheck; expected procedure (array)");

        return vm.scheduleLoop(proc);
    }

    // ( -- ) Signal exit from a loop
    inline bool op_exit(PSVirtualMachine& vm) 
    {
        return vm.exit();
    }

    // ( initial increment limit proc -- ) Numeric for-loop
    inline bool op_for(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 4)
            return vm.error("op_for: stackunderflow");

        PSObject proc;
        PSObject limit;
        PSObject increment;
        PSObject initial;

        s.pop(proc);
        s.pop(limit);
        s.pop(increment);
        s.pop(initial);

        if (!initial.isNumber() || !increment.isNumber() || !limit.isNumber())
            return vm.error("op_for: typecheck; expected numbers");

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("op_for: typecheck; expected procedure (array)");

        return vm.scheduleFor(initial.asReal(), increment.asReal(), limit.asReal(), proc);
    }

    inline bool op_forall(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("forall: stackunderflow");

        PSObject proc;
        PSObject container;

        if (!s.pop(proc))
            return vm.error("forall: missing procedure");

        if (!s.pop(container))
            return vm.error("forall: missing container");

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("forall: procedure must be executable");

        switch (container.type)
        {
        case PSObjectType::Array:
        case PSObjectType::String:
        case PSObjectType::Dictionary:
            return vm.scheduleForAll(container, proc);

        default:
            return vm.error("forall: unsupported container type");
        }
    }


    // ( -- ) Signal stop condition
    inline bool op_stop(PSVirtualMachine& vm) 
    {
        return vm.stop();
    }

    // ( proc -- bool ) Execute procedure with stop protection
    inline bool op_stopped(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_stopped: stackunderflow");

        PSObject proc;
        if (!s.pop(proc))
            return vm.error("op_stopped: stackunderflow");

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("op_stopped: typecheck");

        return vm.scheduleStopped(proc);
    }

    // Operator table
    inline const PSOperatorFuncMap& getControlOps() {
        static const PSOperatorFuncMap table = {
            { "exec",      op_exec },
            { "if",        op_if },
            { "ifelse",    op_ifelse },
            { "repeat",    op_repeat },
            { "loop",      op_loop },
            { "exit",      op_exit },
            { "for",       op_for },
            { "forall",     op_forall},
            { "stop",      op_stop },
            { "stopped",   op_stopped },
        };
        return table;
    }

} // namespace waavs
