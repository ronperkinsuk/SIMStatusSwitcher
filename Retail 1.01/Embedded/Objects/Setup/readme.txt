AppInst Readme
===============
This sample demonstrates how to install applications using the CAB Wizard (CabWiz) application top build installable CAB files, interact with the ActiveSync Application Manager on the desktop, and include a custom setup DLL in your installation to allow you to run code to perform operations during install and uninstall that are not directly supported by the CAB file options.
This sample illustrates the installation process only. The Blackjack.EXE sample does not actually contain an implementation of the card game blackjack, it simply displays a message box. Likewise, the setup.dll file displays message boxes in each of its exported functions.

Files
-----
build.bat     - calls CabWiz to create the CAB files
register.bat  - calls CEAppMgr to register the app

common\*.*      - sample shared files
ARM_bins\*.* 	- sample binaries for Pocket PC ARM CPU
x86emu_bins\*.* - sample binaries for Pocket PC Emulator x86 CPU

cabfiles\*.*  - a copy of the CabWiz-generated CAB files

code\blackjack setup\*.*  - code used for "blkjack.exe"
code\setupdll\*.* - code used for the CESetupDLL "Blackjack Setup.dll"
code\selfregister\*.*  - code used for the self-registering "SelfRegister.dll"

Notes
-----
- The CAB files created by this sample can be used on either a real Pocket PC 2000 or later device or in the Pocket PC 2002 or later emulator.
- You may need to modify the pathnames in the .BAT files appropriately.
- Run "build.bat" to create the CAB files, and then "register.bat"
to register the sample app with CEAppMgr.  If you cannot create the CAB
files, you can use the CABs found in the "cabfiles" subdirectory.
- This only works with Active Sync 3.0 and above. It doesn't work with previous version of active Sync.

