/* Phaethon - A FLOSS resource explorer for BioWare's Aurora engine games
 *
 * Phaethon is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * Phaethon is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * Phaethon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phaethon. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file gui/mainwindow.cpp
 *  Phaethon's main window.
 */

#include <deque>

#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/statbox.h>
#include <wx/splitter.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/menu.h>
#include <wx/artprov.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>

#include <wx/generic/stattextg.h>

#include "common/ustring.h"
#include "common/version.h"
#include "common/util.h"
#include "common/error.h"
#include "common/filepath.h"
#include "common/stream.h"
#include "common/file.h"

#include "sound/sound.h"
#include "sound/audiostream.h"

#include "aurora/util.h"
#include "aurora/zipfile.h"
#include "aurora/erffile.h"
#include "aurora/rimfile.h"
#include "aurora/keyfile.h"
#include "aurora/keydatafile.h"
#include "aurora/biffile.h"
#include "aurora/bzffile.h"

#include "gui/eventid.h"
#include "gui/about.h"
#include "gui/mainwindow.h"
#include "gui/resourcetree.h"
#include "gui/panelpreviewsound.h"

#include "cline.h"

namespace GUI {

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(kEventFileOpenDir , MainWindow::onOpenDir)
	EVT_MENU(kEventFileOpenFile, MainWindow::onOpenFile)
	EVT_MENU(kEventFileClose   , MainWindow::onClose)
	EVT_MENU(kEventFileQuit    , MainWindow::onQuit)
	EVT_MENU(kEventHelpAbout   , MainWindow::onAbout)

	EVT_BUTTON(kEventButtonExportRaw   , MainWindow::onExportRaw)
	EVT_BUTTON(kEventButtonExportBMUMP3, MainWindow::onExportBMUMP3)
	EVT_BUTTON(kEventButtonExportWAV   , MainWindow::onExportWAV)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString &title, const wxPoint &pos, const wxSize &size) :
	wxFrame(NULL, wxID_ANY, title, pos, size) {

	createLayout();

	resourceTreeSelect(0);
}

MainWindow::~MainWindow() {
}

