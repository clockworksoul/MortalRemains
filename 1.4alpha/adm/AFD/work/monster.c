// File: /std/monster.c
// The monster body is from the TMI-2 lib. Part of the bodies project and
// implemented by Watcher@TMI-2 and Mobydick@TMI-2. The code contained
// in this object is heavily based on code found in the original user.c
// and monster.c at TMI.
//
// The relevant code headers follow:
// Original author of monster.c: Huma@TMI.
// Subsequent work done by Buddha@TMI-2 and Mobydick@TMI-2.
// Original authors of user.c were Sulam@TMI and Buddha@TMI.
// Many other people have added to this as well.
// This file has a long revision history dating back to the hideous
// mudlib.n and is now probably not at all the same.
// This file is part of the TMI distribution mudlib.
// Please keep this header with any code within.
//
// leto might have fixed type checking, might have killed it ;)
// (07-27-96) Chronos started adding real headers to his code again.  Bwaha
//            Added second attacks for monsters, reworked some of the old
//            combat code to make it more efficient and clean. 
#include <config.h>
#include <daemons.h>
#include <net/daemons.h>
#include <mudlib.h>
#include <move.h>
#include <money.h>
#include <priv.h>
#include <uid.h>
#include <logs.h>
#include <body.h>
#include <monster.h>

object tmp ;
void set_level(int lvl);
static int speed, hb_status, level;
 
inherit LIVING ;
 
int environment_check();
void spell_cast (string spell, int chance) ;
void move_around() ;
void monster_chat();
void unwield_weapon(object weapon) ;
void unequip_armor(object armor) ;
varargs private void execute_attack(int hit_mod, int dam_mod) ;
static int create_ghost();
static void die();
 
mapping alias ;
 
//  Setup basic and command catch systems
 
void basic_commands() {
   add_action("quit", "quit");
}

static void init_commands() {
   string path;

   add_action( "cmd_hook", "", 1 );
 
   if(link_data("wizard")) {
   enable_wizard();
   path = NEW_WIZ_PATH;
   }
 
   else path = USER_CMDS;
 
      set("PATH", path, MASTER_ONLY);
}
 
//  This function resets the monster's living_name after a user 
//  shells out of the monster body.
 
void reset_monster() {
   if(!interactive(this_object())) {
     set_living_name( query("name") );
     seteuid(getuid(this_object()));
     link = 0;
   }
}
 
void clear_monster() { if(geteuid(previous_object()) == ROOT_UID) seteuid(0); }
 
string process_input (string arg) {
        arg = do_alias(arg) ;
        return arg ;
}
 
//   Setup standard user command hook system.  This system interfaces
//   with the cmd bin system, the environment's exits, and feeling entries.

nomask static int cmd_hook(string cmd) {
   string file;
   string verb;

   verb = query_verb();
 
   if (environment() && environment()->valid_exit(verb)) {
      verb = "go";
      cmd = query_verb();
   }

   file = (string)CMD_D->find_cmd(verb, explode(query("PATH"), ":"));
 
   if (file && file != "")
      return (int)call_other(file, "cmd_" + verb, cmd);
 
   if (environment() && environment()->query("quiet")) return 0 ;
   return (int)EMOTE_D->parse(verb, cmd);
}

/*
* Move the player to another room. Give the appropriate
* message to on-lookers.
* The optional message describes the message to give when the player
* leaves.
*/
 
varargs int move_player(mixed dest, string message, string dir) {
   object prev;
   int res;
   
   prev = environment( this_object() );
 
   if( res = move(dest) != MOVE_OK ) {
      write("You remain where you are.\n");
      return res;   }
 
   if(message == "SNEAK") {
     set_temp("force_to_look", 1);
     command("look");
     set_temp("force_to_look", 0);
   return 0; } 
 
   if(!query("invisible")) {
 
      if (message == 0 || message == "") {

        if(dir && dir != "") {
           tell_room(prev, (string)this_object()->query_mout(dir) + "\n",
                        ({ this_object() }));
           say((string)this_object()->query_min() + "\n", ({ this_object() }));
        } else {
           tell_room(prev, (string)this_object()->query_mmout() + "\n",
                        ({ this_object() }));
           say((string)this_object()->query_mmin() + "\n", ({ this_object() }));
        }
     } else {
        tell_room(prev, message + "\n", ({ this_object() }));
        say((string)this_object()->query_min() + "\n", ({ this_object() }));
     }
   }
 
    set_temp ("force_to_look", 1) ;
    command("look") ;
    set_temp("force_to_look", 0) ;

 
   //	Follow/track mudlib support
 
   if(!this_object()->query("no_follow") && environment() != prev)
   all_inventory(prev)->follow_other(this_object(), environment(), dir);
 
   return 0;
}

