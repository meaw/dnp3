#call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
cd temp\boost_1_47_0

call ./bootstrap.bat

set STAGE_DIR=stage64

bjam.exe toolset=msvc link=static,shared address-model=64 --with-program_options --with-system --with-date_time --with-thread --with-filesystem --with-test --stagedir=%STAGE_DIR% stage

set libdir=%TOOLS_HOME%\boostlib\boost_1_47_0\wow64

echo Installing libs to %libdir%
md %libdir%
copy %STAGE_DIR%\lib\* %libdir%

pause