void MainWindow::createLayout() {
	CreateStatusBar();
	GetStatusBar()->SetStatusText(wxT("Idle..."));

	wxMenu *menuFile = new wxMenu;

	wxMenuItem *menuFileOpenDir =
		new wxMenuItem(0, kEventFileOpenDir, wxT("Open &directory\tCtrl-D"), Common::UString("Open directory"));
	wxMenuItem *menuFileOpenFile =
		new wxMenuItem(0, kEventFileOpenFile, wxT("Open &file\tCtrl-D"), Common::UString("Open file"));
	wxMenuItem *menuFileClose =
		new wxMenuItem(0, kEventFileClose, wxT("&Close\tCtrl-W"), Common::UString("Close"));

	menuFileOpenDir->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER_OPEN));
	menuFileOpenFile->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN));
	menuFileClose->SetBitmap(wxArtProvider::GetBitmap(wxART_CLOSE));

	menuFile->Append(menuFileOpenDir);
	menuFile->Append(menuFileOpenFile);
	menuFile->AppendSeparator();
	menuFile->Append(menuFileClose);
	menuFile->AppendSeparator();
	menuFile->Append(kEventFileQuit, wxT("&Quit\tCtrl-Q"), Common::UString("Quit ") + PHAETHON_NAME);


	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(kEventHelpAbout, wxT("&About\tF1"), Common::UString("About ") + PHAETHON_NAME);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, wxT("&File"));
	menuBar->Append(menuHelp, wxT("&Help"));

	SetMenuBar(menuBar);


	wxSplitterWindow *splitterMainLog = new wxSplitterWindow(this, wxID_ANY);
	wxSplitterWindow *splitterTreeRes = new wxSplitterWindow(splitterMainLog, wxID_ANY);

	_splitterInfoPreview = new wxSplitterWindow(splitterTreeRes, wxID_ANY);

	wxPanel *panelLog  = new wxPanel( splitterMainLog    , wxID_ANY);
	wxPanel *panelInfo = new wxPanel(_splitterInfoPreview, wxID_ANY);
	wxPanel *panelTree = new wxPanel( splitterTreeRes    , wxID_ANY);

	_panelPreviewEmpty = new wxPanel(_splitterInfoPreview, wxID_ANY);
	_panelPreviewSound = new PanelPreviewSound(_splitterInfoPreview, "Preview");

	_panelPreviewSound->Hide();

	_resourceTree = new ResourceTree(panelTree, *this);

	_resInfoName     = new wxGenericStaticText(panelInfo, wxID_ANY, wxEmptyString);
	_resInfoSize     = new wxGenericStaticText(panelInfo, wxID_ANY, wxEmptyString);
	_resInfoFileType = new wxGenericStaticText(panelInfo, wxID_ANY, wxEmptyString);
	_resInfoResType  = new wxGenericStaticText(panelInfo, wxID_ANY, wxEmptyString);

	wxTextCtrl *log = new wxTextCtrl(panelLog, wxID_ANY, wxEmptyString, wxDefaultPosition,
	                                 wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

	wxBoxSizer *sizerWindow = new wxBoxSizer(wxVERTICAL);

	wxStaticBox *boxLog  = new wxStaticBox(panelLog , wxID_ANY,  wxT("Log"));
	wxStaticBox *boxInfo = new wxStaticBox(panelInfo, wxID_ANY,  wxT("Resource info"));
	wxStaticBox *boxTree = new wxStaticBox(panelTree, wxID_ANY,  wxT("Resources"));

	boxLog->Lower();
	boxInfo->Lower();
	boxTree->Lower();

	wxStaticBox *boxPreviewEmpty = new wxStaticBox(_panelPreviewEmpty, wxID_ANY,  wxT("Preview"));

	boxPreviewEmpty->Lower();

	wxStaticBoxSizer *sizerLog  = new wxStaticBoxSizer(boxLog , wxHORIZONTAL);
	wxStaticBoxSizer *sizerInfo = new wxStaticBoxSizer(boxInfo, wxVERTICAL);
	wxStaticBoxSizer *sizerTree = new wxStaticBoxSizer(boxTree, wxHORIZONTAL);

	wxStaticBoxSizer *sizerPreviewEmpty = new wxStaticBoxSizer(boxPreviewEmpty, wxHORIZONTAL);

	sizerTree->Add(_resourceTree, 1, wxEXPAND, 0);
	panelTree->SetSizer(sizerTree);

	_sizerExport = new wxBoxSizer(wxHORIZONTAL);

	_buttonExportRaw    = new wxButton(panelInfo, kEventButtonExportRaw   , wxT("Save"));
	_buttonExportBMUMP3 = new wxButton(panelInfo, kEventButtonExportBMUMP3, wxT("Export as MP3"));
	_buttonExportWAV    = new wxButton(panelInfo, kEventButtonExportWAV   , wxT("Export as PCM WAV"));

	_buttonExportRaw->Disable();
	_buttonExportBMUMP3->Hide();
	_buttonExportWAV->Hide();

	_sizerExport->Add(_buttonExportRaw   , 0, wxEXPAND, 0);
	_sizerExport->Add(_buttonExportBMUMP3, 0, wxEXPAND, 0);
	_sizerExport->Add(_buttonExportWAV   , 0, wxEXPAND, 0);

	sizerInfo->Add(_resInfoName    , 0, wxEXPAND, 0);
	sizerInfo->Add(_resInfoSize    , 0, wxEXPAND, 0);
	sizerInfo->Add(_resInfoFileType, 0, wxEXPAND, 0);
	sizerInfo->Add(_resInfoResType , 0, wxEXPAND, 0);

	sizerInfo->Add(_sizerExport, 0, wxEXPAND | wxTOP, 5);

	panelInfo->SetSizer(sizerInfo);

	_panelPreviewEmpty->SetSizer(sizerPreviewEmpty);

	sizerLog->Add(log, 1, wxEXPAND, 0);
	panelLog->SetSizer(sizerLog);

	_splitterInfoPreview->SetMinimumPaneSize(20);
	splitterTreeRes->SetMinimumPaneSize(20);
	splitterMainLog->SetMinimumPaneSize(20);

	splitterMainLog->SetSashGravity(1.0);

	_splitterInfoPreview->SplitHorizontally(panelInfo, _panelPreviewEmpty);
	splitterTreeRes->SplitVertically(panelTree, _splitterInfoPreview);
	splitterMainLog->SplitHorizontally(splitterTreeRes, panelLog);

	sizerWindow->Add(splitterMainLog, 1, wxEXPAND, 0);
	SetSizer(sizerWindow);

	Layout();

	_splitterInfoPreview->SetSashPosition(150);
	splitterTreeRes->SetSashPosition(200);
	splitterMainLog->SetSashPosition(480);
}