private int clean_up_attackers() {
   mixed *attackers_tmp;
   int i;
   attackers_tmp = ({});
   for (i=0; i < sizeof(attackers); i++) {
// If he's dead, then forget about him entirely.
	if (attackers[i] == 0 || !living(attackers[i])) continue;
// If he's not here, then we consider two cases.
    if (environment(attackers[i]) != environment(TP) ||
    (userp(attackers[i]) && !interactive(attackers[i])) )
	{
// If this is a monster who is not forgetful, then add this attacker to the
// will_attack, the list of objects to be attacked on sight.
	   if (!query("forgetful")) {
	  	if (!will_attack) will_attack = ({ attackers[i] }) ; else
	       will_attack += ({ attackers[i] }) ;
	   }
// If a forgetful monster, then forget about him.
	    continue ;
	    }
// If he's a ghost, we've done enough to him already :)
      if ((int)attackers[i]->query("ghost")) continue ;
// If we get this far, then we still want to be attacking him
      attackers_tmp += ({ attackers[i] });
   }
// Copy the tmp list over to the attackers list.
   attackers = attackers_tmp;
   if (sizeof(attackers_tmp) == 0)
      any_attack = 0;
   return any_attack;
}

// Continue_attack is called from heart_beat in monster.c and user.c.
// here is where we can try to see if we're dead or in combat.

void continue_attack() {
// Check if this object has just died. if so, do the death stuff.
int hand;
   if (query("hit_points") < 0 && !query("ghost")) {  die();  return;  }
 
// If there's no one to attack, then we are finished.
   if (!any_attack) return;
// Call the clean_up_attackers function to see who's left. If it returns
// 0, then there's no one left.
   if (clean_up_attackers() == 0) {
      write("The melee has ended.\n"); /* No attackers in the room */
      return;
   }
// Check to see if the monster is doing something that prevents him from
// making an attack.
   if (query("stop_attack")>0) return ; 
// Check to see if we're under wimpy, and if so, run away.
// Chronos made monsters flee 10% of the time.  Otherwise you can't
// hit them after they wimpy.
  if (query("hit_points")*100/query("max_hp") < (int)query("wimpy")
      && !random(10)) {
       run_away();
       return;
   }
   
// If this is a spellcasting monster, then maybe we ought to throw a spell.
   if (query("spellcaster")==1 && random(100)<query("spell_chance")) 
        call_other(this_object(),query("spell_to_cast")+"_cast") ;
// If we don't throw a spell, or are not spellcasting, then just make a
// normal weapon attack.
// If they are wielding two weapons, they get two attacks. ;)
    if (query("right_hand") && query("left_hand")) {
          hand = random(2) + 1; 
          execute_attack(0,0,hand);
          if (hand==1) hand = 2;
          else hand = 1;
  if ((int)query("level")*3<random(100)) {
        write("Attacking again .. ");
        message("combat", "Attacking again .. ", environment(),
             ({ this_object() }) );
                execute_attack(0,0,hand);
  }
  }
  else   // if we are here, we have 1 weapon, or none.
   execute_attack(0,0,1);
}

// This is the big, ugly CPU hogging function where the combat actually
// takes place.
 
static int noise, loop;

