#pragma once

#include "pscore.h"
#include "psvm.h"

#include <bit>
#include <cstdint>


namespace waavs {

    inline bool op_and(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (a.isBool() && b.isBool()) {
            s.push(PSObject::fromBool(a.asBool() && b.asBool()));
            return true;
        }
        if (a.isInt() && b.isInt()) {
            s.push(PSObject::fromInt(a.asInt() & b.asInt()));
            return true;
        }
        return false;
    }

    inline bool op_or(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (a.isBool() && b.isBool()) {
            s.push(PSObject::fromBool(a.asBool() || b.asBool()));
            return true;
        }
        if (a.isInt() && b.isInt()) {
            s.push(PSObject::fromInt(a.asInt() | b.asInt()));
            return true;
        }
        return false;
    }

    inline bool op_xor(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (a.isBool() && b.isBool()) {
            s.push(PSObject::fromBool(a.asBool() != b.asBool()));
            return true;
        }
        if (a.isInt() && b.isInt()) {
            s.push(PSObject::fromInt(a.asInt() ^ b.asInt()));
            return true;
        }
        return false;
    }

    inline bool op_not(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject a;
        s.pop(a);

        if (a.isBool()) {
            s.push(PSObject::fromBool(!a.asBool()));
            return true;
        }
        if (a.isInt()) {
            s.push(PSObject::fromInt(~a.asInt()));
            return true;
        }
        return false;
    }

    inline bool op_bitshift(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("bitshift: stackunderflow");

        PSObject shiftObj, valueObj;
        s.pop(shiftObj);
        s.pop(valueObj);

        //if (!valueObj.isInt())
        //    return vm.error("bitshift: typecheck; expected integer value");

        //if (!shiftObj.isInt())
        //    return vm.error("bitshift: typecheck; expected integer shift");


        if (!valueObj.isInt() || !shiftObj.isInt())
        {
            /*
            // DIAGNOSTIC
            std::printf("\n=== ACTUAL BITSHIFT TYPECHECK ===\n");

            std::printf("value: ");
            writeObjectDeep(valueObj);

            std::printf("\nshift: ");
            writeObjectDeep(shiftObj);
            std::printf("\n");

            std::printf("Remaining operand stack:\n");
            for (size_t i = 0; i < std::min<size_t>(s.size(), 12); ++i)
            {
                PSObject item;
                if (!s.nth(i, item)) break;

                std::printf("[%zu] ", i);
                writeObjectDeep(item);
                std::printf("\n");
            }
            */

            return vm.error("bitshift: typecheck");
        }
        int32_t shift = shiftObj.asInt();
        uint32_t bits = static_cast<uint32_t>(valueObj.asInt());

        if (shift >= 32 || shift <= -32)
            bits = 0;
        else if (shift > 0)
            bits <<= static_cast<uint32_t>(shift);
        else if (shift < 0)
            bits >>= static_cast<uint32_t>(-static_cast<int64_t>(shift));

        /*
        // DIAGNOSTIC
        {
            static uint64_t bitshiftCalls = 0;

            if (++bitshiftCalls < 100)
            {
                std::printf("BITSHIFT: value=%d (0x%08X) shift=%d result=%d (0x%08X)\n",
                    valueObj.asInt(), static_cast<uint32_t>(valueObj.asInt()),
                    shift, std::bit_cast<int32_t>(bits), bits);
            }
        }
        */

        return s.pushInt(std::bit_cast<int32_t>(bits));
    }


    inline const PSOperatorFuncMap& getLogicOps()
    {
        static const PSOperatorFuncMap table = {
            { "and",        op_and },
            { "or",         op_or },
            { "xor",        op_xor },
            { "not",        op_not },
            { "bitshift",   op_bitshift }
        };
        return table;
    }

} // namespace waavs
