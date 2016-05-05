
#include <mudlib.h>
#include <where.h>
inherit MONSTER ;

//this declares that an object will be cloned with the monster
object axe;
//more than 1 item would look somthing like 
//object ob1, ob2, ob3;      ///etc...

void create () {
    ::create();
    seteuid(getuid());
    //short is what you see in an inventory, id is used to determine what
    //it is exactally you want to look at
    set("short", "a young orc");
    set ("long", @Foo
This orc seems too young to be in such an establishment but
after a moment you realize this isn't such an inexperienced
youth.
Foo
    );
    //long is what is seen after an exa is used. the @Foo statement
    //makes the code a bit cleaner
    set("id", ({ "orc" }) );
    //set_level is one of Cyanide's brainstorms. what it does is 
    //basically everything, you need this line if you plan to make a 
    //working monster, it does hp, exp, stats, and such
    set_level(5);
    //size is just what it sounds like, the size of the monster
    set_size(5);
    //this line makes the file say who is hitting who
    set_name("orc") ;
    /* set_living_name() is not needed since set_name() calls it for you. :) */
    //gender, female, male, neuter
    set("gender", "male");
    //race can be whatever you like
    set("race", "orc");
    //this calls the other object and moves it to the monsters inventory
    /*** There is a better, easier way to do this now.  See the end of create().
       axe = clone_object("/doc/examples/axe.c");
       axe->move(this_object());
    ***/
    //damage for the monster when it is barehanded
    set("damage", ({ 1,4 }) );
    //this is so the monster dosen't hit you with 0
    set("weapon_name", "claws") ;
    //verbs are what you see, verbs2 is what others see
    set_verbs( ({ "attack", "claw at", "swing at" }) ) ;
    // you only need set_verbs2() if your verbs will be grammatically incorrect
    // when you add an "s" to the end of the first word.  Normally, without
    // set_verbs2() being set, the verbs from set_verbs() just have an "s"
    // appended to the first word to make them grammatically correct.  
    // Since english is fucked up, not all words follow this rule, and that is
    // where set_verbs2() comes in.  Example
    // er.. Example:  You set verbs to be "punch at", "swing at", "attack"
    // The monster would see "punch at", "swing at", and "attack" while the
    // victim and onlookers would see "punchs at", "swings at", and "attacks"
    // Guess which is fucked up?  Punchs at.  Wrong-o.  It's one of those fun
    // words that needs the "es".  Well set it that way in set_verbs2()!
    // "punches at", "swings at", and "attacks" would go in your set_verbs2()
    // in this example.  In this file, however... the grammar works out
    // just fine without a set_verbs2().. so we won't use it. :)

    //this is for the wielding and equiping of shit.
    enable_commands() ;
    //this gives a monster 1 attack per round barehanded
    set("natt", 1);
    //this sets the chance that a monster actually says somthing
    //  (note: 10 is really chatty.  very annoying.)
    set ("chat_chance", 5);
    //this is what the monster says, or does outside of a combat situation
    set ("chat_output", ({
      "The orc asks you: What's your problem?\n",
    }) );
    //this is what a monster says or does in a combat situation
    set ("att_chat_output", ({
      "The orc swears at you.\n",
    }) );
    // Chronos' new arming monster function.. Easy!
    // This function will clone the first argument (in this case,
    // "/doc/examples/axe", and try to wield the second argument immediately
    // after cloning the first argument.  The wear() function works exactly
    // the same.  These functions should always be called last in the create()
    // to make sure all the necessary properties for wielding have already
    // been set. :D
    arm("/doc/examples/axe", "axe") ;
}
