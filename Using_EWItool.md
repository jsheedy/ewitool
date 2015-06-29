This page describes basic use of the EWItool.  Detailed patch editing might get discussed on another page at some point...  You should probably also take a quick look at [KeyDifferences](KeyDifferences.md) to see how EWItool differs from other patch editors.


# Start EWItool #

You may start EWItool either from a shell window, or from your desktop, by invoking the `ewitool` binary that you created by following the steps on the [Building\_EWItool](Building_EWItool.md) page.

You can get EWItool to tell you a lot of the gory details of what it's up to by adding the `--verbose` switch if you start it from a shell.  I suggest you don't normally bother with that unless you are helping chase down a bug.

## First Run ##

The first time you run EWItool (or if you have deleted its settings file) it will ask you what Library Directory to use.  This will be where EWItool stores your patch libraries, so you should probably start off by specifying an empty directory.

The next thing to do is to choose the MIDI ports you are going to use to connect to the EWI;     go to the **EWI** menu and select **MIDI Connections** - that should be self-explanatory.

Now you are ready to get going.

# EWItool Terms and Concepts #

A few things you might like to know before using the tool...

### The Clipboard ###

This holds any number of patches ready for (individual) insertion into the active EWI Patch Set, renaming, swapping with other via the Patch Exchange, exporting to a file or viewing in hex.  The clipboard is held on disk and saved every time it changes so you can resume work wherever you left off last session.  Typical usage would be to copy patches from several of your patch libraries or the EWI Patch Set into the clipboard, then paste them into the EWI patch set when you are happy with the collection.

### Set Library ###

The library contains all your Patch Sets.

Each patch contains one complete EWI Patch Set and is stored on disk in the location you specified at first run (see above).  The set are stored in raw SysEx format (.syx) which I believe some other EWI software can use(!).  From the **File** menu you can **Import** patch collections (a.k.a. 'soundbanks') in the EWI-specific `.bnk` or `.sqs` formats as a new library.

### EWI Patch Set ###

This represents the entire contents of your EWI.  Because it lives in the EWI there is no concept of a name for this set and it will never be empty unless your EWI contains no sounds.

# Typical Usage #

Generally, the first thing you will do is go to the **EWI** menu and select **Fetch All Patches**.  EWItool will attempt to load all patches from your EWI and you should see a progress bar to show how things are going.  On the author's system it takes about 7 seconds - this is mainly limited by the standard MIDI bandwidth so your transfer should take about the same time.

Now the EWI Patch Set is loaded, **right-click** on the patch you are going to work on and choose **Edit** from the pop-up menu.  At this point EWItool loads the patch into the EWI's (transient) edit buffer and switches to the the **Current Patch** tab.

Changes you make on the editor will now be sent to the EWI as you make them, and most take immediate effect - some will require you to retrigger (i.e. tongue) a note.

At any point during the editing you can hit the **Revert** button to go back to the original settings.

Similarly, at any stage you can **Copy** your patch onto the clipboard - where it will stay in its current state until you delete it.

Patches in the Clipboard can be **Pasted** into the EWI Patch Set by **right-clicking** on a button in the Set and choosing **Paste** from the menu that appears.  Then you can right click again and choose **Edit** to work on the patch.

Finally - once you are happy with your altered sound, you can **Save** it back to the EWI.  This overwrites the stored patch in the EWI with your new sound.

Once you have built up a new set of patches on the **EWI Patch Set** tab, you can create a new library by choosing **Save As...** from the **File** menu.  You will be prompted to provide a name for the new library which will then be stored on disk.

# Menu Items #
## File ##
### Import ###
The import command will try to import .bnk or .sqs files into a EWItool's native .syx format.  If the file you try to import contains only a partial set (i.e. fewer than 100 patches) then the patches found are put onto the Clipboard instead of being saved to a .syx file.
### Print ###
The print command will behave differently depending upon which tab you are working on when you invoke it.

From the **EWI Patch Set** page a summary of the complete patch set (numbers and names) will be generated which should fit neatly on half a page.

From the **Current Patch** tab a direct copy of the graphical editor will be printed.

## EWI ##
### Fetch All Patches ###
This connect to the EWI and copies all the patches from the instrument into EWItool.
### MIDI Connections ###
### Send SysEx File ###
This is intended for one purpose only: to send a .syx file containing a **single** patch to the EWI.  Please do not try to use this for other purposes.
## Patch ##
This menu is only available when you have a patch loaded into the EWI for editing.
### Save ###
This will save the current (edited) patch into the EWI and into the EWI Patch Set.  It doesn't get written to disk unless you export it, put it on the Clipboard, or save the entire EWI Patch Set.
### Copy As ###
Put the current (edited) patch onto the Clipboard with a new name.
### Export ###
Create a .syx file in the /export subdirectory of your Set Library which contains the MIDI instructions to load the current patch into slot 99 of the EWI4000s.  Other programs (such as MIDI-Ox) should be able to load these .syx files into the EWI too.
### Revert ###
Cancel all edits and go back to the patch as it was when loaded from the EWI.
### Generate ###
Make a new sound...
  * Default Patch - a very simple tone
  * Random Patch - a very random sound.  (Some sounds may not be playable.)
### Process ###
Apply operations to the current patch...
  * Make Dry - remove all chorus, delay and reverb
  * Remove Noise - remove all white noise
  * Randomise (10%) - randomly adjust most parameters by up to 10%
  * Merge With... - mix the current patch 50:50 with another patch you select from the EWI Patch Set
## Help ##
### EWItool Help ###
Choosing this item will cause your computer to try to load **this** help page in your default web browser.