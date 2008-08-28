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

#include "ewilistwidget.h"
#include "midi_data.h"
#include "patchexchange.h"

const QString	HELP_URL			= "http://code.google.com/p/ewitool/wiki/Using_EWItool";
const QString	GPL3_URL			= "http://www.gnu.org/licenses/gpl-3.0.txt";
//const QString	EXPORT_DIR 			= "/export";
const int		EXPORT_PATCH_NUM 	= 99;
const QString 	CLIPBOARD_FILE  	= "/CLIPBOARD.CLP";
const QString	LIBRARY_EXTENSION	= ".syx";
const int		STATUS_MSG_TIMEOUT 	= 3000;
const int		LIBRARY_TAB			= 0;
const int		EPX_TAB				= 1;
const int		EWI_TAB				= 2;
const int		PATCH_TAB			= 3;

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
		void settings();
		void quit();
		void MIDIconnections();
		void panic();
		void fetchAllPatches();
		void externalHelp();
		void externalLicence();
		void about();
		// EPX
		void populateEPXtab( QStringList dropdown_data );
		// GUI actions
		void saveClipboard();
		void saveCurrentPatchAs();
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
		void mixInPatch( int, int );
		
		void setList_chosen(QListWidgetItem *);
		void deletePatchSet();
		void sendLibraryToEWI();
		void copyToClipboard();
		void clearClipboard();
		void deleteClipboard();
		void renameClipboard();
		void viewHexClipboard();
		void exportClipboard();
		void exportClipboardResponse( QString response );
		void epxQuery();
		void epxQueryResults( QString patch_list );
		void epxChosen();
		void epxDetailsResults( QString details );
		void epxDelete();
		void epxCopy();
				
		void changeSlider( int );
		void changeDial( int );
		void changeOctaveCombo( int );
		void changeSemitoneCombo( int );
		void changeGenericCombo( int );
		void changeCheckBox( int );
		
		void tabChanged( int );
	
	private:
		void setupPatchTab();
		void setupEPXtab();
		void setupLibraryTab();
		void setupEWItab();
		//void contextMenuEvent( QContextMenuEvent *);
		void savePatchSetAs();
		void printEWIpatches();
		void printSetPatches();
		void printCurrentPatch();
		void printPatchList( bool current );
				
		QAction *editAct, *copyAct, *pasteAct, *renameAct;
		QMenu *EWIcontextMenu;
		
		void loadClipboard();
		void loadSettings();
		void saveSettings();
		void savePatchSet( QString );
		QString trimPatchName( char * );
		int randBetween( int, int );
		int randNear( int, int, int );
		int mixInts( int, int, int );
		
		QString		libraryLocation;
		QString		libraryName;
		patchExchange *epx;
		QList<int>	epx_ids;
		int			epx_details_id;
		QString		epx_hex_patch;
		//QString		currentPatchSetName;
		
		midi_seq  seq;
		midi_port midiOut;
		midi_data *mididata;
		
		EWIListWidget 	*EWIList;
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
