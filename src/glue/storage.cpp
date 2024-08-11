/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#include "storage.h"
#include "channel.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/init.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/sequencer.h"
#include "core/wave.h"
#include "core/waveFactory.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/track.h"
#include "gui/ui.h"
#include "src/core/actions/actionRecorder.h"
#include "utils/fs.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>

extern giada::m::Engine* g_engine;
extern giada::v::Ui*     g_ui;

namespace giada::c::storage
{
namespace
{
void printLoadError_(int res)
{
	if (res == G_FILE_UNREADABLE)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PATCHUNREADABLE));
	else if (res == G_FILE_INVALID)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PATCHINVALID));
	else if (res == G_FILE_UNSUPPORTED)
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PATCHUNSUPPORTED));
}

/* -------------------------------------------------------------------------- */

bool validateFileName_(const std::string& name)
{
	if (name == "")
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_CHOOSEFILENAME));
		return false;
	}
	if (!u::fs::isValidFileName(name))
	{
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_FILEHASINVALIDCHARS));
		return false;
	}
	return true;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void loadProject(void* data)
{
	v::gdBrowserLoad* browser = static_cast<v::gdBrowserLoad*>(data);

	const std::string projectPath = browser->getSelectedItem();

	/* Close all sub-windows first (browser included), in case there are VST 
	editors visible. VST editors must be closed before deleting their plug-in 
	processors. */

	g_ui->closeAllSubwindows();

	/* Stop the UI updater: the current model is about to change, but the UI
	still retains the old one. Updating during this stage would trigger
	assertions/crashes. */

	g_ui->stopUpdater();

	auto uiProgress     = g_ui->mainWindow->getScopedProgress(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_LOADINGPROJECT));
	auto engineProgress = [&uiProgress](float v) { uiProgress.setProgress(v); };

	m::model::LoadState state = g_engine->getStorageApi().loadProject(projectPath, engineProgress);

	if (state.patch.status != G_FILE_OK)
	{
		printLoadError_(state.patch.status);
		g_ui->startUpdater();
		return;
	}

	g_ui->model.patchPath = u::fs::getUpDir(projectPath);
	g_ui->load(state.patch);
	g_ui->startUpdater();

	if (!state.isGood())
		layout::openMissingAssetsWindow(state);
}

/* -------------------------------------------------------------------------- */

void saveProject(void* data)
{
	v::gdBrowserSave* browser = static_cast<v::gdBrowserSave*>(data);

	const std::string projectName = u::fs::stripExt(browser->getName());
	const std::string projectPath = u::fs::join(browser->getCurrentPath(), projectName + G_PROJECT_EXT);

	if (!validateFileName_(projectName))
		return;

	if (u::fs::dirExists(projectPath) &&
	    !v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_PROJECTEXISTS)))
		return;

	auto uiProgress     = g_ui->mainWindow->getScopedProgress(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECT));
	auto engineProgress = [&uiProgress](float v) { uiProgress.setProgress(v); };

	g_ui->model.projectName = projectName;

	if (g_engine->getStorageApi().storeProject(projectPath, g_ui->model, engineProgress))
	{
		g_ui->setMainWindowTitle(projectName);
		g_ui->model.patchPath = u::fs::getUpDir(projectPath);
		browser->do_callback();
	}
	else
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_SAVINGPROJECTERROR));
}

/* -------------------------------------------------------------------------- */

void loadSample(void* data)
{
	v::gdBrowserLoad* browser  = static_cast<v::gdBrowserLoad*>(data);
	std::string       fullPath = browser->getSelectedItem();

	if (fullPath.empty())
		return;

	g_ui->model.samplePath = u::fs::dirname(fullPath);

	c::channel::loadChannel(browser->getChannelId(), fullPath);

	browser->do_callback();
}

/* -------------------------------------------------------------------------- */

void saveSample(void* data)
{
	v::gdBrowserSave* browser    = static_cast<v::gdBrowserSave*>(data);
	const std::string name       = browser->getName();
	const std::string folderPath = browser->getCurrentPath();
	const ID          channelId  = browser->getChannelId();

	if (!validateFileName_(name))
		return;

	const std::string filePath = u::fs::join(folderPath, u::fs::stripExt(name) + ".wav");

	if (u::fs::fileExists(filePath) &&
	    !v::gdConfirmWin(g_ui->getI18Text(v::LangMap::COMMON_WARNING),
	        g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_FILEEXISTS)))
		return;

	if (!g_engine->getChannelsApi().saveSample(channelId, filePath))
		v::gdAlert(g_ui->getI18Text(v::LangMap::MESSAGE_STORAGE_SAVINGFILEERROR));
	else
	{
		g_ui->model.samplePath = u::fs::dirname(filePath);
	}

	browser->do_callback();
}
} // namespace giada::c::storage