varargs void execute_attack(int hit_mod, int dam_mod, int hand) {

        int att_sk, def_sk, str, dex, ac, wc, hit_chance ;
  string name, verb1, verb2, vname, damstr, damstr2, wepstr, *verbs;
        mixed *contents;
        object *prots ;
        object weapon ;
        int old_inv ;
        string victim ;
        string combat_str;
        int *damrange ;
// Insurance.  Sometimes things don't wanna fight back.  Heeee-ya!!
   if (attackers[0]) attackers[0]->kill_ob(this_object());

// hit_mod and dam_mod are modifiers that can be passed to the attack.
// The heartbeat doesn't add them but you can make special attacks by
// calling execute_attack directly. Be careful if you do so, you'll want
// to also call kill_ob to make sure a fight starts...
 
        if (!hit_mod) hit_mod=0 ;
        if (!dam_mod) dam_mod=0 ;
 
// Check to see if primary target is dead, if so move to the next attack
// in the attackers queue. If the attackers queue is empty, stop attack call.

        if(attackers[0]->query("hit_points") < 0) {
          attackers -= ({ attackers[0] });
          if(!attackers || !sizeof(attackers))  return;
        }
// Is the target being protected? If so, find the alternate target.
        prots = attackers[0]->query("protectors") ;
        if (prots && sizeof(prots)>0) {
// Get rid of all ineligible protectors: dead or not present.
            prots = filter_array(prots,"valid_protect", this_object()) ;
// If there are eligible protectors, then move the protector to the top
// of the list, adding him if needed.
                if (sizeof(prots)) {
  //                    victim = prots[random(sizeof(prots))] ;
// changed by chronos.
// this was fucking up cuz it was using the wrong var type.  OOPS.
       weapon = prots[random(sizeof(prots))] ;
// re-using a variable to save memory - sorry bout that :(
                        ac = member_array(weapon,attackers) ;
                        if (ac>-1) {
                                attackers[ac]=attackers[0] ;
                                attackers[0]=weapon ;
                        } else {
                                attackers = ({ weapon }) + attackers ;
                                weapon -> kill_ob (this_object()) ;
                        }
                }
        }
   attackers[0]->kill_ob(this_object());
 
// Collect the various statistics needed to get the hit chance and damage.
        str = this_object()->query("stat/strength") ;
        dex = attackers[0] ->query("stat/dexterity") ;
        ac = attackers[0]->query_ac() ;
   switch(hand) {
     case 1:
        weapon = this_object()->query("left_hand");
    if (!weapon) weapon = this_object()->query("right_hand");
   break ;
     case 2: weapon = this_object()->query("right_hand");
   if (!weapon) weapon = this_object()->query("left_hand");
    break ;
  }
// If they don't have a weapon, they get their intrinsic combat skills.
        if (!weapon) {
                        wc = query_wc() ;
                        damrange = allocate(2) ;
                        damrange = query("damage") ;
                        dmin = damrange[0] ;
                        dmax = damrange[1] ;
                        verbs = allocate(2) ;
                        verbs = get_verb() ;
                        verb1 = verbs[0] ;
                        verb2 = verbs[1] ;
                        wepstr = query("weapon_name") ;
        } else {
// If we get here, then the monster has a weapon, and we query the weapon
// for its attack properties.
                wc = weapon->query("weapon") ;
                damrange = weapon->query("damage") ;
                dmin = damrange[0] ;
                dmax = damrange[1] ;
                verbs = allocate(2) ;
                verbs = weapon->get_verb() ;
                verb1 = verbs[0] ;
                verb2 = verbs[1] ;
                wepstr = (string)weapon->query("name") ;
                weapontype = capitalize(weapon->query("type")+" weapons") ;
        }
        name = query("cap_name") ;
        vname = attackers[0]->query("cap_name") ;

// Check the attacker's attack skill and the defenders skill(s).

        att_sk = this_object()->query_skill("attack") ;
// Ditto for the defender.
        if (!(int)attackers[0]->query_monster()) {
                if (attackers[0]->query("armor/shield")) {
                        def_sk = attackers[0]->query_skill("Shield defense") ;
                } else {
                        def_sk = attackers[0]->query_skill("Parrying defense") ;
                }
        } else {
                def_sk = attackers[0]->query_skill("defense") ;
        }

// This is the combat formula.
// If you are using drunkenness, and want it to affect combat, then
// call query("drunk"), which goes 1-25, and subtract it from the
// hit chance.

  hit_chance = 30 + str + att_sk + 3*wc - dex - def_sk - ac/6 + hit_mod ;
  if (query("drunk")>0) hit_chance -= query("drunk") * 3;
 if (!query("vision")||query("blind")) {
   if (query("blind") || !query("infravision")) 
         hit_chance -= 50;
  }

// Hitting invisible things is hard.

        if ((int)attackers[0]->query("invisible")>0) hit_chance=hit_chance/5 ;

// The hit chance is constrained to be between 2 and 98 percent.
        if (hit_chance<2) hit_chance = 2 ;
        if (hit_chance>98) hit_chance = 98 ;


// Improve the skills of the defender, if a player.

// The probability of the skill improving depends on the hit chance. If the
// hit chance is 0 or 100, the skill does not improve. If the hit chance is
// 50%, then the skill improves automatically. The closer the hit chance is
// to 50%, the more likely the skill is to improve. This rewards players for
// taking on monsters roughly equal in skill to themselves.

        skill_improve_prob = hit_chance * (100-hit_chance) / 25 ;
        if (random(100)<skill_improve_prob) {
                if (interactive(attackers[0])) {
                        if (attackers[0]->query("armor/shield")) {
                                attackers[0]->improve_skill("Shield defense",1) ;
                        } else  {
                                attackers[0]->improve_skill("Parrying defense",1) ;
                        }
                }
        }

// Get a list of who is listening in the room.
 
        contents = all_inventory(environment(this_object()));
        contents = filter_array(contents, "filter_listening", this_object());
 
// This is the damage formula.
// We have to calculate this first because we don't want to print messages
// of the form "You hit for zero damage." If the damage is less than zero,
// we print a "You miss" message regardless of the hit_chance roll.

    damage = random(dmax-dmin+1)+dmin+str/8-1+att_sk/10-def_sk/5+dam_mod ;
    if (weapon) damage += weapon->weapon_hit(damage);

// Before printing any messages, we have to make ourselves visible so that
// the victim definitely gets the message. At the end of the attack we'll
// restore the old invisibility setting.

        old_inv = query("invisible") ;
        set ("invisible", 0) ;

// If positive damage, and the hit lands, then we do damage and print
// the appropriate damage messages.
  if (damage>0 && random(100)<hit_chance) {
    str = attackers[0]->query("hit_points") ;
   if (damage) {
     if (weapon) attackers[0]->receive_damage(damage, weapon->query("damage_type"));
     else attackers[0]->receive_damage(damage);
  }
    qs = objective((string)attackers[0]->query("gender")) ;

  /*  These are the variables we have:
   *  name              Attacker name.
   *  vname             Victim name.
   *  qs                Objective of victim;
   *  verb1             The verb that the attacker sees.
   *  verb2             The verb that everyone else sees.
   *  wepstr            The weapon name.
   *
   * damstr is what the ATTACKER sees.
   * damstr2 is what the OBSERVERS see.
   * combat_str is what the VICTIM sees.
   */
    switch (damage) {
      case 1: {
       damstr = "You "+verb1+" "+vname+" with your "+ wepstr +
                      " and scratch "+qs+".";
       damstr2 = name +" "+verb2+" "+vname+ " with " + 
                      possessive(this_object()->query("gender"))+" "+ 
                      wepstr + " and scratches "+qs+".";
       combat_str = name+" "+verb2+" you with " + 
                      possessive(this_object()->query("gender"))+" "+ 
                      wepstr + " and scratches you."; 
        break ;
      }  
      case 2..3 : {
        damstr = "You barely tap "+vname+" with your " + wepstr+ "." ;
        damstr2 = name+" barely taps "+vname+" with " +
               possessive(this_object()->query("gender"))+" " +wepstr+"." ;
        combat_str = name+" barely taps you with "+ 
          possessive(this_object()->query("gender"))+" " +wepstr+".";
        break ;
      }  
      case 4..6 : {
        damstr = "You hit "+vname+"." ;
        damstr2 = name+ " hits "+vname+"." ;
        combat_str = name+" hits you."; 
        break ;
      }
      case 7..9 : {
        damstr = vname+" grunts as you deal "+qs+" a solid blow.";
        damstr2 = vname+" grunts as "+name+" deals "+qs+" a solid blow." ;
        combat_str = "You grunt as "+name+" deals you a solid blow."; 
        break ;
      }
      case 10..15 : {
     damstr="You "+verb1+" "+vname+" with your "+
                     wepstr+" and hit "+qs+" hard.";
       damstr2 = name +" "+verb2+" "+vname+ " with " + 
             possessive(this_object()->query("gender"))+" "+ 
                   wepstr + " and hits "+qs+" hard.";
       combat_str = name+" "+verb2+" you with " + 
              possessive(this_object()->query("gender"))+" "+ 
              wepstr + " and hits you hard."; 
        break ;
      }
      case 16..20 : {
        damstr  = "You "+verb1+" "+name+" with your "+
                 wepstr+" and devastate "+qs+".";
        damstr2 = name +" "+verb2+" "+vname+ " with " + 
             possessive(this_object()->query("gender"))+" "+ 
             wepstr + " and devastates "+qs+"."; 
        combat_str = name+" "+verb2+" you with " + 
             possessive(this_object()->query("gender"))+" "+ 
             wepstr + " and devastates you."; 
        break ;
      }
      case 21..30 : {
        damstr = "You nearly knock the wind from "+vname+" with your powerful blow.";
        damstr2 = name+" nearly knocks the wind from "+vname+" with "+
             possessive(this_object()->query("gender"))+" powerful blow.";
        combat_str = name+" nearly knocks the wind from you with "+
        possessive(this_object()->query("gender"))+" powerful blow.";
        break ;
      }
      case 31..40 : {
        damstr = vname+" screams as "+name+" deals "+qs+" a crushing blow.";
        damstr2 = vname+" screams as "+name+" deals "+qs+" a crushing blow.";
        combat_str = "You scream as " +name+" deals you a crushing blow.";
        break ;
      }
      default : {
        damstr = "Blood flies through the air as you brutally maim "+
              vname+" with your "+wepstr+"!";
        damstr2 = "Blood flies through the air as "+name+" brutally maims "+
              vname+" with " +possessive(this_object()->query("gender"))+ 
                " "+ wepstr+ "!";
        combat_str = "Blood flies through the air as " +name+
                      " brutally maims you with " +
                          possessive(this_object()->query("gender"))+ 
                           " " + wepstr+ "!";break;
      }
    }

        // The routines below check to see if all the listeners really
        // want to hear how the battle is going (Watcher, 4/27/93).
 
        if(!(this_object()->query("noise_level") && damage < 2))
     tell_object(this_object(), damstr + "\n");
 

        for(loop=0; loop<sizeof(contents); loop++) {
          if(contents[loop])
                noise = (int)contents[loop]->query("noise_level");
 
          if(noise && (noise > 1 || (noise == 1 && damage < 2)))
                continue;

     tell_object(contents[loop], damstr2 + "\n");

  }
 
        if(attackers[0] && 
           !(attackers[0]->query("noise_level") && damage < 2))
        tell_object(attackers[0], 
          combat_str + "\n");

        } else {

// If we got here, it means we missed the hit roll, or did zero damage.

     damstr2=possessive(this_object()->query("gender"));
   qs = subjective((string)attackers[0]->query("gender"));
        switch(random(11)) {
          case 0: damstr = "You "+verb1 + " " + vname + " with your " +
                    wepstr + ", but you miss.\n";break;
          case 1: damstr = "You "+ verb1 + " " + vname + " with your " +
                  wepstr+", but "+qs+
                       " dodges your clumsy blow.\n";break;
          case 2: damstr = "You " + verb1 + " " + vname + " with your "
                       +wepstr+", and barely miss.\n";break;
          case 3: damstr ="You " + verb1 + " " + vname + " with your " 
                    +wepstr + ", but does no damage.\n";
                         break;
          case 4: damstr =vname+" blocks your "+wepstr+
                         " with ease.\n";break;
          case 5: damstr ="You "+verb1+" "+vname+
                        ", but trip and fall.\n";break;
          case 6: damstr ="You " + verb1+" "+vname+" with your " 
                 + wepstr +" and deal a mighty blow to empty air.\n";
                break;
         case 7: damstr = "You " + verb1+ " "+vname + " with your "+
                 wepstr+", but only throw yourself off balance.\n";break;
          case 8: damstr ="You miss "+vname+" by an inch!\n";break;
          case 9: damstr ="You miss "+vname+" by a mile!\n";break;
          default: damstr ="You miss.\n";break;
        }
        if(!this_object()->query("noise_level"))
    tell_object(this_object(), damstr);
 
        switch(random(11)) {
          case 0: damstr = name+" "+verb2 + " " + vname + " with " +
                    damstr2+ " "+wepstr+", but misses.\n";
                    break;
          case 1: damstr = name+" "+ verb2 + " " + vname + " with " +
         damstr2+ " " + wepstr + ", but "+qs+
       " dodges "+damstr2+" clumsy blow.\n";break;
          case 2: damstr = name+" " + verb2 + " " + vname + " with " +
                     damstr2+" "+wepstr+", but barely misses.\n";break;
          case 3: damstr =name+" " + verb2 + " " + vname + " with " 
                    +damstr2+ " "+wepstr + ", but does no damage.\n";
                               break;
          case 4: damstr =vname+" blocks "+damstr2+" "+wepstr+
                         " with ease.\n";break;
          case 5: damstr =name+" "+verb2+" "+vname+
             ", but trips and falls.\n";break;
          case 6: damstr =name+" " + verb2+" "+vname+" with "+damstr2+" " 
                 + wepstr +" and deals a mighty blow to empty air.\n";
                break;
       case 7: damstr = name + " "+verb2+" "+vname+" with "+damstr2+" "+
   wepstr+", but only loses "+damstr2+" balance.\n";break;
          case 8: damstr =name+" misses "+vname+" by an inch!\n";
                break;
          case 9: damstr =name+" misses "+vname+" by a mile!\n";
                break;
          default: damstr =name+" misses.\n";
                break;
        }
        for(loop=0; loop<sizeof(contents); loop++) 
          if(contents[loop] && !contents[loop]->query("noise_level"))
          tell_object(contents[loop],
  damstr);
 
        switch(random(11)) {
          case 0: damstr = name+" "+verb2 + " you with " +
                    damstr2+ " " + wepstr + ", but misses.\n";
                    break;
          case 1: damstr = name+" "+ verb2 + " you with " +
                     damstr2+" "+wepstr + ", but you dodge "+damstr2
                        +" clumsy blow.\n";break;
          case 2: damstr = name+" " + verb2 + " you with " +
                     damstr2+" "+wepstr+
                      ", but barely misses.\n";break;
          case 3: damstr =name+" " + verb2 + " you with " 
                    +damstr2+ " "+wepstr + ", but does no damage.\n";
                break;
          case 4: damstr = "You block "+damstr2+" "+wepstr+
                         " with ease.\n";break;
          case 5: damstr =name+" "+verb2+
                " you, but trips and falls.\n";break;
          case 6: damstr =name+" " + verb2+" you with "+damstr2+" "
                 + wepstr +" and deals a mighty blow to empty air.\n";
                break;
           case 7: damstr = name+" "+verb2+" you with "+damstr2+
              " "+wepstr+", but only loses "+damstr2+" balance.\n";
                  break;
          case 8: damstr =name+" misses you by an inch!\n";
                break;
          case 9: damstr =name+" misses you by a mile!\n";
                break;
          default: damstr =name+" misses.\n";
                break;
        }
        if(attackers[0] && !attackers[0]->query("noise_level"))
          tell_object(attackers[0],
  damstr);
        }

// Restore the old invis setting.

        if (old_inv) set ("invisible", old_inv) ;

return; }

