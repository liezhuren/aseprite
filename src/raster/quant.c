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

#include "config.h"

#ifndef USE_PRECOMPILED_HEADER

#include "raster/blend.h"
#include "raster/image.h"
#include "raster/quant.h"

#endif

Image *image_set_imgtype (Image *image, int imgtype,
			  int dithering_method,
			  RGB_MAP *rgb_map,
			  RGB *palette)
{
  unsigned long *rgb_address;
  unsigned short *gray_address;
  unsigned char *idx_address;
  int i, c, r, g, b, size;
  Image *new_image;

  /* no convertion */
  if (image->imgtype == imgtype)
    return NULL;
  /* RGB -> Indexed with ordered dithering */
  else if (image->imgtype == IMAGE_RGB &&
	   imgtype == IMAGE_INDEXED &&
	   dithering_method == DITHERING_ORDERED)
    return image_rgb_to_indexed (image, 0, 0, rgb_map, palette);

  new_image = image_new (imgtype, image->w, image->h);
  if (!new_image)
    return NULL;

  size = image->w*image->h;

  switch (image->imgtype) {

    case IMAGE_RGB:
      rgb_address = image->dat;

      switch (new_image->imgtype) {
	/* RGB -> Grayscale */
	case IMAGE_GRAYSCALE:
	  gray_address = new_image->dat;
	  for (i=0; i<size; i++) {
	    c = *rgb_address;
	    r = _rgba_getr (c);
	    g = _rgba_getg (c);
	    b = _rgba_getb (c);
	    rgb_to_hsv_int (&r, &g, &b);
	    *gray_address = _graya (b, _rgba_geta (c));
	    rgb_address++;
	    gray_address++;
	  }
	  break;
        /* RGB -> Indexed */
	case IMAGE_INDEXED:
	  idx_address = new_image->dat;
	  for (i=0; i<size; i++) {
	    c = *rgb_address;
	    r = _rgba_getr (c);
	    g = _rgba_getg (c);
	    b = _rgba_getb (c);
	    if (_rgba_geta (c) == 0)
	      *idx_address = 0;
	    else
	      *idx_address = rgb_map->data[r>>3][g>>3][b>>3];
	    rgb_address++;
	    idx_address++;
	  }
	  break;
      }
      break;

    case IMAGE_GRAYSCALE:
      gray_address = image->dat;

      switch (new_image->imgtype) {
	/* Grayscale -> RGB */
	case IMAGE_RGB:
	  rgb_address = new_image->dat;
	  for (i=0; i<size; i++) {
	    c = *gray_address;
	    g = _graya_getk (c);
	    *rgb_address = _rgba (g, g, g, _graya_geta (c));
	    gray_address++;
	    rgb_address++;
	  }
	  break;
	/* Grayscale -> Indexed */
	case IMAGE_INDEXED:
	  idx_address = new_image->dat;
	  for (i=0; i<size; i++) {
	    c = *gray_address;
	    if (_graya_geta (c) == 0)
	      *idx_address = 0;
	    else
	      *idx_address = _graya_getk (c);
	    gray_address++;
	    idx_address++;
	  }
	  break;
      }
      break;

    case IMAGE_INDEXED:
      idx_address = image->dat;

      switch (new_image->imgtype) {
	/* Indexed -> RGB */
	case IMAGE_RGB:
	  rgb_address = new_image->dat;
	  for (i=0; i<size; i++) {
	    c = *idx_address;
	    if (c == 0)
	      *rgb_address = 0;
	    else
	      *rgb_address = _rgba (_rgb_scale_6[palette[c].r],
				    _rgb_scale_6[palette[c].g],
				    _rgb_scale_6[palette[c].b], 255);
	    idx_address++;
	    rgb_address++;
	  }
	  break;
	/* Indexed -> Grayscale */
	case IMAGE_GRAYSCALE:
	  gray_address = new_image->dat;
	  for (i=0; i<size; i++) {
	    c = *idx_address;
	    if (c == 0)
	      *gray_address = 0;
	    else {
	      r = _rgb_scale_6[palette[c].r];
	      g = _rgb_scale_6[palette[c].g];
	      b = _rgb_scale_6[palette[c].b];
	      rgb_to_hsv_int (&r, &g, &b);
	      *gray_address = _graya (b, 255);
	    }
	    idx_address++;
	    gray_address++;
	  }
	  break;
      }
      break;
  }

  return new_image;
}

