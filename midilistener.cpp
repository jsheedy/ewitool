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
using namespace std;


#include "midilistener.h"
#include "midi_data.h"
				 
MidiListener::MidiListener( QObject *parent )
{
	// midi_data_shr = shared_data;
	tmidi_data = (midi_data *)parent;
	
	if (tmidi_data->verboseMode) cout << "MIDI Listener thread created\n";
}

MidiListener::~MidiListener()
{
	if (tmidi_data->verboseMode) cout << "MIDI Listener thread destroyed\n";
}

void MidiListener::run() {
		

	
	midi_seq seq;
	midi_port ip;
	
	if (tmidi_data->verboseMode) cout << "MidiListener thread running\n";
	
	//seq = &(tmidi_data->seq);
	seq = tmidi_data->seq;
	ip =  tmidi_data->inp_port;
	
	//createMidiInPort( seq, ip );
	
	snd_seq_event_t *ev;
	
	int this_patch_num;
	if (tmidi_data->verboseMode) cout << "MIDIlistener: Seq name: " <<snd_seq_name( seq.seq_handle );
	while (true) {

		if (snd_seq_event_input( seq.seq_handle, &ev ) < 0) {
			cout << "Error fetching MIDI event - data may have been lost\n";
		}
		
		switch ( ev->type )
		{
		case SND_SEQ_EVENT_SYSEX:
			if (tmidi_data->verboseMode) cout << "MidiListener: SysEx response received " << ev->data.ext.len << " bytes\n";
			patch_t this_patch;
			memcpy ( this_patch.whole_patch, ( char * ) ev->data.ext.ptr, EWI_PATCH_LENGTH );
			if (this_patch.parameters.header[3] == 0x7f ) {
				this_patch_num = ( int ) this_patch.parameters.patch_num++; 
				memcpy ( tmidi_data->patches[this_patch_num].whole_patch, ( char * ) ev->data.ext.ptr, EWI_PATCH_LENGTH );
				tmidi_data->last_patch_loaded = this_patch_num;
				if (tmidi_data->verboseMode) cout << "MidiListener: Received " << this_patch_num+1 << " - " << this_patch.parameters.name << "\n";
			}
			tmidi_data->mymutex.lock();
			tmidi_data->sysexDone.wakeAll();
			tmidi_data->mymutex.unlock();
			break;
			
		// Everything else is thrown away
		default: 
			//if (tmidi_data->verboseMode) cout << "MidiListener: Ignoring event type: " << (int) ev->type <<"\n";
			;
		} // end-switch
		
		snd_seq_free_event( ev );
	}
	

}







