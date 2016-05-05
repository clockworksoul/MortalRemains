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
	"southwest" : "/u/c/chakan/edge/streets/fountain.c",
	"north" : "/u/c/chakan/edge/res/?",
	"south" : "/u/c/chakan/edge/streets/r28.c",
	"east" : "/u/c/chakan/edge/res/?",
	"west" : "/u/c/chakan/edge/streets/r30.c",
  ]) );
}
