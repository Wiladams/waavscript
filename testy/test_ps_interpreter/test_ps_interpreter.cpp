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
	vm->interpret(input);
    // PSInterpreter interp(*vm);
    //interp.interpret(input);

    // If we want, we can save output here
}

// ------------ Test Cases ------------
static void test_meta() {
    printf("== Meta Operators ==\n");
    runPostscript("languagelevel ="); // expect: 3, 2, 1
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
    0.1 setlinewidth stroke
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

    runPostscript(numeric_s1); 

}

static void test_arithmetic_ops() {
    printf("== Arithmetic Operators ==\n");
    runPostscript("10 5 add =   % expect 15");
    runPostscript("10 5 sub =");   // expect: 5
    runPostscript("10 5 mul =");   // expect: 50
    runPostscript("10 5 div =");   // expect: 2
}

static void test_stack_ops() {
    printf("\n== Stack Operators ==\n");
    runPostscript("1 2 3 dup = = =");       // expect: 3, 3, 2
    runPostscript("1 2 exch = =");          // expect: 1, 2
    runPostscript("1 2 3 pop = =");         // expect: 2, 1
    runPostscript("1 2 3 3 copy = = = = = = "); // expect: 3, 2, 1, 3, 2, 1,
    runPostscript("clear pstack");                // just clear the stack
}
const char* stopped_s1 = R"||(
{
  (before stop) =
  stop
  (after stop) =
} stopped
(continued) =
)||";

static void test_op_stopped() {
    runPostscript(stopped_s1); // should print "before stop" and then "continued"
}

static void test_control_flow() {
    printf("\n== Control Flow  ==\n");
    runPostscript("true { 1 } if = ");     // should not error if 'if' is implemented
    runPostscript("0 1 2 { = } repeat  % expect: 1, 0"); // should not error if 'repeat' is implemented,    expect: 1, 0
	runPostscript("0 1 3 {=} for % prints 0 1 2 3"); // expect: 0, 1, 2, 3
	runPostscript("3 -1 0 {=} for % prints 3 2 1 0"); // expect: 3, 2, 1, 0
    runPostscript("0 0.25 3 {=} for % prints 0..3, in increments of 0.25");
}

void test_forall()
{
    printf("\n== ForAll Operator ==\n");
    runPostscript("[ 10 20 30 ] { = } forall");
    runPostscript("(abc) { = } forall");
    runPostscript("[ (hello) 123 true [4 5 6] ] { dup type = } forall");
    runPostscript("[ 1 2 3 4 5 ] { dup 3 eq{ exit } if (Value = ) print =} forall");
    runPostscript("[ 1 2 3 4 5 ] { dup mul = } forall");
}

static void test_debug_ops() {
    printf("\n== Debug Operators ==\n");
    runPostscript("[1 2 3 ] << /foo (bar) /nums [4 5 6] >> =="); // expect: 3, 2, 1
    runPostscript("1 2 3 [4 5 6] stack");
    runPostscript("1 2 3 [4 5 6] pstack");
}

const char* loop_s1 = R"||(
0 {
    (start) =
        dup 
        1 add 
        dup 3 gt 
    { (exit triggered) = exit } if
} loop
)||";

const char* loop_s2 = R"||(
0 10 {
    (start) =
        dup

        1 add

        dup

        3 gt

        { (exit triggered) = exit } 
 pstack
        if
} repeat
)||";


static void test_loop_op()
{
        printf("\n== Loop Operator ==\n");
        //runPostscript("1 dup ="); // expect: 1
		//runPostscript("2 1 add ="); // expect: 3
        //runPostscript("3 2 gt ="); // expect true
		//runPostscript("2 3 gt {(true) =} if  %should print nothing"); // expect: nothing
		//runPostscript("4 3 gt {(yes) =} if  %should print 'yes'"); // expect: yes
        //runPostscript("{ (hi) = exit (after-exit) = } loop");
		runPostscript(loop_s1);
        //runPostscript(loop_s2);

		//runPostscript("0 { dup = 1 add dup 3 gt{ exit } if } loop pop"); 
}

static void test_logic()
{
	printf("\n== Logic Operators ==\n");
    static const char* logic_s1 = R"||(
1 2 gt =
1 2 gt { (should not print) = } if
    )||";

	runPostscript(logic_s1);
}


