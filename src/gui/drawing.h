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

#ifndef G_V_DRAWING_H
#define G_V_DRAWING_H

#include "src/deps/geompp/src/rect.hpp"
#include <FL/fl_draw.H>
#include <string>

namespace giada::v
{
void drawRectf(geompp::Rect<int>, Fl_Color);
void drawRect(geompp::Rect<int>, Fl_Color);
void drawLine(geompp::Line<int>, Fl_Color);
void drawText(const std::string&, geompp::Rect<int>, Fl_Font, Fl_Fontsize, Fl_Color, int alignment = FL_ALIGN_CENTER);
void drawTextVertical(const std::string&, geompp::Rect<int>, Fl_Font, Fl_Fontsize, Fl_Color);

/* drawImage
Draws image into a rectangle, truncating overflow. */

void drawImage(Fl_Image&, geompp::Rect<int>);

/* toImage
Rasterizes a Widget to a Fl_RGB_Image screenshot. */

Fl_RGB_Image* toImage(const Fl_Widget&);
} // namespace giada::v

#endif