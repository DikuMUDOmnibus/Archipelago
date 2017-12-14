/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: actual effects of magical spells                                *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "interpreter.h"
#include "db.h"


/* extern structures */
extern short r_immort_start_room;
extern int top_of_world;
extern struct zone_data *zone_table;
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[];
extern int	pk_allowed;	/* see config.c */
extern int top_of_zone_table;
/* extern functions */
char *find_skill_name(int spl);
void    poison_vict(struct char_data *i);
void    reduce_ice(struct char_data *ch, int dam);
void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
bool    saves_spell(struct char_data *ch, short spell, int versus_lev);
void    weight_change_object(struct obj_data *obj, int weight);
void    damage_obj_equiped(struct char_data *vict,int where, int *spell_type);
void    damage_obj_invent(struct char_data *vict, struct obj_data *obj, int *spell_type);
void    damage_obj_corpse(struct obj_data *corpse, int spell_type, int dam);
void    spell_damage_equipment(struct char_data *ch, struct char_data *vict, int spell_no,int dam);
bool    aff_by_spell(struct char_data *k, int spell);
int     can_see_hidden(struct char_data *sub, struct char_data *obj);
char    *report_cost(int gold);
void    scrap_item(struct obj_data *obj);
int     calc_difficulty(struct char_data *ch, int number);

ASPELLN(spell_feast_for_five)
{
  int mult,lev;
  struct obj_data *tmp_obj;
  assert(ch);

  lev = spell_info[spell_no].min_level - level;   
  
  while (level < -SPELL_MULT){
    mult += 1;
    level += SPELL_MULT;
  }
  mult = MAX(1,mult);
  mult = MIN(12,mult);
  
  tmp_obj = read_object(1216, VIRTUAL, 1);
  
  obj_to_room(tmp_obj, ch->in_room, FALSE);
  
  act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
  act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
   
}

ASPELLN(spell_lungs_ot_fish)
{
  struct affected_type af;
  int lev,mult;
  
  
  assert(victim && ch);
  lev = spell_info[spell_no].min_level - level;
  
  while (level < -SPELL_MULT){
    mult += 1;
    level += SPELL_MULT;
  }
  mult = MAX(1,mult);
  mult = MIN(12,mult);
  
  if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			 IS_ANIMAL(victim) ||
			 (GET_RACE(victim) == CLASS_PLANT))){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  if (IS_AFFECTED(victim, AFF_WATER_BREATH)){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  
  af.type      = SPELL_LUNGS_OT_FISH;
  af.duration  = 12 + mult;
  af.modifier  = 0;
  af.location  = 0;
  af.level = lev;
  af.bitvector = AFF_WATER_BREATH;
  
  affect_join(victim, &af,TRUE,FALSE);
  send_to_char("The air suddenly feels thicker.\r\n", victim);
}
ASPELLN(spell_detect_invis)
{
  struct affected_type af;
  int lev,mult;
  
  lev = spell_info[spell_no].min_level - level;
  assert(victim && ch);
  
  while (level < -SPELL_MULT){
    mult += 1;
    level += SPELL_MULT;
  }
  mult = MAX(1,mult);
  mult = MIN(12,mult);
  
  if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			 IS_ANIMAL(victim) ||
			 (GET_RACE(victim) == CLASS_PLANT))){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  if (IS_AFFECTED(victim, AFF_DETECT_INVISIBLE)){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  
  af.type      = spell_no;
  af.duration  = 12 + mult;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = AFF_DETECT_INVISIBLE;
  af.level = lev;
  affect_join(victim, &af,TRUE,FALSE);
  af.modifier  = mult/3 +2;
  af.location  = APPLY_PER;
  affect_join(victim, &af,TRUE,FALSE);  
  send_to_char("You feel your perceptions heighten.\r\n", victim);
}
ASPELLN(spell_dispel_invis)
{
  struct affected_type *af;
  int lev,mult;
  bool success = FALSE;
  
  lev = spell_info[spell_no].min_level - level;
  assert(victim && ch);
  
  while (level < -SPELL_MULT){
    mult += 1;
    level += SPELL_MULT;
  }
  mult = MAX(1,mult);
  mult = MIN(12,mult);
  
  if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			 (GET_RACE(victim) == CLASS_PLANT))){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  if (!IS_AFFECTED(victim, AFF_INVISIBLE)){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  if ((af = affected_by_spell(victim, SPELL_INVIS1))
      && (af->level < lev)){
    success = TRUE;
    affect_from_char(victim, SPELL_INVIS1);    
  }
  else if ((af = affected_by_spell(victim, SPELL_INVIS2))
      && (af->level < lev)){
    success = TRUE;
    affect_from_char(victim, SPELL_INVIS2);
  }
  else if ((af = affected_by_spell(victim, SPELL_INVIS3))
      && (af->level < lev)){
    success = TRUE;
    affect_from_char(victim, SPELL_INVIS3);    
  }
  else if (GET_LEVEL(victim) < lev){
    success = TRUE;
    REMOVE_BIT(victim->specials.affected_by, AFF_INVISIBLE);
  }
  if (success){
    if (victim != ch){
      act("$N becomes visible.",FALSE,ch,0,victim,TO_CHAR);
      act("You feel exposed.",FALSE,ch,0,victim,TO_VICT);
    }
    else
      act("$N becomes visible.",FALSE,ch,0,victim,TO_NOTCHAR);
      act("You feel exposed.",FALSE,ch,0,victim,TO_CHAR);
  }
  else
    send_to_char("Your spell has no effect.\r\n",ch);
  return;  
}

ASPELLN(spell_invis1)
{
  struct affected_type af;
  int lev,mult;
  
  lev = spell_info[SPELL_INVIS1].min_level - level;
  assert(victim && ch);
  
  while (level < -SPELL_MULT){
    mult += 1;
    level += SPELL_MULT;
  }
  mult = MAX(1,mult);
  mult = MIN(12,mult);
  
  if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			 IS_ANIMAL(victim) ||
			 (GET_RACE(victim) == CLASS_PLANT))){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  if (IS_AFFECTED(victim, AFF_INVISIBLE)){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  
  af.type      = SPELL_INVIS1;
  af.duration  = 12 + mult;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = AFF_HIDE;
  af.level = lev;
  affect_join(victim, &af,TRUE,FALSE);
  send_to_char("You blend into the shadows.\r\n", victim);
}

ASPELLN(spell_invis2)
{
  /* group spell */
  struct follow_type *f=0;
  struct char_data *leader=0;

  assert(ch);

  leader = ch->master;
  
  if (!leader)
    leader = ch;
  
  for (f = leader->followers; f; f = f->next)
    if (f->follower)
      spell_invis1(spell_no,level, ch, f->follower, 0);
  spell_invis1(spell_no,level, ch, leader, 0);
}

ASPELLN(spell_invis3)
{
  struct affected_type af;
  int lev,mult;
  lev = spell_info[SPELL_INVIS3].min_level - level;  
  
  assert(victim && ch);
  
  while (level < -SPELL_MULT){
    mult += 1;
    level += SPELL_MULT;
  }
  mult = MAX(1,mult);
  mult = MIN(12,mult);
  
  if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			 IS_ANIMAL(victim) ||
			 (GET_RACE(victim) == CLASS_PLANT))){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  if (IS_AFFECTED(victim, AFF_INVISIBLE)){
    send_to_char("Your spell has no effect.\r\n",ch);
    return;
  }
  
  af.type      = SPELL_INVIS3;
  af.duration  = 12 + mult;
  af.modifier  = 0;
  af.location  = 0;
  af.level = lev;
  af.bitvector = AFF_INVISIBLE;
  
  affect_join(victim, &af,TRUE,FALSE);
  send_to_char("You fade into invisibility.\r\n", victim);
  act("$n fades into invisibility.",TRUE,ch,0,0,TO_NOTCHAR);
}

