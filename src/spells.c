/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"


/* Global data */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct obj_data *object_list;



/* extern functions */
int     spell_lev(struct char_data *caster, int spell);
void	say_spell( struct char_data *ch, int si );
bool saves_spell(struct char_data *ch, short spell, int versus_lev);
int	add_follower(struct char_data *ch, struct char_data *victim);


ACASTN(cast_quench_thirst)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_quench_thirst(spell_no, level, ch, 0, tar_obj);
      break;
   case SPELL_TYPE_CANTRIP:
      spell_quench_thirst(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_SCROLL:
      spell_quench_thirst(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_WAND:
      spell_quench_thirst(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;
   case SPELL_TYPE_ROD:
      spell_quench_thirst(spell_no, 0-number(15,45), ch, 0, tar_obj);
      break;      
   default :
      logg("SYSERR: Serious screw-up in quench thirst!");
      break;
   }
}
ACASTN(cast_enchant1)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_enchant1(spell_no, level, ch, 0, tar_obj);
      break;
   case SPELL_TYPE_CANTRIP:
      spell_enchant1(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_SCROLL:
      spell_enchant1(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_WAND:
      spell_enchant1(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;
   case SPELL_TYPE_ROD:
      spell_enchant1(spell_no, 0-number(15,45), ch, 0, tar_obj);
      break;      
   default :
      logg("SYSERR: Serious screw-up in Edge of the Razor!");
      break;
   }
}
ACASTN(cast_detect_magic1)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_detect_magic1(spell_no, level, ch, 0, tar_obj);
      break;
   case SPELL_TYPE_CANTRIP:
      spell_detect_magic1(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_SCROLL:
      spell_detect_magic1(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_WAND:
      spell_detect_magic1(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;
   case SPELL_TYPE_ROD:
      spell_detect_magic1(spell_no, 0-number(15,45), ch, 0, tar_obj);
      break;      
   default :
      logg("SYSERR: Serious screw-up in Sense the Nature of Vis!");
      break;
   }
}
ACASTN(cast_detect_magic2)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_detect_magic2(spell_no, level, ch, 0, tar_obj);
      break;
   case SPELL_TYPE_CANTRIP:
      spell_detect_magic2(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_SCROLL:
      spell_detect_magic2(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_WAND:
      spell_detect_magic2(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;
   case SPELL_TYPE_ROD:
      spell_detect_magic2(spell_no, 0-number(15,45), ch, 0, tar_obj);
      break;      
   default :
      logg("SYSERR: Serious screw-up in Scales of the Magical Weight!");
      break;
   }
}
ACASTN(cast_detect_magic3)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_detect_magic3(spell_no, level, ch, 0, 0);
      break;
   case SPELL_TYPE_CANTRIP:
      spell_detect_magic3(spell_no, 0-number(0,30), ch, 0, 0);
      break;      
   case SPELL_TYPE_SCROLL:
      spell_detect_magic3(spell_no, 0-number(0,30), ch, 0, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_detect_magic3(spell_no, 0-number(0,30), ch, 0, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_detect_magic3(spell_no, 0-number(15,45), ch, 0, 0);
      break;      
   default :
      logg("SYSERR: Serious screw-up in Perceive Scent of Magic!");
      break;
   }
}
ACASTN(cast_feast_for_five)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_feast_for_five(spell_no, level, ch, 0, tar_obj);
      break;
   case SPELL_TYPE_CANTRIP:
      spell_feast_for_five(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_SCROLL:
      spell_feast_for_five(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_WAND:
      spell_feast_for_five(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;
   case SPELL_TYPE_ROD:
      spell_feast_for_five(spell_no, 0-number(15,45), ch, 0, tar_obj);
      break;      
   default :
      logg("SYSERR: Serious screw-up in feast for five men!");
      break;
   }
}
ACASTN(cast_walking_corpse)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_walking_corpse(spell_no, level, ch, 0, tar_obj);
      break;
   case SPELL_TYPE_CANTRIP:
      spell_walking_corpse(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_SCROLL:
      spell_walking_corpse(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;      
   case SPELL_TYPE_WAND:
      spell_walking_corpse(spell_no, 0-number(0,30), ch, 0, tar_obj);
      break;
   case SPELL_TYPE_ROD:
      spell_walking_corpse(spell_no, 0-number(15,45), ch, 0, tar_obj);
      break;      
   default :
      logg("SYSERR: Serious screw-up in the walking corpse!");
      break;
   }
}
ACASTN(cast_bind_wounds)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_bind_wounds(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_bind_wounds(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_bind_wounds(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_bind_wounds(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_bind_wounds(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_bind_wounds(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_bind_wounds(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_bind_wounds(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in bind wounds!");
      break;
   }
}
ACASTN(cast_lungs_ot_fish)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_lungs_ot_fish(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_lungs_ot_fish(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_lungs_ot_fish(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_lungs_ot_fish(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_lungs_ot_fish(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_lungs_ot_fish(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_lungs_ot_fish(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_lungs_ot_fish(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in lungs of the fish!");
      break;
   }
}
ACASTN(cast_dispel_invis)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_dispel_invis(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_dispel_invis(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_dispel_invis(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_dispel_invis(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_dispel_invis(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_dispel_invis(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_dispel_invis(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_dispel_invis(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in Restoration of the Lost Image!");
      break;
   }
}
ACASTN(cast_detect_invis)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_detect_invis(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_detect_invis(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_detect_invis(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_detect_invis(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_detect_invis(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_detect_invis(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_detect_invis(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_detect_invis(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in Discern Image of Truth and Falsehood!");
      break;
   }
}
ACASTN(cast_invis1)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_invis1(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_invis1(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_invis1(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_invis1(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_invis1(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_invis1(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_invis1(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_invis1(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in Invis of the Standing Wizard!");
      break;
   }
}
ACASTN(cast_invis2)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_invis2(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_invis2(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_invis2(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_invis2(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_invis2(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_invis2(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_invis2(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_invis2(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in Chamber of invisibility!");
      break;
   }
}
ACASTN(cast_invis3)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_invis3(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_invis3(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_invis3(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_invis3(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_invis3(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_invis3(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_invis3(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_invis3(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in Veil of Invisibility!");
      break;
   }
}

ACASTN(cast_gift_of_vigour)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   return;
      spell_gift_of_vigour(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_gift_of_vigour(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_gift_of_vigour(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_gift_of_vigour(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_gift_of_vigour(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_gift_of_vigour(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_gift_of_vigour(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_gift_of_vigour(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in gift of vigour!");
      break;
   }
}
ACASTN(cast_endurance)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
       spell_endurance(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_endurance(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_endurance(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_endurance(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_endurance(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_endurance(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_endurance(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_endurance(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in endurance of the beserker!");
      break;
   }
}
ACASTN(cast_healing_touch)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
       spell_healing_touch(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_healing_touch(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_healing_touch(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_healing_touch(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_healing_touch(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_healing_touch(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_healing_touch(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_healing_touch(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in healing touch!");
      break;
   }
}
ACASTN(cast_soothe_pains_ot_beast)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
       spell_soothe_pains_ot_beast(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_soothe_pains_ot_beast(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_soothe_pains_ot_beast(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_soothe_pains_ot_beast(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_soothe_pains_ot_beast(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_soothe_pains_ot_beast(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_soothe_pains_ot_beast(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_healing_touch(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in soothe the pains of the beast!");
      break;
   }
}
ACASTN(cast_body_made_whole)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
       spell_body_made_whole(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_body_made_whole(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_body_made_whole(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_body_made_whole(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_body_made_whole(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_body_made_whole(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_body_made_whole(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_body_made_whole(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in BODY_MADE_WHOLE!");
      break;
   }
}
ACASTN(cast_lightning_swordsman)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
       spell_lightning_swordsman(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_lightning_swordsman(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_lightning_swordsman(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_lightning_swordsman(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_lightning_swordsman(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_fortitude_ot_bear(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_lightning_swordsman(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      break;
   default :
      logg("SYSERR: Serious screw-up in Grip of the Lightning Swordsman!");
      break;
   }
}
ACASTN(cast_fortitude_ot_bear)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
       spell_fortitude_ot_bear(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_fortitude_ot_bear(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_fortitude_ot_bear(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_fortitude_ot_bear(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_fortitude_ot_bear(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_fortitude_ot_bear(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_fortitude_ot_bear(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      break;
   default :
      logg("SYSERR: Serious screw-up in fortitude of the bear!");
      break;
   }
}
ACASTN(cast_spasm)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
      spell_spasm(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_spasm(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_spasm(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_spasm(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_spasm(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_spasm(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_spasm(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_spasm(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in spasm of the trembling hand!");
      break;
   }
}
ACASTN(cast_restoration)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_restoration(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_restoration(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_restoration(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_restoration(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_restoration(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_restoration(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_restoration(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_restoration(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in restoration!");
      break;
   }
}
ACASTN(cast_blacksmith_might)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_blacksmith_might(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_blacksmith_might(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_blacksmith_might(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_blacksmith_might(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_blacksmith_might(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_blacksmith_might(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_blacksmith_might(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_blacksmith_might(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in blacksmith might!");
      break;
   }
}
ACASTN(cast_youthful_beauty)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_youthful_beauty(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_youthful_beauty(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_youthful_beauty(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_youthful_beauty(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_youthful_beauty(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_youthful_beauty(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_youthful_beauty(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_youthful_beauty(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in youthful beauty!");
      break;
   }
}
ACASTN(cast_nimble_cat)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_nimble_cat(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_nimble_cat(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_nimble_cat(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_nimble_cat(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_nimble_cat(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_nimble_cat(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_nimble_cat(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_nimble_cat(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in nimble cat!");
      break;
   }
}
ACASTN(cast_hearty_health)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_hearty_health(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_hearty_health(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_hearty_health(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	   victim = ch;
      spell_hearty_health(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_hearty_health(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_hearty_health(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_hearty_health(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_hearty_health(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in hearty health!");
      break;
   }
}
ACASTN(cast_breath_of_vigor)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (victim == ch)
	   return;
      spell_breath_of_vigor(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_breath_of_vigor(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_breath_of_vigor(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	 victim == ch;
      spell_breath_of_vigor(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_breath_of_vigor(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_breath_of_vigor(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_breath_of_vigor(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
	   victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_breath_of_vigor(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in breath of vigor!");
      break;
   }
}
ACASTN(cast_invok_ot_milky_eyes)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (victim == ch)
	   return;
      spell_invok_ot_milky_eyes(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_invok_ot_milky_eyes(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_invok_ot_milky_eyes(spell_no, 15-number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
       if (!victim)
	 victim == ch;
      spell_invok_ot_milky_eyes(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_SCROLL:
       if (!victim)
	   victim = ch;
      spell_invok_ot_milky_eyes(spell_no, 0-number(0,30), ch, victim, 0);
      break;      
   case SPELL_TYPE_WAND:
      spell_invok_ot_milky_eyes(spell_no, 0-number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      spell_invok_ot_milky_eyes(spell_no, 0-number(15,45), ch, victim, 0);
      break;      
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
	   victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_invok_ot_milky_eyes(spell_no,-15 - number(0,30), ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in incantation of the milky eyes!");
      break;
   }
}
ACASTN(cast_lamp_wo_flame)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	if (tar_obj) {
	  if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_GLOW) ){
	      send_to_char("Nothing new seems to happen.\r\n", ch);
	      break;}
	}
	spell_lamp_wo_flame(spell_no,level, ch, 0, tar_obj);
	break;
    case SPELL_TYPE_PHILTRE:
     break;
    case SPELL_TYPE_CANTRIP:
      if (tar_obj) {
	if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_GLOW)) )
	  spell_lamp_wo_flame(spell_no,0 - number(0,30), ch, 0, tar_obj);
      } 
     break;
    case SPELL_TYPE_POTION:
	break;
    case SPELL_TYPE_SCROLL:
	if (tar_obj) {
	    if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_GLOW)) )
	      spell_lamp_wo_flame(spell_no,0 - number(15,45), ch, 0, tar_obj);
	} 
	break;
    case SPELL_TYPE_WAND:
	if (tar_obj) {
	    if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_GLOW)) )
		spell_lamp_wo_flame(spell_no,0 - number(0,30), ch, 0, tar_obj);
	} 
	break;
    case SPELL_TYPE_ROD:
	if (tar_obj) {
	    if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_GLOW)) )
	      spell_lamp_wo_flame(spell_no,0 - number(15,45), ch, 0, tar_obj);
	} 
	break;	
    case SPELL_TYPE_STAFF:
	break;
    default :
	logg("SYSERR: Serious screw-up in lamp without flame!");
	break;
    }
}
ACASTN(cast_leap)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_leap(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_leap(spell_no,0 - number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_leap(spell_no,15 - number(0,30), ch, ch, 0);
      break;      
   case SPELL_TYPE_CANTRIP:
      if (tar_obj)
	 return;
      if (victim)
	  if (GET_LEVEL(victim) > LEVEL_BUILDER && victim != ch){
	      act("$N laughs at your feeble magic.",TRUE,ch,0,victim,TO_CHAR);
	      act("$n just tried to recall you..",TRUE,ch,0,victim,TO_VICT);
	      break;
	  }		  
      if (!victim)
	 victim = ch;
      spell_leap(spell_no, 0 - number(0,30) , ch, victim, 0);
      break;
   case SPELL_TYPE_SCROLL:
      if (tar_obj)
	 return;
      if (victim)
	  if (GET_LEVEL(victim) > LEVEL_BUILDER && victim != ch){
	      act("$N laughs at your feeble magic.",TRUE,ch,0,victim,TO_CHAR);
	      act("$n just tried to recall you..",TRUE,ch,0,victim,TO_VICT);
	      break;
	  }		  
      if (!victim)
	 victim = ch;
      spell_leap(spell_no, 0 - number(15,45) , ch, victim, 0);
      break;
   case SPELL_TYPE_WAND:
      if (tar_obj)
	 return;
      spell_leap(spell_no, 0 - number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      if (tar_obj)
	 return;
      spell_leap(spell_no, 0 - number(15,45), ch, victim, 0);
      break;
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_leap(spell_no,-15-number(0,30) , ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in Leap of Homecoming!");
      break;
   }
}
ACASTN(cast_seven_league)
{
   switch (type) {
   case SPELL_TYPE_SPELL:
       if (!victim)
	   victim = ch;
      spell_seven_league(spell_no,level, ch, victim, 0);
      break;
   case SPELL_TYPE_PHILTRE:
      spell_seven_league(spell_no,15 - number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_POTION:
      spell_seven_league(spell_no,0 - number(0,30), ch, ch, 0);
      break;
   case SPELL_TYPE_CANTRIP:
      if (tar_obj)
	 return;
      if (victim)
	  if (GET_LEVEL(victim) > LEVEL_BUILDER && victim != ch){
	      act("$N laughs at your feeble magic.",TRUE,ch,0,victim,TO_CHAR);
	      break;
	  }		  
      if (!victim)
	 victim = ch;
      spell_seven_league(spell_no, 0 - number(0,30) , ch, victim, 0);
      break;
   case SPELL_TYPE_SCROLL:
      if (tar_obj)
	 return;
      if (victim)
	  if (GET_LEVEL(victim) > LEVEL_BUILDER && victim != ch){
	      act("$N laughs at your feeble magic.",TRUE,ch,0,victim,TO_CHAR);
	      break;
	  }		  
      if (!victim)
	 victim = ch;
      spell_seven_league(spell_no, 0 - number(15,45) , ch, victim, 0);
      break;
   case SPELL_TYPE_WAND:
      if (tar_obj)
	 return;
      spell_seven_league(spell_no, 0 - number(0,30), ch, victim, 0);
      break;
   case SPELL_TYPE_ROD:
      if (tar_obj)
	 return;
      spell_seven_league(spell_no, 0 - number(15,45), ch, victim, 0);
      break;
   case SPELL_TYPE_STAFF:
      for (victim = world[ch->in_room].people ; 
          victim ; victim = victim->next_in_room)
	 if (victim != ch)
	    spell_seven_league(spell_no,-15-number(0,30) , ch, victim, 0);
      break;
   default :
      logg("SYSERR: Serious screw-up in Seven League Stride!");
      break;
   }
}
ACASTN(cast_gen_dam){
    switch(type){
    case SPELL_TYPE_SPELL:
	spell_gen_dam(spell_no, level, ch, victim, 0);
	break;
    case SPELL_TYPE_PHILTRE:
	spell_gen_dam(spell_no,15-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_gen_dam(spell_no,0-number(0,30), ch, victim, 0);
	break;	
    case SPELL_TYPE_CANTRIP:
	spell_gen_dam(spell_no,15-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	spell_gen_dam(spell_no,0-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_WAND:
	spell_gen_dam(spell_no,0-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_ROD:
	spell_gen_dam(spell_no,0-number(15,45), ch, victim, 0);
	break;	
    case SPELL_TYPE_STAFF:
      	spell_gen_dam(spell_no,0-number(30,60), ch, victim, 0);
	break;
    default:
	logg("SYSERR: Serious screw-up in general damage!");
	break;		
    }
    return;
}
ACASTN(cast_encase_in_ice){

    switch(type){
    case SPELL_TYPE_SPELL:
	spell_encase_in_ice(spell_no, level, ch, victim, 0);
	break;
    case SPELL_TYPE_PHILTRE:
	spell_encase_in_ice(spell_no,15-number(0,30), ch, ch, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_encase_in_ice(spell_no,0-number(0,30), ch, ch, 0);
	break;	
    case SPELL_TYPE_CANTRIP:
	spell_encase_in_ice(spell_no,15-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	spell_encase_in_ice(spell_no,0-number(0,30), ch, victim, 0);
	break;	
    case SPELL_TYPE_WAND:
	spell_encase_in_ice(spell_no,0-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_ROD:
	spell_encase_in_ice(spell_no,0-number(15,45), ch, victim, 0);
	break;
    case SPELL_TYPE_STAFF:
	spell_encase_in_ice(spell_no,0-number(30,60), ch, victim, 0);
	break;
    default:
	logg("SYSERR: Serious screw-up in encase in ice!");
	break;	
    }
    return;
}
ACASTN(cast_web){

    switch(type){
    case SPELL_TYPE_SPELL:
	spell_web(spell_no, level, ch, victim, 0);
	break;
    case SPELL_TYPE_PHILTRE:
	spell_web(spell_no,15-number(0,30), ch, ch, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_web(spell_no,0-number(0,30), ch, ch, 0);
	break;	
    case SPELL_TYPE_CANTRIP:
	spell_web(spell_no,15-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	spell_web(spell_no,0-number(0,30), ch, victim, 0);
	break;	
    case SPELL_TYPE_WAND:
	spell_web(spell_no,0-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_ROD:
	spell_web(spell_no,0-number(15,45), ch, victim, 0);
	break;
    case SPELL_TYPE_STAFF:
	spell_web(spell_no,0-number(30,60), ch, victim, 0);
	break;
    default:
	logg("SYSERR: Serious screw-up in web spell!");
	break;	
    }
    return;
}
ACASTN(cast_footstep_slippery){
    switch(type){
    case SPELL_TYPE_SPELL:
	spell_footstep_slippery(spell_no, level, ch, victim, 0);
	break;
    case SPELL_TYPE_PHILTRE:
    case SPELL_TYPE_POTION:      
	break;
    case SPELL_TYPE_CANTRIP:
	spell_footstep_slippery(spell_no,15-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	spell_footstep_slippery(spell_no,0-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_WAND:
	spell_footstep_slippery(spell_no,0-number(0,30), ch, victim, 0);
	break;
    case SPELL_TYPE_ROD:
	spell_footstep_slippery(spell_no,0-number(15,45), ch, victim, 0);
	break;
    case SPELL_TYPE_STAFF:
	spell_footstep_slippery(spell_no,0-number(30,60), ch, victim, 0);
	break;
    default:
	logg("SYSERR: Serious screw-up in slippery oil!");
	break;	
    }
    return;
}

