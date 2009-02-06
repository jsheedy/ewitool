/***************************************************************************
 *   Copyright (C) 2008 by Steve Merrony                                   *
 *      *
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
#ifndef MIDI_DATA_H
#define MIDI_DATA_H

#include "RtMidi.h"

#include <QMutex>
#include <QStringList>
#include <QWaitCondition>

#include "ewi4000spatch.h"

const QString	LIBRARY_EXTENSION	= ".syx";
const int	MAX_SYSEX_LENGTH		= 262144;
const int	EWI_SOUNDBANK_MAX_HEADER_LENGTH = 0x450;	// looks safe from observation
// below is the sequence which appears to start the real body of SQS files, there
// seem to be various false BODYs before the main one we care about
const char	EWI_SQS_BODY_START[]	= { 'B', 'O', 'D', 'Y', 0x00, 0x00, 0x50, 0x78, 0xf0, 0x47, 0x64, 0x7f, 0x00, 0x00 };
const int	EWI_SQS_MAX_HEADER_LENGTH = 0x1000;			// looks ok from observation

/**
	@author Steve Merrony
 */

class midi_data{
	
public:
    midi_data();
    ~midi_data();
	
	void createOurMIDIports();
	void sendPanic();
	bool requestPatch( char );
	void sendLiveControl(int, int, int );
	void sendCC( int, int, int = 0 );
	void sendSysEx(char *, int );
	void sendSysExFile( QString fileName );
	void sendPatch( patch_t, char = EWI_SAVE );
	
	void scanPorts();
	void connectOutput( int );
	void connectInput( int );
	void disconnectInput();
	void disconnectOutput();
	
	bool    verboseMode;
	
	QMutex  mymutex;
	QWaitCondition sysexDone;
	
	int			last_patch_loaded;
	patch_t 	patches[EWI_NUM_PATCHES];
	
	QStringList inPortList, outPortList;
	QList<int>  inPortPorts, outPortPorts;
	int         connectedInPort;
	int         connectedOutPort;
	RtMidiIn	*midiIn;
	RtMidiOut	*midiOut;
};

#endif
