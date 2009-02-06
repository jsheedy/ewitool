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
#include <iostream>
using namespace std;

#include <QMessageBox>
#include <QSettings>
#include <QString>

#include "patchexchange.h"

patchExchange::patchExchange( QWidget * parent) :QObject( parent )
{
	owner = parent;
	http = new QHttp( this );
	html_arr = new QByteArray();
	html_buff = new QBuffer( html_arr );
	
	host_id = 0;	ct_id = 0;	vu_id = 0;	dd_id = 0;	ins_id = 0;	
	qry_id = 0, details_id = 0; delete_id = 0; stats_id = 0;
	
	//connect( http, SIGNAL( done(bool) ), this, SLOT( httpDone( bool ) ) );
	connect( http, SIGNAL( requestFinished( int, bool )), this, SLOT( finished( int, bool ) ) );
	
}


patchExchange::~patchExchange()
{
}

void patchExchange::testConnection( QString url ) {
	
	host_id = http->setHost( url );
	ct_id = http->get( "/EPX/epx.php?action=connectionTest", html_buff );
}

void patchExchange::testUser( QString url, QString userid, QString passwd ) {
	
	host_id = http->setHost( url );
	vu_id = http->get( "/EPX/epx.php?action=validUser&userid=" + userid + "&passwd=" + passwd, html_buff );
}

void patchExchange::getDropdowns( QString url ) {
	
	host_id = http->setHost( url );
	dd_id = http->get( "/EPX/epx.php?action=dropdownData", html_buff );
}

void patchExchange::insertPatch( 
				  QString url,
				  QString userid, 
				  QString passwd, 
				  QString patch_name, 
				  QString origin, 
				  QString patch_type, 
				  QString description,
				  QString isprivate,
				  QString tags,
				  QString hex_patch ) {
	

	host_id = http->setHost( url );
	ins_id = http->get( "/EPX/epx.php?action=insertPatch&userid=" + userid + "&passwd=" + passwd +
						"&name=" + patch_name.replace( " ", "%20" ) +
						"&origin=" + origin.replace( " ", "%20" ) +
						"&type=" + patch_type.replace( " ", "%20" ) +
						"&desc=" + description.replace( " ", "%20" ) +
						"&private=" + isprivate +
						"&tags=" + tags.replace( " ", "%20" ) +
						"&hexpatch=" + hex_patch,
					  html_buff );
}
				  
void patchExchange::query( QString url,
						   QString userid, 
						   QString passwd, 
						    QString ptype,
						 	QString since,
						 	QString contrib,
						 	QString origin,
						 	QString tags ) {
	
	host_id = http->setHost( url );	
	qry_id = http->get( "/EPX/epx.php?action=query&userid=" + userid + "&passwd=" + passwd +
						"&type=" + ptype +
						"&since=" + since.replace( " ", "%20" ) +
						"&contrib=" + contrib +
						"&origin=" + origin.replace( " ", "%20" ) +
						"&tags=" + tags.replace( " ", "%20" ) ,
						html_buff );	
}

void patchExchange::getDetails( QString url,
								QString userid, 
								QString passwd, 
								int patch_id ) {
	host_id = http->setHost( url );	
	QString id_str;
	id_str.setNum( patch_id );
	details_id = http->get( "/EPX/epx.php?action=fetchPatch&userid=" + userid + "&passwd=" + passwd +
							"&id=" + id_str,
							html_buff );
}

void patchExchange::getStats( QString url ) {
	  host_id = http->setHost( url );	
	  stats_id = http->get( "/EPX/epx.php?action=stats",
	 						html_buff );
}

void patchExchange::deletePatch( QString url,
								QString userid, 
								QString passwd, 
								int patch_id ) {
									
	host_id = http->setHost( url );	
	QString id_str;
	id_str.setNum( patch_id );
	delete_id = http->get( "/EPX/epx.php?action=deletePatch&userid=" + userid + "&passwd=" + passwd +
							"&id=" + id_str,
						html_buff );
}

void patchExchange::finished( int id, bool error ) {

	//cout << "Request " << id << " finished" << endl;

	if ( id == host_id ) {}

	if ( id == ct_id ) {
		if ( !error )
			emit connectionState( requestStatus() );
		else
			emit connectionState( qPrintable( http->errorString() ) );
	}

	if ( id == vu_id ) {
		if (!error) 
			emit loginState( requestStatus() );
		else
			emit loginState( qPrintable( http->errorString() ) );
	}
	
	if ( id == dd_id ) {
		if (!error) 
			emit dropdownData( requestStatus().split( "\n" ) );
		//else
		//	emit dropdownData( qPrintable( http->errorString() ) );
	}
	
	if (id == ins_id ) {
		emit insertResponse( requestStatus() );
	}
	
	if (id == delete_id ) {
		emit deleteResponse( requestStatus() );
	}
	
	if ( id == qry_id ) {
		if (!error) 
			//cout << "Query response: " << qPrintable( requestStatus() ) << endl;
			emit queryResponse( requestStatus() );
		//else
		//	emit dropdownData( qPrintable( http->errorString() ) );
	}
	if ( id == details_id ) {
		if (!error) 
			//cout << "Query response: " << qPrintable( requestStatus() ) << endl;
		emit detailsResponse( requestStatus() );
		//else
		//	emit dropdownData( qPrintable( http->errorString() ) );
	}
	if ( id == stats_id ) {
		emit statsResponse( requestStatus() );
	}
}

QString patchExchange::requestStatus() {
	
	QString	status;
	
	int sstart = html_arr->lastIndexOf("<body>") + 7;
	int sfinish = html_arr->lastIndexOf("</body>") - 2;
	
	status = html_arr->mid( sstart, sfinish - sstart );

	return status;
}

void patchExchange::exchangeClipboardResponse( QString response ) {
	
	// cout << "EPX Export response: " << qPrintable( response ) << endl;
	if (response.startsWith( "Resource id #" )) { 	// success
		QMessageBox::information( owner, "EWItool - Patch Exchanged",
								  tr("Patch Succesfully sent to EWI Patch Exchange - Thank You") );
		QSettings settings( "EWItool", "EWItool" );
		QString url = settings.value( "PatchExchange/Server" ).toString();
		getStats( url );
	}
	else {
		// duplicate?
		if (response.contains( "duplicate key" )) {
			QMessageBox::warning( owner, "EWItool - Exchange Error",
								  tr( "EPX Export Error\n\nThat patch is already in the Exchange" ) );
		}
		else {	// some other error
			QMessageBox::warning( owner, "EWItool - Exchange Error",
								  tr( "EPX Export Error\n\n" ) + 
										  response.mid( response.indexOf( "ERROR:" ) + 7,
										  response.indexOf( " in <b>" ) - response.indexOf( "ERROR:" ) + 7) );
		}
	}
}
