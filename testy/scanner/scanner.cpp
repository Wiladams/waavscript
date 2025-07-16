
#include "mappedfile.h"

#include "ps_scanner.h"
#include "ps_print.h"


using namespace waavs;

/*
bool next(PSObject& obj)
{

    while (!stack.empty()) {
        PSLexemeGenerator& top = stack.back();

        PSLexeme lex;
        if (!top.next(lex)) {
            stack.pop_back(); // finished current stream
            continue;
        }

        switch (lex.type)
        {

        case PSLexType::Eof:
            stack.pop_back(); // done with this generator
            continue;

        case PSLexType::EexecSwitch: {
            // --- decrypt remaining input ---
            std::vector<uint8_t> hexDecoded;

            if (!spanToHexString(lex.span, hexDecoded))
                return false;

            eexecDecrypt(hexDecoded, decrypted);

            // Optional: trim 512 random bytes from beginning
            // if the first 4–8 lines look like garbage

            // Push new generator
            stack.emplace_back(OctetCursor(decrypted.data(), decrypted.size()));
            continue;
        }

        case PSLexType::LBRACE:
            return scanProcedure(top, obj);

        case PSLexType::RBRACE:
            return obj.reset();

        default:
            return objectFromLex(lex, obj);
        }
    }

    return obj.reset(); // All sources exhausted
}
*/

static void test_objectgen(const OctetCursor& s)
{
	PSObjectGenerator gen(s);
	PSObject obj;
	while (gen.next(obj))
	{
        switch (obj.type)
        {

        default:
            writeObjectDeep(obj, std::cout);
            printf("\n");
            break;
        }

	}
}

static void test_objectgenFile(const char* filename)
{
	// create an mmap for the specified file
	auto mapped = MappedFile::create_shared(filename);

	if (nullptr == mapped)
		return ;


	OctetCursor s(mapped->data(), mapped->size());

	test_objectgen(s);

	mapped->close();
}


int main(int argc, char* argv[]) {
	if (argc >= 2)
	{
		test_objectgenFile(argv[1]);
		return 1;
	}

	test_objectgen(OctetCursor("/x 42 def x ="));

	return 0;
}

