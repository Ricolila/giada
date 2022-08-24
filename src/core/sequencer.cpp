/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/sequencer.h"
#include "core/actions/actionRecorder.h"
#include "core/jackTransport.h"
#include "core/kernelAudio.h"
#include "core/metronome.h"
#include "core/midiSynchronizer.h"
#include "core/model/model.h"
#include "core/quantizer.h"
#include "utils/log.h"
#include "utils/math.h"
#include "utils/time.h"

namespace giada::m
{
namespace
{
constexpr int Q_ACTION_REWIND = 0;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Sequencer::Sequencer(model::Model& m, MidiSynchronizer& s, JackTransport& j)
: onAboutStart(nullptr)
, onAboutStop(nullptr)
, m_model(m)
, m_midiSynchronizer(s)
, m_jackTransport(j)
, m_quantizerStep(1)
{
	m_quantizer.schedule(Q_ACTION_REWIND, [this](Frame delta) { rewindQ(delta); });
}
/* -------------------------------------------------------------------------- */

bool      Sequencer::canQuantize() const { return m_model.get().sequencer.canQuantize(); }
bool      Sequencer::isRunning() const { return m_model.get().sequencer.isRunning(); }
bool      Sequencer::isActive() const { return m_model.get().sequencer.isActive(); }
bool      Sequencer::isOnBar() const { return m_model.get().sequencer.a_isOnBar(); }
bool      Sequencer::isOnBeat() const { return m_model.get().sequencer.a_isOnBeat(); }
bool      Sequencer::isOnFirstBeat() const { return m_model.get().sequencer.a_isOnFirstBeat(); }
float     Sequencer::getBpm() const { return m_model.get().sequencer.bpm; }
int       Sequencer::getBeats() const { return m_model.get().sequencer.beats; }
int       Sequencer::getBars() const { return m_model.get().sequencer.bars; }
int       Sequencer::getCurrentBeat() const { return m_model.get().sequencer.a_getCurrentBeat(); }
Frame     Sequencer::getCurrentFrame() const { return m_model.get().sequencer.a_getCurrentFrame(); }
Frame     Sequencer::getCurrentFrameQuantized() const { return quantize(getCurrentFrame()); }
float     Sequencer::getCurrentSecond(int sampleRate) const { return getCurrentFrame() / static_cast<float>(sampleRate); }
Frame     Sequencer::getFramesInBar() const { return m_model.get().sequencer.framesInBar; }
Frame     Sequencer::getFramesInBeat() const { return m_model.get().sequencer.framesInBeat; }
Frame     Sequencer::getFramesInLoop() const { return m_model.get().sequencer.framesInLoop; }
Frame     Sequencer::getFramesInSeq() const { return m_model.get().sequencer.framesInSeq; }
int       Sequencer::getQuantizerValue() const { return m_model.get().sequencer.quantize; }
int       Sequencer::getQuantizerStep() const { return m_quantizerStep; }
SeqStatus Sequencer::getStatus() const { return m_model.get().sequencer.status; }

/* -------------------------------------------------------------------------- */

Frame Sequencer::getMaxFramesInLoop(int sampleRate) const
{
	return (sampleRate * (60.0f / G_MIN_BPM)) * getBeats();
}

/* -------------------------------------------------------------------------- */

float Sequencer::calcBpmFromRec(Frame recordedFrames, int sampleRate) const
{
	return (60.0f * getBeats()) / (recordedFrames / static_cast<float>(sampleRate));
}

/* -------------------------------------------------------------------------- */

Frame Sequencer::quantize(Frame f) const
{
	if (!canQuantize())
		return f;
	return u::math::quantize(f, m_quantizerStep) % getFramesInLoop(); // No overflow
}

/* -------------------------------------------------------------------------- */

void Sequencer::reset(int sampleRate)
{
	model::Sequencer& s = m_model.get().sequencer;

	s.bars     = G_DEFAULT_BARS;
	s.beats    = G_DEFAULT_BEATS;
	s.bpm      = G_DEFAULT_BPM;
	s.quantize = G_DEFAULT_QUANTIZE;
	recomputeFrames(sampleRate); // Model swap is done here, no need to call it twice
	rewind();
}

/* -------------------------------------------------------------------------- */

void Sequencer::react(const EventDispatcher::EventBuffer& events, int sampleRate)
{
	for (const EventDispatcher::Event& e : events)
	{
		switch (e.type)
		{
		case EventDispatcher::EventType::SEQUENCER_START:
			if (!m_jackTransport.start())
				rawStart();
			break;

		case EventDispatcher::EventType::SEQUENCER_STOP:
			if (!m_jackTransport.stop())
				rawStop();
			break;

		case EventDispatcher::EventType::SEQUENCER_REWIND:
			if (!m_jackTransport.setPosition(0))
				rawRewind();
			break;

		case EventDispatcher::EventType::SEQUENCER_GO_TO_BEAT:
		{
			const float bpm   = m_model.get().sequencer.bpm;
			const int   beat  = std::any_cast<int>(e.data);
			const Frame frame = u::time::beatToFrame(beat, sampleRate, bpm);
			if (!m_jackTransport.setPosition(frame))
				rawGoToBeat(beat, sampleRate);
			break;
		}
		case EventDispatcher::EventType::SEQUENCER_BPM:
		{
			const float bpm = std::any_cast<float>(e.data);
			if (!m_jackTransport.setBpm(bpm))
				rawSetBpm(bpm, sampleRate);
			break;
		}
#ifdef WITH_AUDIO_JACK
		case EventDispatcher::EventType::SEQUENCER_START_JACK:
			rawStart();
			break;

		case EventDispatcher::EventType::SEQUENCER_STOP_JACK:
			rawStop();
			break;

		case EventDispatcher::EventType::SEQUENCER_REWIND_JACK:
			rawRewind();
			break;

		case EventDispatcher::EventType::SEQUENCER_BPM_JACK:
			rawSetBpm(std::any_cast<float>(e.data), sampleRate);
			break;
#endif

		default:
			break;
		}
	}
}

/* -------------------------------------------------------------------------- */

const Sequencer::EventBuffer& Sequencer::advance(Frame bufferSize, int sampleRate, const ActionRecorder& actionRecorder)
{
	m_eventBuffer.clear();

	const model::Sequencer& sequencer = m_model.get().sequencer;

	const Frame start        = sequencer.a_getCurrentFrame();
	const Frame end          = start + bufferSize;
	const Frame framesInLoop = sequencer.framesInLoop;
	const Frame framesInBar  = sequencer.framesInBar;
	const Frame framesInBeat = sequencer.framesInBeat;
	const Frame nextFrame    = end % framesInLoop;

	/* Process events in the current block. */

	for (Frame i = start, local = 0; i < end; i++, local++)
	{

		Frame global = i % framesInLoop; // wraps around 'framesInLoop'

		if (global == 0)
		{
			m_eventBuffer.push_back({EventType::FIRST_BEAT, global, local});
			m_metronome.trigger(Metronome::Click::BEAT, local);
		}
		else if (global % framesInBar == 0)
		{
			m_eventBuffer.push_back({EventType::BAR, global, local});
			m_metronome.trigger(Metronome::Click::BAR, local);
		}
		else if (global % framesInBeat == 0)
		{
			m_metronome.trigger(Metronome::Click::BEAT, local);
		}

		const std::vector<Action>* as = actionRecorder.getActionsOnFrame(global);
		if (as != nullptr)
			m_eventBuffer.push_back({EventType::ACTIONS, global, local, as});
	}

	/* Advance this and quantizer after the event parsing. */

	sequencer.a_setCurrentFrame(nextFrame, sampleRate);
	m_quantizer.advance(Range<Frame>(start, end), getQuantizerStep());
	m_midiSynchronizer.advance(geompp::Range<Frame>(start, end), sequencer.framesInBeat);

	return m_eventBuffer;
}

/* -------------------------------------------------------------------------- */

void Sequencer::render(mcl::AudioBuffer& outBuf)
{
	if (m_metronome.running)
		m_metronome.render(outBuf);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawStart()
{
	assert(onAboutStart != nullptr);

	const SeqStatus status = getStatus();

	onAboutStart(status);

	switch (status)
	{
	case SeqStatus::STOPPED:
		setStatus(SeqStatus::RUNNING);
		break;
	case SeqStatus::WAITING:
		setStatus(SeqStatus::RUNNING);
		break;
	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawStop()
{
	assert(onAboutStop != nullptr);

	onAboutStop();
	setStatus(SeqStatus::STOPPED);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawRewind()
{
	if (canQuantize())
		m_quantizer.trigger(Q_ACTION_REWIND);
	else
		rewindQ(/*delta=*/0);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rewind()
{
	const model::Sequencer& c = m_model.get().sequencer;

	c.a_setCurrentFrame(0, /*sampleRate=*/0); // No need for sampleRate, it's just 0
}

/* -------------------------------------------------------------------------- */

bool Sequencer::isMetronomeOn() const { return m_metronome.running; }
void Sequencer::toggleMetronome() { m_metronome.running = !m_metronome.running; }
void Sequencer::setMetronome(bool v) { m_metronome.running = v; }

/* -------------------------------------------------------------------------- */

void Sequencer::rewindQ(Frame delta)
{
	rewind();
	m_eventBuffer.push_back({EventType::REWIND, 0, delta});
}

/* -------------------------------------------------------------------------- */

void Sequencer::recomputeFrames(int sampleRate)
{
	model::Sequencer& s = m_model.get().sequencer;

	s.framesInBeat = u::time::beatToFrame(1, sampleRate, s.bpm);
	s.framesInBar  = s.framesInBeat * s.bars;
	s.framesInLoop = s.framesInBeat * s.beats;
	s.framesInSeq  = s.framesInBeat * G_MAX_BEATS;

	if (s.quantize != 0)
		m_quantizerStep = s.framesInBeat / s.quantize;

	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Sequencer::setBpm(float b, int sampleRate)
{
	b = std::clamp(b, G_MIN_BPM, G_MAX_BPM);

	/* If JACK is being used, let it handle the bpm change. */
	if (!m_jackTransport.setBpm(b))
		rawSetBpm(b, sampleRate);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawSetBpm(float v, int sampleRate)
{
	assert(onBpmChange != nullptr);

	const float oldVal = m_model.get().sequencer.bpm;
	const float newVal = std::clamp(v, G_MIN_BPM, G_MAX_BPM);

	m_model.get().sequencer.bpm = newVal;
	recomputeFrames(sampleRate);
	m_model.swap(model::SwapType::HARD);

	onBpmChange(oldVal, newVal, m_quantizerStep);

	u::log::print("[clock::rawSetBpm] Bpm changed to %f\n", newVal);
}
/* -------------------------------------------------------------------------- */

void Sequencer::setBeats(int newBeats, int newBars, int sampleRate)
{
	newBeats = std::clamp(newBeats, 1, G_MAX_BEATS);
	newBars  = std::clamp(newBars, 1, newBeats); // Bars cannot be greater than beats

	m_model.get().sequencer.beats = newBeats;
	m_model.get().sequencer.bars  = newBars;
	recomputeFrames(sampleRate);

	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawGoToBeat(int beat, int sampleRate)
{
	m_model.get().sequencer.a_setCurrentBeat(beat, sampleRate);
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Sequencer::setQuantize(int q, int sampleRate)
{
	m_model.get().sequencer.quantize = q;
	recomputeFrames(sampleRate);

	m_model.swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void Sequencer::setStatus(SeqStatus s)
{
	m_model.get().sequencer.status = s;
	m_model.swap(model::SwapType::SOFT);

	/* Additional things to do when the status changes. */

	switch (s)
	{
	case SeqStatus::WAITING:
		rewind();
		m_midiSynchronizer.sendRewind();
		break;
	case SeqStatus::STOPPED:
		m_midiSynchronizer.sendStop();
		break;
	case SeqStatus::RUNNING:
		m_midiSynchronizer.sendStart();
		break;
	default:
		break;
	}
}
} // namespace giada::m