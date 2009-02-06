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
#ifndef PATCHEXCHANGEGUI_H
#define PATCHEXCHANGEGUI_H

#include <QWidget>

#include "clipboard.h"
#include "midi_data.h"
#include "patchexchange.h"

#include "ui_patchExchange_form.h"

/**
	@author Steve Merrony <merrony AT googlemail dOt com>
*/

class patchExchangeGUI : public QWidget, private Ui::patchExchange_form
{
Q_OBJECT
public:
    patchExchangeGUI(Clipboard *clipbrd, patchExchange *main_epx, QWidget *parent );
    ~patchExchangeGUI();
	
public slots:
	void populateEPXtab( QStringList dropdown_data );
	void epxQuery();
	void epxQueryResults( QString patch_list );
	void epxChosen();
	void epxDetailsResults( QString details );
	void epxStatsResults( QString stats_list );
	void deleteResults( QString r );
	void epxDelete();
	void epxCopy();
	
private:
	patchExchange *epx;
	QString 	url;
	Clipboard	*clipboard;
	midi_data	*mididata;
	QList<int>	epx_ids;
	int			epx_details_id;
	QString		epx_hex_patch;
};

#endif
