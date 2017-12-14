/* ************************************************************************
*  handler.c                                     Part of CircleMUD        *
*  Usage: internal funcs: moving and finding chars/objs                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"

/* external vars */
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern char	*MENU;
extern char	*ANSI_MENU;
extern int    top_of_world;
/* external functions */
int     report_money_weight(int amount);
void    set_timer(struct obj_data *object);
void    clear_timer(struct obj_data *object);
void	free_char(struct char_data *ch);
void	stop_fighting(struct char_data *ch);
void	remove_follower(struct char_data *ch);
void	clearMemory(struct char_data *ch);
void    snarf_limited_from_char(struct obj_data *object);
int     can_see_hidden(struct char_data *sub, struct char_data *obj);
void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
void    limited_from_char(int obj_no);
void    limited_to_char(int obj_no);
void    extract_event(struct char_data *ch);
void    extract_limited(int obj_no);
char    *report_cost(int gold);
void    unhitch(struct char_data *ch, struct obj_data *cart, struct char_data *vict, char quiet);
void    free_hitched(struct obj_data *obj);
void    extinguish_light(struct obj_data *obj);
int     illumination_level(int value, struct char_data *ch);
bool    can_see_obj(struct char_data *ch, struct obj_data *vict);
bool    can_see_char(struct char_data *ch, struct char_data *vict);
void    remove_weight_from_char(struct char_data *ch, int amount);
void    add_weight_to_char(struct char_data *ch, int amount);
void change_gold(struct char_data *ch, int amount);
		 
void change_gold(struct char_data *ch, int amount)
{
  remove_weight_from_char(ch, report_money_weight(GET_GOLD(ch)));
  GET_GOLD(ch) += amount;
  add_weight_to_char(ch, report_money_weight(GET_GOLD(ch)));
}
void add_weight_to_char(struct char_data *ch, int amount)
{
  IS_CARRYING_W(ch) += amount;
  if (ch->specials.carried_by)
    IS_CARRYING_W(ch->specials.carried_by) += amount;
  if (ch->specials.mount)
    IS_CARRYING_W(ch->specials.mount) += amount;      
  
}

void remove_weight_from_char(struct char_data *ch, int amount)
{
  IS_CARRYING_W(ch) -= amount;
  if (IS_CARRYING_W(ch) < 0)
    IS_CARRYING_W(ch) =0;
  if (ch->specials.carried_by) {
    IS_CARRYING_W(ch->specials.carried_by)-= amount;
    if (IS_CARRYING_W(ch->specials.carried_by) < 0)
      IS_CARRYING_W(ch->specials.carried_by) =0;
  }
  if (ch->specials.mount) {
    IS_CARRYING_W(ch->specials.mount) -= amount;
    if (IS_CARRYING_W(ch->specials.mount) < 0)
      IS_CARRYING_W(ch->specials.mount) =0;
  }
}

char	*fname(char *namelist)
{
   static char	holder[30];
   register char	*point;

   for (point = holder; isalpha(*namelist); namelist++, point++)
      *point = *namelist;

   *point = '\0';

   return(holder);
}


int	isname(char *str, char *namelist)
{
   register char	*curname, *curstr;

   curname = namelist;
   for (; ; ) {
      for (curstr = str; ; curstr++, curname++) {
	 if (!*curstr && !isalpha(*curname))
	    return(1);

	 if (!*curname)
	    return(0);

	 if (!*curstr || *curname == ' ')
	    break;

	 if (LOWER(*curstr) != LOWER(*curname))
	    break;
      }

      /* skip to next name */

      for (; isalpha(*curname); curname++)
	 ;
      if (!*curname)
	 return(0);
      curname++;			/* first char of new name */
   }
}



