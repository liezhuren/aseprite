/* jinete - a GUI library
 * Copyright (C) 2003-2005 by David A. Capello
 *
 * Jinete is gift-ware.
 */

#include <allegro/keyboard.h>

#include "jinete/list.h"
#include "jinete/manager.h"
#include "jinete/message.h"
#include "jinete/rect.h"
#include "jinete/system.h"
#include "jinete/theme.h"
#include "jinete/view.h"
#include "jinete/widget.h"

static bool listbox_msg_proc(JWidget widget, JMessage msg);
static void listbox_request_size(JWidget widget, int *w, int *h);
static void listbox_set_position(JWidget widget, JRect rect);
static void listbox_dirty_children(JWidget widget);

static bool listitem_msg_proc(JWidget widget, JMessage msg);
static void listitem_request_size(JWidget widget, int *w, int *h);

JWidget jlistbox_new(void)
{
  JWidget widget = jwidget_new (JI_LISTBOX);

  jwidget_add_hook (widget, JI_LISTBOX, listbox_msg_proc, NULL);
  jwidget_focusrest (widget, TRUE);
  jwidget_init_theme (widget);

  return widget;
}

JWidget jlistitem_new(const char *text)
{
  JWidget widget = jwidget_new (JI_LISTITEM);

  jwidget_add_hook (widget, JI_LISTITEM, listitem_msg_proc, NULL);
  jwidget_set_align (widget, JI_LEFT | JI_MIDDLE);
  jwidget_set_text (widget, text);
  jwidget_init_theme (widget);

  return widget;
}

JWidget jlistbox_get_selected_child(JWidget widget)
{
  JLink link;
  JI_LIST_FOR_EACH(widget->children, link) {
    if (jwidget_is_selected((JWidget)link->data))
      return (JWidget)link->data;
  }
  return 0;
}

int jlistbox_get_selected_index(JWidget widget)
{
  JLink link;
  int i = 0;

  JI_LIST_FOR_EACH(widget->children, link) {
    if (jwidget_is_selected((JWidget)link->data))
      return i;
    i++;
  }

  return -1;
}

void jlistbox_select_child(JWidget widget, JWidget listitem)
{
  JWidget child;
  JLink link;

  JI_LIST_FOR_EACH(widget->children, link) {
    child = (JWidget)link->data;

    if (jwidget_is_selected(child)) {
      if ((listitem) && (child == listitem))
	return;

      jwidget_deselect(child);
    }
  }

  if (listitem) {
    JWidget view = jwidget_get_view(widget);

    jwidget_select(listitem);

    if (view) {
      JRect vp = jview_get_viewport_position(view);
      int scroll_x, scroll_y;

      jview_get_scroll(view, &scroll_x, &scroll_y);

      if (listitem->rc->y1 < vp->y1)
	jview_set_scroll (view, scroll_x, listitem->rc->y1 - widget->rc->y1);
      else if (listitem->rc->y1 > vp->y2 - jrect_h(listitem->rc))
	jview_set_scroll (view, scroll_x,
			    listitem->rc->y1 - widget->rc->y1
			    - jrect_h(vp) + jrect_h(listitem->rc));

      jrect_free(vp);
    }
  }

  jwidget_emit_signal(widget, JI_SIGNAL_LISTBOX_CHANGE);
}

void jlistbox_select_index(JWidget widget, int index)
{
  JWidget child = jlist_nth_data(widget->children, index);
  if (child)
    jlistbox_select_child(widget, child);
}

/* setup the scroll to center the selected item in the viewport */
void jlistbox_center_scroll(JWidget widget)
{
  JWidget view = jwidget_get_view(widget);
  JWidget listitem = jlistbox_get_selected_child(widget);

  if (view && listitem) {
    JRect vp = jview_get_viewport_position(view);
    int scroll_x, scroll_y;

    jview_get_scroll (view, &scroll_x, &scroll_y);
    jview_set_scroll (view,
			scroll_x,
			(listitem->rc->y1 - widget->rc->y1)
			- jrect_h(vp)/2 + jrect_h(listitem->rc)/2);
    jrect_free (vp);
  }
}

