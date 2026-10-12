#pragma once
#include "psvm.h"

namespace waavs {

    inline bool op_usertime(PSVirtualMachine& vm)
    {
        return vm.opStack().pushInt(vm.usertime());
    }

    inline bool op_realtime(PSVirtualMachine& vm)
    {
        return vm.opStack().pushInt(vm.realtime());
    }

    bool op_languagelevel(PSVirtualMachine& vm) {
        // Push the current language level onto the stack
        vm.opStack().pushInt(vm.languageLevel());

        return true;
	}

    inline bool op_gsave(PSVirtualMachine& vm) {
        vm.graphics()->gsave();
        return true;
    }

    inline bool op_grestore(PSVirtualMachine& vm) {
        vm.graphics()->grestore();
        return true;
    }

    bool op_save(PSVirtualMachine& vm) {
        // Ask the VM for the PSSaveState object handle
        // push it onto the stack
        op_gsave(vm); 
        vm.opStack().push(PSObject::fromSave());

        return true;
    }

    bool op_restore(PSVirtualMachine& vm) 
    {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("restore: stack underflow");

        // Pop the saved state from the stack
        // and apply it to the VM
        PSObject saveObj;
        s.pop(saveObj);
        op_grestore(vm);

        return true;
    }

    // ---------------- Language Features --------------------
    inline bool op_currentglobal(PSVirtualMachine& vm)
    {
        return vm.opStack().pushBool(vm.currentGlobal());
    }

    inline bool op_setglobal(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_setglobal: stackunderflow");

        bool value;
        if (!s.popBool(value))
            return vm.error("op_setglobal: typecheck");

        vm.setGlobal(value);
        return true;
    }

    inline bool op_currentpacking(PSVirtualMachine& vm)
    {
        return vm.opStack().pushBool(vm.currentPacking());
    }

    inline bool op_setpacking(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_setpacking: stackunderflow");

        bool value;
        if (!s.popBool(value))
            return vm.error("op_setpacking: typecheck");

        vm.setPacking(value);
        return true;
    }


    bool op_setpagedevice(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_setpagedevice: stackunderflow");

        PSObject dictObj;
        s.pop(dictObj);

        if (!dictObj.isDictionary())
            return vm.error("op_setpagedevice: typecheck");

        auto dict = dictObj.asDictionary();

        PSObject pageSizeObj;
        if (dict->get("PageSize", pageSizeObj))
        {
            if (!pageSizeObj.isArray())
                return vm.error("op_setpagedevice: PageSize must be an array");

            auto pageSize = pageSizeObj.asArray();

            if (!pageSize || pageSize->size() < 2)
                return vm.error("op_setpagedevice: invalid PageSize");

            PSObject widthObj;
            PSObject heightObj;

            if (!pageSize->get(0, widthObj) || !pageSize->get(1, heightObj))
                return vm.error("op_setpagedevice: invalid PageSize");

            if (!widthObj.isNumber() || !heightObj.isNumber())
                return vm.error("op_setpagedevice: PageSize entries must be numeric");

            vm.graphics()->setPageSize(widthObj.asReal(), heightObj.asReal());
        }

        return true;
    }

    bool op_currentpagedevice(PSVirtualMachine& vm) 
    {
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
        auto* grph = vm.graphics();

        grph->initGraphics();

        //reset();              // Clear state stack
        ctm.reset();           // Identity CTM
        grph->currentPath().reset();            // Clear current path
        
        //setRGB(0, 0, 0);      // Default black color
        grph->currentState()->strokePaint = PSPaint::fromRGB(0, 0, 0);
        grph->currentState()->fillPaint = PSPaint::fromRGB(0, 0, 0);

        //setLineWidth(1);      // Default line width
        //initClipPath();      // Default clip path

        return true;
    }

    bool op_showpage(PSVirtualMachine& vm)
    {
        auto* grph = vm.graphics();

        PSObject savedFont = grph->fontObject();

        grph->showPage();
        grph->initGraphics();

        if (savedFont.isDictionary())
            grph->setFont(savedFont.asDictionary());

        auto& ctm = grph->getCTM();
        ctm.reset();

        grph->currentPath().reset();

        grph->currentState()->strokePaint = PSPaint::fromRGB(0, 0, 0);
        grph->currentState()->fillPaint = PSPaint::fromRGB(0, 0, 0);

        return true;
    }

    bool op_erasepage(PSVirtualMachine& vm) 
    {
        auto* grph = vm.graphics();
        grph->erasePage();
        grph->currentPath().reset(); // Clear the current path after erasing the page

        return true;
    }

    bool op_setjobtimeout(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("setjobtimeout: stackunderflow");

        PSObject timeObj;
        s.pop(timeObj);

        if (!timeObj.isInt())
            return vm.error("setjobtimeout: typecheck");

        return true;
    }


    inline const PSOperatorFuncMap& getEnviroOps() {
        static const PSOperatorFuncMap table = {
            { "currentglobal",      op_currentglobal },        // Push the current global allocation state onto the stack
            { "setglobal",          op_setglobal },            // Set the global allocation state
            { "currentpacking",      op_currentpacking },        // Push the current packing state onto the stack
            { "setpacking",          op_setpacking },            // Set the packing state

            { "save",            op_save },                 // Save the current state of the VM
            { "restore",         op_restore },              // Restore the saved state of the VM
            { "usertime",           op_usertime }, 
            { "realtime",           op_realtime },
			{ "languagelevel",      op_languagelevel },

            // Device related
            { "setpagedevice",      op_setpagedevice },
            { "currentpagedevice",  op_currentpagedevice },

            // Graphics state management
            { "gsave",         op_gsave },
            { "grestore",      op_grestore },

            { "initclip",           op_initclip},
            { "initgraphics",       op_initgraphics },
            { "showpage",           op_showpage },
            { "erasepage",          op_erasepage },

            // Job control compatibility
            { "setjobtimeout",      op_setjobtimeout },
        };
        return table;
    }

}
