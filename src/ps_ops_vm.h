#pragma once

namespace waavs {
    struct PSVMOps {

        // Calculate the maximum between two values
        const char* op_code_max = R"||(
/.max {2 copy lt { exch } if pop } bind def
)||";

        const char* op_code_min = R"||(
/.min {2 copy gt { exch } if pop } bind def
)||";
    };

    struct PSVMEncodings {
        const char* fontMapPS = R"||(
/FontMap <<
  % Times family
  /Times-Roman             /timesnewromanpsmt
  /Times-Bold              /timesnewromanps-boldmt
  /Times-Italic            /timesnewromanps-italicmt
  /Times-BoldItalic        /timesnewromanps-bolditalicmt

  % Helvetica family mapped to Arial
  /Arial              /arialmt  
  /Helvetica               /arialmt
  /Helvetica-Bold          /arial-boldmt
  /Helvetica-Oblique       /arial-italicmt
  /Helvetica-BoldOblique   /arial-bolditalicmt

  % Courier family
  /Courier                 /couriernewpsmt
  /Courier-Bold            /couriernewps-boldmt
  /Courier-Oblique         /couriernewps-italicmt
  /Courier-BoldOblique     /couriernewps-bolditalicmt

  % Symbol
  /Symbol                  /symbolmt

  % ZapfDingbats substitute
  /ZapfDingbats            /wingdings-regular       % reasonable decorative symbol fallback

  

  % AvantGarde family substitute using Century Gothic-like feellMT is a good substitute for AvantGarde fonts

  /AvantGarde-Book         /arialmt
  /AvantGarde-BookOblique  /arial-italicmt
  /AvantGarde-Demi         /arial-boldmt
  /AvantGarde-DemiOblique  /arial-bolditalicmt

  % Bookman substitute using Georgia
  /Bookman-Light           /georgia
  /Bookman-LightItalic     /georgia-italic
  /Bookman-Demi            /georgia-bold
  /Bookman-DemiItalic      /georgia-bolditalic

  % Helvetica Narrow substitute with Calibri
  /Helvetica-Narrow            /calibri
  /Helvetica-Narrow-Bold       /calibri-bold
  /Helvetica-Narrow-Oblique    /calibri-italic
  /Helvetica-Narrow-BoldOblique /calibri-bolditalic

  % New Century Schoolbook substitute with Cambria
  /NewCenturySchlbk-Roman      /cambria
  /NewCenturySchlbk-Italic     /cambria-italic
  /NewCenturySchlbk-Bold       /cambria-bold
  /NewCenturySchlbk-BoldItalic /cambria-bolditalic

  % Palatino family
  /Palatino                /palatinolinotype-roman
  /Palatino-Roman          /palatinolinotype-roman
  /Palatino-Italic         /palatinolinotype-italic
  /Palatino-Bold           /palatinolinotype-bold
  /Palatino-BoldItalic     /palatinolinotype-bolditalic
>> def

/FindSystemFont
{
    dup FontMap exch known
    {
        FontMap exch get
    } if
    findfont
} def

)||";

        const char* standardEncodingPS = R"||(
/StandardEncoding [
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/space /exclam /quotedbl /numbersign /dollar /percent /ampersand /quoteright
/parenleft /parenright /asterisk /plus /comma /hyphen /period /slash
/zero /one /two /three /four /five /six /seven
/eight /nine /colon /semicolon /less /equal /greater /question
/at /A /B /C /D /E /F /G
/H /I /J /K /L /M /N /O
/P /Q /R /S /T /U /V /W
/X /Y /Z /bracketleft /backslash /bracketright /asciicircum /underscore
/quoteleft /a /b /c /d /e /f /g
/h /i /j /k /l /m /n /o
/p /q /r /s /t /u /v /w
/x /y /z /braceleft /bar /braceright /asciitilde /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/space /exclamdown /cent /sterling /fraction /yen /florin /section
/currency /quotesingle /quotedblleft /guillemotleft /guilsinglleft /guilsinglright /fi /fl
/.notdef /endash /dagger /daggerdbl /periodcentered /.notdef /paragraph /bullet
/quotesinglbase /quotedblbase /quotedblright /guillemotright /ellipsis /perthousand /.notdef /questiondown
/.notdef /grave /acute /circumflex /tilde /macron /breve /dotaccent
/dieresis /.notdef /ring /cedilla /.notdef /hungarumlaut /ogonek /caron
/emdash /.notdef /AE /ordfeminine /Lslash /Oslash /OE /ordmasculine
/.notdef /ae /dotlessi /lslash /oslash /oe /germandbls /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
] def
)||";


        const char* isoLatin1EncodingPS = R"||(