void	affect_modify(struct char_data *ch, byte loc, sbyte mod, long bitv, bool add)
{
   int	maxabil;

   if (add) {
      SET_BIT(ch->specials.affected_by, bitv);
   } else {
      REMOVE_BIT(ch->specials.affected_by, bitv);
      mod = -mod;
   }


   maxabil = (IS_NPC(ch) ? 30 : 30);

   switch (loc) {
   case APPLY_NONE: break;

   case APPLY_STR:
       if (GET_STR(ch) + mod > 30)
	   mod = 30 - GET_STR(ch);
       GET_STR(ch) += mod;
       break;
   case APPLY_DEX:
       if (GET_DEX(ch) + mod > 30)
	   mod = 30 - GET_DEX(ch);
       GET_DEX(ch) += mod;
       break;
   case APPLY_INT:
       if (GET_INT(ch) + mod > 30)
	   mod = 30 - GET_INT(ch);
       GET_INT(ch) += mod;
       break;
   case APPLY_WIS:
       if (GET_WIS(ch) + mod > 30)
	   mod = 30 - GET_WIS(ch);
       GET_WIS(ch) += mod;
       break;
   case APPLY_CON:
       if (GET_CON(ch) + mod > 30)
	   mod = 30 - GET_CON(ch);
       GET_CON(ch) += mod;
       break;
   case APPLY_CHR:
       if (GET_CHR(ch) + mod > 30)
	   mod = 30 - GET_CHR(ch);
       GET_CHR(ch) += mod;
       break;
   case APPLY_PER:
       if (GET_PER(ch) + mod > 30)
	   mod = 30 - GET_PER(ch);
       GET_PER(ch) += mod;
       break;
   case APPLY_GUI:
       if (GET_GUI(ch) + mod > 30)
	   mod = 30 - GET_GUI(ch);
       GET_GUI(ch) += mod;
       break;
   case APPLY_LUC:
       if (GET_LUC(ch) + mod > 30)
           mod = 30 - GET_LUC(ch);
       GET_LUC(ch) += mod;
       break;
   case APPLY_FOCUS:
       if (GET_FOC(ch) + mod > 30)
	   mod = 30 - GET_FOC(ch);
       GET_FOC(ch) += mod;
       break;
   case APPLY_DEVOTION:
       if (GET_DEV(ch) + mod > 30)
	   mod = 30 - GET_DEV(ch);
       GET_DEV(ch) += mod;
       break;       
       
   case APPLY_SEX:
      break;

   case APPLY_RACE:
      break;

   case APPLY_LEVEL:
      break;

   case APPLY_AGE:
      ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);
      break;

   case APPLY_CHAR_WEIGHT:
      GET_WEIGHT(ch) += mod;
      break;

   case APPLY_CHAR_HEIGHT:
      GET_HEIGHT(ch) += mod;
      break;

   case APPLY_MANA:
      break;

   case APPLY_POWER:
      break;       

   case APPLY_HIT:
      ch->points.max_hit += mod;
      break;

   case APPLY_MOVE:
      ch->points.max_move += mod;
      break;

   case APPLY_GOLD:
      break;

   case APPLY_EXP:
      break;

   case APPLY_BODY_AC:
      GET_BODY_AC(ch) += mod;
      break;

   case APPLY_LEGS_AC:
       GET_LEGS_AC(ch) += mod;
      break;

   case APPLY_ARMS_AC:
      GET_ARMS_AC(ch) += mod;
      break;

   case APPLY_HEAD_AC:
      GET_HEAD_AC(ch) += mod;
      break;

   case APPLY_ALL_STOPPING:
       GET_BODY_STOPPING(ch) += mod;
       GET_LEGS_STOPPING(ch) += mod;
       GET_ARMS_STOPPING(ch) += mod;
       GET_HEAD_STOPPING(ch) += mod;
      break;
   case APPLY_ALL_AC:
       GET_BODY_AC(ch) += mod;
       GET_LEGS_AC(ch) += mod;
       GET_ARMS_AC(ch) += mod;
       GET_HEAD_AC(ch) += mod;
       break;              

   case APPLY_BODY_STOPPING:
      GET_BODY_STOPPING(ch) += mod;
      break;

   case APPLY_LEGS_STOPPING:
      GET_LEGS_STOPPING(ch) += mod;
      break;

   case APPLY_ARMS_STOPPING:
      GET_ARMS_STOPPING(ch) += mod;
      break;
       
   case APPLY_HEAD_STOPPING:
      GET_HEAD_STOPPING(ch) += mod;
      break;

   case APPLY_HITROLL:
      GET_HITROLL(ch) += mod;
      break;

   case APPLY_DAMROLL2:
      GET_DAMROLL2(ch) += mod;
      break;
   case APPLY_HITROLL2:
      GET_HITROLL2(ch) += mod;
      break;

   case APPLY_DAMROLL:
      GET_DAMROLL(ch) += mod;
      break;
      
   case APPLY_ALL_SAVE:
      ch->specials2.apply_saving_throw[0] += mod;
      ch->specials2.apply_saving_throw[1] += mod;
      ch->specials2.apply_saving_throw[2] += mod;
      ch->specials2.apply_saving_throw[3] += mod;
      ch->specials2.apply_saving_throw[4] += mod;       
       break;              
   case APPLY_SAVING_PARA:
      ch->specials2.apply_saving_throw[0] += mod;
      break;

   case APPLY_SAVING_ROD:
      ch->specials2.apply_saving_throw[1] += mod;
      break;

   case APPLY_SAVING_PETRI:
      ch->specials2.apply_saving_throw[2] += mod;
      break;

   case APPLY_SAVING_BREATH:
      ch->specials2.apply_saving_throw[3] += mod;
      break;

   case APPLY_SAVING_SPELL:
      ch->specials2.apply_saving_throw[4] += mod;
      break;

   default:
      logg("SYSERR: Unknown apply adjust attempt (handler.c, affect_modify).");
      break;

   } /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void	affect_total(struct char_data *ch)
{
   struct affected_type *af;
   int	i, j;

   for (i = 0; i < MAX_WEAR; i++) {
     if (ch->equipment[i])
       for (j = 0; j < MAX_OBJ_AFFECT; j++) {
	 if ((i == HOLD)  &&
	     (ch->equipment[i]->obj_flags.type_flag == ITEM_WEAPON)
	     && (ch->equipment[i]->affected[j].location == APPLY_DAMROLL))
	   affect_modify(ch, APPLY_DAMROLL2,
			 ch->equipment[i]->affected[j].modifier,
			 ch->equipment[i]->obj_flags2.bitvector_aff, FALSE);
	 else if ((i == HOLD)  &&
		  (ch->equipment[i]->obj_flags.type_flag == ITEM_WEAPON)
		  && (ch->equipment[i]->affected[j].location == APPLY_HITROLL))
	   affect_modify(ch, APPLY_HITROLL2,
			 ch->equipment[i]->affected[j].modifier,
			 ch->equipment[i]->obj_flags2.bitvector_aff, FALSE);
	 else
	   affect_modify(ch, ch->equipment[i]->affected[j].location,
			 ch->equipment[i]->affected[j].modifier,
			 ch->equipment[i]->obj_flags2.bitvector_aff, FALSE);
	 
	 
       }
   }
   
   for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

   ch->tmpabilities = ch->abilities;


   for (i = 0; i < MAX_WEAR; i++) {
     if (ch->equipment[i])
       for (j = 0; j < MAX_OBJ_AFFECT; j++) {
	 if ((i == HOLD)  &&
	     (ch->equipment[i]->obj_flags.type_flag == ITEM_WEAPON)
	     && (ch->equipment[i]->affected[j].location == APPLY_DAMROLL))
	   affect_modify(ch, APPLY_DAMROLL2,
			 ch->equipment[i]->affected[j].modifier,
			 ch->equipment[i]->obj_flags2.bitvector_aff, TRUE);
	 else if ((i == HOLD)  &&
		  (ch->equipment[i]->obj_flags.type_flag == ITEM_WEAPON)
		  && (ch->equipment[i]->affected[j].location == APPLY_HITROLL))
	   affect_modify(ch, APPLY_HITROLL2,
			 ch->equipment[i]->affected[j].modifier,
			 ch->equipment[i]->obj_flags2.bitvector_aff, TRUE);
	 else
	   affect_modify(ch, ch->equipment[i]->affected[j].location,
			 ch->equipment[i]->affected[j].modifier,
			 ch->equipment[i]->obj_flags2.bitvector_aff, TRUE);
       }
   }
   for (af = ch->affected; af; af = af->next)
      affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

   /* Make certain values are between 0..25, not < 0 and not > 25! */

   i = (IS_NPC(ch) ? 30 : 30);

   GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), i));
   GET_INT(ch) = MAX(0, MIN(GET_INT(ch), i));
   GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), i));
   GET_CON(ch) = MAX(0, MIN(GET_CON(ch), i));
   GET_STR(ch) = MAX(0, GET_STR(ch));

   if (IS_NPC(ch)) 
      GET_STR(ch) = MIN(GET_STR(ch), i);
   if ((GET_RACE(ch) == RACE_HALF_ELF || GET_RACE(ch) == RACE_DWARF
       || GET_RACE(ch) == RACE_AMARYA || GET_RACE(ch) == RACE_ELVEN)
	&& !IS_AFFECTED(ch,AFF_INFRARED))
       SET_BIT(ch->specials.affected_by, AFF_INFRARED);
   if (GET_RACE(ch) == RACE_TROLL)
       SET_BIT(ch->specials.affected_by, AFF_HEALING);
   if (GET_RACE(ch) == RACE_PIXIE && !IS_AFFECTED(ch,AFF_FLY))
       SET_BIT(ch->specials.affected_by, AFF_FLY);
   if (GET_RACE(ch) == RACE_MERMAN && !IS_AFFECTED(ch,AFF_WATER_BREATH))
       SET_BIT(ch->specials.affected_by, AFF_WATER_BREATH);
   if (GET_RACE(ch) == RACE_MERMAN && !IS_AFFECTED(ch,AFF_FREE_ACTION))
       SET_BIT(ch->specials.affected_by, AFF_FREE_ACTION);   
   
}



/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void	affect_to_char( struct char_data *ch, struct affected_type *af )
{
   struct affected_type *affected_alloc;

   CREATE(affected_alloc, struct affected_type, 1);

   *affected_alloc = *af;
   affected_alloc->next = ch->affected;
   ch->affected = affected_alloc;

   affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
   affect_total(ch);
}



/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NULL! Frees mem and calls 
   affect_location_apply                                                */
void	affect_remove( struct char_data *ch, struct affected_type *af )
{
   struct affected_type *hjp;

   assert(ch->affected);

   affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

   /* remove structure *af from linked list */
   if (ch->affected == af) {
      /* remove head of list */
      ch->affected = af->next;
   } else {
      for (hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next)
	 ;

      if (hjp->next != af) {
	 logg("SYSERR: FATAL : Could not locate affected_type in ch->affected. (handler.c, affect_remove)");
	 exit(1);
      }
      hjp->next = af->next; /* skip the af element */
   }

   free (af);

   affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void	affect_from_char( struct char_data *ch, int skill)
{
   struct affected_type *hjp, *next;

   for (hjp = ch->affected; hjp; hjp = next){
     next = hjp->next;
     if (hjp->type == skill)
       affect_remove(ch, hjp);
   }
}

/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
struct affected_type  *affected_by_spell( struct char_data *ch, int skill )
{
   struct affected_type *hjp;

   for (hjp = ch->affected; hjp; hjp = hjp->next)
      if (hjp->type == skill)
	 return(hjp);

   return(FALSE);
}



void	affect_join( struct char_data *ch, struct affected_type *af,
bool avg_dur, bool avg_mod )
{
   struct affected_type *hjp;
   bool found = FALSE;

   for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
      if ( hjp->type == af->type ) {

	 af->duration += hjp->duration;
	 if (avg_dur)
	    af->duration /= 2;

	 af->modifier += hjp->modifier;
	 if (avg_mod)
	    af->modifier /= 2;

	 affect_remove(ch, hjp);
	 affect_to_char(ch, af);
	 found = TRUE;
      }
   }
   if (!found)
      affect_to_char(ch, af);
}


