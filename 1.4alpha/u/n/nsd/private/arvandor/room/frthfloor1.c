/*
** File: frthfloor1.c
** Author: Lady Nsd
** Other comments: To secure peace is to prepare for war!!!!
*/
#include <config.h>
#include <mudlib.h>
#include "arv.h"

inherit ROOM;

void create() {
    ::create();
    seteuid( getuid() );
    set( "light", 1 );
    set("author", "nsd");
    set("short", "Inside the tower");
    set( "long", @EndText
You are in the hallway of the fourth floor. This is where the
passage starts. It continues to east and west. To the south
is the door that leads you back to the stairway room.
EndText
    );
    set( "exits", ([
      "south" : ARV_ROOM+"seelietower4.c",
      "west" : ARV_ROOM+"frthfloor2.c",
      "east" : ARV_ROOM+"frthfloor8.c",
    ]) );
    set("item_desc", ([
    ]) );
}

/* EOF */
