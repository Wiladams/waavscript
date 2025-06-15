#pragma once

namespace waavs {
struct PSVMOps {

const char* op_executive = R"||(
% calling format
% executive
% found in: userdict
% referenced in 1 (specialswitch)
/executive
{
	//execdict
	begin
	cearinterrupt
	disableinterrupt

/execdepth
1
add
def

(\nPostScript (tm) Version )

print
version
print
(\nCopyright (c) 1985 Adobe Systems Incorporated.\n)

print

% begin executive loop
{
} def
)||";
	};

const char* op_setnulldevice = R"||(
/setnulldevice
{
  nulldevice

  $printerdict
  /mtx
  get

  setmatrix
} def
)||";

const char* op_start = R"||(
/start
{
  disableinterrupt
  execdict
/execdepth
0
put
ReadIdleFonts

} def
)||";

}