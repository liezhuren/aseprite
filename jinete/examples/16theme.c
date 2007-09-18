/* jinete - a GUI library
 * Copyright (C) 2003-2005 by David A. Capello
 *
 * Jinete is gift-ware.
 */

#include <allegro.h>

#include "jinete.h"

void set_my_palette (void);
JTheme my_theme_new (void);

int main (int argc, char *argv[])
{
  JWidget manager, window1, window2, window3;
  JWidget box2, box3, button1, button2a, button2b, button3a, button3b;
  JTheme my_theme;

  /* Allegro stuff */
  allegro_init ();
  if (set_gfx_mode (GFX_AUTODETECT, 320, 200, 0, 0) < 0) {
    allegro_message ("%s\n", allegro_error);
    return 1;
  }
  install_timer ();
  install_keyboard ();
  install_mouse ();

  set_my_palette ();

  /* Jinete initialization */
  manager = jmanager_new ();

  /* change to custom theme */
  my_theme = my_theme_new ();
  ji_set_theme (my_theme);

  window1 = jwindow_new ("Window1");
  window2 = jwindow_new ("Window2");
  box2 = jbox_new (JI_HORIZONTAL | JI_HOMOGENEOUS);
  button1 = jbutton_new ("Button1");
  button2a = jbutton_new ("Button2a");
  button2b = jbutton_new ("Button2b");

  /* change to default theme */
  ji_set_standard_theme ();

  window3 = jwindow_new ("Window3");
  box3 = jbox_new (JI_HORIZONTAL | JI_HOMOGENEOUS);
  button3a = jbutton_new ("Button3a");
  button3b = jbutton_new ("Button3b");

  /* setup widgets */
  jwidget_disable (button2b);
  jwidget_disable (button3b);

  jwidget_add_child (window1, button1);
  jwidget_add_child (window2, box2);
  jwidget_add_child (box2, button2a);
  jwidget_add_child (box2, button2b);
  jwidget_add_child (window3, box3);
  jwidget_add_child (box3, button3a);
  jwidget_add_child (box3, button3b);

  jwindow_open (window1);
  jwindow_open (window2);
  jwindow_open (window3);

  jmanager_run (manager);

  /* back to the custom theme */
  ji_set_theme (my_theme);

  jalert ("Warning<<This is a test alert||&Yes||&No||&Cancel");

#if 0
  /* back to the custom theme */
  ji_set_theme (my_theme);

  Alert alert ("Warning");
  alert.add_label ("This is a test alert");
  alert.add_button ("&Yes");
  alert.add_button ("&No");
  alert.add_button ("&Cancel");
  alert.run ();
#endif

  jtheme_free (my_theme);
  jmanager_free (manager);
  return 0;
}

END_OF_MAIN ();

void set_my_palette (void)
{
  PALETTE pal;
  int i, c = 0;

  for (i=0; i<32; i++, c++) pal[c].r = pal[c].g = pal[c].b = 63 * i / 31;
  for (i=0; i<32; i++, c++) pal[c].r = 63 * i / 31, pal[c].g = pal[c].b = 0;
  for (i=0; i<32; i++, c++) pal[c].r = 0, pal[c].g = 63 * i / 31, pal[c].b = 0;
  for (i=0; i<32; i++, c++) pal[c].r = pal[c].g = 0, pal[c].b = 63 * i / 31;
  for (i=0; i<32; i++, c++) pal[c].r = pal[c].g = 63 * i / 31, pal[c].b = 0;

  for (i=0; i<32; i++, c++) {
    pal[c].r = 0;
    pal[c].b = 63 * i / 31;
    pal[c].g = i;
  }

  for (; c<256; c++)
    pal[c].r = pal[c].b = pal[c].g = 0;

  set_palette (pal);
  rgb_map = malloc (sizeof (RGB_MAP));
  create_rgb_table (rgb_map, pal, NULL);
}

/**********************************************************************/
/* Theme */

static void theme_regen (void);
static void theme_set_cursor (int type);
static void theme_init_widget (JWidget widget);
static JRegion theme_get_window_mask (JWidget widget);

static void theme_draw_box (JWidget widget);
static void theme_draw_button (JWidget widget);
static void theme_draw_label (JWidget widget);
static void theme_draw_window (JWidget widget);

JTheme my_theme_new (void)
{
  JTheme theme;

  theme = jtheme_new ();
  if (!theme)
    return NULL;

  theme->name = "My Theme";
  theme->check_icon_size = 8;
  theme->radio_icon_size = 8;

  theme->regen = theme_regen;
  theme->set_cursor = theme_set_cursor;
  theme->init_widget = theme_init_widget;
  theme->get_window_mask = theme_get_window_mask;

  jtheme_set_method (theme, JI_BOX, theme_draw_box);
  jtheme_set_method (theme, JI_BUTTON, theme_draw_button);
  jtheme_set_method (theme, JI_LABEL, theme_draw_label);
  jtheme_set_method (theme, JI_WINDOW, theme_draw_window);

  return theme;
}

static void theme_regen (void)
{
  ji_get_theme ()->desktop_color = makecol (64, 100, 128);
}

static void theme_set_cursor (int type)
{
  set_mouse_sprite (NULL);	/* use default Allegro cursor */
}