ASPELLN(spell_gift_of_vigour)
{
    int mult, need, has;
    assert(ch && victim);
    
    while (level < -SPELL_MULT){
	mult += 1;
	level += SPELL_MULT;
    }
    mult = MAX(1,mult);
    mult = MIN(4,mult);
    
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
    need = GET_MAX_MOVE(victim) - GET_MOVE(victim);
    has = (mult*GET_MOVE(ch))/4 ;

    if (has > need)
      has = need;
    
    if (has){
	GET_MOVE(ch) -= has;
	GET_MOVE(victim) += has;
	act("$n staggers as energy flows from $m",FALSE,ch,0,0,TO_ROOM);
	act("$n seems invigorated",FALSE,victim,0,0,TO_ROOM);
	act("You feel drained.",FALSE,ch,0,0,TO_CHAR);
	act("You feel energised.",FALSE,victim,0,0,TO_CHAR);
    }
    else
	send_to_char("Nothing seems to happen.\r\n",ch);
}
ASPELLN(spell_walking_corpse)
{
    ACMD(do_wear);
    ACMD(do_wield);
    void get_check_money(struct char_data *ch, struct obj_data *obj);
    int add_follower(struct char_data *ch, struct char_data *leader);
    struct affected_type af;
    struct char_data *mob;
    struct obj_data *tmp, *next=0;
    int size, mult = 1,lev;
    assert(ch && obj);

    lev = spell_info[spell_no].min_level - level;

    while (level < -SPELL_MULT){
	mult +=1;
	level += SPELL_MULT;
    }
    if (!obj){
      send_to_char("Your target has disappeard.\r\n",ch);
      return;
    }
    if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER){
       send_to_char("That is not a corpse.\r\n",ch);
       return;
    }
    else if(obj->obj_flags.value[3] !=  -1){
	send_to_char("That is not a corpse.\r\n",ch);
	return;
    }
    if (obj->obj_flags.value[4] < 20){
      send_to_char("Your spell fails.\r\n",ch);
      return;
    }      
    
    if (obj->carried_by)
      obj_from_char(obj,0);
    else if (obj->in_obj)
      obj_from_obj(obj);
    else
      obj_from_room(obj);
    if (obj->obj_flags.value[2] < 0)
      {
	if (!(mob = read_mobile(obj->obj_flags.value[7], VIRTUAL))){
	  send_to_char("Your spell fails.\r\n",ch);
	  return;
	}
      }
    else {
	if (!(mob = read_mobile(1206, VIRTUAL))){
	  send_to_char("Your spell fails.\r\n",ch);
	  return;
	}
    }
    CREATE(mob->player.short_descr, char, sizeof(char)*(strlen(obj->short_description) + 1));
    strcpy(mob->player.short_descr, obj->short_description);
    sprintf(buf,"%s zombie",obj->name);
    CREATE(mob->player.name, char, sizeof(char)*(strlen(buf) + 1));
    strcpy(mob->player.name, buf);
    sprintf(buf,"%s is here, mindlessly wandering about.\r\n", obj->short_description);
    CREATE(mob->player.long_descr, char, sizeof(char)*(strlen(buf) + 1));
    strcpy(mob->player.long_descr, CAP(buf));
    CREATE(mob->player.description, char, sizeof(char)*(strlen("   The zombie is pretty gruesome with bits of skin dropping off.\r\n") + 1));
    strcpy(mob->player.description, "The zombie is pretty gruesome with bits of skin dropping off.\r\n");
    GET_RAW_INT(mob) = 0;
    GET_INT(mob) = 0;
    GET_EXP(mob) = 0;
    GET_RACE(mob) = CLASS_UNDEAD;
    if (obj->obj_flags.value[2] >= 0) {
      GET_LEVEL(mob) = mult*4;
      mob->points.max_hit = mult*number(10,50);
      mob->points.hit = mob->points.max_hit;
      mob->points.max_move = mob->points.max_hit;
      mob->points.move = mob->points.max_hit;
      mob->specials.damnodice = MAX(2,MIN(8,mult/2));
      mob->specials.damsizedice = MAX(2,MIN(5,mult/2));
      size = GET_OBJ_SIZE(obj);
      switch(size)
	{
	case 6:
	  mob->player.height = 99;
	  mob->player.weight = 400;	
	  break;
	case 5:
	  mob->player.height = 134;
	  mob->player.weight = 900;	
	  break;
	case 4:
	  mob->player.height = 160;
	  mob->player.weight = 1500;	
	  break;
	case 3:
	  mob->player.height = 180;
	  mob->player.weight = 2000;	
	  break;
	case 2:
	  mob->player.height = 210;
	  mob->player.weight = 2500;	
	  break;
	case 1:
	  mob->player.height = 225;
	  mob->player.weight = 3000;	
	  break;
	default:
	  mob->player.height = 180;
	  mob->player.weight = 2000;	
	  break;
	}
    }
    else {
      GET_LEVEL(mob) *= mult;
      mob->points.max_hit *= mult;
      mob->points.max_move *= mult;
      mob->specials.damnodice *= mult;
      mob->specials.damsizedice *= mult;
      GET_LEVEL(mob) /= 6;
      mob->points.max_hit /= 6;
      mob->points.max_move /= 6;
      mob->specials.damnodice /= 6;
      mob->specials.damsizedice /= 6;
      mob->points.hit = mob->points.max_hit;
      mob->points.move = mob->points.max_move;
    }
    GET_GOLD(mob) = 0;
    if (IS_SET(mob->specials2.act, MOB_SPEC))
      REMOVE_BIT(mob->specials2.act, MOB_SPEC);
    if (MOB_FLAGGED(mob,MOB_CITIZEN))
      REMOVE_BIT(MOB_FLAGS(mob), MOB_CITIZEN);
    if (MOB_FLAGGED(mob,MOB_MOODS))
      REMOVE_BIT(MOB_FLAGS(mob), MOB_MOODS);
    if (MOB_FLAGGED(mob,MOB_SPELL_CASTER))
      REMOVE_BIT(MOB_FLAGS(mob), MOB_SPELL_CASTER);            
    if (MOB_FLAGGED(mob,MOB_MOODS))
      	REMOVE_BIT(MOB_FLAGS(mob), MOB_MOODS);      

    af.type = SPELL_WALKING_CORPSE;
    af.duration = mult*2;
    af.location = 0;
    af.modifier = 0;
    af.level = lev;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob,&af);
    GET_ALIGNMENT(ch) -= number(10,100);
    GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch));
    GET_ALIGNMENT(mob) = GET_ALIGNMENT(ch);
    
    if ((mob->inventory =  obj->contains))
	{
	    object_list_new_owner(mob->inventory, mob);
	    for (tmp = mob->inventory; tmp;tmp = next){
		next = tmp->next_content;
		tmp->in_obj = 0;
		get_check_money(mob, tmp);
	    }
	    do_wear(mob,"all",0,0);
	    do_wield(mob,"weapon",0,0);
	}
    obj->contains = 0;
    char_to_room(mob, ch->in_room, FALSE);
    act("$p animates.",FALSE,ch,obj,0,TO_ROOM);
    act("$p animates.",FALSE,ch,obj,0,TO_CHAR);    
    add_follower(mob,ch);
    extract_obj(obj,0);    
}

ASPELLN(spell_quench_thirst)
{
   void	name_to_drinkcon(struct obj_data *obj, int type);
   void	name_from_drinkcon(struct obj_data *obj);
   int	water,lev;

   assert(ch && obj);

   lev = 1;
   while (level < -SPELL_MULT){
       lev +=1;
       level += SPELL_MULT;
   }
   act("A small thunderstorm forms over $p.", FALSE, ch, obj, 0, TO_CHAR);
   act("A small thunderstorm forms over $p.", FALSE, ch, obj, 0, TO_ROOM);     
   if ((GET_ITEM_TYPE(obj) == ITEM_DRINKCON) || 
       (GET_ITEM_TYPE(obj) == ITEM_FOUNTAIN)) {
      if ((obj->obj_flags.value[2] != LIQ_WATER)
           && (obj->obj_flags.value[1] != 0)) {
	 obj->obj_flags.value[2] = LIQ_SLIME;
	 name_to_drinkcon(obj, LIQ_SLIME);
      } else {
	 water = MAX(100,5*level*((weather_info.sky >= SKY_RAINING) ? 2 : 1));
	 /* Calculate water it can contain, or water created */
	 water = MIN(obj->obj_flags.value[0] - obj->obj_flags.value[1], water);

	 if (water > 0) {
	    obj->obj_flags.value[2] = LIQ_WATER;
	    obj->obj_flags.value[1] += water;

	    weight_change_object(obj, water);

	    name_to_drinkcon(obj, LIQ_WATER);
	    act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
	    act("$p is filled.", FALSE, ch, obj, 0, TO_ROOM);	    
	 }
      }
   } /* if itemtype == DRINKCON */ else{
       act("$p gets wet!", FALSE, ch, obj, 0, TO_CHAR);
       act("$p gets wet!", FALSE, ch, obj, 0, TO_ROOM);
   }
}

ASPELLN(spell_enchant1)
{
   int	i,j,ii,lev, bonus, totpoints, tohitdam=0; 

   bool full, dam_bon, ok = TRUE;

   assert(ch && obj);

   lev = 1;
   while (level < -SPELL_MULT){
       lev +=1;
       level += SPELL_MULT;
   }
   bonus = MAX(1, lev/3);
   
   if (GET_ITEM_TYPE(obj) != ITEM_WEAPON)
     ok = FALSE;
   else if (!((obj->obj_flags.value[3] == 2) ||
	    (obj->obj_flags.value[3] == 3) ||
	    (obj->obj_flags.value[3] == 5) ||
	    (obj->obj_flags.value[3] == 6)))
     ok = FALSE;
   
   if (ok && (assess_item(obj) > 50))
     ok = FALSE;
   if (ok){
     dam_bon = FALSE;   
     for (i = 0;i< MAX_OBJ_AFFECT; i++){
       if ((obj->affected[i].location == APPLY_DAMROLL)){
	 if (obj->affected[i].modifier > 0)
	   ok = FALSE;
	 else
	   dam_bon = TRUE;
	 break;
       }
     }
     full = TRUE;
     if (!dam_bon)
       for (ii = 0; ii < MAX_OBJ_AFFECT; ii++)
	 if (obj->affected[ii].location == 0){
	   i = ii;
	   full = FALSE;
	   break;
	 }
   }
   if (!ok || (full && !dam_bon)){
     send_to_char("Nothing seems to happen.\r\n",ch);
     return;
   }
   
   for (j = 0; j < MAX_OBJ_AFFECT; j++)
     if (obj->affected[j].modifier) {
       switch (obj->affected[j].location){
       case APPLY_HITROLL:
       case APPLY_HITROLL2:		  
       case APPLY_DAMROLL:
       case APPLY_DAMROLL2:		  
	 tohitdam += obj->affected[j].modifier;
       default:
	 break;
       }
     }
   
   while ((bonus + tohitdam) > 7)
     bonus--;

   totpoints = assess_item(obj);
   
   while ((bonus*12 + totpoints) > 125)
     bonus--;

   obj->obj_flags.cost = (bonus*12 + totpoints)*100;
   obj->obj_flags.cost_per_day = (bonus*12 + totpoints)*10;
   
   if (!bonus){
     send_to_char("Nothing seems to happen.\r\n",ch);
     return;
   }   
   if (obj->worn_by && dam_bon)
     GET_DAMROLL(obj->worn_by) -= obj->affected[i].modifier;
   obj->affected[i].modifier += number(1,bonus);
   if (obj->worn_by && dam_bon)
     GET_DAMROLL(obj->worn_by) += obj->affected[i].modifier;
   if (!dam_bon)
     obj->affected[i].location = APPLY_DAMROLL;   
   act("$p seems sharper.",FALSE,ch,obj,0,TO_ROOM);
   act("$p seems sharper.",FALSE,ch,obj,0,TO_CHAR);   
}

