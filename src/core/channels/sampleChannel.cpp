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

#include "core/channels/sampleChannel.h"
#include "core/const.h"

namespace giada::m
{
SampleChannel::SampleChannel()
: inputMonitor(false)
, overdubProtection(false)
, mode(SamplePlayerMode::SINGLE_BASIC)
, pitch(G_DEFAULT_PITCH)
{
}

/* -------------------------------------------------------------------------- */

SampleChannel::SampleChannel(const Patch::Channel& p)
: inputMonitor(p.inputMonitor)
, overdubProtection(p.overdubProtection)
, mode(p.mode)
, pitch(p.pitch)
{
}

/* -------------------------------------------------------------------------- */

bool SampleChannel::isAnyLoopMode() const
{
	return mode == SamplePlayerMode::LOOP_BASIC ||
	       mode == SamplePlayerMode::LOOP_ONCE ||
	       mode == SamplePlayerMode::LOOP_REPEAT ||
	       mode == SamplePlayerMode::LOOP_ONCE_BAR;
}
} // namespace giada::m