static void theme_init_widget (JWidget widget)
{
#define BORDER(n)				\
  widget->border_width.l = n;			\
  widget->border_width.t = n;			\
  widget->border_width.r = n;			\
  widget->border_width.b = n;

#define BORDER4(L,T,R,B)			\
  widget->border_width.l = L;			\
  widget->border_width.t = T;			\
  widget->border_width.r = R;			\
  widget->border_width.b = B;

  switch (widget->draw_type) {

    case JI_BOX:
      BORDER (8);
      widget->child_spacing = 8;
      break;

    case JI_BUTTON:
      BORDER (6);
      widget->child_spacing = 0;
      break;

    case JI_WINDOW:
      BORDER4 (8, 12+jwidget_get_text_height (widget)+12, 8, 8);
      widget->child_spacing = 1;
      break;
  }
}

static JRegion theme_get_window_mask (JWidget widget)
{
  JRegion reg1 = jregion_new (NULL, 0);
  JRegion reg2 = jregion_new (NULL, 0);
  int table[6] = { 6, 4, 3, 2, 1, 1 };
  JRect rect = jrect_new (0, 0, 0, 0);
  JRect pos = jwidget_get_rect (widget);
  bool overlap;
  int c;

  for (c=0; c<6; c++) {
    jrect_replace (rect,
		     pos->x1+table[c], pos->y1+c,
		     pos->x2-table[c], pos->y1+c+1);
    jregion_reset (reg2, rect);
    jregion_append (reg1, reg2);

    jrect_replace (rect,
		     pos->x1+table[c], pos->y2-c-1,
		     pos->x2-table[c], pos->y2-c);
    jregion_reset (reg2, rect);
    jregion_append (reg1, reg2);
  }

  jrect_replace (rect, pos->x1, pos->y1+c, pos->x2, pos->y2-c);
  jregion_reset (reg2, rect);
  jregion_append (reg1, reg2);

  jrect_free (pos);
  jrect_free (rect);
  jregion_free (reg2);

  jregion_validate (reg1, &overlap);
  return reg1;
}

static void theme_draw_box (JWidget widget)
{
  JWidget window = jwidget_get_window (widget);
  if (window) {
    JRect pos = jwidget_get_child_rect (window);
    jdraw_rectshade (pos, makecol (128, 128, 128), makecol (196, 196, 196),
		       JI_VERTICAL);
    jrect_free (pos);
  }
}

static void theme_draw_button (JWidget widget)
{
  JRect pos = jwidget_get_rect (widget);
  int c1, c2, fg, bg;

  if (jwidget_is_disabled (widget)) {
    if (jwidget_is_selected (widget)) {
      bg = makecol (64, 64, 64);
      fg = makecol (128, 128, 128);
      c1 = makecol (0, 0, 0);
      c2 = makecol (255, 255, 255);
    }
    else {
      bg = makecol (128, 128, 128);
      fg = makecol (64, 64, 64);
      c1 = makecol (255, 255, 255);
      c2 = makecol (0, 0, 0);
    }
  }
  else {
    if (jwidget_is_selected (widget)) {
      bg = makecol (255, 255, 255);
      fg = makecol (128, 128, 128);
      c1 = makecol (0, 0, 0);
      c2 = makecol (255, 255, 255);
    }
    else {
      bg = makecol (128, 128, 128);
      fg = makecol (255, 255, 255);
      c1 = makecol (255, 255, 255);
      c2 = makecol (0, 0, 0);
    }
  }

  jdraw_rectedge (pos, c1, c2);

  jrect_shrink (pos, 1);
  jdraw_rectedge (pos, makecol (128, 128, 128), makecol (64, 64, 64));

  jrect_shrink (pos, 1);
  jdraw_rectshade (pos, fg, bg, JI_VERTICAL);

  if (jwidget_get_text (widget)) {
    struct jrect text;
    int s = jwidget_is_selected (widget) ? 1: 0;

    jwidget_get_texticon_info (widget, NULL, &text, NULL, 0, 0, 0);

    jdraw_text (widget->text_font, widget->text,
		  text.x1+s+1, text.y1+s+1, makecol (0, 0, 0), bg, FALSE);
    jdraw_text (widget->text_font, widget->text,
		  text.x1+s, text.y1+s, fg, bg, FALSE);
  }

  jrect_free (pos);
}

static void theme_draw_label (JWidget widget)
{
  /* draw background as Box */
  theme_draw_box (widget);

  jdraw_widget_text (widget, makecol (0, 0, 0), makecol (196, 196, 196), FALSE);
}

static void theme_draw_window (JWidget widget)
{
  JRect pos;

  pos = jwidget_get_rect (widget);
  jdraw_rectfill (pos, makecol (196, 196, 0));
  jrect_shrink (pos, 8);

  /* title bar */
  pos->y2 = pos->y1+4+jwidget_get_text_height (widget)+4;
  jdraw_rectshade (pos, makecol (0, 0, 0), makecol (128, 128, 128), JI_VERTICAL);

  pos->x1 += 1;
  jdraw_text (widget->text_font, widget->text, pos->x1, pos->y1+4,
		makecol (255, 255, 255), makecol (196, 196, 0), FALSE);

  jrect_free (pos);
}
