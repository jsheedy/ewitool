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

#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;

#include <QByteArray>
#include <QDataStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QProgressDialog>
#include <QSettings>
#include <QString>
#include <QUrl>

#include "mainwindow.h"
				 
#include "epxsubmit_dialog.h"
#include "midiportsdialog.h"
#include "mergepatch_dialog.h"
#include "pastepatch_dialog.h"
#include "settings_dialog.h"
#include "viewhex_dialog.h"

MainWindow::MainWindow( volatile midi_data *shared_midi_data,QWidget * parent, Qt::WFlags f )
	: QMainWindow(parent, f)
{
	mididata = (midi_data *) shared_midi_data;
	
	loadSettings();  // get (or set) the stored app settings
	
	setupUi( this );
	// connect the menu items to their slots
	connect(action_Import, SIGNAL(triggered()), this, SLOT(import()));
	connect(action_Save, SIGNAL(triggered()), this, SLOT(save()));
	connect(actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(actionPrint, SIGNAL(triggered()), this, SLOT(print()));
	connect(actionSe_ttings, SIGNAL(triggered()), this, SLOT(settings()));
	connect(actionE_xit, SIGNAL(triggered()), this, SLOT(quit()));
	
	connect(action_Connections, SIGNAL(triggered()), this, SLOT(MIDIconnections()));
	connect(action_Panic, SIGNAL(triggered()), this, SLOT(panic()));
	connect(actionFetch_All_Patches, SIGNAL(triggered()), this, SLOT(fetchAllPatches()));
	
	menu_Patch->setEnabled( false );
	connect(action_Patch_Save_As, SIGNAL(triggered()), this, SLOT(saveCurrentPatchAs()));
	connect(action_Patch_Copy, SIGNAL(triggered()), this, SLOT(saveCurrentPatch()));
	connect(action_Patch_Revert, SIGNAL(triggered()), this, SLOT(revertPatch()));
	connect(action_Default_Patch, SIGNAL(triggered()), this, SLOT(defaultPatch()));
	connect(action_Random_Patch, SIGNAL(triggered()), this, SLOT(randomPatch()));
	connect(actionMake_Dry, SIGNAL(triggered()), this, SLOT(makeDry()));
	connect(actionRemove_Noise, SIGNAL(triggered()), this, SLOT(deNoise()));
	connect(action_Randomise_10, SIGNAL(triggered()), this, SLOT(randomisePatch()));
	connect(action_Merge_With, SIGNAL(triggered()), this, SLOT(mergePatch()));
	
	connect(actionEWItool_Help, SIGNAL(triggered()), this, SLOT(externalHelp()));
	connect(actionLicence, SIGNAL( triggered() ), this, SLOT( externalLicence() ) );
	connect(action_About, SIGNAL(triggered()), this, SLOT(about()));

	connect( mainTabSet, SIGNAL( currentChanged( int ) ), this, SLOT( tabChanged( int ) ) );
	
	// by default we don't want context menus appearing
	this->setContextMenuPolicy( Qt::NoContextMenu );
	
	setupEWItab();
	patch_tab->setEnabled( false );
	epx_tab->setEnabled( false );  	// EPX tab disabled unless we can connect to EPX
	setupPatchTab();
	setupLibraryTab();
}


MainWindow::~MainWindow()
{
}

void MainWindow::loadSettings() {
	
	QSettings settings( "EWItool", "EWItool" );
	// check for existence of settings
	if (settings.allKeys().isEmpty()) {
		// this is the first run, or user zapped the library directory
		libraryLocation = QFileDialog::getExistingDirectory(this,
				tr( "Please Choose the Library Directory" ),
				"/home",
				QFileDialog::ShowDirsOnly );
		if (libraryLocation.isEmpty()) {
			loadSettings();	//user was perverse and cancelled the op
		}
		else {
			QSettings settings( "EWItool", "EWItool" );
			settings.setValue( "library/location", libraryLocation );
			//QDir::QDir( libraryLocation ).mkdir( EXPORT_DIR );		// create an export subdirectory
		}
	} else {
		libraryLocation = settings.value( "library/location" ).toString();
		if (settings.contains( "MIDI/OutClient" ))
				  mididata->connectOutput( settings.value("MIDI/OutClient").toInt(), settings.value("MIDI/OutPort").toInt() );
		if (settings.contains( "MIDI/InClient" ))
			mididata->connectInput( settings.value("MIDI/InClient").toInt(), settings.value("MIDI/InPort").toInt() );
		
		if (settings.contains( "PatchExchange/UserID" )) {
			setupEPXtab();
		}
		else {
			//epx_tab->setEnabled( false );  // crashes on win32...
		}
	}
}

void MainWindow::settings() {
	settings_dialog *d = new settings_dialog();
	if (d->exec()) {
		loadSettings();
	}
	delete d;
}

// tabSet handlers...

void MainWindow::tabChanged( int new_tab ) {
	// conextualise the menus
	switch ( new_tab ) {
		case LIBRARY_TAB:
			action_Import->setEnabled( true );
			actionSave_As->setEnabled( false );
			actionPrint->setEnabled( true );
			break;
		case EPX_TAB:
			action_Import->setEnabled( false );
			actionSave_As->setEnabled( false );
			actionPrint->setEnabled( false );
			break;
		case EWI_TAB:
			action_Import->setEnabled( false );
			actionSave_As->setEnabled( true );
			actionPrint->setEnabled( true );
			break;
		case PATCH_TAB:
			action_Import->setEnabled( false );
			actionSave_As->setEnabled( true );
			actionPrint->setEnabled( true );
			break;
		default:
			cerr << "Oops - unexpected tab change signal";
	}
	
}

// handlers fror the top-level menu items

/**
 * Import a .bnk or .sqs sound bank.
 * Full banks are written out to a (native) .syx file.
 * Patches from partial (<100) banks are put on the Clipboard
 */
void MainWindow::import() {
	
	int rc;
	
	QString fileName = QFileDialog::getOpenFileName(this,
			tr( "Import Soundbank" ),
			libraryLocation,
			tr( "Soundbanks/Collections (*.bnk *.sqs)" ));
	if (fileName.isEmpty()) return;
	
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly)) {
		return;
	}
	
	// look busy!
	QApplication::setOverrideCursor(Qt::WaitCursor);
	
	//construct the new name
	QFileInfo info( fileName );
	QString sname = info.baseName() + LIBRARY_EXTENSION;
	
	QDataStream inp(&file);
	int body_start;
	
	if (fileName.endsWith( ".bnk", Qt::CaseInsensitive )) {
		
		char	header_section[EWI_SOUNDBANK_MAX_HEADER_LENGTH];
	
		inp.readRawData( (char *) &header_section, EWI_SOUNDBANK_MAX_HEADER_LENGTH );
		QByteArray *ba_header_section = new QByteArray( (const char *) &header_section, EWI_SOUNDBANK_MAX_HEADER_LENGTH );
		body_start = ba_header_section->indexOf( "BODY" );
	}
	else {
		char	header_section[EWI_SQS_MAX_HEADER_LENGTH];
	
		inp.readRawData( (char *) &header_section, EWI_SQS_MAX_HEADER_LENGTH );
		QByteArray *ba_header_section = new QByteArray( (const char *) &header_section, EWI_SQS_MAX_HEADER_LENGTH );
		QByteArray ba_body_start( EWI_SQS_BODY_START, 14 );
		body_start = ba_header_section->indexOf( ba_body_start, 0 );
		//body_start = ba_header_section->lastIndexOf( "BODY\0\0" );
	}
	
	if (body_start == -1) {
		QMessageBox::warning( this, "EWItool",
							  tr( "Import Error\n\nUnknown Soundbank format (Can't find BODY)" ) );
		QApplication::restoreOverrideCursor();
		return;
	}
	
	body_start += 8; 	// skip BODY and next 4 bytes
	
	// skip the header
	file.reset();
	rc = inp.skipRawData( body_start );
	
	// try to read the rest of the soundbank into patchSet[]
	setContents_listWidget->clear();
	for (int p = 0; p < EWI_NUM_PATCHES; p++) {
		inp.readRawData( patchSet[p].whole_patch, EWI_PATCH_LENGTH );
		// trivial check that we got a patch
		if (patchSet[p].parameters.trailer_f7 != (char) 0xf7) {
			// either it's not a bnk file we understand at all, or it's a partial bank...
			if (p == 0) { 
				// first patch seems corrupt, give up
				QMessageBox::warning( this, "EWItool",
									  tr( "Import Error\n\nUnknown Soundbank format (Can't find 1st patch)" ) );
				QApplication::restoreOverrideCursor();
				return;
			}
			else {
				// we got some patches - but not a full set, so put them on the clipboard
				for (int cp = 0; cp < p; cp++) {
					clipboard_listWidget->addItem( trimPatchName( patchSet[cp].parameters.name ) );
					clipboard.append( patchSet[cp] );
				}
				saveClipboard();
				QApplication::restoreOverrideCursor();
				statusBar()->showMessage(tr("Partial Soundbank imported and saved on Clipboard"), STATUS_MSG_TIMEOUT);
				return;
			}
		}
		setContents_listWidget->addItem( trimPatchName( patchSet[p].parameters.name ) );
	}
	
	// write new patchset to disk
	QFile outfile( libraryLocation + "/" + sname );
	if (!outfile.open(QFile::WriteOnly)) {
		QMessageBox::warning(this, "EWItool",
							 tr("Cannot write file %1:\n%2.")
									 .arg(fileName)
									 .arg(file.errorString()));
		return;
	}

	QDataStream out(&outfile);
	for (int p = 0; p < EWI_NUM_PATCHES; p++) {
		out.writeRawData( patchSet[p].whole_patch, EWI_PATCH_LENGTH );
	}
	
	setList_listWidget->addItem( sname );
	sendLibraryToEWI_pushButton->setEnabled( true );
	
	QApplication::restoreOverrideCursor();
	
	statusBar()->showMessage(tr("Complete Soundbank imported and saved"), STATUS_MSG_TIMEOUT);
}

void MainWindow::save() {
	//if (mainTabSet->currentWidget() == library_tab ) saveClipboard();
}

void MainWindow::saveAs() {
	if (mainTabSet->currentWidget() == EWI_tab ) savePatchSetAs();
}

void MainWindow::print() {
	
	switch (mainTabSet->currentIndex()) {
		case LIBRARY_TAB:
			printSetPatches();
			break;
		case EWI_TAB:
			printEWIpatches();
		 	break;
		case PATCH_TAB:
		 	printCurrentPatch();
		 	break;
	}
}

void MainWindow::quit() {
	qApp->quit();
}

void MainWindow::externalHelp() {
	QDesktopServices::openUrl( QUrl( HELP_URL ) );
}

void MainWindow::externalLicence() {
	QDesktopServices::openUrl( QUrl( GPL3_URL ) );
}

void MainWindow::about() {
	QMessageBox::about(this,  
					   "About EWItool",
					   "<center><b>EWItool</b><br><br>"
						"Version: 0.4<br><br>"
						"&copy; 2008 Steve Merrony<br><br>"
						"Please see<br>"
						"<a href='http://code.google.com/p/ewitool/'>http://code.google.com/p/ewitool/</a><br>"
						"for more information</center>" );
}

void MainWindow::MIDIconnections() {
	
	//MIDIports_Dialog mp = new MIDIports_Dialog( this );
	MIDIports_Dialog d( mididata );
	d.exec();
	
}

void MainWindow::panic() {
	
	mididata->sendPanic();
	
}

