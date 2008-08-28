/***************************************************************************
 *   Copyright (C) 2008 by Steve Merrony   *
 *   steve@brahma   *
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
#include "settings_dialog.h"

#include <QMessageBox>

settings_dialog::settings_dialog() : QDialog()
{
	setupUi( this );
	
	settings = new QSettings( "EWItool", "EWItool" );
	libraryLocation_lineEdit->setText( settings->value( "library/location" ).toString() );
	if (settings->contains( "PatchExchange/UserID" ))
		pxUserID_lineEdit->setText( settings->value( "PatchExchange/UserID" ).toString() );
	if (settings->contains( "PatchExchange/Password" ))
		pxPasswrd_lineEdit->setText( settings->value( "PatchExchange/Password" ).toString() );
	if (settings->contains( "PatchExchange/Server" ))
		pxServer_lineEdit->setText( settings->value( "PatchExchange/Server" ).toString() );
	
	px = new patchExchange( this );	// the 'this' is critical...
	
	connect( clear_pushButton, SIGNAL(clicked()), this, SLOT( clearSettings() ) );
	connect( test_pushButton, SIGNAL(clicked()), this, SLOT( testEPX() ) );
	connect( px, SIGNAL(connectionState(QString)), this, SLOT( updateConnectionState( QString ) ) );
	connect( px, SIGNAL(loginState(QString)), this, SLOT( updateConnectionState( QString ) ) );
}


settings_dialog::~settings_dialog()
{
	delete px;
}

void settings_dialog::clearSettings() {
	
	if (QMessageBox::question( this, "EWItool - Settings",
		tr( "This will remove all settings for EWItool, including the MIDI port settings.\n\n"
			"Do you really want to do this?" ),
		QMessageBox::No | QMessageBox::Yes) == QMessageBox::No) return;
	
	settings->clear();
	QDialog::done( true );
}

void settings_dialog::accept() {
	
	bool changes = false;
	
	if (settings->value( "library/location" ).toString() != libraryLocation_lineEdit->text() ) {
		changes = true;
		settings->setValue( "library/location", libraryLocation_lineEdit->text() );
	}
	if (settings->value( "PatchExchange/UserID" ).toString() != pxUserID_lineEdit->text() ) {
		changes = true;
		settings->setValue( "PatchExchange/UserID", pxUserID_lineEdit->text() );
	}
	if (settings->value( "PatchExchange/Password" ).toString() != pxPasswrd_lineEdit->text() ) {
		changes = true;
		settings->setValue( "PatchExchange/Password", pxPasswrd_lineEdit->text() );
	}
	if (settings->value( "PatchExchange/Server" ).toString() != pxServer_lineEdit->text() ) {
		changes = true;
		settings->setValue( "PatchExchange/Server", pxServer_lineEdit->text() );
	}
	
	QDialog::done( changes == true );
}

void settings_dialog::testEPX() {
	px->testConnection( pxServer_lineEdit->text() );
	px->testUser( pxServer_lineEdit->text(),  pxUserID_lineEdit->text(), pxPasswrd_lineEdit->text() );
}

void settings_dialog::updateConnectionState( QString state ){
	pxTest_label->setText( state );
}

