
#include <memory>
#include <cstdio>
#include <filesystem>

#include "psvmfactory.h"
#include "b2dcontext.h"



using namespace waavs;

std::unique_ptr<PSVirtualMachine> vm;

// Scan all the fonts in the specified directory, making them
// known to the FontMonger.
//
size_t loadFontsInDirectory(PSVirtualMachine *vm, const char* dirpath) {
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

static void interactiveLoop()
{
    // read a line from stdin and execute it as a script
    std::string line;
    while (true) {
        std::printf("WS> ");
        std::getline(std::cin, line);
        if (line.empty()) {
            continue; // skip empty lines
        }
        if (line == "exit" || line == "quit") {
            break; // exit the loop
        }

        OctetCursor oc(line.data(), line.size());
        vm->interpret(oc);
    }

}


int main() {
    vm = PSVMFactory::createVM();
    auto ctx = std::make_unique<waavs::Blend2DGraphicsContext>(800, 800);
    ctx->initGraphics();
    vm->setGraphicsContext(std::move(ctx));

    loadFontsInDirectory(vm.get(), "c:/windows/fonts");

    interactiveLoop();

    return 0;
}
