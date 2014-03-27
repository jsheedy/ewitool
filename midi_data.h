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
#include "ewi4000sQuickPC.h"

// SysEx commands from Akai
#define MIDI_PRESET_DUMP      0x00
#define MIDI_PRESET_DUMP_REQ  0x40
#define MIDI_QUICKPC_DUMP     0x01
#define MIDI_QUICKPC_DUMP_REQ 0x41
#define MIDI_EDIT_LOAD        0x10
#define MIDI_EDIT_STORE       0x11
#define MIDI_EDIT_DUMP        0x20
#define MIDI_EDIT_DUMP_REQ    0x60

#define MIDI_SYSEX_HEADER     0xf0
#define MIDI_SYSEX_TRAILER    0xf7
#define MIDI_SYSEX_AKAI_ID    0x47
#define MIDI_SYSEX_AKAI_EWI4K 0x64
#define MIDI_SYSEX_CHANNEL    0x00
#define MIDI_SYSEX_ALLCHANNELS 0x7f

#define MIDI_CC_DATA_ENTRY    0x06
#define MIDI_CC_NRPN_LSB      0x62
#define MIDI_CC_NRPN_MSB      0x63

const QString	LIBRARY_EXTENSION	= ".syx";
const int	MAX_SYSEX_LENGTH		= 262144;
const int   EWI_SYSEX_PRESET_DUMP_LEN = 206;
const int   EWI_SYSEX_QUICKPC_DUMP_LEN = 91;
const int   MIDI_TIMEOUT_MS         = 3000;
const int	EWI_SOUNDBANK_MAX_HEADER_LENGTH = 0x450;	// looks safe from observation
// below is the sequence which appears to start the real body of SQS files, there
// seem to be various false BODYs before the main one we care about
//const char	EWI_SQS_BODY_START[]	= { 'B', 'O', 'D', 'Y', 0x00, 0x00, 0x50, 0x78, 0xf0, 0x47, 0x64, 0x7f, 0x00, 0x00 };
const char	EWI_SQS_BODY_START[]	= { 'B', 'O', 'D', 'Y', 0, 0, '\x50', '\x78', '\xf0', '\x47', '\x64', '\x7f', 0, 0 };
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
	bool requestPatch( unsigned char );
    bool requestQuickPCs();
    bool sendQuickPCs();
	void sendLiveControl(int, int, int );
	void sendCC( int, int, int = 0 );
	void sendSysExFile( QString fileName );
	void sendPatch( patch_t, unsigned char = EWI_SAVE );
	
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
    all_quickpcs_t quickPCs;   
	
	QStringList inPortList, outPortList;
	QList<int>  inPortPorts, outPortPorts;
	int         connectedInPort;
	int         connectedOutPort;
	RtMidiIn	*midiIn;
	RtMidiOut	*midiOut;
    
  private:
    void sendSysEx(char *, int );
};

#endif
