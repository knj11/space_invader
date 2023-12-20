pre: git clone --recursive "http link"
1) Open developer x64 tool set cmd prompt
- Windows Button -> All Apps -> Visual Studio Folder -> x64 Native Tools Command Prompt Visual Studio 2022
2) cd to root of project
3) mkdir build
4) run "cmake cmake -S . -B build -G "NMake Makefiles"
5) cmake --build build
6) build\space_invaders.exe


x64 Enviroment
run
"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Visual Studio 2022\\Visual Studio Tools\\VC\\x64 Native Tools Command Prompt for VS 2022.lnk"
cd 