/***************************************************************************
 *   Copyright (C) 2008 by Steve Merrony   *
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
#include <cstdlib>
#include <iostream>
using namespace std;

#include <QDataStream>
#include <QFile>
#include <QSettings>
#include <QString>

#include "midi_data.h"
				 
// Platform-dependent sleep routines.
#if defined(__WINDOWS_MM__)
 #include <windows.h>
 #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
 #include <unistd.h>
 #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif
				 
midi_data::midi_data() {
	verboseMode = false;
	createOurMIDIports();
	last_patch_loaded = -1;
	connectedInPort = -1; 
	connectedOutPort = -1;
}


midi_data::~midi_data() {
	
	delete midiIn;
	delete midiOut;
}

void midi_data::createOurMIDIports() {
	
	midiIn = 0;
	midiOut = 0;
	
	try {
		midiIn = new RtMidiIn();
	}
	catch ( RtError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}
	
	// Do(n't) ignore sysex, timing, or active sensing messages.
	midiIn->ignoreTypes( false, true, true );
	
	
	try {
		midiOut = new RtMidiOut();
	}
	catch ( RtError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}

}

void midi_data::sendPanic() {

	for (int mc = 0; mc < 16; mc++) {
		
		//snd_seq_ev_set_controller (&ev, mc, MIDI_CTL_ALL_NOTES_OFF,0);

	}
}

/**
 * Requests a single patch from the EWI and waits for one to be returned.
 * @param p 
 * @return 
 */
bool midi_data::requestPatch (char p) {

	std::vector<unsigned char> message;
	
	message.push_back( 0xf0 );
	message.push_back( 0x47 );
	message.push_back( 0x64 );
	message.push_back( 0x00 );
	message.push_back( 0x40 );
	message.push_back( p );		// 6th byte is patch #
	message.push_back( 0xf7 );
	
	//char sysex_fetch_patch[] = { 0xf0, 0x47, 0x64, 0x00, 0x40, 0x00, 0xf7 }; // 6th byte is patch #
	
	try {
		midiOut->sendMessage( &message );
	}
	catch ( RtError &error ) {
		error.printMessage();
	}
	
	mymutex.lock();  // wait for a SysEx to be returned
	if (!sysexDone.wait( &mymutex, 3000 )) {  // up to 3 secs
		// we timed out
		cout << "Timeout waiting for response from EWI\n";	
		mymutex.unlock();
		return false;
	}
	mymutex.unlock();

	// it seems the EWI can get out of sync - so retry if we didn't ge the patch back we asked for
	if (last_patch_loaded != (int) p) requestPatch( p );
	
	return true;
}

void midi_data::sendLiveControl (int lsb, int msb, int cvalue) {
	sendCC (98, lsb);
	sendCC (99, msb);
	sendCC (6, cvalue);
	sendCC (98, 127);
	sendCC (99, 127);
}

void midi_data::sendCC (int cc, int val, int ch) {
	
	std::vector<unsigned char> message;

	message.push_back( 176+ch );
	message.push_back( cc );
	message.push_back( val );
	
	try {
		midiOut->sendMessage( &message );
	}
	catch ( RtError &error ) {
		error.printMessage();
	}
}


/**
 * Currently only use by sendPatch()
 * @param sysex 
 * @param len 
 */
void midi_data::sendSysEx (char *sysex, int len) {
	
	vector<unsigned char> message;
	
	for (int i = 0; i<len; i++) {
		// int h = sysex[i];
		message.push_back( sysex[i] );
		// cout <<dec<< i << " : "; cout << hex << h << endl;
	}
	
	try {
		midiOut->sendMessage( &message );
	}
	catch ( RtError &error ) {
		error.printMessage();
	}
	
	SLEEP( 250 );
}

void midi_data::sendSysExFile( QString fileName ) {
	
	//  N.B. RtMidi assumes only 1 sysex per message.  We should check and break up files
	//       containing multiple sysexes here...
	
	int nbytes;
	char sysex[MAX_SYSEX_LENGTH];
	QFile sysex_file( fileName );
	sysex_file.open( QIODevice::ReadOnly );
	QDataStream sysex_data( &sysex_file );
	
	nbytes = sysex_data.readRawData( &sysex[0], MAX_SYSEX_LENGTH );
	
	sendSysEx( &sysex[0], nbytes );
	
}

void midi_data::sendPatch (patch_t p, char mode) {
	
	p.parameters.mode = mode;

	sendSysEx( &p.whole_patch[0], EWI_PATCH_LENGTH );
	
	if (mode == EWI_EDIT) {
		// if we're going to edit we need to send it again as patch 0 with the 
		// edit flag set...
		p.parameters.patch_num = 0x00;
		sendSysEx( &p.whole_patch[0], EWI_PATCH_LENGTH );
	}
}

void midi_data::scanPorts() {
	
	QString port_name;

	inPortList.clear();
	inPortPorts.clear();
	outPortList.clear();
	outPortPorts.clear();
	
	// Check inputs.
	unsigned int nPorts = midiIn->getPortCount();
	std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
	std::string portName;
	for ( unsigned int i=0; i<nPorts; i++ ) {
		try {
			portName = midiIn->getPortName(i);
		}
		catch ( RtError &error ) {
			error.printMessage();
			//goto cleanup;
		}
		//std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
		inPortPorts.append( i );
		inPortList.append( QString::number( i ) + " : " + QString::fromStdString( portName ) );
	}
	
	// Check outputs.
	nPorts = midiOut->getPortCount();
	std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
	for ( unsigned int i=0; i<nPorts; i++ ) {
		try {
			portName = midiOut->getPortName(i);
		}
		catch (RtError &error) {
			error.printMessage();
			//goto cleanup;
		}
		//std::cout << "  Output Port #" << i+1 << ": " << portName << '\n';
		outPortPorts.append( i );
		outPortList.append( QString::number( i ) + " : " + QString::fromStdString( portName ) );
	}

}

void midi_data::connectOutput( int o_port ) {
	
	if (connectedOutPort != -1 ) disconnectOutput();
	try {
		midiOut->openPort( o_port );
	}
	catch (RtError &error) {
		error.printMessage();
			//goto cleanup;
	}
	connectedOutPort = o_port;
	if (verboseMode) cout << "Connected to MIDI output: " << o_port << endl;
	QSettings settings( "EWItool", "EWItool" );
	settings.setValue( "MIDI/OutPort", o_port );
}

void midi_data::connectInput( int i_port ) {
	
	if (connectedInPort != -1) disconnectInput();
	try {
		midiIn->openPort( i_port );
	}
	catch (RtError &error) {
		error.printMessage();
			//goto cleanup;
	}
	
	connectedInPort = i_port;
	if (verboseMode) cout << "Connected to MIDI input: " << i_port << endl;
	QSettings settings( "EWItool", "EWItool" );
	settings.setValue( "MIDI/InPort", i_port );
	last_patch_loaded = -1;
}

void midi_data::disconnectInput() {
	
	midiIn->closePort();
	
	connectedInPort = -1;
}

void midi_data::disconnectOutput() {
	
	midiOut->closePort();
	
	connectedOutPort = -1;
}




