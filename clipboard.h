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
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QListWidget>
#include <QWidget>

#include "ui_clipboardform.h"

#include "ewi4000spatch.h"
#include "patchexchange.h"

/**
	@author Steve Merrony <merrony AT googlemail dOt com>
*/

const QString 	CLIPBOARD_FILE  	= "/CLIPBOARD.CLP";
const QString	EXPORT_DIR 			= "/export";
const int		EXPORT_PATCH_NUM 	= 99;

class Clipboard : public QWidget, private Ui::ClipboardForm
{
Q_OBJECT
public:
    Clipboard( QString libraryLocation, patchExchange *main_epx, QWidget *parent = 0);
    ~Clipboard();
public slots:	
	void clearAll( );
	void load( );
	void save( );
	void appendItem( patch_t patch );
	void deleteItem();
	void renameItem();
	void viewHex();
	void exportToEPX();
	void exportToFile();
	void selectionChanged();
	bool onClipboard( QString pname );
	int  count();
	QString getNameAt( int i );
	patch_t getPatchAt( int i );
	
private:
	QString			libraryLocation;
	QList<patch_t>	clipboard_list;	
	patchExchange	*epx;
};

#endif
