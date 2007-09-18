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

#include "core/core.h"
#include "core/modules.h"
#include "effect/effect.h"
#include "modules/chkmthds.h"
#include "modules/color.h"
#include "modules/editors.h"
#include "modules/gfx.h"
#include "modules/gui.h"
#include "modules/palette.h"
#include "modules/recent.h"
#include "modules/render.h"
#include "modules/rootmenu.h"
#include "modules/sprites.h"
#include "modules/tools.h"
#include "script/script.h"

#endif

#define DEF_MODULE(name, reqs) \
  { #name, init_module_##name, exit_module_##name, (reqs), FALSE }

typedef struct Module
{
  const char *name;
  int (*init) (void);
  void (*exit) (void);
  int reqs;
  int installed;
} Module;

static Module module[] =
{
  /* This sorting is very important because last modules depend of
     first ones.  */

  DEF_MODULE(palette,		REQUIRE_INTERFACE | REQUIRE_SCRIPTING),
  DEF_MODULE(color,		REQUIRE_INTERFACE | REQUIRE_SCRIPTING),
  DEF_MODULE(sprites,		REQUIRE_INTERFACE | REQUIRE_SCRIPTING),
  DEF_MODULE(script,		REQUIRE_INTERFACE | REQUIRE_SCRIPTING),
  DEF_MODULE(effect,		REQUIRE_INTERFACE | REQUIRE_SCRIPTING),
  DEF_MODULE(tools,		REQUIRE_INTERFACE | REQUIRE_SCRIPTING),
  DEF_MODULE(graphics,		REQUIRE_INTERFACE),
  DEF_MODULE(render,		REQUIRE_INTERFACE),
  DEF_MODULE(gui,		REQUIRE_INTERFACE),
  DEF_MODULE(recent,		REQUIRE_INTERFACE),
  DEF_MODULE(check_methods,	REQUIRE_INTERFACE),
  DEF_MODULE(rootmenu,		REQUIRE_INTERFACE),
  DEF_MODULE(editors,		REQUIRE_INTERFACE),
};

static int modules = sizeof (module) / sizeof (Module);

int modules_init (int requirements)
{
  int c;

  for (c=0; c<modules; c++)
    if (module[c].reqs & requirements) {
      PRINTF ("Installing module: %s\n", module[c].name);
      if ((*module[c].init) () < 0)
	return -1;
      module[c].installed = TRUE;
    }

  return 0;
}

void modules_exit (void)
{
  int c;

  for (c=modules-1; c>=0; c--)
    if (module[c].installed) {
      PRINTF ("Unstalling module: %s\n", module[c].name);
      (*module[c].exit) ();
      module[c].installed = FALSE;
    }
}
