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
#ifndef EPXSUBMIT_DIALOG_H
#define EPXSUBMIT_DIALOG_H

/**
	@author Steve Merrony <steve@brahma>
*/

#include <QDialog>
#include <QSettings>
#include <QStringList>

#include "patchexchange.h"

#include "ui_epxSubmit_dialog.h"

class epxSubmit_dialog : public QDialog, Ui::epxSubmit_Dialog {
		Q_OBJECT

	public:
		epxSubmit_dialog( QString pname );
		~epxSubmit_dialog();

		QString p_origin;
		QString p_private;
		QString	p_type;
		QString p_desc;
		QString p_tags;
		
	private slots:
		void accept();
		void populateDropdowns( QStringList dd_data );
		
	private:
		QSettings *settings;
		patchExchange *epx;
};

#endif
