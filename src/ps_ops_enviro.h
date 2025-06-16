#pragma once
#include "psvm.h"

namespace waavs {


    bool op_languagelevel(PSVirtualMachine& vm) {
        // Push the current language level onto the stack
        vm.opStack().push(PSObject::fromInt(vm.languageLevel()));
        return true;
	}

    bool op_save(PSVirtualMachine& vm) {
        // Ask the VM for the PSSaveState object handle
        // push it onto the stack
        vm.opStack().push(PSObject::fromSave());

        return true;
    }

    bool op_restore(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("restore: stack underflow");

        // Pop the saved state from the stack
        // and apply it to the VM

        return true;
    }

    bool op_setpagedevice(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("setpagedevice: operand stack is empty");

        PSObject dictObj;
        s.pop(dictObj);

        if (!dictObj.isDictionary())
            return vm.error("setpagedevice: expected dictionary");

        auto dict = dictObj.asDictionary();

        PSObject pageSizeObj;
        if (!dict->get("PageSize", pageSizeObj))
            return vm.error("setpagedevice: missing PageSize key");

        if (!pageSizeObj.isArray())
            return vm.error("setpagedevice: PageSize must be an array");

        // Execute the array to resolve any procedure-based values
        if (!vm.execProc(pageSizeObj))
            return vm.error("setpagedevice: failed to evaluate PageSize array");

        // Retrieve evaluated results from operand stack (height on top)
        PSObject heightObj, widthObj;
        if (!s.pop(heightObj) || !s.pop(widthObj))
            return vm.error("setpagedevice: stackunderflow while retrieving PageSize values");

        double width, height;
        if (!widthObj.get(width) || !heightObj.get(height))
            return vm.error("setpagedevice: PageSize must contain numeric values");

        vm.graphics()->setPageSize(width, height);
        return true;
    }

    bool op_currentpagedevice(PSVirtualMachine& vm) {
        double w = 0.0, h = 0.0;
        vm.graphics()->getPageSize(w, h);

        auto arr = PSArray::create(2);
        arr->put(0, PSObject::fromReal(w));
        arr->put(1, PSObject::fromReal(h));

        auto dict = PSDictionary::create();
        dict->put("PageSize", PSObject::fromArray(arr));

        vm.opStack().push(PSObject::fromDictionary(dict));
        return true;
    }


    bool op_initgraphics(PSVirtualMachine& vm) {
        vm.graphics()->initGraphics();
        return true;
    }

    bool op_showpage(PSVirtualMachine& vm) {
        vm.graphics()->showPage();
        return true;
    }

    bool op_erasepage(PSVirtualMachine& vm) {
        vm.graphics()->erasePage();
        return true;
    }


    inline const PSOperatorFuncMap& getEnviroOps() {
        static const PSOperatorFuncMap table = {
            { "save",            op_save },                 // Save the current state of the VM
            { "restore",         op_restore },              // Restore the saved state of the VM

			{ "languagelevel",   op_languagelevel },
            { "setpagedevice",    op_setpagedevice },
            { "currentpagedevice", op_currentpagedevice },
            { "initgraphics",     op_initgraphics },
            { "showpage",         op_showpage },
            { "erasepage",        op_erasepage }
        };
        return table;
    }

}
