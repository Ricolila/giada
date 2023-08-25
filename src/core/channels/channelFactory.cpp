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

#include "core/channels/channelFactory.h"
#include "core/channels/channel.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/wave.h"
#include <cassert>
#include <memory>

namespace giada::m::channelFactory
{
namespace
{
IdManager channelId_;

/* -------------------------------------------------------------------------- */

std::unique_ptr<ChannelShared> makeShared_(ChannelType type, int bufferSize, Resampler::Quality quality)
{
	std::unique_ptr<ChannelShared> shared = std::make_unique<ChannelShared>(bufferSize);

	if (type == ChannelType::SAMPLE || type == ChannelType::PREVIEW)
	{
		shared->quantizer.emplace();
		shared->renderQueue.emplace();
		shared->resampler.emplace(quality, G_MAX_IO_CHANS);
	}

	return shared;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ID getNextId()
{
	return channelId_.getNext();
}

/* -------------------------------------------------------------------------- */

void reset()
{
	channelId_ = IdManager();
}

/* -------------------------------------------------------------------------- */

Data create(ID channelId, ChannelType type, ID columnId, int position, int bufferSize, Resampler::Quality quality, bool overdubProtection)
{
	std::unique_ptr<ChannelShared> shared = makeShared_(type, bufferSize, quality);
	Channel                        ch     = Channel(type, channelId_.generate(channelId), columnId, position, *shared.get());

	if (ch.sampleChannel)
		ch.sampleChannel->overdubProtection = overdubProtection;

	return {ch, std::move(shared)};
}

/* -------------------------------------------------------------------------- */

Data create(const Channel& o, int bufferSize, Resampler::Quality quality)
{
	std::unique_ptr<ChannelShared> shared = makeShared_(o.type, bufferSize, quality);
	Channel                        ch     = Channel(o);

	ch.id     = channelId_.generate();
	ch.shared = shared.get();

	return {ch, std::move(shared)};
}

/* -------------------------------------------------------------------------- */

Data deserializeChannel(const Patch::Channel& pch, float samplerateRatio, int bufferSize, Resampler::Quality quality, Wave* wave, std::vector<Plugin*> plugins)
{
	channelId_.set(pch.id);

	std::unique_ptr<ChannelShared> shared = makeShared_(pch.type, bufferSize, quality);
	Channel                        ch     = Channel(pch, *shared.get(), samplerateRatio, wave, plugins);

	return {ch, std::move(shared)};
}

/* -------------------------------------------------------------------------- */

const Patch::Channel serializeChannel(const Channel& c)
{
	Patch::Channel pc;

	for (const Plugin* p : c.plugins)
		pc.pluginIds.push_back(p->id);

	pc.id                = c.id;
	pc.type              = c.type;
	pc.columnId          = c.columnId;
	pc.position          = c.position;
	pc.height            = c.height;
	pc.name              = c.name;
	pc.key               = c.key;
	pc.mute              = c.isMuted();
	pc.solo              = c.isSoloed();
	pc.volume            = c.volume;
	pc.pan               = c.pan;
	pc.hasActions        = c.hasActions;
	pc.readActions       = c.shared->readActions.load();
	pc.armed             = c.armed;
	pc.midiIn            = c.midiInput.enabled;
	pc.midiInFilter      = c.midiInput.filter;
	pc.midiInKeyPress    = c.midiInput.keyPress.getValue();
	pc.midiInKeyRel      = c.midiInput.keyRelease.getValue();
	pc.midiInKill        = c.midiInput.kill.getValue();
	pc.midiInArm         = c.midiInput.arm.getValue();
	pc.midiInVolume      = c.midiInput.volume.getValue();
	pc.midiInMute        = c.midiInput.mute.getValue();
	pc.midiInSolo        = c.midiInput.solo.getValue();
	pc.midiInReadActions = c.midiInput.readActions.getValue();
	pc.midiInPitch       = c.midiInput.pitch.getValue();
	pc.midiOutL          = c.midiLightning.enabled;
	pc.midiOutLplaying   = c.midiLightning.playing.getValue();
	pc.midiOutLmute      = c.midiLightning.mute.getValue();
	pc.midiOutLsolo      = c.midiLightning.solo.getValue();

	if (c.type == ChannelType::SAMPLE)
	{
		pc.waveId            = c.sampleChannel->getWaveId();
		pc.mode              = c.sampleChannel->mode;
		pc.begin             = c.sampleChannel->begin;
		pc.end               = c.sampleChannel->end;
		pc.pitch             = c.sampleChannel->pitch;
		pc.shift             = c.sampleChannel->shift;
		pc.midiInVeloAsVol   = c.sampleChannel->velocityAsVol;
		pc.inputMonitor      = c.sampleChannel->inputMonitor;
		pc.overdubProtection = c.sampleChannel->overdubProtection;
	}
	else if (c.type == ChannelType::MIDI)
	{
		pc.midiOut     = c.midiChannel->outputEnabled;
		pc.midiOutChan = c.midiChannel->outputFilter;
	}

	return pc;
}
} // namespace giada::m::channelFactory
