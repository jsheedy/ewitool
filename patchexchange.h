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
#ifndef PATCHEXCHANGE_H
#define PATCHEXCHANGE_H

#include <QBuffer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

/**
	@author Steve Merrony 
*/
class patchExchange : public QObject {
		Q_OBJECT

	public:
		patchExchange( QWidget *parent = 0 );
		~patchExchange();

		void testConnection( QString );
		void testUser( QString, QString, QString );
		void getDropdowns( QString url );
		QString requestStatus();
		void getStats( QString url );
		void insertPatch( QString url, QString userid, QString passwd, 
						  QString patch_name, 
						  QString origin, 
						  QString patch_type, 
						  QString description,
						  QString isprivate,
						  QString tags,
						  QString hex_patch );
		void query( QString url, QString userid, QString passwd, 
				   QString ptype,
				   QString since,
				   QString contrib,
				   QString origin,
				   QString tags );
		void getDetails( QString url, QString userid, QString passwd, 
						 int patch_id );
		void deletePatch( QString url, QString userid, QString passwd, 
						 int patch_id );
		
	signals:
		void connectionState( QString );
		void loginState( QString );
		void dropdownData( QStringList );
		void insertResponse( QString );
		void queryResponse( QString );
		void detailsResponse( QString );
		void statsResponse( QString );
		void deleteResponse( QString );
		
	private slots:
		//void httpDone( bool );
        //void finished( int, bool);
        void finished( QNetworkReply * );
		void exchangeClipboardResponse( QString response );
		
	private:
        QNetworkAccessManager *qnam;
        QNetworkRequest request;
		QBuffer *html_buff;
		QByteArray *html_arr;
		QWidget *owner;
		int		host_id, ct_id, vu_id, dd_id, ins_id, qry_id, details_id, delete_id, stats_id;
		
};

#endif