static void test_procedure()
{
    printf("\n== Procedure Operators ==\n");
    runPostscript("true { { (Nested procedure executed) = } exec }  if");
	runPostscript("3 { { (Hello) = } exec } repeat                 % Should print (Hello) 3 times");
    runPostscript("3 { (Hello) = } repeat                          % Should print (Hello) 3 times");
    runPostscript("true { true { (Both conditions met) = } if } if          % Should printf (Both conditions met)");
}

static void test_repeat()
{
    printf("\n== Repeat Operator ==\n");

    runPostscript("0 3 { (inside repeat) = } repeat");
}



static void test_nested()
{
    const char* nested_s1 = R"||(
% Nested execution
/innerProc { (hello from inner) = } def
/outerProc { innerProc  } def
outerProc
)||";
    
    const char* nested_s2 = R"||(
% Nested execution
/outerProc { innerProc exec } def
outerProc
)||";

    const char* nested_s3 = R"||(
/innerProc { (hello from inner) = } def
/outerProc { /innerProc load exec } def
outerProc

)||";

    runPostscript(nested_s1);
    //runPostscript(nested_s2);
    //runPostscript(nested_s3);


}


static void test_op_dict()
{

    printf("\n== Operator Dictionary ==\n");

    const char* test_s1 = R"||(
1 dict begin
/x 42 def
{x}         % this should not error
end
exec        % this should push 42 if `x` is still visible
)||";

    const char* test_s2 = R"||(
1 dict begin
/y 123 def
{ y } exec   % should push 123
pstack
end
)||";

    const char* test_s3 = R"||(
/minidict 24 dict def
minidict begin
/ld { load def } def
/gs /gsave ld
/gr /grestore ld
/np /newpath ld
/cp /closepath ld
/mt /moveto ld
/rt /rmoveto ld
/li /lineto ld
/rl /rlineto ld
/ct /curveto ld
/tr /translate ld 
/st /stroke ld
/set { gs setlinewidth st gr } def      % use # set to set line width
/gray {gs setgray fill gr} def          % use # gray, to fill with gray
/ro /rotate ld
/rp /repeat ld
/box {np mt rl rl rl cp set} def        % composite box, no fill
/circle {np arc set} def                % composite circle, no fill
end

% Use the dictionary
minidict begin
)||";

    //runPostscript(test_s1);
    //runPostscript(test_s2);
    runPostscript(test_s3);

}


static void test_exec()
{
    runPostscript("{ 1 2 add } exec =");
}

static void test_operator_def()
{
    printf("\n== Operator Definition ==\n");
    runPostscript("/x 42 def x =");
}

static void test_unimplemented_op() 
{
    printf("\n== Unimplemented Operator ==\n");
    runPostscript("foobar"); // Should trigger an undefined name or op message
}


//============================================================================
// Idiomatic Tests
//============================================================================

static void test_tail()
{
    // This will crash the interpreter if tail call optimization is not implemented
    const char* test_s1 = R"||(
% fact_tail: ( n acc -- result )
/fact_tail {
  dup 0 eq
  {
    pop            % drop n
  }
  {
    dup 1 sub      % n n-1
    exch mul       % n-1 (n * acc)
    exch           % restore (n acc)
    fact_tail      % tail call
  } ifelse
} def

/fact {
  1 exch
  fact_tail
} def

% Try a large number:
1000 fact pop     % discard result; just test for crash or stack overflow

(OK) =

)||";

    const char* test_s2 = R"||(
/fact {
  dup 1 le
  { pop 1 }
  { dup 1 sub fact mul }
  ifelse
} def

5 fact =  % should print 120
)||";

    runPostscript(test_s1);
    //runPostscript(test_s2);
}

static void test_factorial()
{
	printf("\n== Factorial Function ==\n");

    const char* test_s1 = R"||(
/fact {
  dup 1 le
  { pop 1 }
  { dup 1 sub fact mul }
  ifelse
} def

5 fact =  % should print 120
)||";

    runPostscript(test_s1);
}

static void test_average()
{
	printf("\n== Average Function ==\n");
    const char* test_s1 = R"||(
/average { add 2 div } def
/printnum { 4 string cvs print } def
/printnl { <0A> print } def
40 60 average printnum printnl
)||";

    printf("\n== Average Function ==\n");
    const char* test_s2 = R"||(
