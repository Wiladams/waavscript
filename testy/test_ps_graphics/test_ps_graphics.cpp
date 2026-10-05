#include <memory>
#include <cstdio>
#include <filesystem>

#include "psvmfactory.h"
#include "b2dcontext.h"



using namespace waavs;

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
static void runPostscript(const char* sourceText) 
{
    OctetCursor input(sourceText);

    printf("+-----------------------------------------+\n");
    printf("PS INPUT\n %s\n", sourceText);
    printf("+-----------------------------------------+\n");

    auto vm = PSVMFactory::createVM();
    loadFontsInDirectory(vm.get(), "c:/windows/fonts");

    if (!vm) {
        printf("Failed to create virtual machine\n");
        return;
    }

    // Setup the VM and run the interpreter
    auto ctx = std::make_unique<waavs::Blend2DGraphicsContext>(1700, 2200);
    vm->setGraphicsContext(std::move(ctx));
    vm->interpret(input);

    // If we want, we can save output here
    static_cast<waavs::Blend2DGraphicsContext*>(vm->graphics())->getImage().write_to_file("output.png");

}

// ------------ Test Cases ------------

static void test_lines()
{
    const char* test_s1 = R"||(
newpath
0 10 360 {
  gsave
    400 400 translate
    /y exch def
    /x 10 def
    y rotate
    x y moveto
    200  y lineto
    stroke
  grestore
} for
)||";

    runPostscript(test_s1);
}

static void test_current_path()
{
	// Let's see if the current path is part of the interpreter's state
    // or not.
    const char* test_s1 = R"||(
newpath
10 10 moveto
10 200 lineto
200 200 lineto
3 setlinewidth

gsave
  1 0 0 setrgbcolor
  9 setlinewidth
  stroke
grestore

% do another stroke.  The path should have been preserved
% as well as the default color (black) and linewidth (1)
% so a skinny black line atop a thicker red line should be visible.
stroke        

)||";
    runPostscript(test_s1);
}

static void test_path_forall()
{
    const char* test_s1 = R"||(
newpath
10 20 moveto
30 40 lineto
50 60 70 80 90 100 curveto
closepath

{ (M ) print exch = = }
{ (L ) print exch = = }
{ (C ) print 6 { = } repeat }
{ (Z) = }
pathforall
)||";
    runPostscript(test_s1);
}

static void test_numeric() {
    printf("== Numeric parsing ==\n");
    const char* numeric_s1 = R"||(
/bigcircle {12 12 8 0 360 arc 0 setgray .1 setlinewidth stroke} def
/littlecircle {12 12 3 0 360 arc 0 setgray .1 setlinewidth stroke } def
/rbox { 
    -7 0 moveto 
    0 199 rlineto 
    299 0 rlineto 
    0 -199 rlineto 
    closepath
    2 setlinewidth stroke
} def

/twocircles {
    bigcircle 
    littlecircle 
} def

/whiteborder {
    {
        twocircles 
        11 0 translate
    } repeat
} def



gsave 
    70 95 translate 
    rbox 
    0.6 0.6 scale   
    43 whiteborder
    90 rotate 11 -13 translate 28 whiteborder
    90 rotate 11 -13 translate 43 whiteborder
    90 rotate 11 -13 translate 28 whiteborder
grestore
)||";

    const char* numeric_s2 = R"||(
/bigcircle {12 12 8 0 360 arc 0 setgray 2 setlinewidth stroke} def
/littlecircle {12 12 3 0 360 arc 0 setgray 2 setlinewidth stroke } def
/rbox { 
    -7 0 moveto 
    0 199 rlineto 
    299 0 rlineto 
    0 -199 rlineto 
    closepath
    2 setlinewidth stroke
} def

bigcircle
littlecircle

1 0 0 setrgbcolor
rbox

)||";

    runPostscript(numeric_s1);
    //runPostscript(numeric_s2);

}

static void test_op_widthshow()
{
    const char* test_s1 = R"||(
%!PS

/Helvetica findfont 30 scalefont setfont

70 280 moveto
0 0 0 setrgbcolor
(BANANA) show

70 220 moveto
0 0 1 setrgbcolor
12 0 65 (BANANA) widthshow

showpage
)||";
    runPostscript(test_s1);
}

static void test_op_awidthshow()
{
    const char* test_s1 = R"||(
%!PS

/Helvetica findfont 28 scalefont setfont

% Normal reference
70 300 moveto
0 0 0 setrgbcolor
(This is awidthshow) show

% Add 2 units after every character,
% and an additional 12 units after every space.
70 240 moveto
1 0 0 setrgbcolor
12 0 32 2 0 (This is awidthshow) awidthshow

showpage
)||";
    runPostscript(test_s1);
}