//  This function filters an array returning only living objects.
 
int filter_listening(object obj) {
   if(obj == attackers[0] || obj == this_object())  return 0;
return living(obj); }

int valid_protect (string str) {

	object foo ;

  foo = find_living(str);
	if (!foo) return 0 ;
	if (environment(foo)!=environment(this_object())) {
		return 0 ;
	}
        if ((int)foo->query("hit_points")<0) return 0 ;
        return 1 ;
}
 
//	If the monster body is inhabited by a user, give the
//	monster's living hash table name as that of the user.
 
void init_setup() {

	if(!query_link())  return 0;
 
	set_heart_beat(1);
	seteuid(getuid(this_object()));
	set_living_name( link_data("name") );
 
	basic_commands();
	init_commands();
}
 
void create() {
// Monsters need euid set so that they can clone corpses if they die.
	seteuid(getuid()) ;

// no default aliases.
   alias = ([ ]);
  set("npc", 1, LOCKED);
 
// Monsters default to the same size and carrying ability as humans. You
// might want to change this in non-humanoid monsters.
 
   set("autosave", 500);
   set("volume", 500);
   set("max_vol", 500) ;
   set("capacity", 5000);
   set("max_cap", 5000) ;
   set("mass", 7500) ;
   set("bulk", 1000) ;
   set("time_to_heal",10) ;
   set("short", "@@query_short");
   set("vision", "@@query_vision");
   set("gender", "neuter");
   enable_commands();

	set ("languages", ([ "common" : 100 ]) ) ;
   set_heart_beat(1);

  set_level(1);
  set("race", "monster");
   set ("PATH", "/cmds/std") ;
   basic_commands() ;
   init_commands() ;
}

