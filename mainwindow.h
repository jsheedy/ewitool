/***************************************************************************
 *   Copyright (C) 2008 by Steve Merrony                                   *
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/**
	@author Steve Merrony <ewitool At merrony dot flyer dot co dot uk>
*/

#include <alsa/asoundlib.h>

#include <QGridLayout>
#include <QLCDNumber>

#include "ui_mainwindow.h"

#include "midi_data.h"

const QString	EXPORT_DIR 			= "export";
const int		EXPORT_PATCH_NUM 	= 99;
const QString 	CLIPBOARD_FILE  	= "/CLIPBOARD.CLP";
const QString	LIBRARY_EXTENSION	= ".syx";
const int		STATUS_MSG_TIMEOUT 	= 3000;

class MainWindow: public QMainWindow, public Ui::MainWindow{
Q_OBJECT
	public:	
		MainWindow( volatile midi_data *, QWidget * parent = 0, Qt::WFlags f = 0 );
    	~MainWindow();
	
	private slots:
		// menu
		void import() ;
		void save();
		void saveAs();
		void print();
		void quit();
		void MIDIconnections();
		void panic();
		void fetchAllPatches();
		void about();
		// GUI actions
		void saveClipboard();
		void saveCurrentPatchAs();
		void saveCurrentPatch();
		void revertPatch();
		void patchSelected( int );
		void displayPatch();
		void copyEWIPatch();
		void pasteEWIPatch();
		void renameEWIPatch();
		
		void setList_chosen(QListWidgetItem *);
		void sendLibraryToEWI();
		void copyToClipboard();
		void clearClipboard();
		void deleteClipboard();
		void renameClipboard();
		void viewHexClipboard();
		void exportClipboard();
				
		void changeSlider( int );
		void changeDial( int );
		void changeOctaveCombo( int );
		void changeSemitoneCombo( int );
		void changeGenericCombo( int );
		void changeCheckBox( int );
	
	private:
		void setupPatchTab();
		void setupLibraryTab();
		void setupEWItab();
		void savePatchSetAs();
		void printEWIpatches();
		void printCurrentPatch();
		
		void loadClipboard();
		void loadSettings();
		void saveSettings();
		void savePatchSet( QString );
		QString trimPatchName( char * );
		
		QString		libraryLocation;
		QString		libraryName;
		//QString		currentPatchSetName;
		
		midi_seq  seq;
		midi_port midiOut;
		midi_data *mididata;
		
		QGridLayout		*EWI_grid;
		QButtonGroup	*EWI_patch_group;
		QPushButton		*EWI_patch_name[100];
		QLCDNumber		*EWI_patch_num[100];
		
		patch_t			edit_patch;
		patch_t			backup_patch;
		patch_t 		patchSet[EWI_NUM_PATCHES];
		QList<patch_t> 	clipboard;
		//int	    clipboardCounter;
		
};

#endif
