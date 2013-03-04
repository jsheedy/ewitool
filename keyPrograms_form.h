/***************************************************************************
 *   Copyright (C) 2009 by Steve Merrony   *
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
#ifndef KEYPROGRAMSFORM_H
#define KEYPROGRAMSFORM_H

#include <QWidget>

#include "midi_data.h"

#include "ui_keyPrograms_form.h"

#include "ewi4000sQuickPC.h"

class keyPrograms_form : public QWidget, private Ui::KeyPrograms_Form
{
  Q_OBJECT
  public:
    keyPrograms_form( QWidget *parent, midi_data *mididata );
    ~keyPrograms_form();
    void printQuickPCs();
        
  public slots:
    void request_quickPCs();
    void send_quickPCs();
    
  private:
    void populate_dropdowns();
    
    midi_data *mididata;
};



#endif