void remove() {
   mixed *inv;
   int loop;
 
   inv = all_inventory(this_object());
   for(loop=0; loop<sizeof(inv); loop++)
     if(inv[loop]->query("prevent_drop"))
	inv[loop]->remove();
 
   CMWHO_D->remove_user(this_object());
   if (link) link->remove();
   living::remove();
}
 
int quit(string str) {
   object ob;
   int i, j;

   if(str) {
   notify_fail("Quit what?\n");
   return 0; }
 
#ifdef LOGOUT_MSG
   tell_object(this_object(), LOGOUT_MSG);
#endif
 
#ifdef QUIT_LOG
   if(this_object() && interactive(this_object()))
   log_file(QUIT_LOG, link_data("cap_name") + ": Monster quit from " +
	    query_ip_name(this_object()) + " [" +
	    extract(ctime(time()), 4, 15) + "]\n");
#endif
  
   //   If the connection object is somehow missing ... 
   //	just remove the monster 
 
   if(!query_link()) {
     if(interactive(this_object()))  remove();
     reset_monster();
   return 1; }
 
   ob = new(OBJECT);			// Temporary body
 
   query_link()->set("tmp_body", ob);
   query_link()->switch_body();
   ob->remove();
 
   query_link()->remove();
   reset_monster();
 
return 1; }