void MainWindow::onQuit(wxCommandEvent &event) {
	close();
	Close(true);
}

void MainWindow::onAbout(wxCommandEvent &event) {
	AboutDialog *about = new AboutDialog(this);
	about->show();
}

void MainWindow::onOpenDir(wxCommandEvent &event) {
	Common::UString path = dialogOpenDir("Open Aurora game directory");
	if (path.empty())
		return;

	open(path);
}

void MainWindow::onOpenFile(wxCommandEvent &event) {
	Common::UString path = dialogOpenFile("Open Aurora game resource file",
	                                      "Aurora game resource (*.*)|*.*");
	if (path.empty())
		return;

	open(path);
}

void MainWindow::onClose(wxCommandEvent &event) {
	close();
}

void MainWindow::onExportRaw(wxCommandEvent &event) {
	ResourceTreeItem *item = _resourceTree->getSelection();
	if (!item)
		return;

	Common::UString path = dialogSaveFile("Save Aurora game resource file",
	                                      "Aurora game resource (*.*)|*.*", item->getName());
	if (path.empty())
		return;

	exportRaw(*item, path);
}

void MainWindow::onExportBMUMP3(wxCommandEvent &event) {
	ResourceTreeItem *item = _resourceTree->getSelection();
	if (!item)
		return;

	assert(item->getFileType() == Aurora::kFileTypeBMU);

	Common::UString path = dialogSaveFile("Save MP3 file", "MP3 file (*.mp3)|*.mp3",
	                                      TypeMan.setFileType(item->getName(), Aurora::kFileTypeMP3));
	if (path.empty())
		return;

	exportBMUMP3(*item, path);
}

void MainWindow::onExportWAV(wxCommandEvent &event) {
	ResourceTreeItem *item = _resourceTree->getSelection();
	if (!item)
		return;

	Common::UString path = dialogSaveFile("Save PCM WAV file", "WAV file (*.wav)|*.wav",
	                                      TypeMan.setFileType(item->getName(), Aurora::kFileTypeWAV));
	if (path.empty())
		return;

	exportWAV(*item, path);
}

void MainWindow::forceRedraw() {
	Refresh();
	Update();
}

Common::UString MainWindow::dialogOpenDir(const Common::UString &title) {
	wxDirDialog dialog(this, title, wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dialog.ShowModal() == wxID_OK)
		return dialog.GetPath();

	return "";
}

