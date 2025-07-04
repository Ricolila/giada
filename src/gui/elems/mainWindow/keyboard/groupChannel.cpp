/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/mainWindow/keyboard/groupChannel.h"
#include "src/glue/channel.h"
#include "src/glue/layout.h"
#include "src/glue/main.h"
#include "src/gui/dispatcher.h"
#include "src/gui/elems/basics/dial.h"
#include "src/gui/elems/basics/imageButton.h"
#include "src/gui/elems/basics/menu.h"
#include "src/gui/elems/mainWindow/keyboard/channelProgress.h"
#include "src/gui/elems/mainWindow/keyboard/groupChannelButton.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/elems/mainWindow/keyboard/track.h"
#include "src/gui/elems/midiActivity.h"
#include "src/gui/graphics.h"
#include "src/gui/ui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
enum class Menu
{
	SETUP_MIDI_INPUT,
	EDIT_ROUTING,
	RENAME_CHANNEL,
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geGroupChannel::geGroupChannel(c::channel::Data d)
: geChannel(0, 0, 0, 0, d)
{
	playButton   = new geImageButton(graphics::channelPlayOff, graphics::channelPlayOn);
	arm          = new geImageButton(graphics::armOff, graphics::armOn);
	mainButton   = new geGroupChannelButton(m_channel);
	midiActivity = new geMidiActivity(/*hasOut=*/false);
	mute         = new geImageButton(graphics::muteOff, graphics::muteOn);
	solo         = new geImageButton(graphics::soloOff, graphics::soloOn);
	fx           = new geImageButton(graphics::fxOff, graphics::fxOn);
	vol          = new geDial(0, 0, 0, 0);

	addWidget(playButton, G_GUI_UNIT);
	addWidget(mainButton);
	addWidget(midiActivity, 10);
	addWidget(mute, G_GUI_UNIT);
	addWidget(solo, G_GUI_UNIT);
	addWidget(fx, G_GUI_UNIT);
	addWidget(vol, G_GUI_UNIT);
	end();

	playButton->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_PLAY));
	midiActivity->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_MIDIACTIVITY_INONLY));
	mute->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_MUTE));
	solo->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_SOLO));
	fx->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_FX));
	vol->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_VOLUME));

	fx->forceValue(m_channel.plugins.size() > 0);
	fx->onClick = [this]()
	{
		c::layout::openChannelPluginListWindow(m_channel.id);
	};

	playButton->when(FL_WHEN_CHANGED); // On keypress && on keyrelease
	playButton->onClick = [this]()
	{
		g_ui->dispatcher.dispatchTouch(*this, playButton->getValue());
	};

	mute->setToggleable(true);
	mute->onClick = [this]()
	{
		c::channel::toggleMuteChannel(m_channel.id, Thread::MAIN);
	};

	solo->setToggleable(true);
	solo->onClick = [this]()
	{
		c::channel::toggleSoloChannel(m_channel.id, Thread::MAIN);
	};

	mainButton->onClick = [this]()
	{ openMenu(); };

	vol->value(m_channel.volume);
	vol->callback(cb_changeVol, (void*)this);

	size(w(), h()); // Force responsiveness
}

/* -------------------------------------------------------------------------- */

void geGroupChannel::openMenu()
{
	geMenu menu;

	menu.addItem((ID)Menu::SETUP_MIDI_INPUT, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_MIDIINPUT));
	menu.addItem((ID)Menu::EDIT_ROUTING, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_EDITROUTING));
	menu.addItem((ID)Menu::RENAME_CHANNEL, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_RENAME));

	menu.onSelect = [&data = m_channel](ID id)
	{
		switch (static_cast<Menu>(id))
		{
		case Menu::SETUP_MIDI_INPUT:
			c::layout::openChannelMidiInputWindow(data.id);
			break;
		case Menu::EDIT_ROUTING:
			c::layout::openChannelRoutingWindow(data.id);
			break;
		case Menu::RENAME_CHANNEL:
			c::layout::openRenameChannelWindow(data);
		}
	};

	menu.popup();
}

/* -------------------------------------------------------------------------- */

void geGroupChannel::resize(int X, int Y, int W, int H)
{
	geChannel::resize(X, Y, W, H);
}
} // namespace giada::v