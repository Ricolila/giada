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

#include "core/channels/channelShared.h"

namespace giada::m
{
ChannelShared::ChannelShared(ID id, Frame bufferSize)
: id(id)
, audioBuffer(bufferSize, G_MAX_IO_CHANS)
{
}

/* -------------------------------------------------------------------------- */

bool ChannelShared::isReadingActions() const
{
	const ChannelStatus status = recStatus.load();
	const bool          read   = readActions.load();
	return read && (status == ChannelStatus::PLAY || status == ChannelStatus::ENDING);
}

/* -------------------------------------------------------------------------- */

void ChannelShared::setBufferSize(int bufferSize)
{
	audioBuffer.alloc(bufferSize, audioBuffer.countChannels());
}
} // namespace giada::m