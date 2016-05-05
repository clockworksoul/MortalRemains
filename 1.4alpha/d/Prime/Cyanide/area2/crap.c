#include "main.h"
#include <config.h>
#include <mudlib.h>

inherit ROOM;

void create()
{
  ::create();//<- gets all the information from the original create()
  seteuid( getuid());
  set( "light", 1 );
set("area_type", ([
"black" : 2,
]) );
set("short", "south road");
  set( "long", @EndText
Here, on the southern road of the City of Urborg, the massive
destruction which befell the once grand metropolis has forced
the very ground itself downward. The road takes a sharp
decline here, reaching an angle of about thirty or so degrees.
Directly to your southeast, a pool of water lies stagnating,
its black water reeking of death.
EndText
  );
set("arena", 1);
  set( "exits", ([
"north" : MAIN+"/room6'5.c",
"southeast" : MAIN+"/room8'3.c",
  ]) );
}

void init () {
  add_action ("southeast", "wetcheck");
 }

int wetcheck () {
  object wet;
  wet=present("wet_obj", this_player());
  if (!wet) {return 0;}
    return 0;
}
