#include <mudlib.h>

inherit MONSTER ;

#include "../food.h"

void create () {
    ::create();
    seteuid(getuid());
    enable_commands() ;
    set("author", "nightmask");
    set("prevent_summon", 1);
    set("short", "cheesecake villager");
    set ("long", @EndText
This is a cheesecake villager, not much to look at but boy do they
look yummy, and well, mabye a little bit deadly.
EndText
    );
    set("id", ({ "cheesecake", "villager" }) );
    set_size(4);
    set_name("cheesecake");
    set("wealth", random(60));
    set("gender", "neuter");
    set("race", "dessert");
    set("damage", ({ 1,4 }) );
    set("weapon_name", "creamy fists");
    set_verbs( ({ "attack", "claw at", "swing at" }) ) ;
    set("natt", 1);
    set ("chat_chance", 10);
    set ("chat_output", ({
    }) );
    set ("att_chat_output", ({
    }) );
    set_level(random(3));
    arm(WEAP+"cake.c");
    wear(ARM+"cocohelm.c");
}