/ISOLatin1Encoding [
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/space /exclam /quotedbl /numbersign /dollar /percent /ampersand /quotesingle
/parenleft /parenright /asterisk /plus /comma /hyphen /period /slash
/zero /one /two /three /four /five /six /seven
/eight /nine /colon /semicolon /less /equal /greater /question
/at /A /B /C /D /E /F /G
/H /I /J /K /L /M /N /O
/P /Q /R /S /T /U /V /W
/X /Y /Z /bracketleft /backslash /bracketright /asciicircum /underscore
/grave /a /b /c /d /e /f /g
/h /i /j /k /l /m /n /o
/p /q /r /s /t /u /v /w
/x /y /z /braceleft /bar /braceright /asciitilde /.notdef
/nbspace /exclamdown /cent /sterling /currency /yen /brokenbar /section
/dieresis /copyright /ordfeminine /guillemotleft /logicalnot /softhyphen /registered /macron
/degree /plusminus /twosuperior /threesuperior /acute /mu /paragraph /periodcentered
/cedilla /onesuperior /ordmasculine /guillemotright /onequarter /onehalf /threequarters /questiondown
/Agrave /Aacute /Acircumflex /Atilde /Adieresis /Aring /AE /Ccedilla
/Egrave /Eacute /Ecircumflex /Edieresis /Igrave /Iacute /Icircumflex /Idieresis
/Eth /Ntilde /Ograve /Oacute /Ocircumflex /Otilde /Odieresis /multiply
/Oslash /Ugrave /Uacute /Ucircumflex /Udieresis /Yacute /Thorn /germandbls
/agrave /aacute /acircumflex /atilde /adieresis /aring /ae /ccedilla
/egrave /eacute /ecircumflex /edieresis /igrave /iacute /icircumflex /idieresis
/eth /ntilde /ograve /oacute /ocircumflex /otilde /odieresis /divide
/oslash /ugrave /uacute /ucircumflex /udieresis /yacute /thorn /ydieresis
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
] def
)||";





        const char* winAnsiEncodingPS = R"||(
/WinAnsiEncoding [
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/space /exclam /quotedbl /numbersign /dollar /percent /ampersand /quotesingle
/parenleft /parenright /asterisk /plus /comma /hyphen /period /slash
/zero /one /two /three /four /five /six /seven
/eight /nine /colon /semicolon /less /equal /greater /question
/at /A /B /C /D /E /F /G
/H /I /J /K /L /M /N /O
/P /Q /R /S /T /U /V /W
/X /Y /Z /bracketleft /backslash /bracketright /asciicircum /underscore
/grave /a /b /c /d /e /f /g
/h /i /j /k /l /m /n /o
/p /q /r /s /t /u /v /w
/x /y /z /braceleft /bar /braceright /asciitilde /.notdef
/.notdef /quotesinglbase /florin /quotedblbase /ellipsis /dagger /daggerdbl /circumflex
/perthousand /Scaron /guilsinglleft /OE /.notdef /Zcaron /.notdef /.notdef
/.notdef /quoteleft /quoteright /quotedblleft /quotedblright /bullet /endash /emdash
/tilde /scaron /guilsinglright /oe /.notdef /zcaron /.notdef /Ydieresis
/space /exclamdown /cent /sterling /currency /yen /brokenbar /section
/dieresis /copyright /ordfeminine /guillemotleft /logicalnot /hyphen /registered /macron
/degree /plusminus /twosuperior /threesuperior /acute /mu /paragraph /periodcentered
/cedilla /onesuperior /ordmasculine /guillemotright /onequarter /onehalf /threequarters /questiondown
/Agrave /Aacute /Acircumflex /Atilde /Adieresis /Aring /AE /Ccedilla
/Egrave /Eacute /Ecircumflex /Edieresis /Igrave /Iacute /Icircumflex /Idieresis
/Eth /Ntilde /Ograve /Oacute /Ocircumflex /Otilde /Odieresis /multiply
/Oslash /Ugrave /Uacute /Ucircumflex /Udieresis /Yacute /Thorn /germandbls
/agrave /aacute /acircumflex /atilde /adieresis /aring /ae /ccedilla
/egrave /eacute /ecircumflex /edieresis /igrave /iacute /icircumflex /idieresis
/eth /ntilde /ograve /oacute /ocircumflex /otilde /odieresis /divide
/oslash /ugrave /uacute /ucircumflex /udieresis /yacute /thorn /ydieresis
] def
)||";


        const char* macRomanEncodingPS = R"||(
