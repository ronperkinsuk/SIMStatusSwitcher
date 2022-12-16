@echo off
REM
REM build.bat creates the CAB files
REM

REM
REM You Must modify the following directories to point to the correct locations.
REM

set fileWiVSSS="D:\Development\Projects\SIM Status Switcher\Retail 1.1\Embedded\Install\WiVSSS10.inf"
set fileCabwiz="c:\program files\windows ce tools\wce420\pocket pc 2003\Tools\cabwiz.exe"

if not exist %fileWiVSSS% goto Usage
if not exist %fileCabwiz% goto Usage
echo "Building" %fileCabwiz% %fileWiVSSS%
%fileCabwiz% %fileWiVSSS% /err WiVSSS.err /cpu ARM 
echo "Built"
goto Exit

:Usage
@echo ---
@echo Edit this batch file to point to the correct directories
@echo    fileWiVSSS = %fileWiVSSS%
@echo    fileCabwiz  = %fileCabwiz%
@echo       (these files are included in the Windows CE SDK)
@echo ---

:Exit
