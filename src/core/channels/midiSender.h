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

#ifndef G_CHANNEL_MIDI_SENDER_H
#define G_CHANNEL_MIDI_SENDER_H

#include "core/patch.h"
#include "core/sequencer.h"

namespace giada::m
{
class KernelMidi;
class MidiSender final
{
public:
	MidiSender(KernelMidi&);
	MidiSender(const Patch::Channel& p, KernelMidi&);

	void advance(ID channelId, const Sequencer::Event& e, int outputFilter) const;

	void stop(int outputFilter);

	KernelMidi* kernelMidi;

	/* enabled
    Tells whether MIDI output is enabled or not. */

	bool enabled;

	/* filter
    Which MIDI channel data should be sent to. */

	int filter;

	/* onSend
	Callback fired when a MIDI signal has been sent. */

	std::function<void()> onSend;

private:
	void send(MidiEvent e, int outputFilter) const;
	void parseActions(ID channelId, const std::vector<Action>& as, int outputFilter) const;
};
} // namespace giada::m

#endif
