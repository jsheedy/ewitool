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
#include "ewilistwidget.h"

#include <iostream>
using namespace std;

#include <QAction>
#include <QGridLayout>
#include <QLCDNumber>

EWIListWidget::EWIListWidget( QWidget *parent )
	: QWidget( parent )
{
	/* The body of EWI_tab will be a grid of 10cols x 20rows containing patch
	numbers and names */ 
	
	QAction *editAct[100],
			*copyAct[100],
			*pasteAct[100],
			*renameAct[100];
	
	edit_signal_mapper = new QSignalMapper( this );
	copy_signal_mapper = new QSignalMapper( this );
	paste_signal_mapper = new QSignalMapper( this );
	rename_signal_mapper = new QSignalMapper( this );
			
	QGridLayout *EWI_grid = new QGridLayout();
	
	QFont font( "Helvetica", 9 );
	
	for (int col = 0; col < 5; col++) {
		for (int row = 0; row < 20; row++) {
			// first the LCD-style patch number (displayed as real number + 1)
			QLCDNumber *EWI_patch_num = new QLCDNumber( 3 );
			EWI_patch_num->setSegmentStyle(QLCDNumber::Filled);
			EWI_patch_num->display( col*20 + row + 1 );
			EWI_grid->addWidget( EWI_patch_num, row, col*2 );
			// now the name button
			QPushButton *name_button = new QPushButton( QString( "          <Empty " + QString().setNum( row + 20*col + 1 ) + ">          ") );
			name_button->setFont( font );
			patch_button_list.append( name_button );
			name_button->setCheckable( false );		// makes it a 'toggle' button
			
			// the context menu
			editAct[row + 20*col] = new QAction( tr( "&Edit" ), this );
			copyAct[row + 20*col] = new QAction( tr( "&Copy" ), this );
			pasteAct[row + 20*col] = new QAction( tr( "&Paste" ), this );
			renameAct[row + 20*col] = new QAction( tr( "&Rename" ), this );
			
			name_button->addAction( editAct[row + 20*col] );
			name_button->addAction( copyAct[row + 20*col] );
			name_button->addAction( pasteAct[row + 20*col] );
			name_button->addAction( renameAct[row + 20*col] );
			
			edit_signal_mapper->setMapping( editAct[row + 20*col], row + 20*col );
			connect( editAct[row + 20*col], SIGNAL( triggered() ), edit_signal_mapper, SLOT( map() ) );
			copy_signal_mapper->setMapping( copyAct[row + 20*col], row + 20*col );
			connect( copyAct[row + 20*col], SIGNAL( triggered() ), copy_signal_mapper, SLOT( map() ) );
			paste_signal_mapper->setMapping( pasteAct[row + 20*col], row + 20*col );
			connect( pasteAct[row + 20*col], SIGNAL( triggered() ), paste_signal_mapper, SLOT( map() ) );
			rename_signal_mapper->setMapping( renameAct[row + 20*col], row + 20*col );
			connect( renameAct[row + 20*col], SIGNAL( triggered() ), rename_signal_mapper, SLOT( map() ) );
			
			EWI_grid->addWidget( name_button, row, (col*2)+1 );
			// disable the context menu (until button is populated with a meaningful value via setLabel() )
			name_button->setContextMenuPolicy( Qt::NoContextMenu );
		}
	}
	
	connect( edit_signal_mapper, SIGNAL( mapped( int ) ), this, SIGNAL( edit_signal( int ) ) );
	connect( copy_signal_mapper, SIGNAL( mapped( int ) ), this, SIGNAL( copy_signal( int ) ) );
	connect( paste_signal_mapper, SIGNAL( mapped( int ) ), this, SIGNAL( paste_signal( int ) ) );
	connect( rename_signal_mapper, SIGNAL( mapped( int ) ), this, SIGNAL( rename_signal( int ) ) );
	
	setLayout( EWI_grid );
}


EWIListWidget::~EWIListWidget()
{
}

void EWIListWidget::setLabel( int patch_num, QString patch_name) {
	patch_button_list.at( patch_num )->setText( patch_name );
	patch_button_list.at( patch_num )->setContextMenuPolicy( Qt::ActionsContextMenu );
}

QString EWIListWidget::getLabel( int patch_num ) {
	return patch_button_list.at( patch_num )->text();
}
