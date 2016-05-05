/*
// This file contains the code for a generic workroom.
// It is used by _home.c if your own workroom won't load.
// It is used by _sponsor.c to provide new wizards
//   with a basic home that they can edit to their own taste.
*/

// config.h only included here for the defines of START and VOID
// it doesn't need to be included in most rooms.
#include <config.h>
#include <mudlib.h>
#include "generic/GenericLib.h"

inherit ROOM;

void create()
{
    object obj;
    seteuid( getuid() );
    obj = find_object_or_load("/u/g/grull/generic_room");
    if (obj) obj->foo();
    ::create();
    set( "light", 1 );
    set( "short", "A workroom" );
    // NB: "@EndText" should not have spaces after it.
    // "EndText" should be on a line of its own (no indentation, tabs or spaces)
    set( "long", @EndText
This is a temporary workroom for those wizards
who do not have one of their own that will load.
EndText
    );
    set( "exits", ([
      "start" : START,
      "void"  : VOID,
      "monsters" : "/u/g/grull/generic_room.c",
    ]) );

}
