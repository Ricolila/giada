/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#ifndef G_WAVE_FX_H
#define G_WAVE_FX_H


class Wave;


namespace giada {
namespace m {
namespace wfx
{
static const int FADE_IN  = 0;
static const int FADE_OUT = 1;
static const int SMOOTH_SIZE = 32;

/* normalizeSoft
Normalizes the wave by returning the dB value for the boost volume. */

float normalizeSoft(Wave* w);

int monoToStereo(Wave* w);
void silence(Wave* w, int a, int b);
int cut(Wave* w, int a, int b);
int trim(Wave* w, int a, int b);

/* fade
 * fade in or fade out selection. Fade In = type 0, Fade Out = type 1 */

void fade(Wave* w, int a, int b, int type);

/* smooth
 * smooth edges of selection. */

void smooth(Wave* w, int a, int b);
}}}; // giada::m::wfx::

#endif
