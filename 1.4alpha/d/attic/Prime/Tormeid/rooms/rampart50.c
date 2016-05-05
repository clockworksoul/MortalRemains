// Coded by Chronos.
#include <mudlib.h>
#include "../defs.h"

inherit ROOM;

void create() {
  seteuid(getuid());
  set("short", "On the ramparts.");
  set("long", "You stand on the ramparts of the castle wall.
");
  set("light", 1) ;
  set_outside("Light");
  set("exits", ([
    "east" : ROOMS(rampart49.c),
    "west" : ROOMS(rampart51.c),
  ]) ) ;
  ::create();
}