static void test_text_along_curve()
{
    const char* test_s1 = R"||(
%!PS

/Helvetica-Bold findfont 28 scalefont setfont
0 0 0 setrgbcolor

/cx 300 def         % circle center x
/cy 250 def         % circle center y
/r 140 def          % radius

/angle 180 def      % start at left side of upper semicircle
/step 20 def        % 10 chars -> 9 gaps -> 180 / 9 = 20 degrees

/base matrix currentmatrix def

/placeAtAngle {
    % stack: angle
    /a exch def

    base setmatrix

    % Move origin to the point on the circle
    cx a cos r mul add
    cy a sin r mul add
    translate

    % Rotate baseline to follow the tangent
    a 90 sub rotate

    0 0 moveto
} def

% Position the first character
angle placeAtAngle

% Between each character pair, advance to the next angle
{
    pop pop
    /angle angle step sub def
    angle placeAtAngle
} (POSTSCRIPT) kshow

showpage
)||";
    runPostscript(test_s1);
}



static void test_op_curveto()
{
    const char* test_s1 = R"||(
4 setlinewidth
newpath
100 100 moveto
120 140 160 140 180 100 curveto
stroke
)||";

    const char* test_s3 = R"||(
newpath
100 100 moveto
120 130 140 170 160 160 curveto
currentpoint
pstack
)||";

    runPostscript(test_s1);
    //runPostscript(test_s3);
}

static void test_op_arc()
{
	const char* test_s1 = R"||(
4 setlinewidth
1 0 0 setrgbcolor

newpath
200 200 50 0 360 arc
stroke
showpage
)||";
    runPostscript(test_s1);
}

static void test_op_arcto()
{
    const char* test_s1 = R"||(
100 100 moveto
150 150 200 100 10 arcto
4 2 roll lineto lineto  % use the tangent points
stroke
)||";

    const char* test_s2 = R"||(
%!PS
/R 20 def
1 0 0 setrgbcolor
4 setlinewidth

newpath
0 150 moveto
0 400 200 400 R 2 mul arcto
400 400 400 200 R 2 mul arcto
400 150 250 150 R 2 mul arcto
100 0 100 150 R 4 div arcto
closepath
gsave
  0 0 1 setrgbcolor
  fill
grestore

stroke
)||";

    //runPostscript(test_s1);
    runPostscript(test_s2);
}

//============================================================================
// Idiomatic Tests
//============================================================================

static void test_simple()
{
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
/grid {
0.5 0.3 0 0 setcmykcolor 

% Draw X-Y axis lines
gsave
    2 setlinewidth 
    400 0 moveto 
    0 0 lineto 
    0 500 lineto 
    stroke
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
4 { 
    100 20 moveto 
    100 500 lineto 
    stroke 
    100 0 translate 
} repeat
grestore
} def

grid
)||";

    runPostscript(test_s1);
}

static void star()
{
    const char* test_s1 = R"||(
/side {72 0 lineto currentpoint translate 144 rotate} bind def

newpath
5 30 translate 0 0 moveto
4 { side } repeat closepath
stroke
showpage
)||";
    runPostscript(test_s1);
}


static void truchet()
{
	const char* test_s1 = R"||(
%!PS-Adobe-3.0 EPSF-3.0 
%%BoundingBox: 0 0 595 842 
2.835 dup scale 
% 5 4 translate 
1 setlinecap 
0 0 200 290 rectstroke 
100 145 translate 
/W 10 def 
/W2 { W 2 div }  bind def 
/DRAWUNIT { 
gsave  
translate 
rotate 
 W2 neg W2 neg W2   0  90 arc 
stroke 
 W2 W2 W2 180 270 arc 
stroke 
grestore 
} def -95 W 95 { 
  /x exch def 
  -140 W 140 { 
    /y 
exch def 
    rand 4 mod 90 mul  x y  DRAWUNIT 
  } for 
} for 
showpage 
)||";
    runPostscript(test_s1);
}

static void pbourke_example9()
{
    const char *test_s1 = R"||(
%!
%%Example 9
100 200 translate
26 34 scale
26 34 8 [26 0 0 34 0 34]
{<
ffffffffffffffffffffffffffffffffffffffffffffffffffff
ff000000000000000000000000000000000000ffffffffffffff
ff00efefefefefefefefefefefefefefefef0000ffffffffffff
ff00efefefefefefefefefefefefefefefef00ce00ffffffffff
ff00efefefefefefefefefefefefefefefef00cece00ffffffff
ff00efefefefefefefefefefefefefefefef00cecece00ffffff
ff00efefefefefefefefefefefefefefefef00cececece00ffff
ff00efefefefefefefefefefefefefefefef00000000000000ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efef000000ef000000ef000000ef0000ef0000efefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efef000000ef00000000ef00000000ef000000efefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efef0000ef00000000000000ef000000ef0000efefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff00efefefefefefefefefefefefefefefefefefefefefef00ff
ff000000000000000000000000000000000000000000000000ff
ffffffffffffffffffffffffffffffffffffffffffffffffffff
>}
image

showpage
)||";

    runPostscript(test_s1);
}


static void test_core()
{
    //test_lines();
    //test_op_curveto();
    //test_op_arc();
    //test_op_arcto();
    //test_current_path();
    //test_path_forall();
    //test_numeric();
    //test_simple();
    //test_op_widthshow();
    test_op_awidthshow();
}

static void test_idioms()
{
    //test_flower();
	//gridOfCircles();
    //radialLines();
    //scaledRectangles();
    //grid();
    //truchet();
    //pbourke_example9();
    //star();
    test_text_along_curve();
}

int main() {

    //test_core();
    test_idioms();

    return 0;
}

