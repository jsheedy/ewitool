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

#include "pastepatch_dialog.h"

pastePatch_dialog::pastePatch_dialog( int toOverwrite, QList<patch_t> *clipboard ) : QDialog() {
	
	setupUi( this );
	
	// copy clipboard names into list
	for (int i = 0; i < clipboard->size(); i ++ ) {
		QString *sname = new QString( clipboard->at(i).parameters.name );
		sname->truncate( EWI_PATCHNAME_LENGTH );
		listWidget->addItem( sname->trimmed() );
	}
	
	lcdNumber->display( toOverwrite + 1 );
	
	connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
}


pastePatch_dialog::~pastePatch_dialog()
{
}

void pastePatch_dialog::accept() {
	
	chosenRow = listWidget->currentRow();
	QDialog::done( true );
}
