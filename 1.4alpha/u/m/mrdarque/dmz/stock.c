#include <config.h>
#include <mudlib.h>
#include "/u/m/mrdarque/mrd.h",

inherit ROOM;

void create()
{
    ::create();
    seteuid( getuid() );
    set( "light", 1 );
set( "short", "ROOM NAME HERE" );
    // NB: "@EndText" should not have spaces after it.
    // "EndText" should be on a line of its own (no indentation, tabs or spaces)
    set( "long", @EndText

EndText
    );
    set( "exits", ([
"east" : "rchain.c",
"up" : "loft1.c",
    ]) );
}