Common::UString MainWindow::dialogOpenFile(const Common::UString &title,
                                           const Common::UString &mask) {

	wxFileDialog dialog(this, title, wxEmptyString, wxEmptyString, mask,
	                    wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
	if (dialog.ShowModal() == wxID_OK)
		return dialog.GetPath();

	return "";
}

Common::UString MainWindow::dialogSaveFile(const Common::UString &title,
                                           const Common::UString &mask,
                                           const Common::UString &def) {

	wxFileDialog dialog(this, title, wxEmptyString, def, mask,
	                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_OK)
		return dialog.GetPath();

	return "";
}

bool MainWindow::open(Common::UString path) {
	close();

	if (!Common::FilePath::isDirectory(path) && !Common::FilePath::isRegularFile(path)) {
		warning("Path \"%s\" is neither a directory nor a regular file", path.c_str());
		return false;
	}

	path = Common::FilePath::normalize(path);

	if (Common::FilePath::isDirectory(path))
		GetStatusBar()->PushStatusText(Common::UString("Recursively adding all files in ") + path + "...");
	else
		GetStatusBar()->PushStatusText(Common::UString("Adding file ") + path + "...");

	forceRedraw();

	try {
		_files.readPath(path, -1);
	} catch (Common::Exception &e) {
		GetStatusBar()->PopStatusText();

		Common::printException(e, "WARNING: ");
		return false;
	}

	_path = path;

	GetStatusBar()->PopStatusText();
	GetStatusBar()->PushStatusText(Common::UString("Populating resource tree..."));
	_resourceTree->populate(_files.getRoot());
	GetStatusBar()->PopStatusText();

	return true;
}

void MainWindow::close() {
	_resourceTree->DeleteAllItems();

	_files.clear();
	_path.clear();

	for (ArchiveMap::iterator a = _archives.begin(); a != _archives.end(); ++a)
		delete a->second;

	_archives.clear();

	for (KEYDataFileMap::iterator d = _keyDataFiles.begin(); d != _keyDataFiles.end(); ++d)
		delete d->second;

	_keyDataFiles.clear();

	resourceTreeSelect(0);
}

void MainWindow::showExportButtons(bool enableRaw, bool showMP3, bool showWAV) {
	_buttonExportRaw->Enable(enableRaw);
	_buttonExportBMUMP3->Show(showMP3);
	_buttonExportWAV->Show(showWAV);
}

void MainWindow::showPreviewPanel(Aurora::ResourceType type) {
	switch (type) {
		case Aurora::kResourceSound:
			showPreviewPanel(_panelPreviewSound);
			break;

		default:
			showPreviewPanel(_panelPreviewEmpty);
			break;
	}
}

void MainWindow::showPreviewPanel(wxPanel *panel) {
	wxWindow *old = _splitterInfoPreview->GetWindow2();

	Freeze();
	old->Hide();
	_splitterInfoPreview->ReplaceWindow(old, panel);
	panel->Show();
	Thaw();
}

Common::UString MainWindow::getSizeLabel(uint32 size) {
	if (size == Common::kFileInvalid)
		return "-";

	if (size < 1024)
		return Common::UString::sprintf("%u", size);

	Common::UString humanRead = Common::FilePath::getHumanReadableSize(size);

	return Common::UString::sprintf("%s (%u)", humanRead.c_str(), size);
}

Common::UString MainWindow::getFileTypeLabel(Aurora::FileType type) {
	Common::UString label = Common::UString::sprintf("%d", type);
	if (type != Aurora::kFileTypeNone)
		label += Common::UString::sprintf(" (%s)", TypeMan.getExtension(type).c_str());

	return label;
}

Common::UString MainWindow::getResTypeLabel(Aurora::ResourceType type) {
	Common::UString label = Common::UString::sprintf("%d", type);
	if (type != Aurora::kResourceNone)
		label += Common::UString::sprintf(" (%s)", getResourceTypeDescription(type).c_str());

	return label;
}

void MainWindow::resourceTreeSelect(const ResourceTreeItem *item) {
	Common::UString labelInfoName     = "Resource name: ";
	Common::UString labelInfoSize     = "Size: ";
	Common::UString labelInfoFileType = "File type: ";
	Common::UString labelInfoResType  = "Resource type: ";

	if (item) {
		labelInfoName += item->getName();

		if (item->getSource() == ResourceTreeItem::kSourceDirectory) {

			showExportButtons(false, false, false);
			showPreviewPanel(Aurora::kResourceNone);

			labelInfoSize     += "-";
			labelInfoFileType += "Directory";
			labelInfoResType  += "Directory";

		} else if ((item->getSource() == ResourceTreeItem::kSourceFile) ||
		           (item->getSource() == ResourceTreeItem::kSourceArchiveFile)) {

			Aurora::FileType     fileType = item->getFileType();
			Aurora::ResourceType resType  = item->getResourceType();

			labelInfoSize     += getSizeLabel(item->getSize());
			labelInfoFileType += getFileTypeLabel(fileType);
			labelInfoResType  += getResTypeLabel(resType);

			showExportButtons(true, fileType == Aurora::kFileTypeBMU, resType == Aurora::kResourceSound);
			showPreviewPanel(resType);

		}
	} else {
		showExportButtons(false, false, false);
		showPreviewPanel(Aurora::kResourceNone);
	}

	_resInfoName->SetLabel(labelInfoName);
	_resInfoSize->SetLabel(labelInfoSize);
	_resInfoFileType->SetLabel(labelInfoFileType);
	_resInfoResType->SetLabel(labelInfoResType);

	_sizerExport->Layout();

	_panelPreviewSound->setCurrentItem(item);
}

void MainWindow::resourceTreeActivate(const ResourceTreeItem &item) {
	if (item.getResourceType() == Aurora::kResourceSound) {
		_panelPreviewSound->setCurrentItem(&item);
		_panelPreviewSound->play();;
	}
}

bool MainWindow::exportRaw(const ResourceTreeItem &item, const Common::UString &path) {
	Common::UString msg = Common::UString("Saving \"") + item.getName() + "\" to \"" + path + "\"...";
	GetStatusBar()->PushStatusText(msg);

	Common::SeekableReadStream *res = 0;
	try {
		res = item.getResourceData();

		Common::DumpFile file(path);

		file.writeStream(*res);

		if (!file.flush() || file.err())
			throw Common::Exception(Common::kWriteError);

		delete res;

	} catch (Common::Exception &e) {
		delete res;

		GetStatusBar()->PopStatusText();
		Common::printException(e, "WARNING: ");
		return false;
	}

	GetStatusBar()->PopStatusText();
	return true;
}

bool MainWindow::exportBMUMP3(const ResourceTreeItem &item, const Common::UString &path) {
	Common::UString msg = Common::UString("Exporting \"") + item.getName() + "\" to \"" + path + "\"...";
	GetStatusBar()->PushStatusText(msg);

	Common::SeekableReadStream *res = 0;
	try {
		res = item.getResourceData();

		Common::DumpFile file(path);

		exportBMUMP3(*res, file);

		if (!file.flush() || file.err())
			throw Common::Exception(Common::kWriteError);

		delete res;

	} catch (Common::Exception &e) {
		delete res;

		GetStatusBar()->PopStatusText();
		Common::printException(e, "WARNING: ");
		return false;
	}

	GetStatusBar()->PopStatusText();
	return true;
}

bool MainWindow::exportWAV(const ResourceTreeItem &item, const Common::UString &path) {
	Common::UString msg = Common::UString("Exporting \"") + item.getName() + "\" to \"" + path + "\"...";
	GetStatusBar()->PushStatusText(msg);

	Common::SeekableReadStream *res = 0;
	Common::DumpFile *file = 0;
	try {
		res  = item.getResourceData();
		file = new Common::DumpFile(path);
	} catch (Common::Exception &e) {
		delete res;

		GetStatusBar()->PopStatusText();
		Common::printException(e, "WARNING: ");
		return false;
	}

	try {
		exportWAV(res, *file);

		if (!file->flush() || file->err())
			throw Common::Exception(Common::kWriteError);

	} catch (Common::Exception &e) {
		delete file;

		GetStatusBar()->PopStatusText();
		Common::printException(e, "WARNING: ");
		return false;
	}

	delete file;

	GetStatusBar()->PopStatusText();
	return true;
}

void MainWindow::exportBMUMP3(Common::SeekableReadStream &bmu, Common::WriteStream &mp3) {
	if ((bmu.size() <= 8) ||
	    (bmu.readUint32BE() != MKTAG('B', 'M', 'U', ' ')) ||
	    (bmu.readUint32BE() != MKTAG('V', '1', '.', '0')))
		throw Common::Exception("Not a valid BMU file");

	mp3.writeStream(bmu);
}

struct SoundBuffer {
	int16 buffer[4096];
	int samples;

	SoundBuffer() : samples(0) {
	}
};

void MainWindow::exportWAV(Common::SeekableReadStream *soundData, Common::WriteStream &wav) {
	Sound::AudioStream *sound = 0;
	try {
		sound = SoundMan.makeAudioStream(soundData);
	} catch (Common::Exception &e) {
		delete soundData;
		throw;
	}

	const uint16 channels = sound->getChannels();
	const uint32 rate     = sound->getRate();

	std::deque<SoundBuffer> buffers;

	uint64 length = Sound::RewindableAudioStream::kInvalidLength;
	Sound::RewindableAudioStream *rewSound = dynamic_cast<Sound::RewindableAudioStream *>(sound);
	if (rewSound)
		length = rewSound->getLength();

	warning("LENGTH: %lu, DURATION: %lu", length, rewSound ? rewSound->getDuration() : 0);

	if (length != Sound::RewindableAudioStream::kInvalidLength)
		buffers.resize((length / (ARRAYSIZE(SoundBuffer::buffer) / channels)) + 1);

	uint32 samples = 0;
	std::deque<SoundBuffer>::iterator buffer = buffers.begin();
	while (!sound->endOfStream()) {
		if (buffer == buffers.end()) {
			buffers.push_back(SoundBuffer());
			buffer = --buffers.end();
		}

		try {
			buffer->samples = sound->readBuffer(buffer->buffer, 4096);
		} catch (Common::Exception &e) {
			delete sound;
			throw;
		}

		if (buffer->samples > 0)
			samples += buffer->samples;

		++buffer;
	}

	delete sound;

	samples /= channels;

	warning("Real samples: %u", samples);

	const uint32 dataSize   = samples * channels * 2;
	const uint32 byteRate   = rate * channels * 2;
	const uint16 blockAlign = channels * 2;

	wav.writeUint32BE(MKTAG('R', 'I', 'F', 'F'));
	wav.writeUint32LE(36 + dataSize);
	wav.writeUint32BE(MKTAG('W', 'A', 'V', 'E'));

	wav.writeUint32BE(MKTAG('f', 'm', 't', ' '));
	wav.writeUint32LE(16);
	wav.writeUint16LE(1);
	wav.writeUint16LE(channels);
	wav.writeUint32LE(rate);
	wav.writeUint32LE(byteRate);
	wav.writeUint16LE(blockAlign);
	wav.writeUint16LE(16);

	wav.writeUint32BE(MKTAG('d', 'a', 't', 'a'));
	wav.writeUint32LE(dataSize);

	for (std::deque<SoundBuffer>::const_iterator b = buffers.begin(); b != buffers.end(); ++b)
		for (int i = 0; i < b->samples; i++)
			wav.writeUint16LE(b->buffer[i]);
}

Aurora::Archive *MainWindow::getArchive(const boost::filesystem::path &path) {
	ArchiveMap::iterator a = _archives.find(path.c_str());
	if (a != _archives.end())
		return a->second;

	Aurora::Archive *arch = 0;
	switch (TypeMan.getFileType(path.c_str())) {
		case Aurora::kFileTypeZIP:
			arch = new Aurora::ZIPFile(path.c_str());
			break;

		case Aurora::kFileTypeERF:
		case Aurora::kFileTypeMOD:
		case Aurora::kFileTypeNWM:
		case Aurora::kFileTypeSAV:
		case Aurora::kFileTypeHAK:
			arch = new Aurora::ERFFile(path.c_str());
			break;

		case Aurora::kFileTypeRIM:
			arch = new Aurora::RIMFile(path.c_str());
			break;

		case Aurora::kFileTypeKEY: {
				Aurora::KEYFile *key = new Aurora::KEYFile(path.c_str());
				loadKEYDataFiles(*key);

				arch = key;
				break;
			}

		default:
			throw Common::Exception("Invalid archive file \"%s\"", path.c_str());
	}

	_archives.insert(std::make_pair(path.c_str(), arch));
	return arch;
}

Aurora::KEYDataFile *MainWindow::getKEYDataFile(const Common::UString &file) {
	KEYDataFileMap::iterator d = _keyDataFiles.find(file);
	if (d != _keyDataFiles.end())
		return d->second;

	Common::UString path = Common::FilePath::normalize(_path + "/" + file);
	if (path.empty())
		throw Common::Exception("No such file or directory \"%s\"", (_path + "/" + file).c_str());

	Aurora::FileType type = TypeMan.getFileType(file);

	Aurora::KEYDataFile *dataFile = 0;
	switch (type) {
		case Aurora::kFileTypeBIF:
			dataFile = new Aurora::BIFFile(path);
			break;

		case Aurora::kFileTypeBZF:
			dataFile = new Aurora::BZFFile(path);
			break;

		default:
			throw Common::Exception("Unknown KEY data file type %d\n", type);
	}

	_keyDataFiles.insert(std::make_pair(file, dataFile));
	return dataFile;
}

void MainWindow::loadKEYDataFiles(Aurora::KEYFile &key) {
	const std::vector<Common::UString> dataFiles = key.getDataFileList();
	for (uint i = 0; i < dataFiles.size(); i++) {
		try {

			GetStatusBar()->PushStatusText(Common::UString("Loading data file") + dataFiles[i] + "...");

			Aurora::KEYDataFile *dataFile = getKEYDataFile(dataFiles[i]);
			key.addDataFile(i, dataFile);

			GetStatusBar()->PopStatusText();

		} catch (Common::Exception &e) {
			e.add("Failed to load KEY data file \"%s\"", dataFiles[i].c_str());

			GetStatusBar()->PopStatusText();
			Common::printException(e, "WARNING: ");
		}
	}
}

} // End of namespace GUI