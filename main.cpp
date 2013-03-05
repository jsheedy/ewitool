/***************************************************************************
 *   Copyright (C) 2008 by Steve Merrony   *
 *   ewitool At merrony dot flyer dot co dot uk   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QApplication>

#include <iostream>
using namespace std;

#include "mainwindow.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif
				 
//#ifndef Q_WS_WIN
#include "midilistener.h"
//#endif
				 
#include "midi_data.h"
				 
int main(int argc, char *argv[])
{
	//Q_INIT_RESOURCE(application);

	midi_data *main_midi_data = new midi_data();
	QString argStr;
	QApplication app(argc, argv);
	
	// handle any arguments passed in
	if ( app.argc() > 1 ) {
		for (int arg = 1; arg<app.argc(); arg++) {
			argStr = app.argv()[arg];
			if (argStr.compare( "--help" ) == 0) {
				cout << "\nUsage: EWItool [options]\n\
Options:\n\
--help       This help\n\
--verbose    Be a lot more verbose in the console while EWItool is running\n\
\n\
For more information please visit http://code.google.com/p/ewitool/ \n";
				exit(0);
			}
			if (argStr.compare( "--verbose" ) == 0) { main_midi_data->verboseMode = TRUE; continue; }
		}
	}
#ifdef Q_WS_WIN
	// if we're in win32 and user specifed verbose, then open a console for the messages
	if (main_midi_data->verboseMode) {
		AllocConsole();
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
	}
#endif
	app.setQuitOnLastWindowClosed( true );
	MainWindow mw( main_midi_data );
	mw.show();

	MidiListener *mlThread = new MidiListener( (QObject *) main_midi_data );
	mlThread->start();
	
    const int retval = app.exec();
    mlThread->terminate();
    delete main_midi_data;
    return retval;
}