/average { add 2 div } def
40 60 average =
)||";

	//runPostscript(test_s1);
	runPostscript(test_s2);
}

static void test_fizzbuzz()
{
    printf("\n== FizzBuzz Function ==\n");
	const char* test_s1 = R"||(
% fizzbuzz is a simple function that returns a string based on the given integer
% multiples of 3, but not 5 should return fizz
% multiples of 5, but not 3 should return buzz
% multiples of 15 (i.e. both 3 and 5) should return fizzbuzz
% all other integers should return the integer as a string

/fizzbuzz {
  1 dict begin
  /num exch def
  num 15 mod 0 eq { (fizzbuzz) } {
    num 5 mod 0 eq { (buzz) } {
      num 3 mod 0 eq { (fizz) } {
        20 string num cvs
      } ifelse
    } ifelse
  } ifelse
  end
} def

30 fizzbuzz
)||";

	runPostscript(test_s1);
}

static void test_matrix_ops()
{
    printf("\n== Matrix Operators ==\n");
    const char* test_s1 = R"||(
% Create a matrix and print it
matrix
dup =
% Expected: identity matrix [1 0 0 1 0 0]
)||";

    const char* test_s2 = R"||(
% Translate by (10, 20)
10 20 matrix translate
dup =
% Expected: [1 0 0 1 10 20]
)||";

    const char* test_s3 = R"||(
% Scale by (2, 3)
2 3 matrix scale
dup =
% Expected: [2 0 0 3 0 0]
)||";

    const char* test_s4 = R"||(
% Rotate 45 degrees
45 matrix rotate
dup =
% Expected: [0.7071 0.7071 -0.7071 0.7071 0 0]
)||";

    const char* test_s5 = R"||(
% Compose translation and scaling
10 0 matrix translate
dup =
2 2 matrix scale
dup =
matrix
concatmatrix
dup =
% Expected: [2 0 0 2 10 0]
)||";

    const char* test_s6 = R"||(
% Invert a matrix [1 2 3 4 5 6]
1 2 3 4 5 6 6 array astore
invertmatrix
dup =
% Expected: [-2 1.5 1 -0.5 -2 1]
)||";

    const char* test_s7 = R"||(
% Push point
1 2                             % x = 1, y = 2

% Push matrix [1 0 0 1 10 5]
1 0 0 1 10 5 6 array astore

% Transform
transform = = 
% Expected: 11 7
)||";

    const char* test_s8 = R"||(
% dtransform (linear part only)
1 2 dtransform = =
% Expected: 1 2
)||";


    //runPostscript(test_s1);
	//runPostscript(test_s2);
    //runPostscript(test_s3);
    //runPostscript(test_s4);
    runPostscript(test_s5);
    //runPostscript(test_s6);
    //runPostscript(test_s7);
    //runPostscript(test_s8);

}

static void test_dictionary_inline()
{
    printf("\n== Dictionary Inline ==\n");
    const char* test_s1 = R"||(
/inchesWide 8.5 def % letter
/inchesHigh 11 def
/ppi 72 def % adobe points per inch
/pointsWide inchesWide ppi mul def
/pointsHigh inchesHigh ppi mul def
/escala_cm 1 2.54 div 72 mul def 
<<
  /PageSize [pointsWide pointsHigh]
>> setpagedevice
)||";

	runPostscript(test_s1);
}

static void test_resources()
{
    printf("\n== Resources ==\n");
    const char* test_s1 = R"||(
% Define a simple resource in the Font category
/myFontResource <<
  /FontType 3
  /FontMatrix [0.001 0 0 0.001 0 0]
  /Encoding StandardEncoding
>> 
/Font defineresource

% Check its status
/myFontResource /Font resourcestatus
% Should print: true
{ = = = } if

% Find the resource
/myFontResource /Font findresource
% Should print the dictionary
==

% List all Font resources using resourceforall
/Font {
  (resourceforall:) print
  = % key
  = % value
} resourceforall

% Save the resource stack
resourcestack
dup
% Should print the array
==

% Now replace the resource stack with just the system dictionary
dup 0 1 getinterval setresourcestack

% Test that the user resource is no longer visible
/myFontResource /Font resourcestatus
% Should print: false
=

% Restore the saved stack
setresourcestack

% Verify the resource is found again
/myFontResource /Font resourcestatus
% Should print: true
{ = = = } if

% Test beginresource / endresource
/myComposite /Pattern beginresource
% define pattern parts here if you wanted
endresource

)||";

    runPostscript(test_s1);
}

