@echo off
REM This script provides a commandline shell properly configured to run OSG
REM executables directly from the OpenThreads/OpenSceneGraph source
REM trees.
REM mew 2004-07-16

cd ..\..
set PATH=%CD%\OpenThreads\bin\win32;%CD%\bin;%CD%\3rdParty\bin;%PATH%
set OSG_FILE_PATH=%CD%\OpenSceneGraph-Data;%OSG_FILE_PATH%

REM uncomment one of these for your desired notify level...
rem set OSG_NOTIFY_LEVEL=ALWAYS
rem set OSG_NOTIFY_LEVEL=FATAL
rem set OSG_NOTIFY_LEVEL=WARN
rem set OSG_NOTIFY_LEVEL=NOTICE
rem set OSG_NOTIFY_LEVEL=DEBUG
rem set OSG_NOTIFY_LEVEL=DEBUG_FP
rem set OSG_NOTIFY_LEVEL=INFO

TITLE osgShell
%COMSPEC% /K
 

