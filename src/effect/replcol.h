/* ase -- allegro-sprite-editor: the ultimate sprites factory
 * Copyright (C) 2001-2005  David A. Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef EFFECT_REPLCOL_H
#define EFFECT_REPLCOL_H

struct Effect;

void set_replace_colors (int from, int to, int fuzziness);

void apply_replace_color4 (struct Effect *effect);
void apply_replace_color2 (struct Effect *effect);
void apply_replace_color1 (struct Effect *effect);

#endif /* EFFECT_REPLCOL_H */
