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

#include <QFile>
#include <QInputDialog>
#include <QMessageBox>

#include "clipboard.h"

#include "epxsubmit_dialog.h"
#include "viewhex_dialog.h"

Clipboard::Clipboard( QString libraryLoc, patchExchange *main_epx, QWidget *parent)
	: QWidget( parent )
{
	libraryLocation = libraryLoc;
	
	setupUi( this );
	
	epx = main_epx;
	
	// connect the buttons
	connect( clearClipboard_pushButton, SIGNAL(clicked()), this, SLOT( clearAll() ));
	connect( deleteClipboard_pushButton, SIGNAL(clicked()), this, SLOT( deleteItem() ));
	connect( renameClipboard_pushButton, SIGNAL(clicked()), this, SLOT( renameItem() ));
	connect( viewHexClipboard_pushButton, SIGNAL(clicked()), this, SLOT( viewHex() ));
	connect( exchangeClipboard_pushButton, SIGNAL(clicked()), this, SLOT( exportToEPX() ));
	connect( exportClipboard_pushButton, SIGNAL(clicked()), this, SLOT( exportToFile() ));
	
	// connect other interesting events
	connect( clipboard_listWidget, SIGNAL( itemSelectionChanged( ) ), this, SLOT( selectionChanged( ) ) ) ;
	
	load();
	selectionChanged();
	if (clipboard_listWidget->count() == 0) clearClipboard_pushButton->setEnabled( false );
}


Clipboard::~Clipboard()
{
}

void Clipboard::clearAll( ) {
	
	if (QMessageBox::question (this, "EWItool - Clear Clipboard",
		tr ("Do you realy want to clear the Clipboard completely?" ),
			QMessageBox::No | QMessageBox::Yes
							  ) ==  QMessageBox::No) return;
	
	clipboard_listWidget->clear();
	clipboard_list.clear();
	
	QString fileName = libraryLocation + "/CLIPBOARD.CLP";
	QFile file(fileName);
	file.remove();
	clearClipboard_pushButton->setEnabled( false );
}

void Clipboard::load(  ) {
	
	patch_t tmp_patch;
	QString fileName = libraryLocation + CLIPBOARD_FILE;
	
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly)) {
		return;
	}

	clipboard_listWidget->clear();
	clipboard_list.clear();
	
	QDataStream inp(&file);
	while (!inp.atEnd()) {
		inp.readRawData( (char *) &tmp_patch, EWI_PATCH_LENGTH );
		clipboard_listWidget->addItem( ewi4000sPatch::toQString( tmp_patch.parameters.name ) );
		clipboard_list.append( tmp_patch );
	}
}

void Clipboard::save(  ) {
	
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
	for (int p = 0; p < clipboard_list.count(); p++ ) {
		out.writeRawData( clipboard_list.at(p).whole_patch, EWI_PATCH_LENGTH );
	}
	//statusBar()->showMessage(tr("Clipboard saved"), STATUS_MSG_TIMEOUT);
}

void Clipboard::appendItem( patch_t patch ) {
	
	QString npname;
	
	npname = ewi4000sPatch::toQString( patch.parameters.name );
	
	clipboard_listWidget->addItem( npname );
	clipboard_list.append( patch );
	
	save();
	
	clearClipboard_pushButton->setEnabled( true );
}

void Clipboard::deleteItem() {
	
	if (QMessageBox::question (this, "EWItool - Delete Item",
		tr ("Do you realy want to delete this item from the Clipboard?" ),
			QMessageBox::No | QMessageBox::Yes
							  ) ==  QMessageBox::No) return;
	int row = clipboard_listWidget->currentRow();
	if (row != -1) {
		delete clipboard_listWidget->takeItem( row );
		clipboard_list.removeAt( row );
	}
	save();
	
	if (clipboard_listWidget->count() == 0) clearClipboard_pushButton->setEnabled( false );
}

void Clipboard::renameItem() {
	if (clipboard_list.count() > 0 && clipboard_listWidget->currentRow() > -1) {

		// Get new name from the user
		bool ok;

		QListWidgetItem *curitem = clipboard_listWidget->currentItem();
		int r = clipboard_listWidget->row (curitem);
		QString text = curitem->text();
		QString new_name = QInputDialog::getText (this, "EWItool - Rename Patch", "New Patch Name", QLineEdit::Normal, text, &ok);

		if (!ok || new_name.isEmpty()) return;

		new_name = new_name.simplified();

		if (new_name.length() > EWI_PATCHNAME_LENGTH) {
			QMessageBox::warning (this, tr ("EWItool - Rename"),
								  tr ("Patch name too long!\nEnter up to %1 characters").arg (EWI_PATCHNAME_LENGTH));
			renameItem();
		}

		// check name not already on clipboard
		if (clipboard_listWidget->findItems (new_name, Qt::MatchExactly).count() > 0) {
			QMessageBox::warning (this, tr ("EWItool - Rename"),
								  tr ("That name already used on the Clipboard!\nPlease try again"));
			renameItem();
		}

		// Save patch in the Clipboard
		clipboard_listWidget->takeItem (r);
		delete curitem;
		clipboard_listWidget->insertItem (r, new_name);
		clipboard_listWidget->setCurrentRow (r);
		QByteArray ba = new_name.leftJustified (EWI_PATCHNAME_LENGTH, ' ').toLatin1();
		memcpy ( (void *) &clipboard_list[r].parameters.name, (void *) ba.data(), EWI_PATCHNAME_LENGTH);
		
		save();
	}
}

