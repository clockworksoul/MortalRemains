
#include <mudlib.h>
#include "../defs.h"

inherit ROOM;   

void create() {
  seteuid(getuid());   

  set("author", "Waxer");  

  set("light", 0);


  set("short", "In a narrow tunnel.");
  
set("long", @ENDLONG
You are in a narrow cave.  The walls are colored slate grey, 
the color of natural granite and are completely smooth and barren.
ENDLONG
   );

  


  set("smell", ([
    "default" : "You smell a sickly sweet smell in the air.\n",
    
    "air"  : "It reminds you of something you can't quite place, a sweet, decaying odor.\n",
    
  ]) );
  set("listen", ([
   "default" : "You hear a light clicking sounds.\n",
    
    
    
  ]) );

  

  set("exits", ([
    
    "north" : ROOMS(l5),
    "south" :  ROOMS(l3),
    
    
  ]) );
set("objects", ([
    MON(warant) : 2,
  ]) );
   set("pre_exit_func", ([
     "north" : "guarded",
  ]) );
  ::create();
}

int guarded() {


  if (present("ant", TO)) {
    write("The warrior ant refuses to let you pass.\n");
   return 1;
  }
  return 0;
}