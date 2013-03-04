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

#include <iostream>
#include <signal.h>

using namespace std;


#include "midilistener.h"
#include "midi_data.h"
		
// Platform-dependent sleep routines.
#if defined(__WINDOWS_MM__)
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif		 
				 
				 
MidiListener::MidiListener( QObject *parent )
{
	// midi_data_shr = shared_data;
	tmidi_data = (midi_data *)parent;
	
	if (tmidi_data->verboseMode) cout << "MIDIListener thread created\n";
}

MidiListener::~MidiListener()
{
	if (tmidi_data->verboseMode) cout << "MIDI Listener thread destroyed\n";
}

void MidiListener::run() {

	vector<unsigned char> message;
	int nBytes;
	double stamp;
	RtMidiIn *midiIn =  tmidi_data->midiIn;

	int this_patch_num;

	if ( tmidi_data->verboseMode ) cout << "MidiListener thread running\n";

	// Install an interrupt handler function
	//(void) signal(SIGINT, finish);

	while ( true ) {

		stamp = midiIn->getMessage( &message );
		nBytes = message.size();

		if ( nBytes > 0 ) {

			if ( message[0] == MIDI_SYSEX_HEADER ) {
              // if ( tmidi_data->verboseMode ) cout << "MidiListener: SysEx response received " << nBytes << " bytes\n";
              switch (message[4]) {
                case MIDI_PRESET_DUMP:            
				  if ( tmidi_data->verboseMode ) cout << "MidiListener: SysEx Preset Dump response received " << nBytes << " bytes\n";

				  patch_t this_patch;

                  if (nBytes != EWI_SYSEX_PRESET_DUMP_LEN) {
                    cout << "Invalid Preset Dump received from EWI (" << nBytes << " bytes)\n";
                    break;
                  }
                  
				  for ( int i = 0; i < EWI_PATCH_LENGTH; i++ ) this_patch.whole_patch[i] = message[i];

                  if ( this_patch.parameters.header[3] == MIDI_SYSEX_ALLCHANNELS ) {
					  this_patch_num = ( int ) this_patch.parameters.patch_num++;
                      if (this_patch_num < 0 || this_patch_num >= EWI_NUM_PATCHES) {
                        cout << "Illegal patch number recieved from EWI - " << this_patch_num << "\n";
                        break;                  
                      }               
					  for ( int i = 0; i < EWI_PATCH_LENGTH; i++ ) tmidi_data->patches[this_patch_num].whole_patch[i] = message[i];
					  tmidi_data->last_patch_loaded = this_patch_num;

					  if ( tmidi_data->verboseMode ) cout << "MidiListener: Received " << this_patch_num + 1 << " - " << this_patch.parameters.name << "\n";
				  }
                  break;
                case MIDI_QUICKPC_DUMP:
                  if ( tmidi_data->verboseMode ) cout << "MidiListener: SysEx QuickPC Dump response received " << nBytes << " bytes\n";
                  if (nBytes != EWI_SYSEX_QUICKPC_DUMP_LEN) {
                    cout << "Invalid QuickPC Dump received from EWI (" << nBytes << " bytes)\n";
                    break;
                  }
                  for (int i = 0; i < EWI_NUM_QUICKPCS; i++) {
                    tmidi_data->quickPCs[i] = message[i+6];
                  }
                  break;
                default:
                   cerr << "MidiListener: Unrecognised SysEx type received (type " << message[4] <<")\n";
                  break;
              }            
			  tmidi_data->mymutex.lock();
              tmidi_data->sysexDone.wakeAll();
			  tmidi_data->mymutex.unlock();
              
            }
              // All non-SysEx messages are ignored
		}

		SLEEP( 10 );
	}
}







