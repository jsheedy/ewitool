# Introduction #

EWItool is distributed as a source archive  which can built on either Linux or Windows. Here are some simple instructions to get you started.

You will need the Qt 4 development libraries installed before you begin.  On the Windows platform you will also need MinGW installed and the 'zlib' package which provides the unarchive command used in step one below.

## Step 1 - Unpack the archive ##

`tar xzvf EWItool-0.1.tar.gz`

(On Windows the command might be `bsdtar` rather than `tar`.)

This will unpack the source archive into a subdirectory of you current directory called 'EWItool', so next you  should:

`cd EWItool`

## Step 2 - Run qmake ##

`qmake`

If you only have Qt version 4 installed simply run the `qmake` command.  If you have versions 3 and 4 installed you will have to invoke the appropriate qmake for version 4.  On my (Debian-based) system I type `qmake-qt4` .

## Step 3 - Make EWItool ##

`make`

If you have multiple cores on your machine you can speed things up with the `-j` option.  On my quad-core machine I type `make -j 4`.

You should not get errors when building EWItool, if you do, please visit the Issues page.

## Step 4 - Running EWItool ##

The program has been built in the `bin` subdirectory, so you can type:

`./bin/ewitool`

There is a `--help` option if you really must!

Have fun and please provide feedback.