/MacRomanEncoding [
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/space /exclam /quotedbl /numbersign /dollar /percent /ampersand /quotesingle
/parenleft /parenright /asterisk /plus /comma /hyphen /period /slash
/zero /one /two /three /four /five /six /seven
/eight /nine /colon /semicolon /less /equal /greater /question
/at /A /B /C /D /E /F /G
/H /I /J /K /L /M /N /O
/P /Q /R /S /T /U /V /W
/X /Y /Z /bracketleft /backslash /bracketright /asciicircum /underscore
/grave /a /b /c /d /e /f /g
/h /i /j /k /l /m /n /o
/p /q /r /s /t /u /v /w
/x /y /z /braceleft /bar /braceright /asciitilde /.notdef
/dotlessi /grave /acute /circumflex /tilde /macron /breve /dotaccent
/dieresis /ring /cedilla /hungarumlaut /ogonek /caron /.notdef /.notdef
/space /exclamdown /cent /sterling /currency /yen /brokenbar /section
/dieresis /copyright /ordfeminine /guillemotleft /logicalnot /hyphen /registered /macron
/degree /plusminus /twosuperior /threesuperior /acute /mu /paragraph /periodcentered
/cedilla /onesuperior /ordmasculine /guillemotright /onequarter /onehalf /threequarters /questiondown
/Agrave /Aacute /Acircumflex /Atilde /Adieresis /Aring /AE /Ccedilla
/Egrave /Eacute /Ecircumflex /Edieresis /Igrave /Iacute /Icircumflex /Idieresis
/Eth /Ntilde /Ograve /Oacute /Ocircumflex /Otilde /Odieresis /multiply
/Oslash /Ugrave /Uacute /Ucircumflex /Udieresis /Yacute /Thorn /germandbls
/agrave /aacute /acircumflex /atilde /adieresis /aring /ae /ccedilla
/egrave /eacute /ecircumflex /edieresis /igrave /iacute /icircumflex /idieresis
/eth /ntilde /ograve /oacute /ocircumflex /otilde /odieresis /divide
/oslash /ugrave /uacute /ucircumflex /udieresis /yacute /thorn /ydieresis
] def
)||";



        const char* expertEncodingPS = R"||(
/ExpertEncoding [
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/space /exclamsmall /Hungarumlautsmall /.notdef /dollaroldstyle /dollarsuperior /ampersandsmall /Acutesmall
/parenleftsuperior /parenrightsuperior /twodotenleader /onedotenleader /comma /hyphen /period /fraction
/zerooldstyle /oneoldstyle /twooldstyle /threeoldstyle /fouroldstyle /fiveoldstyle /sixoldstyle /sevenoldstyle
/eightoldstyle /nineoldstyle /colon /semicolon /commasuperior /threequartersemdash /periodsuperior /questionsmall
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
] def
)||";



        const char* symbolEncodingPS = R"||(