void MainWindow::setupPatchTab() {
	
	// connect Current patch GUI objects to their slots
	connect(osc1_octave_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeOctaveCombo(int)));
	connect(osc2_octave_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeOctaveCombo(int)));
	connect(osc1_semitone_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSemitoneCombo(int)));
	connect(osc2_semitone_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSemitoneCombo(int)));
	
	connect(oscFilterLink_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	connect(noiseFilterLink_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	connect(oscfilter1_mode_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	connect(oscfilter2_mode_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	connect(noisefilter1_mode_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	connect(noisefilter2_mode_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	
	connect(xfade_checkBox, SIGNAL(stateChanged(int)), this, SLOT(changeCheckBox(int)));
	
	connect(osc1_saw_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(osc1_tri_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(osc1_sqr_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(osc1_level_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(osc2_saw_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(osc2_tri_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(osc2_sqr_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(osc2_level_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(oscfilter1_freq_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(oscfilter2_freq_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(noiseLevel_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(noisefilter1_freq_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(noisefilter2_freq_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(chorusLFOfreq_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(delayLevel_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(reverbLevel_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(ampLevel_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(octaveLevel_verticalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	
	connect(antiAliasSwitch_checkBox, SIGNAL(stateChanged(int)), this, SLOT(changeCheckBox(int)));
	connect(antiAliasCutoff_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(antiAliasKeyFollow_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	connect(osc1_fine_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_beat_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_pulseWidth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_PWMfreq_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_PWMdepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_sweepDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_sweepTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_breathDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_breathAttain_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_breathCurve_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc1_breathThresh_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_fine_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_beat_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_pulseWidth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_PWMfreq_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_PWMdepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_sweepDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_sweepTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_breathDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_breathAttain_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_breathCurve_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(osc2_breathThresh_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	connect(formantFilter_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	connect(keyTrigger_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	
	connect(oscfilter1_Q_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_keyFollow_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_breathMod_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_LFOfreq_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_LFOdepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_LFObreath_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_LFOthreshold_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_sweepDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_sweepTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter1_breathCurve_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
			
	connect(oscfilter2_Q_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_keyFollow_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_breathMod_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_LFOfreq_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_LFOdepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_LFObreath_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_LFOthreshold_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_sweepDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_sweepTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(oscfilter2_breathCurve_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
			
	connect(noiseTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noiseBreath_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	
	connect(noisefilter1_Q_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_keyFollow_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_breathMod_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_LFOfreq_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_LFOdepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_LFObreath_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_LFOthreshold_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_sweepDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_sweepTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter1_breathCurve_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
			
	connect(noisefilter2_Q_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_keyFollow_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_breathMod_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_LFOfreq_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_LFOdepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_LFObreath_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_LFOthreshold_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_sweepDepth_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_sweepTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(noisefilter2_breathCurve_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	connect(chorusSwitch_checkBox, SIGNAL(stateChanged(int)), this, SLOT(changeCheckBox(int)));
	
	connect(chorusDelay1_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(chorusModLev1_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(chorusWetLev1_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(chorusDelay2_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(chorusModLev2_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(chorusWetLev2_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(chorusDryLevel_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(chorusFeedback_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	connect(delayTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(delayFeedback_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(delayDamp_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(delayMix_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	connect(reverbTime_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(reverbDensity_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(reverbDamp_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(reverbMix_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	connect(biteVibrato_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	connect(biteTremolo_dial, SIGNAL(valueChanged(int)), this, SLOT(changeDial(int)));
	
	connect(bendStepMode_checkBox, SIGNAL(stateChanged(int)), this, SLOT(changeCheckBox(int)));
	connect(bendRange_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeGenericCombo(int)));
	
	// init the random # generator
	srand( time( 0 ) );
}

void MainWindow::setupLibraryTab() {
	
	QDir dir = QDir( libraryLocation, "*" + LIBRARY_EXTENSION );
	QStringList lib_names;
	lib_names = dir.entryList();
	setList_listWidget->clear();
	setList_listWidget->addItems( lib_names );
	
	loadClipboard();
	
	// connect set library GUI objects to their slots
	connect(setList_listWidget, SIGNAL(itemClicked( QListWidgetItem *)), this, SLOT( setList_chosen(QListWidgetItem *) ));
	connect(deleteSet_pushButton, SIGNAL(clicked()), this, SLOT( deletePatchSet() ));
	connect(sendLibraryToEWI_pushButton, SIGNAL(clicked()), this, SLOT( sendLibraryToEWI() ));
	
	connect(copyToClipboard_pushButton, SIGNAL(clicked()), this, SLOT( copyToClipboard() ));
	connect(clearClipboard_pushButton, SIGNAL(clicked()), this, SLOT( clearClipboard() ));
	connect(deleteClipboard_pushButton, SIGNAL(clicked()), this, SLOT( deleteClipboard() ));
	connect(renameClipboard_pushButton, SIGNAL(clicked()), this, SLOT( renameClipboard() ));
	connect(viewHexClipboard_pushButton, SIGNAL(clicked()), this, SLOT( viewHexClipboard() ));
	connect(exportClipboard_pushButton, SIGNAL(clicked()), this, SLOT( exportClipboard() ));
	
}

/**
 * Here we _request_ the dropdown data for the EPX tab.
 */
void MainWindow::setupEPXtab() {
	
	QSettings settings( "EWItool", "EWItool" );
	QString url = settings.value( "PatchExchange/Server" ).toString();
	epx = new patchExchange( this );
	// data returns
	connect( epx, SIGNAL( dropdownData( QStringList ) ), this, SLOT( populateEPXtab( QStringList ) ) );
	connect( epx, SIGNAL( insertResponse( QString ) ), this, SLOT( exportClipboardResponse( QString ) ) );
	connect( epx, SIGNAL( queryResponse( QString ) ), this, SLOT( epxQueryResults( QString ) ) );
	connect( epx, SIGNAL( detailsResponse( QString ) ), this, SLOT( epxDetailsResults( QString ) ) );
	epx->getDropdowns( url );
}

/**
 * The data for EPX have arrived - populate the tab
 */
void MainWindow::populateEPXtab( QStringList dropdown_data ) {
	
	QStringList sl = dropdown_data.at( 0 ).split( "," );
	type_comboBox->addItems( sl );
	sl = dropdown_data.at( 1 ).split( "," );
	contributor_comboBox->addItems( sl );
	sl = dropdown_data.at( 2 ).split( "," );
	origin_comboBox->addItems( sl );
	// widgets
	connect( epxQuery_pushButton, SIGNAL( clicked() ), this, SLOT( epxQuery() ) );
	connect( epxDelete_pushButton, SIGNAL( clicked() ), this, SLOT( epxDelete() ) );
	connect( epxCopy_pushButton, SIGNAL( clicked() ), this, SLOT( epxCopy() ) );
	connect( results_listWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( epxChosen() ) ) ;
	epx_tab->setEnabled( true );
}

void MainWindow::setupEWItab() {
	
	EWIList = new EWIListWidget( patchSet_widget );
	connect( EWIList, SIGNAL( edit_signal( int ) ), this, SLOT( displayPatch( int ) ) );
	connect( EWIList, SIGNAL( copy_signal( int ) ), this, SLOT( copyEWIPatch( int ) ) );
	connect( EWIList, SIGNAL( paste_signal( int ) ), this, SLOT( pasteEWIPatch( int ) ) );
	connect( EWIList, SIGNAL( rename_signal( int ) ), this, SLOT( renameEWIPatch( int ) ) );
	
}

void MainWindow::saveClipboard() {
	
	/* 	this is not invoked directly by the user, but every function that changes the clipboard 
		contents should call this once it has finished making changes to commit the clipoard to disk */
	
	QString fileName = libraryLocation + CLIPBOARD_FILE;
	
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly)) {
		QMessageBox::warning(this, tr("EWItool"),
							 tr("Cannot write file %1:\n%2.")
									 .arg(fileName)
									 .arg(file.errorString()));
		return;
	}

	QDataStream out(&file);
	for (int p = 0; p < clipboard.count(); p++ ) {
		out.writeRawData( clipboard.at(p).whole_patch, EWI_PATCH_LENGTH );
	}
	statusBar()->showMessage(tr("Clipboard saved"), STATUS_MSG_TIMEOUT);
}

void MainWindow::loadClipboard() {
	
	patch_t tmp_patch;
	QString fileName = libraryLocation + CLIPBOARD_FILE;
	
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly)) {
		return;
	}

	clipboard_listWidget->clear();
	clipboard.clear();
	
	QDataStream inp(&file);
	while (!inp.atEnd()) {
		inp.readRawData( (char *) &tmp_patch, EWI_PATCH_LENGTH );
		clipboard_listWidget->addItem( trimPatchName( tmp_patch.parameters.name ) );
		clipboard.append( tmp_patch );
	}
}

void MainWindow::savePatchSetAs() {
	
	QString fileName = QFileDialog::getSaveFileName(this,
												   tr( "Patch Set File Name" ),
												   libraryLocation,
												   tr( "Patch Sets (*.syx *.SYX)" ));
	if (fileName.isEmpty()) return;

	if (!fileName.endsWith( LIBRARY_EXTENSION ) ) fileName += LIBRARY_EXTENSION;
	
	savePatchSet( fileName );
	
	setupLibraryTab();
}

void MainWindow::savePatchSet( QString fileName ) {
	
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly)) {
		QMessageBox::warning(this, tr("EWItool"),
							 tr("Cannot write file %1:\n%2.")
									 .arg(fileName)
									 .arg(file.errorString()));
		return;
	}

	QDataStream out(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	for (int p = 0; p < EWI_NUM_PATCHES; p++) {
		out.writeRawData( mididata->patches[p].whole_patch, EWI_PATCH_LENGTH );
	}
	QApplication::restoreOverrideCursor();

	//currentPatchSetName = fileName;
	
	statusBar()->showMessage(tr("File saved"), STATUS_MSG_TIMEOUT);
}

// Menu actions

void MainWindow::saveCurrentPatchAs() {
	
	// Get new name from the user
	bool ok;
	QString new_name = QInputDialog::getText(this, "EWItool",
										 tr("New Patch Name:"), QLineEdit::Normal,
										 "", &ok);
	
	if (!ok || new_name.isEmpty()) return;
	
	if (new_name.length() > EWI_PATCHNAME_LENGTH ) { 
		QMessageBox::warning(this, tr("EWItool"),
							 tr("Patch name too long!\nEnter up to %1 characters").arg(EWI_PATCHNAME_LENGTH));
		saveCurrentPatchAs();
	}
	
	// check name not already on clipboard
	if (clipboard_listWidget->findItems( new_name, Qt::MatchExactly ).count() > 0) {
		QMessageBox::warning(this, tr("EWItool"),
							 tr("That name already used on the Clipboard!\nPlease try again"));
		saveCurrentPatchAs();
	}
		
	// Save patch in the Clipboard
	clipboard_listWidget->addItem( new_name );
	QByteArray ba = new_name.leftJustified( EWI_PATCHNAME_LENGTH, ' ' ).toLatin1();
	memcpy( (void *) &edit_patch.parameters.name, (void *) ba.data(), EWI_PATCHNAME_LENGTH);
	clipboard.append( edit_patch );
	
	statusBar()->showMessage(tr("Patch saved to Clipboard"), STATUS_MSG_TIMEOUT);
}

void MainWindow::saveCurrentPatch() {

	if (QMessageBox::question (this, "EWItool",
	                           tr ("This will overwrite this patch in the EWI and the EWI Patch Set.\n\n"
								   "(Nothing will get stored on disk at this stage.)\n\n"
	                               "Do you really want to do this?"),
	                           QMessageBox::No | QMessageBox::Yes
	                          ) ==  QMessageBox::No) return;
	
	// send patch to EWI using its current patch number (i.e. overwrite
	mididata->sendPatch( edit_patch, EWI_SAVE );
	
	// copy patch into the EWI patch set
	patchSet[edit_patch.parameters.patch_num] = edit_patch;
}

/**
 * Fetch all 100 patches from the EWI
 * Switch GUI to the EWI tab and display all the names
 */
void MainWindow::fetchAllPatches() {
	
	QString sname;
	
	QProgressDialog progressDialog( tr("Requesting all patches from the EWI"),
									0,
									1, 100,
									this);
	
	progressDialog.setWindowTitle(tr("Fetching Patches"));
	
	mainTabSet->setCurrentWidget( EWI_tab );
	mididata->last_patch_loaded = -1;
	int p_int;
	
	for ( char p = 0; p < EWI_NUM_PATCHES; p++ )
	{ 
		p_int = (int) p;
		if (mididata->verboseMode) cout << "Reqesting patch " << p_int << endl;
		
		if (!mididata->requestPatch( p ) ) { // will not return until a patch is received or timed out
			 QMessageBox::critical (this,
                       tr ("Unable to Contact EWI"),
                       tr ("No response was received from the EWI.  Please check that it is connected, turned on, and that the correct MIDI ports have been selected."));
return;
		}
		if (mididata->verboseMode) cout << "Received patch " << mididata->last_patch_loaded << endl;
		progressDialog.setValue( p_int );
		progressDialog.setLabelText(tr("Fetching patch number %1 of %2...").arg(p_int).arg( EWI_NUM_PATCHES ));
		qApp->processEvents();
	}
	
	for (int p = 0; p < EWI_NUM_PATCHES; p++ ) {
		EWIList->setLabel( p, trimPatchName( mididata->patches[p].parameters.name ) );
	} 
	
	libraryName.clear();  					// as we've just loaded patches direct from the EWI there's no current libary name
	patchSet_widget->setEnabled( true );  	// now patches are loaded we can choose between them
	
}

/**
 * Prints a table of the current patch numbers and names to fit on top half of an A4 page
 */
void MainWindow::printEWIpatches( ) {
	
	printPatchList( true );
}

/**
 * Prints a table of the patch numbers and names from a set to fit on top half of an A4 page
 */
void MainWindow::printSetPatches( ) {
	
	if (setContents_listWidget->count() > 1 )
		printPatchList( false );	
}

/**
 * Prints a table of the patch numbers and names from a set to fit on top half of an A4 page
 */
void MainWindow::printPatchList( bool current ) {

	QPrinter printer;
	QPrintDialog *dialog = new QPrintDialog( &printer, this );
	dialog->setWindowTitle( tr( "Print EWI Patch Set" ) );

	if ( dialog->exec() != QDialog::Accepted ) return;

	// Create a QPainter object to draw on the printer
	QPainter p( &printer );
	QString plab;

	const int half_inch = 36;
	const int one_inch = 72;
	const int col_spacing = 180;
	const int line_height = 18;

	// patch set name
	if ( current )
		p.drawText( half_inch, half_inch, "Current EWI Contents" );
	else
		p.drawText( half_inch, half_inch, setList_listWidget->currentItem()->text() );

	for ( int row = 0; row < 25; row++ ) {
		for ( int col = 0; col < 4; col++ ) {
			plab.setNum( col*25 + row + 1 );

			if ( current )
				p.drawText( half_inch + ( col*col_spacing ), one_inch + ( row*line_height ), plab + ": " + EWIList->getLabel( row + 25*col ) );
			else
				p.drawText( half_inch + ( col*col_spacing ), one_inch + ( row*line_height ), plab + ": " + trimPatchName( patchSet[row + 25*col].parameters.name ) );
		}
	}
	p.end();
}

/**
 * Prints a graphical dump of the editor window, should fit on top half of an A4 page
 */
void MainWindow::printCurrentPatch() {
	
	QPrinter *printer = new QPrinter();
	printer->setOrientation( QPrinter::Portrait );
	printer->setResolution( 300 );
	printer->setOutputFormat( QPrinter::PostScriptFormat );
	//printer.setup( this );
	
	QPrintDialog *dialog = new QPrintDialog( printer, this );
	dialog->setWindowTitle(tr("Print EWI Patch"));
	if (dialog->exec() != QDialog::Accepted) return;

    // Create a QPainter object to draw on the printer
	QPainter p( printer );
	QPixmap  pm;
	
	pm = QPixmap::grabWidget( mainTabSet ).scaledToWidth( printer->width() );
	p.drawPixmap( 0, 0, pm );
	//p.rotate( 90.0 );
	
	p.end();
}

void MainWindow::revertPatch() {
	
	edit_patch = backup_patch;
	displayPatch();
	
}

void MainWindow::patchSelected( int patch_num ) {
	
	edit_patch = mididata->patches[patch_num];
}

void MainWindow::displayPatch() {
	
	//patchSelected( patch_num);
	//cout << "£diting " << qPrintable (this->objectName() ) << endl;
	
	// store a copy of the patch for 'revert' functiom
	backup_patch = edit_patch;
	
	// block signals from patch_tab so none emitted when we set the values
	QList<QWidget *> pchildren = patch_tab->findChildren<QWidget *>();
	foreach (QWidget * current, pchildren) {
		current->blockSignals( true );
	}
		
	patch_tab->setEnabled( true );
	mainTabSet->setCurrentWidget( patch_tab );
	//edit_patch = mididata->patches[patch_num];  // already done in patchSelected
	patchnum_lcdNumber->display( edit_patch.parameters.patch_num + 1 );
	
	// tell the EWI what we're up to
	mididata->sendPatch( edit_patch, EWI_EDIT ); 
			
	patchname_lineEdit->setText( trimPatchName( edit_patch.parameters.name)  );
	
	// ix 0 = +2
	osc1_octave_comboBox->setCurrentIndex( edit_patch.parameters.osc1.octave - 64 + 2 );
	osc1_semitone_comboBox->setCurrentIndex( edit_patch.parameters.osc1.semitone - 64 + 12 );
	osc1_fine_dial->setValue( edit_patch.parameters.osc1.fine );
	osc1_beat_dial->setValue( edit_patch.parameters.osc1.beat );
	osc1_saw_verticalSlider->setValue( edit_patch.parameters.osc1.sawtooth );
	osc1_tri_verticalSlider->setValue( edit_patch.parameters.osc1.triangle );
	osc1_sqr_verticalSlider->setValue( edit_patch.parameters.osc1.square );
	osc1_pulseWidth_dial->setValue( edit_patch.parameters.osc1.pulseWidth );
	osc1_PWMfreq_dial->setValue( edit_patch.parameters.osc1.PWMfreq );
	osc1_PWMdepth_dial->setValue( edit_patch.parameters.osc1.PWMdepth );
	osc1_sweepTime_dial->setValue( edit_patch.parameters.osc1.sweepTime );
	osc1_sweepDepth_dial->setValue( edit_patch.parameters.osc1.sweepDepth );
	osc1_breathAttain_dial->setValue( edit_patch.parameters.osc1.breathAttain );
	osc1_breathDepth_dial->setValue( edit_patch.parameters.osc1.breathDepth );
	osc1_breathThresh_dial->setValue( edit_patch.parameters.osc1.breathThreshold );
	osc1_breathCurve_dial->setValue( edit_patch.parameters.osc1.breathCurve );
	osc1_level_verticalSlider->setValue( edit_patch.parameters.osc1.level );
	
	osc2_octave_comboBox->setCurrentIndex( edit_patch.parameters.osc2.octave - 64 + 2 );
	osc2_semitone_comboBox->setCurrentIndex( edit_patch.parameters.osc2.semitone - 64 + 12 );
	osc2_fine_dial->setValue( edit_patch.parameters.osc2.fine );
	osc2_beat_dial->setValue( edit_patch.parameters.osc2.beat );
	osc2_saw_verticalSlider->setValue( edit_patch.parameters.osc2.sawtooth );
	osc2_tri_verticalSlider->setValue( edit_patch.parameters.osc2.triangle );
	osc2_sqr_verticalSlider->setValue( edit_patch.parameters.osc2.square );
	osc2_pulseWidth_dial->setValue( edit_patch.parameters.osc2.pulseWidth );
	osc2_PWMfreq_dial->setValue( edit_patch.parameters.osc2.PWMfreq );
	osc2_PWMdepth_dial->setValue( edit_patch.parameters.osc2.PWMdepth );
	osc2_sweepTime_dial->setValue( edit_patch.parameters.osc2.sweepTime );
	osc2_sweepDepth_dial->setValue( edit_patch.parameters.osc2.sweepDepth );
	osc2_breathAttain_dial->setValue( edit_patch.parameters.osc2.breathAttain );
	osc2_breathDepth_dial->setValue( edit_patch.parameters.osc2.breathDepth );
	osc2_breathThresh_dial->setValue( edit_patch.parameters.osc2.breathThreshold );
	osc2_breathCurve_dial->setValue( edit_patch.parameters.osc2.breathCurve );
	osc2_level_verticalSlider->setValue( edit_patch.parameters.osc2.level );
	
	xfade_checkBox->setChecked( edit_patch.parameters.osc2Xfade == 0x01 );
	
	oscFilterLink_comboBox->setCurrentIndex( edit_patch.parameters.oscFilterLink );
	
	oscfilter1_mode_comboBox->setCurrentIndex( edit_patch.parameters.oscFilter1.mode );
	oscfilter1_freq_horizontalSlider->setValue( edit_patch.parameters.oscFilter1.freq );
	oscfilter1_keyFollow_dial->setValue( edit_patch.parameters.oscFilter1.keyFollow );
	oscfilter1_breathMod_dial->setValue( edit_patch.parameters.oscFilter1.breathMod );
	oscfilter1_breathCurve_dial->setValue( edit_patch.parameters.oscFilter1.breathCurve );
	oscfilter1_LFOfreq_dial->setValue( edit_patch.parameters.oscFilter1.LFOfreq );
	oscfilter1_LFOdepth_dial->setValue( edit_patch.parameters.oscFilter1.LFOdepth );
	oscfilter1_LFObreath_dial->setValue( edit_patch.parameters.oscFilter1.LFObreath );
	oscfilter1_LFOthreshold_dial->setValue( edit_patch.parameters.oscFilter1.LFOthreshold );
	oscfilter1_sweepDepth_dial->setValue( edit_patch.parameters.oscFilter1.sweepDepth );
	oscfilter1_sweepTime_dial->setValue( edit_patch.parameters.oscFilter1.sweepTime );
	
	oscfilter2_mode_comboBox->setCurrentIndex( edit_patch.parameters.oscFilter2.mode );
	oscfilter2_freq_horizontalSlider->setValue( edit_patch.parameters.oscFilter2.freq );
	oscfilter2_keyFollow_dial->setValue( edit_patch.parameters.oscFilter2.keyFollow );
	oscfilter2_breathMod_dial->setValue( edit_patch.parameters.oscFilter2.breathMod );
	oscfilter2_breathCurve_dial->setValue( edit_patch.parameters.oscFilter2.breathCurve );
	oscfilter2_LFOfreq_dial->setValue( edit_patch.parameters.oscFilter2.LFOfreq );
	oscfilter2_LFOdepth_dial->setValue( edit_patch.parameters.oscFilter2.LFOdepth );
	oscfilter2_LFObreath_dial->setValue( edit_patch.parameters.oscFilter2.LFObreath );
	oscfilter2_LFOthreshold_dial->setValue( edit_patch.parameters.oscFilter2.LFOthreshold );
	oscfilter2_sweepDepth_dial->setValue( edit_patch.parameters.oscFilter2.sweepDepth );
	oscfilter2_sweepTime_dial->setValue( edit_patch.parameters.oscFilter2.sweepTime );
	
	formantFilter_comboBox->setCurrentIndex( edit_patch.parameters.formantFilter );
	
	keyTrigger_comboBox->setCurrentIndex( edit_patch.parameters.keyTrigger );
	
	noiseTime_dial->setValue( edit_patch.parameters.noiseTime );
	noiseBreath_dial->setValue( edit_patch.parameters.noiseBreath );
	noiseLevel_verticalSlider->setValue( edit_patch.parameters.noiseLevel );
	noiseFilterLink_comboBox->setCurrentIndex( edit_patch.parameters.noiseFilterLink );
	
	noisefilter1_mode_comboBox->setCurrentIndex( edit_patch.parameters.noiseFilter1.mode );
	noisefilter1_freq_horizontalSlider->setValue( edit_patch.parameters.noiseFilter1.freq );
	noisefilter1_keyFollow_dial->setValue( edit_patch.parameters.noiseFilter1.keyFollow );
	noisefilter1_breathMod_dial->setValue( edit_patch.parameters.noiseFilter1.breathMod );
	noisefilter1_breathCurve_dial->setValue( edit_patch.parameters.noiseFilter1.breathCurve );
	noisefilter1_LFOfreq_dial->setValue( edit_patch.parameters.noiseFilter1.LFOfreq );
	noisefilter1_LFOdepth_dial->setValue( edit_patch.parameters.noiseFilter1.LFOdepth );
	noisefilter1_LFObreath_dial->setValue( edit_patch.parameters.noiseFilter1.LFObreath );
	noisefilter1_LFOthreshold_dial->setValue( edit_patch.parameters.noiseFilter1.LFOthreshold );
	noisefilter1_sweepDepth_dial->setValue( edit_patch.parameters.noiseFilter1.sweepDepth );
	noisefilter1_sweepTime_dial->setValue( edit_patch.parameters.noiseFilter1.sweepTime );
	
	noisefilter2_mode_comboBox->setCurrentIndex( edit_patch.parameters.noiseFilter2.mode );
	noisefilter2_freq_horizontalSlider->setValue( edit_patch.parameters.noiseFilter2.freq );
	noisefilter2_keyFollow_dial->setValue( edit_patch.parameters.noiseFilter2.keyFollow );
	noisefilter2_breathMod_dial->setValue( edit_patch.parameters.noiseFilter2.breathMod );
	noisefilter2_breathCurve_dial->setValue( edit_patch.parameters.noiseFilter2.breathCurve );
	noisefilter2_LFOfreq_dial->setValue( edit_patch.parameters.noiseFilter2.LFOfreq );
	noisefilter2_LFOdepth_dial->setValue( edit_patch.parameters.noiseFilter2.LFOdepth );
	noisefilter2_LFObreath_dial->setValue( edit_patch.parameters.noiseFilter2.LFObreath );
	noisefilter2_LFOthreshold_dial->setValue( edit_patch.parameters.noiseFilter2.LFOthreshold );
	noisefilter2_sweepDepth_dial->setValue( edit_patch.parameters.noiseFilter2.sweepDepth );
	noisefilter2_sweepTime_dial->setValue( edit_patch.parameters.noiseFilter2.sweepTime );
	
	antiAliasSwitch_checkBox->setChecked( edit_patch.parameters.antiAliasSwitch == 0x01 );
	antiAliasCutoff_horizontalSlider->setValue( edit_patch.parameters.antiAliasCutoff );
	antiAliasKeyFollow_dial->setValue( edit_patch.parameters.antiAliasKeyFollow );
	
	chorusSwitch_checkBox->setChecked( edit_patch.parameters.chorusSwitch == 0x01 );
	chorusDelay1_dial->setValue( edit_patch.parameters.chorusDelay1 );
	chorusModLev1_dial->setValue( edit_patch.parameters.chorusModLev1 );
	chorusWetLev1_dial->setValue( edit_patch.parameters.chorusWetLev1 );
	chorusDelay2_dial->setValue( edit_patch.parameters.chorusDelay2 );
	chorusModLev2_dial->setValue( edit_patch.parameters.chorusModLev2 );
	chorusWetLev2_dial->setValue( edit_patch.parameters.chorusWetLev2 );
	chorusDryLevel_dial->setValue( edit_patch.parameters.chorusDryLevel );
	chorusFeedback_dial->setValue( edit_patch.parameters.chorusFeedback );
	chorusLFOfreq_horizontalSlider->setValue( edit_patch.parameters.chorusLFOfreq );
	
	delayTime_dial->setValue( edit_patch.parameters.delayTime );
	delayFeedback_dial->setValue( edit_patch.parameters.delayFeedback );
	delayDamp_dial->setValue( edit_patch.parameters.delayDamp );
	delayMix_dial->setValue( edit_patch.parameters.delayMix );
	delayLevel_verticalSlider->setValue( edit_patch.parameters.delayLevel );
	
	reverbTime_dial->setValue( edit_patch.parameters.reverbTime );
	reverbDensity_dial->setValue( edit_patch.parameters.reverbDensity );
	reverbDamp_dial->setValue( edit_patch.parameters.reverbDamp );
	reverbMix_dial->setValue( edit_patch.parameters.reverbMix );
	reverbLevel_verticalSlider->setValue( edit_patch.parameters.reverbLevel );
	
	biteTremolo_dial->setValue( edit_patch.parameters.biteTremolo );
	biteVibrato_dial->setValue( edit_patch.parameters.biteVibrato );
	
	bendStepMode_checkBox->setChecked( edit_patch.parameters.bendStepMode == 0x01 );
	bendRange_comboBox->setCurrentIndex( edit_patch.parameters.bendRange );
	
	ampLevel_verticalSlider->setValue( edit_patch.parameters.ampLevel );
	octaveLevel_verticalSlider->setValue( edit_patch.parameters.octaveLevel );
	
	// unblock signals from patch_tab 
	foreach (QWidget * current, pchildren) {
		current->blockSignals( false );
	}
	
	menu_Patch->setEnabled( true );
}

void MainWindow::displayPatch( int p ) {
	
	patchSelected( p );
	displayPatch();

}

// the Patch editor widget handlers...

void MainWindow::changeOctaveCombo( int new_oct ) {
	
	QObject *obj = sender();
	int nl = 64 + (new_oct - 2);
	if ( obj->objectName() == "osc1_octave_comboBox" )	{ mididata->sendLiveControl( 0, 64, nl ); edit_patch.parameters.osc1.octave = nl; return; }
	if ( obj->objectName() == "osc2_octave_comboBox" )	{ mididata->sendLiveControl( 0, 65, nl ); edit_patch.parameters.osc2.octave = nl; return; }
	cerr << "Oops - unhandled change for octave combo\n";
}

void MainWindow::changeSemitoneCombo( int new_semi ) {
	
	QObject *obj = sender();
	int nl = 64 + (new_semi - 12);
	if ( obj->objectName() == "osc1_semitone_comboBox" )	{ mididata->sendLiveControl( 1, 64, nl ); edit_patch.parameters.osc1.semitone = nl; return; }
	if ( obj->objectName() == "osc2_semitone_comboBox" )	{ mididata->sendLiveControl( 1, 65, nl ); edit_patch.parameters.osc1.semitone= nl; return; }
	cerr << "Oops - unhandled change for semitone combo\n";
}

void MainWindow::changeGenericCombo( int nl ) {
	
	QObject *obj = sender();
	if ( obj->objectName() == "formantFilter_comboBox" )	{ mididata->sendLiveControl( 5, 81, nl ); edit_patch.parameters.formantFilter = nl; return; }
	if ( obj->objectName() == "keyTrigger_comboBox" )		{ mididata->sendLiveControl( 7, 81, nl ); edit_patch.parameters.keyTrigger = nl; return; }
	if ( obj->objectName() == "oscFilterLink_comboBox" )	{ mididata->sendLiveControl( 3, 81, nl ); edit_patch.parameters.oscFilterLink = nl; return; }
	if ( obj->objectName() == "noiseFilterLink_comboBox" )	{ mididata->sendLiveControl( 4, 81, nl ); edit_patch.parameters.noiseFilterLink = nl; return; }
	if ( obj->objectName() == "oscfilter1_mode_comboBox" )	{ mididata->sendLiveControl( 0, 72, nl ); edit_patch.parameters.oscFilter1.mode = nl; return; }
	if ( obj->objectName() == "oscfilter2_mode_comboBox" )	{ mididata->sendLiveControl( 0, 73, nl ); edit_patch.parameters.oscFilter2.mode = nl; return; }
	if ( obj->objectName() == "noisefilter1_mode_comboBox" )	{ mididata->sendLiveControl( 0, 74, nl ); edit_patch.parameters.noiseFilter1.mode = nl; return; }
	if ( obj->objectName() == "noisefilter2_mode_comboBox" )	{ mididata->sendLiveControl( 0, 75, nl ); edit_patch.parameters.noiseFilter2.mode = nl; return; }
	if ( obj->objectName() == "bendRange_comboBox" )			{ mididata->sendLiveControl( 0, 81, nl ); edit_patch.parameters.bendRange = nl; return; }
	cerr << "Oops - unhandled change for generic combo\n";
}

void MainWindow::changeCheckBox( int s ) {
	
	QObject *obj = sender();
	int nl;
	if ( s == Qt::Checked ) 
		nl = 1; 
	else 
		nl = 0;
	if ( obj->objectName() == "antiAliasSwitch_checkBox" )		{ mididata->sendLiveControl( 0, 79, nl ); edit_patch.parameters.antiAliasSwitch = nl; return; }
	if ( obj->objectName() == "xfade_checkBox" )		{ mididata->sendLiveControl( 6, 81, nl ); edit_patch.parameters.osc2Xfade= nl; return; }
	if ( obj->objectName() == "chorusSwitch_checkBox" )	{ mididata->sendLiveControl( 9, 81, nl ); edit_patch.parameters.chorusSwitch= nl; return; }
	if ( obj->objectName() == "bendStepMode_checkBox" )	{ mididata->sendLiveControl( 1, 81, nl ); edit_patch.parameters.bendStepMode = nl; return; }
	cerr << "Oops - unhandled change for check box\n";
}

void MainWindow::changeSlider( int nl ) {
	
	statusBar()->showMessage( QString( "%1" ).arg( nl ), STATUS_MSG_TIMEOUT ); 
	QObject *obj = sender();
	if ( obj->objectName() == "osc1_saw_verticalSlider" )	{ mididata->sendLiveControl( 5, 64, nl ); edit_patch.parameters.osc1.sawtooth = nl; return; }
	if ( obj->objectName() == "osc1_tri_verticalSlider" )	{ mididata->sendLiveControl( 6, 64, nl ); edit_patch.parameters.osc1.triangle = nl; return; }
	if ( obj->objectName() == "osc1_sqr_verticalSlider" )	{ mididata->sendLiveControl( 7, 64, nl ); edit_patch.parameters.osc1.square = nl; return; }
	if ( obj->objectName() == "osc1_level_verticalSlider" )	{ mididata->sendLiveControl( 17, 64, nl ); edit_patch.parameters.osc1.level = nl; return; }
	if ( obj->objectName() == "osc2_saw_verticalSlider" )	{ mididata->sendLiveControl( 5, 65, nl ); edit_patch.parameters.osc2.sawtooth = nl; return; }
	if ( obj->objectName() == "osc2_tri_verticalSlider" )	{ mididata->sendLiveControl( 6, 65, nl ); edit_patch.parameters.osc2.triangle = nl; return; }
	if ( obj->objectName() == "osc2_sqr_verticalSlider" )	{ mididata->sendLiveControl( 7, 65, nl ); edit_patch.parameters.osc2.square = nl; return; }
	if ( obj->objectName() == "osc2_level_verticalSlider" )	{ mididata->sendLiveControl( 17, 65, nl ); edit_patch.parameters.osc2.level = nl; return; }
	if ( obj->objectName() == "oscfilter1_freq_horizontalSlider" )	{ mididata->sendLiveControl( 1, 72, nl ); edit_patch.parameters.oscFilter1.freq = nl; return; }
	if ( obj->objectName() == "oscfilter2_freq_horizontalSlider" )	{ mididata->sendLiveControl( 1, 73, nl ); edit_patch.parameters.oscFilter2.freq = nl; return; }
	if ( obj->objectName() == "noiseLevel_verticalSlider" )			{ mididata->sendLiveControl( 2, 80, nl ); edit_patch.parameters.noiseLevel = nl; return; }
	if ( obj->objectName() == "noisefilter1_freq_horizontalSlider" )	{ mididata->sendLiveControl( 1, 74, nl ); edit_patch.parameters.noiseFilter1.freq = nl; return; }
	if ( obj->objectName() == "noisefilter2_freq_horizontalSlider" )	{ mididata->sendLiveControl( 1, 75, nl ); edit_patch.parameters.noiseFilter2.freq = nl; return; }
	if ( obj->objectName() == "chorusLFOfreq_horizontalSlider" )		{ mididata->sendLiveControl( 7, 112, nl ); edit_patch.parameters.chorusLFOfreq = nl; return; }
	if ( obj->objectName() == "delayLevel_verticalSlider" )			{ mididata->sendLiveControl( 3, 113, nl ); edit_patch.parameters.delayLevel = nl; return; }
	if ( obj->objectName() == "reverbLevel_verticalSlider" )		{ mididata->sendLiveControl( 1, 114, nl ); edit_patch.parameters.reverbLevel = nl; return; }
	if ( obj->objectName() == "ampLevel_verticalSlider" )			{ mididata->sendLiveControl( 1, 88, nl ); edit_patch.parameters.ampLevel = nl; return; }
	if ( obj->objectName() == "octaveLevel_verticalSlider" )		{ mididata->sendLiveControl( 2, 88, nl ); edit_patch.parameters.octaveLevel = nl; return; }
	if ( obj->objectName() == "antiAliasCutoff_horizontalSlider" )		{ mididata->sendLiveControl( 1, 79, nl ); edit_patch.parameters.antiAliasCutoff = nl; return; }
	cerr << "Oops - unhandled change for slider\n";
}

void MainWindow::changeDial( int nl ) {
	
	// send the change to the EWI and write it into edit_patch
	statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( 100*nl/127 ), STATUS_MSG_TIMEOUT ); 
	QObject *obj = sender();
	
	if ( obj->objectName() == "antiAliasKeyFollow_dial" )		{ mididata->sendLiveControl( 2, 79, nl ); edit_patch.parameters.antiAliasKeyFollow = nl; return; }
	
	if ( obj->objectName() == "osc1_fine_dial" )	{ 
		mididata->sendLiveControl( 2, 64, nl ); 
		edit_patch.parameters.osc1.fine = nl; 
		statusBar()->showMessage( QString( "Value: %1 (%2 cents)" ).arg( nl ).arg( nl-64 ), STATUS_MSG_TIMEOUT ); 
		return; }
	if ( obj->objectName() == "osc1_beat_dial" )	{ mididata->sendLiveControl( 3, 64, nl ); edit_patch.parameters.osc1.beat = nl;return; }
	if ( obj->objectName() == "osc1_pulseWidth_dial" )	{ mididata->sendLiveControl( 8, 64, nl ); edit_patch.parameters.osc1.pulseWidth = nl;return; }
	if ( obj->objectName() == "osc1_PWMfreq_dial" )		{ mididata->sendLiveControl( 9, 64, nl ); edit_patch.parameters.osc1.PWMfreq = nl;return; }
	if ( obj->objectName() == "osc1_PWMdepth_dial" )	{ mididata->sendLiveControl( 10, 64, nl ); edit_patch.parameters.osc1.PWMdepth = nl;return; }
	if ( obj->objectName() == "osc1_sweepDepth_dial" )	{ 
		mididata->sendLiveControl( 11, 64, nl ); 
		edit_patch.parameters.osc1.sweepDepth = nl;
		statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );
		return; }
	if ( obj->objectName() == "osc1_sweepTime_dial" )	{ mididata->sendLiveControl( 12, 64, nl ); edit_patch.parameters.osc1.sweepTime = nl;return; }
	if ( obj->objectName() == "osc1_breathDepth_dial" )	{ mididata->sendLiveControl( 13, 64, nl ); edit_patch.parameters.osc1.breathDepth = nl;return; }
	if ( obj->objectName() == "osc1_breathAttain_dial" )	{ mididata->sendLiveControl( 14, 64, nl ); edit_patch.parameters.osc1.breathAttain = nl;return; }
	if ( obj->objectName() == "osc1_breathCurve_dial" )		{ mididata->sendLiveControl( 15, 64, nl ); edit_patch.parameters.osc1.breathCurve = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "osc1_breathThresh_dial" )	{ mididata->sendLiveControl( 16, 64, nl ); edit_patch.parameters.osc1.breathThreshold = nl;return; }
	
	if ( obj->objectName() == "osc2_fine_dial" )	{ mididata->sendLiveControl( 2, 65, nl ); edit_patch.parameters.osc2.fine = nl; statusBar()->showMessage( QString( "Value: %1 (%2 cents)" ).arg( nl ).arg( nl-64 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "osc2_beat_dial" )	{ mididata->sendLiveControl( 3, 65, nl ); edit_patch.parameters.osc2.beat = nl;return; }
	if ( obj->objectName() == "osc2_pulseWidth_dial" )	{ mididata->sendLiveControl( 8, 65, nl ); edit_patch.parameters.osc2.pulseWidth = nl;return; }
	if ( obj->objectName() == "osc2_PWMfreq_dial" )		{ mididata->sendLiveControl( 9, 65, nl ); edit_patch.parameters.osc2.PWMfreq = nl;return; }
	if ( obj->objectName() == "osc2_PWMdepth_dial" )	{ mididata->sendLiveControl( 10, 65, nl ); edit_patch.parameters.osc2.PWMdepth = nl;return; }
	if ( obj->objectName() == "osc2_sweepDepth_dial" )	{ mididata->sendLiveControl( 11, 65, nl ); edit_patch.parameters.osc2.sweepDepth = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "osc2_sweepTime_dial" )	{ mididata->sendLiveControl( 12, 65, nl ); edit_patch.parameters.osc2.sweepTime = nl;return; }
	if ( obj->objectName() == "osc2_breathDepth_dial" )	{ mididata->sendLiveControl( 13, 65, nl ); edit_patch.parameters.osc2.breathDepth = nl;return; }
	if ( obj->objectName() == "osc2_breathAttain_dial" )	{ mididata->sendLiveControl( 14, 65, nl ); edit_patch.parameters.osc2.breathAttain = nl;return; }
	if ( obj->objectName() == "osc2_breathCurve_dial" )		{ mididata->sendLiveControl( 15, 65, nl ); edit_patch.parameters.osc2.breathCurve = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "osc2_breathThresh_dial" )	{ mididata->sendLiveControl( 16, 65, nl ); edit_patch.parameters.osc2.breathThreshold = nl;return; }
	
		
	if ( obj->objectName() == "oscfilter1_Q_dial" )			{ mididata->sendLiveControl( 2, 72, nl ); edit_patch.parameters.oscFilter1.Q = nl; statusBar()->showMessage( QString( "Value: %1 (%2 Hz)" ).arg( nl ).arg( nl * 100 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "oscfilter1_keyFollow_dial" )	{ mididata->sendLiveControl( 3, 72, nl ); edit_patch.parameters.oscFilter1.keyFollow = nl; statusBar()->showMessage( QString( "Value: %1 (%2:1)" ).arg( nl ).arg( nl - 64 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "oscfilter1_breathMod_dial" )	{ mididata->sendLiveControl( 4, 72, nl ); edit_patch.parameters.oscFilter1.breathMod = nl; return; }
	if ( obj->objectName() == "oscfilter1_LFOfreq_dial" )	{ mididata->sendLiveControl( 5, 72, nl ); edit_patch.parameters.oscFilter1.LFOfreq = nl; return; }
	if ( obj->objectName() == "oscfilter1_LFOdepth_dial" )	{ mididata->sendLiveControl( 6, 72, nl ); edit_patch.parameters.oscFilter1.LFOdepth = nl; return; }
	if ( obj->objectName() == "oscfilter1_LFObreath_dial" )	{ mididata->sendLiveControl( 7, 72, nl ); edit_patch.parameters.oscFilter1.LFObreath = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "oscfilter1_LFOthreshold_dial" )	{ mididata->sendLiveControl( 8, 72, nl ); edit_patch.parameters.oscFilter1.LFOthreshold = nl; return; }
	if ( obj->objectName() == "oscfilter1_sweepDepth_dial" )	{ mididata->sendLiveControl( 9, 72, nl ); edit_patch.parameters.oscFilter1.sweepDepth = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "oscfilter1_sweepTime_dial" )		{ mididata->sendLiveControl( 10, 72, nl ); edit_patch.parameters.oscFilter1.sweepTime = nl; return; }
	if ( obj->objectName() == "oscfilter1_breathCurve_dial" )	{ mididata->sendLiveControl( 11, 72, nl ); edit_patch.parameters.oscFilter1.breathCurve = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
		
	if ( obj->objectName() == "oscfilter2_Q_dial" )			{ mididata->sendLiveControl( 2, 73, nl ); edit_patch.parameters.oscFilter2.Q = nl;statusBar()->showMessage( QString( "Value: %1 (%2 Hz)" ).arg( nl ).arg( nl * 100 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "oscfilter2_keyFollow_dial" )	{ mididata->sendLiveControl( 3, 73, nl ); edit_patch.parameters.oscFilter2.keyFollow = nl; statusBar()->showMessage( QString( "Value: %1 (%2:1)" ).arg( nl ).arg( nl - 64 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "oscfilter2_breathMod_dial" )	{ mididata->sendLiveControl( 4, 73, nl ); edit_patch.parameters.oscFilter2.breathMod = nl; return; }
	if ( obj->objectName() == "oscfilter2_LFOfreq_dial" )	{ mididata->sendLiveControl( 5, 73, nl ); edit_patch.parameters.oscFilter2.LFOfreq = nl; return; }
	if ( obj->objectName() == "oscfilter2_LFOdepth_dial" )	{ mididata->sendLiveControl( 6, 73, nl ); edit_patch.parameters.oscFilter2.LFOdepth = nl; return; }
	if ( obj->objectName() == "oscfilter2_LFObreath_dial" )	{ mididata->sendLiveControl( 7, 73, nl ); edit_patch.parameters.oscFilter2.LFObreath = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "oscfilter2_LFOthreshold_dial" )	{ mididata->sendLiveControl( 8, 73, nl ); edit_patch.parameters.oscFilter2.LFOthreshold = nl; return; }
	if ( obj->objectName() == "oscfilter2_sweepDepth_dial" )	{ mididata->sendLiveControl( 9, 73, nl ); edit_patch.parameters.oscFilter2.sweepDepth = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "oscfilter2_sweepTime_dial" )		{ mididata->sendLiveControl( 10, 73, nl ); edit_patch.parameters.oscFilter2.sweepTime = nl; return; }
	if ( obj->objectName() == "oscfilter2_breathCurve_dial" )	{ mididata->sendLiveControl( 11, 73, nl ); edit_patch.parameters.oscFilter2.breathCurve = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	
	if ( obj->objectName() == "noiseTime_dial" )			{ mididata->sendLiveControl( 0, 80, nl ); edit_patch.parameters.noiseTime = nl; return; }
	if ( obj->objectName() == "noiseBreath_dial" )			{ mididata->sendLiveControl( 1, 80, nl ); edit_patch.parameters.noiseBreath = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	
	if ( obj->objectName() == "noisefilter1_Q_dial" )			{ mididata->sendLiveControl( 2, 74, nl ); edit_patch.parameters.noiseFilter1.Q = nl;statusBar()->showMessage( QString( "Value: %1 (%2 Hz)" ).arg( nl ).arg( nl * 100 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "noisefilter1_keyFollow_dial" )	{ mididata->sendLiveControl( 3, 74, nl ); edit_patch.parameters.noiseFilter1.keyFollow = nl; statusBar()->showMessage( QString( "Value: %1 (%2:1)" ).arg( nl ).arg( nl - 64 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "noisefilter1_breathMod_dial" )	{ mididata->sendLiveControl( 4, 74, nl ); edit_patch.parameters.noiseFilter1.breathMod = nl; return; }
	if ( obj->objectName() == "noisefilter1_LFOfreq_dial" )		{ mididata->sendLiveControl( 5, 74, nl ); edit_patch.parameters.noiseFilter1.LFOfreq = nl; return; }
	if ( obj->objectName() == "noisefilter1_LFOdepth_dial" )	{ mididata->sendLiveControl( 6, 74, nl ); edit_patch.parameters.noiseFilter1.LFOdepth = nl; return; }
	if ( obj->objectName() == "noisefilter1_LFObreath_dial" )	{ mididata->sendLiveControl( 7, 74, nl ); edit_patch.parameters.noiseFilter1.LFObreath = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "noisefilter1_LFOthreshold_dial" )	{ mididata->sendLiveControl( 8, 74, nl ); edit_patch.parameters.noiseFilter1.LFOthreshold = nl; return; }
	if ( obj->objectName() == "noisefilter1_sweepDepth_dial" )		{mididata->sendLiveControl( 9, 74, nl ); edit_patch.parameters.noiseFilter1.sweepDepth = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "noisefilter1_sweepTime_dial" )		{ mididata->sendLiveControl( 10, 74, nl ); edit_patch.parameters.noiseFilter1.sweepTime = nl; return; }
	if ( obj->objectName() == "noisefilter1_breathCurve_dial" )		{mididata->sendLiveControl( 11, 74, nl ); edit_patch.parameters.noiseFilter1.breathCurve = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	
	if ( obj->objectName() == "noisefilter2_Q_dial" )			{ mididata->sendLiveControl( 2, 75, nl ); edit_patch.parameters.noiseFilter2.Q = nl;statusBar()->showMessage( QString( "Value: %1 (%2 Hz)" ).arg( nl ).arg( nl * 100 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "noisefilter2_keyFollow_dial" )	{ mididata->sendLiveControl( 3, 75, nl ); edit_patch.parameters.noiseFilter2.keyFollow = nl;statusBar()->showMessage( QString( "Value: %1 (%2:1)" ).arg( nl ).arg( nl - 64 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "noisefilter2_breathMod_dial" )	{ mididata->sendLiveControl( 4, 75, nl ); edit_patch.parameters.noiseFilter2.breathMod = nl; return; }
	if ( obj->objectName() == "noisefilter2_LFOfreq_dial" )		{ mididata->sendLiveControl( 5, 75, nl ); edit_patch.parameters.noiseFilter2.LFOfreq = nl; return; }
	if ( obj->objectName() == "noisefilter2_LFOdepth_dial" )	{ mididata->sendLiveControl( 6, 75, nl ); edit_patch.parameters.noiseFilter2.LFOdepth = nl; return; }
	if ( obj->objectName() == "noisefilter2_LFObreath_dial" )	{ mididata->sendLiveControl( 7, 75, nl ); edit_patch.parameters.noiseFilter2.LFObreath = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "noisefilter2_LFOthreshold_dial" )	{ mididata->sendLiveControl( 8, 75, nl ); edit_patch.parameters.noiseFilter2.LFOthreshold = nl; return; }
	if ( obj->objectName() == "noisefilter2_sweepDepth_dial" )		{mididata->sendLiveControl( 9, 75, nl ); edit_patch.parameters.noiseFilter2.sweepDepth = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "noisefilter2_sweepTime_dial" )		{ mididata->sendLiveControl( 10, 75, nl ); edit_patch.parameters.noiseFilter2.sweepTime = nl; return; }
	if ( obj->objectName() == "noisefilter2_breathCurve_dial" )		{mididata->sendLiveControl( 11, 75, nl ); edit_patch.parameters.noiseFilter2.breathCurve = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
		
	if ( obj->objectName() == "chorusDelay1_dial" )			{ mididata->sendLiveControl( 0, 112, nl ); edit_patch.parameters.chorusDelay1 = nl; statusBar()->showMessage( QString( "Value: %1 (%2 ms)" ).arg( nl ).arg( nl ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "chorusModLev1_dial" )		{ mididata->sendLiveControl( 1, 112, nl ); edit_patch.parameters.chorusModLev1 = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "chorusWetLev1_dial" )		{ mididata->sendLiveControl( 2, 112, nl ); edit_patch.parameters.chorusWetLev1 = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "chorusDelay2_dial" )			{ mididata->sendLiveControl( 3, 112, nl ); edit_patch.parameters.chorusDelay2 = nl; statusBar()->showMessage( QString( "Value: %1 (%2 ms)" ).arg( nl ).arg( nl ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "chorusModLev2_dial" )		{ mididata->sendLiveControl( 4, 112, nl ); edit_patch.parameters.chorusModLev2 = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "chorusWetLev2_dial" )		{ mididata->sendLiveControl( 5, 112, nl ); edit_patch.parameters.chorusWetLev2 = nl;statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT ); return; }
	if ( obj->objectName() == "chorusDryLevel_dial" )		{ mididata->sendLiveControl( 8, 112, nl ); edit_patch.parameters.chorusDryLevel = nl; return; }
	if ( obj->objectName() == "chorusFeedback_dial" )		{ mididata->sendLiveControl( 6, 112, nl ); edit_patch.parameters.chorusFeedback = nl; statusBar()->showMessage( QString( "Value: %1 (%2\%)" ).arg( nl ).arg( (100*nl/127) - 50 ), STATUS_MSG_TIMEOUT );return; }
	
	if ( obj->objectName() == "delayTime_dial" )			{ mididata->sendLiveControl( 0, 113, nl ); edit_patch.parameters.delayTime = nl; statusBar()->showMessage( QString( "Value: %1 (%2 ms)" ).arg( nl ).arg( nl * 10 ), STATUS_MSG_TIMEOUT );return; }
	if ( obj->objectName() == "delayFeedback_dial" )		{ mididata->sendLiveControl( 1, 113, nl ); edit_patch.parameters.delayFeedback = nl; return; }
	if ( obj->objectName() == "delayDamp_dial" )			{ mididata->sendLiveControl( 2, 113, nl ); edit_patch.parameters.delayDamp = nl; return; }
	if ( obj->objectName() == "delayMix_dial" )			{ mididata->sendLiveControl( 4, 113, nl ); edit_patch.parameters.delayMix = nl; return; }
	
	if ( obj->objectName() == "reverbTime_dial" )			{ mididata->sendLiveControl( 3, 114, nl ); edit_patch.parameters.reverbTime = nl; return; }
	if ( obj->objectName() == "reverbDensity_dial" )		{ mididata->sendLiveControl( 2, 114, nl ); edit_patch.parameters.reverbDensity = nl; return; }
	if ( obj->objectName() == "reverbDamp_dial" )			{ mididata->sendLiveControl( 4, 114, nl ); edit_patch.parameters.reverbDamp = nl; return; }
	if ( obj->objectName() == "reverbMix_dial" )			{ mididata->sendLiveControl( 0, 114, nl ); edit_patch.parameters.reverbMix = nl; return; }
	
	if ( obj->objectName() == "biteVibrato_dial" )			{ mididata->sendLiveControl( 2, 81, nl ); edit_patch.parameters.biteVibrato = nl; return; }
	if ( obj->objectName() == "biteTremolo_dial" )			{ mididata->sendLiveControl( 0, 88, nl ); edit_patch.parameters.biteTremolo = nl; return; }
	
	cerr << "Oops - unhandled change for dial\n";
}

void MainWindow::defaultPatch() {
	// just a plain triangle wave @ 75% volume
	osc1_octave_comboBox->setCurrentIndex( 2 );
	osc1_semitone_comboBox->setCurrentIndex( 12 );
	osc1_fine_dial->setValue( 64 );
	osc1_beat_dial->setValue( 64 );
	osc1_saw_verticalSlider->setValue( 0 );
	osc1_tri_verticalSlider->setValue( 127 );
	osc1_sqr_verticalSlider->setValue( 0 );
	osc1_pulseWidth_dial->setValue( 64 );
	osc1_PWMfreq_dial->setValue( 0 );
	osc1_PWMdepth_dial->setValue( 0 );
	osc1_sweepTime_dial->setValue( 0 );
	osc1_sweepDepth_dial->setValue( 64 );
	osc1_breathAttain_dial->setValue( 0 );
	osc1_breathDepth_dial->setValue( 0 );
	osc1_breathThresh_dial->setValue( 0 );
	osc1_breathCurve_dial->setValue( 64 );
	osc1_level_verticalSlider->setValue( 96 );
			// osc2 silent
	xfade_checkBox->setCheckState( Qt::Unchecked );
	osc2_octave_comboBox->setCurrentIndex( 2 );
	osc2_semitone_comboBox->setCurrentIndex( 12 );
	osc2_fine_dial->setValue( 64 );
	osc2_beat_dial->setValue( 64 );
	osc2_saw_verticalSlider->setValue( 0 );
	osc2_tri_verticalSlider->setValue( 0 );
	osc2_sqr_verticalSlider->setValue( 0 );
	osc2_pulseWidth_dial->setValue( 64 );
	osc2_PWMfreq_dial->setValue( 0 );
	osc2_PWMdepth_dial->setValue( 0 );
	osc2_sweepTime_dial->setValue( 0 );
	osc2_sweepDepth_dial->setValue( 64 );
	osc2_breathAttain_dial->setValue( 0 );
	osc2_breathDepth_dial->setValue( 0 );
	osc2_breathThresh_dial->setValue( 0 );
	osc2_breathCurve_dial->setValue( 64 );
	osc2_level_verticalSlider->setValue( 0 );
			// no filtering
	formantFilter_comboBox->setCurrentIndex( 0 );
	keyTrigger_comboBox->setCurrentIndex( 1 );
	// 
	oscFilterLink_comboBox->setCurrentIndex( 2 );
	oscfilter1_mode_comboBox->setCurrentIndex( 4 ); // off
	oscfilter1_freq_horizontalSlider->setValue( 0 );
	oscfilter1_Q_dial->setValue( 5 );
	oscfilter1_keyFollow_dial->setValue( 64 );
	oscfilter1_breathMod_dial->setValue( 0 );
	oscfilter1_breathCurve_dial->setValue( 64 );
	oscfilter1_LFOfreq_dial->setValue( 0 );
	oscfilter1_LFOdepth_dial->setValue( 0 );
	oscfilter1_LFObreath_dial->setValue( 64 );
	oscfilter1_LFOthreshold_dial->setValue( 0 );
	oscfilter1_sweepTime_dial->setValue( 0 );
	oscfilter1_sweepDepth_dial->setValue( 64 );
			
	oscfilter2_mode_comboBox->setCurrentIndex( 4 );
	oscfilter2_freq_horizontalSlider->setValue( 0 );
	oscfilter2_Q_dial->setValue( 5 );
	oscfilter2_keyFollow_dial->setValue( 64 );
	oscfilter2_breathMod_dial->setValue( 0 );
	oscfilter2_breathCurve_dial->setValue( 64 );
	oscfilter2_LFOfreq_dial->setValue( 0 );
	oscfilter2_LFOdepth_dial->setValue( 0 );
	oscfilter2_LFObreath_dial->setValue( 64 );
	oscfilter2_LFOthreshold_dial->setValue( 0 );
	oscfilter2_sweepTime_dial->setValue( 0 );
	oscfilter2_sweepDepth_dial->setValue( 64 );
	
	antiAliasSwitch_checkBox->setCheckState( Qt::Checked );
	antiAliasCutoff_horizontalSlider->setValue( 76 );
	antiAliasKeyFollow_dial->setValue( 12 );
	
	makeDry();
	deNoise();		
	// sensible volumes
	ampLevel_verticalSlider->setValue( 96 );
	octaveLevel_verticalSlider->setValue( 50 );	
}

void MainWindow::makeDry() {
	chorusSwitch_checkBox->setCheckState( Qt::Unchecked );
	chorusDelay1_dial->setValue( 0 );
	chorusModLev1_dial->setValue( 0 );
	chorusWetLev1_dial->setValue( 0 );
	chorusDelay2_dial->setValue( 0 );
	chorusModLev2_dial->setValue( 0 );
	chorusWetLev2_dial->setValue( 0 );
	chorusDryLevel_dial->setValue( 64 );
	chorusFeedback_dial->setValue( 0 );
	chorusLFOfreq_horizontalSlider->setValue( 0 );
	delayTime_dial->setValue( 0 );
	delayDamp_dial->setValue( 0 );
	delayFeedback_dial->setValue( 0 );
	delayMix_dial->setValue( 127 );		
	delayLevel_verticalSlider->setValue( 0 );
	reverbTime_dial->setValue( 0 );
	reverbDamp_dial->setValue( 0 );
	reverbMix_dial->setValue( 127 ); 		
	reverbDensity_dial->setValue( 0 );
	reverbLevel_verticalSlider->setValue( 0 );
}

void MainWindow::deNoise() {
	noiseTime_dial->setValue( 0 );
	noiseBreath_dial->setValue( 0 );
	noiseLevel_verticalSlider->setValue( 0 );
	noiseFilterLink_comboBox->setCurrentIndex( 2 );
	noisefilter1_mode_comboBox->setCurrentIndex( 4 );
	noisefilter1_freq_horizontalSlider->setValue( 0 );
	noisefilter1_Q_dial->setValue( 5 );
	noisefilter1_keyFollow_dial->setValue( 64 );
	noisefilter1_breathMod_dial->setValue( 0 );
	noisefilter1_breathCurve_dial->setValue( 64 );
	noisefilter1_LFOfreq_dial->setValue( 0 );
	noisefilter1_LFOdepth_dial->setValue( 0 );
	noisefilter1_LFObreath_dial->setValue( 64 );
	noisefilter1_LFOthreshold_dial->setValue( 0 );
	noisefilter1_sweepTime_dial->setValue( 0 );
	noisefilter1_sweepDepth_dial->setValue( 64 );
	noisefilter2_mode_comboBox->setCurrentIndex( 4 );
	noisefilter2_freq_horizontalSlider->setValue( 0 );
	noisefilter2_Q_dial->setValue( 5 );
	noisefilter2_keyFollow_dial->setValue( 64 );
	noisefilter2_breathMod_dial->setValue( 0 );
	noisefilter2_breathCurve_dial->setValue( 64 );
	noisefilter2_LFOfreq_dial->setValue( 0 );
	noisefilter2_LFOdepth_dial->setValue( 0 );
	noisefilter2_LFObreath_dial->setValue( 64 );
	noisefilter2_LFOthreshold_dial->setValue( 0 );
	noisefilter2_sweepTime_dial->setValue( 0 );
	noisefilter2_sweepDepth_dial->setValue( 64 );
}

void MainWindow::randomPatch() {
	
	osc1_octave_comboBox->setCurrentIndex( 2 );		// we'll keep the base octave and semitone standard (8')
	osc1_semitone_comboBox->setCurrentIndex( 12 );
	osc1_fine_dial->setValue( randBetween( 0, 127 ) );
	osc1_beat_dial->setValue( randBetween( 0, 127 ) );
	osc1_saw_verticalSlider->setValue( randBetween( 0, 127 ) );
	osc1_tri_verticalSlider->setValue( randBetween( 0, 127 ) );
	osc1_sqr_verticalSlider->setValue( randBetween( 0, 127 ) );
	osc1_pulseWidth_dial->setValue( randBetween( 0, 127 ) );
	osc1_PWMfreq_dial->setValue( randBetween( 0, 127 ) );
	osc1_PWMdepth_dial->setValue( randBetween( 0, 127 ) );
	osc1_sweepTime_dial->setValue( randBetween( 0, 127 ) );
	osc1_sweepDepth_dial->setValue( randBetween( 0, 127 ) );
	osc1_breathAttain_dial->setValue( randBetween( 0, 127 ) );
	osc1_breathDepth_dial->setValue( randBetween( 0, 127 ) );
	osc1_breathThresh_dial->setValue( randBetween( 0, 127 ) );
	osc1_breathCurve_dial->setValue( randBetween( 0, 127 ) );
	osc1_level_verticalSlider->setValue( randBetween( 48, 127 ) );  // we want some volume

	if (randBetween( 0, 1) == 0)
		xfade_checkBox->setCheckState( Qt::Unchecked );
	else
		xfade_checkBox->setCheckState( Qt::Checked );
	
	osc2_octave_comboBox->setCurrentIndex( randBetween( 0, 4 ) );
	osc2_semitone_comboBox->setCurrentIndex( 12 );		//maybe keep this sane for now
	osc2_fine_dial->setValue( randBetween( 0, 127 ) );
	osc2_beat_dial->setValue( randBetween( 0, 127 ) );
	osc2_saw_verticalSlider->setValue( randBetween( 0, 127 ) );
	osc2_tri_verticalSlider->setValue( randBetween( 0, 127 ) );
	osc2_sqr_verticalSlider->setValue( randBetween( 0, 127 ) );
	osc2_pulseWidth_dial->setValue( randBetween( 0, 127 ) );
	osc2_PWMfreq_dial->setValue( randBetween( 0, 127 ) );
	osc2_PWMdepth_dial->setValue( randBetween( 0, 127 ) );
	osc2_sweepTime_dial->setValue( randBetween( 0, 127 ) );
	osc2_sweepDepth_dial->setValue( randBetween( 0, 127 ) );
	osc2_breathAttain_dial->setValue( randBetween( 0, 127 ) );
	osc2_breathDepth_dial->setValue( randBetween( 0, 127 ) );
	osc2_breathThresh_dial->setValue( randBetween( 0, 127 ) );
	osc2_breathCurve_dial->setValue( randBetween( 0, 127 ) );
	osc2_level_verticalSlider->setValue( randBetween( 0, 127 ) );  
	
	formantFilter_comboBox->setCurrentIndex( randBetween( 0, 2 ) );
	keyTrigger_comboBox->setCurrentIndex( randBetween( 0, 1 ) );
	// 
	oscFilterLink_comboBox->setCurrentIndex( randBetween( 0, 2 ) );
	
	oscfilter1_mode_comboBox->setCurrentIndex( randBetween( 0, 4 ) ); 
	oscfilter1_freq_horizontalSlider->setValue( randBetween( 0, 127 ) );
	oscfilter1_Q_dial->setValue( randBetween( 5, 127 ) );
	oscfilter1_keyFollow_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_breathMod_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_breathCurve_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_LFOfreq_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_LFOdepth_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_LFObreath_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_LFOthreshold_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_sweepTime_dial->setValue( randBetween( 0, 127 ) );
	oscfilter1_sweepDepth_dial->setValue( randBetween( 0, 127 ) );
			
	oscfilter2_mode_comboBox->setCurrentIndex( randBetween( 0, 4 ) ); 
	oscfilter2_freq_horizontalSlider->setValue( randBetween( 0, 127 ) );
	oscfilter2_Q_dial->setValue( randBetween( 5, 127 ) );
	oscfilter2_keyFollow_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_breathMod_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_breathCurve_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_LFOfreq_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_LFOdepth_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_LFObreath_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_LFOthreshold_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_sweepTime_dial->setValue( randBetween( 0, 127 ) );
	oscfilter2_sweepDepth_dial->setValue( randBetween( 0, 127 ) );
	
	if (randBetween( 0, 1) == 0)
		chorusSwitch_checkBox->setCheckState( Qt::Unchecked );
	else
		chorusSwitch_checkBox->setCheckState( Qt::Checked );
	chorusDelay1_dial->setValue( randBetween( 0, 127 ) );
	chorusModLev1_dial->setValue( randBetween( 0, 127 ) );
	chorusWetLev1_dial->setValue( randBetween( 0, 127 ) );
	chorusDelay2_dial->setValue( randBetween( 0, 127 ) );
	chorusModLev2_dial->setValue( randBetween( 0, 127 ) );
	chorusWetLev2_dial->setValue( randBetween( 0, 127 ) );
	chorusDryLevel_dial->setValue( randBetween( 0, 127 ) );
	chorusFeedback_dial->setValue( randBetween( 0, 127 ) );
	chorusLFOfreq_horizontalSlider->setValue( randBetween( 0, 127 ) );
	delayTime_dial->setValue( randBetween( 0, 127 ) );
	delayDamp_dial->setValue( randBetween( 0, 127 ) );
	delayFeedback_dial->setValue( randBetween( 0, 127 ) );
	delayMix_dial->setValue( 127 );							// ...
	delayLevel_verticalSlider->setValue( randBetween( 0, 127 ) );
	reverbTime_dial->setValue( randBetween( 0, 127 ) );
	reverbDamp_dial->setValue( randBetween( 0, 127 ) );
	reverbDensity_dial->setValue( randBetween( 0, 127 ) );
	reverbMix_dial->setValue( 127 );						//  seems necessary
	reverbLevel_verticalSlider->setValue( randBetween( 0, 127 ) );
}

void MainWindow::randomisePatch() {
	
	//sc1_octave_comboBox->setCurrentIndex( 2 );		// we'll keep the base octave and semitone standard (8')
	//osc1_semitone_comboBox->setCurrentIndex( 12 );
	osc1_fine_dial->setValue( randNear( 0, 127, osc1_fine_dial->value() ) );
	osc1_beat_dial->setValue( randNear( 0, 127, osc1_beat_dial->value() ) );
	osc1_saw_verticalSlider->setValue( randNear( 0, 127, osc1_saw_verticalSlider->value() ) );
	osc1_tri_verticalSlider->setValue( randNear( 0, 127, osc1_tri_verticalSlider->value() ) );
	osc1_sqr_verticalSlider->setValue( randNear( 0, 127, osc1_sqr_verticalSlider->value() ) );
	osc1_pulseWidth_dial->setValue( randNear( 0, 127, osc1_pulseWidth_dial->value() ) );
	osc1_PWMfreq_dial->setValue( randNear( 0, 127, osc1_PWMfreq_dial->value() ) );
	osc1_PWMdepth_dial->setValue( randNear( 0, 127, osc1_PWMdepth_dial->value() ) );
	osc1_sweepTime_dial->setValue( randNear( 0, 127, osc1_sweepTime_dial->value() ) );
	osc1_sweepDepth_dial->setValue( randNear( 0, 127, osc1_sweepDepth_dial->value() ) );
	osc1_breathAttain_dial->setValue( randNear( 0, 127, osc1_breathAttain_dial->value() ) );
	osc1_breathDepth_dial->setValue( randNear( 0, 127, osc1_breathDepth_dial->value() ) );
	osc1_breathThresh_dial->setValue( randNear( 0, 127, osc1_breathThresh_dial->value() ) );
	osc1_breathCurve_dial->setValue( randNear( 0, 127, osc1_breathCurve_dial->value() ) );
	osc1_level_verticalSlider->setValue( randNear( 48, 127, osc1_level_verticalSlider->value() ) );  // we want some volume

	//osc2_octave_comboBox->setCurrentIndex( randNear( 0, 4 ) );
	//osc2_semitone_comboBox->setCurrentIndex( 12 );		//maybe keep this sane for now
	osc2_fine_dial->setValue( randNear( 0, 127, osc2_fine_dial->value() ) );
	osc2_beat_dial->setValue( randNear( 0, 127, osc2_beat_dial->value() ) );
	osc2_saw_verticalSlider->setValue( randNear( 0, 127, osc2_saw_verticalSlider->value() ) );
	osc2_tri_verticalSlider->setValue( randNear( 0, 127, osc2_tri_verticalSlider->value() ) );
	osc2_sqr_verticalSlider->setValue( randNear( 0, 127, osc2_sqr_verticalSlider->value() ) );
	osc2_pulseWidth_dial->setValue( randNear( 0, 127, osc2_pulseWidth_dial->value() ) );
	osc2_PWMfreq_dial->setValue( randNear( 0, 127, osc2_PWMfreq_dial->value() ) );
	osc2_PWMdepth_dial->setValue( randNear( 0, 127, osc2_PWMdepth_dial->value() ) );
	osc2_sweepTime_dial->setValue( randNear( 0, 127, osc2_sweepTime_dial->value() ) );
	osc2_sweepDepth_dial->setValue( randNear( 0, 127, osc2_sweepDepth_dial->value() ) );
	osc2_breathAttain_dial->setValue( randNear( 0, 127, osc2_breathAttain_dial->value() ) );
	osc2_breathDepth_dial->setValue( randNear( 0, 127, osc2_breathDepth_dial->value() ) );
	osc2_breathThresh_dial->setValue( randNear( 0, 127, osc2_breathThresh_dial->value() ) );
	osc2_breathCurve_dial->setValue( randNear( 0, 127, osc2_breathCurve_dial->value() ) );
	osc2_level_verticalSlider->setValue( randNear( 48, 127, osc2_level_verticalSlider->value() ) );
	
	//formantFilter_comboBox->setCurrentIndex( randNear( 0, 2 ) );
	//keyTrigger_comboBox->setCurrentIndex( randNear( 0, 1 ) );
	// 
	//oscFilterLink_comboBox->setCurrentIndex( randNear( 0, 2 ) );
	
	//oscfilter1_mode_comboBox->setCurrentIndex( randNear( 0, 4 ) ); 
	oscfilter1_freq_horizontalSlider->setValue( randNear( 0, 127, oscfilter1_freq_horizontalSlider->value() ) );
	oscfilter1_Q_dial->setValue( randNear( 5, 127, oscfilter1_Q_dial->value() ) );
	oscfilter1_keyFollow_dial->setValue( randNear( 0, 127, oscfilter1_keyFollow_dial->value() ) );
	oscfilter1_breathMod_dial->setValue( randNear( 0, 127, oscfilter1_breathMod_dial->value() ) );
	oscfilter1_breathCurve_dial->setValue( randNear( 0, 127, oscfilter1_breathCurve_dial->value() ) );
	oscfilter1_LFOfreq_dial->setValue( randNear( 0, 127, oscfilter1_LFOfreq_dial->value() ) );
	oscfilter1_LFOdepth_dial->setValue( randNear( 0, 127, oscfilter1_LFOdepth_dial->value() ) );
	oscfilter1_LFObreath_dial->setValue( randNear( 0, 127, oscfilter1_LFObreath_dial->value() ) );
	oscfilter1_LFOthreshold_dial->setValue( randNear( 0, 127, oscfilter1_LFOthreshold_dial->value() ) );
	oscfilter1_sweepTime_dial->setValue( randNear( 0, 127, oscfilter1_sweepTime_dial->value() ) );
	oscfilter1_sweepDepth_dial->setValue( randNear( 0, 127, oscfilter1_sweepDepth_dial->value() ) );
			
	//oscfilter2_mode_comboBox->setCurrentIndex( randNear( 0, 4 ) ); 
	oscfilter2_freq_horizontalSlider->setValue( randNear( 0, 127, oscfilter2_freq_horizontalSlider->value() ) );
	oscfilter2_Q_dial->setValue( randNear( 5, 127, oscfilter2_Q_dial->value() ) );
	oscfilter2_keyFollow_dial->setValue( randNear( 0, 127, oscfilter2_keyFollow_dial->value() ) );
	oscfilter2_breathMod_dial->setValue( randNear( 0, 127, oscfilter2_breathMod_dial->value() ) );
	oscfilter2_breathCurve_dial->setValue( randNear( 0, 127, oscfilter2_breathCurve_dial->value() ) );
	oscfilter2_LFOfreq_dial->setValue( randNear( 0, 127, oscfilter2_LFOfreq_dial->value() ) );
	oscfilter2_LFOdepth_dial->setValue( randNear( 0, 127, oscfilter2_LFOdepth_dial->value() ) );
	oscfilter2_LFObreath_dial->setValue( randNear( 0, 127, oscfilter2_LFObreath_dial->value() ) );
	oscfilter2_LFOthreshold_dial->setValue( randNear( 0, 127, oscfilter2_LFOthreshold_dial->value() ) );
	oscfilter2_sweepTime_dial->setValue( randNear( 0, 127, oscfilter2_sweepTime_dial->value() ) );
	oscfilter2_sweepDepth_dial->setValue( randNear( 0, 127, oscfilter2_sweepDepth_dial->value() ) );
	
	chorusDelay1_dial->setValue( randNear( 0, 127, chorusDelay1_dial->value() ) );
	chorusModLev1_dial->setValue( randNear( 0, 127, chorusModLev1_dial->value() ) );
	chorusWetLev1_dial->setValue( randNear( 0, 127, chorusWetLev1_dial->value() ) );
	chorusDelay2_dial->setValue( randNear( 0, 127, chorusDelay2_dial->value() ) );
	chorusModLev2_dial->setValue( randNear( 0, 127, chorusModLev2_dial->value() ) );
	chorusWetLev2_dial->setValue( randNear( 0, 127, chorusWetLev2_dial->value() ) );
	chorusDryLevel_dial->setValue( randNear( 0, 127, chorusDryLevel_dial->value() ) );
	chorusFeedback_dial->setValue( randNear( 0, 127, chorusFeedback_dial->value() ) );
	chorusLFOfreq_horizontalSlider->setValue( randNear( 0, 127, chorusLFOfreq_horizontalSlider->value() ) );
	delayTime_dial->setValue( randNear( 0, 127, delayTime_dial->value() ) );
	delayDamp_dial->setValue( randNear( 0, 127, delayDamp_dial->value() ) );
	delayFeedback_dial->setValue( randNear( 0, 127, delayFeedback_dial->value() ) );
	delayMix_dial->setValue( randNear( 0, 127, delayMix_dial->value() ) );
	delayLevel_verticalSlider->setValue( randNear( 0, 127, delayLevel_verticalSlider->value() ) );
	reverbTime_dial->setValue( randNear( 0, 127, reverbTime_dial->value() ) );
	reverbDamp_dial->setValue( randNear( 0, 127, reverbDamp_dial->value() ) );
	reverbMix_dial->setValue( randNear( 0, 127, reverbMix_dial->value() ) );
	reverbDensity_dial->setValue( randNear( 0, 127, reverbDensity_dial->value() ) );
	reverbLevel_verticalSlider->setValue( randNear( 0, 127, reverbLevel_verticalSlider->value() ) );
}

void MainWindow::mixInPatch( int mp, int pct ) {
	//sc1_octave_comboBox->setCurrentIndex( 2 );		// we'll keep the base octave and semitone standard (8')
	//osc1_semitone_comboBox->setCurrentIndex( 12 );
	osc1_fine_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.fine, pct, osc1_fine_dial->value() ) );
	osc1_beat_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.beat, pct, osc1_beat_dial->value() ) );
	osc1_saw_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.osc1.sawtooth, pct,  osc1_saw_verticalSlider->value() ) );
	osc1_tri_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.osc1.triangle, pct,  osc1_tri_verticalSlider->value() ) );
	osc1_sqr_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.osc1.square, pct,  osc1_sqr_verticalSlider->value() ) );
	osc1_pulseWidth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.pulseWidth, pct,  osc1_pulseWidth_dial->value() ) );
	osc1_PWMfreq_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.PWMfreq, pct,  osc1_PWMfreq_dial->value() ) );
	osc1_PWMdepth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.PWMdepth, pct,  osc1_PWMdepth_dial->value() ) );
	osc1_sweepTime_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.sweepTime, pct,  osc1_sweepTime_dial->value() ) );
	osc1_sweepDepth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.sweepDepth, pct,  osc1_sweepDepth_dial->value() ) );
	osc1_breathAttain_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.breathAttain, pct,  osc1_breathAttain_dial->value() ) );
	osc1_breathDepth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.breathDepth, pct,  osc1_breathDepth_dial->value() ) );
	osc1_breathThresh_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.breathThreshold, pct,  osc1_breathThresh_dial->value() ) );
	osc1_breathCurve_dial->setValue( mixInts( mididata->patches[mp].parameters.osc1.breathCurve, pct,  osc1_breathCurve_dial->value() ) );
	osc1_level_verticalSlider->setValue(mixInts( mididata->patches[mp].parameters.osc1.level, pct,  osc1_level_verticalSlider->value() ) );  

	//osc2_octave_comboBox->setCurrentIndex( randNear( 0, 4 ) );
	//osc2_semitone_comboBox->setCurrentIndex( 12 );		//maybe keep this sane for now
	osc2_fine_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.fine, pct, osc2_fine_dial->value() ) );
	osc2_beat_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.beat, pct,  osc2_beat_dial->value() ) );
	osc2_saw_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.osc2.sawtooth, pct,  osc2_saw_verticalSlider->value() ) );
	osc2_tri_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.osc2.triangle, pct,  osc2_tri_verticalSlider->value() ) );
	osc2_sqr_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.osc2.square, pct,  osc2_sqr_verticalSlider->value() ) );
	osc2_pulseWidth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.pulseWidth, pct,  osc2_pulseWidth_dial->value() ) );
	osc2_PWMfreq_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.PWMfreq, pct,  osc2_PWMfreq_dial->value() ) );
	osc2_PWMdepth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.PWMdepth, pct,  osc2_PWMdepth_dial->value() ) );
	osc2_sweepTime_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.sweepTime, pct,  osc2_sweepTime_dial->value() ) );
	osc2_sweepDepth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.sweepDepth, pct,  osc2_sweepDepth_dial->value() ) );
	osc2_breathAttain_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.breathAttain, pct,  osc2_breathAttain_dial->value() ) );
	osc2_breathDepth_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.breathDepth, pct,  osc2_breathDepth_dial->value() ) );
	osc2_breathThresh_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.breathThreshold, pct,  osc2_breathThresh_dial->value() ) );
	osc2_breathCurve_dial->setValue( mixInts( mididata->patches[mp].parameters.osc2.breathCurve, pct,  osc2_breathCurve_dial->value() ) );
	osc2_level_verticalSlider->setValue(mixInts( mididata->patches[mp].parameters.osc2.level, pct,  osc2_level_verticalSlider->value() ) );  
	
	//formantFilter_comboBox->setCurrentIndex( randNear( 0, 2 ) );
	//keyTrigger_comboBox->setCurrentIndex( randNear( 0, 1 ) );
	// 
	//oscFilterLink_comboBox->setCurrentIndex( randNear( 0, 2 ) );
	
	//oscfilter1_mode_comboBox->setCurrentIndex( randNear( 0, 4 ) ); 
	oscfilter1_freq_horizontalSlider->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.freq, pct,  oscfilter1_freq_horizontalSlider->value() ) );
	oscfilter1_Q_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.Q, pct,  oscfilter1_Q_dial->value() ) );
	oscfilter1_keyFollow_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.keyFollow, pct,  oscfilter1_keyFollow_dial->value() ) );
	oscfilter1_breathMod_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.breathMod, pct,  oscfilter1_breathMod_dial->value() ) );
	oscfilter1_breathCurve_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.breathCurve, pct,  oscfilter1_breathCurve_dial->value() ) );
	oscfilter1_LFOfreq_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.LFOfreq, pct,  oscfilter1_LFOfreq_dial->value() ) );
	oscfilter1_LFOdepth_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.LFOdepth, pct,  oscfilter1_LFOdepth_dial->value() ) );
	oscfilter1_LFObreath_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.LFObreath, pct,  oscfilter1_LFObreath_dial->value() ) );
	oscfilter1_LFOthreshold_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.LFOthreshold, pct,  oscfilter1_LFOthreshold_dial->value() ) );
	oscfilter1_sweepTime_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.sweepTime, pct,  oscfilter1_sweepTime_dial->value() ) );
	oscfilter1_sweepDepth_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter1.sweepDepth, pct,  oscfilter1_sweepDepth_dial->value() ) );
			
	//oscfilter2_mode_comboBox->setCurrentIndex( randNear( 0, 4 ) ); 
	oscfilter2_freq_horizontalSlider->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.freq, pct,  oscfilter2_freq_horizontalSlider->value() ) );
	oscfilter2_Q_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.Q, pct,  oscfilter2_Q_dial->value() ) );
	oscfilter2_keyFollow_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.keyFollow, pct,  oscfilter2_keyFollow_dial->value() ) );
	oscfilter2_breathMod_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.breathMod, pct,  oscfilter2_breathMod_dial->value() ) );
	oscfilter2_breathCurve_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.breathCurve, pct,  oscfilter2_breathCurve_dial->value() ) );
	oscfilter2_LFOfreq_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.LFOfreq, pct,  oscfilter2_LFOfreq_dial->value() ) );
	oscfilter2_LFOdepth_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.LFOdepth, pct,  oscfilter2_LFOdepth_dial->value() ) );
	oscfilter2_LFObreath_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.LFObreath, pct,  oscfilter2_LFObreath_dial->value() ) );
	oscfilter2_LFOthreshold_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.LFOthreshold, pct,  oscfilter2_LFOthreshold_dial->value() ) );
	oscfilter2_sweepTime_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.sweepTime, pct,  oscfilter2_sweepTime_dial->value() ) );
	oscfilter2_sweepDepth_dial->setValue( mixInts( mididata->patches[mp].parameters.oscFilter2.sweepDepth, pct,  oscfilter2_sweepDepth_dial->value() ) );
	
	chorusDelay1_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusDelay1, pct,  chorusDelay1_dial->value() ) );
	chorusModLev1_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusModLev1, pct,  chorusModLev1_dial->value() ) );
	chorusWetLev1_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusWetLev1, pct,  chorusWetLev1_dial->value() ) );
	chorusDelay2_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusDelay2, pct,  chorusDelay2_dial->value() ) );
	chorusModLev2_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusModLev2, pct,  chorusModLev2_dial->value() ) );
	chorusWetLev2_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusWetLev2, pct,  chorusWetLev2_dial->value() ) );
	chorusDryLevel_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusDryLevel, pct,  chorusDryLevel_dial->value() ) );
	chorusFeedback_dial->setValue( mixInts( mididata->patches[mp].parameters.chorusFeedback, pct,  chorusFeedback_dial->value() ) );
	chorusLFOfreq_horizontalSlider->setValue( mixInts( mididata->patches[mp].parameters.chorusLFOfreq, pct,  chorusLFOfreq_horizontalSlider->value() ) );
	delayTime_dial->setValue( mixInts( mididata->patches[mp].parameters.delayTime, pct,  delayTime_dial->value() ) );
	delayDamp_dial->setValue( mixInts( mididata->patches[mp].parameters.delayDamp, pct,  delayDamp_dial->value() ) );
	delayFeedback_dial->setValue( mixInts( mididata->patches[mp].parameters.delayFeedback, pct,  delayFeedback_dial->value() ) );
	delayMix_dial->setValue( mixInts( mididata->patches[mp].parameters.delayMix, pct,  delayMix_dial->value() ) );
	delayLevel_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.delayLevel, pct,  delayLevel_verticalSlider->value() ) );
	reverbTime_dial->setValue( mixInts( mididata->patches[mp].parameters.reverbTime, pct,  reverbTime_dial->value() ) );
	reverbDamp_dial->setValue( mixInts( mididata->patches[mp].parameters.reverbDamp, pct,  reverbDamp_dial->value() ) );
	reverbMix_dial->setValue( mixInts( mididata->patches[mp].parameters.reverbMix, pct,  reverbMix_dial->value() ) );
	reverbDensity_dial->setValue( mixInts( mididata->patches[mp].parameters.reverbDensity, pct,  reverbDensity_dial->value() ) );
	reverbLevel_verticalSlider->setValue( mixInts( mididata->patches[mp].parameters.reverbLevel, pct,  reverbLevel_verticalSlider->value() ) );
}

