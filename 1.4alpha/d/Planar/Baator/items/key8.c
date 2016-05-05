/*
** FILE: obsword.c
** PURPOSE: Example of set_bonus() function.
** CREATOR: Cyanide@Nightfall's Keep
** CHANGES:
**    9 Apr 97    Cyanide    Created the file.
**   18 Jun 97    Cyanide    Updated the weapon to include the new
**                           set_weapon_type function.
*/

#include <mudlib.h>

inherit OBJECT;

void create() {
	set("mass", 1);
	set("id", ({ "key", "8key8" }) );
	set("short", "key of the eighth pit");
	set("long", @LONG
An intricate key.
It freezes your hands as you hold it and it's shape reminds you of
ice floes and river serpents.
LONG
	);
 set("value", 80000); 
   }