/* Based on Gary Oberbrunner: */
/*----------------------------------------------------------------------
 * Color image quantizer, from Paul Heckbert's paper in
 * Computer Graphics, vol.16 #3, July 1982 (Siggraph proceedings),
 * pp. 297-304.
 * By Gary Oberbrunner, copyright c. 1988.
 *----------------------------------------------------------------------
 */

/* Bayer-method ordered dither.  The array line[] contains the
 * intensity values for the line being processed.  As you can see, the
 * ordered dither is much simpler than the error dispersion dither.
 * It is also many times faster, but it is not as accurate and
 * produces cross-hatch * patterns on the output.
 */

static int pattern[8][8] = {
  {  0, 32,  8, 40,  2, 34, 10, 42 }, /* 8x8 Bayer ordered dithering  */
  { 48, 16, 56, 24, 50, 18, 58, 26 }, /* pattern.  Each input pixel   */
  { 12, 44,  4, 36, 14, 46,  6, 38 }, /* is scaled to the 0..63 range */
  { 60, 28, 52, 20, 62, 30, 54, 22 }, /* before looking in this table */
  {  3, 35, 11, 43,  1, 33,  9, 41 }, /* to determine the action.     */
  { 51, 19, 59, 27, 49, 17, 57, 25 },
  { 15, 47,  7, 39, 13, 45,  5, 37 },
  { 63, 31, 55, 23, 61, 29, 53, 21 }
};

#define DIST(r1,g1,b1,r2,g2,b2) (3 * ((r1)-(r2)) * ((r1)-(r2)) +	\
				 4 * ((g1)-(g2)) * ((g1)-(g2)) +	\
				 2 * ((b1)-(b2)) * ((b1)-(b2)))

Image *image_rgb_to_indexed (Image *src_image,
			     int offsetx, int offsety,
			     RGB_MAP *rgb_map,
			     RGB *palette)
{
  int oppr, oppg, oppb, oppnrcm;
  Image *dst_image;
  int dither_const;
  int nr, ng, nb;
  int r, g, b, a;
  int nearestcm;
  int c, x, y;

  dst_image = image_new (IMAGE_INDEXED, src_image->w, src_image->h);
  if (!dst_image)
    return NULL;

  for (y=0; y<src_image->h; y++) {
    for (x=0; x<src_image->w; x++) {
      c = src_image->method->getpixel (src_image, x, y);

      r = _rgba_getr (c);
      g = _rgba_getg (c);
      b = _rgba_getb (c);
      a = _rgba_geta (c);

      if (a != 0) {
	nearestcm = rgb_map->data[r>>3][g>>3][b>>3];
	/* rgb values for nearest color */
	nr = _rgb_scale_6[palette[nearestcm].r];
	ng = _rgb_scale_6[palette[nearestcm].g];
	nb = _rgb_scale_6[palette[nearestcm].b];
	/* Color as far from rgb as nrngnb but in the other direction */
	oppr = MID (0, 2*r - nr, 255);
	oppg = MID (0, 2*g - ng, 255);
	oppb = MID (0, 2*b - nb, 255);
	/* Nearest match for opposite color: */
	oppnrcm = rgb_map->data[oppr>>3][oppg>>3][oppb>>3];
	/* If they're not the same, dither between them. */
	/* Dither constant is measured by where the true
	   color lies between the two nearest approximations.
	   Since the most nearly opposite color is not necessarily
	   on the line from the nearest through the true color,
	   some triangulation error can be introduced.  In the worst
	   case the r-nr distance can actually be less than the nr-oppr
	   distance. */
	if (oppnrcm != nearestcm) {
	  oppr = _rgb_scale_6[palette[oppnrcm].r];
	  oppg = _rgb_scale_6[palette[oppnrcm].g];
	  oppb = _rgb_scale_6[palette[oppnrcm].b];

	  dither_const = DIST(nr, ng, nb, oppr, oppg, oppb);
	  if (dither_const != 0) {
	    dither_const = MIN (63, (64 * DIST(r, g, b, nr, ng, nb))
				    / dither_const);

	    if (pattern[(x+offsetx) & 7][(y+offsety) & 7] < dither_const)
	      nearestcm = oppnrcm;
	  }
	}
      }
      else
	nearestcm = 0;

      dst_image->method->putpixel (dst_image, x, y, nearestcm);
    }
  }

  return dst_image;
}
