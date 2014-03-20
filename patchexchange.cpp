/***************************************************************************
 *   Copyright (C) 2008-2014 by Steve Merrony                              *
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

#include <QByteArray>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QSettings>
#include <QString>
#include <QUrl>         

#include "patchexchange.h"

patchExchange::patchExchange( QWidget * parent) :QObject( parent ) {
	owner = parent;
    qnam = new QNetworkAccessManager( this );
	html_arr = new QByteArray();
	html_buff = new QBuffer( html_arr );
	
	host_id = 0;	ct_id = 0;	vu_id = 0;	dd_id = 0;	ins_id = 0;	
	qry_id = 0, details_id = 0; delete_id = 0; stats_id = 0;
	
    //connect( qnam, SIGNAL( requestFinished( int, bool )), this, SLOT( finished( int, bool ) ) );
    connect( qnam, SIGNAL( finished( QNetworkReply* )), this, SLOT( finished( QNetworkReply* ) ) );
	
}


patchExchange::~patchExchange() { }

void patchExchange::testConnection( QString url ) {
	
    request.setUrl( QUrl( "http://" + url + "/EPX/epx.php?action=connectionTest" ) );
    QNetworkReply *reply = qnam->get( request );
}

void patchExchange::testUser( QString url, QString userid, QString passwd ) {
	
    request.setUrl( QUrl( "http://" + url + "/EPX/epx.php?action=validUser&userid=" + userid + "&passwd=" + passwd ) );
    QNetworkReply *reply = qnam->get( request );
}

void patchExchange::getDropdowns( QString url ) {

    QString OS;

  // Added the OS string in 0.7 to get some idea of what platform most users are on
#ifdef Q_OS_LINUX
  OS = "Linux";
#endif
#ifdef Q_OS_WIN
  OS = "Windows";
#endif
#ifdef Q_OS_MAC
  OS = "Mac";
#endif
  
    request.setUrl( QUrl( "http://" + url + "/EPX/epx.php?action=dropdownData&OS=" + OS ) );
    QNetworkReply *reply = qnam->get( request );
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


    QByteArray urlBA;
    QUrl    tmp_url;

    urlBA.append( "?action=insertPatch" );
    urlBA.append( "&userid=" + userid );
    urlBA.append( "&passwd=" + passwd );
    urlBA.append( "&name=" + patch_name.toHtmlEscaped() );
    urlBA.append( "&origin=" + origin.toHtmlEscaped() );
    urlBA.append( "&type=" + patch_type );
    urlBA.append( "&desc=" + description.toHtmlEscaped() );
    urlBA.append( "&private=" + isprivate );
    urlBA.append( "&tags=" + tags.toHtmlEscaped() );
    urlBA.append( "&hexpatch=" + hex_patch );

    tmp_url = QUrl( "http://" + url + "/EPX/epx.php" + QString( urlBA ) );
    request.setUrl( tmp_url );

    QNetworkReply *reply = qnam->get( request );
}
				  
void patchExchange::query( QString url,
						   QString userid, 
						   QString passwd, 
						    QString ptype,
						 	QString since,
						 	QString contrib,
						 	QString origin,
						 	QString tags ) {

    QByteArray urlBA;
    QUrl    tmp_url;
    
    urlBA.append( "?action=query" );
    urlBA.append( "&userid=" + userid );
    urlBA.append( "&passwd=" + passwd );
    urlBA.append( "&type=" + ptype );
    urlBA.append( "&since=" + since );
    urlBA.append( "&contrib=" + contrib.toHtmlEscaped() );
    urlBA.append( "&origin=" + origin.toHtmlEscaped() );
    urlBA.append( "&tags=" + tags.toHtmlEscaped() );

    tmp_url = QUrl( "http://" + url + "/EPX/epx.php" + QString( urlBA ) );
    request.setUrl( tmp_url );

    QNetworkReply *reply = qnam->get( request );
}

void patchExchange::getDetails( QString url,
								QString userid, 
								QString passwd, 
								int patch_id ) {

	QString id_str;
	id_str.setNum( patch_id );

    request.setUrl( QUrl( "http://" + url + "/EPX/epx.php?action=fetchPatch&userid=" + userid +
                          "&passwd=" + passwd + "&id=" + id_str ) );
    QNetworkReply *reply = qnam->get( request );
}

void patchExchange::getStats( QString url ) {

      request.setUrl( QUrl( "http://" + url + "/EPX/epx.php?action=stats" ) );
      QNetworkReply *reply = qnam->get( request );
}

void patchExchange::deletePatch( QString url,
								QString userid, 
								QString passwd, 
								int patch_id ) {
									
	QString id_str;
	id_str.setNum( patch_id );
    request.setUrl( QUrl( "http://" + url + "/EPX/epx.php?action=deletePatch&userid=" + userid +
                          "&passwd=" + passwd + "&id=" + id_str ) );
    QNetworkReply *reply = qnam->get( request );
}

void patchExchange::finished( QNetworkReply* reply ) {

    QByteArray responseBA = reply->readAll();

    // get action that was requested from header meta data
    int hstart = responseBA.lastIndexOf( "EPX-Requested-Action" ) + 22;
    int hfinish = responseBA.lastIndexOf("</head>") - 2;
    QByteArray htmp = responseBA.mid( hstart, hfinish - hstart );
    // extract to content
    QString requestedAction = htmp.mid( htmp.indexOf( '"' ) + 1 , htmp.lastIndexOf( '"' ) - htmp.indexOf( '"' ) -1 );


    int bstart = responseBA.lastIndexOf("<body>") + 7;
    int bfinish = responseBA.lastIndexOf("</body>") - 2;
    QByteArray tmp = responseBA.mid( bstart, bfinish - bstart );
    QString status = QString::fromUtf8( tmp.constData(), tmp.length()); // required so that UTF8 chars do not get mangled

    if (requestedAction == "connectionTest") emit connectionState( status );

    if (requestedAction == "validUser")      emit loginState( status );

    if (requestedAction == "dropdownData")   emit dropdownData( status.split( "\n" ) );

    if (requestedAction == "insertPatch")    emit insertResponse( status );

    if (requestedAction == "deletePatch")    emit deleteResponse( status );

    if (requestedAction == "query")          emit queryResponse( status );

    if (requestedAction == "fetchPatch")     emit detailsResponse( status );

    if (requestedAction == "stats")          emit statsResponse( status );

}

/*
void patchExchange::finished( int id, bool error ) {

	//cout << "Request " << id << " finished" << endl;

	if ( id == host_id ) {}

	if ( id == ct_id ) {
		if ( !error )
			emit connectionState( requestStatus() );
		else
            emit connectionState( qPrintable( qnam->errorString() ) );
	}

	if ( id == vu_id ) {
		if (!error) 
			emit loginState( requestStatus() );
		else
            emit loginState( qPrintable( qnam->errorString() ) );
	}
	
	if ( id == dd_id ) {
		if (!error) 
			emit dropdownData( requestStatus().split( "\n" ) );
		//else
        //	emit dropdownData( qPrintable( qnam->errorString() ) );
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
        //	emit dropdownData( qPrintable( qnam->errorString() ) );
	}
	if ( id == details_id ) {
		if (!error) 
			//cout << "Query response: " << qPrintable( requestStatus() ) << endl;
		emit detailsResponse( requestStatus() );
		//else
        //	emit dropdownData( qPrintable( qnam->errorString() ) );
	}
	if ( id == stats_id ) {
		emit statsResponse( requestStatus() );
	}
}
*/

QString patchExchange::requestStatus() {

    QByteArray tmp;
	QString	status;
  
	int sstart = html_arr->lastIndexOf("<body>") + 7;
	int sfinish = html_arr->lastIndexOf("</body>") - 2;
    tmp = html_arr->mid( sstart, sfinish - sstart );
    status = QString::fromUtf8( tmp.constData(), tmp.length()); // required so that UTF8 chars do not get mangled

//cout << qPrintable( status );
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
