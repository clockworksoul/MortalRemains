#include "macrosoft.h"

inherit FLOOR_5("std5.c");

void create() {
    ::create();
    set("exits", ([
    "north" : FLOOR_5("e3.c"),
    "south" : FLOOR_5("e5.c"),
    ]) );
    reset();
}

/* EOF */