/* move a player out of a room */
void	char_from_room(struct char_data *ch)
{
   struct char_data *i;
   struct room_data *rm;

   if (ch->in_room >= 0 && ch->in_room <= top_of_world){
       rm = &world[ch->in_room];
       if (world[ch->in_room].obj){
	   GET_OBJ_WEIGHT(world[ch->in_room].obj) =
	        MAX(100,GET_OBJ_WEIGHT(world[ch->in_room].obj)
		     - GET_WEIGHT(ch));
	   GET_OBJ_WEIGHT(world[ch->in_room].obj) =
	       MAX(100,GET_OBJ_WEIGHT(world[ch->in_room].obj)
		   - IS_CARRYING_W(ch));		   
       }

       if (ch == world[ch->in_room].people)  /* head of list */
	   world[ch->in_room].people = ch->next_in_room;
       else /* locate the previous element */ {
	   for (i = world[ch->in_room].people; i->next_in_room &&
		i->next_in_room != ch; i = i->next_in_room)
	       ;
	   
	   i->next_in_room = ch->next_in_room;
       }
       ch->in_room = NOWHERE;
       ch->next_in_room = 0;
       rm->light = compute_room_light_value(rm);
   }

}


/* place a character in a room */
void	char_to_room(struct char_data *ch, int room, bool virtual)
{
   void	raw_kill(struct char_data *ch);

   if (virtual)
       room = real_room(room);

   ch->next_in_room = world[room].people;
   world[room].people = ch;

	   
   ch->in_room = room;
   if((world[ch->in_room].sector_type== SECT_UNDER_WATER) ||
      (world[ch->in_room].sector_type== SECT_WATER_SWIM) ||
      (world[ch->in_room].sector_type== SECT_WATER_NOSWIM))
       water_extinguish_lights(ch);

   if (world[ch->in_room].tele_delay > -1 &&
       (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))) {
       add_event(world[ch->in_room].tele_delay,
		 EVENT_TELEPORT,0,0,0,0,0,&world[ch->in_room],ch);
       if (ch->specials.mount)
	 add_event(world[ch->in_room].tele_delay,EVENT_TELEPORT
		   ,0,0,0,0,0,&world[ch->in_room],ch->specials.mount);
   }
   if (world[room].obj){
       GET_OBJ_WEIGHT(world[room].obj) += GET_WEIGHT(ch);
       GET_OBJ_WEIGHT(world[room].obj) += IS_CARRYING_W(ch);
       }
   world[room].light = compute_room_light_value(&world[room]);
}


/* give an object to a char   */
void	obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
    struct obj_data *tmp;
    int virtual;
    
    object->next_content = ch->inventory;
    ch->inventory = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    add_weight_to_char(ch,GET_OBJ_WEIGHT(object));
    IS_CARRYING_N(ch)++;
    clear_timer(object);
    if (mode == 0 && !IS_NPC(ch)){
	if (object->obj_flags.value[6] >0)
	    {
		virtual = (object->item_number >= 0) ? obj_index[object->item_number].virtual : 0;
		limited_to_char(virtual);
	    }
	if (object->contains)
	    for (tmp = object->contains;tmp;tmp = tmp->next_content)
		if (tmp->obj_flags.value[6] > 0)
		    {
			virtual = (tmp->item_number >= 0) ? obj_index[tmp->item_number].virtual : 0;
			limited_to_char(virtual);
		    }
	
    }
    /* set flag for crash-save system */
    SET_BIT(PLR_FLAGS(ch), PLR_CRASH);
}


/* take an object from a char */
void	obj_from_char(struct obj_data *object, int mode)
{
    struct obj_data *tmp;
    struct char_data *ch;

    ch = object->carried_by;
    
    if (object->carried_by->inventory == object)   /* head of list */
	object->carried_by->inventory = object->next_content;
    
    else {
	for (tmp = object->carried_by->inventory; 
	     tmp && (tmp->next_content != object); 
	     tmp = tmp->next_content)
	   ; /* locate previous */
	
       tmp->next_content = object->next_content;
    }
    if (GET_LEVEL(ch) < LEVEL_BUILDER)
	set_timer(object);
     /* set flag for crash-save system */
    SET_BIT(PLR_FLAGS(object->carried_by), PLR_CRASH);

    remove_weight_from_char(object->carried_by,GET_OBJ_WEIGHT(object));
    IS_CARRYING_N(object->carried_by)--;
    if (mode == 0 && !IS_NPC(ch))
	snarf_limited_from_char(object);
    object->carried_by = 0;
    object->next_content = 0;
}

void set_timer(struct obj_data *object)
{
    struct obj_data *tmp;
    
    if ( (GET_ITEM_TYPE(object) == ITEM_CONTAINER)
	 && (object->obj_flags.value[3]) )
	return;
    object->obj_flags.timer = 24;
    if (object->contains)
	for (tmp = object->contains;tmp;tmp = tmp->next_content)
	    set_timer(tmp);
}

void clear_timer(struct obj_data *object)
{
    struct obj_data *tmp;
    
    if ( (GET_ITEM_TYPE(object) == ITEM_CONTAINER)
	 && (object->obj_flags.value[3]) )
	return;
    object->obj_flags.timer = -1;
    if (object->contains)
	for (tmp = object->contains;tmp;tmp = tmp->next_content)
	    clear_timer(tmp);
}

void snarf_limited_from_char(struct obj_data *object)
{
    int virtual;
    struct obj_data *tmp;
    
    if (object->obj_flags.value[6] >0)
	{
	  virtual = (object->item_number >= 0) ? obj_index[object->item_number].virtual : 0;
	  limited_from_char(virtual);
	}
    if (object->contains)
      for (tmp = object->contains;tmp;tmp = tmp->next_content)
	if (tmp->obj_flags.value[6] > 0)
	  {
	    virtual = (tmp->item_number >= 0) ? obj_index[tmp->item_number].virtual : 0;
	    limited_from_char(virtual);
	  }
    
}

/* Return the effect of a piece of armor in position eq_pos */
int	apply_ac(struct char_data *ch, int eq_pos)
{
   int factor;

   assert(ch->equipment[eq_pos]);

   if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
      return 0;

   switch (eq_pos) {

   case WEAR_BODY	: factor = 3; break;	/* 30% */
   case WEAR_HEAD	: factor = 2; break;	/* 20% */
   case WEAR_LEGS	: factor = 2; break;	/* 20% */
   default		: factor = 1; break;	/* all others 10% */
   }

   return (factor * ch->equipment[eq_pos]->obj_flags.value[0]);
}



