//ilz-2

#include <m_spells.h>              
#include <magic.h>
#include <mudlib.h>

inherit OBJECT;


void create() {
  seteuid(getuid());
  set("id", ({ "#PROT_VENOM#" }) );
  set("prevent_drop", 1);
  set("dest_at_die", 1);
}

int begin_spell(object caster) {
  set("level", caster->query_level());

  set("enchantment", DIM);
  set("schools", ({ "abjuration" }) );
  caster->resistance("poison", 50);
  
  call_out("remove", caster->query_level() * 30 );
  return 1;
}


int remove() {
  object caster = environment(this_object());

  if ((caster) && living(caster)) {
tell_object(caster, "Your Venom Protection spell ends.\n");
caster->resistance("poison", 200);
  }
  ::remove();
  return 1;
}                                                  

/* EOF */
