#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    static const PSOperatorFuncMap stackOps = {

        { "dup", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject top;
            if (!s.top(top))
				return false;
			s.push(top);

			return true;
        }},

        { "pop", [](PSVirtualMachine& vm)  -> bool {
            PSObject obj;
			return vm.opStack().pop(obj);
        }},

        { "exch", [](PSVirtualMachine& vm)  -> bool {
            return vm.opStack().exch();
        }},

		// Finds the 'index'th item on the stack and duplicates it 
        // on the top of the stack
        { "index", [](PSVirtualMachine& vm)  -> bool {
			// Top of stack should be an integer indicating the index
			// of the object to retrieve (0-based index)
			auto& s = vm.opStack();

            PSObject countObj;
			
			if (!s.pop(countObj))
                return false;

			// type check for integer
            if (!countObj.isInt())
                return false;

            int n = countObj.asInt();

			// Try to get the nth object from the stack
            PSObject nthObj;
            if (!s.nth(n, nthObj)) 
				return false;

			// Push the nth object back onto the stack
            return s.push(nthObj);
        }},

        { "roll", [](PSVirtualMachine& vm)  -> bool {
			auto& s = vm.opStack();

            PSObject jObj;
            PSObject nObj;

            if (!s.pop(jObj)) return false;
            if (!s.pop(nObj)) return false;

			// type check for integers
            if (!jObj.isInt() || !nObj.isInt())
				return false;

            // get actual values
            int count = nObj.asInt();
            int shift = jObj.asInt();

            return s.roll(count, shift);
        }},

        // clear
        // removes all items from the stack
        { "clear", [](PSVirtualMachine& vm)  -> bool {
			return vm.opStack().clear();
        }},

        // count
		// Pushes the count of the current size of the operand stack 
        // onto the operand stack
        { "count", [](PSVirtualMachine& vm)  -> bool {
            int count = static_cast<int>(vm.opStack().size());
			return vm.opStack().push(PSObject::fromInt(count));
        }},

        // Places a mark on the current position of the operand stack
        { "mark", [](PSVirtualMachine& vm)  -> bool {
            return vm.opStack().mark();
        }},

        // Removes objects from the operand stack up to and including the most recent mark
        { "cleartomark", [](PSVirtualMachine& vm)  -> bool {
            return vm.opStack().clearToMark();
        }},

		// Counts the number of objects on the operand stack until it finds a mark object
        { "counttomark", [](PSVirtualMachine& vm)  -> bool {
            int count = 0;
            if (!vm.opStack().countToMark(count))
                return false;
			
            return vm.opStack().push(PSObject::fromInt(count));
        }},
    };

} // namespace waavs
