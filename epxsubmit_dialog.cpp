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
#include "epxsubmit_dialog.h"

epxSubmit_dialog::epxSubmit_dialog( QString pname ) : QDialog()
{
	setupUi( this );
	
	name_label->setText( pname );
	
	
	settings = new QSettings( "EWItool", "EWItool" );
	QString url = settings->value( "PatchExchange/Server" ).toString();
	epx = new patchExchange( this );
	connect( epx, SIGNAL( dropdownData( QStringList )), this, SLOT( populateDropdowns( QStringList ) ) );
	epx->getDropdowns( url );
	this->setEnabled( false );
}


epxSubmit_dialog::~epxSubmit_dialog()
{
}

/**
 * Data is a string containing 2 lines of CSVs, one for each dropdown
 * For patch submission we only need the first one
 * @param dd_data 
 */
void epxSubmit_dialog::populateDropdowns( QStringList dd_data ) {
	
	QStringList sl = dd_data.at( 0 ).split( "," );
	type_comboBox->addItem( "" );
	type_comboBox->addItems( sl );
			
	this->setEnabled( true );
}

/**
 * Check for gross user input errors here:
 *  Origin must not be empty, type must be selected, desc not too long
 */
void epxSubmit_dialog::accept() {

	// *** should be more friendly and popup alerts here... ***
	if (origin_lineEdit->text().length() == 0) return;
	if (type_comboBox->currentIndex() < 1) return;
	if (desc_textEdit->toPlainText().length() > 256) return;
	
	p_origin = origin_lineEdit->text();
	if (private_checkBox->isChecked()) 
		p_private = "true"; 
	else
		p_private = "false";
	p_type = type_comboBox->currentText();
	p_desc = desc_textEdit->toPlainText();
	p_tags = tags_lineEdit->text();
			
	QDialog::done( true );
}