ASPELLN(spell_detect_magic1)
{

  int lev, mult=1;
  char *name=0;
  
  assert(ch && obj);
  lev = spell_info[spell_no].min_level - level;   
  
  while (level < -SPELL_MULT){
    mult +=1;
    level += SPELL_MULT;
  }
  if (GET_ITEM_TYPE(obj) != ITEM_VIS){
    act("You detect no raw Vis in $p.",FALSE,ch,obj,0, TO_CHAR);
    return;
  }
  if (obj->obj_flags.value[1]){
    name = find_skill_name(obj->obj_flags.value[1]);
    if (name)
      sprintf(buf, "You detect that $p contains some raw %s Vis.",name);
    act(buf,FALSE,ch,obj,0,TO_CHAR);
    mult -= 3;
  }
  if (obj->obj_flags.value[2]){
    if (mult > 0){
      name = find_skill_name(obj->obj_flags.value[2]);
      if (name)
	sprintf(buf, "You detect that $p contains some raw %s Vis.",name);
      act(buf,FALSE,ch,obj,0,TO_CHAR);
    }
    else{
      act("You detect a specific type of magic form Vis in $p",
	  FALSE,ch,obj,0,TO_CHAR);
      send_to_char("But you are unable to determine which Form.\r\n",ch);
	}
  }
}
ASPELLN(spell_detect_magic2)
{

  int lev, mult=1, pawns;
  char *name=0;
  
  assert(ch && obj);
  lev = spell_info[spell_no].min_level - level;   
  pawns = obj->obj_flags.value[3];

  while (level < -SPELL_MULT){
    mult +=1;
    level += SPELL_MULT;
  }
  if ((GET_ITEM_TYPE(obj) != ITEM_VIS) || (pawns == 0)){
    act("You detect no raw Vis in $p.",FALSE,ch,obj,0, TO_CHAR);
    return;
  }
  sprintf(buf, "You detect that $p contains %d pawns of raw Vis.",obj->obj_flags.value[3]);
  act(buf,FALSE,ch,obj,0,TO_CHAR);

}
ASPELLN(spell_detect_magic3)
{
  struct affected_type af;
  int lev, mult=1;
  char *name=0;
  
  assert(ch);
  lev = spell_info[spell_no].min_level - level;   
  
  while (level < -SPELL_MULT){
    mult +=1;
    level += SPELL_MULT;
  }

  af.type      = spell_no;
  af.duration  = mult;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = AFF_DETECT_MAGIC;
  af.level = lev;
  affect_join(ch, &af,TRUE,FALSE);

  send_to_char("You feel your nose twitch.\r\n", ch);

}
ASPELLN(spell_bind_wounds)
{
   int	healpoints, mult= 1;

   assert(ch && victim);
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(1,mult);
   mult = MIN(10,mult);
   
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   healpoints = (dice(1, 4))*mult;

   if ( (healpoints + GET_HIT(victim)) > GET_MAX_HIT(victim))
      GET_HIT(victim) = GET_MAX_HIT(victim);
   else
      GET_HIT(victim) += healpoints;

   update_pos( victim );

   send_to_char("You feel better!\r\n", victim);
}
ASPELLN(spell_breath_of_vigor)
{
   int	healpoints, mult= 1;

   assert(ch && victim);
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(1,mult);
   mult = MIN(15,mult);
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   
   healpoints = 15*mult;
   

   if ( (healpoints + GET_MOVE(victim)) > GET_MAX_MOVE(victim))
      GET_MOVE(victim) = GET_MAX_MOVE(victim);
   else
      GET_MOVE(victim) += healpoints;

   update_pos( victim );

   send_to_char("You feel a refreshing breeze blow across your skin.\r\n", victim);
   send_to_char("You feel invigorated!\r\n",victim);
   act("$n seems less tired.",FALSE,victim,0,0,TO_NOTCHAR);
}

ASPELLN(spell_invok_ot_milky_eyes)
{
   struct affected_type af;
   int mult = 1,lev;

   assert(ch && victim);
   lev = spell_info[SPELL_MILKY_EYES].min_level - level;
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(1,mult);
   mult = MIN(15,mult);
   
   if (saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch)) ||
       affected_by_spell(victim, SPELL_MILKY_EYES) ||
       (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT)))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   
   act("$n's eyes turn milky white!", TRUE, victim, 0, 0, TO_ROOM);
   send_to_char("You have been blinded!\r\n", victim);
   
   af.type      = SPELL_MILKY_EYES;
   af.location  = APPLY_HITROLL;
   af.modifier  = -5;  /* Make hitroll worse */
   af.duration  = mult;
   af.level = lev;
   af.bitvector = AFF_BLIND;
   affect_to_char(victim, &af);


   af.location = APPLY_ALL_AC;
   af.modifier = + 50; /* Make AC Worse! */
   affect_to_char(victim, &af);
}

