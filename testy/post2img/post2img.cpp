
#include <filesystem>
#include <memory>
#include <cstdio>
#include <string>

#include "mappedfile.h"
#include "psvmfactory.h"
#include "b2dcontext.h"




using namespace waavs;

#define CAN_WIDTH 800
#define CAN_HEIGHT 600


size_t loadFontsInDirectory(PSVirtualMachine* vm, const char* dirpath) {
	namespace fs = std::filesystem;
	size_t count = 0;
	for (const auto& entry : fs::directory_iterator(dirpath)) {
		if (!entry.is_regular_file()) continue;

		auto ext = entry.path().extension().string();
		if (ext == ".ttf" || ext == ".otf" || ext == ".ttc") {
			PSObject pathObj = PSObject::fromString(entry.path().string().c_str());
			vm->opStack().push(pathObj);
			if (FontMonger::instance()->loadFontResource(*vm))
				++count;
		}
	}
	return count;
}

// Utility to wrap input and run interpreter
static void runPostscript(OctetCursor input, const char *outfilename) 
{
	auto vm = PSVMFactory::createVM();

	if (!vm) {
		printf("Failed to create virtual machine\n");
		return;
	}

	auto ctx = std::make_unique<waavs::Blend2DGraphicsContext>(1700, 2200);	// US Letter size in points (8.5 x 11 inches, 200dpi)
	ctx->initGraphics();
	vm->setGraphicsContext(std::move(ctx));
	loadFontsInDirectory(vm.get(), "c:/windows/fonts");

	// Run the interpreter
	//PSInterpreter interp(*vm);
	vm->interpret(input);

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
