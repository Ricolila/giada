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

#ifndef GD_MIDI_OUTPUT_BASE_H
#define GD_MIDI_OUTPUT_BASE_H

#include "src/glue/io.h"
#include "src/gui/dialogs/window.h"
#include "src/gui/elems/midiIO/midiLearner.h"
#include "src/gui/elems/midiIO/midiLearnerPack.h"

/* There's no such thing as a gdMidiOutputMaster vs gdMidiOutputChannel. MIDI
output master is managed by the configuration window, hence gdMidiOutput deals
only with channels.

Both MidiOutputMidiCh and MidiOutputSampleCh have the MIDI lighting widget set.
In addition MidiOutputMidiCh has the MIDI message output box. */

namespace giada::v
{
class geCheck;
class geTextButton;
class geLightningLearnerPack : public geMidiLearnerPack
{
public:
	geLightningLearnerPack(int x, int y, ID channelId);

	void update(const c::io::Channel_OutputData&);
};

/* -------------------------------------------------------------------------- */

class gdMidiOutputBase : public gdWindow
{
public:
	gdMidiOutputBase(int w, int h, ID channelId);
	~gdMidiOutputBase();

protected:
	ID m_channelId;

	geLightningLearnerPack* m_learners;
	geTextButton*           m_close;
	geCheck*                m_enableLightning;
};
} // namespace giada::v

#endif
