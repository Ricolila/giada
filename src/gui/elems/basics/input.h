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

#ifndef GE_INPUT_H
#define GE_INPUT_H

#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/flex.h"
#include <FL/Fl_Input.H>
#include <functional>
#include <string>

namespace giada::v
{
class geInput : public geFlex
{
public:
	geInput(int x, int y, int w, int h, const char* l = nullptr, int labelWidth = 0);
	geInput(const char* l = nullptr, int labelWidth = 0);

	std::string getValue() const;

	void setType(int);
	void setWhen(int);
	void setLabelAlign(Fl_Align);
	void setMaximumSize(int);
	void setReadonly(bool);
	void setCursorColor(int);
	void setValue(const std::string&);

	std::function<void(const std::string&)> onChange = nullptr;

private:
	static void cb_onChange(Fl_Widget* w, void* p);
	void        cb_onChange();

	geBox*    m_text;
	Fl_Input* m_input;
};
} // namespace giada::v

#endif
