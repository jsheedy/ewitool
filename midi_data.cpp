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

#include <QSettings>
#include <QString>

#include "midi_data.h"

midi_data::midi_data() {
	verboseMode = false;
	createOurMIDIports();
	last_patch_loaded = 0;
	connectedInPort = -1; connectedInClient = -1;
	connectedOutPort = -1; connectedOutClient = -1;
}


midi_data::~midi_data() {
}

void midi_data::createOurMIDIports() {
	int res;
	midi_port *ip, *op;

	midi_seq * seqp = &seq;
	ip = &inp_port;
	op = &out_port;

	// open the ALSA device
	res = snd_seq_open (&seqp->seq_handle, "hw", SND_SEQ_OPEN_INPUT|SND_SEQ_OPEN_OUTPUT, 0);
	if (verboseMode) cout << "midi_data: Seq name: " <<snd_seq_name( seq.seq_handle );
	snd_seq_set_client_name (seqp->seq_handle, "EWItool");
	// get a port
	seqp->my_client = snd_seq_client_id (seqp->seq_handle);

	// Create the ALSA output ports thru which EWItool sends all MIDI output

	// create a new ALSA midi output port
	op->my_port = snd_seq_create_simple_port (seqp->seq_handle, "EWItool Output",
	              SND_SEQ_PORT_CAP_SUBS_READ |
	              SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_READ,
	              SND_SEQ_PORT_TYPE_MIDI_GENERIC |
	              SND_SEQ_PORT_TYPE_APPLICATION);

	if (op->my_port < 0) {
		perror ("create output port");
		exit (1);
	}

	// Create the ALSA input port via which EWItool will receive MIDI
	ip->my_port = snd_seq_create_simple_port (seqp->seq_handle, "EWItool Input",
	              SND_SEQ_PORT_CAP_WRITE |
	              SND_SEQ_PORT_CAP_SUBS_WRITE
	              // SND_SEQ_PORT_CAP_READ,
	              ,
	              SND_SEQ_PORT_TYPE_MIDI_GENERIC |
	              SND_SEQ_PORT_TYPE_APPLICATION);

	if (op->my_port < 0) {
		perror ("create input port");
		exit (1);
	}

}

void midi_data::sendPanic() {

	snd_seq_event_t ev;

	int rc;

	for (int mc = 0; mc < 16; mc++) {
		snd_seq_ev_clear (&ev);
		snd_seq_ev_set_controller (&ev, mc, MIDI_CTL_ALL_NOTES_OFF,0);

		snd_seq_ev_set_subs (&ev);
		snd_seq_ev_set_direct (&ev);
		snd_seq_ev_set_source (&ev, out_port.my_port);

		rc = snd_seq_event_output_direct (seq.seq_handle, &ev);

		if (rc < 0) {
			cout << "Error: sending sequencer controller event (" << snd_strerror (rc) << ")\n";
		}
	}
}

bool midi_data::requestPatch (char p) {
	snd_seq_event_t ev;

	int rc;
	char sysex_fetch_patch[] = { 0xf0, 0x47, 0x64, 0x00, 0x40, 0x00, 0xf7 }; // 6th byte is patch #

	sysex_fetch_patch[5] = p;

	snd_seq_ev_clear (&ev);
	snd_seq_ev_set_sysex (&ev, 7, (void *) sysex_fetch_patch);

	// send request
	snd_seq_ev_set_subs (&ev);
	snd_seq_ev_set_direct (&ev);
	snd_seq_ev_set_source (&ev, out_port.my_port);

	rc = snd_seq_event_output (seq.seq_handle, &ev);

	if (rc < 0) {
		cout << "Error: sending SysEx request (" << snd_strerror (rc) << ")\n";
	}

	snd_seq_drain_output (seq.seq_handle);

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
	
	snd_seq_event_t ev;
	int rc;

	snd_seq_ev_clear (&ev);

	// send request
	snd_seq_ev_set_source (&ev, out_port.my_port);
	snd_seq_ev_set_subs (&ev);
	snd_seq_ev_set_direct (&ev);
	snd_seq_ev_set_controller (&ev, ch , cc, val);
	rc = snd_seq_event_output (seq.seq_handle, &ev);

	if (rc < 0) {
		cout << "Error: sending CC request (" << snd_strerror (rc) << ")\n";
	}

	snd_seq_drain_output (seq.seq_handle);
}


