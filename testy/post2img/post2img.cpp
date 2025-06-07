
#include <filesystem>

#include "mappedfile.h"

#include "ps_interpreter.h"
#include "psvmfactory.h"
#include "b2dcontext.h"

#include <memory>
#include <cstdio>
#include <string>


using namespace waavs;

#define CAN_WIDTH 800
#define CAN_HEIGHT 600


/*

static void loadFontDirectory(const char* dir)
{
	const std::filesystem::path fontPath(dir);

	for (const auto& dir_entry : std::filesystem::directory_iterator(fontPath))
	{
		if (dir_entry.is_regular_file())
		{
			if ((dir_entry.path().extension() == ".ttf") ||
				(dir_entry.path().extension() == ".otf") ||
				(dir_entry.path().extension() == ".TTF"))
			{
				BLFontFace ff{};
				if (!FontHandler::getFontHandler()->loadFontFace(dir_entry.path().generic_string().c_str(), ff))
				{
					printf("FontHandler::loadFontFace() failed: %s\n", dir_entry.path().generic_string().c_str());
					return;
				}
			}
		}
	}
}
*/

static void setupFonts()
{
	//loadFontDirectory("c:\\Windows\\Fonts");
}


// Utility to wrap input and run interpreter
static void runPostscript(OctetCursor input, const char *outfilename) 
{
	auto vm = PSVMFactory::createVM();

	if (!vm) {
		printf("Failed to create virtual machine\n");
		return;
	}

	auto ctx = std::make_unique<waavs::Blend2DGraphicsContext>(612, 792);	// US Letter size in points (8.5 x 11 inches)
	vm->setGraphicsContext(std::move(ctx));

	// Run the interpreter
	PSInterpreter interp(*vm);
	interp.interpret(input);

	// If we want, we can save output here
	static_cast<waavs::Blend2DGraphicsContext*>(vm->graphics())->getImage().writeToFile(outfilename);

}

// Utility to replace .ps with .png or append .png if no .ps is found
std::string defaultOutputFilename(const std::string& inputFilename) {
	std::string output = inputFilename;
	size_t pos = output.rfind(".ps");
	if (pos != std::string::npos && pos == output.length() - 3) {
		// Found ".ps" at the end
		output.replace(pos, 3, ".png");
	}
	else {
		// No .ps suffix; just append .png
		output += ".png";
	}
	return output;
}

int main(int argc, char** argv)
{

	if (argc < 2)
	{
		printf("Usage: post2img <postscript file>  [output file]\n");
		return 1;
	}

	setupFonts();

	// create an mmap for the specified file
	const char* filename = argv[1];

	auto mapped = MappedFile::create_shared(filename);

	// if the mapped file does not exist, return
	if (mapped == nullptr)
	{
		printf("File not found: %s\n", filename);
		return 1;
	}

	OctetCursor mappedSpan(mapped->data(), mapped->size());


	auto outfilename = defaultOutputFilename(filename);

	runPostscript(mappedSpan, outfilename.c_str());

	return 0;
}
