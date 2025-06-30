/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "dispatcher.h"
#include "glue/channel.h"
#include "glue/main.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/ui.h"
#include <FL/Fl.H>
#include <cassert>

namespace giada::v
{
Dispatcher::Dispatcher()
: m_model(nullptr)
, m_keyPressed(-1)
{
}

/* -------------------------------------------------------------------------- */

void Dispatcher::init(gdMainWindow& mw, const Model& model)
{
	m_mainWindow = &mw;
	m_model      = &model;
}

/* -------------------------------------------------------------------------- */

void Dispatcher::perform(ID channelId, int event) const
{
	if (event == FL_KEYDOWN)
	{
		if (Fl::event_shift())
			c::channel::killChannel(channelId, Thread::MAIN);
		else
			c::channel::pressChannel(channelId, G_MAX_VELOCITY_FLOAT, Thread::MAIN);
	}
	else if (event == FL_KEYUP)
		c::channel::releaseChannel(channelId, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

/* Walk channels array, trying to match button's bound key with the event. If
found, trigger the key-press/key-release function. */

void Dispatcher::dispatchChannels(int event) const
{
	m_mainWindow->keyboard->forEachChannel([this, event](geChannel& c)
	{
		if (c.handleKey(event))
			perform(c.getData().id, event);
	});
}

/* -------------------------------------------------------------------------- */

void Dispatcher::dispatchKey(int event)
{
	assert(onEventOccured != nullptr);
	assert(m_model != nullptr);

	/* These events come from the keyboard, not from a direct interaction on the
	UI with the mouse/touch. */

	if (event == FL_KEYDOWN)
	{
		if (m_keyPressed == Fl::event_key()) // Avoid key retrig
			return;

		m_keyPressed = Fl::event_key();

		if (m_keyPressed == m_model->keyBindPlay)
			c::main::toggleSequencer();
		else if (m_keyPressed == m_model->keyBindRewind)
			c::main::rewindSequencer();
		else if (m_keyPressed == m_model->keyBindRecordActions)
			c::main::toggleActionRecording();
		else if (m_keyPressed == m_model->keyBindRecordInput)
			c::main::toggleInputRecording();
		else if (m_keyPressed == m_model->keyBindExit)
		{
			c::main::stopActionRecording();
			c::main::stopInputRecording();
		}
		else
		{
			onEventOccured();
			dispatchChannels(event);
		}
	}
	else if (event == FL_KEYUP)
	{
		m_keyPressed = -1;
		dispatchChannels(event);
	}
}

/* -------------------------------------------------------------------------- */

void Dispatcher::dispatchTouch(const geChannel& gch, bool status)
{
	assert(onEventOccured != nullptr);

	onEventOccured();
	perform(gch.getData().id, status ? FL_KEYDOWN : FL_KEYUP);
}
} // namespace giada::v