void midi_data::sendSysEx (char *sysex, int len) {

	snd_seq_event_t ev;
	int rc;

	snd_seq_ev_clear (&ev) ;
	//snd_seq_ev_set_sysex (&ev, len, (void *) sysex);

	// send request
	snd_seq_ev_set_source (&ev, out_port.my_port);
	snd_seq_ev_set_subs (&ev);
	snd_seq_ev_set_direct (&ev);
	snd_seq_ev_set_sysex (&ev, len, (void *) sysex);
	rc = snd_seq_event_output (seq.seq_handle, &ev);

	if (rc < 0) {
		cerr << "Error: sending SysEx  (" << snd_strerror (rc) << ")\n";
	}

	snd_seq_drain_output (seq.seq_handle);

	 usleep( 250000 );
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

	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;

	QString port_name;

	inPortList.clear();
	inPortClients.clear();
	inPortPorts.clear();
	outPortList.clear();
	outPortClients.clear();
	outPortPorts.clear();
	
	snd_seq_client_info_alloca (&cinfo);
	snd_seq_port_info_alloca (&pinfo);
	snd_seq_client_info_set_client (cinfo, -1);

	while (snd_seq_query_next_client (seq.seq_handle, cinfo) >= 0) {
		if (snd_seq_client_info_get_client (cinfo) != seq.my_client) {  // ignore our own port!
			/* reset query info */
			snd_seq_port_info_set_client (pinfo, snd_seq_client_info_get_client (cinfo));
			snd_seq_port_info_set_port (pinfo, -1);

			while (snd_seq_query_next_port (seq.seq_handle, pinfo) >= 0) {

				// Is this an output port we can send MIDI to?

				if ( ( (snd_seq_port_info_get_capability (pinfo) & (SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE))
				        == (SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE))
				        &&  // only interested in real MIDI devices
				        ( (snd_seq_port_info_get_type (pinfo) & (SND_SEQ_PORT_TYPE_MIDI_GENERIC)) == (SND_SEQ_PORT_TYPE_MIDI_GENERIC))
				   ) {

					port_name = QString ("%1:%2 %3").arg (snd_seq_client_info_get_client (cinfo)).arg (snd_seq_port_info_get_port (pinfo)).arg (snd_seq_port_info_get_name (pinfo));
					outPortList.append(port_name);
					outPortClients.append( snd_seq_client_info_get_client (cinfo) );
					outPortPorts.append( snd_seq_port_info_get_port(pinfo) );
				}

				// Is this an input port we can read MIDI from?

				if ( ( (snd_seq_port_info_get_capability (pinfo) & (SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ))
				        == (SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ))
				        &&  // only interested in real MIDI devices
				        ( (snd_seq_port_info_get_type (pinfo) & (SND_SEQ_PORT_TYPE_MIDI_GENERIC)) == (SND_SEQ_PORT_TYPE_MIDI_GENERIC))
				   ) {
					port_name = QString ("%1:%2 %3").arg (snd_seq_client_info_get_client (cinfo)).arg (snd_seq_port_info_get_port (pinfo)).arg (snd_seq_port_info_get_name (pinfo));
					inPortList.append(port_name);
					inPortClients.append( snd_seq_client_info_get_client (cinfo) );
					inPortPorts.append( snd_seq_port_info_get_port(pinfo) );
				}

			} // end while each port
		} // end-if
	} // end while each client
}

