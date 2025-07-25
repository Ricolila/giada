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

#include "src/gui/elems/basics/scrollPack.h"
#include "src/gui/elems/basics/boxtypes.h"
#include <cassert>
#include <cstddef>

namespace giada::v
{
geScrollPack::geScrollPack(int x, int y, int w, int h, int type, Direction dir,
    int gutter)
: geScroll(x, y, w, h, type)
, m_direction(dir)
, m_gutter(gutter)
{
	end();
}

/* -------------------------------------------------------------------------- */

geScrollPack::geScrollPack(int type, Direction dir, int gutter)
: geScrollPack(0, 0, 0, 0, type, dir, gutter)
{
}

/* -------------------------------------------------------------------------- */

std::size_t geScrollPack::countChildren() const
{
	return m_widgets.size();
}

/* -------------------------------------------------------------------------- */

void geScrollPack::add(Fl_Widget* w)
{
	if (countChildren() == 0)
		w->position(x(), y());
	else if (m_direction == Direction::HORIZONTAL)
		w->position((getLastChild()->x() + getLastChild()->w() + m_gutter), y());
	else
		w->position(x(), (getLastChild()->y() + getLastChild()->h() + m_gutter));

	geScroll::add(w);
	m_widgets.push_back(w);
}

/* -------------------------------------------------------------------------- */

Fl_Widget* geScrollPack::getChild(std::size_t i)
{
	return m_widgets.at(i); // Throws std::out_of_range in case
}

/* -------------------------------------------------------------------------- */

Fl_Widget* geScrollPack::getLastChild()
{
	return m_widgets.at(m_widgets.size() - 1); // Throws std::out_of_range in case
}
} // namespace giada::v
