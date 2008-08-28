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
#include "mergepatch_dialog.h"

mergePatch_dialog::mergePatch_dialog( patch_t ewi_patches[EWI_NUM_PATCHES] ) : QDialog()
{
	setupUi( this );
	
	// copy path names into list
	for (int i = 0; i < EWI_NUM_PATCHES; i++ ) {
		QString *sname = new QString( ewi_patches[i].parameters.name );
		sname->truncate( EWI_PATCHNAME_LENGTH );
		patch_listWidget->addItem( sname->trimmed() );
	}
	
	connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
}


mergePatch_dialog::~mergePatch_dialog()
{
}

void mergePatch_dialog::accept() {
	
	chosenRow = patch_listWidget->currentRow();
	if (chosenRow == -1)
		QDialog::done( false );
	else
		QDialog::done( true );
}

