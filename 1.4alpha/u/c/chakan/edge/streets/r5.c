#include <mudlib.h>

inherit ROOM;

void create()
{
  ::create();
   // Inspiral fears Root euid.  94-08-18
  seteuid( 0 );
  set( "light", 1 );
set( "short" , "basic street" );
  set( "long", @EndText
Generic street.
EndText
  );
  set( "exits", ([
	"north" : "/u/c/chakan/edge/park/r1.c",
	"east" : "/u/c/chakan/edge/streets/r6.c",
	"west" : "/u/c/chakan/edge/streets/r4.c",
  ]) );
}
