// Coded by Chronos.
#include <mudlib.h>
#include "../defs.h"

inherit ROOM;

void create() {
  seteuid(getuid());
  set("short", "On the ramparts.");
  set("long", "You stand on the ramparts on the castle wall.
");
  set("light", 1) ;
  set_outside("Light");
  set("exits", ([
    "north" : ROOMS(rampart37.c),
    "south" : ROOMS(rampart39.c),
  ]) ) ;
  ::create();
}
