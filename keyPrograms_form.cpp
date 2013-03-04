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
#include <iostream>
using namespace std;

#include <QDate>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>

#include "ewi4000spatch.h"

#include "keyPrograms_form.h"

keyPrograms_form::keyPrograms_form( QWidget *parent, midi_data *md )
  : QWidget(parent)
{
  setupUi( this );

  mididata = md;
  connect( fetchKP_pushButton, SIGNAL( clicked() ), this, SLOT(request_quickPCs() ));
  connect( store_pushButton, SIGNAL( clicked() ), this, SLOT( send_quickPCs() ));
}

keyPrograms_form::~keyPrograms_form()
{
}

void keyPrograms_form::request_quickPCs() {

  if (mididata->last_patch_loaded == -1) {
    QMessageBox::warning(this, tr("EWItool"), tr("You must Fetch All Patches from the EWI before using this function."));
      return;
  }

  populate_dropdowns();
  
  if (!mididata->requestQuickPCs()) {
    cerr << "Error requesting QuickPCs from EWI - aborting\n";
    exit(1);
  }

  C_comboBox->setCurrentIndex( mididata->quickPCs[0] );
  Db_comboBox->setCurrentIndex( mididata->quickPCs[1] );
  D_comboBox->setCurrentIndex( mididata->quickPCs[2] );
  Eb_comboBox->setCurrentIndex( mididata->quickPCs[3] );
  E_comboBox->setCurrentIndex( mididata->quickPCs[4] );
  F_comboBox->setCurrentIndex( mididata->quickPCs[5] );
  Gb_comboBox->setCurrentIndex( mididata->quickPCs[6] );
  G_comboBox->setCurrentIndex( mididata->quickPCs[7] );
  Ab_comboBox->setCurrentIndex( mididata->quickPCs[8] );
  A_comboBox->setCurrentIndex( mididata->quickPCs[9] );
  Bb_comboBox->setCurrentIndex( mididata->quickPCs[10] );
  B_comboBox->setCurrentIndex( mididata->quickPCs[11] );

  //store_pushButton->setEnabled( true );  DISABLED for now (0.7) as the current EWI firmware (2.4) seems to misbehave when QUICKPCs are set
}

void keyPrograms_form::printQuickPCs() {

  QPrinter *printer = new QPrinter();
  printer->setOrientation( QPrinter::Portrait );
  printer->setResolution( 600 );
  printer->setOutputFormat( QPrinter::PostScriptFormat );
  
  QPrintDialog *dialog = new QPrintDialog( printer, this );
  dialog->setWindowTitle( tr( "Print EWI Quick Patch Changes" ) );

  if ( dialog->exec() != QDialog::Accepted ) return;

  // Create a QPainter object to draw on the printer
  QPainter p( printer );
  QPixmap  pm;
    
  pm = QPixmap::grabWidget( this ).scaledToWidth( printer->width() );
  p.drawPixmap( 0, 0, pm );
    //p.rotate( 90.0 );
  p.end();
}

void keyPrograms_form::send_quickPCs() {

  mididata->quickPCs[0] = (unsigned char) C_comboBox->currentIndex();
  mididata->quickPCs[1] = (unsigned char) Db_comboBox->currentIndex();
  mididata->quickPCs[2] = (unsigned char) D_comboBox->currentIndex();
  mididata->quickPCs[3] = (unsigned char) Eb_comboBox->currentIndex();
  mididata->quickPCs[4] = (unsigned char) E_comboBox->currentIndex();
  mididata->quickPCs[5] = (unsigned char) F_comboBox->currentIndex();
  mididata->quickPCs[6] = (unsigned char) Gb_comboBox->currentIndex();
  mididata->quickPCs[7] = (unsigned char) G_comboBox->currentIndex();
  mididata->quickPCs[8] = (unsigned char) Ab_comboBox->currentIndex();
  mididata->quickPCs[9] = (unsigned char) A_comboBox->currentIndex();
  mididata->quickPCs[10] = (unsigned char) Bb_comboBox->currentIndex();
  mididata->quickPCs[11] = (unsigned char) B_comboBox->currentIndex();
  
  mididata->sendQuickPCs();
  
  request_quickPCs();
}

void keyPrograms_form::populate_dropdowns() {

  QString patch_label;
  
  // always clear out the combos in case reloading, or Patch set has changed
  C_comboBox->clear();
  Db_comboBox->clear();
  D_comboBox->clear();
  Eb_comboBox->clear();
  E_comboBox->clear();
  F_comboBox->clear();
  Gb_comboBox->clear();
  G_comboBox->clear();
  Ab_comboBox->clear();
  A_comboBox->clear();
  Bb_comboBox->clear();
  B_comboBox->clear();
  
  for (int p = 0; p < EWI_NUM_PATCHES; p++) {
    patch_label.setNum(p + 1);
    patch_label += " - " + ewi4000sPatch::toQString( mididata->patches[p].parameters.name );
    C_comboBox->addItem( patch_label );
    Db_comboBox->addItem( patch_label );
    D_comboBox->addItem( patch_label );
    Eb_comboBox->addItem( patch_label );
    E_comboBox->addItem( patch_label );
    F_comboBox->addItem( patch_label );
    Gb_comboBox->addItem( patch_label );
    G_comboBox->addItem( patch_label );
    Ab_comboBox->addItem( patch_label );
    A_comboBox->addItem( patch_label );
    Bb_comboBox->addItem( patch_label );
    B_comboBox->addItem( patch_label );

  }
  
}