ASPELLN(spell_soothe_pains_ot_beast)
{
   int	healpoints, mult= 1,lev;
   struct affected_type af;
   
   assert(ch && victim);
   lev = spell_info[SPELL_SOOTHE_PAINS_OT_BEAST].min_level - level;
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   if (!IS_NPC(victim) || !IS_ANIMAL(victim)){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (IS_AFFECTED(victim, AFF_HEALING) || victim->specials.fighting) {
     send_to_char("Nothing seems to happen.\r\n",ch);
     return;
   }
   mult = MAX(mult, 1);
   mult = MIN(mult, 4);
   af.type = SPELL_SOOTHE_PAINS_OT_BEAST;
   af.duration = -1;
   af.modifier = mult +1;
   af.location = APPLY_CON;
   af.level = lev;
   af.bitvector = AFF_HEALING;
   affect_join(victim, &af, TRUE, TRUE);
   
   healpoints = (GET_MAX_HIT(victim) - GET_HIT(victim));
   healpoints *= mult;
   healpoints /= 20;   

   if ( (healpoints + GET_HIT(victim)) > GET_MAX_HIT(victim))
      GET_HIT(victim) = GET_MAX_HIT(victim);
   else
      GET_HIT(victim) += healpoints;

   update_pos( victim );
   send_to_char("You feel better!\r\n", victim);
}
ASPELLN(spell_healing_touch)
{
   int	healpoints, mult= 1, lev;
   struct affected_type af;
   
   assert(ch && victim);
   lev = spell_info[SPELL_HEALING_TOUCH].min_level - level;   
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (IS_AFFECTED(victim, AFF_HEALING) || victim->specials.fighting) {
     send_to_char("Nothing seems to happen.\r\n",ch);
     return;
   }
   mult = MAX(mult, 1);
   mult = MIN(mult, 4);
   af.type = SPELL_HEALING_TOUCH;
   af.duration = -1;
   af.modifier = mult +1;
   af.location = APPLY_CON;
   af.level = lev;
   af.bitvector = AFF_HEALING;
   affect_join(victim, &af, TRUE, TRUE);
   
   healpoints = (GET_MAX_HIT(victim) - GET_HIT(victim));
   healpoints *= mult;
   healpoints /= 20;   

   if ( (healpoints + GET_HIT(victim)) > GET_MAX_HIT(victim))
      GET_HIT(victim) = GET_MAX_HIT(victim);
   else
      GET_HIT(victim) += healpoints;

   update_pos( victim );

   send_to_char("You feel better!\r\n", victim);
}
ASPELLN(spell_body_made_whole)
{
   int	healpoints, mult= 1, lev;
   struct affected_type af;
   
   assert(ch && victim);
   lev = spell_info[SPELL_BODY_MADE_WHOLE].min_level - level;   
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (IS_AFFECTED(victim, AFF_HEALING) || victim->specials.fighting) {
     send_to_char("Nothing seems to happen.\r\n", ch);
     return;
   }

   mult = MAX(mult, 1);
   mult = MIN(mult, 4);
   af.type = SPELL_BODY_MADE_WHOLE;
   af.duration = mult +1;
   af.modifier = mult +1;
   af.location = APPLY_CON;
   af.bitvector = AFF_HEALING;
   af.level = lev;
   affect_join(victim, &af, TRUE, TRUE);
   
   healpoints = (GET_MAX_HIT(victim) - GET_HIT(victim));
   healpoints /= 5;   

   if ( (healpoints + GET_HIT(victim)) > GET_MAX_HIT(victim))
      GET_HIT(victim) = GET_MAX_HIT(victim);
   else
      GET_HIT(victim) += healpoints;

   update_pos( victim );

   send_to_char("You feel better!\r\n", victim);
}
ASPELLN(spell_lightning_swordsman)
{
   struct affected_type af;
   int mult=1,lev;

   assert(ch && victim);
   lev = spell_info[SPELL_LIGHTNING_SWORDSMAN].min_level - level;
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(mult, 1);
   mult = MIN(mult, 6);

   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if ((!victim->specials.fighting) && 
       (!affected_by_spell(victim, SPELL_LIGHTNING_SWORDSMAN))) {
     
     send_to_char("You feel your sword arms limber up.\r\n", victim);
     af.type      = SPELL_LIGHTNING_SWORDSMAN;
     af.duration  = 1 + mult;
     af.modifier  = 1;
     af.location  = APPLY_HITROLL;
     af.bitvector = 0;
     af.level = lev;
     affect_to_char(victim, &af);
     af.location  = APPLY_HITROLL2;
     affect_to_char(victim, &af);
     
   }
   else 
     send_to_char("Nothing seems to happen.\r\n",ch);
}

ASPELLN(spell_fortitude_ot_bear)
{
   int	mult= 1, lev;
   struct affected_type af;
   
   assert(ch && victim);
   lev = spell_info[SPELL_FORTITUDE_BEAR].min_level - level;   
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (affected_by_spell(victim, SPELL_FORTITUDE_BEAR)) {
     send_to_char("Nothing seems to happen\r\n",ch);
     return;
   }

   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(mult, 1);
   mult = MIN(mult, 4);
   af.type = SPELL_FORTITUDE_BEAR;
   af.duration = number(2,6) + mult;
   af.modifier = mult;
   af.location = APPLY_ALL_STOPPING;
   af.bitvector = 0;
   af.level = lev;
   affect_join(victim, &af, TRUE, TRUE);
   
   send_to_char("Your skin toughens!\r\n", victim);
   act ("$n's skin seems thicker", FALSE, victim,0,0,TO_ROOM);
}
ASPELLN(spell_endurance)
{
   int	mult= 1, lev;
   struct affected_type af;
   
   assert(ch && victim);
   lev = spell_info[spell_no].min_level - level;   
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(1,mult);
   mult = MIN(10,mult);
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   af.type = SPELL_ENDURANCE;
   af.duration = mult/2 + 1;
   af.bitvector = 0;
   af.level = lev;   
   af.location = APPLY_NONE;
   affect_to_char(victim, &af);

   update_pos( victim );

   send_to_char("Raw power surges through your body!\r\n", victim);
   act("$n roars as pure energy courses through $s veins.", FALSE,victim, 0,0,TO_ROOM);
}
ASPELLN(spell_restoration)
{
   int	mult= 1, lev;
   struct affected_type *af, *next;

   assert(ch && victim);
   
   lev = spell_info[SPELL_RESTORATION].min_level - level;
   
   if (victim->specials.fighting) {
     send_to_char("Nothing seems to happen.\r\n",ch);
     return;
   }
   
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(mult, 1);
   mult = MIN(mult, 10);
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (victim){
     if (victim != ch) {
       act("$n's lays $s hands on $N's head.",TRUE,ch,0,victim,TO_NOTVICT);
       act("$N's lays $s hands on your head.",TRUE,victim,0,ch,TO_CHAR);
       act("You lay your hands on $N's head.",TRUE,ch,0,victim,TO_CHAR);
     }
     else {
       act("$n's touches $s hands to $s temples.",TRUE,ch,0,victim,TO_NOTVICT);
       act("You touch your hands to your temples.",TRUE,victim,0,ch,TO_CHAR);
     }
     for (af = victim->affected;af;af = next){
       next = af->next;
       if (af->level > lev)
	 continue;
       if (af->type == SPELL_SPASMS){
	 affect_remove(victim,af);
	 act("$n's hands stop shaking.",TRUE,victim,0,0,TO_ROOM);
	 send_to_char("Your hands stop shaking.\r\n", victim);
       }
       if (af->type == SPELL_MILKY_EYES){
	 if (af->location == APPLY_HITROLL){
	   if (mult > af->duration)
	     affect_remove(victim,af);
	   continue;
	 }
	 if (mult > af->duration){
	   act("$n's eyes become clear again.",FALSE,victim,0,0,TO_ROOM);
	   send_to_char("You can see once more.\r\n",victim);
	   affect_remove(victim,af);
	 }
	 else {
	   act("$n's eyes become clear briefly.",FALSE,victim,0,0,TO_ROOM);
	   act("$n's eyes cloud over.",FALSE,victim,0,0,TO_ROOM);
	   send_to_char("You can see for a brief instant.\r\n",victim);
	   send_to_char("Then darkness descends once more.\r\n",victim);
	 }
       }
       else if (af->bitvector == AFF_SPELLBOTCH){
	 affect_remove(victim,af);
	 act("$n seems more hale.",TRUE,victim,0,0,TO_ROOM);
	 send_to_char("You feel your health return.\r\n", victim);
       }
       else if (af->bitvector == AFF_CONFUSION){
	 affect_remove(victim,af);
	 act("$n seems less dazed.",TRUE,victim,0,0,TO_ROOM);
	 send_to_char("You feel your faculties clear.\r\n", victim);
       }   
       else if (af->type == SPELL_POISON  && af->location == APPLY_NONE){
	 if (mult < af->modifier){
	   af->modifier -= mult;
	   act("$n seems less sick.",TRUE,victim,0,0,TO_ROOM);
	   send_to_char("The burning in your veins lessens.\r\n", victim);}
	 else{
	   affect_remove(victim,af);
	   act("$n is restored to health.",TRUE,victim,0,0,TO_ROOM);
	   send_to_char("You feel better!\r\n", victim);
	 }
       }
     }
     update_pos( victim );
   }
}

ASPELLN(spell_encase_in_ice)
{
   struct affected_type af;
   int mult=1, lev;
   assert(victim && ch);

   lev = spell_info[SPELL_ENCASE_IN_ICE].min_level - level;
   
   while (level < -SPELL_MULT){
     mult += 1;
     level += SPELL_MULT;
   }
   if (!IS_NPC(ch) && !IS_NPC(victim)){
     send_to_char("You cannot encase players.\r\n",ch);
     return;
   }
   mult = MIN(10, mult);
   if (!saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch))){
     af.type      = SPELL_ENCASE_IN_ICE;
     af.location  = APPLY_NONE;
     af.modifier  = 0;
     af.level = lev;
     af.duration  = MAX(1,2*mult);
     af.bitvector = AFF_PARALYSIS;	
     
     act("A thick covering of ice starts to grow on $N's skin.",
	 TRUE,ch,0,victim,TO_ROOM);
     act("A thick covering of ice starts to grow on $N's skin.",
	 TRUE,ch,0,victim,TO_CHAR);
     act("A thick covering of ice starts to grow on your skin.",
	 TRUE,ch,0,victim,TO_VICT);
     if (IS_AFFECTED(ch, AFF_RESIST_COLD)
	 || (!number(0,10) &&  ((number(0,31) + 2*mult)  < GET_STR(victim)))){
       act("$N shatters the ice.",TRUE,ch,0,victim, TO_ROOM);
       act("$N shatters the ice.",TRUE,ch,0,victim, TO_CHAR);
       act("You shatter the ice.",TRUE,ch,0,victim, TO_VICT);
       act("$n becomes enraged.",FALSE,victim,0,0,TO_ROOM);
       damage(ch,victim,0,SPELL_ENCASE_IN_ICE, -1,1);
     }
     else{
       act("$N is encased in ice.",TRUE,ch,0,victim, TO_ROOM);
       act("$N is encased in ice.",TRUE,ch,0,victim, TO_CHAR);
       act("You are encased in the ice. You can't move!",TRUE,ch,0,victim, TO_VICT);
       remember(victim, ch);
       affect_to_char(victim, &af);
     }
	
   }
}
ASPELLN(spell_web)
{
  struct affected_type af;
  int mult=1, lev;
  assert(victim && ch);

   if (!IS_NPC(ch) && !IS_NPC(victim)){
     send_to_char("You cannot encase players.\r\n",ch);
     return;
   }
  lev = spell_info[SPELL_WEB].min_level - level;  
  while (level < -SPELL_MULT){
    mult += 1;
    level += SPELL_MULT;
  }
  
  mult = MIN(15, mult);
  if (!saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch))){
    act("$N is entangled in thick webs.",TRUE,ch,0,victim,TO_ROOM);
    act("$N is entangled in thick webs.",TRUE,ch,0,victim,TO_CHAR);
    act("You are entangled in thick webs.",TRUE,ch,0,victim,TO_VICT);
    if ((number(1,31) + mult)  < GET_STR(victim)){
      act("$N rips the webs to shreds.",TRUE,ch,0,victim, TO_ROOM);
      act("$N rips the webs to shreds.",TRUE,ch,0,victim, TO_CHAR);
      act("You easily tear free.",TRUE,ch,0,victim, TO_VICT);
    }
    else {
      af.type      = SPELL_WEB;
      af.location  = APPLY_DEX;
      af.modifier  = MIN(-3,-2*(mult/2));
      af.duration  = MAX(1,mult);
      af.level = lev;
      af.bitvector = AFF_PARALYSIS;
      affect_to_char(victim, &af);
      af.location  = APPLY_ALL_AC;
      af.modifier  = 9 + 2*mult;
      remember(victim, ch);      
      affect_to_char(victim, &af);
      return;
    }
  }
  act("$n becomes enraged.",FALSE,victim,0,0,TO_ROOM);
  damage(ch,victim,0,SPELL_WEB, -1,1);
  
}

