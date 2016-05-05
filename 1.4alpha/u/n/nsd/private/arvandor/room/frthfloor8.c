/*
** File: thrdfloor8.c
** Author: Lady Nsd
** Other comments: To secure peace is to prepare for war!!!
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
You are in the northeast corner of the fourth floor. The passage
continues to west and southeast. The walls are a little dusted.
EndText);
    set( "exits", ([
      "southeast" : ARV_ROOM+"frthfloor7.c",
      "west" : ARV_ROOM+"frthfloor1.c",
    ]) );
    set("item_desc", ([
    ]) );
}

/* EOF */
