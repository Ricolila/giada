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

#include "midiReceiver.h"
#include "core/eventDispatcher.h"
#include "core/plugins/pluginHost.h"

namespace giada::m
{
void MidiReceiver::advance(ID channelId, ChannelShared::MidiQueue& midiQueue, const Sequencer::Event& e) const
{
	if (e.type != Sequencer::EventType::ACTIONS)
		return;
	for (const Action& action : *e.actions)
		if (action.channelId == channelId)
			sendToPlugins(midiQueue, action.event, e.delta);
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::render(ChannelShared& shared, const std::vector<Plugin*>& plugins, PluginHost& pluginHost) const
{
	shared.midiBuffer.clear();

	MidiEvent e;
	while (shared.midiQueue.pop(e))
	{
		juce::MidiMessage message = juce::MidiMessage(
		    e.getStatus(),
		    e.getNote(),
		    e.getVelocity());
		shared.midiBuffer.addEvent(message, e.getDelta());
	}

	pluginHost.processStack(shared.audioBuffer, plugins, &shared.midiBuffer);
	shared.midiBuffer.clear();
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::stop(ChannelShared::MidiQueue& midiQueue) const
{
	sendToPlugins(midiQueue, MidiEvent::makeFromRaw(G_MIDI_ALL_NOTES_OFF, /*numBytes=*/3), 0);
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::sendToPlugins(ChannelShared::MidiQueue& midiQueue, const MidiEvent& e, Frame localFrame) const
{
	MidiEvent eWithDelta(e);
	eWithDelta.setDelta(localFrame);
	midiQueue.push(eWithDelta);
}

/* -------------------------------------------------------------------------- */

void MidiReceiver::parseMidi(ChannelShared::MidiQueue& midiQueue, const MidiEvent& e) const
{
	/* Now all messages are turned into Channel-0 messages. Giada doesn't care 
	about holding MIDI channel information. Moreover, having all internal 
	messages on channel 0 is way easier. Then send it to plug-ins. */

	MidiEvent flat(e);
	flat.setChannel(0);
	sendToPlugins(midiQueue, flat, /*delta=*/0);
}
} // namespace giada::m
