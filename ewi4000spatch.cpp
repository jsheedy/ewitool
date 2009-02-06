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
#include "ewi4000spatch.h"

ewi4000sPatch::ewi4000sPatch()
{
}


ewi4000sPatch::~ewi4000sPatch()
{
}

QString ewi4000sPatch::toQString( char *rawName ) {
	// utility function to get a nicely formatted patch name
	QString sname = rawName;
	sname.truncate( EWI_PATCHNAME_LENGTH );
	return sname.trimmed();
}

/**
 * Returns a human-readable Hex interpretation of the passed patch.
 * @param bin_patch 
 * @param with_spaces 
 * @return 
 */
QString ewi4000sPatch::hexify( char *bin_patch, bool with_spaces ) {
	
	QString hex_patch = "";
	QString format;
	int dlen;
	
	if (with_spaces) {
		format = "%1 ";
		dlen = 3;
	}
	else {
		format = "%1";
		dlen = 2;
	}
	for ( int i = 0; i < EWI_PATCH_LENGTH; i++ ) {
		hex_patch += QString( format ).arg( (uint) bin_patch[i], 2, 16, QChar( '0' ) ).right( dlen );
	}
	//cout << "Hexified patch: " << qPrintable( hex_patch ) << endl;
	return hex_patch;
}


patch_t ewi4000sPatch::dehexify( QString hex_patch, bool with_spaces ) {
	
	patch_t dehexed;
	QString onebyte;
	bool ok;
	int  bwidth;
	
	if (with_spaces)
		bwidth = 3;
	else
		bwidth = 2;
	
	for ( int i = 0; i < EWI_PATCH_LENGTH; i++ ) {
		onebyte = hex_patch.mid( i * bwidth, 2 );
		dehexed.whole_patch[i] = onebyte.toInt( &ok, 16 );
	}
	
	return dehexed;
}