static void test_encodings()
{
    printf("\n== Encoding Tests ==\n");

    const char* test_s1 = R"||(
% Check StandardEncoding
(StandardEncoding:) print
StandardEncoding type ==
StandardEncoding length ==     % 256
StandardEncoding 65 get ==     % /A
StandardEncoding 97 get ==     % /a

% Check ISOLatin1Encoding
(ISOLatin1Encoding:) print
ISOLatin1Encoding type ==
ISOLatin1Encoding length ==    % 256
ISOLatin1Encoding 160 get ==   % /space
ISOLatin1Encoding 173 get ==   % /hyphen
ISOLatin1Encoding 255 get ==   % /ydieresis

% Check WinAnsiEncoding
(WinAnsiEncoding:) print
WinAnsiEncoding type ==
WinAnsiEncoding length ==      % 256
WinAnsiEncoding 128 get ==     % /Euro
WinAnsiEncoding 164 get ==     % /currency
WinAnsiEncoding 216 get ==     % /Oslash

% Check MacRomanEncoding
(MacRomanEncoding:) print
MacRomanEncoding type ==
MacRomanEncoding length ==     % 256
MacRomanEncoding 65 get ==     % /A
MacRomanEncoding 128 get ==    % /Adieresis
MacRomanEncoding 141 get ==    % /Oslash

% Check ExpertEncoding
(ExpertEncoding:) print
ExpertEncoding type ==
ExpertEncoding length ==       % 256
ExpertEncoding 33 get ==       % /exclamsmall
ExpertEncoding 100 get ==      % /onequarter
ExpertEncoding 200 get ==      % /bracketlefttp

% Check SymbolEncoding
(SymbolEncoding:) print
SymbolEncoding type ==
SymbolEncoding length ==       % 256
SymbolEncoding 65 get ==       % /Alpha
SymbolEncoding 97 get ==       % /alpha
SymbolEncoding 240 get ==      % /therefore

% Check ZapfDingbatsEncoding
(ZapfDingbatsEncoding:) print
ZapfDingbatsEncoding type ==
ZapfDingbatsEncoding length == % 256
ZapfDingbatsEncoding 33 get ==   % /a1
ZapfDingbatsEncoding 97 get ==   % /a60
ZapfDingbatsEncoding 191 get ==  % /a191
ZapfDingbatsEncoding 250 get ==  % /.notdef

% All done
(Done testing encodings.) print


)||";

    runPostscript(test_s1);
}

static void test_encodings2()
{
    printf("\n== Resources ==\n");
    const char* test_s1 = R"||(
/listarray
{
    0                          % start index at 0
    /myindex exch def          % store in myindex
    20 string /indexstr exch def  % reusable string buffer
    {
        myindex indexstr cvs print   % convert and print index (no newline)
        ( Value: ) print
        =                            % print element with newline
        /myindex myindex 1 add def
    } forall
} def

StandardEncoding listarray
% ISOLatin1Encoding listarray
% WinAnsiEncoding listarray
% ExpertEncoding listarray
% ZapfDingbatsEncoding listarray
)||";

    runPostscript(test_s1);
}

// ------------ Entry ------------

static void test_core()
{
	printf("== Core Tests ==\n");
    //test_meta();
    test_arithmetic_ops();
    test_stack_ops();
    test_control_flow();
    test_debug_ops();
    test_loop_op();
    test_forall();
    test_logic();
    test_procedure();
    test_repeat();
    test_nested();
    test_exec();
    //test_op_stopped();
    test_operator_def();
    //test_op_dict();
	test_matrix_ops();
    //test_unimplemented_op();
    //test_dictionary_inline();
    //test_numeric();
    //test_resources();
    //test_encodings();
    //test_encodings2();
}

static void test_idioms()
{
	printf("\n== Idiomatic Tests ==\n");
    //test_tail();
    //test_factorial();
    test_average();
    //test_fizzbuzz();

}




int main() {
    test_core();
    //test_idioms();

    return 0;
}

