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

#include <QMessageBox>
#include <QSettings>

#include "patchexchangegui.h"

patchExchangeGUI::patchExchangeGUI(Clipboard *clipbrd, patchExchange *main_epx, QWidget *parent )
 : QWidget(parent)
{
	setupUi( this );
	
	QSettings settings( "EWItool", "EWItool" );
	url = settings.value( "PatchExchange/Server" ).toString();
	epx = main_epx;
	// data returns
	connect( epx, SIGNAL( dropdownData( QStringList ) ), this, SLOT( populateEPXtab( QStringList ) ) );
	connect( epx, SIGNAL( insertResponse( QString ) ), epx, SLOT( exchangeClipboardResponse( QString ) ) );
	connect( epx, SIGNAL( queryResponse( QString ) ), this, SLOT( epxQueryResults( QString ) ) );
	connect( epx, SIGNAL( detailsResponse( QString ) ), this, SLOT( epxDetailsResults( QString ) ) );
	connect( epx, SIGNAL( deleteResponse( QString ) ) , this, SLOT( deleteResults( QString ) ) );
	connect( epx, SIGNAL( statsResponse( QString ) ) , this, SLOT( epxStatsResults( QString ) ) );
	epx->getDropdowns( url );
	epx->getStats( url );
	
	clipboard = clipbrd;
}

patchExchangeGUI::~patchExchangeGUI()
{
}

/**
 * The data for EPX have arrived - populate the tab
 */
void patchExchangeGUI::populateEPXtab( QStringList dropdown_data ) {
	
	QStringList sl = dropdown_data.at( 0 ).split( "," );
	type_comboBox->addItems( sl );
	sl = dropdown_data.at( 1 ).split( "," );
	contributor_comboBox->addItems( sl );
	sl = dropdown_data.at( 2 ).split( "," );
	origin_comboBox->addItems( sl );
	// widgets
	connect( epxQuery_pushButton, SIGNAL( clicked() ), this, SLOT( epxQuery() ) );
	connect( epxDelete_pushButton, SIGNAL( clicked() ), this, SLOT( epxDelete() ) );
	connect( epxCopy_pushButton, SIGNAL( clicked() ), this, SLOT( epxCopy() ) );
	connect( results_listWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( epxChosen() ) ) ;
	
	//epx_tab->setEnabled( true );
	this->setEnabled( true );
}


void patchExchangeGUI::epxQuery() {

	QSettings *settings = new QSettings( "EWItool", "EWItool" );
	epx->query( settings->value( "PatchExchange/Server" ).toString(),
	            settings->value( "PatchExchange/UserID" ).toString(),
	            settings->value( "PatchExchange/Password" ).toString(),
	            type_comboBox->currentText(),
	            since_comboBox->currentText(),
	            contributor_comboBox->currentText(),
	            origin_comboBox->currentText(),
	            tags_lineEdit->text() );

	epxCopy_pushButton->setEnabled( false );
	epxDelete_pushButton->setEnabled( false );
}

void patchExchangeGUI::epxQueryResults( QString patch_list ) {
	
	int id;
	
	results_listWidget->clear();
	epx_ids.clear();
	
	QStringList pl = patch_list.split( "\n" );
	for (int i = 0; i < pl.size(); i++ ) {
		results_listWidget->addItem( pl.at(i).left( pl.at(i).lastIndexOf( "," ) ) );
		id = pl.at(i).mid( pl.at(i).lastIndexOf( "," ) + 1 ).toInt();
		epx_ids.append( id );
	}
}

void patchExchangeGUI::epxStatsResults( QString stats_list ) {
	
	QStringList sl = stats_list.split( "\n" );
	users_label->setText( sl.at( 0 ) );
	patches_label->setText( sl.at( 1 ) );
	contributors_label->setText( sl.at( 2 ) );
	origins_label->setText( sl.at( 3 ) );
	access_label->setText( sl.at( 4 ) );
}

void patchExchangeGUI::epxChosen() {
	
	int id;
	
	QSettings *settings = new QSettings( "EWItool", "EWItool" );
	id = epx_ids.at( results_listWidget->currentRow() );
	epx->getDetails( settings->value( "PatchExchange/Server" ).toString(),
					 settings->value( "PatchExchange/UserID" ).toString(),
									  settings->value( "PatchExchange/Password" ).toString(),
											  id
				   );
}

void patchExchangeGUI::epxDetailsResults( QString details ) {
	
	if (details.contains( "," )) {
		QStringList parms = details.split( "," ); //////////// this needs fixing ***
		name_label->setText( parms.at( 0 ) );
		contributor_label->setText( parms.at( 1 ) );
		originator_label->setText( parms.at( 2 ) );
		epx_hex_patch = parms.at( 3 );
		type_label->setText( parms.at( 4 ) );
		desc_label->setText( parms.at( 5 ) );
		added_label->setText( parms.at( 6 ) );
		if (parms.at(7) == "f")
			private_label->setText( "public" );
		else
			private_label->setText( "private" );
		tags_label->setText( parms.at( 8 ) );
	
		epxCopy_pushButton->setEnabled( true );
		epxDelete_pushButton->setEnabled( true );
	}
}

void patchExchangeGUI::epxDelete() {
	
	if ( results_listWidget->currentRow() >= 0 ) {
	
		QSettings *settings = new QSettings( "EWItool", "EWItool" );
		int id = epx_ids.at( results_listWidget->currentRow() );
		epx->deletePatch( settings->value( "PatchExchange/Server" ).toString(),
						  settings->value( "PatchExchange/UserID" ).toString(),
										   settings->value( "PatchExchange/Password" ).toString(),
												   id
						);
		epx_ids.removeAt( id );
	
		name_label->clear();
		contributor_label->clear();
		originator_label->clear();
		type_label->clear();
		desc_label->clear();
		tags_label->clear();
		added_label->clear();
		epxCopy_pushButton->setEnabled( false );
		epxDelete_pushButton->setEnabled( false );
	
	// force a re-query
		epxQuery();
	
	}
}

void patchExchangeGUI::deleteResults( QString r ) {
	 
	epxQuery();
	epx->getStats( url );
}

void patchExchangeGUI::epxCopy() {
	
	patch_t new_patch;
	
	new_patch = ewi4000sPatch::dehexify( epx_hex_patch, false );
	
	clipboard->appendItem( new_patch );
}






