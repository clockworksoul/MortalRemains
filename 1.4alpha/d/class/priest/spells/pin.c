//ilz

#include <magic.h>
#include <m_spells.h>
#include <mudlib.h>

#define COST 20
#define SYNTAX "Syntax: invoke pin at target\n" 
string help();

inherit SPELL;

int invoke_pin (string str) {
  object target, shield;
int flag, x;
 
 seteuid(getuid());

   if (!str) str = "NONE";
   if (str=="NONE") target = TP->query_current_attacker();
   if (!target) target = present(str, environment(TP));
   if (!target) {
      write(SYNTAX);
      return 0;
   }

   if (!check_valid_targ(TP, target)) return 0;
  
     if (!living(target)) {
        write("The target must be alive.\n");
        return 1;
      }
    
  if (present("#PIN#", target)) {
     write("They are already affected by pin!\n");
     return 0;
  }

  if (!TP->invoke_spell(COST)) {
    write(NOT_ENOUGH_SP);
    return 0;
  }
 
    if (!check_magic_resist((TP->query_level()),target,COST,0)) return;
 
 
  write ("You begin casting pin upon "+target->query("cap_name")+".\n");
 say(TPN+" begins casting pin upon "+target->query("cap_name")+".\n");
   x = target->save_throw(x);

      if (x) {
      write(target->query("cap_name")+" resists the spell.\n");
 say(target->query("cap_name")+" resists the spell.\n");
 target->kill_ob(TP);
     return 1;
     }
    write(target->query("cap_name")+" is pinned.\n");
    say(target->query("cap_name")+" is pinned.\n");    
  shield = clone_object("/d/class/priest/obj/pin_ob");
 
  if (!shield) {
    write("Your spell fizzles.  Contact Cyanide.\n");
    say(TPN+"'s spell fizzles...\n");
    return 0;
  }

  shield->move(target);
  shield->begin_spell(target, (TP->query_level()));
  target->kill_ob(TP);
  return 1;
}

string spellcraft() {
    return (@EndSpellcraft
Pin (Priest - Level III - Combat)

Duration: 5 sec/lvl 		Area of Effect: 1 Target
Cost: 20 TP

	This spell robs the target of movement for a
time.
EndSpellcraft
    );
}