ASPELLN(spell_footstep_slippery)
{
    struct affected_type af;
    int mult=1, lev;
    assert(victim && ch);

   lev = spell_info[SPELL_FOOTSTEPS_OT_SLIPPERY].min_level - level;    
    while (level < -SPELL_MULT){
	mult += 1;
	level += SPELL_MULT;
    }
    if (!number(0,20))
	{
	    act("$n becomes enraged.",FALSE,victim,0,0,TO_ROOM);
	    damage(ch,victim,0,SPELL_SPASMS, -1,1);
	}
    mult = MIN(10, mult);
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (IS_AFFECTED(victim, AFF_FLY)){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   } 
       
   if (!saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch))
       && !IS_AFFECTED(victim, AFF_SLIPPY)){
     af.type      = SPELL_FOOTSTEPS_OT_SLIPPERY;
     af.location  = APPLY_HITROLL;
     af.modifier  = 0 - mult;  /* Make hitroll worse */
     af.duration  = 1;
     af.level = lev;
     af.bitvector = AFF_SLIPPY;	
     affect_to_char(victim, &af);
     af.location = APPLY_DEX;
     af.modifier = (0 - mult)/2; 
     affect_to_char(victim, &af);
     act("$n creates some slippy oil under $N's feet.",TRUE,ch,0,victim,TO_NOTVICT);
     act("You create some slippy oil under $N's feet.",TRUE,ch,0,victim,TO_CHAR);
     act("You slip on some oil created by $n",TRUE,ch,0,victim,TO_VICT);
   }
   if (victim->specials.fighting == ch &&
       !saves_spell(ch, SAVING_SPELL, GET_LEVEL(ch))
       && !IS_AFFECTED(ch, SPELL_FOOTSTEPS_OT_SLIPPERY)){
     af.type      = SPELL_FOOTSTEPS_OT_SLIPPERY;
     af.location  = APPLY_HITROLL;
     af.modifier  = 0 - mult;  /* Make hitroll worse */
     af.duration  = 1;
     affect_to_char(ch, &af);
     af.location = APPLY_DEX;
     af.modifier = (0 - mult)/2; 
     affect_to_char(ch, &af);
     act("You slip on the oil too!",TRUE,ch,0,victim,TO_CHAR);
     act("$n slips on $s own oil.",TRUE,ch,0,victim,TO_NOTCHAR);
   }    
   return;
}
ASPELLN(spell_gen_dam)
{
    bool saved = FALSE;
    struct affected_type af;
    int mult, dam, i, damtype, odamtype, lev;
    struct char_data *tmp_vict, *temp;
    lev = spell_info[spell_no].min_level - level;
    damtype = spell_info[spell_no].sec_stats;
    odamtype = damtype;
    if (victim)
      if (saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch)))
	saved = TRUE;
    mult = 1;
    while (level < -SPELL_MULT){
	mult += 1;
	level += SPELL_MULT;
    }
    mult = MIN(10, mult);
    switch (spell_no)
	{
	case SPELL_CARBUNCLE_OT_EARTH:
	  if ((world[ch->in_room].sector_type == SECT_WATER_SWIM) ||
	      (world[ch->in_room].sector_type == SECT_WATER_NOSWIM) ||
	      (world[ch->in_room].sector_type == SECT_UNDER_WATER) ||
	      (world[ch->in_room].sector_type == SECT_FLY)){
	    send_to_char("This spell only works on land.\r\n",ch);
	    return;
	  }
	  dam = dice(10,7);
	  break;
	case SPELL_EARTH_SHOCK:
	  if ((world[ch->in_room].sector_type == SECT_WATER_SWIM) ||
	      (world[ch->in_room].sector_type == SECT_WATER_NOSWIM) ||
	      (world[ch->in_room].sector_type == SECT_UNDER_WATER) ||
	      (world[ch->in_room].sector_type == SECT_FLY)){
	    send_to_char("This spell only works on land.\r\n",ch);
	    return;
	  }
	  for (tmp_vict = character_list; tmp_vict; tmp_vict = temp) {
	    temp = tmp_vict->next;
	    if ((ch->in_room == tmp_vict->in_room) && (ch != tmp_vict) &&
		(!IS_AFFECTED(tmp_vict,AFF_HIDE)
		 || can_see_hidden(ch,tmp_vict)))
	      {
		dam = dice(3,10);
		if (tmp_vict == victim)
		  dam += dice(8,10);
		if (mult > 0)
		  dam *= mult;
		if ( saves_spell(tmp_vict, SAVING_SPELL,level) &&
		     dam > 3)
		  dam = number(dam/4,dam/3);
		sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",
			(saved ? "Yes": "No"),mult,dam);
		if (PRF_FLAGGED(ch,PRF_DEBUG))
		  send_to_char(buf,ch);
		damage(ch, tmp_vict, dam, spell_no,-1, saved);
	      } 
	  }
	  return;
	  break;
	case SPELL_PALM_OF_FLAME:
	  dam = dice(5,4);
	  break;
	case SPELL_LUNGS_OF_WATER_DEATH:
	  if (IS_AFFECTED(victim, AFF_WATER_BREATH)
	      || IS_UNDEAD(victim))
	    dam = 0;
	  else
	    dam  = dice(15,16);
	  odamtype = 0;
	    break;
	case SPELL_BLADE_OF_SHIVERING_ICE:
	  dam = dice(10,12);
	  send_to_room("A shivering blade of ice starts to form in the air.\r\n",
		       ch->in_room, FALSE);
	  if (world[ch->in_room].sector_type == SECT_FIRE)
		 dam /= 10;
	  else if (world[ch->in_room].sector_type == SECT_ICE)
	    dam *= 4;
	  else if (world[ch->in_room].sector_type == SECT_DESERT_HOT)
	    dam /=4;
	  else if (world[ch->in_room].sector_type == SECT_DESERT_COLD)
	    dam /=2;
	  if (IS_SET(world[ch->in_room].room_flags, HOT))
	    dam /=4;
	  else if (IS_SET(world[ch->in_room].room_flags, COLD))
	    dam *= 3;
	  else if (IS_SET(world[ch->in_room].room_flags, WARM))
	    dam /= 2;
	  else if (IS_SET(world[ch->in_room].room_flags, COOL)){
	    dam *= 3;
	    dam /= 2;
	  }
	  if (saved && dam > 3)
	    dam = number(dam/4, dam/3);
	  odamtype = 0;
	  break;	    
	case SPELL_FLASH_OF_SCARLET_FLAME:
    	    dam = dice(10,5);
	    if (!saved)
		{
		  if (!IS_AFFECTED(victim, AFF_BLIND) && AWAKE(victim)) {
		    act("You are blinded!",FALSE,ch,0,victim,TO_VICT);
		    act("$N is blinded!",FALSE,ch,0,victim,TO_CHAR);
		    act("$N is blinded!",FALSE,ch,0,victim,TO_NOTVICT);
		     af.type      = SPELL_FLASH_OF_SCARLET_FLAME;
		     af.location  = APPLY_HITROLL;
		     af.modifier  = -5;  /* Make hitroll worse */
		     af.duration  = 1;
		     af.level = lev;
		     af.bitvector = AFF_BLIND;
		     affect_to_char(victim, &af);
		  }
		    af.type      = SPELL_FLASH_OF_SCARLET_FLAME;
		    af.duration  = 1;
		    af.level = lev;
		    af.location = APPLY_ALL_AC;
		    af.bitvector = 0;		    
		    af.modifier = + 50; /* Make AC Worse! */
		    affect_to_char(victim, &af);
		}
	    break;
	case SPELL_DELUGE_OF_RUSHING:
    	    dam = dice(10,4);
	    if (world[ch->in_room].sector_type == SECT_UNDER_WATER)
		dam += dice(10,4);	    
	    else if (weather_info.sky >= SKY_RAINING)
		dam += dice(10,2);
	    if (world[ch->in_room].sector_type == SECT_WATER_SWIM)
		dam += dice(10,2);
	    if (world[ch->in_room].sector_type == SECT_WATER_NOSWIM)
		dam += dice(10,2);
	    if (IS_AFFECTED(victim, AFF_WATER_BREATH)
		|| IS_UNDEAD(victim))
		dam/= 2;
	    if (!saved && !IS_AFFECTED(victim, AFF_WATER_BREATH))
		{
		  if (!IS_AFFECTED(victim, AFF_BLIND)) {
		    act("You are blinded!",FALSE,ch,0,victim,TO_VICT);
		    act("$N is blinded!",FALSE,ch,0,victim,TO_CHAR);
		    act("$N is blinded!",FALSE,ch,0,victim,TO_NOTVICT);
		     af.type      = SPELL_DELUGE_OF_RUSHING;
		     af.location  = APPLY_HITROLL;
		     af.modifier  = -5;  /* Make hitroll worse */
		     af.duration  = 1;
		     af.level = lev;
		     af.bitvector = AFF_BLIND;
		     affect_to_char(victim, &af);
		  }
		  af.type      = SPELL_DELUGE_OF_RUSHING;
		  af.duration  = 1;
		  af.level = lev;
		  af.location = APPLY_ALL_AC;
		  af.modifier = + 50; /* Make AC Worse! */
		  af.bitvector = AFF_BASH;		    
		  affect_to_char(victim, &af);
		  GET_POS(victim) = POSITION_SITTING;
		}
	    if (saved && dam> 3)
		dam = number(dam/4, dam/3);
	    odamtype = 0;
	    break;
	case SPELL_MIGHTY_TORRENT:
    	    dam = dice(5,10);
	    if (world[ch->in_room].sector_type == SECT_UNDER_WATER)
		dam += dice(6,10);	    
	    else if (weather_info.sky >= SKY_RAINING)
		dam += dice(2,10);
	    if (world[ch->in_room].sector_type == SECT_WATER_SWIM)
		dam += dice(2,10);
	    if (world[ch->in_room].sector_type == SECT_WATER_NOSWIM)
		dam += dice(3,10);
	    if (IS_AFFECTED(victim, AFF_WATER_BREATH)
		|| IS_UNDEAD(victim))
		dam/= 2;
	    if (!saved && !IS_AFFECTED(victim, AFF_WATER_BREATH))
		{
		  if (!IS_AFFECTED(victim, AFF_BLIND)) {
		    act("You are blinded!",FALSE,ch,0,victim,TO_VICT);
		    act("$N is blinded!",FALSE,ch,0,victim,TO_CHAR);
		    act("$N is blinded!",FALSE,ch,0,victim,TO_NOTVICT);
		    af.type      = SPELL_MIGHTY_TORRENT;
		    af.location  = APPLY_HITROLL;
		    af.modifier  = -5;  /* Make hitroll worse */
		    af.duration  = 2;
		    af.level = lev;
		    af.bitvector = AFF_BLIND;
		    affect_to_char(victim, &af);
		  }
		  af.type      = SPELL_MIGHTY_TORRENT;		  
		  af.duration  = 2;
		  af.level = lev;
		  af.location = APPLY_ALL_AC;
		  af.modifier = + 50; /* Make AC Worse! */
		  af.bitvector = AFF_BASH;
		  affect_to_char(victim, &af);
		  GET_POS(victim) = POSITION_SITTING;
		}
	    if (saved && dam> 3)
		dam = number(dam/4, dam/3);
	    odamtype = 0;
	    break;
	case SPELL_PILUM_OF_FIRE:
	    dam = dice(10,8);
	    break;
	case SPELL_ARC_OF_FIERY_RIBBONS: 
	    for (tmp_vict = world[ch->in_room].people; tmp_vict;
						       tmp_vict = temp){
		temp = tmp_vict->next_in_room;
		if ((ch->in_room == tmp_vict->in_room) && (ch != tmp_vict) &&
		    (!IS_AFFECTED(tmp_vict,AFF_HIDE)
		     || can_see_hidden(ch,tmp_vict)))
		    {
			dam = dice(8,7);
			if (mult > 0)
			    dam *= mult;
			if (saves_spell(tmp_vict, SAVING_SPELL, GET_LEVEL(ch)))
			    if (dam > 2)
				dam = number(dam/4,dam/2);
			if (IS_AFFECTED(tmp_vict, AFF_RESIST_HEAT)
			    && damtype == SKILL_IGNEM)
			    dam /= 2;
			if (IS_AFFECTED(tmp_vict, AFF_RESIST_COLD)
			    && damtype == SKILL_IGNEM)
			    dam *= 2;			
			sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n"
				,(saved ? "Yes": "No"),mult,dam);
			if (PRF_FLAGGED(ch,PRF_DEBUG))
			    send_to_char(buf,ch);	   		       
			if (affected_by_spell(tmp_vict, SPELL_ENCASE_IN_ICE)){
			  reduce_ice(tmp_vict, dam);
			  if (affected_by_spell(tmp_vict, SPELL_ENCASE_IN_ICE))
			    dam = 0;
			}
			damage(ch, tmp_vict, dam, spell_no,-1, saved);
			  
		    }
	    }
	    return;
	    break;
	case SPELL_BALL_OF_ABYSMAL_FLAME:
	    for (tmp_vict = character_list; tmp_vict; tmp_vict = temp) {
		temp = tmp_vict->next;
		if ((ch->in_room == tmp_vict->in_room) && (ch != tmp_vict) &&
		     (!IS_AFFECTED(tmp_vict,AFF_HIDE)
		      || can_see_hidden(ch,tmp_vict)))
		    {
			dam = dice(5,10);
			if (tmp_vict == victim)
			    dam += dice(7,10);
			if (mult > 0)
			    dam *= mult;
			if ( saves_spell(tmp_vict, SAVING_SPELL,level) &&
			    dam > 3)
			    dam = number(dam/4,dam/3);
			if (IS_AFFECTED(ch, AFF_RESIST_HEAT)
			    && damtype == SKILL_IGNEM)
			    dam /= 2;
			if (IS_AFFECTED(victim, AFF_RESIST_COLD)
			    && damtype == SKILL_IGNEM)
			    dam *= 2;			
			sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",
				(saved ? "Yes": "No"),mult,dam);
			if (PRF_FLAGGED(ch,PRF_DEBUG))
			    send_to_char(buf,ch);
			if (affected_by_spell(tmp_vict, SPELL_ENCASE_IN_ICE)){
			  reduce_ice(tmp_vict, dam);
			  if (affected_by_spell(tmp_vict, SPELL_ENCASE_IN_ICE))
			    dam = 0;
			}
			damage(ch, tmp_vict, dam, spell_no,-1, saved);
			if ((IS_MOB(ch) &&
			     !IS_MOB(tmp_vict))
			     || (!IS_MOB(ch)  &&  IS_MOB(tmp_vict)))
			    spell_damage_equipment(ch, tmp_vict,odamtype,dam);
		    } 
	    }
	    return;
	    break;
	case SPELL_POOL_OF_JENNY_GREENTEETH:
	    send_to_room("The air suddenly seems murkey and green.\r\n",ch->in_room, FALSE);
	    send_to_room("You hear a faint cackling off in the distance.\r\n",ch->in_room, FALSE);			
	    for (tmp_vict = character_list; tmp_vict; tmp_vict = temp) {
		temp = tmp_vict->next;
		if (ch->in_room == tmp_vict->in_room)
		    {
			dam = dice(15,16);
			if (mult > 0)
			    dam *= mult;
			if (IS_AFFECTED(tmp_vict, AFF_WATER_BREATH)
			    || IS_UNDEAD(victim))
			    dam = 0;
			sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",(saved ? "Yes": "No"),mult,dam);
			if (PRF_FLAGGED(ch,PRF_DEBUG))
			    send_to_char(buf,ch);
			damage(ch, tmp_vict, dam, spell_no -1,-1, saved);

		    } 
	    }
	    return;
	    break;
	case SPELL_LAST_FLIGHT_OT_POENIX:
	  for (tmp_vict = character_list; tmp_vict; tmp_vict = temp) {
	      temp = tmp_vict->next;
	      if ((ch->in_room == tmp_vict->in_room)){
		  dam  = dice(15,20);
		  if (mult > 0)
		    dam *= mult;
		  if (tmp_vict == ch)
		    dam > 2;
		  if (saves_spell(tmp_vict, SAVING_SPELL,level) &&
		       dam > 3)
		    dam = number(dam/4,dam/3);
		  if (IS_AFFECTED(tmp_vict, AFF_RESIST_HEAT)
		      && damtype == SKILL_IGNEM)
		    dam /= 2;
		  if (IS_AFFECTED(tmp_vict, AFF_RESIST_COLD)
		      && damtype == SKILL_IGNEM)
		    dam *= 2;			
		  sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",
			  (saved ? "Yes": "No"),mult,dam);
		  if (PRF_FLAGGED(ch,PRF_DEBUG))
		    send_to_char(buf,ch);
		  if (affected_by_spell(tmp_vict, SPELL_ENCASE_IN_ICE)){
		    reduce_ice(tmp_vict, dam);
		    if (affected_by_spell(tmp_vict, SPELL_ENCASE_IN_ICE))   
		      dam = 0;
		  }
		  damage(ch, tmp_vict, dam, spell_no,-1, saved);
		  if ((IS_MOB(ch) &&
		       !IS_MOB(tmp_vict))
		      || (!IS_MOB(ch)  &&  IS_MOB(tmp_vict)))
		    spell_damage_equipment(ch, tmp_vict,odamtype,dam);
	      } 
	  }
	  return;
	  break;
	case SPELL_RAIN_OF_STONES:
	  {
	    int d_dice = 8, d_pulse = 0;
	    if (!OUTSIDE(ch)){
	      send_to_char("You can't cast this spell indoors.\r\n",ch);
	      return;
	    }
	    if (weather_info.sky == SKY_CLOUDLESS){
	      send_to_char("It's not raining.  Your spell has no effect.\r\n",ch);
	      return;
	    }
	    while (d_dice > 0){
	      dam = dice(4,d_dice);
	      if (mult)
		dam *= mult;
	      switch (weather_info.sky){
	      case SKY_CLOUDY:
		dam /=2;
		break;
	      case SKY_RAINING:
		break;
	      case SKY_LIGHTNING:
		dam *= 2;
		break;
	      }
	      sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",
		      "N/a",mult,dam);
	      if (PRF_FLAGGED(ch,PRF_DEBUG))
		send_to_char(buf,ch);
	      add_event(d_pulse,EVENT_ROOM_DAMAGE,dam,
			world[ch->in_room].number,
			SPELL_RAIN_OF_STONES,FALSE,
			"Stones rain down all around!",ch,ch);
	      d_dice -=2;
	      d_pulse += 8;
	    }
	    return;
	  }
	  break;
	  
	case SPELL_BREATH_OF_VULCAN:
	  {
	    int d_dice = 8, d_pulse = 0;
	    while (d_dice > 0){
	      dam = dice(10,d_dice);
	      if (mult)
		dam *= mult;
	      sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",
		      "N/a",mult,dam);
	      if (PRF_FLAGGED(ch,PRF_DEBUG))
		send_to_char(buf,ch);
	      add_event(d_pulse,EVENT_ROOM_DAMAGE,dam,
			world[ch->in_room].number,
			SPELL_BREATH_OF_VULCAN,FALSE,
			"The air is thick with burning ashes!",ch,ch);
	      d_dice -=2;
	      d_pulse += 12;
	    }
	    return;
	  }
	  break;
	case SPELL_CALL_OF_THE_WATERY_GRAVE:
	  {
	    int d_dice = 18, d_pulse = 0;
	    send_to_room("A gleaming pool of water materialises\r\n", ch->in_room, FALSE);
	    while (d_dice > 0){
	      dam = dice(15,d_dice);
	      if (mult)
		dam *= mult;
	      sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",
		      "N/a",mult,dam);
	      if (PRF_FLAGGED(ch,PRF_DEBUG))
		send_to_char(buf,ch);
	      add_event(d_pulse,EVENT_ROOM_DAMAGE,dam,
			world[ch->in_room].number,
			SPELL_CALL_OF_THE_WATERY_GRAVE,TRUE,
			"Tendrils of water reach out to grab you!"
			,ch,ch);
	      d_dice -=4;
	      d_pulse += 12;
		    odamtype = 0;
	    }
	    return;
	  }
	  break;
	case SPELL_LANCE_OF_SOLAR_FURY:
	  dam = dice(15,12);
	  switch (weather_info.sky){
	  case SKY_CLOUDY:
	    dam *= 3;
	    dam /=4;
	    break;
	    case SKY_RAINING:
	      dam /=2;
	      break;
	  case SKY_LIGHTNING:
	    dam /= 4;
	    break;
	  }
	  if (weather_info.sky >= SKY_CLOUDY)
	    act("Your lance of solar fury is affected by the weather.",
		    FALSE,ch,0,0,TO_CHAR);
	  switch (weather_info.sunlight)
	    {
	    case SUN_RISE:
	    case SUN_SET:
	      dam *= 3;
	      dam /=4;
		    break;
	    case SUN_DARK:
	      dam = 0;
	      break;
	    }
	  if (weather_info.sunlight != SUN_LIGHT &&
	      weather_info.sunlight != SUN_DARK)
	    act("The sun is low in the sky.\r\nYour lance of solar fury is diminished.",
		FALSE,ch,0,0,TO_CHAR);
	    if (weather_info.sunlight == SUN_DARK)
	      act("It's nighttime.\r\nYour lance of solar fury fails.",
		  FALSE,ch,0,0,TO_CHAR);	    
	    
	    break;
	default:
	  dam = dice(4,10);
	  break;
	}
    if (mult > 0)
      dam *= mult;
    if (saved && dam > 4)
      dam = number(0,dam/4);
    if (damtype == SKILL_IGNEM &&
	(affected_by_spell(victim, SPELL_ENCASE_IN_ICE))){
      reduce_ice(victim, dam);
      if (affected_by_spell(victim, SPELL_ENCASE_IN_ICE))
	dam =0;
    }
    if (IS_AFFECTED(victim, AFF_RESIST_HEAT)
	&& damtype == SKILL_IGNEM)
      dam /= 2;
    if (IS_AFFECTED(victim, AFF_RESIST_COLD) && damtype == SKILL_IGNEM)
      dam *= 2;
    sprintf(buf,"Saved %s, Mult: %d, Spell damage: %d\r\n",(saved ? "Yes": "No"),mult,dam);
    if (PRF_FLAGGED(ch,PRF_DEBUG))
      send_to_char(buf,ch);
    
    damage(ch, victim, dam, spell_no,-1, saved);
    if (odamtype && ((IS_MOB(ch) && !IS_MOB(victim))
		     || (!IS_MOB(ch) && IS_MOB(victim))))
      spell_damage_equipment(ch, victim, odamtype, dam);
}

