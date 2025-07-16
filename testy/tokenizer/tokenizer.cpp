#include <unordered_map>

#include "ps_lexer.h"
#include "mappedfile.h"
#include "ocspan.h"
#include "ps_scanner.h"


using namespace waavs;



std::unordered_map<PSLexType, const char*> lexTypeNames = {
	{PSLexType::Invalid, "Invalid"},
	{PSLexType::Whitespace, "Whitespace"},
	{PSLexType::Name, "Name"},
	{PSLexType::LiteralName, "LiteralName"},
	{PSLexType::SystemName, "SystemName"},
	{PSLexType::Number, "Number"},
	{PSLexType::String, "String"},
	{PSLexType::UnterminatedString, "UnterminatedString"},
	{PSLexType::HexString, "HexString"},
	{PSLexType::LBRACE, "LBRACE"},
	{PSLexType::RBRACE, "RBRACE"},
	{PSLexType::LBRACKET, "LBRACKET"},
	{PSLexType::RBRACKET, "RBRACKET"},
	{PSLexType::LLANGLE, "LLANGLE"},
	{PSLexType::RRANGLE, "RRANGLE"},
	{PSLexType::Comment, "Comment"},
	{PSLexType::DSCComment, "DSCComment"},
	{PSLexType::Delimiter, "Delimiter"},
    {PSLexType::EexecSwitch, "EexecSwitch"},
	{PSLexType::Eof, "Eof"}
};

void printLexeme(const PSLexeme& lex)
{
    const char* typeName = "UNKNOWN";
    auto it = lexTypeNames.find(lex.type);
	if (it != lexTypeNames.end()) {
		typeName = it->second;
    }
	printf("Lexeme Type: %2d  %16s  VALUE: %.*s\n", (int)lex.type, typeName, (unsigned int)lex.span.size(), lex.span.data());
}

static void test_lexgen(OctetCursor s)
{
	PSLexemeGenerator gen(s);
	PSLexeme lexeme;
	while(gen.next(lexeme)) 
	{
		switch (lexeme.type)
		{
			case PSLexType::EexecSwitch: {
				printf("EexecSwitch encountered, decrypting...\n");
				std::vector<uint8_t> hexstring;
                std::vector<uint8_t> decrypted;
				if (spanToHexString(lexeme.span, hexstring)) {
					eexecDecrypt(hexstring, decrypted);
					// print out the decrypted data
                    printf("Decrypted Eexec data: \n");
					for (size_t i = 0; i < decrypted.size(); ++i) {
						printf("%c", decrypted[i]);
						//if ((i + 1) % 80 == 0) {
						//	printf("\n");
						//}
					}
				} else {
					printf("Failed to decode hex string in EexecSwitch.\n");
				}
				printf("\n");
				break;
            }

			default:
				printLexeme(lexeme);
			break;
		}
	}
}

static void test_lexgenfile(const char *filename)
{
	auto mapped = MappedFile::create_shared(filename);
	if (nullptr == mapped)
		return;
	
	OctetCursor s(mapped->data(), mapped->size());
	test_lexgen(s);
	mapped->close();
}

int main(int argc, char *argv[]) {
	if (argc >= 2)
	{
		test_lexgenfile(argv[1]);
		return 0;
	}

	test_lexgen("10 { square s s translate	-5.0 rotate	/s s phi div def } repeat");

	return 0;
}