void MainWindow::mergePatch() {
	
	mergePatch_dialog *d = new mergePatch_dialog( mididata->patches );
	if (d->exec()) {
		mixInPatch( d->chosenRow, 50 );
	}
	delete d;
}

// library handling...

void MainWindow::setList_chosen(QListWidgetItem *item) {
	
	// a patch set has been chosen, we'll open it from disk, display 
	// the patches it contains and store it in patchSet[]
	
	QString file_name = libraryLocation + "/" + item->text();
	QFile file( file_name );
	if (!file.open(QFile::ReadOnly)) {
		QMessageBox::warning(this, tr("EWItool"),
							 tr("Cannot read file %1:\n%2.")
									 .arg(file_name)
									 .arg(file.errorString()));
		return;
	}
	
	setContents_listWidget->clear();
	
	QDataStream inp(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	for (int p = 0; p < EWI_NUM_PATCHES; p++) {
		inp.readRawData( patchSet[p].whole_patch, EWI_PATCH_LENGTH );
		setContents_listWidget->addItem( trimPatchName( patchSet[p].parameters.name ) );
	}
	QApplication::restoreOverrideCursor();
	
	libraryName = item->text();
	deleteSet_pushButton->setEnabled( true );
	sendLibraryToEWI_pushButton->setEnabled( true );
}

void MainWindow::deletePatchSet() {
	
	if (QMessageBox::question (this, "EWItool",
		tr ("Do you realy want to delete this Patch Set from disk?" ),
		QMessageBox::No | QMessageBox::Yes
							  ) ==  QMessageBox::No) return;
	
	QString file_name = libraryLocation + "/" + libraryName;
	QFile file( file_name );
	file.remove();
	setupLibraryTab();
}

void MainWindow::sendLibraryToEWI() {

	if (QMessageBox::question( this, "EWItool",
		tr( "This will overwrite all patches in the EWI with the chosen library.\n"
				"Do you really want to do this?" ),
		QMessageBox::No | QMessageBox::Yes
							 )
		   ==
		   QMessageBox::No) return;
	
	QProgressDialog progressDialog (tr ("Sending Library to the EWI"), 0, 1, 100,this);
	progressDialog.setWindowTitle (tr ("Sending Patches"));

	// we have to send one patch at a time (ALSA size limitation)
	for (int p = 0; p < EWI_NUM_PATCHES; p++) {
		mididata->sendPatch( patchSet[p], EWI_SAVE );
		progressDialog.setValue( p );
		progressDialog.setLabelText(tr("Sending patch number %1 of %2...").arg(p).arg( EWI_NUM_PATCHES ));
		mididata->patches[p] = patchSet[p];  // copy into mididata
		EWI_patch_name[p]->setText( trimPatchName( mididata->patches[p].parameters.name ) );  // update EWI tab labels
		qApp->processEvents();
	}

}


void MainWindow::copyToClipboard() {
	
	QModelIndexList indexes = setContents_listWidget->selectionModel()->selectedIndexes();
	foreach(QModelIndex index, indexes)	{
		// check item not already on the clipboard
		bool already = false;
		for (int i = 0; i < clipboard.size(); i++) {
			if ( trimPatchName( (char *) clipboard.at(i).parameters.name ) == trimPatchName( patchSet[index.row()].parameters.name ) ) {
				already = true;
				break;
			}
		}
		if (!already) {
			clipboard_listWidget->addItem( trimPatchName( patchSet[index.row()].parameters.name ) );
			clipboard.append( patchSet[index.row()] );
		}
	} 
	setContents_listWidget->clearSelection();
	
	saveClipboard();
}

void MainWindow::copyEWIPatch( int p ) {
	edit_patch = mididata->patches[p];
	clipboard_listWidget->addItem( trimPatchName( edit_patch.parameters.name ) );
	clipboard.append( edit_patch );
	
	saveClipboard();
}

void MainWindow::pasteEWIPatch( int p ) {
	edit_patch = mididata->patches[p];
	
	pastePatch_dialog *d = new pastePatch_dialog( (int) edit_patch.parameters.patch_num, &clipboard);
	if (d->exec()) {
		// set the new patch number in the patch
		clipboard[d->chosenRow].parameters.patch_num = edit_patch.parameters.patch_num;
		// copy into the working patch set and editing patch
		patchSet[edit_patch.parameters.patch_num] = clipboard[d->chosenRow];
		edit_patch = clipboard[d->chosenRow];
		// and in the MIDI structure as we're not going to waste time re-reading it from the EWI
		mididata->patches[edit_patch.parameters.patch_num] = clipboard[d->chosenRow];
		// save in the EWI
		mididata->sendPatch( edit_patch, EWI_SAVE ); 
		// update the displayed list
		EWIList->setLabel( p, trimPatchName( patchSet[edit_patch.parameters.patch_num].parameters.name ) );
	}
	delete d;
}

void MainWindow::renameEWIPatch( int p ) {
	edit_patch = mididata->patches[p];
	bool ok;
	QString text = trimPatchName( edit_patch.parameters.name );
	QString new_name = QInputDialog::getText(this, "EWItool", "New Patch Name", QLineEdit::Normal, text, &ok);
	if (!ok || new_name.isEmpty()) return;

	new_name = new_name.simplified();

	if (new_name.length() > EWI_PATCHNAME_LENGTH ) { 
		QMessageBox::warning(this, tr("EWItool"),
							 tr("Patch name too long!\nEnter up to %1 characters").arg(EWI_PATCHNAME_LENGTH));
		
		renameEWIPatch( p );
	}
	EWIList->setLabel( p, new_name );
	QByteArray ba = new_name.leftJustified( EWI_PATCHNAME_LENGTH, ' ' ).toLatin1();
	memcpy( (void *) &edit_patch.parameters.name, (void *) ba.data(), EWI_PATCHNAME_LENGTH);
	memcpy( (void *) &patchSet[edit_patch.parameters.patch_num].parameters.name, (void *) ba.data(), EWI_PATCHNAME_LENGTH);
	memcpy( (void *) &mididata->patches[edit_patch.parameters.patch_num].parameters.name, (void *) ba.data(), EWI_PATCHNAME_LENGTH);
	// save in the EWI
	mididata->sendPatch( edit_patch, EWI_SAVE ); 
}

void MainWindow::clearClipboard() {
	
	clipboard_listWidget->clear();
	clipboard.clear();
	
	QString fileName = libraryLocation + "/CLIPBOARD.CLP";
	QFile file(fileName);
	file.remove();
}

void MainWindow::deleteClipboard() { 
	
	int row = clipboard_listWidget->currentRow();
	if (row != -1) {
		delete clipboard_listWidget->takeItem( row );
		clipboard.removeAt( row );
	}
	saveClipboard();
}

void MainWindow::renameClipboard() {

	if (clipboard.count() > 0 && clipboard_listWidget->currentRow() > -1) {

		// Get new name from the user
		bool ok;

		QListWidgetItem *curitem = clipboard_listWidget->currentItem();
		int r = clipboard_listWidget->row (curitem);
		QString text = curitem->text();
		QString new_name = QInputDialog::getText (this, "EWItool", "New Patch Name", QLineEdit::Normal, text, &ok);

		if (!ok || new_name.isEmpty()) return;

		new_name = new_name.simplified();

		if (new_name.length() > EWI_PATCHNAME_LENGTH) {
			QMessageBox::warning (this, tr ("EWItool"),
			                      tr ("Patch name too long!\nEnter up to %1 characters").arg (EWI_PATCHNAME_LENGTH));
			renameClipboard();
		}

		// check name not already on clipboard
		if (clipboard_listWidget->findItems (new_name, Qt::MatchExactly).count() > 0) {
			QMessageBox::warning (this, tr ("EWItool"),
			                      tr ("That name already used on the Clipboard!\nPlease try again"));
			renameClipboard();
		}

		// Save patch in the Clipboard
		clipboard_listWidget->takeItem (r);
		delete curitem;
		clipboard_listWidget->insertItem (r, new_name);
		clipboard_listWidget->setCurrentRow (r);
		QByteArray ba = new_name.leftJustified (EWI_PATCHNAME_LENGTH, ' ').toLatin1();
		memcpy ( (void *) &clipboard[r].parameters.name, (void *) ba.data(), EWI_PATCHNAME_LENGTH);
		
		saveClipboard();
	}
}

void MainWindow::viewHexClipboard() {
	
	if (clipboard.count() > 0 && clipboard_listWidget->currentRow() > -1) {
		const char *raw_patch = &clipboard.at( clipboard_listWidget->currentRow() ).whole_patch[0];
		QString hex_patch;
	
		hex_patch = mididata->hexify( (char *) raw_patch, true );
		viewHex_dialog *h = new viewHex_dialog( hex_patch );
		h->exec();
	}
}

// EPX actions

/**
 * Export a patch from the Clipboard to the Patch Exchange
 */
void MainWindow::exportClipboard() {
	if (clipboard.count() > 0 && clipboard_listWidget->currentRow() > -1) {
		
		patch_t e_patch;
	
		e_patch = clipboard.at( clipboard_listWidget->currentRow() );
		// get extra info for EPX from the user
		epxSubmit_dialog *d = new epxSubmit_dialog( trimPatchName( e_patch.parameters.name ) );
		if (d->exec()) {
			// submit the patch
			e_patch.parameters.mode = EWI_EDIT;
			e_patch.parameters.patch_num = 0x00;
			QSettings *settings = new QSettings( "EWItool", "EWItool" );
			epx->insertPatch( 
				settings->value( "PatchExchange/Server" ).toString(),
				settings->value( "PatchExchange/UserID" ).toString(),
				settings->value( "PatchExchange/Password" ).toString(),
				trimPatchName( e_patch.parameters.name ),
				d->p_origin,
				d->p_type, 
				d->p_desc,
				d->p_private,
				d->p_tags,
				mididata->hexify( e_patch.whole_patch, false )
			);
			
		}
		delete d;
	}
}

void MainWindow::exportClipboardResponse( QString response ) {
	
	// cout << "Export response: " << qPrintable( response ) << endl;
	if (response.startsWith( "Resource id #" )) { 	// success
		statusBar()->showMessage(tr("Patch Succesfully sent to EWI Patch Exchange - Thank You"), STATUS_MSG_TIMEOUT);
	}
	else {
		// duplicate?
		if (response.contains( "duplicate key" )) {
			QMessageBox::warning( this, "EWItool",
								  tr( "Export Error\n\nThat patch is already in the Exchange" ) );
		}
		else {	// some other error
			QMessageBox::warning( this, "EWItool",
								  tr( "Export Error\n\n" ) + 
										  response.mid( response.indexOf( "ERROR:" ) + 7,
										  response.indexOf( " in <b>" ) - response.indexOf( "ERROR:" ) + 7) );
		}
	}
}

void MainWindow::epxQuery() {
	
	QSettings *settings = new QSettings( "EWItool", "EWItool" );
	epx->query( settings->value( "PatchExchange/Server" ).toString(),
				settings->value( "PatchExchange/UserID" ).toString(),
				settings->value( "PatchExchange/Password" ).toString(),
				type_comboBox->currentText(),
				since_comboBox->currentText(),
				contributor_comboBox->currentText(),
				origin_comboBox->currentText(),
				tags_lineEdit->text() );
	
	epxCopy_pushButton->setEnabled( false );
	epxDelete_pushButton->setEnabled( false );
}

void MainWindow::epxQueryResults( QString patch_list ) {
	
	int id;
	
	results_listWidget->clear();
	epx_ids.clear();
	
	QStringList pl = patch_list.split( "\n" );
	for (int i = 0; i < pl.size(); i++ ) {
		results_listWidget->addItem( pl.at(i).left( pl.at(i).lastIndexOf( "," ) ) );
		id = pl.at(i).mid( pl.at(i).lastIndexOf( "," ) + 1 ).toInt();
		epx_ids.append( id );
	}
}

void MainWindow::epxChosen() {
	
	int id;
	
	QSettings *settings = new QSettings( "EWItool", "EWItool" );
	id = epx_ids.at( results_listWidget->currentRow() );
	epx->getDetails( settings->value( "PatchExchange/Server" ).toString(),
					 settings->value( "PatchExchange/UserID" ).toString(),
					 settings->value( "PatchExchange/Password" ).toString(),
					 id
				   );
}

void MainWindow::epxDetailsResults( QString details ) {
	
	if (details.contains( "," )) {
	QStringList parms = details.split( "," );
	name_label->setText( parms.at( 0 ) );
	contributor_label->setText( parms.at( 1 ) );
	originator_label->setText( parms.at( 2 ) );
	epx_hex_patch = parms.at( 3 );
	type_label->setText( parms.at( 4 ) );
	desc_label->setText( parms.at( 5 ) );
	added_label->setText( parms.at( 6 ) );
	if (parms.at(7) == "f")
		private_label->setText( "public" );
	else
		private_label->setText( "private" );
	tags_label->setText( parms.at( 8 ) );
	
	epxCopy_pushButton->setEnabled( true );
	epxDelete_pushButton->setEnabled( true );
	}
}

void MainWindow::epxDelete() {
	
	if ( results_listWidget->currentRow() >= 0 ) {
	
	QSettings *settings = new QSettings( "EWItool", "EWItool" );
	int id = epx_ids.at( results_listWidget->currentRow() );
	epx->deletePatch( settings->value( "PatchExchange/Server" ).toString(),
					 settings->value( "PatchExchange/UserID" ).toString(),
					 settings->value( "PatchExchange/Password" ).toString(),
					 id
				   );
	epx_ids.removeAt( id );
	//results_listWidget->setCurrentRow( 0 );
	// force a re-query
	epxQuery();
	
	}
}

void MainWindow::epxCopy() {
	
	patch_t new_patch;
	
	new_patch = mididata->dehexify( epx_hex_patch, false );
	
	clipboard_listWidget->addItem( trimPatchName( new_patch.parameters.name ) );
	clipboard.append( new_patch );
	saveClipboard();
}



// utility functions...

QString MainWindow::trimPatchName( char *rawName ) {
	// utility function to get a nicely formatted patch name
	QString sname = rawName;
	sname.truncate( EWI_PATCHNAME_LENGTH );
	return sname.trimmed();
}

int MainWindow::randBetween( int min, int max ) {
	
	return min + rand() % (max - min);
	
}

/**
 * Returns current value randonly altered by up to 10% but within specified bounds
 * @param min 
 * @param max 
 * @param currval 
 * @return 
 */
int MainWindow::randNear( int min, int max, int currval ) {
	
	// a random 10% alteration of the value
	int newmin = currval - (max/10); 
	if (newmin < min) newmin = min;
    if (newmin == max) newmin -= max/10;	
	
	int newmax = currval + (max/10);
	if (newmax > max) newmax = max;
	if (newmax == min) newmax += max/10;
	
	return randBetween( newmin, newmax );
	
}

/**
 * Returns pct of p1 plus 1-pct of p2 i.e. avergage if pct == 50
 * @param p1 
 * @param pct 
 * @param p2 
 * @return 
 */
int MainWindow::mixInts( int p1, int pct, int p2 ) {
	
	return ((p1 * pct) / 100) + ((p2 * (100-pct)) / 100 );
	
}