static bool listbox_msg_proc(JWidget widget, JMessage msg)
{
  switch (msg->type) {

    case JM_REQSIZE:
      listbox_request_size (widget, &msg->reqsize.w, &msg->reqsize.h);
      return TRUE;

    case JM_SETPOS:
      listbox_set_position (widget, &msg->setpos.rect);
      return TRUE;

    case JM_DIRTYCHILDREN:
      listbox_dirty_children (widget);
      return TRUE;

    case JM_OPEN:
      jlistbox_center_scroll (widget);
      break;

    case JM_BUTTONPRESSED:
      jwidget_capture_mouse (widget);

    case JM_MOTION:
      if (jwidget_has_capture (widget)) {
	int select = jlistbox_get_selected_index (widget);
	JWidget view = jwidget_get_view (widget);
	bool pick_item = TRUE;

	if (view) {
	  JRect vp = jview_get_viewport_position (view);

	  if (msg->mouse.y < vp->y1) {
	    int num = MAX (1, (vp->y1 - msg->mouse.y) / 8);
	    jlistbox_select_index (widget, select-num);
	    pick_item = FALSE;
	  }
	  else if (msg->mouse.y >= vp->y2) {
	    int num = MAX (1, (msg->mouse.y - (vp->y2-1)) / 8);
	    jlistbox_select_index (widget, select+num);
	    pick_item = FALSE;
	  }

	  jrect_free (vp);
	}

	if (pick_item) {
	  JWidget picked;

	  if (view) {
	    picked = jwidget_pick (jview_get_viewport (view),
				     msg->mouse.x, msg->mouse.y);
	  }
	  else {
	    picked = jwidget_pick (widget, msg->mouse.x, msg->mouse.y);
	  }

	  /* if the picked widget is a child of the list, select it */
	  if (picked && jwidget_has_child (widget, picked))
	    jlistbox_select_child (widget, picked);
	}

	return TRUE;
      }
      break;

    case JM_BUTTONRELEASED:
      jwidget_release_mouse (widget);
      break;

    case JM_WHEEL: {
      JWidget view = jwidget_get_view (widget);
      if (view) {
	int scroll_x, scroll_y;

	jview_get_scroll(view, &scroll_x, &scroll_y);
	jview_set_scroll(view,
			   scroll_x,
			   scroll_y +
			   (ji_mouse_z (1) - ji_mouse_z (0))
			   *jwidget_get_text_height(widget)*3);
      }
      break;
    }

    case JM_CHAR:
      if (jwidget_has_focus(widget) && !jlist_empty(widget->children)) {
	int select = jlistbox_get_selected_index(widget);
	JWidget view = jwidget_get_view(widget);
	int bottom = MAX(0, jlist_length(widget->children)-1);

	switch (msg->key.scancode) {
	  case KEY_UP:
	    select--;
	    break;
	  case KEY_DOWN:
	    select++;
	    break;
	  case KEY_HOME:
	    select = 0;
	    break;
	  case KEY_END:
	    select = bottom;
	    break;
	  case KEY_PGUP:
	    if (view) {
	      JRect vp = jview_get_viewport_position(view);
	      select -= jrect_h(vp) / jwidget_get_text_height(widget);
	      jrect_free (vp);
	    }
	    else
	      select = 0;
	    break;
	  case KEY_PGDN:
	    if (view) {
	      JRect vp = jview_get_viewport_position(view);
	      select += jrect_h(vp) / jwidget_get_text_height(widget);
	      jrect_free(vp);
	    }
	    else
	      select = bottom;
	    break;
	  case KEY_LEFT:
	  case KEY_RIGHT:
	    if (view) {
	      JRect vp = jview_get_viewport_position(view);
	      int sgn = (msg->key.scancode == KEY_LEFT) ? -1: 1;
	      int scroll_x, scroll_y;

	      jview_get_scroll(view, &scroll_x, &scroll_y);
	      jview_set_scroll(view, scroll_x + jrect_w(vp)/2*sgn, scroll_y);
	      jrect_free(vp);
	    }
	    break;
	  default:
	    return FALSE;
	}

	jlistbox_select_index(widget, MID(0, select, bottom));
	return TRUE;
      }
      break;

    case JM_DOUBLECLICK:
      jwidget_emit_signal(widget, JI_SIGNAL_LISTBOX_SELECT);
      return TRUE;
  }

  return FALSE;
}

static void listbox_request_size(JWidget widget, int *w, int *h)
{
  int req_w, req_h;
  JLink link;

  *w = *h = 0;

  JI_LIST_FOR_EACH(widget->children, link) {
    jwidget_request_size(link->data, &req_w, &req_h);

    *w = MAX(*w, req_w);
    *h += req_h + ((link->next)? widget->child_spacing: 0);
  }

  *w += widget->border_width.l + widget->border_width.r;
  *h += widget->border_width.t + widget->border_width.b;
}

static void listbox_set_position(JWidget widget, JRect rect)
{
  int req_w, req_h;
  JWidget child;
  JRect cpos;
  JLink link;

  jrect_copy(widget->rc, rect);
  cpos = jwidget_get_child_rect(widget);

  JI_LIST_FOR_EACH(widget->children, link) {
    child = (JWidget)link->data;

    jwidget_request_size(child, &req_w, &req_h);

    cpos->y2 = cpos->y1+req_h;
    jwidget_set_rect(child, cpos);

    cpos->y1 += jrect_h(child->rc) + widget->child_spacing;
  }

  jrect_free(cpos);
}

static void listbox_dirty_children(JWidget widget)
{
  JWidget view = jwidget_get_view(widget);
  JWidget child;
  JLink link;
  JRect vp;

  if (!view) {
    JI_LIST_FOR_EACH(widget->children, link)
      jwidget_dirty(link->data);
  }
  else {
    vp = jview_get_viewport_position(view);

    JI_LIST_FOR_EACH(widget->children, link) {
      child = (JWidget)link->data;

      if (child->rc->y2 <= vp->y1)
	continue;
      else if (child->rc->y1 >= vp->y2)
	break;

      jwidget_dirty(child);
    }

    jrect_free(vp);
  }
}

static bool listitem_msg_proc(JWidget widget, JMessage msg)
{
  switch (msg->type) {

    case JM_REQSIZE:
      listitem_request_size(widget, &msg->reqsize.w, &msg->reqsize.h);
      return TRUE;

    case JM_SETPOS: {
      JRect crect;
      JLink link;

      jrect_copy(widget->rc, &msg->setpos.rect);
      crect = jwidget_get_child_rect(widget);

      JI_LIST_FOR_EACH(widget->children, link)
	jwidget_set_rect(link->data, crect);

      jrect_free(crect);
      return TRUE;
    }
  }

  return FALSE;
}

static void listitem_request_size(JWidget widget, int *w, int *h)
{
  int max_w, max_h;
  int req_w, req_h;
  JLink link;

  if (widget->text) {
    max_w = jwidget_get_text_length(widget);
    max_h = jwidget_get_text_height(widget);
  }
  else
    max_w = max_h = 0;

  JI_LIST_FOR_EACH(widget->children, link) {
    jwidget_request_size(link->data, &req_w, &req_h);

    max_w = MAX (max_w, req_w);
    max_h = MAX (max_h, req_h);
  }

  *w = widget->border_width.l + max_w + widget->border_width.r;
  *h = widget->border_width.t + max_h + widget->border_width.b;
}
