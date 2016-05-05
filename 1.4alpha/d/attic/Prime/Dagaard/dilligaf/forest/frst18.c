#include <config.h>
#include <mudlib.h>
#include "rooms.h"

inherit ROOM;

void create()
{
  ::create();
  seteuid( 0 );
  set( "light", 1 );
  set( "short" , "A forest path");
  set( "long" , @EndText
  You are walking along a forest path. Nothing really amusing here,
  just a bunch of trees and a few bushes line the path. 
    It continues either east and west.

EndText
  );
  set( "exits", ([
    "west" : FOREST+"frst17.c",
    "east" : FOREST+"frst34.c",
    "south" : FOREST+"frst25.c",
    "southwest" : FOREST+"frst24.c",
    "northwest" : FOREST+"frst11.c",
  ]) );
  set("item_desc" , ([
    "trees" : "Large trees.\n",
    "bushes" : "Bushes.\n",
    "path" : "You wonder if you should stay on it.\n",
  ]));
}
