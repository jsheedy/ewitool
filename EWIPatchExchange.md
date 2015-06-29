# Introduction #

The EWI Patch Exchange (EPX) is an online patch repository which users of EWItool can use to store and swap patches.  Access to EPX has been a feature of EWItool since version 0.4.  You can use EWItool without EPX if you like.

## Fair Use and Terms ##

EPX is provided freely on a best-endeavours basis - there are no guarantees!  Please join our [Google Group](http://groups.google.co.uk/group/ewitool-users?hl=en) if you are going to use EPX (we might require this at some point in the future).  We will try to notify any planned downtime in advance on the Google Group.  DO NOT publicly share any commercial patches on the EPX or ask other people to do this.  Do not share your userid and password with others.  Failure to behave reasonably will result in your EPX account being deleted.


# Getting Access #

To access EPX you will need a userid, password and the server address.

To choose a userid and obtain a password visit http://stephenmerrony.co.uk/EPX/

The current server address is `stephenmerrony.co.uk`

Once you have the required information go to File->Settings in EWItool and enter the information in the appropriate boxes.  Hit the Test button to verify your settings.  The test actually consists of two parts: the first checks to see if the EPX host is accessible from your machine, then the second checks your userid and password on EPX.  Your settings will be saved if you 'OK' out of the Settings window.

# Using EPX #

After you have set up the EPX info as described above EWItool will try to access the EPX whenever you start the program; if successful, the Patch Exchange tab will be enabled - this tab is where you get patches out of EPX.  To put patches into EPX you first put them onto your EWItool Clipboard, then use the Exchange button on the 'Set Library' tab (more details below).

## Querying EPX ##

From the 'Patch Exchange' tab you can query the EPX database.  Hopefully the Query form is self-explanatory.  Tags are keywords that a contributor can assign to a patch when it is submitted.

![http://stephenmerrony.co.uk/public/EWItool/PatchExchangeTab.png](http://stephenmerrony.co.uk/public/EWItool/PatchExchangeTab.png)

Once you run the query, the results should appear in the middle section.  The format of the results is "<patch name> - 

&lt;origin&gt;

".  Click on any of the results to see the full details on the right-hand of the page.

When you have details of a patch showing on the right you can hit 'Copy' to place it on the EWItool Clipboard.  If you submitted the patch, this is also where you can delete it  - but why would you? :-)

## Submitting Patches to EPX ##

To submit a patch you must first put it on your EWItool Clipboard, then from the 'Set Library' tab select the patch and hit the 'Exchange' button.

A form will pop up and you should fill in as much of it as you can.  The first two fields are compulsory.

![http://stephenmerrony.co.uk/public/EWItool/SubmitToPatchExchange.png](http://stephenmerrony.co.uk/public/EWItool/SubmitToPatchExchange.png)

**Origin** - put a sensible value in the Origin, good examples are 'Akai', 'Jo Bloggs' - please don't put in things like 'My Own' which won't mean anything to anyone else.

**Type** - please choose the most appropriate value.  If you have other suggestions please let us know via the Google Group.

**Private** - if you want to put a patch on the exchange for your own private purposes mark it as 'private'.  Others will not be able to see patches which are marked private.

**Description** - put a meaningful sentence or two here, maybe including performance notes.

**Tags** - if you can think of any keywords relevant to the patch you can put them in here.  Separate them with spaces.

To prevent duplication in the Exchange you cannot submit the same patch twice, nor can you submit a patch with the same name and origin as one already in the Exchange.