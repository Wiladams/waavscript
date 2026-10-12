
#include <memory>
#include <cstdio>
#include <filesystem>

#include "psvmfactory.h"
#include "b2dcontext.h"
#include "fontmonger.h"


using namespace waavs;

std::unique_ptr<PSVirtualMachine> vm;


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
