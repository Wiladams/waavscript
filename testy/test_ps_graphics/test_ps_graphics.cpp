#include "ps_interpreter.h"
#include "psvmfactory.h"
#include "b2dcontext.h"

#include <memory>
#include <cstdio>

using namespace waavs;

// Utility to wrap input and run interpreter
static void runPostscript(const char* sourceText) {
    OctetCursor input(sourceText);

    printf("+-----------------------------------------+\n");
    printf("PS INPUT\n %s\n", sourceText);
    printf("+-----------------------------------------+\n");

    auto vm = PSVMFactory::createVM();

    if (!vm) {
        printf("Failed to create virtual machine\n");
        return;
    }

    auto ctx = std::make_unique<waavs::Blend2DGraphicsContext>(640, 480);
    vm->setGraphicsContext(std::move(ctx));

    // Run the interpreter
    PSInterpreter interp(*vm);
    interp.interpret(input);

    // If we want, we can save output here
    static_cast<waavs::Blend2DGraphicsContext*>(vm->graphics())->getImage().writeToFile("output.png");

}

// ------------ Test Cases ------------




//============================================================================
// Idiomatic Tests
//============================================================================

static void test_simple()
{
    // This will crash the interpreter if tail call optimization is not implemented
    const char* test_s1 = R"||(
% draw a red triangle
1 0 0 setrgbcolor    % red
4 setlinewidth

newpath
100 100 moveto
200 100 lineto
150 200 lineto
closepath

stroke

)||";

    runPostscript(test_s1);

}

static void test_flower()
{
    const char* test_s1 = R"||(
% Define constants
/pi 3.1415926535 def
/radius 50 def
/petal_angle 90 def
/circle_radius 20 def

% Draw a petal (placeholder logic — needs real curve logic to look good)
% Note: currentpoint and rotate are misused in the original; simplified here
/draw_petal {
  newpath
  100 300 moveto
  150 350 lineto
  100 400 lineto
  50 350 lineto
  closepath
  stroke
} def

% Draw the flower stem
/draw_stem {
  newpath
  100 200 moveto
  100 300 lineto
  stroke
} def

% Draw a leaf
/draw_leaf {
  newpath
  80 250 moveto
  60 230 lineto
  70 200 lineto
  closepath
  0 1 0 setrgbcolor   % Green
  fill
} def

% Draw flower petals around a center point
/draw_flower {
  1 0 0 setrgbcolor   % Red
  6 {
    gsave
      100 300 translate
      60 rotate
      -50 0 translate
      draw_petal
    grestore
  } repeat
} def

% Draw the flower center
/draw_flower_center {
  newpath
  100 300 circle_radius 0 360 arc
  1 1 0 setrgbcolor   % Yellow
  fill
} def

% Draw the complete flower
/draw_flower_full {
  draw_stem
  draw_leaf
  draw_flower
  draw_flower_center
} def

% Start drawing
draw_flower_full

showpage

)||";
	runPostscript(test_s1);
}
// ------------ Entry ------------

static void test_core()
{
    //test_simple();
    test_flower();
}

static void test_idioms()
{

}

int main() {

    test_core();
    //test_idioms();

    return 0;
}

