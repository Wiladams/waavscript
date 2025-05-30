
#include "mappedfile.h"

#include "ps_scanner.h"


using namespace waavs;


/*
		PS_TOKEN_Invalid = 0,
		PS_TOKEN_EOI,                // End of input
		PS_TOKEN_Boolean,            // true / false
		PS_TOKEN_Number,             // Any numeric value
		PS_TOKEN_LiteralName,        // /foo
		PS_TOKEN_ExecutableName,     // foo
		PS_TOKEN_String,             // (abc)
		PS_TOKEN_HexString,          // <48656C6C6F>
		PS_TOKEN_ProcBegin,          // {
		PS_TOKEN_ProcEnd,            // }
		PS_TOKEN_ArrayBegin,         // [
		PS_TOKEN_ArrayEnd,           // ]
		PS_TOKEN_Operator,           // Bound operator (optional at scan time)
		PS_TOKEN_Mark,               // e.g. from 'mark' operator
		PS_TOKEN_Null                // 'null'
*/
void printToken(const PSToken& tok)
{
	printf("Token Type: %d   Value: %.*s\n", (int)tok.type, (unsigned int)tok.span.size(), tok.span.data());
	
	switch (tok.type)
	{
	case PSTokenType::PS_TOKEN_LiteralName:
		printf("  LiteralName: %.*s\n", (unsigned int)tok.span.size(), tok.span.data());
		break;
	case PSTokenType::PS_TOKEN_String:
		printf("  String: %.*s\n", (unsigned int)tok.span.size(), tok.span.data());
		break;
	case PSTokenType::PS_TOKEN_Number:
		printf("  Number: %f\n", tok.numberValue);
		break;
	case PSTokenType::PS_TOKEN_ExecutableName:
		printf("  ExecName: %.*s\n", (unsigned int)tok.span.size(), tok.span.data());
		break;
	case PSTokenType::PS_TOKEN_HexString:
		printf("  HexString: %.*s\n", (unsigned int)tok.span.size(), tok.span.data());
		break;
	case PSTokenType::PS_TOKEN_Boolean:
		printf("  Boolean: %d\n", (unsigned int)tok.boolValue);
		break;
	}
}



static void test_tokengen(const OctetCursor& s)
{
	PSTokenGenerator gen(s);
	PSToken tok;
	while (gen.next(tok))
	{
		printToken(tok);
	}
}

static void test_tokengenFile(const char* filename)
{
	// create an mmap for the specified file
	auto mapped = MappedFile::create_shared(filename);

	if (nullptr == mapped)
		return ;


	OctetCursor s(mapped->data(), mapped->size());

	test_tokengen(s);

	mapped->close();
}


int main(int argc, char* argv[]) {
	if (argc >= 2)
	{
		test_tokengenFile(argv[1]);
		return 1;
	}

	test_tokengen(OctetCursor("/x 42 def x ="));

	return 0;
}

