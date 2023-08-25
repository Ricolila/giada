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

#include "core/channels/midiSender.h"
#include "core/kernelMidi.h"
#include "core/mixer.h"

namespace giada::m
{
MidiSender::MidiSender(KernelMidi& k)
: kernelMidi(&k)
, onSend(nullptr)
{
}

/* -------------------------------------------------------------------------- */

MidiSender::MidiSender(const Patch::Channel& p, KernelMidi& k)
: kernelMidi(&k)
{
}

/* -------------------------------------------------------------------------- */

void MidiSender::advance(ID channelId, const Sequencer::Event& e, int outputFilter) const
{
	if (e.type == Sequencer::EventType::ACTIONS)
		parseActions(channelId, *e.actions, outputFilter);
}

/* -------------------------------------------------------------------------- */

void MidiSender::stop(int outputFilter)
{
	send(MidiEvent::makeFromRaw(G_MIDI_ALL_NOTES_OFF, /*numBytes=*/3), outputFilter);
}

/* -------------------------------------------------------------------------- */

void MidiSender::send(MidiEvent e, int outputFilter) const
{
	assert(onSend != nullptr);

	e.setChannel(outputFilter);
	kernelMidi->send(e);
	onSend();
}

/* -------------------------------------------------------------------------- */

void MidiSender::parseActions(ID channelId, const std::vector<Action>& as, int outputFilter) const
{
	for (const Action& a : as)
		if (a.channelId == channelId)
			send(a.event, outputFilter);
}
} // namespace giada::m