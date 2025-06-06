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

    auto ctx = std::make_unique<waavs::Blend2DGraphicsContext>(800,800);
    vm->setGraphicsContext(std::move(ctx));

    // Run the interpreter
    PSInterpreter interp(*vm);
    interp.interpret(input);

    // If we want, we can save output here
    static_cast<waavs::Blend2DGraphicsContext*>(vm->graphics())->getImage().writeToFile("output.png");

}

// ------------ Test Cases ------------
static void test_op_curveto()
{
    // This will crash the interpreter if tail call optimization is not implemented
    const char* test_s1 = R"||(
4 setlinewidth
newpath
100 100 moveto
120 140 160 140 180 100 curveto
stroke
)||";

    runPostscript(test_s1);

}

static void test_op_arc()
{
    // This will crash the interpreter if tail call optimization is not implemented
	const char* test_s1 = R"||(
4 setlinewidth
1 0 0 setrgbcolor

newpath
200 200 50 0 360 arc
stroke
)||";
    runPostscript(test_s1);
}


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

static void radialLines()
{
	const char* test_s1 = R"||(
% Create a radial burst pattern
% draw some axis lines
 
gsave
4 setlinewidth

newpath
0 0 1 setrgbcolor
0 0 moveto
0 600 lineto
stroke

newpath
1 0 0 setrgbcolor
0 0 moveto
600 0 lineto
stroke
grestore

% Now do the radial lines
gsave
300 400 translate

0 1 60 {
  /i exch def
  gsave
    i 6 mul rotate
    newpath
    0 0 moveto
    100 0 lineto
    stroke
  grestore
} for
grestore
)||";
	runPostscript(test_s1);
}

static void scaledRectangles()
{
	const char* test_s1 = R"||(
% Draw a stack of scaled rectangles

newpath
100 100 translate          % Move origin away from edge

0 1 5 {
  /i exch def
  gsave
    i 0.5 add dup scale     % Scale up each time
    i 60 mul 0 translate    % Offset each rectangle in X to prevent overlap
    0 0 50 30 rectfill
  grestore
} for
)||";
    runPostscript(test_s1);
}


static void gridOfCircles()
{
	const char* test_s1 = R"||(
% Draw a 10x10 grid of stroked circles

0 1 9 {
  /y exch def
  0 1 9 {
    /x exch def
    gsave
      x 50 mul 50 add      % map x = 0..9 to 50,100,...,500
      y 50 mul 50 add      % map y = 0..9 to 50,100,...,500
      translate            % move origin to circle center

      newpath
      0 0 20 0 360 arc     % circle centered at origin
      closepath
      stroke               % stroke with current CTM
    grestore
  } for
} for

)||";
	runPostscript(test_s1);
}

static void test_flower()
{
    const char* test_s1 = R"||(
% Draw a flower-like burst using radial strokes

gsave
300 400 translate          % Move origin to center of canvas

0 1 59 {
  /i exch def
  gsave
    i 6 mul rotate         % Rotate CTM by i * 6 degrees
    newpath
    0 0 moveto
    100 0 lineto           % Line pointing right, rotated by CTM
    stroke                 % Stroke with current transform
  grestore
} for

grestore
)||";

	runPostscript(test_s1);
}

static void grid()
{
    const char* test_s1 = R"||(
100 100 translate
/grid {0.5 0.3 0 0 setcmykcolor 
gsave
% Draw X-Y axis lines
2 setlinewidth 400 0 moveto 0 0 lineto 0 500 lineto stroke
grestore

% draw horizontal lines
gsave
0.3 setlinewidth
5 { 
    30 100 moveto 
    400 100 lineto 
    stroke 
    0 100 translate 
} repeat
grestore

% draw vertical lines
gsave
0.3 setlinewidth
4 { 100 20 moveto 100 500 lineto stroke 100 0 translate } repeat
grestore
} def

grid
)||";

    runPostscript(test_s1);
}

static void test_core()
{
    //test_op_curveto();
    //test_op_arc();

    test_simple();

}

static void test_idioms()
{
    //test_flower();
	//gridOfCircles();
    //radialLines();
    //scaledRectangles();
    grid();
}

int main() {

    //test_core();
    test_idioms();

    return 0;
}

