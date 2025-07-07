#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs
{
    inline bool op_findresource(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& rs = vm.getResourceStack();

        // stack: key category
        PSObject categoryObj;
        PSObject keyObj;

        if (!s.pop(categoryObj))
            return vm.error("findresource: missing category");

        if (!s.pop(keyObj))
            return vm.error("findresource: missing key");

        if (!categoryObj.isName())
            return vm.error("findresource: category must be a name");

        if (!keyObj.isName())
            return vm.error("findresource: key must be a name");

        const PSName& categoryName = categoryObj.asName();
        const PSName& resourceKey = keyObj.asName();

        // find the category dictionary on the resource stack
        PSObject obj;
        PSDictionaryHandle categoryDict;
        if (!rs.load(categoryName, obj))
            return vm.error("findresource: category not found");

        if (!obj.isDictionary())
            return vm.error("findresource: category is not a dictionary");

        categoryDict = obj.asDictionary();
        
        PSObject foundResource;
        if (!categoryDict->get(resourceKey, foundResource))
            return vm.error("findresource: resource not found");

        s.push(foundResource);
        return true;
    }

    // key category value  defineresource
    inline bool op_defineresource(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& rs = vm.getResourceStack();

        // stack: key value category
        PSObject keyObj;
        PSObject categoryObj;
        PSObject valueObj;



        if (!s.pop(valueObj))
            return vm.error("defineresource: missing value");

        if (!s.pop(categoryObj))
            return vm.error("defineresource: missing category");

        if (!s.pop(keyObj))
            return vm.error("defineresource: missing key");

        // Validate types
        if (!categoryObj.isName())
            return vm.error("defineresource: category must be a name");

        if (!keyObj.isName())
            return vm.error("defineresource: key must be a name");

        auto categoryName = categoryObj.asName();
        auto resourceKey = keyObj.asName();

        printf("defineresource: category=%s, key=%s\n", categoryName.c_str(), resourceKey.c_str());

        //
        // get top of the resource stack
        //
        PSObject topResDirObj;
        if (!rs.currentdict()->get(categoryName, topResDirObj)) {
            // category dictionary not yet present, create it
            auto categoryDict = PSDictionary::create();
            rs.currentdict()->put(categoryName, PSObject::fromDictionary(categoryDict));
            topResDirObj = PSObject::fromDictionary(categoryDict);
        }

        if (!topResDirObj.isDictionary())
            return vm.error("defineresource: category entry is not a dictionary");

        auto categoryDict = topResDirObj.asDictionary();

        categoryDict->put(resourceKey, valueObj);

        // PostScript defineresource leaves the value on the stack
        s.push(valueObj);

        return true;
    }

    inline bool op_undefineresource(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& rs = vm.getResourceStack();

        // stack: key category
        PSObject categoryObj;
        PSObject keyObj;

        if (!s.pop(categoryObj))
            return vm.error("undefineresource: missing category");

        if (!s.pop(keyObj))
            return vm.error("undefineresource: missing key");

        if (!categoryObj.isName())
            return vm.error("undefineresource: category must be a name");

        if (!keyObj.isName())
            return vm.error("undefineresource: key must be a name");

        const PSName& categoryName = categoryObj.asName();
        const PSName& resourceKey = keyObj.asName();

        //
        // get top of the resource stack (current resource directory)
        //
        PSObject topResDirObj;
        if (!rs.currentdict()->get(categoryName, topResDirObj))
            return vm.error("undefineresource: category not found");

        if (!topResDirObj.isDictionary())
            return vm.error("undefineresource: category is not a dictionary");

        auto categoryDict = topResDirObj.asDictionary();

        if (!categoryDict->remove(resourceKey))
            return vm.error("undefineresource: resource key not found");

        return true;
    }

    inline bool op_resourcestatus(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& rs = vm.getResourceStack();

        // stack: key category
        PSObject categoryObj;
        PSObject keyObj;

        if (!s.pop(categoryObj))
            return vm.error("resourcestatus: missing category");

        if (!s.pop(keyObj))
            return vm.error("resourcestatus: missing key");

        if (!categoryObj.isName())
            return vm.error("resourcestatus: category must be a name");

        if (!keyObj.isName())
            return vm.error("resourcestatus: key must be a name");

        const PSName& categoryName = categoryObj.asName();
        const PSName& resourceKey = keyObj.asName();

        bool found = false;

        rs.forEachFromTop([&](const PSDictionaryHandle& resourceDir) -> bool
            {
                PSObject catDictObj;
                if (!resourceDir->get(categoryName, catDictObj))
                    return true; // keep going

                if (!catDictObj.isDictionary())
                    return true; // keep going

                auto catDict = catDictObj.asDictionary();

                PSObject resource;
                if (catDict->get(resourceKey, resource))
                {
                    // found
                    s.push(resource);              // bottom
                    s.push(categoryObj);           // middle
                    s.push(PSObject::fromBool(true)); // top
                    found = true;
                    return false; // stop
                }
                return true; // continue searching
            });

        if (!found) {
            s.push(PSObject::fromBool(false));
        }

        return true;
    }

    inline bool op_resourceforall(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& estk = vm.execStack();
        auto& rs = vm.getResourceStack();

        // stack: category proc
        PSObject procObj;
        PSObject categoryObj;

        if (!s.pop(procObj))
            return vm.error("resourceforall: missing procedure");

        if (!s.pop(categoryObj))
            return vm.error("resourceforall: missing category");

        if (!categoryObj.isName())
            return vm.error("resourceforall: category must be a name");

        if (!procObj.isExecutable())
            return vm.error("resourceforall: proc must be executable");

        const PSName& categoryName = categoryObj.asName();

        rs.forEachFromTop([&](const PSDictionaryHandle& resourceDir) -> bool
            {
                PSObject catDictObj;
                if (!resourceDir->get(categoryName, catDictObj))
                    return true; // continue

                if (!catDictObj.isDictionary())
                    return true; // continue

                auto catDict = catDictObj.asDictionary();

                // forEach of the category dictionary
                catDict->forEach([&](PSName key, PSObject& value) -> bool
                    {
                        // push key, value, proc, exec
                        s.push(PSObject::fromName(key));
                        s.push(value);
                        estk.push(procObj);
                        if (!vm.run())
                        {
                            // if exec fails, break everything
                            return false;
                        }
                        return true;
                    });

                return true; // keep scanning resource stack
            });

        return true;
    }

    inline bool op_beginresource(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& rs = vm.getResourceStack();

        // stack: key category
        PSObject categoryObj;
        PSObject keyObj;

        if (!s.pop(categoryObj))
            return vm.error("beginresource: missing category");

        if (!s.pop(keyObj))
            return vm.error("beginresource: missing key");

        if (!categoryObj.isName())
            return vm.error("beginresource: category must be a name");

        if (!keyObj.isName())
            return vm.error("beginresource: key must be a name");

        const PSName& categoryName = categoryObj.asName();
        const PSName& resourceKey = keyObj.asName();

        //
        // get top of the resource stack
        //
        PSObject topResDirObj;
        if (!rs.currentdict()->get(categoryName, topResDirObj))
        {
            // category dictionary missing, create it
            auto categoryDict = PSDictionary::create();
            rs.currentdict()->put(categoryName, PSObject::fromDictionary(categoryDict));
            topResDirObj = PSObject::fromDictionary(categoryDict);
        }

        if (!topResDirObj.isDictionary())
            return vm.error("beginresource: category is not a dictionary");

        auto categoryDict = topResDirObj.asDictionary();

        //
        // put a placeholder to reserve the key
        //
        categoryDict->put(resourceKey, PSObject::fromMark(PSMark()));  // mark is a placeholder

        // mark that we are inside a beginresource (could be tracked if needed)
        // you might push a marker on a dedicated resource context stack if you wish
        // for now we just leave the standard mark on the operand stack
        s.push(PSObject::fromMark(PSMark()));

        return true;
    }

    inline bool op_endresource(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (!s.clearToMark())
            return vm.error("endresource: no matching beginresource mark");

        return true;
    }

    inline bool op_resourcestack(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& rs = vm.getResourceStack();

        // get a copy of the current stack as an array
        auto stackArray = rs.getStack();

        s.push(PSObject::fromArray(stackArray));

        return true;
    }

    inline bool op_setresourcestack(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& rs = vm.getResourceStack();

        PSObject arrayObj;
        if (!s.pop(arrayObj))
            return vm.error("setresourcestack: missing array");

        if (!arrayObj.isArray())
            return vm.error("setresourcestack: expected array");

        auto arr = arrayObj.asArray();

        // validate each element is a dictionary
        std::vector<PSDictionaryHandle> newStack;

        for (size_t i = 0; i < arr->size(); ++i)
        {
            PSObject item;
            if (!arr->get(i, item))
                return vm.error("setresourcestack: invalid array element");

            if (!item.isDictionary())
                return vm.error("setresourcestack: all elements must be dictionaries");

            newStack.push_back(item.asDictionary());
        }

        // replace the current resource stack
        rs.setStack(newStack);

        return true;
    }

    inline bool op_ResourceDirectory(PSVirtualMachine& vm)
    {
        // push the system resource directory handle on the operand stack
        auto rsh = vm.getSystemResourceDirectory();

        return vm.opStack().push(PSObject::fromDictionary(rsh));
    }




    inline const PSOperatorFuncMap& getResourceOperators()
    {
        static const PSOperatorFuncMap table = {
            { "findresource",       op_findresource       },
            { "defineresource",     op_defineresource     },
            { "undefineresource",   op_undefineresource   },
            { "resourcestatus",     op_resourcestatus     },
            { "resourceforall",     op_resourceforall     },
            { "beginresource",      op_beginresource      },
            { "endresource",        op_endresource        },
            { "resourcestack",      op_resourcestack      },
            { "setresourcestack",   op_setresourcestack   },
            { "ResourceDirectory",  op_ResourceDirectory  },
        };
        return table;
    }

}

