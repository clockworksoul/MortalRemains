/* Do not remove the headers from this file! see /USAGE for more info. */

inherit VERB_OB;

void do_wear_obj(object ob)
{
    if (!try_to_acquire(ob))
	return;

    ob->do_wear();
}

array query_verb_info()
{
    return ({ ({ "OBJ" }), ({ "put on" }) });
}
