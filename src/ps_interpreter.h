#pragma once


#include "psvm.h"
#include "ps_scanner.h"

namespace waavs
{

    static bool spanToHexString(OctetCursor src, std::vector<uint8_t>& out) noexcept
    {
        out.clear();

        while (!src.empty()) {
            skipWhile(src, PS_WHITESPACE);
            if (src.empty()) break;

            uint8_t hiChar = *src.fStart++;
            skipWhile(src, PS_WHITESPACE);

            uint8_t loChar = src.empty() ? '0' : *src.fStart++;

            uint8_t hi, lo;
            if (!decodeHex(hiChar, hi) || !decodeHex(loChar, lo))
                return false;

            out.push_back((hi << 4) | lo);
        }

        return true;
    }



    // Take a single token and try to convert it into a PSObject
    static bool transformTokenToPSObject(const PSToken& tok, PSObject& obj, bool inProc) noexcept
    {
        switch (tok.type) {
        case PSTokenType::PS_TOKEN_Boolean:
            return obj.resetFromBool(tok.boolValue);

        case PSTokenType::PS_TOKEN_Number:
            // Use int if the number is whole
            if (tok.numberValue == static_cast<int32_t>(tok.numberValue))
                return obj.resetFromInt(static_cast<int32_t>(tok.numberValue));
            else
                return obj.resetFromReal(tok.numberValue);

        case PSTokenType::PS_TOKEN_LiteralName:
            obj.resetFromName(tok.span);
            return true;

        case PSTokenType::PS_TOKEN_ExecutableName:
            obj.resetFromName(tok.span);
            obj.setExecutable(true);
            return true;

        case PSTokenType::PS_TOKEN_String: {
            auto str = PSString::createFromSpan(tok.span.size(), tok.span.data());
            return obj.resetFromString(str);
        }

        case PSTokenType::PS_TOKEN_HexString: {
            std::vector<uint8_t> decoded;
            if (!spanToHexString(tok.span, decoded))
                return false;
            auto str = PSString::createFromVector(decoded);
            return obj.resetFromString(str);
        }

        case PSTokenType::PS_TOKEN_Null:
            return obj.reset();


        case PSTokenType::PS_TOKEN_Mark:
            return obj.resetFromMark();

        default:
            return obj.reset();
        }
    }
} // namespace waavs


namespace waavs 
{
	// The PSInterpreter takes a strea of tokens and runs them through the PSVirtualMachine.
    // This is the main entry point when you're running a PS script
    // or running an interactive interface.
    struct PSInterpreter 
    {
    private:
        PSVirtualMachine& fVM;

    public:
        PSInterpreter(PSVirtualMachine& vm)
            : fVM(vm) 
        {
        }
        
        bool parseArray(PSTokenGenerator& tokgen, PSObject& out, bool isProc) {
            auto arr = PSArray::create();
            arr->setIsProcedure(isProc);

            PSObject element;

            while (true) {
                PSToken tok;
                if (!tokgen.next(tok)) break;

                if ((isProc && tok.type == PSTokenType::PS_TOKEN_ProcEnd) ||
                    (!isProc && tok.type == PSTokenType::PS_TOKEN_ArrayEnd)) {
                    break;
                }

                if (tok.type == PSTokenType::PS_TOKEN_ProcBegin) {
                    PSObject subProc;
                    if (!parseArray(tokgen, subProc, true)) return false;
                    // if we parse a sub-procedure, we need to mark it as executable
                    //subProc.setExecutable(true);
                    arr->append(subProc);
                }
                else if (tok.type == PSTokenType::PS_TOKEN_ArrayBegin) {
                    PSObject subArray;
                    if (!parseArray(tokgen, subArray, false)) return false;
                    arr->append(subArray);
                }
                else {
                    if (!transformTokenToPSObject(tok, element, isProc)) return false;
                    arr->append(element);
                }
            }

            out.resetFromArray(arr);
            return true;
        }


        bool parseObject(PSTokenGenerator& tokgen, PSObject& out) {
            PSToken tok;
            if (!tokgen.next(tok)) return false;

            if (tok.type == PSTokenType::PS_TOKEN_ProcBegin) {
                if (!parseArray(tokgen, out, true))
                    return false;
                return true;
            }

            if (tok.type == PSTokenType::PS_TOKEN_ArrayBegin) {
                return parseArray(tokgen, out, false); // not a procedure
            }

            return transformTokenToPSObject(tok, out, false);
        }

        bool interpret(PSTokenGenerator &tokGen) 
        {

            while (true)
            {
                PSObject obj;

                if (!parseObject(tokGen, obj))
					return true;
                    
                if (obj.isExecutable()) {
                    // push to execution stack then run
                    fVM.execStack().push(obj);
                    fVM.run();

                    if (fVM.isExitRequested()) {
                        break;
                    }

                    if (fVM.isStopRequested()) {
                        fVM.clearStopRequest();
                        break;
                    }
                }
                else {
                    fVM.opStack().push(obj);
                }
            }

            return true;
        }
        
        bool interpret(const OctetCursor input) {
            PSTokenGenerator tokGen(input);
        
			return interpret(tokGen);
        }

    };
}