void heart_beat() {

// If we're chasing someone, better go after him.

   if (query("in_pursuit")) {
      command("go "+query("in_pursuit")) ;
      if (present((object)query("pursued"),environment(this_object()))) {
         tell_object((object)query("pursued"),this_object()->query("cap_name")+" attacks you!\n") ;
         query("pursued")->kill_ob(this_object()) ;
      }
      delete ("in_pursuit") ;
      delete ("pursued") ;
   }

   continue_attack();
 
   if (query("moving")==1) move_around() ;
   if(random(100)+1 < query("chat_chance"))  monster_chat();
 
   unblock_attack();
   heal_up();
 
   //  If no-one is around and fully healed, shut down the heartbeat
 
   if(!environment_check() && query("hit_points") == query("max_hp")) {
 
   if(query("max_sp") && query("max_sp") != query("spell_points"))
	return;

// If we're whanging on someone, best not to shut it down... :)

	if (sizeof(attackers)) return ;
 
   set_heart_beat(0);  hb_status = 0;  
   }
 
}
 
//  This function allows monsters to talk or give environmental sounds.

void monster_chat() {
   mixed *chats;

   if(attackers && sizeof(attackers))  chats = query("att_chat_output");
   else  chats = query("chat_output");

   if(!chats || !sizeof(chats))  return ;
 
   if(environment())
   tell_room(environment(), chats[ random(sizeof(chats)) ]);

return; }
 
 
// Let's get rid of this somehow.
void set_name (string str) {
	if (!str)  return;
	set ("name", str) ;
	set ("cap_name", capitalize(str)) ;
	set_living_name(str) ;
}
 
static void die() {
   object killer, ghost, corpse, coins, *stuff;
   mapping wealth ;
   string *names ;
   int i, res, totcoins ;
 
   // 	Define the monster's killer

   killer = query("last_attacker");
 
   init_attack() ;
 
   //	Setup corpse with monster's specifics
 
   if(query("alt_corpse"))
  	corpse = clone_object(query("alt_corpse"));
   else corpse = clone_object(STANDARD_CORPSE);
 
   corpse->set_name(query("cap_name")) ;
 
   i = query("mass") ;
   if (i>0) corpse->set("mass", i) ;
 
   i = query("bulk") ;
   if (i>0) corpse ->set("bulk", i) ;
 
   i = query("capacity") ;
   if (i>0) corpse ->set ("capacity", i) ;
 
   i = query("volume") ;
   if (i>0) corpse ->set("volume", i) ;
 
   corpse->move(environment(this_object()));
 
   stuff = all_inventory(this_object());
 
   for(i=0;i<sizeof(stuff);i++)
   stuff[i]->move(corpse); 
 
  if (this_object()->query_coins()) {
  coins = clone_object(COINS);
  coins->set_number(this_object()->query_coins());
	 if (coins) coins->move(corpse) ;
   }
 
   //  Set the last_kill variable ... file_name and short of this_object

   if(killer)
   killer->set("last_kill", ({ file_name(this_object()), query("short"),
	       time() }));
 
   // Announce the sad facts of life and non-life.
 
   write("You have died.\n");
   say(query("cap_name") + " staggers and falls to the ground ... dead.\n",
  ({ this_object() }) );
  if (killer && userp(killer)) {
   say(killer->query("cap_name")+ " killed "+query("cap_name")+".\n",
    ({ this_object(), killer }) );
   tell_object(killer, "You killed "+query("cap_name")+".\n");
 }

   //  	NOTE:  If you want to give a reward of skill points or xp or
   //	       whatever to the killer, define the following function
   //	       in the monster's inherit file.

  if (killer) {
    this_object()->kill_reward(killer);
    killer->autokill();
    killer->add("kills", 1);
  }
 
   //	If monster is inhabited, switch user to ghost body
 
   if(interactive(this_object())) {

   	ghost = create_ghost();
	if(!ghost)  return;

#ifdef GHOST_START_LOCATION
   if(ghost->move(GHOST_START_LOCATION) != MOVE_OK)
	ghost->move(START);
#else
   ghost->move(START);
#endif
 
   }

   //	Delay the remove call with a call_out to ensure all the 
   //	housekeeping from above is complete.
 
   call_out("remove", 0);
 
}