void reduce_ice(struct char_data *ch, int dam)
{
  struct affected_type *af, *next;
  
  for (af = ch->affected; af; af = next){
    next = af->next;
    if (af->type == SPELL_ENCASE_IN_ICE)
      {
	af->duration -= MAX(1,dam/20);
	if (af->duration <= 0){
	  affect_remove(ch, af);
	  act("The ice encasing $n melts away.", FALSE,ch,0,0,TO_NOTCHAR);
	  send_to_char("The ice encasing you melts away.\r\n",ch);
	}
	else{
	  act("The ice encasing $n melts a little.", FALSE,ch,0,0,TO_NOTCHAR);
	  send_to_char("The ice encasing you melt a little.\r\n",ch);
	}
      }
  }
}

ASPELLN(spell_lamp_wo_flame)
{
   struct obj_data *tmp_obj;
   int dur,llev=1;
   assert(ch);

   dur = GET_FOC(ch)/3;
   while (level < -SPELL_MULT){
       dur++;
       llev++;
       level += SPELL_MULT;
   }
   llev = MAX(10,llev);
   dur = MIN(12,dur);
   dur = MAX(4,dur);  
   if (obj) {
       if (IS_OBJ_STAT(obj, ITEM_MAGIC)){
	   send_to_char("Nothing seems to happen.\r\n",ch);
	   return;
       }
       act("$p starts to glow.", FALSE, ch, obj, 0, TO_CHAR);
       act("$p starts to glow.", TRUE, ch, obj, 0, TO_ROOM);
	 
       if (!IS_SET(obj->obj_flags.extra_flags, ITEM_GLOW) ) {
	   SET_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
	   obj->obj_flags2.light +=  (llev*2);
       }
       if (obj->worn_by) {
	 obj->worn_by->light = compute_char_light_value(obj->worn_by);
	 if (obj->worn_by->in_room >= 0
	     && (obj->worn_by->in_room <= top_of_world))
	   world[obj->worn_by->in_room].light =
	     compute_room_light_value(&world[obj->worn_by->in_room]);
       }
       return;
   }

   CREATE(tmp_obj, struct obj_data, 1);
   clear_object(tmp_obj);
   tmp_obj = read_object(1217, VIRTUAL, 1);
   tmp_obj->obj_flags2.light = (llev*2);
   if (tmp_obj->obj_flags2.light > 20)
       tmp_obj->obj_flags2.light = 20;
   obj_to_room(tmp_obj, ch->in_room, FALSE);
   act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
   act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}
