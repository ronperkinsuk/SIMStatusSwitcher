@echo off
REM
REM register.bat registers the SIM Status Switcher Application with CEAppMgr.exe
REM

REM
REM You must modify the following directories to point to the correct locations. 
REM Make sure the CAB file(s) to be installed are in the appinst\ root directory.
REM

set fileWiVSSS="D:\Development\Projects\SIM Status Switcher\Retail\Install\WiVSSS.ini"
set fileCEAppMgr="C:\Program Files\Microsoft ActiveSync\ceappmgr.exe"

if not exist %fileWiVSSS% goto Usage
if not exist %fileCEAppMgr% goto Usage

%fileCEAppMgr% %fileWiVSSS%
goto Exit

:Usage
@echo ---
@echo Edit this batch file to point to the correct directories
@echo    fileWiVSSS  = %fileWiVSSS%
@echo    fileCEAppMgr = %fileCEAppMgr%
@echo       (this file is installed by Windows CE Services)
@echo ---

:Exit
