#include <mudlib.h>
#include <sewers.h>

inherit M_ROOM+"sewerstd.c";

void create() {
  ::create();
  set("exits", ([
     "north" : M_ROOM+"sewer42.c",
     "south" : M_ROOM+"sewer62.c",
     "east"  : M_ROOM+"sewer53.c",
     "west"  : M_ROOM+"sewer51.c",
    ]) );
}