ASPELLN(spell_blacksmith_might)
{
    struct affected_type af;
    int bon, mult = 1, lev;

    assert(victim && ch);
   lev = spell_info[spell_no].min_level - level;    
    while (level < -SPELL_MULT){
	mult++;
       level += SPELL_MULT;
   }
   
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (!aff_by_spell(victim, SPELL_MIGHT)) {
       if (GET_STR(victim) == 30)
	   return;
       else if (GET_STR(victim) == 29)
       bon = 1;
       else if (GET_STR(victim) == 28)
	   bon = number(1,2);
       else if (GET_STR(victim) == 27)
	   bon = number(1,3);
       else
	   bon = number(1,4);
       
       act("You feel stronger.", FALSE, victim, 0, 0, TO_CHAR);
       act("$n seems stronger.", FALSE, victim, 0, 0, TO_ROOM);       
       
       af.type      = SPELL_MIGHT;
       af.duration  = MIN(12,5 + mult);
       af.modifier  = bon;
       af.level = lev;
       af.location  = APPLY_STR;
       af.bitvector = 0;
       
       affect_to_char(victim, &af);
   }
   else
       send_to_char("Nothing seems to happen.\r\n",victim);

}
ASPELLN(spell_hearty_health)
{
   struct affected_type af;
   int bon, mult = 1, lev;

   assert(victim && ch);
   lev = spell_info[spell_no].min_level - level;       
   while(level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (!aff_by_spell(victim, SPELL_HEALTH)) {
       if (GET_CON(victim) == 30)
	   return;
       else if (GET_CON(victim) == 29)
	   bon = 1;
       else if (GET_CON(victim) == 28)
	   bon = number(1,2);
   else
       bon = number(1,3);
       
       act("You feel healthier.", FALSE, victim, 0, 0, TO_CHAR);
       
       af.type      = SPELL_HEALTH;
       af.duration  = MIN(12,5 + mult);
       af.modifier  = bon;
       af.level = lev;
       af.location  = APPLY_CON;
       af.bitvector = 0;
       
       affect_to_char(victim, &af);
   }
   else
       send_to_char("Nothing seems to happen.\r\n",victim);       
}
ASPELLN(spell_nimble_cat)
{
   struct affected_type af;
   int bon, mult = 1, lev;

   assert(victim && ch);
   lev = spell_info[spell_no].min_level - level;       
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }

   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (!aff_by_spell(victim, SPELL_NIMBLENESS))
     {
       if (GET_DEX(victim) == 30)
	 return;
       else if (GET_DEX(victim) == 29)
	 bon = 1;
       else if (GET_DEX(victim) == 28)
	 bon = number(1,2);
       else
	 bon = number(1,3);
       
       act("You feel as nimble as a cat.", FALSE, victim, 0, 0, TO_CHAR);
       
       af.type      = SPELL_NIMBLENESS;
       af.duration  = MIN(12,5 + mult);
       af.modifier  = bon;
       af.level = lev;
       af.location  = APPLY_DEX;
       af.bitvector = 0;
       
       affect_to_char(victim, &af);
     }
   else
     send_to_char("Nothing seems to happen.\r\n",victim); 
}
ASPELLN(spell_youthful_beauty)
{
   struct affected_type af;
   int bon, mult = 1, lev;

   assert(victim && ch);
   lev = spell_info[spell_no].min_level - level;    
   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }

   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (!aff_by_spell(victim, SPELL_BEAUTY)) {   
       if (GET_CHR(victim) == 30)
	   return;
       else if (GET_CHR(victim) == 29)
	   bon = 1;
       else if (GET_CHR(victim) == 28)
	   bon = number(1,2);
       else
	   bon = number(1,3);
       
       act("You feel more attractive.", FALSE, victim, 0, 0, TO_CHAR);
       
       af.type      = SPELL_BEAUTY;
       af.duration  = MIN(12,5 + mult);
       af.modifier  = bon;
       af.level = lev;
       af.location  = APPLY_CHR;
       af.bitvector = 0;
       
       affect_to_char(victim, &af);
   }
   else
       send_to_char("Nothing seems to happen.\r\n",victim);       
}
ASPELLN(spell_seven_league)
{
   int	to_room, lev=0;
   extern int	top_of_world;      /* ref to the top element of world */

   ACMD(do_look);
   void	death_cry(struct char_data *ch);
   assert(victim && ch);

   while (level < -SPELL_MULT){
       lev++;
       level += SPELL_MULT;
   }
   lev = spell_info[spell_no].min_level - level;       

   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
   if (IS_NPC(victim) && (saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch))
			  || saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch))))
       {
	   act("$n becomes enraged.",FALSE,victim,0,0,TO_ROOM);
	   damage(ch,victim,0,SPELL_SEVEN_LEAGUE, -1, 1);
	   return;
       }

   if (IS_SET(world[victim->in_room].room_flags, RECALL_DISTORT)
       && (number(1,10) > lev)){
       send_to_char("You feel momentarily disoriented.\r\n",victim);
       act("$n slowly fades out of existence.", FALSE, victim, 0, 0, TO_ROOM);
       act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
       return;
   }
   do {
      to_room = number(0, top_of_world);
   } while (IS_SET(world[to_room].room_flags, PRIVATE)
	    || (!zone_table[world[to_room].zone].open)
	    || (IS_SET(world[to_room].room_flags, UNFINISHED)
		&& GET_LEVEL(victim) < LEVEL_BUILDER));

   act("$n slowly fades out of existence.", FALSE, victim, 0, 0, TO_ROOM);
   char_from_room(victim);
   char_to_room(victim, to_room, FALSE);
   act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);

   do_look(victim, "", 15, 0);

   if (IS_SET(world[to_room].room_flags, DEATH) && !IS_NPC(victim) &&
       level < LEVEL_BUILDER) {
      log_death_trap(victim);
      death_cry(victim);
      extract_char(victim, TRUE);
   }
}