void	equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
   int	j,damg;

   assert(pos >= 0 && pos < MAX_WEAR);

   if (ch->equipment[pos]) {
      sprintf(buf, "SYSERR: Char is already equipped: %s, %s", GET_NAME(ch),
	      obj->short_description);
      logg(buf);
      return;
   }

   if (obj->carried_by) {
      logg("SYSERR: EQUIP: Obj is carried_by when equip.");
      return;
   }

   if (obj->in_room != NOWHERE) {
      logg("SYSERR: EQUIP: Obj is in_room when equip.");
      return;
   }

   if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) || 
       (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) || 
       (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
      if (ch->in_room != NOWHERE) {

	 act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
	 act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
	 obj_to_room(obj, ch->in_room, FALSE);
	 return;
      } else {

	 act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
	 act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
	 obj_to_char(obj, ch,1); /* what happens if you drop something in NOWHERE? */
      }
   }
   ch->equipment[pos] = obj;
   damg = obj->obj_flags.value[4];
   if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
       switch(pos) {
       case WEAR_BODY:
	   GET_BODY_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_BODY_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_BACK:
	   GET_BODY_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_BODY_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_LEGS:
	 GET_LEGS_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	 GET_LEGS_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	 break;
       case WEAR_ARMS:
	 GET_ARMS_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	 GET_ARMS_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	 break;
       case WEAR_WRIST_R:
	 GET_ARMS_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	 GET_ARMS_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	 break;	   
       case WEAR_FINGER_R:
	 GET_ARMS_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	 GET_ARMS_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	 break;	   
       case WEAR_HEAD:
	 GET_HEAD_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	 GET_HEAD_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	 break;
       case WEAR_NECK_1:
	 GET_HEAD_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	 GET_HEAD_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	 break;
       case WEAR_EYES:
	   GET_HEAD_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_EARS_R:
	   GET_HEAD_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;	   
       case WEAR_MOUTH:
	   GET_HEAD_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;	   
       case WEAR_HANDS:
	   GET_ARMS_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_ARMS_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_FEET:
 	   GET_LEGS_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_LEGS_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       default:
	   GET_BODY_AC(ch) -= (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_BODY_STOPPING(ch) += (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       }
   obj->worn_by = ch;
   
   for (j = 0; j < MAX_OBJ_AFFECT; j++) {
     if (pos == HOLD &&
	 (obj->obj_flags.type_flag == ITEM_WEAPON) &&
	 (obj->affected[j].location == APPLY_DAMROLL))
       affect_modify(ch, APPLY_DAMROLL2,
		     obj->affected[j].modifier,
		     obj->obj_flags2.bitvector_aff, TRUE);
     else if (pos == HOLD &&
	      (obj->obj_flags.type_flag == ITEM_WEAPON) &&
	      (obj->affected[j].location == APPLY_HITROLL))
       affect_modify(ch, APPLY_HITROLL2,
		     obj->affected[j].modifier,
		     obj->obj_flags2.bitvector_aff, TRUE);
     else
       affect_modify(ch, obj->affected[j].location,
		     obj->affected[j].modifier,
		     obj->obj_flags2.bitvector_aff, TRUE);
   }
   if (obj->obj_flags.type_flag == ITEM_LIGHT
       && !IS_OBJ_STAT(obj, ITEM_GLOW))
     SET_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
   ch->light = compute_char_light_value(ch);
   if (ch->in_room >=0 && ch->in_room <= top_of_world)
     world[ch->in_room].light = compute_room_light_value(&world[ch->in_room]);
   affect_total(ch);
}



struct obj_data *unequip_char(struct char_data *ch, int pos)
{
   int	j,damg;
   struct obj_data *obj;

   assert(pos >= 0 && pos < MAX_WEAR);
   assert(ch->equipment[pos]);
   
   obj = ch->equipment[pos];
   damg = obj->obj_flags.value[4];
   if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
       switch(pos) {
       case WEAR_BODY:
	   GET_BODY_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_BODY_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_BACK:
	   GET_BODY_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_BODY_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_LEGS:
	   GET_LEGS_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_LEGS_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_ARMS:
	   GET_ARMS_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_ARMS_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_WRIST_R:
	   GET_ARMS_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_ARMS_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_FINGER_R:
	   GET_ARMS_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_ARMS_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_HEAD:
	   GET_HEAD_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_NECK_1:
	   GET_HEAD_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_EYES:
	   GET_HEAD_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_EARS_R:
	   GET_HEAD_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_MOUTH:
	   GET_HEAD_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_HEAD_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;	   
       case WEAR_HANDS:
	   GET_ARMS_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_ARMS_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       case WEAR_FEET:
	   GET_LEGS_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_LEGS_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       default:
	   GET_BODY_AC(ch) += (ch->equipment[pos]->obj_flags.value[0])*(10-damg)/10;
	   GET_BODY_STOPPING(ch) -= (ch->equipment[pos]->obj_flags.value[1])*(10-damg)/10;
	   break;
       }

   ch->equipment[pos] = 0;
   obj->worn_by = 0;

   for (j = 0; j < MAX_OBJ_AFFECT; j++) {
     if (pos == HOLD &&
	 (obj->obj_flags.type_flag == ITEM_WEAPON) &&
	 (obj->affected[j].location == APPLY_DAMROLL))
       affect_modify(ch, APPLY_DAMROLL2,
		     obj->affected[j].modifier,
		     obj->obj_flags2.bitvector_aff, FALSE);
     else if (pos == HOLD &&
	      (obj->obj_flags.type_flag == ITEM_WEAPON) &&
	      (obj->affected[j].location == APPLY_HITROLL))
       affect_modify(ch, APPLY_HITROLL2,
		     obj->affected[j].modifier,
		     obj->obj_flags2.bitvector_aff, FALSE);
     else
       affect_modify(ch, obj->affected[j].location,
		     obj->affected[j].modifier,
		     obj->obj_flags2.bitvector_aff, FALSE);
   }
   if (obj->obj_flags.type_flag == ITEM_LIGHT
       && IS_OBJ_STAT(obj, ITEM_GLOW))
     REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
   ch->light = compute_char_light_value(ch);
   if (ch->in_room >=0 && ch->in_room <= top_of_world)
     world[ch->in_room].light = compute_room_light_value(&world[ch->in_room]);
   affect_total(ch);

   return(obj);
}
int compute_room_light_value(struct room_data *room)
{
    struct char_data *tmp;
    struct obj_data *tmp_o;
    int light = 5, door;
    
    if (IS_SET(room->room_flags, DARK))
	light = -15;
    else if (room->sector_type != SECT_INSIDE
	     && !IS_SET(room->room_flags, INDOORS)) {
      switch(weather_info.sunlight) {
      case SUN_SET:
      case SUN_RISE:
	light = 2;
	break;
      case SUN_DARK:
	light = -5;
	break;
      default:
	light = 15;
	break;
      }
      switch(weather_info.moonlight) {
      case MOON_RISE:
      case MOON_SET:
	light += 2;
	break;
      case MOON_LIGHT:
	light += 4;
	break;
      default:
	break;
      }
      switch (weather_info.sky) {
      case SKY_CLOUDY:
	light -= 2;
	break;
      case SKY_RAINING:
	light -= 3;
	break;
      case SKY_LIGHTNING:
	light -= 4;
	break;
      default:
	break;
      }
    }
    if (room->sector_type == SECT_CITY)
      light += 7;
    if (room->sector_type == SECT_FIRE)
      light += 12;    
    if (room->sector_type == SECT_FOREST)
      light -= 4;
    if (room->sector_type == SECT_UNDER_WATER)
      light -= 10;
    
    for (door=0;door<=5;door++) {
      if (room->dir_option[door] &&
	  (room->dir_option[door]->to_room != NOWHERE) &&
	  !IS_SET(room->dir_option[door]->exit_info, EX_CLOSED) &&
	  !IS_SET(room->dir_option[door]->exit_info, EX_DARK) &&
	  (world[real_room(room->dir_option[door]->to_room)].light > 2)
	  && (light < 10) &&
	  ((room->sector_type == SECT_INSIDE ||
	    IS_SET(room->room_flags,INDOORS)) ||
	   (weather_info.sunlight == SUN_DARK)))
	light += world[real_room(room->dir_option[door]->to_room)].light/2;
    }
    for (tmp = room->people;tmp;tmp = tmp->next_in_room)
      {
	if (light < 10)
	  light += tmp->light;
	else if ((tmp->light > 3) || (tmp->light < 0))
	  light += tmp->light;
      }
    for (tmp_o = room->contents; tmp_o; tmp_o = tmp_o->next_content)
      if (IS_OBJ_STAT(tmp_o, ITEM_GLOW)) {
	if (light < 10)
	  light += MIN(7,tmp_o->obj_flags2.light/3);
	else if ((tmp_o->obj_flags2.light/3 > 3) ||
		 (tmp_o->obj_flags2.light/3 < 0))
	  light += MIN(7,tmp_o->obj_flags2.light/3);
      }
    return(light);
}
bool can_see_char(struct char_data *ch, struct char_data *vict)
{
  int vict_light;
  int invis_lev=0, lev=0;
  struct affected_type *af;
  
  for (af = ch->affected; af; af = af->next){
    if (af->type == SPELL_DETECT_INVISIBLE)
      lev = af->level;
  }
  
  if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE) && (lev == 0))
    lev = GET_LEVEL(ch);

  lev += GET_PER(ch);
  
  if (hates(ch, vict))
    lev += GET_PER(ch)/2;
  
  for (af = vict->affected; af; af = af->next){
    if ((af->type == SPELL_INVIS1) ||
	(af->type == SPELL_INVIS2) ||
	(af->type == SPELL_INVIS3))
      if (af->level > invis_lev)
	invis_lev = af->level;
  }
  if (IS_AFFECTED(vict, AFF_INVISIBLE) && (invis_lev == 0))
    invis_lev = GET_LEVEL(vict);
  
  if (IS_AFFECTED(vict, AFF_HIDE) && (invis_lev == 0))
    invis_lev = GET_LEVEL(vict);
  
  if ((invis_lev > 0) && (lev < invis_lev))
    return(FALSE);

  vict_light = compute_char_light_value(vict);
  switch(illumination_level(world[vict->in_room].light, ch)) {
  case -1:
    if ((vict_light > 3) && !number(0,7) && (GET_SIZE(vict) <= 4))
      return(TRUE);
    else if ((vict_light > 3) && !number(0,12) && (GET_SIZE(vict) > 4))
      return(TRUE);
    break;
  case 0:
    if ((vict_light > 3) && number(0,3) && (GET_SIZE(vict) <= 4))
      return(TRUE);
    else if ((vict_light > 3) && !number(0,3) && (GET_SIZE(vict) > 4))
      return(TRUE);
    break;
  case 1:    
    if ((vict_light >= 0) && (GET_SIZE(vict) <= 5))
      return(TRUE);
    if ((vict_light >= 0) && GET_PER(ch) > 15 && number(0,2)
	&& (GET_SIZE(vict) > 4))
      return(TRUE);
    break;
  case 2:
    if (vict_light < 20)
      return(TRUE);
    break;
  case 3:
    if (vict_light < 15)
      return(TRUE);
    break;
  case 4:
    if (vict_light < 10)
      return(TRUE);
    break;
  case 5:
    if (vict_light < 0)
      return(TRUE);
    break;
  }
  return(FALSE);
}
bool can_see_obj(struct char_data *ch, struct obj_data *vict)
{
  int vict_light=0;
  if (IS_OBJ_STAT(vict, ITEM_GLOW))
      vict_light = MIN(7,vict->obj_flags2.light/3);
  switch(illumination_level(world[vict->in_room].light, ch)) {
  case -1:
    if ((vict_light > 3) && !number(0,7) && (GET_OBJ_SIZE(vict) <= 4))
      return(TRUE);
    else if ((vict_light > 3) && !number(0,12) && (GET_OBJ_SIZE(vict) > 4))
      return(TRUE);
    break;
  case 0:
    if ((vict_light > 3) && !number(0,3) && (GET_OBJ_SIZE(vict) <= 4))
      return(TRUE);
    else if ((vict_light > 3) && !number(0,7) && (GET_OBJ_SIZE(vict) > 4))
      return(TRUE);
    break;
  case 1:    
    if ((vict_light >= 0) && (GET_OBJ_SIZE(vict) <= 4))
      return(TRUE);
    if ((vict_light >= 0) && !number(0,1) && (GET_OBJ_SIZE(vict) > 4))
      return(TRUE);
    break;
  case 2:
    if ((vict_light >= -2)  && (vict_light < 10) && (GET_OBJ_SIZE(vict) < 6))
      return(TRUE);
    if ((vict_light >= -2) && !number(0,1) && (vict_light < 10)
	&& (GET_OBJ_SIZE(vict) > 5))
      return(TRUE);
    break;
  case 3:
    if (vict_light < 15)
      return(TRUE);
    break;
  case 4:
    if (vict_light < 10)
      return(TRUE);
    break;
  case 5:
    if (vict_light < 0)
      return(TRUE);
    break;
  }
  return(FALSE);
}

