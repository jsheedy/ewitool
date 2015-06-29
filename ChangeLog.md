### 0.7 (WIP) ###

New Key Patches tab, Max Volume function for patches and minor layout tweaks to fit laptops better.

New Features:
  * "Key Patches" tab to show/print the settings of the quick program change keys - [Issue #39](https://code.google.com/p/ewitool/issues/detail?id=#39)
  * Add "Maximise Volume" function under Patch|Process menu to push main levels in patch up to max level (but retain ratio between them) - [Issue #37](https://code.google.com/p/ewitool/issues/detail?id=#37)
  * Migrate to Qt 5 - changes to clipboard.cpp, keyPrograms\_form.cpp, main.cpp, mainwindow.cpp, mainwindow.h, patchexchange,cpp, patchexchange.h, src.pro
  * Runs on Macs! - EWItool now builds and runs natively on MacOS

Fixes:
  * Reduce height of main window to fit laptops better - [Issue #36](https://code.google.com/p/ewitool/issues/detail?id=#36)
    * Compact layout of patch editor
    * Reduce height of all elements in Patch Set frame
  * Fix handling of UTF-8 (international) characters in the Patch Exchange - [Issue #40](https://code.google.com/p/ewitool/issues/detail?id=#40)
  * (Cleanup) Eliminate 'magic numbers' in midi\_data class, introduce MIDI defines in midi\_data.h
  * Update RtMidi library to version 2.0.1
  * Improve robustness of MIDI message handling by verifying sanity of patch numbers and lengths of SysEx messages
  * Bail out when trying to import nonsense patches
  * Increase resolution of graphic print of a patch to 600dpi (from 300dpi)
  * Fix Qt 4 compile warning in main() - QDir
  * Fix hang-on-exit on Windows platform
  * Fix exposed SQL on some empty queries in EPX - [Issue #47](https://code.google.com/p/ewitool/issues/detail?id=#47)

### 0.6 (Released 20081215) ###

A significant release with new features and many issues closed.

EWItool moves to using the portable RtMidi library for all MIDI access which runs on Linux, Windows and MacOS X, it should now be a small step to get EWItool running on MacOS X.  A useful function to export a single patch to a .syx file has been added which should aid patch sharing and interoperability with other programs; naturally there is a complementary new function to send a .syx file to the EWI.

New Features:
  * Add "Export" action for saving an individual patch to a file to Patch and Clipboard menus - [Issue #29](https://code.google.com/p/ewitool/issues/detail?id=#29)
  * Move to the portable RtMidi MIDI library, remove ALSA and MCI code
  * Add new Send SysEx file function to EWI menu
  * Add statistics panel to EPX tab - part of [Issue #30](https://code.google.com/p/ewitool/issues/detail?id=#30)
  * Add confirmation dialog when Clear-ing and Delete-ing from the Clipboard - part of [Issue #27](https://code.google.com/p/ewitool/issues/detail?id=#27)
  * Add RtMidi licence text to COPYING file
  * Print current date when printing 'Current EWI Contents' - [Issue #32](https://code.google.com/p/ewitool/issues/detail?id=#32)

Fixes:
  * Reduce window size again - should fit on 1024x768 screens now - [Issue #34](https://code.google.com/p/ewitool/issues/detail?id=#34)
  * Rename confusing option on Patch menu: "Save As..." is now "Copy As..."
  * Remove redundant MIDI Panic action - [Issue #33](https://code.google.com/p/ewitool/issues/detail?id=#33)
  * Lock size of LCDNumbers on EWI Patch Set tab so they don't grow and take up too much space - [Issue #31](https://code.google.com/p/ewitool/issues/detail?id=#31)
  * Reduce creation of spurious EPX and midi\_data objects and delete genuine temporary ones when finished with
  * Abstract Clipboard into its own class - part of [Issue #11](https://code.google.com/p/ewitool/issues/detail?id=#11)
  * Create an ewi4000sPatch class - part of [Issue #11](https://code.google.com/p/ewitool/issues/detail?id=#11)
    * Eliminate abuse of midi\_data class for patch handling
  * Move all Patch Exchange GUI handling into the patchExchangeGUI class - part of [Issue #11](https://code.google.com/p/ewitool/issues/detail?id=#11)
  * Fix window title for clipboard|rename - part of [Issue #27](https://code.google.com/p/ewitool/issues/detail?id=#27)
  * Fix window title for Delete and Send to EWI on Set Library tab - part of [Issue #27](https://code.google.com/p/ewitool/issues/detail?id=#27)
  * Disable Clipboard buttons if no item selected/exist - part of [Issue #27](https://code.google.com/p/ewitool/issues/detail?id=#27)
  * Fix various behaviours when a Patch Set is deleted from the Library - part of [Issue #27](https://code.google.com/p/ewitool/issues/detail?id=#27)
  * Put GPL v.3 licence text in COPYING file

### 0.5 (Released 20080908) ###

A bug-fix release.  All users should upgrade.

Fixes:
  * Fix crash on send patch Set to EWI (thanks to Matthew Walton for spotting/diagnosing that) - [Issue #25](https://code.google.com/p/ewitool/issues/detail?id=#25)
  * Remove other unused properties from MainWindow relating to EWI tab and now implemented in EWIListWidget
  * Add 'Save' action to Patch menu so currently edited patch can be committed to EWI - [Issue #26](https://code.google.com/p/ewitool/issues/detail?id=#26)
  * Increase size and shrink font in Details/Description box on EPX tab - [Issue #19](https://code.google.com/p/ewitool/issues/detail?id=#19)
  * Remove old patch details when a patch is deleted in EPX - [Issue #20](https://code.google.com/p/ewitool/issues/detail?id=#20)

### 0.4 (Released 20080828) ###

This release features integration with an on-line EWI Patch Exchange - allowing sharing and storage of patches.

New Features:
  * EWI Patch Exchange integration - query via new tab and submit patches from Clipboard - [Issue #14](https://code.google.com/p/ewitool/issues/detail?id=#14)
  * Merge two patches via Patch->Process->Merge With... menu item - [Issue #7](https://code.google.com/p/ewitool/issues/detail?id=#7)
  * Settings dialog - [Issue #13](https://code.google.com/p/ewitool/issues/detail?id=#13)
  * Delete Patch Set - [Issue #15](https://code.google.com/p/ewitool/issues/detail?id=#15)
  * Print contents of any Patch Set from the Set Library tab - [Issue #17](https://code.google.com/p/ewitool/issues/detail?id=#17)

Fixes:
  * Improve Patch editing interface by moving all actions to a "Patch" menu item
  * Put appropriate title on printed patch set list
  * Reduce height of GUI a little more

### 0.3 (Released 20080801) ###

EWItool now provides about half a dozen more patch parameters than the proprietary editors and will now import most .bnk and .sqs files.  N.B. Slight change to install procedure for Windows - see InstallGuide for details.

New Features:
  * Add Delay Mix and Reverb Mix parameters and controls
  * Import 'partial' (<100 patch) .bnk files
  * Import .sqs files - [Issue #5](https://code.google.com/p/ewitool/issues/detail?id=#5)
  * Add Licence link to GPL v3 statement
  * Win32 downloads split differently so that the Qt DLLs only need to be downloaded if the version used in EWItool changes (saves download time/space/bandwidth)

Fixes:
  * Rename mystery '?' control group to Anti-Alias and correctly name its parameters (Enable, Cutoff Freq & Key Follow)
  * Improve usability of EWI Patch Set tab by using right-click context menus for the actions (also see [Issue #11](https://code.google.com/p/ewitool/issues/detail?id=#11))
  * Fix [Issue #2](https://code.google.com/p/ewitool/issues/detail?id=#2) - some .bnk files not importing
  * Fix [Issue #4](https://code.google.com/p/ewitool/issues/detail?id=#4) - crash on Paste if nothing selected
  * Tweak 'Totally Random' patch generator so more generated patches are audible
  * Improve About box and add link to Google Code page

### 0.2 (Released 20080721) ###

Main focus of this release is to provide a version that will run on 32-bit Windows platforms.  Only tested on XP so far.

Fixes:
  * Contextualise menus to prevent nonsensical user actions
  * Increase size of Hex Viewer dialog to show whole patch
  * Remove some unnecessary #includes
  * Disable (non-functioning) patch Export until a sensible output format is decided
  * Reduce height of GUI a little to fit on 1152x864 desktops
  * Fix Delay Damping control to allow full range

New Features:
  * Add Special actions to patch editor:
    * Default - a plain single triangle wave - unfiltered
    * Dry - turn off all reverb/chorus etc.
    * Noiseless - turn off noise generator
    * Randomise (10%) - randomly alter most parameters by up to 10%
    * Totally Random - generate a very(!) random patch
  * Show value (and interpretation) of adjustments in patch editor in the status bar
  * Win32 version! (Using standard Windows MCI)
    * midiportsdialog ported
    * midi\_data ported (createOurMIDIports (nop), scanPorts, connectInput, connectOutput, disconnectInput, disconnectOutput, requestPatch, win32MIDIinCallback, sendSysEx, sendCC..)
    * minor addition to MainWindow::fetchAllPatches
    * create a console in main.cpp to see messages if in verbose mode

### 0.1 (Released: 20080703) ###

  * Initial release