// These are the hooks for aggressive and killer monsters.
// If a monster turns on the "aggressive" property, then if they receive
// a message of the form "Foo enters" then they attack Foo. This is done
// in receive_messages. Players can get past these monsters by sneaking
// into the room or being invisible. Also, they don't attack if they enter
// the player's environment, only if the player enters theirs. Monsters
// which turn on the "killer" property will attack any player that triggers
// the monster's init. It can't be fooled by invis or sneaking, and it will
// attack you if it moves into your room. Moving killer monsters can be
// pretty nasty things.

void relay_message(string class, string str) {

   string name ;
   string direction ;
   object victim ;

   if (str && (sscanf(str,"%s enters.",name)==1 ||
	sscanf (str, "Suddenly %s appears as if out of thin air.",name)==1)) {
      name = lower_case(name) ;
      victim = present(name,environment(this_object())) ;
      if (!victim) return ;
     // Are we looking for you?
     if (will_attack && member_array(victim, will_attack)!=-1) 
        execute_attack();
 
      if(query("aggressive") && !victim->query("ghost")) {
	tell_object (victim, "Suddenly the "+query("name")+" attacks you!\n") ;
	kill_ob(victim) ;
      }
   }
   if (query("pursuing") && str && sscanf(str,"%s leaves %s.",name,direction)==2) {
      name = lower_case(name) ;
      victim = find_living(name) ;
      if (attackers && member_array(victim,attackers)>-1) {
	set ("in_pursuit", direction) ;
	set ("pursued", victim) ;
      }
   }

}

void init() {
 
   //  If object is not a monster and is visible...think about attacking.
 
   if(!this_player()->query("npc") && !this_player()->query("ghost") &&
      visible(this_player())) {
 
      if (query("killer")) {
      write("As you enter, " + capitalize(query("name")) + " suddenly " +
	    "attacks you!\n");
      kill_ob( this_player() );
      }
 
      if(will_attack && member_array(this_player(), will_attack) != -1) {
      will_attack -= ({ this_player() });
      kill_ob(TP);
      TP->kill_ob(TO);
      }
 
      //  If heartbeat is turned off ... turn it back on.
 
      if(!hb_status) {  hb_status = 1;  set_heart_beat(1);  }
 
   }
}

// This lets a pursuing monster chase you. It's call_out'd from
// relay_message.


int query_monster() { return 1; }
 
int query_npc() { return 1; }
 
int query_hb_status() {  return hb_status;  }

void move_around() {

   mapping doors,exits, tmp, patrol ;
   string foo,tstr ;
   string *nogos ;
   string *dirs ;
   int i ;

   if (query("moving")==1) set("moving",-1) ;
// If we have no environment, then stop moving around. Done for two
// reasons. First, there's nowhere to go anyway, so save CPU. Second,
// this stops the callout loop in master copies of objects.
// We also shut it down if there's no exits: same logic.
   if (!environment(this_object()) ||
		!environment(this_object())->query("exits")) {
	return ;
  }
  call_out("move_around", (int)query("speed") * 2);
// If we're in combat, probably oughta stay here.
   if (sizeof(attackers)) return ;

// If we have a patrol pattern to follow, get the direction from
// that. If not, pick a direction at random.

   exits = environment(this_object())->query("exits") ;
   patrol = this_object()->query("patrol") ;
   if (sizeof(patrol)) {
      i = this_object()->query("patrol_step") ;
      foo = patrol[i] ;
   } else {
      dirs = keys(exits) ;
      foo = dirs[random(sizeof(dirs))] ;
      nogos = query("forbidden_rooms") ;
      if (nogos && member_array(exits[foo],nogos)!=-1) {
		return ;
	}
   }
   doors = environment(this_object())->query("doors") ;
   if (doors && sizeof(doors) && doors[foo] &&
		doors[foo]["status"]!="open") {
      command("open "+foo+" door") ;
      return ;
   }
   tmp = environment(this_object())->query("exit_msg");
   if(tmp && mapp(tmp) && tmp[foo]) {
        tstr = substr(tmp[foo], "$N", (string)query("cap_name"));
         move_player(exits[foo], tstr);
   } else {
      tstr = query_mout(foo);
      move_player(exits[foo], tstr);
   }
    if (sizeof(patrol)) {
      i = i + 1 ;
      if (i==sizeof(patrol)) i=0 ;
      set ("patrol_step", i) ;
    }
}

void spell_cast (string spell, int chance) {
	set ("spellcaster",1) ;
	set ("spell_to_cast",spell) ;
	set ("spell_chance",chance) ;
}