int illumination_level(int value, struct char_data *ch)
{
  if (GET_RACE(ch) == RACE_DWARF)
    value += 3;
  if (GET_RACE(ch) == RACE_ELVEN)
    value += 2;
  if (GET_RACE(ch) == RACE_TROLL)
    value += 2;
  if (GET_RACE(ch) == RACE_HALF_ELF)
    value += 1;
  if (GET_RACE(ch) == RACE_AMARYA)
    value += 2;
  if (GET_RACE(ch) == RACE_MERMAN)
    value += 3;  
  if (IS_AFFECTED(ch, AFF_INFRARED))
    value += 7;

  if (value < -10)
    return(-1);
  else if (value < 0)
    return(0);
  else if (value < 10)
    return(1);
  else if (value < 20)
    return(2);
  else if (value < 30)
    return(3);
  else if (value < 40)
    return(4);
  else
    return(5);
}

int    compute_char_light_value(struct char_data *ch)
{
    int j,light=0;

    if (IS_AFFECTED(ch, AFF_SANCTUARY))
	light += 3;
    for (j=0; j< MAX_WEAR; j++)
      if (ch->equipment[j]
	  && IS_OBJ_STAT(ch->equipment[j], ITEM_GLOW)) {
	if (light < 10)
	  light += MIN(7,ch->equipment[j]->obj_flags2.light/2);
	else if ((ch->equipment[j]->obj_flags2.light/3 > 3) ||
		 (ch->equipment[j]->obj_flags2.light/3 < 0))
	  light += MIN(7,ch->equipment[j]->obj_flags2.light/2);
      }
    return(light);
}

int	get_number(char **name)
{

   int	i;
   char	*ppos;
   char	number[MAX_INPUT_LENGTH] = "";

   if ((ppos = strchr(*name, '.'))) {
      *ppos++ = '\0';
      strcpy(number, *name);
      strcpy(*name, ppos);
      
      for (i = 0; *(number + i); i++)
	 if (!isdigit(*(number + i)))
	    return(0);

      return(atoi(number));
   }

   return(1);
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
   struct obj_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = list, j = 1; i && (j <= number); i = i->next_content)
      if (isname(tmp, i->name)) {
	 if (j == number)
	    return(i);
	 j++;
      }

   return(0);
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
   struct obj_data *i;

   for (i = list; i; i = i->next_content)
      if (i->item_number == num)
	 return(i);

   return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
   struct obj_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = object_list, j = 1; i && (j <= number); i = i->next)
      if (isname(tmp, i->name)) {
	 if (j == number)
	    return(i);
	 j++;
      }

   return(0);
}





/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
   struct obj_data *i;

   for (i = object_list; i; i = i->next)
      if (i->item_number == nr)
	 return(i);

   return(0);
}





/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
   struct char_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);
   
   for (i = world[room].people, j = 1; i && (j <= number); i = i->next_in_room)
      if (isname(tmp, i->player.name)) {
	 if (j == number)
	    return(i);
	 j++;
      }

   return(0);
}





/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
   struct char_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = character_list, j = 1; i && (j <= number); i = i->next)
      if (isname(tmp, i->player.name)) {
	 if (j == number)
	    return(i);
	 j++;
      }

   return(0);
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
   struct char_data *i;

   for (i = character_list; i; i = i->next)
      if (i->nr == nr)
	 return(i);

   return(0);
}




