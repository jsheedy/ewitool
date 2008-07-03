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
#include <iostream>
using namespace std;

#include "midiportsdialog.h"

MIDIports_Dialog::MIDIports_Dialog (midi_data *m) : QDialog() {
	
	setupUi (this);
    
	mididata = m;
	
	m->scanPorts();
	inPorts_listWidget->addItems( m->inPortList );
	outPorts_listWidget->addItems( m->outPortList );
	
	// highlight the currently selected ports
	for ( int i = 0; i < inPorts_listWidget->count(); i++ ) {
		if (inPorts_listWidget->item(i)->text().startsWith( QString( "%1:%2" ).arg( m->connectedInClient ).arg( m->connectedInPort ) ) ) {
			inPorts_listWidget->setCurrentRow( i );
		}
	}
	for ( int i = 0; i < outPorts_listWidget->count(); i++ ) {
		if (outPorts_listWidget->item(i)->text().startsWith( QString( "%1:%2" ).arg( m->connectedOutClient ).arg( m->connectedOutPort ) ) ) {
			outPorts_listWidget->setCurrentRow( i );
		}
	}
	
	connect( inPorts_listWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT(inPortSelected( int)) );
	connect( outPorts_listWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT(outPortSelected( int)) );
}

/*
MIDIports_Dialog::~MIDIports_Dialog()
{
}
*/

void MIDIports_Dialog::inPortSelected( int r ) {
	mididata->connectInput( mididata->inPortClients.at( r ), mididata->inPortPorts.at( r ) );
}

void MIDIports_Dialog::outPortSelected( int r ) {
	mididata->connectOutput( mididata->outPortClients.at( r ), mididata->outPortPorts.at( r ) );
}


