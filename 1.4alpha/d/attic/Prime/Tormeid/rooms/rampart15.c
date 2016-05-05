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
    "west" : ROOMS(rampart14.c),
    "east" : ROOMS(rampart16.c),
  ]) ) ;
  ::create();
}