/* put an object in a room */
void	obj_to_room(struct obj_data *object, int room, bool virtual)
{
    if (virtual)
	room = real_room(room);
    if (world[room].obj)
	GET_OBJ_WEIGHT(world[room].obj) += GET_OBJ_WEIGHT(object);
    object->next_content = world[room].contents;
    world[room].contents = object;
    object->in_room = room;
    object->carried_by = 0;
    if (((world[object->in_room].sector_type== SECT_UNDER_WATER) ||
	 (world[object->in_room].sector_type== SECT_WATER_SWIM) ||
	 (world[object->in_room].sector_type== SECT_WATER_NOSWIM))  && (
	(object->obj_flags.type_flag == ITEM_LIGHT)
	&& IS_OBJ_STAT(object, ITEM_GLOW)))
	under_water_extinguish_light(object);
    if (room >= 0 && room <= top_of_world)
	world[room].light = compute_room_light_value(&world[room]);
}


/* Take an object from a room */
void	obj_from_room(struct obj_data *object)
{
   struct obj_data *i;
   struct room_data *rm;

   /* remove object from room */
   
   rm = &world[object->in_room];
   if (object == world[object->in_room].contents)  /* head of list */
      world[object->in_room].contents = object->next_content;
   else /* locate previous element in list */	 {
      for (i = world[object->in_room].contents; i && 
          (i->next_content != object); i = i->next_content)
	 ;

      i->next_content = object->next_content;
   }

   object->in_room = NOWHERE;
   object->next_content = 0;
   
   rm->light = compute_room_light_value(rm);
}


/* put an object in an object (quaint)  */
void	obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
   struct obj_data *tmp_obj;
   int to_room=0;
   if (GET_ITEM_TYPE(obj_to) == ITEM_CONTAINER)
     to_room = real_room(obj_to->obj_flags.value[3]);
   
   if (to_room > 0 && to_room < top_of_world ){
       if (obj->in_room != NOWHERE){
	   obj_from_room(obj);
	   obj_to_room(obj,to_room, FALSE);
       }
       else
	   obj_to_room(obj,to_room, FALSE);
       GET_OBJ_WEIGHT(obj_to) += GET_OBJ_WEIGHT(obj);
   }
   else{
       obj->next_content = obj_to->contains;
       obj_to->contains = obj;
       obj->in_obj = obj_to;
   
       for (tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj)
	   GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
       
       /* top level object.  Subtract weight from inventory if necessary. */
       GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
       if (tmp_obj->carried_by)
	   add_weight_to_char(tmp_obj->carried_by,GET_OBJ_WEIGHT(obj));
   }
}


/* remove an object from an object */
void	obj_from_obj(struct obj_data *obj)
{
   struct obj_data *tmp, *obj_from;
   

   if (obj->in_obj) {
      obj_from = obj->in_obj;
      if (obj == obj_from->contains)   /* head of list */
	 obj_from->contains = obj->next_content;
      else {
	 for (tmp = obj_from->contains; 
	     tmp && (tmp->next_content != obj); 
	     tmp = tmp->next_content)
	    ; /* locate previous */

	 if (!tmp) {
	     perror("SYSERR: Fatal error in object structures.");
	     abort();
	 }

	 tmp->next_content = obj->next_content;
      }


      /* Subtract weight from containers container */
      for (tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj)
	  GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
      
      /* Subtract weight from char that carries the object */
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
      if (tmp->carried_by)
	remove_weight_from_char(tmp->carried_by,GET_OBJ_WEIGHT(obj));

      obj->in_obj = 0;
      obj->next_content = 0;
   }
   else {
      perror("SYSERR: Trying to object from object when in no object.");
      abort();
   }
}


/* Set all carried_by to point to new owner */
void	object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
   if (list) {
      object_list_new_owner(list->contains, ch);
      object_list_new_owner(list->next_content, ch);
      list->carried_by = ch;
   }
}


/* Extract an object from the world */
void	extract_obj(struct obj_data *obj, int mode)
{
  struct obj_data *temp1, *temp2;
  
  if (!obj)
    return;
   
  if (obj->in_room != NOWHERE)
    obj_from_room(obj);
  else if (obj->carried_by)
    obj_from_char(obj,mode);
  else if (obj->in_obj) {
    temp1 = obj->in_obj;
    GET_OBJ_WEIGHT(temp1) -= GET_OBJ_WEIGHT(obj);
    if (temp1->contains == obj)   /* head of list */
      temp1->contains = obj->next_content;
    else
      {
	for ( temp2 = temp1->contains ; 
	      temp2 && (temp2->next_content != obj); 
	      temp2 = temp2->next_content )
	  ;
	
	if (temp2) {
	  temp2->next_content = 
	    obj->next_content;
	}
      }
  }
  
  for ( ; obj->contains; extract_obj(obj->contains,mode))
    ;
  /* leaves nothing ! */
  
  if (object_list == obj )       /* head of list */
    object_list = obj->next;
  else
    {
      for (temp1 = object_list; 
	   temp1 && (temp1->next != obj); 
	   temp1 = temp1->next)
	;
      
      if (temp1)
	temp1->next = obj->next;
    }
  if (obj->pulled_by)
    free_hitched(obj);
  if (HASROOM(obj))
    if (world[real_room(obj->obj_flags.value[3])].obj == obj)
      world[real_room(obj->obj_flags.value[3])].obj = 0;
  if (obj->obj_flags.value[6] > 0)
    extract_limited(obj_index[obj->item_number].virtual);
  if (obj->reset && obj->reset->num > 0)
    obj->reset->num--;
  if (obj->item_number >= 0)
    (obj_index[obj->item_number].number)--;
  free_obj(obj);
  obj=0;
}

void free_hitched(struct obj_data *obj)
{
  struct follow_type *k,*j;
  k = obj->pulled_by;
  while (k)
    {
      k->follower->specials.cart = 0;
      j = k->next;
      free(k);
      k = j;
    }
  obj->pulled_by = 0;
  return;

}
void water_extinguish_lights(struct char_data *ch)
{
    int j;
    struct obj_data *obj;

    for (j =0;j<=MAX_WEAR;j++)
	if (ch->equipment[j]
	    && ch->equipment[j]->obj_flags.type_flag == ITEM_LIGHT)
	    under_water_extinguish_light(ch->equipment[j]);
    for (obj=ch->inventory;obj; obj= obj->next_content)
	if (obj->obj_flags.type_flag != ITEM_LIGHT)
    	    under_water_extinguish_light(obj);
}
void under_water_extinguish_light(struct obj_data *obj)
{
    if (obj->obj_flags.type_flag != ITEM_LIGHT)
	return;
    if( obj->obj_flags2.light <= 0)
	return;
    obj->obj_flags2.light = 0;
    if (IS_OBJ_STAT(obj, ITEM_GLOW)){
	if (obj->worn_by){
	    act("$n's $o is extinguished.",FALSE,obj->worn_by,obj,0,TO_ROOM);
	    act("Your $o is extinguished.",FALSE,obj->worn_by,obj,0,TO_CHAR);

	}
	else if (obj->carried_by)
	    act("Your $o is extinguished.",FALSE,obj->carried_by,obj,0,TO_CHAR);
	else if (obj->in_room != NOWHERE)
	    act("$p is extinguished.",FALSE,0,obj,0,TO_ROOM);
	REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
    }
    extinguish_light(obj);    
}

