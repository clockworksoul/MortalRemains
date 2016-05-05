// Coded by Chronos.
#include <mudlib.h>
#include "../defs.h"

inherit ROOM ;

void create() {
  seteuid(getuid()) ;
  set("author", "chronos");
  set("light", 1) ;
  set("short", "The Bachelor Enlisted Quarters, Third floor.");
  set("long", @ENDLONG
You stand in a long hallway lined with doors, mostly made of concrete,
with a shabby carpet lining the floor.
The stairwell is to the north.
ENDLONG
  );
  set("item_desc", ([
  ]) ) ;
  set("exits", ([
    "north" : ROOMS(beq_st3),
    "south" : ROOMS(3beqrm1),
    "east" : ROOMS(3beq2),
  ]) ) ;
  ::create();
  reset();
}
