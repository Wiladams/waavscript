#pragma once

#include "psvm.h"
#include "ps_type_file.h"
#include "ps_file_filter.h"

namespace waavs {

    //
    // PostScript File Operators (Structured Stubs)
    //

    //
    // File Opening and Closing
    //
    inline bool op_file(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject accessObj, filenameObj;
        s.pop(accessObj);
        s.pop(filenameObj);

        if (!filenameObj.isString() || !accessObj.isString())
            return vm.error("typecheck: expected (filename) (access)");

        const PSString& filename = filenameObj.asString();
        const PSString& access = accessObj.asString();

        auto pf = PSDiskFile::create(filename, access);
        if (!pf || !pf->isValid())
            return vm.error("file: could not open");

        s.push(PSObject::fromFile(pf));
        return true;
    }

    inline bool op_closefile(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1) return vm.error("stackunderflow");

        PSObject file;
        s.pop(file);

        if (!file.isFile())
            return vm.error("typecheck: expected file");

        auto fileHandle = file.asFile();

        //file->close();

        return vm.error("closefile operator not yet implemented");
    }

    inline bool op_deletefile(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1) return vm.error("stackunderflow");

        PSObject filename;
        s.pop(filename);

        if (!filename.isString())
            return vm.error("typecheck: expected filename string");

        return vm.error("deletefile operator not yet implemented");
    }

    inline bool op_renamefile(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return vm.error("stackunderflow");

        PSObject newname, oldname;
        s.pop(newname);
        s.pop(oldname);

        if (!oldname.isString() || !newname.isString())
            return vm.error("typecheck: expected (oldname) (newname)");

        return vm.error("renamefile operator not yet implemented");
    }

    inline bool op_status(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1) return vm.error("stackunderflow");

        PSObject filename;
        s.pop(filename);

        if (!filename.isString())
            return vm.error("typecheck: expected filename string");

        return vm.error("status operator not yet implemented");
    }

    //
    // File Reading
    //
    inline bool op_read(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1)
            return vm.error("stackunderflow");

        PSObject fileObj;
        s.pop(fileObj);

        if (!fileObj.isFile())
            return vm.error("typecheck: expected file");

        auto file = fileObj.asFile();
        if (!file || !file->isValid())
            return vm.error("invalidfileaccess: file not valid");

        uint8_t byte = 0;
        if (file->readByte(byte)) {
            s.push(PSObject::fromInt(static_cast<int32_t>(byte)));
            s.push(PSObject::fromBool(true));
        }
        else {
            s.push(PSObject::fromBool(false));
        }

        return true;
    }


    inline bool op_readstring(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject strObj, fileObj;
        s.pop(strObj);
        s.pop(fileObj);

        if (!fileObj.isFile() || !strObj.isString())
            return vm.error("typecheck: expected file and string");

        auto file = fileObj.asFile();
        if (!file || !file->isValid())
            return vm.error("invalidfileaccess: file not valid");

        PSString str = strObj.asString();
        size_t count = str.capacity();
        size_t actual = 0;

        for (; actual < count; ++actual) {
            uint8_t b;
            if (!file->readByte(b))
                break;

            if (!str.put(static_cast<uint32_t>(actual), b))
                break;
        }

        str.setLength(static_cast<uint32_t>(actual));
        s.push(PSObject::fromString(str));
        s.push(PSObject::fromBool(actual == count));

        return true;
    }



    inline bool op_readhexstring(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject strObj, fileObj;
        s.pop(strObj);
        s.pop(fileObj);

        if (!fileObj.isFile() || !strObj.isString())
            return vm.error("typecheck: expected file and string");

        auto file = fileObj.asFile();
        if (!file || !file->isValid())
            return vm.error("invalidfileaccess: file not valid");

        PSString str = strObj.asString();
        size_t count = str.capacity();
        size_t written = 0;

        auto isHexDigit = [](uint8_t ch) -> bool {
            return ('0' <= ch && ch <= '9') ||
                ('A' <= ch && ch <= 'F') ||
                ('a' <= ch && ch <= 'f');
            };

        auto hexValue = [](uint8_t ch) -> uint8_t {
            if ('0' <= ch && ch <= '9') return ch - '0';
            if ('A' <= ch && ch <= 'F') return ch - 'A' + 10;
            if ('a' <= ch && ch <= 'f') return ch - 'a' + 10;
            return 0xFF;
            };

        while (written < count) {
            uint8_t c1 = 0, c2 = 0;

            // Skip whitespace and find first hex digit
            do {
                if (!file->readByte(c1)) goto eof;
            } while (c1 <= 0x20);  // skip control and space

            if (!isHexDigit(c1)) break; // non-hex ends read

            // Skip whitespace and find second hex digit
            do {
                if (!file->readByte(c2)) {
                    // Handle trailing odd nibble case (treat missing second digit as '0')
                    c2 = '0';
                    break;
                }
            } while (c2 <= 0x20);

            if (!isHexDigit(c2)) break;

            uint8_t high = hexValue(c1);
            uint8_t low = hexValue(c2);

            if (high == 0xFF || low == 0xFF)
                return vm.error("typecheck: invalid hex digit");

            uint8_t byte = (high << 4) | low;

            if (!str.put(static_cast<uint32_t>(written), byte))
                break;

            ++written;
        }

    eof:
        str.setLength(static_cast<uint32_t>(written));
        s.push(PSObject::fromString(str));
        s.push(PSObject::fromBool(written == count));
        return true;
    }


    inline bool op_readline(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject strObj, fileObj;
        s.pop(strObj);
        s.pop(fileObj);

        if (!fileObj.isFile() || !strObj.isString())
            return vm.error("typecheck: expected file and string");

        auto file = fileObj.asFile();
        if (!file || !file->isValid())
            return vm.error("invalidfileaccess: file not valid");

        PSString str = strObj.asString();
        size_t cap = str.capacity();
        size_t len = 0;
        bool sawChar = false;

        while (len < cap) {
            uint8_t c;
            if (!file->readByte(c)) break;

            sawChar = true;

            if (c == '\n') {
                // Unix newline: done
                break;
            }

            if (c == '\r') {
                // Mac or Windows newline
                // Check if next char is \n and skip it
                uint8_t next = 0;
                if (file->readByte(next)) {
                    if (next != '\n')
                        file->setPosition(file->position() - 1); // unread
                }
                break;
            }

            if (!str.put(static_cast<uint32_t>(len), c))
                break;

            ++len;
        }

        str.setLength(static_cast<uint32_t>(len));
        s.push(PSObject::fromString(str));
        s.push(PSObject::fromBool(sawChar));
        return true;
    }


    inline bool op_bytesavailable(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1)
            return vm.error("stackunderflow");

        PSObject fileObj;
        s.pop(fileObj);

        if (!fileObj.isFile())
            return vm.error("typecheck: expected file");

        auto file = fileObj.asFile();
        if (!file || !file->isValid())
            return vm.error("invalidfileaccess: file not valid");

        size_t available = file->size() - file->position();
        s.push(PSObject::fromInt(static_cast<int32_t>(available)));
        return true;
    }


    //
    // File Writing
    //
    inline bool op_write(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return vm.error("stackunderflow");

        PSObject ch, file;
        s.pop(ch);
        s.pop(file);

        if (!file.isFile() || !ch.isInt())
            return vm.error("typecheck: expected file and integer");

        return vm.error("write operator not yet implemented");
    }

    inline bool op_writestring(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return vm.error("stackunderflow");

        PSObject str, file;
        s.pop(str);
        s.pop(file);

        if (!file.isFile() || !str.isString())
            return vm.error("typecheck: expected file and string");

        return vm.error("op_writestring: operator not yet implemented");
    }

    inline bool op_writehexstring(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return vm.error("stackunderflow");

        PSObject str, file;
        s.pop(str);
        s.pop(file);

        if (!file.isFile() || !str.isString())
            return vm.error("typecheck: expected file and string");

        return vm.error("op_writehexstring: operator not yet implemented");
    }

    inline bool op_flushfile(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1) return vm.error("stackunderflow");

        PSObject fileObj;
        s.pop(fileObj);

        if (!fileObj.isFile())
            return vm.error("typecheck: expected file");

        fileObj.asFile()->flush();
        
        return true;
    }


    inline bool op_flush(PSVirtualMachine& vm) {
        //std::cout.flush();
        return vm.error("flush operator not yet implemented");
    }

    //
    // File Positioning
    //
    inline bool op_fileposition(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1)
            return vm.error("stackunderflow");

        PSObject fileObj;
        s.pop(fileObj);

        if (!fileObj.isFile())
            return vm.error("typecheck: expected file");

        auto file = fileObj.asFile();
        if (!file || !file->isValid())
            return vm.error("invalidfileaccess: file not valid");

        size_t pos = file->position();
        s.push(PSObject::fromInt(static_cast<int32_t>(pos)));
        return true;
    }


    inline bool op_setfileposition(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject posObj, fileObj;
        s.pop(posObj);
        s.pop(fileObj);

        if (!fileObj.isFile() || !posObj.isInt())
            return vm.error("typecheck: expected file and integer");

        auto file = fileObj.asFile();
        if (!file || !file->isValid())
            return vm.error("invalidfileaccess: file not valid");

        int32_t offset = posObj.asInt();
        if (offset < 0 || static_cast<size_t>(offset) > file->size())
            return vm.error("rangecheck: offset out of bounds");

        file->setPosition(static_cast<size_t>(offset));
        return true;
    }


    //
    // Special File Access
    //
    inline bool op_currentfile(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();

        PSFileHandle fileHandle;
        if (!vm.getCurrentFile(fileHandle))
            return vm.error("op_currentfile: valuecheck; no currentfile available");

        return ostk.pushFile(fileHandle);
    }


    // op_filter
    // BUGBUG - only doing the one that takes two parameters
    bool op_filter(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();

        // Support 2 parameter formm
        PSObject sourceObj, nameObj;

        if (!ostk.pop(nameObj) || !ostk.pop(sourceObj))
            return vm.error("op_filter: stackunderflow");


        if (!nameObj.isName())
            return vm.error("op_filter: typecheck: expected filter name");
        if (!sourceObj.isFile())
            return vm.error("op_filter: typecheck: expected file as source");

        PSName filterName = nameObj.asName();
        auto sourceFile = sourceObj.asFile();


        // Lookup filter by name and create a wrapper
        std::shared_ptr<PSFile> fileWrapper;

        if (filterName == "ASCII85Decode")
        {
            fileWrapper = std::make_shared<ASCII85DecodeFilter>(sourceFile);
        } else if (filterName == "RunLengthDecode")
        {
            fileWrapper = std::make_shared<RunLengthDecodeFilter>(sourceFile);
        }
        else
        {
            return vm.error("undefined: unknown filter");
        }


        return ostk.push(PSObject::fromFile(fileWrapper));
    }



