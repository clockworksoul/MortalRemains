#include <mudlib.h>
#include "/d/Dark/High/high.h"

inherit ROOM;

void create()
{
  seteuid(geteuid());
  set("author","highlander");
  set( "light", 1 );
  set_outside();
  set("short","A boring room");
  set( "long", @EndText
Totally destroyed house.
EndText
  );
  set( "exits", ([
     "north":HIGHRM"/ze2.c",
    ]) );
     set("item_desc", ([
       ]) );
  set("search_desc", ([
	]) );
  ::create();
}