/*
void wield_weapon (object weapon) {

	object foo ;
	string str ;

       foo = query("left_hand");
	if (foo) unwield_weapon(foo) ;
	set ("weapon1", weapon) ;
	weapon->set("wielded", 1) ;
	str = weapon->query("wield_func") ;
	if (str) call_other(weapon,str) ;
}

void equip_armor (object armor) {

	string type ;
	object foo ;

	type = armor->query("type") ;
	foo = query("armor/"+type) ;
	if (foo) unequip_armor(foo) ;
	set ("armor/"+type,(int)armor->query("armor")) ;
	armor->set("equipped",1) ;
	calc_armor_class() ;
}

void unwield_weapon (object weapon) {

	string str ;

	set ("weapon1", 0) ;
	weapon->unwield() ;
	str = weapon->query("unwield_func") ;
        if (str) call_other(weapon,str) ;
}

void unequip_armor (object armor) {

	string type ;

	type = armor->query("type") ;
	delete ("armor/"+type) ;
	armor->unequip() ;
	calc_armor_class() ;
}
*/
void wield_weapon (mixed weapon) {
    if(objectp(weapon))
        command("wield "+((string *)(weapon->query("id")))[0], this_object());
    if(stringp(weapon))
        command("wield "+weapon, this_object());
}

void unwield_weapon (mixed weapon) {
    if(objectp(weapon))
        command("unwield "+((string *)(weapon->query("id")))[0], this_object());
    if(stringp(weapon))
        command("unwield "+weapon, this_object());
}

void equip_armor (mixed armour) {
    if(objectp(armour))
        command("wear "+((string *)(armour->query("id")))[0], this_object());
    if(stringp(armour))
        command("wear "+armour, this_object());
}

void unequip_armor (string armour) {
    if(objectp(armour))
        command("remove "+((string *)(armour->query("id")))[0], this_object());
    if(stringp(armour))
        command("remove "+armour, this_object());
}

void kill_reward(object killer) {
   int exp, killer_lvl, level;
  level = (int)this_object()->query_level();
  killer_lvl = (int)killer->query_level();
        exp = this_object()->query_exp();
       if (!exp) exp = 30;
        if (exp<30) exp = 30;
        if (exp > MAX_EXP) exp = MAX_EXP;
// Examples:  50 exp mon gives 25 xp this way.
        exp = exp / 50 ;  // Roughly 50 monstes of your level to go up 1
  if (killer_lvl-10>level) exp = 0;
   if (exp<30) exp = 30;
        if (userp(killer))
    if (wizardp(killer)) 
  tell_object(killer,
   "[Wizard] You just received "+exp+" xp from that kill.\n");
        killer->add_exp(exp);
        killer->add_alignment(-1 * query_alignment());
        return;
}

int set_hide_race() {
  set("supress_race", 1);
  return 1;
}

int set_hide_condition() {
  set("supress_condition", 1);
  return 1;
}

 
//  This function checks to see if there are any players present in 
//  the monsters room, so heartbeat can be turned off periodically.

int environment_check() {
 
   if(interactive(this_object()))  return 1;
 
   if(environment())
   return sizeof(filter_array(all_inventory(environment()), "filter_env",
		 this_object()));  
return 0; }
 
static int filter_env(object obj) {
   return (interactive(obj) && visible(obj, this_object())); }

void set_level(int lvl) {
  level=lvl;
  set_skill("defense", level, "dexterity");
  enable_commands();
  set_skill("attack", level, "dexterity");
  set("level", lvl);
  set("stat/strength",level);
  set("stat/spirit", level);
  set("stat/agility", level);
  set("stat/intelligence",level);
  set("stat/constitution",level);
  set("stat/dexterity",level); 
  if (lvl > sizeof(STAT))
    lvl = sizeof(STAT);
  else if (lvl < 1)
    lvl = 1;
  lvl--;
  set("hit_points",STAT[lvl][0]);
  set("max_hp",query("hit_points"));
  set("spell_points",query("hit_points"));
  set("max_sp",query("hit_points"));
  set("attack_strength",STAT[lvl][1]);
  set("armor_class", "@@query_ac");
  set("monster_ac", STAT[lvl][2]);
  set("spell_chance",STAT[lvl][3]);
  set("mon_spell_dam",STAT[lvl][4]);
  set("spell_mess1",SPELL_MSG[0]);
  set("spell_mess2",SPELL_MSG[1]);
  if(level>1)
    experience=GUILD_D->query_xp_for_next(level-1);
  else
    experience= random(100)+100;
  if (experience<30) experience = 30;
   add_exp(experience);
}
void set_alignment(int x) {
  this_object()->add_alignment(x);
}

// For easily arming a monster with a weapon.
void arm(string wep, string wep2) {
  if (!wep || !wep2) return ;
  clone_object(wep)->move(this_object());
  command("wield "+wep2, this_object());
  return ;
}

void wear(string arm, string arm2) {
  if (!arm || !arm2) return ;
  clone_object(arm)->move(this_object());
  command("wear "+arm2, this_object());
  return ;
}

int query_level() { return query("level"); }
/* EOF */
