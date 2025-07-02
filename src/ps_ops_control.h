
#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    // ( proc -- ) Executes a procedure
    inline bool op_exec(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("exec::stackunderflow");

        PSObject proc;
        s.pop(proc);

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("exec::typecheck: expected procedure (array)");

        return vm.execProc(proc);
    }

    // ( bool proc -- ) If condition is true, execute procedure
    inline bool op_if(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject proc, cond;
        s.pop(proc);
        s.pop(cond);

        if (!cond.isBool())
            return vm.error("typecheck: expected boolean");

        if (cond.asBool())
            return vm.execProc(proc);

        return true;
    }

    // ( bool proc_true proc_false -- ) Conditional execution
    inline bool op_ifelse(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 3)
            return vm.error("stackunderflow");

        PSObject procFalse, procTrue, cond;
        s.pop(procFalse);
        s.pop(procTrue);
        s.pop(cond);

        if (!cond.isBool())
            return vm.error("typecheck: expected boolean");

        PSObject proc = cond.asBool() ? procTrue : procFalse;
        return vm.execProc(proc);
    }

    // ( count proc -- ) Repeat execution
    inline bool op_repeat(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject proc, count;
        s.pop(proc);
        s.pop(count);

        if (!count.isInt())
            return vm.error("typecheck: expected integer");

        int n = count.asInt();
        for (int i = 0; i < n; ++i) {
            if (!vm.execProc(proc))
                return vm.error("repeat:: execProc() failed");

            if (vm.isExitRequested()) {
                vm.clearExitRequest();
                break;
            }
            if (vm.isStopRequested()) {
                vm.clearStopRequest();
                break;
            }
        }

        return true;
    }

    // ( proc -- ) Infinite loop execution
    inline bool op_loop(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("stackunderflow");

        PSObject proc;
        s.pop(proc);

        while (true) {
            if (!vm.execProc(proc)) {
                return vm.error("op_loop:: execProc() failed");
			}

            if (vm.isExitRequested()) {
                vm.clearExitRequest();
                break;
            }
        }

        return true;
    }

    // ( -- ) Signal exit from a loop
    inline bool op_exit(PSVirtualMachine& vm) {
        vm.exit();
        return true;
    }

    // ( initial increment limit proc -- ) Numeric for-loop
    inline bool op_for(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 4)
            return vm.error("stackunderflow");

        PSObject proc, limit, increment, initial;
        s.pop(proc);
        s.pop(limit);
        s.pop(increment);
        s.pop(initial);

        if (!initial.isNumber() || !increment.isNumber() || !limit.isNumber())
            return vm.error("typecheck: expected numbers");

        double i = initial.asReal();
        double inc = increment.asReal();
        double lim = limit.asReal();

        while ((inc > 0 && i <= lim) || (inc < 0 && i >= lim)) {
            vm.opStack().push(PSObject::fromReal(i));

            if (!vm.execProc(proc))
                return false;

            if (vm.isExitRequested()) {
                vm.clearExitRequest();
                break;
            }

            i += inc;
        }

        return true;
    }

    inline bool op_forall(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("forall: stackunderflow");

        PSObject proc, container;
        s.pop(proc);
        s.pop(container);


        auto apply = [&](const PSObject& val1, const PSObject* val2 = nullptr) -> bool {
            if (val2) s.push(*val2);
            s.push(val1);

            if (!vm.execProc(proc)) {
                return vm.error("forall: failed to run procedure");
            }

            if (vm.isExitRequested()) {
                vm.clearExitRequest();
                return false; // exit terminates loop early
            }
            return true;
            };

        switch (container.type) {
        case PSObjectType::Array: {
            for (const auto& val : container.asArray()->elements) {
                if (!apply(val)) break;
            }
            return true;
        }

        case PSObjectType::String: {
            auto str = container.asString();
            for (int i = 0; i < str.length(); ++i) {
                PSObject obj;
                uint8_t byte;
                str.get(i, byte);
                obj = PSObject::fromInt(static_cast<unsigned char>(byte));
                if (!apply(obj)) break;
            }
            return true;
        }

        case PSObjectType::Dictionary: {
            auto applyToDict = [&](const PSName& keyName, const PSObject& val2) -> bool {
                s.push(val2);
                s.push(PSObject::fromName(keyName));

                if (!vm.execProc(proc)) {
                    return vm.error("forall: failed to run procedure");
                }

                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    return false; // exit terminates loop early
                }
                return true;
                };

                container.asDictionary()->forEach(applyToDict);
            return true;
        }

        default:
            return vm.error("forall: unsupported container type");
        }
    }

    // ( -- ) Signal stop condition
    inline bool op_stop(PSVirtualMachine& vm) {
        vm.stop();
        return true;
    }

    // ( proc -- bool ) Execute procedure with stop protection
    inline bool op_stopped(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("stackunderflow");

        PSObject proc;
        s.pop(proc);

        if (!proc.isArray() || !proc.isExecutable())
            return vm.error("typecheck");

        auto arr = proc.asArray();
        if (!arr)
            return vm.error("op_stopped: valuecheck");

        bool prevStop = vm.isStopRequested();
        vm.clearStopRequest();

        if (!vm.execProc(proc))
            return false;

        bool stopOccurred = vm.isStopRequested();

        if (prevStop)
            vm.stop();
        else
            vm.clearStopRequest();

        s.push(PSObject::fromBool(stopOccurred));
        return true;
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
