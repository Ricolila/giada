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

#ifndef GD_MIDI_ACTION_EDITOR_H
#define GD_MIDI_ACTION_EDITOR_H

#include "src/gui/dialogs/actionEditor/baseActionEditor.h"

namespace giada::v
{
class gePianoRoll;
class geVelocityEditor;
class gdMidiActionEditor : public gdBaseActionEditor
{
public:
	gdMidiActionEditor(ID channelId, const Model&);
	~gdMidiActionEditor();

	void rebuild() override;

private:
	gePianoRoll*      m_pianoRoll;
	geVelocityEditor* m_velocityEditor;
};
} // namespace giada::v

#endif