void	update_object( struct obj_data *obj, int use)
{
    if ((obj->obj_flags2.light > 0)
	&& IS_OBJ_STAT(obj, ITEM_GLOW)
	&& !IS_OBJ_STAT(obj, ITEM_MAGIC)){
	obj->obj_flags2.light--;
	if ((obj->obj_flags2.light == 0) && obj->worn_by){
	    REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
	    act("$n's $o stops glowing.",FALSE,obj->worn_by,obj,0,TO_ROOM);
	    act("Your $o stops glowing.",FALSE,obj->worn_by,obj,0,TO_CHAR);
	    if (obj->obj_flags.type_flag == ITEM_LIGHT)
		extinguish_light(obj);
	}
	else if ((obj->obj_flags2.light <= 4) && (obj->obj_flags2.light > 0)
		 && obj->worn_by)
    	    act("Your $o is growing dim.",FALSE,obj->worn_by,obj,0,TO_CHAR);
    }
    if (obj->obj_flags.timer > 0)
      obj->obj_flags.timer -= use;
   if (obj->contains)
      update_object(obj->contains, use);
   if (obj->next_content)
      update_object(obj->next_content, use);
}
void extinguish_light(struct obj_data *obj){
    sprintf(buf,"%s spent",obj->name);
    CREATE(obj->name, char,strlen(buf)+1);
    strcpy(obj->name, buf);
    sprintf(buf,"%s (spent)",obj->short_description);
    CREATE(obj->short_description,char, strlen(buf)+1);
    strcpy(obj->short_description, buf);
    sprintf(buf,"%s (spent)",obj->description);
    CREATE(obj->description,char, strlen(buf)+1);
    strcpy(obj->description, buf);
    obj->obj_flags.type_flag = ITEM_TRASH;
}

void	update_char_objects( struct char_data *ch )
{
   int	i;

   for (i = 0; i < MAX_WEAR; i++)
       if (ch->equipment[i])
	   update_object(ch->equipment[i], 2);
   ch->light = compute_char_light_value(ch);
    
   if (ch->inventory)
       update_object(ch->inventory, 1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void	extract_char(struct char_data *ch, bool delrent)
{
    struct obj_data *i;
    struct char_data *k, *next_char;
    struct descriptor_data *t_desc;
    int	l, was_in;
    
    extern struct char_data *combat_list;
    
    ACMD(do_save);
    ACMD(do_return);
    
    void	die_follower(struct char_data *ch);

    if (!IS_NPC(ch) && !ch->desc) {
	for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
	    if (t_desc->original == ch)
	    do_return(t_desc->character, "", 0, 0);
    }
    
/*   if (ch->in_room == NOWHERE) {
     logg("SYSERR: NOWHERE extracting char. (handler.c, extract_char)");
      exit(1);
      }
      */
    if (ch->followers || ch->master)
      die_follower(ch);
    if (ch->specials.mount) {
      act("$n falls from $N's back.",FALSE,ch,0,ch->specials.mount,TO_NOTVICT);
      act("$n falls from your back.",FALSE,ch,0,ch->specials.mount,TO_VICT);
      unmount(ch);
    }
    if (ch->specials.carrying) 
      do_drop_mob(ch);
    if (ch->specials.carried_by) 
      do_drop_mob(ch->specials.carried_by);    
    if (ch->specials.rider) {
      act("$n collapses under $N.",FALSE,ch,0,ch->specials.rider,TO_NOTVICT);
      act("$n collapses under you.",FALSE,ch,0,ch->specials.rider,TO_VICT);   
      unmount(ch->specials.rider);
    }
    
    if (IS_PULLING(ch))
	unhitch(0,IS_PULLING(ch),ch,1);
    if (ch->desc) {
	/* Forget snooping */
	if (ch->desc->snoop.snooping)
	    ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
	
	if (ch->desc->snoop.snoop_by) {
	    send_to_char("Your victim is no longer among us.\r\n",
			 ch->desc->snoop.snoop_by);
	    ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
	}

	ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
    }
      
    if (ch->inventory) {
	if (world[ch->in_room].contents)  {
	    for (i = world[ch->in_room].contents; i->next_content; 
						  i = i->next_content)
		;
       
	    i->next_content = ch->inventory;
	} else
	    world[ch->in_room].contents = ch->inventory;
	
	for (i = ch->inventory; i; i = i->next_content) {
	    i->carried_by = 0;
	    if(!IS_NPC(ch))
		snarf_limited_from_char(i);
	    i->in_room = ch->in_room;
	}
    }



   if (ch->specials.fighting)
      stop_fighting(ch);

   for (k = combat_list; k ; k = next_char) {
      next_char = k->next_fighting;
      if (k->specials.fighting == ch)
	 stop_fighting(k);
   }

   /* Must remove from room before removing the equipment! */
   was_in = ch->in_room;
   char_from_room(ch);

   /* clear equipment_list */
   for (l = 0; l < MAX_WEAR; l++)
       if (ch->equipment[l]){
	   if (!IS_NPC(ch))
	       snarf_limited_from_char(ch->equipment[l]);
	   obj_to_room(unequip_char(ch, l), was_in, FALSE);}


   /* pull the char from the list */

   if (ch == character_list)
      character_list = ch->next;
   else {
      for (k = character_list; (k) && (k->next != ch); k = k->next)
	 ;
      if (k)
	 k->next = ch->next;
      else {
	 logg("SYSERR: Trying to remove ?? from character_list. (handler.c, extract_char)");
	 abort();
      }
   }
    /* remove any events */  
   extract_event(ch);

   if (ch->desc) {
      if (ch->desc->original) {
	 do_return(ch, "", 0, 0);
      }
      save_char(ch, NOWHERE);
   }
    if (delrent && !IS_NPC(ch))
	Crash_delete_crashfile(ch);

    if (IS_NPC(ch)) {
      if (ch->reset && ch->reset->num > 0)
	ch->reset->num--;
      if (ch->nr > -1) /* if mobile */
	mob_index[ch->nr].number--;
      clearMemory(ch);   /* Only NPC's can have memory */
      free_char(ch);
      return;
   }
    
    if (ch->desc) {
      ch->desc->connected = CON_SLCT;
      if (ch->desc->color)
	  SEND_TO_Q(ANSI_MENU,ch->desc);
      else
	  SEND_TO_Q(MENU, ch->desc);
   }
}



/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functions
   which incorporate the actual player-data.
   *********************************************************************** */


struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
   struct char_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);
   
   if (!strcmp("me",name) || !strcmp("self",name) || !strcmp("I",name))
       return(ch);

   for (i = world[ch->in_room].people, j = 1; i && (j <= number); i = i->next_in_room)
      if (isname(tmp, i->player.name))
	 if (CAN_SEE(ch, i) && (!IS_AFFECTED(i,AFF_HIDE)
				|| can_see_hidden(ch,i)) )
	     {
	    if (j == number)
	       return(i);
	    j++;
	 }

   return(0);
}


struct char_data *get_player_vis(struct char_data *ch, char *name)
{
   struct char_data *i;

   if (!strcmp("me",name) || !strcmp("self",name) || !strcmp("I",name))
       return(ch);
       
   for (i = character_list; i; i = i->next)
      if (!IS_NPC(i) && !str_cmp(i->player.name, name) && CAN_SEE(ch, i))
	 return i;

   return 0;
}


struct char_data *get_char_vis(struct char_data *ch, char *name)
{
   struct char_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   /* check location */
   if ((i = get_char_room_vis(ch, name)))
      return(i);

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = character_list, j = 1; i && (j <= number); i = i->next)
      if (isname(tmp, i->player.name))
	 if (CAN_SEE(ch, i))	 {
	    if (j == number)
	       return(i);
	    j++;
	 }

   return(0);
}






struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
struct obj_data *list)
{
   struct obj_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   for (i = list, j = 1; i && (j <= number); i = i->next_content)
      if (isname(tmp, i->name))
	 if (CAN_SEE_OBJ(ch, i)) {
	    if (j == number)
	       return(i);
	    j++;
	 }
   return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
   struct obj_data *i;
   int	j, number;
   char	tmpname[MAX_INPUT_LENGTH];
   char	*tmp;

   /* scan items carried */
   if ((i = get_obj_in_list_vis(ch, name, ch->inventory)))
      return(i);

