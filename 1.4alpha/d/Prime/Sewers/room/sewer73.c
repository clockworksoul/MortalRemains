#include <mudlib.h>
#include <sewers.h>

inherit M_ROOM+"sewerstd.c";

void create() {
  ::create();
  set("exits", ([
     "north" : M_ROOM+"sewer63.c",
     "east"  : M_ROOM+"sewer74.c",
     "west"  : M_ROOM+"sewer72.c",
    ]) );
}
