
#include <memory>
#include <cstdio>

#include "psvmfactory.h"
#include "b2dcontext.h"



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
        //try {
            OctetCursor oc(line.data(), line.size());
            vm->interpret(oc);
        //} catch (const std::exception& e) {
        //    std::fprintf(stderr, "Error: %s\n", e.what());
        //}
    }

}


int main() {
    vm = PSVMFactory::createVM();
    auto ctx = std::make_unique<waavs::Blend2DGraphicsContext>(800, 800);
    ctx->initGraphics();
    vm->setGraphicsContext(std::move(ctx));

    interactiveLoop();

    return 0;
}
