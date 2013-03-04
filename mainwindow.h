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

#include <QAction>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QLCDNumber>

#include "ui_mainwindow.h"

#include "clipboard.h"
#include "ewilistwidget.h"
#include "midi_data.h"
#include "patchexchange.h"
#include "patchexchangegui.h"
#include "keyPrograms_form.h"

const QString	HELP_URL			= "http://code.google.com/p/ewitool/wiki/Using_EWItool";
const QString	GPL3_URL			= "http://www.gnu.org/licenses/gpl-3.0.txt";

const int		STATUS_MSG_TIMEOUT 	= 3000;
const int		LIBRARY_TAB			= 0;
const int		EPX_TAB				= 1;
const int		EWI_TAB				= 2;
const int		PATCH_TAB			= 3;
const int       KEYPATCH_TAB        = 4;

class MainWindow: public QMainWindow, public Ui::MainWindow{
Q_OBJECT
	public:	
		MainWindow( volatile midi_data *, QWidget * parent = 0, Qt::WFlags f = 0 );
    	~MainWindow();
	
	private slots:
		// menu
		void import() ;
		void saveAs();
		void print();
		void settings();
		void quit();
		void MIDIconnections();
		void sendSysexFile();
		void fetchAllPatches();
		void externalHelp();
		void externalLicence();
		void about();
		// GUI actions
		void copyCurrentPatchAs();
		void saveCurrentPatch();
		void revertPatch();
		void patchSelected( int );
		void displayPatch();
		void displayPatch( int );
		void copyEWIPatch( int );
		void pasteEWIPatch( int );
		void renameEWIPatch( int );
		
		void defaultPatch();
		void makeDry();
		void deNoise();
		void randomPatch();
		void randomisePatch();
		void mergePatch();
        void maxVolPatch();
		void mixInPatch( int, int );
		
		void setList_chosen(QListWidgetItem *);
		void deletePatchSet();
		void librarySelectionChanged( );
		void sendLibraryToEWI();
		void copyToClipboard();
		void exportCurrentPatch();
				
		void changeSlider( int );
		void changeDial( int );
		void changeOctaveCombo( int );
		void changeSemitoneCombo( int );
		void changeGenericCombo( int );
		void changeCheckBox( int );
		
		void tabChanged( int );
	
	private:
		void setupPatchTab();
		void setupLibraryTab();
		void setupEWItab();
        void setupKeyPatchesTab();
		//void contextMenuEvent( QContextMenuEvent *);
		void savePatchSetAs();
		void printEWIpatches();
		void printSetPatches();
		void printCurrentPatch();
		void printPatchList( bool current );
				
		QAction *editAct, *copyAct, *pasteAct, *renameAct;
		QMenu *EWIcontextMenu;
	
		void loadSettings();
		void saveSettings();
		void savePatchSet( QString );
		int randBetween( int, int );
		int randNear( int, int, int );
		int mixInts( int, int, int );
		
		QString		libraryLocation;
		QString		libraryName;
		QList<int>	epx_ids;
		int			epx_details_id;
		QString		epx_hex_patch;
		
		midi_data *mididata;
		
		EWIListWidget 	*EWIList;
        keyPrograms_form *keyPrograms;
		patchExchange	*epx;
		patchExchangeGUI *epxGUI;
		Clipboard		*clipboard;
		
		patch_t			edit_patch;
		patch_t			backup_patch;
		patch_t 		patchSet[EWI_NUM_PATCHES];
		
};

#endif
