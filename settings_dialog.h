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
#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

/**
	@author Steve Merrony <steve@brahma>
*/

#include <QDialog>
#include <QSettings>

#include "patchexchange.h"

#include "ui_settings_dialog.h"

class settings_dialog : public QDialog, Ui::settings_Dialog {
		Q_OBJECT

	public:
		settings_dialog();
		~settings_dialog();

	private slots:
		void accept();
		void clearSettings();
		void testEPX();
		void updateConnectionState( QString );
		
	private:
		QSettings	*settings;
		patchExchange *px;
};

#endif