/SymbolEncoding [
    /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
    /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
    /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
    /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
    /space /exclam /universal /numbersign /existential /.notdef /.notdef /suchthat
    /parenleft /parenright /asteriskmath /plus /comma /minus /period /slash
    /zero /one /two /three /four /five /six /seven /eight /nine /colon
    /semicolon /less /equal /greater /question /congruent
    /Alpha /Beta /Chi /Delta /Epsilon /Phi /Gamma /Eta
    /Iota /theta1 /Kappa /Lambda /Mu /Nu /Omicron /Pi
    /Theta /Rho /Sigma /Tau /Upsilon /sigma1 /Omega /Xi
    /Psi /Zeta /bracketleft /therefore /bracketright /perpendicular /underscore /.notdef
    /radicalex /alpha /beta /chi /delta /epsilon /phi /gamma
    /eta /iota /phi1 /kappa /lambda /mu /nu /omicron
    /pi /theta /rho /sigma /tau /upsilon /omega1 /omega
    /xi /psi /zeta /braceleft /bar /braceright /similar /.notdef
    /.notdef /Upsilon1 /.notdef /minute /lessequal /fraction /infinity /florin
    /club /diamond /heart /spade /arrowboth /arrowleft /arrowup /arrowright
    /arrowdown /degree /plusminus /second /greaterequal /multiply /proportional /partialdiff
    /bullet /divide /notequal /equivalence /approxequal /ellipsis /arrowvertex /arrowhorizex
    /carriagereturn /aleph /Ifraktur /Rfraktur /weierstrass /circlemultiply
    /circleplus /emptyset /intersection /union /propersuperset /reflexsuperset
    /notsubset /propersubset /reflexsubset /element /notelement /angle /gradient
    /registerserif /copyrightserif /trademarkserif /product /radical /dotmath
    /logicalnot /logicaland /logicalor /arrowdblboth /arrowdblleft /arrowdblup
    /arrowdblright /arrowdbldown /lozenge /angleleft /registersans /copyrightsans
    /trademarksans /summation /parenlefttp /parenleftex /parenleftbt
    /bracketlefttp /bracketleftex /bracketleftbt /bracelefttp /braceleftmid
    /braceleftbt /braceex /parenrighttp /parenrightex /parenrightbt
    /bracketrighttp /bracketrightex /bracketrightbt /bracerighttp
    /bracerightmid /bracerightbt
] def
)||";

    const char* zapfDingbatsEncodingPS = R"||(
/ZapfDingbatsEncoding [
  /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
  /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
  /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
  /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
  /.notdef
  /space /a1 /a2 /a202 /a3 /a4 /a5 /a119
  /a118 /a117 /a11 /a12 /a13 /a14 /a15 /a16
  /a105 /a17 /a18 /a19 /a20 /a21 /a22 /a23
  /a24 /a25 /a26 /a27 /a28 /a6 /a7 /a8
  /a9 /a10 /a29 /a30 /a31 /a32 /a33 /a34
  /a35 /a36 /a37 /a38 /a39 /a40 /a41 /a42
  /a43 /a44 /a45 /a46 /a47 /a48 /a49 /a50
  /a51 /a52 /a53 /a54 /a55 /a56 /a57 /a58
  /a59 /a60 /a61 /a62 /a63 /a64 /a65 /a66
  /a67 /a68 /a69 /a70 /a71 /a72 /a73 /a74
  /a203 /a75 /a204 /a76 /a77 /a78 /a79 /a81
  /a82 /a83 /a84 /a97 /a98 /a99 /a100 /a89
  /a90 /a93 /a94 /a91 /a92 /a205 /a85 /a206
  /a86 /a87 /a88 /a95 /a96 /a101 /a102 /a103
  /a104 /a106 /a107 /a108 /a112 /a111 /a110 /a109
  /a120 /a121 /a122 /a123 /a124 /a125 /a126 /a127
  /a128 /a129 /a130 /a131 /a132 /a133 /a134 /a135
  /a136 /a137 /a138 /a139 /a140 /a141 /a142 /a143
  /a144 /a145 /a146 /a147 /a148 /a149 /a150 /a151
  /a152 /a153 /a154 /a155 /a156 /a157 /a158 /a159
  /a160 /a161 /a163 /a164 /a196 /a165 /a192 /a166
  /a167 /a168 /a169 /a170 /a171 /a172 /a173 /a162
  /a174 /a175 /a176 /a177 /a178 /a179 /a193 /a180
  /a199 /a181 /a200 /a182 /a201 /a183 /a184 /a197
  /a185 /a194 /a198 /a186 /a195 /a187 /a188 /a189
  /a190 /a191
  /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
  /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
  /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
] def
    )||";


	};

}