void midi_data::connectOutput( int out_client, int o_port ) {
	
	snd_seq_addr_t           sender, dest;
	snd_seq_port_subscribe_t *subs;
	
	int err;
	
	if (connectedOutPort != -1 ) disconnectOutput();
	
	sender.client = seq.my_client;
	sender.port = out_port.my_port;
	
	dest.client = out_client;
	dest.port = o_port;
	if (verboseMode) cout << "Trying to connect myself (" << seq.my_client << ":" <<out_port.my_port << ") to "<< out_client << ":" <<o_port << endl;
	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	err = snd_seq_subscribe_port(seq.seq_handle, subs);
	
	if (err < 0) {
		cout << "Cannot subscribe output MIDI port: " << snd_strerror(err) << endl;
	}
	
	connectedOutPort = o_port;
	connectedOutClient = out_client;
	
	QSettings settings( "EWItool", "EWItool" );
	settings.setValue( "MIDI/OutPort", o_port );
	settings.setValue( "MIDI/OutClient", out_client );
}

void midi_data::connectInput( int in_client, int i_port ) {
	
	snd_seq_addr_t           sender, dest;
	snd_seq_port_subscribe_t *subs;
	
	int err;
	
	if (connectedInPort != -1) disconnectInput();
	
	sender.client = in_client;
	sender.port = i_port;
	
	dest.client = seq.my_client;
	dest.port = inp_port.my_port;
	if (verboseMode) cout << "Trying to connect myself (" << seq.my_client << ":" <<inp_port.my_port << ") to "<< in_client << ":" <<i_port << endl;
	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);
	snd_seq_port_subscribe_set_queue(subs, 1);
	snd_seq_port_subscribe_set_time_update(subs, 1);
	snd_seq_port_subscribe_set_time_real(subs, 1);
	err = snd_seq_subscribe_port(seq.seq_handle, subs);
	
	if (err < 0) {
		cout << "Cannot subscribe input MIDI port: " << snd_strerror(err) << endl;
	}
	
	connectedInPort = i_port;
	connectedInClient = in_client;
	
	QSettings settings( "EWItool", "EWItool" );
	settings.setValue( "MIDI/InPort", i_port );
	settings.setValue( "MIDI/InClient", in_client );
}

void midi_data::disconnectInput() {
	
	snd_seq_port_subscribe_t *pAlsaSubs;
	snd_seq_addr_t seq_addr;
    int err;
	
	snd_seq_port_subscribe_alloca(&pAlsaSubs);

	seq_addr.client = connectedInClient;
	seq_addr.port   = connectedInPort;
	snd_seq_port_subscribe_set_sender(pAlsaSubs, &seq_addr);

	seq_addr.client = seq.my_client;
	seq_addr.port   = inp_port.my_port;
	snd_seq_port_subscribe_set_dest(pAlsaSubs, &seq_addr);

	err = snd_seq_unsubscribe_port( seq.seq_handle, pAlsaSubs);
	if (err < 0) {
		cerr << "Cannot unsubscribe input MIDI port: " << snd_strerror(err) << endl;
	}
}

void midi_data::disconnectOutput() {
	
	snd_seq_port_subscribe_t *pAlsaSubs;
	snd_seq_addr_t seq_addr;
	int err;
	
	snd_seq_port_subscribe_alloca(&pAlsaSubs);

	seq_addr.client = seq.my_client;
	seq_addr.port   = out_port.my_port;
	snd_seq_port_subscribe_set_sender(pAlsaSubs, &seq_addr);

	seq_addr.client = connectedOutClient;
	seq_addr.port   = connectedOutPort;
	snd_seq_port_subscribe_set_dest(pAlsaSubs, &seq_addr);

	err = snd_seq_unsubscribe_port( seq.seq_handle, pAlsaSubs);
	if (err < 0) {
		cerr << "Cannot unsubscribe output MIDI port: " << snd_strerror(err) << endl;
	}
}

QString midi_data::getPatchName( char *rawName ) {
	// utility function to get a nicely formatted patch name
	QString sname = rawName;
	sname.truncate( EWI_PATCHNAME_LENGTH );
	return sname.trimmed();
}