/*
    bool op_filter(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();

        // Support both 2- and 3-operand forms
        PSObject nameObj, sourceObj, paramObj;
        bool hasParams = false;

        //if (ostk.size() >= 3 && ostk.peek(2).isDictionary()) {
        //    hasParams = true;
        //    ostk.pop(paramObj);
        //}

        

        if (!ostk.pop(sourceObj) || !ostk.pop(nameObj))
            return vm.error("op_filter: stackunderflow");

        if (!nameObj.isName())
            return vm.error("op_filter: typecheck: expected filter name");
        if (!sourceObj.isFile())
            return vm.error("op_filter: typecheck: expected file as source");

        PSName filterName = nameObj.asName();
        std::shared_ptr<PSFile> sourceFile = sourceObj.asFile();

        // Optional parameters (can be ignored or used)
        std::shared_ptr<PSDictionary> params = nullptr;
        if (hasParams)
            params = paramObj.asDictionary();

        // Lookup filter by name and create a wrapper
        std::shared_ptr<PSFile> filteredFile;
        //filteredFile    = vm.filterRegistry().create(filterName, sourceFile, params);

        if (!filteredFile)
            return vm.error("undefined: unknown filter");

        return ostk.push(PSObject::fromFile(filteredFile));
    }
*/

    inline bool op_resetfile(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1)
            return vm.error("op_resetfile: stackunderflow");

        PSObject fileObj;
        s.pop(fileObj);
        if (!fileObj.isFile())
            return vm.error("op_resetfile: typecheck; expected file");
        auto file = fileObj.asFile();
        
        if (!file || !file->isValid())
            return vm.error("op_resetfile: invalidfileaccess: file not valid");
        
        file->rewind(); // Reset the file position to the beginning
        
        return true;
    }

    inline bool op_run(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1)
            return vm.error("stackunderflow");

        PSObject srcObj;
        s.pop(srcObj);

        PSFileHandle file;

        if (srcObj.isString()) {
            // (filename) run
            const PSString& name = srcObj.asString();
            PSString access = PSString::fromCString("r");
            file = PSDiskFile::create(name, access);
            if (!file || !file->isValid())
                return vm.error("invalidfileaccess: cannot open file");
        }
        else if (srcObj.isFile()) {
            // (file) run
            file = srcObj.asFile();
            if (!file || !file->isValid())
                return vm.error("invalidfileaccess: file not valid");
        }
        else {
            return vm.error("typecheck: expected file or filename");
        }

        // Push currentFile and run the content
        // BUGBUG - maybe just pushing is good enough, and the run
        // loop can take care of popping when it's exhausted?
        vm.pushCurrentFile(file);

        bool ok = vm.interpret(file);

        PSFileHandle lastOne;

        vm.popCurrentFile(lastOne); // restore

        return ok;
    }


    //
    // Operator Registration Table
    //

    inline const PSOperatorFuncMap& getFileOps() {
        static const PSOperatorFuncMap table = {
            // File creation and closing
            { "file",            op_file },
            { "closefile",       op_closefile },
            { "deletefile",      op_deletefile },
            { "renamefile",      op_renamefile },
            { "status",          op_status },

            // File input
            { "read",            op_read },
            { "readstring",      op_readstring },
            { "readhexstring",   op_readhexstring },
            { "readline",        op_readline },
            { "bytesavailable",  op_bytesavailable },

            // File output
            { "write",           op_write },
            { "writestring",     op_writestring },
            { "writehexstring",  op_writehexstring },
            { "flushfile",       op_flushfile },
            { "flush",           op_flush },

            // File positioning
            { "fileposition",    op_fileposition },
            { "setfileposition", op_setfileposition },
            { "resetfile",       op_resetfile },

            // File environment
            { "currentfile",     op_currentfile },
            { "filter",          op_filter },
            { "run",             op_run }
        };
        return table;
    }


} // namespace waavs