void Clipboard::viewHex() {
	if (clipboard_list.count() > 0 && clipboard_listWidget->currentRow() > -1) {
		const char *raw_patch = &clipboard_list.at( clipboard_listWidget->currentRow() ).whole_patch[0];
		QString hex_patch;
		hex_patch = ewi4000sPatch::hexify( (char *) raw_patch, true );
		viewHex_dialog *h = new viewHex_dialog( hex_patch );
		h->exec();
	}
}

void Clipboard::exportToEPX() {
	if ( clipboard_list.count() > 0 && clipboard_listWidget->currentRow() > -1 ) {

		patch_t e_patch;
		
		e_patch = clipboard_list.at( clipboard_listWidget->currentRow() );
		// get extra info for EPX from the user
		epxSubmit_dialog *d = new epxSubmit_dialog( ewi4000sPatch::toQString( e_patch.parameters.name ) );
		if ( d->exec() ) {
			// submit the patch
			e_patch.parameters.mode = EWI_EDIT;
			e_patch.parameters.patch_num = 0x00;
			QSettings *settings = new QSettings( "EWItool", "EWItool" );
			epx->insertPatch(
			    settings->value( "PatchExchange/Server" ).toString(),
			    settings->value( "PatchExchange/UserID" ).toString(),
			    settings->value( "PatchExchange/Password" ).toString(),
				ewi4000sPatch::toQString( e_patch.parameters.name ),
			    d->p_origin,
			    d->p_type,
			    d->p_desc,
			    d->p_private,
			    d->p_tags,
			    ewi4000sPatch::hexify( e_patch.whole_patch, false )
			);

		}

		delete d;
	}
}

/**
 * Exports an item from the Clipboard into the "export" subdir
 */
void Clipboard::exportToFile() {
	
	patch_t e_patch;
	
	QString e_name = libraryLocation + EXPORT_DIR + "/" + clipboard_listWidget->currentItem()->text() + ".syx";
	QFile file(e_name);
	if (!file.open(QFile::WriteOnly)) {
		QMessageBox::warning(this, tr("EWItool"),
							 tr("Cannot write file %1:\n%2.")
									 .arg(e_name)
									 .arg(file.errorString()));
		return;
	}
	
	e_patch = clipboard_list.at( clipboard_listWidget->currentRow() );
	e_patch.parameters.mode = EWI_SAVE;
	e_patch.parameters.patch_num = EXPORT_PATCH_NUM;
	
	QDataStream out(&file);
	out.writeRawData( e_patch.whole_patch, EWI_PATCH_LENGTH );
	//out.writeRawData( e_patch.whole_patch, EWI_PATCH_LENGTH );
	QMessageBox::information(this, tr("EWItool - Patch Export"),
							 tr("Patch exported to ") + e_name );
	
	
}


void Clipboard::selectionChanged( ) {
		
	if (clipboard_listWidget->selectedItems().empty()) { // no row selected
		deleteClipboard_pushButton->setEnabled( false );
		renameClipboard_pushButton->setEnabled( false );
		viewHexClipboard_pushButton->setEnabled( false );
		exchangeClipboard_pushButton->setEnabled( false );
		exportClipboard_pushButton->setEnabled( false );
	}
	else {
		deleteClipboard_pushButton->setEnabled( true );
		renameClipboard_pushButton->setEnabled( true );
		viewHexClipboard_pushButton->setEnabled( true );
		exchangeClipboard_pushButton->setEnabled( true );
		exportClipboard_pushButton->setEnabled( true );
	}
	
}

bool Clipboard::onClipboard( QString pname ) {
	
	if (clipboard_listWidget->findItems( pname, Qt::MatchExactly ).count() > 0)
        return true;
	else
        return false;
	
}

int Clipboard::count() {
	
	return clipboard_list.size();
	
}

QString Clipboard::getNameAt( int i ) {
	
	return QString( clipboard_list.at( i ).parameters.name );
	
}

patch_t Clipboard::getPatchAt( int i ) {
	
	return clipboard_list.at( i );
	
}




