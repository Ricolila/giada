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

#include "core/channels/channel.h"
#include "core/actions/actionRecorder.h"
#include "core/channels/sampleAdvancer.h"
#include "core/conf.h"
#include "core/engine.h"
#include "core/midiMapper.h"
#include "core/model/model.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include <cassert>

extern giada::m::Engine g_engine;

namespace giada::m
{
Channel::Channel(ChannelType type, ID id, ID columnId, int position, ChannelShared& s)
: shared(&s)
, id(id)
, type(type)
, columnId(columnId)
, position(position)
, volume(G_DEFAULT_VOL)
, volume_i(G_DEFAULT_VOL)
, pan(G_DEFAULT_PAN)
, armed(false)
, key(0)
, hasActions(false)
, height(G_GUI_UNIT)
, midiLighter(g_engine.getMidiMapper())
, m_mute(false)
, m_solo(false)
{
	switch (type)
	{
	case ChannelType::SAMPLE:
		samplePlayer.emplace(&(shared->resampler.value()));
		sampleAdvancer.emplace();
		sampleReactor.emplace(*shared, id);
		audioReceiver.emplace();
		sampleActionRecorder.emplace(g_engine.getActionRecorder());
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace(&(shared->resampler.value()));
		sampleReactor.emplace(*shared, id);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(g_engine.getKernelMidi());
		midiActionRecorder.emplace(g_engine.getActionRecorder());
		midiReceiver.emplace();
		break;

	default:
		break;
	}

	initCallbacks();
}

/* -------------------------------------------------------------------------- */

Channel::Channel(const Patch::Channel& p, ChannelShared& s, float samplerateRatio, Wave* wave, std::vector<Plugin*> plugins)
: shared(&s)
, id(p.id)
, type(p.type)
, columnId(p.columnId)
, position(p.position)
, volume(p.volume)
, volume_i(G_DEFAULT_VOL)
, pan(p.pan)
, armed(p.armed)
, key(p.key)
, hasActions(p.hasActions)
, name(p.name)
, height(p.height)
, plugins(plugins)
, midiLearner(p)
, midiLighter(g_engine.getMidiMapper(), p)
, m_mute(p.mute)
, m_solo(p.solo)
{
	shared->readActions.store(p.readActions);
	shared->recStatus.store(p.readActions ? ChannelStatus::PLAY : ChannelStatus::OFF);

	switch (type)
	{
	case ChannelType::SAMPLE:
		samplePlayer.emplace(p, samplerateRatio, &(shared->resampler.value()), wave);
		sampleAdvancer.emplace();
		sampleReactor.emplace(*shared, id);
		audioReceiver.emplace(p);
		sampleActionRecorder.emplace(g_engine.getActionRecorder());
		break;

	case ChannelType::PREVIEW:
		samplePlayer.emplace(p, samplerateRatio, &(shared->resampler.value()), nullptr);
		sampleReactor.emplace(*shared, id);
		break;

	case ChannelType::MIDI:
		midiController.emplace();
		midiSender.emplace(p, g_engine.getKernelMidi());
		midiActionRecorder.emplace(g_engine.getActionRecorder());
		midiReceiver.emplace();
		break;

	default:
		break;
	}

	initCallbacks();
}

/* -------------------------------------------------------------------------- */

Channel::Channel(const Channel& other)
: midiLighter(g_engine.getMidiMapper())
{
	*this = other;
}

/* -------------------------------------------------------------------------- */

Channel& Channel::operator=(const Channel& other)
{
	if (this == &other)
		return *this;

	shared     = other.shared;
	id         = other.id;
	type       = other.type;
	columnId   = other.columnId;
	position   = other.position;
	volume     = other.volume;
	volume_i   = other.volume_i;
	pan        = other.pan;
	m_mute     = other.m_mute;
	m_solo     = other.m_solo;
	armed      = other.armed;
	key        = other.key;
	hasActions = other.hasActions;
	name       = other.name;
	height     = other.height;
	plugins    = other.plugins;

	midiLearner          = other.midiLearner;
	midiLighter          = other.midiLighter;
	samplePlayer         = other.samplePlayer;
	sampleAdvancer       = other.sampleAdvancer;
	sampleReactor        = other.sampleReactor;
	audioReceiver        = other.audioReceiver;
	midiController       = other.midiController;
	midiReceiver         = other.midiReceiver;
	midiSender           = other.midiSender;
	sampleActionRecorder = other.sampleActionRecorder;
	midiActionRecorder   = other.midiActionRecorder;

	if (samplePlayer)
		samplePlayer->waveReader.setResampler(&shared->resampler.value());

	initCallbacks();

	return *this;
}

/* -------------------------------------------------------------------------- */

bool Channel::operator==(const Channel& other)
{
	return id == other.id;
}

/* -------------------------------------------------------------------------- */

bool Channel::isInternal() const
{
	return type == ChannelType::MASTER || type == ChannelType::PREVIEW;
}

bool Channel::isMuted() const
{
	/* Internals can't be muted. */
	return !isInternal() && m_mute;
}

bool Channel::isSoloed() const
{
	return m_solo;
}

bool Channel::isAudible(bool mixerHasSolos) const
{
	if (isInternal())
		return true;
	if (isMuted())
		return false;
	return !mixerHasSolos || (mixerHasSolos && isSoloed());
}

bool Channel::canInputRec() const
{
	if (type != ChannelType::SAMPLE)
		return false;

	bool hasWave     = samplePlayer->hasWave();
	bool isProtected = audioReceiver->overdubProtection;
	bool canOverdub  = !hasWave || (hasWave && !isProtected);

	return armed && canOverdub;
}

bool Channel::canActionRec() const
{
	return hasWave() && !samplePlayer->isAnyLoopMode();
}

bool Channel::hasWave() const
{
	return samplePlayer && samplePlayer->hasWave();
}

bool Channel::isPlaying() const
{
	ChannelStatus s = shared->playStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}

/* -------------------------------------------------------------------------- */

void Channel::setMute(bool v)
{
	if (m_mute != v)
		midiLighter.sendMute(v);
	m_mute = v;
}

void Channel::setSolo(bool v)
{
	if (m_solo != v)
		midiLighter.sendSolo(v);
	m_solo = v;
}

/* -------------------------------------------------------------------------- */

void Channel::initCallbacks()
{
	shared->playStatus.onChange = [this](ChannelStatus status) {
		midiLighter.sendStatus(status, isAudible(/*mixerHasSolos = TODO!*/ false));
	};

	if (samplePlayer)
	{
		samplePlayer->onLastFrame = [this](bool natural, bool seqIsRunning) {
			sampleAdvancer->onLastFrame(*shared, seqIsRunning, natural, samplePlayer->mode,
			    samplePlayer->isAnyLoopMode());
		};
	}
}

/* -------------------------------------------------------------------------- */

void Channel::advance(const Sequencer::EventBuffer& events, Range<Frame> block, Frame quantizerStep) const
{
	if (shared->quantizer)
		shared->quantizer->advance(block, quantizerStep);

	for (const Sequencer::Event& e : events)
	{
		if (midiController)
			midiController->advance(shared->playStatus, e);

		if (samplePlayer)
			sampleAdvancer->advance(id, *shared, e, samplePlayer->mode, samplePlayer->isAnyLoopMode());

		if (midiSender && isPlaying() && !isMuted())
			midiSender->advance(id, e);

		if (midiReceiver && isPlaying())
			midiReceiver->advance(id, shared->midiQueue, e);
	}
}
} // namespace giada::m