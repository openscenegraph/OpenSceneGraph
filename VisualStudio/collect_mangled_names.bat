@echo off
rem Insert below the path to the installed visual studio vcvars32.bat
call "C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin\vcvars32.bat"
cscript collect_mangled_names.js