   /* scan room */
   if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
      return(i);

   strcpy(tmpname, name);
   tmp = tmpname;
   if (!(number = get_number(&tmp)))
      return(0);

   /* ok.. no luck yet. scan the entire obj list   */
   for (i = object_list, j = 1; i && (j <= number); i = i->next)
      if (isname(tmp, i->name))
	 if (CAN_SEE_OBJ(ch, i)) {
	    if (j == number)
	       return(i);
	    j++;
	 }
   return(0);
}



struct obj_data *get_object_in_equip_vis(struct char_data *ch,
char *arg, struct obj_data *equipment[], int *j)
{

   for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
      if (equipment[(*j)])
	 if (CAN_SEE_OBJ(ch, equipment[(*j)]))
	    if (isname(arg, equipment[(*j)]->name))
	       return(equipment[(*j)]);

   return (0);
}


struct obj_data *create_money(int amount)
{
   char	buf[200];

   struct obj_data *obj;
   struct extra_descr_data *new_descr=0;

   if (amount <= 0) {
      logg("SYSERR: Try to create negative or 0 money.");
      exit(1);
   }
   amount = MIN(amount, 499999);
   CREATE(obj, struct obj_data, 1);
   CREATE(new_descr, struct extra_descr_data, 1);
   new_descr->keyword =0;
   new_descr->description =0;
   new_descr->next =0;   
   clear_object(obj);
   if (amount <= 10) {
     obj->name = str_dup("penny coin coins copper coppers");
      obj->short_description = str_dup("a copper penny");
      obj->description = str_dup("One miserable copper penny is lying here.");
      new_descr->keyword = str_dup("penny copper");
      new_descr->description = str_dup("It's just one miserable little copper penny.");
   } else {
      obj->name = str_dup("coins coppers");
      if (amount < 100) {
	 obj->short_description = str_dup("a small pile of copper coins");
	 obj->description = str_dup("A small pile of copper coins is lying here.");
      }
      else if (amount == 100){
	  obj->name = str_dup("coin coins groat silver groats");
	  obj->short_description = str_dup("a silver groat");
	  obj->description = str_dup("A silver groat is lying here.");
      }else if (amount <1000 && (amount%100)) {
	 obj->short_description = str_dup("a pile of silver and copper coins");
	 obj->description = str_dup("A pile of silver and copper coins is lying here.");
      }else if (amount <1000 && !(amount%100)) {
	 obj->short_description = str_dup("a pile of silver coins");
	 obj->description = str_dup("A pile of silver coins is lying here.");
      }else if (amount == 1000){
	  obj->name = str_dup("coin coins gold crown crowns");
	  obj->short_description = str_dup("a gold coin");
	  obj->description = str_dup("A gold coin is lying here.");	 
      }else if (amount <10000 && (amount%1000) && !((amount%1000)%100) ) {
	 obj->short_description = str_dup("a pile of gold and silver coins");
	 obj->description = str_dup("A pile of gold and silver coins is lying here.");
      }else if (amount <10000 && !((amount/100)%10) && (amount%1000)) {
	 obj->short_description = str_dup("a pile of gold and copper coins");
	 obj->description = str_dup("A pile of gold and copper coins is lying here.");
      }else if (amount <10000 && (amount%1000) && (amount%100)) {
	 obj->short_description = str_dup("a pile of gold, silver and copper coins");
	 obj->description = str_dup("A pile of gold, silver and copper coins is lying here.");
	 
      }else if (amount <10000 && !(amount%1000) && !(amount%100)) {
	 obj->short_description = str_dup("a pile of gold coins");
	 obj->description = str_dup("A pile of gold coins is lying here.");
      } else if (amount <= 25000) {
	 obj->short_description = str_dup("a small pile of gold coins");
	 obj->description = str_dup("A small pile of gold coins is lying here.");
      } else if (amount <= 500000) {
	 obj->short_description = str_dup("a pile of gold coins");
	 obj->description = str_dup("A pile of gold coins is lying here.");
      } else {
	 obj->short_description = str_dup("a fortune in gold coins");
	 obj->description = str_dup("A fortune in gold coins is lying here.");
      }

      new_descr->keyword = str_dup("coins pile coin money gold copper silver");
      if (amount < 10) {
	 sprintf(buf, "There are %s.", report_cost(amount));
	 new_descr->description = str_dup(buf);
      } else if (amount < 100) {
	 sprintf(buf, "There are about %s.", report_cost(10 * (amount / 10)));
	 new_descr->description = str_dup(buf);
      } else if (amount < 1000) {
	 sprintf(buf, "It looks to be about %s.", report_cost(100 * (amount / 100)));
	 new_descr->description = str_dup(buf);
      } else if (amount < 100000) {
	 sprintf(buf, "You guess there are, maybe, %s.",
	     report_cost(1000 * ((amount / 1000) + number(0, (amount / 1000)))));
	 new_descr->description = str_dup(buf);
      } else
	 new_descr->description = str_dup("There are a LOT of coins.");
   }

   new_descr->next = 0;
   obj->ex_description = new_descr;

   obj->obj_flags.type_flag = ITEM_MONEY;
   obj->obj_flags.wear_flags = ITEM_TAKE;
   obj->obj_flags.value[0] = amount;
   obj->obj_flags.cost = amount;
   obj->obj_flags.weight = report_money_weight(amount);
   obj->obj_flags.timer = -1;
   obj->item_number = -1;

   obj->next = object_list;
   object_list = obj;

   return(obj);
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int	generic_find(char *arg, int bitvector, struct char_data *ch,
struct char_data **tar_ch, struct obj_data **tar_obj)
{
   static char	*ignore[] = {
      "the",
      "in",
      "on",
      "at",
      "\n"       };

   int	i;
   char	name[256];
   bool found;

   found = FALSE;


   /* Eliminate spaces and "ignore" words */
   while (*arg && !found) {

      for (; *arg == ' '; arg++)
	 ;

      for (i = 0; (name[i] = *(arg + i)) && (name[i] != ' '); i++)
	 ;
      name[i] = 0;
      arg += i;
      if (search_block(name, ignore, TRUE) > -1)
	 found = TRUE;

   }

   if (!name[0])
      return(0);

   *tar_ch  = 0;
   *tar_obj = 0;

   if (IS_SET(bitvector, FIND_CHAR_ROOM))      /* Find person in room */
     if ((*tar_ch = get_char_room_vis(ch, name))) 
       return(FIND_CHAR_ROOM);

   if (IS_SET(bitvector, FIND_CHAR_WORLD)) 
      if ((*tar_ch = get_char_vis(ch, name)))
	 return(FIND_CHAR_WORLD);

   if (IS_SET(bitvector, FIND_OBJ_EQUIP)) 
     if(( *tar_obj = get_object_in_equip_vis(ch, name, ch->equipment, &i)))
	 return(FIND_OBJ_EQUIP);

   if (IS_SET(bitvector, FIND_OBJ_INV)) 
     if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->inventory))) 
       return(FIND_OBJ_INV);

   if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
      if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) 
	  return(FIND_OBJ_ROOM);
      else if (world[ch->in_room].obj)
	  if ((*tar_obj =
	       get_obj_in_list_vis(ch, name,
				   world[world[ch->in_room].obj->in_room]
				   .contents))) 
	      return(FIND_OBJ_ROOM);
   }

   if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
       if ((*tar_obj = get_obj_vis(ch, name))) {
	   return(FIND_OBJ_WORLD);
       }
   }

   return(0);
}


/* a function to scan for "all" or "all.x" */
int	find_all_dots(char *arg)
{
   if (!strcmp(arg, "all"))
      return FIND_ALL;
   else if (!strncmp(arg, "all.", 4)) {
      strcpy(arg, arg+4);
      return FIND_ALLDOT;
   } else
      return FIND_INDIV;
}