ASPELLN(spell_leap)
{
   extern int	top_of_world;
   extern struct char_data *combat_list;
   int	location, top, bottom, mult=1, zone, tries=0;
   bool found = FALSE;
   struct char_data *k, *next_char;
   
   void	stop_fighting(struct char_data *ch);    
   ACMD(do_look);

   while (level < -SPELL_MULT){
       mult++;
       level += SPELL_MULT;
   }
   mult = MAX(1,mult);
   
   assert(victim);

   if (IS_NPC(victim) && !victim->desc)
      return;
   if (IS_SET(world[ch->in_room].room_flags, RECALL_DISTORT)
       && (number(3,15) > mult)){
       send_to_char("Nothing seems to happens.\r\n",ch);
       return;
   }

   
   for (zone = 0; zone_table[zone].number != ch->specials2.rent_zone
	  && zone <= top_of_zone_table;zone++)
     ;
   if (zone > top_of_zone_table){
     send_to_char("You are completely lost.\r\n", victim);
     return;
   }
   for (location = 0; location <= top_of_world; location++)
     if (world[location].zone == zone){
       bottom = location;
       break;
     }
   for (;location < top_of_world; location++)
     if (world[location +1].zone != zone){
       top = location;
       break;
     }
   
   while (!found && (tries < 20)){
     for (location = bottom; location <= top; location++)
       if (!number(0,7)	&&
	   ((world[location].sector_type == SECT_CITY) ||
	    IS_SET(world[location].room_flags, RECALL))) {
	 found = TRUE;
	 break;
       }
     tries++;     
   }
   if (!found){
     found = TRUE;
     location = real_room(2000);
   }
   if (GET_LEVEL(ch) >= LEVEL_BUILDER){
     found = TRUE;     
     if (PLR_FLAGGED(ch, PLR_LOADROOM))
       location = real_room(GET_LOADROOM(ch));
     else
       location = r_immort_start_room;
   }
   if ((location == top_of_world) || !found) {
     send_to_char("You are completely lost.\r\n", victim);
     return;
   }
   
   /* a location has been found. */
   if (victim->specials.fighting)
       stop_fighting(victim);
   for (k = combat_list;k;k = next_char){
       next_char = k->next_fighting;
       if (k->specials.fighting == victim)
	   stop_fighting(k);
   }
   act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
   char_from_room(victim);
   char_to_room(victim, location, FALSE);
   act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
   do_look(victim, "", 15, 0);

}
ASPELLN(spell_spasm)
{
    struct affected_type af;
    struct obj_data *j;
    ACMD(do_drop);
    int mult,lev;
    assert (victim && ch);
    
    lev = spell_info[spell_no].min_level - level;        
    while (level < -SPELL_MULT){
	mult++;
	level += SPELL_MULT;
    }
    mult = MAX(1,mult);
    mult = MIN(5,mult);
   if (IS_NPC(victim) && (IS_UNDEAD(victim) ||
			   IS_ANIMAL(victim) ||
			   (GET_RACE(victim) == CLASS_PLANT))){
     send_to_char("Your spell has no effect.\r\n",ch);
     return;
   }
    if (!number(0,10))
	{
	    act("$n becomes enraged.",FALSE,victim,0,0,TO_ROOM);
	    damage(ch,victim,0,SPELL_SPASMS, -1, 1);
	}
    if (!saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch)))
	{
	    af.type      = SPELL_SPASMS;
	    af.location  = APPLY_DEX;
	    af.modifier  = -1*mult;
	    af.bitvector = 0;
	    af.level = lev;
	    af.duration  = MAX(1,mult/3);
	    affect_join(victim, &af, TRUE, TRUE);
	    act("$n's hands start to tremble.",FALSE,victim,0,0,TO_ROOM);
	    act("Your hands start trembling.",FALSE,victim,0,0,TO_CHAR);
	    if (!saves_spell(victim, SAVING_SPELL, GET_LEVEL(ch)))
		{
		    if ((j = victim->equipment[WIELD]) && !number(0,1)){
			obj_to_char(unequip_char(victim,WIELD),victim, 1);
			sprintf(buf,"%s",j->name);
			do_drop(victim,buf,0,0);
		    }
		    if ((j = victim->equipment[HOLD]) && !number(0,1)){
			obj_to_char(unequip_char(victim,HOLD),victim, 1);
			sprintf(buf,"%s",j->name);
			do_drop(victim,buf,0,0);
		    }		    
		}
	}

}


















