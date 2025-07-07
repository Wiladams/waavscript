#pragma once
#include "psvm.h"

namespace waavs {


    bool op_languagelevel(PSVirtualMachine& vm) {
        // Push the current language level onto the stack
        vm.opStack().pushInt(vm.languageLevel());

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
        PSObject saveObj;
        s.pop(saveObj);

        return true;
    }

    bool op_setpagedevice(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();

        if (ostk.empty())
            return vm.error("setpagedevice: operand stack is empty");

        PSObject dictObj;
        ostk.pop(dictObj);

        if (!dictObj.isDictionary())
            return vm.error("op_setpagedevice: expected dictionary");

        auto dict = dictObj.asDictionary();

        PSObject pageSizeObj;
        if (!dict->get("PageSize", pageSizeObj))
            return vm.error("setpagedevice: missing PageSize key");

        if (!pageSizeObj.isArray())
            return vm.error("setpagedevice: PageSize must be an array");

        auto pageSizeArray = pageSizeObj.asArray();
        PSObject widthObj;
        PSObject heightObj;
        pageSizeArray->get(0, widthObj);
        pageSizeArray->get(1, heightObj);

        vm.graphics()->setPageSize(widthObj.asReal(), heightObj.asReal());
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

    bool op_initclip(PSVirtualMachine& vm) {
        // Initialize the clip path to an empty state
        vm.graphics()->initClipPath();
        return true;
    }

    bool op_initgraphics(PSVirtualMachine& vm) {
        auto& ctm = vm.graphics()->getCTM();

        //reset();              // Clear state stack
        ctm.reset();           // Identity CTM
        //newpath();            // Clear current path
        //setRGB(0, 0, 0);      // Default black color
        //setLineWidth(1);      // Default line width
        //initClipPath();      // Default clip path

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

			{ "languagelevel",      op_languagelevel },
            { "setpagedevice",      op_setpagedevice },
            { "currentpagedevice",  op_currentpagedevice },
            { "initclip",           op_initclip},
            { "initgraphics",       op_initgraphics },
            { "showpage",           op_showpage },
            { "erasepage",          op_erasepage }
        };
        return table;
    }

}
