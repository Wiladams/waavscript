#pragma once

#include "pscore.h"
#include "psvm.h"
#include "ps_print.h"

namespace waavs {



    // --- Operator Implementations ---
    
    inline bool op_def(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_def: stackunderflow");

        PSObject value;
        PSObject key;

        if (!s.pop(value) || !s.pop(key))
            return vm.error("op_def: stackunderflow");

        auto dict = vm.currentDictionary();
        if (!dict)
            return vm.error("op_def: no current dictionary");

        if (!dict->put(key, value))
            return vm.error("op_def: invalide key or dictionary insertion failed");

        return true;
    }

    /*
    // DIAGNOSTIC
    static bool op_undef(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_undef: stackunderflow");

        PSObject key;
        PSObject dictObj;

        if (!s.pop(key) || !s.pop(dictObj))
            return vm.error("op_undef: stackunderflow");

        if (!dictObj.isDictionary())
            return vm.error("op_undef: typecheck");

        auto dict = dictObj.asDictionary();
        if (!dict)
            return vm.error("op_undef: invalidaccess");

        bool exists = dict->contains(key);
        bool removed = dict->remove(key);

        if (!removed)
        {
            std::printf("\nUNDEF FAILED:\n");
            std::printf("  dictionary=%p\n", static_cast<void*>(dict.get()));
            std::printf("  key: ");
            writeObjectDeep(key);
            std::printf("\n");
            std::printf("  contains(key)=%s\n", exists ? "true" : "false");

            return vm.error("op_undef: undefined");
        }

        return true;
    }
    */

    
    static bool op_undef(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_undef: stackunderflow");

        PSObject key;
        PSObject dictObj;

        if (!s.pop(key) || !s.pop(dictObj))
            return vm.error("op_undef: stackunderflow");

        if (!dictObj.isDictionary())
            return vm.error("op_undef: typecheck");

        auto dict = dictObj.asDictionary();
        if (!dict)
            return vm.error("op_undef: invalidaccess");

        // Attempt to remove the key, 
        // but do not treat failure as an error
        dict->remove(key);  

        //if (!dict->remove(key))
        //    return vm.error("op_undef: undefined");

        return true;
    }
    

    static bool op_dict(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        
        if (s.empty())
            return vm.error("op_dict: stackunderflow");

        int32_t sz{ 0 };
        if (!s.popInt(sz))
            return vm.error("op_dict: typecheck; expected int");

        auto d = PSDictionary::create(sz);
        s.pushDictionary(d);

        return true;
    }



    static bool op_maxlength(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        if (s.empty()) 
            return vm.error("op_maxlength: stackunderflow");

        PSDictionaryHandle dictHandle;
        if (!s.popDictionary(dictHandle)) 
            return vm.error("op_maxlength: typecheck; expected dictionary handle");

        // PostScript allows arbitrary max size, but we return a placeholder.
        s.pushInt(999);
        return true;
    }

    static bool op_known(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_known: stackunderflow");

        PSObject key;
        PSObject obj;

        if (!s.pop(key) || !s.pop(obj))
            return vm.error("op_known: stackunderflow");

        PSDictionaryHandle dict;

        if (obj.isDictionary())
        {
            dict = obj.asDictionary();
        }
        //else if (obj.isFont())
        //{
        //    auto font = obj.asFont();
        //    if (font)
        //        dict = font->dictionary;
        //}

        if (!dict)
            return vm.error("op_known: typecheck");

        return s.pushBool(dict->contains(key));
    }


    // From '<<' to '>>', these operators are used for dictionary literals.
    static bool op_dictbegin(PSVirtualMachine& vm) 
    {
        auto& s = vm.opStack();
        return s.pushMark(PSMark("dictbegin"));
    }

    static bool op_dictend(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        int count = 0;
        s.countToMark(count);

        if (count < 0)
            return vm.error("op_dictend: unmatched >> with no mark");

        if ((count % 2) != 0)
            return vm.error("op_dictend: odd number of items in dictionary literal");

        auto dict = PSDictionary::create();

        for (int i = 0; i < count / 2; ++i)
        {
            PSObject value;
            PSObject key;

            s.pop(value);
            s.pop(key);

            if (!dict->put(key, value))
                return vm.error("op_dictend: invalid dictionary key");
        }

        PSObject mark;
        s.pop(mark);

        return s.pushDictionary(dict);
    }

    // --- Operator Map ---

    inline const PSOperatorFuncMap& getDictionaryOps() {
        static const PSOperatorFuncMap table = {
            { "def",               op_def },
            { "undef",             op_undef },
            { "dict",              op_dict },
            { "maxlength",         op_maxlength },
            { "known",             op_known },
            { "<<",                op_dictbegin },
            { ">>",                op_dictend },
        };
        return table;
    }


} // namespace waavs
