/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: command interpreter for 'cast' command (spells)                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* spell_parser.c split into two files spell_parser1.c and spell_parser2.c*/
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"


void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
int     spell_lev(struct char_data *caster, int spell);
int     stress_die(void);
int find_spell_num(char *arg);
char *find_spell_name(int spl);
char *rev_search_list(int num, struct list_index_type *list);              
/* Global data */
void damage_obj_invent(struct char_data *vict, struct obj_data *obj, int *spell_type);
void damage_obj_equiped(struct char_data *vict,int where, int *spell_type);
extern struct room_data *world;
extern struct char_data *character_list;
extern char	*spell_wear_off_msg[];
struct spell_info_type spell_info[MAX_SPL_LIST];


void	affect_update( void )
{
   static struct affected_type *af, *next_af_dude;
   static struct char_data *i;

   for (i = character_list; i; i = i->next)
      for (af = i->affected; af; af = next_af_dude) {
	 next_af_dude = af->next;
	 if (af->duration >= 1)
	    af->duration--;
	 else if (af->duration == -1)
	    /* No action */
	    af->duration = -1;  /* GODs only! unlimited */
	 else {
	    if ((af->type > 0) && (af->type <= 60)) /* It must be a spell */
	       if (!af->next || (af->next->type != af->type) || 
	           (af->next->duration > 0))
		  if (*spell_wear_off_msg[af->type]) {
		     send_to_char(spell_wear_off_msg[af->type], i);
		     send_to_char("\r\n", i);
		  }

	    affect_remove(i, af);
	 }
      }
}


bool circle_follow(struct char_data *ch, struct char_data *victim)
{
   struct char_data *k;

   for (k = victim; k; k = k->master) {
      if (k == ch)
	 return(TRUE);
   }

   return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void	stop_follower(struct char_data *ch)
{
   struct follow_type *j, *k;

   assert(ch->master);

   if (IS_AFFECTED(ch, AFF_CHARM)) {
      act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
      act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
      if (affected_by_spell(ch, SPELL_CHARM_PERSON))
	 affect_from_char(ch, SPELL_CHARM_PERSON);
   } else {
      act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
      act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
   }

   if (ch->master->followers->follower == ch) { /* Head of follower-list? */
      k = ch->master->followers;
      ch->master->followers = k->next;
      free(k);
   } else { /* locate follower who is not head of list */
      for (k = ch->master->followers; k->next->follower != ch; k = k->next)
	 ;

      j = k->next;
      k->next = j->next;
      free(j);
   }

   ch->master = 0;
   REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}
void	stop_follower_quiet(struct char_data *ch)
{
   struct follow_type *j, *k;

   assert(ch->master);

   if (affected_by_spell(ch, SPELL_CHARM_PERSON))
       affect_from_char(ch, SPELL_CHARM_PERSON);
   act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
   act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
   act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);

   if (ch->master->followers->follower == ch) { /* Head of follower-list? */
      k = ch->master->followers;
      ch->master->followers = k->next;
      free(k);
   } else { /* locate follower who is not head of list */
      for (k = ch->master->followers; k->next->follower != ch; k = k->next)
	 ;

      j = k->next;
      k->next = j->next;
      free(j);
   }
   
   ch->master = 0;
   REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void	die_follower(struct char_data *ch)
{
   struct follow_type *j, *k;

   if (ch->master)
      stop_follower(ch);

   for (k = ch->followers; k; k = j) {
      j = k->next;
      stop_follower(k->follower);
   }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
int	add_follower(struct char_data *ch, struct char_data *leader)
{
   struct follow_type *k;
   int nfollow=0;

   assert(!ch->master);

   for (k = leader->followers; k; k = k->next)
     nfollow++;
   
   k =0;
   
   if (nfollow > MAX(1,GET_CHR(leader)/3))
     return(0);
   
   ch->master = leader;

   CREATE(k, struct follow_type, 1);

   k->follower = ch;
   k->next = leader->followers;
   leader->followers = k;

   act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
   act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
   act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
   
   return(1);
}



void	say_spell(struct char_data *ch, int si, int subcmd )
{
   char splwd[MAX_INPUT_LENGTH];
   char *wrd = 0;

   int	j, offs;
   struct char_data *temp_char;


   struct syllable {
      char	org[10];
      char	new[10];
   };

   struct syllable syls[] = {
      { " ", " " },
      { "ar", "ra"   },
      { "au", "mb"    },
      { "ame", "ido" },
      { "ing", "ose" },
      { "ch", "ck" },
      { "cu", "judi" },
      { "de", "oculo" },
      { "en", "unso" },
      { "th", "dh" },
      { "lo", "hi" },
      { "mor", "zak" },
      { "move", "sido" },
      { "ness", "lacri" },
      { "ning", "illa" },
      { "per", "duda" },
      { "ra", "gru"   },
      { "re", "candus" },
      { "son", "sabru" },
      { "tect", "infra" },
      { "tri", "cula" },
      { "ven", "nofo" },
      { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "h" }, { "e", "i" },
      { "f", "y" }, { "g", "o" }, { "h", "e" }, { "i", "u" }, { "j", "y" },
      { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "i" }, { "o", "a" },
      { "p", "s" }, { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
      { "u", "j" }, { "v", "v" }, { "w", "n" }, { "x", "x" }, { "y", "l" },
      { "z", "k" }, 
      { "A", "A" }, { "B", "B" }, { "C", "Q" }, { "D", "H" }, { "E", "I" },
      { "F", "Y" }, { "G", "O" }, { "H", "E" }, { "I", "U" }, { "J", "Y" },
      { "K", "T" }, { "L", "R" }, { "M", "W" }, { "N", "I" }, { "O", "A" },
      { "P", "S" }, { "Q", "D" }, { "R", "F" }, { "S", "G" }, { "T", "H" },
      { "U", "J" }, { "V", "V" }, { "W", "N" }, { "X", "X" }, { "Y", "L" },
      { "Z", "K" }, { "", "" }
   };   


   strcpy(buf, "");
   wrd = find_spell_name(si);
   strcpy(splwd, (wrd ? wrd: "Ohm Mane Padme Ohm"));
   offs = 0;
   
   while (*(splwd + offs)) {
      for (j = 0; *(syls[j].org); j++)
	 if (strncmp(syls[j].org, splwd + offs, strlen(syls[j].org)) == 0) {
	    strcat(buf, syls[j].new);
	    if (strlen(syls[j].org))
	       offs += strlen(syls[j].org);
	    else
	       ++offs;
	 }
   }
   sprintf(buf2, "$n utters the words, '%s'", buf);
   sprintf(buf, "$n utters the words, '%s'",  wrd);
   for (temp_char = world[ch->in_room].people; 
	temp_char; 
	temp_char = temp_char->next_in_room)
     if (temp_char != ch &&
	 (!IS_NPC(temp_char) && GET_SKILL(temp_char, si))) {
       act(buf, FALSE, ch, 0, temp_char, TO_VICT);
       
     }
     else 
       act(buf2, FALSE, ch, 0, temp_char, TO_VICT);			      
}



bool saves_spell(struct char_data *ch, short save_type, int vlev)
{
   int	save=0,roll;

   /* Negative apply_saving_throw makes saving throw better! */

   save = ch->specials2.apply_saving_throw[save_type];

   switch (save_type)
       {
       case SAVING_SPELL:
	   save += MAX(0,90 - (100*(GET_LEVEL(ch)-vlev))/105);
	   break;
       case SAVING_BREATH:
	   save += MAX(0,100 - GET_LEVEL(ch)+vlev);
	   break;
       case SAVING_PETRI:
	   save += MAX(0,70 - (100*(GET_LEVEL(ch)-vlev))/118);
	   break;
       case SAVING_ROD:
	   save += MAX(0,80 - (100*(GET_LEVEL(ch)-vlev))/111);
	   break;	   
       case SAVING_PARA:
	   save += MAX(0,60 - (100*(GET_LEVEL(ch)-vlev))/125);
	   break;	   
       }
       
      if (GET_LEVEL(ch) >= LEVEL_BUILDER)
	 return(TRUE);
   save *= vlev;
   save /= MAX(1,GET_LEVEL(ch));
   if (GET_LEVEL(ch) < LEVEL_BUILDER)
       roll =  number(MIN(199,GET_LEVEL(ch)), 200);
   else
       roll = 200;
/*   printf("save type: %d\n",save_type);
   printf("save: %d, roll: %d, Target lev: %d, level: %d\n",save, roll,GET_LEVEL(ch),vlev);
   */ 
   
   return(MAX(1, save) < roll);
}



char	*skip_spaces(char *string)
{
   for (; *string && (*string) == ' '; string++)
      ;

   return(string);
}



void spell_damage_equipment(struct char_data *ch, struct char_data *vict, int spell_no, int dam)
{
    int where;
    struct obj_data *dam_obj;
    
/* first loop over item equipped */
    for (where = 0; where < MAX_WEAR ; where++)
	if (vict->equipment[where] && !number(0,6) && (number(0,200) < dam))
	    damage_obj_equiped(vict,where,&spell_no);
    
/* now loop over inventory equipment */
    
    for (dam_obj = vict->inventory; dam_obj ; dam_obj = dam_obj->next_content)
	if (!number(0,6) && (number(0,400) < dam))
	    damage_obj_invent(vict,dam_obj,&spell_no);
}

void damage_obj_equiped(struct char_data *vict,int where, int *spell_type)
{
    struct obj_data *obj;
    
    obj = unequip_char(vict,where);
    
    switch (*spell_type)
	{
	case SPELL_POISON:
	    break;
	case SPELL_FIREBALL:
	case SPELL_FIRE_BREATH:
	case SKILL_IGNEM:
	    act("$p is burned!",FALSE,vict,obj,0,TO_ROOM);
	    if (obj->obj_flags.type_flag == ITEM_SCROLL ||
		obj->obj_flags.type_flag == ITEM_POTION ||
		obj->obj_flags.type_flag == ITEM_NOTE ||
		obj->obj_flags.type_flag == ITEM_WAND ||
		obj->obj_flags.type_flag == ITEM_STAFF)
		obj->obj_flags.value[4] += number(5,12);
	    else
		obj->obj_flags.value[4] += number(0,6);
	    break;
	case SPELL_ACID_BREATH:
	    if (obj->obj_flags.type_flag == ITEM_ARMOR ||
		obj->obj_flags.type_flag == ITEM_WEAPON)
		obj->obj_flags.value[4] += number(0,6);
	    else
		obj->obj_flags.value[4] += number(0,3);
	    act("$p is coroded",FALSE,vict,obj,0,TO_ROOM);
	    break;
	case SPELL_LIGHTNING_BREATH:
	    if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		obj->obj_flags.value[4] += number(1,11);
	    else
		obj->obj_flags.value[4] += number(0,3);
	    act("$p is electrified",FALSE,vict,obj,0,TO_ROOM);
	    break;
	case SPELL_FROST_BREATH:
	    if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		obj->obj_flags.value[4] += number(1,11);
	    else
		obj->obj_flags.value[4] += number(0,3);
	    act("$p is frozen",FALSE,vict,obj,0,TO_ROOM);
	    break;
	default:
	    if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		obj->obj_flags.value[4] += number(0,9);
	    else
		obj->obj_flags.value[4] += number(0,2);
	    act("$p is damaged",FALSE,vict,obj,0,TO_ROOM);
	    break;
	}
    if (obj->obj_flags.value[4] > 10){
	obj_to_room(obj,vict->in_room,FALSE);
	scrap_item(obj);
    }
    else
      equip_char(vict,obj,where);
}
void damage_obj_invent(struct char_data *vict, struct obj_data *obj, int *spell_type)
{
    switch (*spell_type)
	{
	case SPELL_POISON:
	    break;
	case SPELL_FIREBALL:
	case SPELL_FIRE_BREATH:
	case SKILL_IGNEM:
	    act("$p is burned!",FALSE,vict,obj,0,TO_ROOM);
	    if (obj->obj_flags.type_flag == ITEM_SCROLL ||
		obj->obj_flags.type_flag == ITEM_POTION ||
		obj->obj_flags.type_flag == ITEM_NOTE ||
		obj->obj_flags.type_flag == ITEM_WAND ||
		obj->obj_flags.type_flag == ITEM_STAFF)
		obj->obj_flags.value[4] += number(4,11);
	    else
		obj->obj_flags.value[4] += number(0,5);
	    break;
	case SPELL_ACID_BREATH:
	    if (obj->obj_flags.type_flag == ITEM_ARMOR ||
		obj->obj_flags.type_flag == ITEM_WEAPON)
		obj->obj_flags.value[4] += number(0,5);
	    else
		obj->obj_flags.value[4] += number(0,2);
	    act("$p is coroded",FALSE,vict,obj,0,TO_ROOM);
	    break;
	case SPELL_LIGHTNING_BREATH:
	    if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		obj->obj_flags.value[4] += number(0,10);
	    else
		obj->obj_flags.value[4] += number(0,2);
	    act("$p is electrified",FALSE,vict,obj,0,TO_ROOM);
	    break;
	case SPELL_FROST_BREATH:
	    if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		obj->obj_flags.value[4] += number(0,10);
	    else
		obj->obj_flags.value[4] += number(0,2);
	    act("$p is frozen",FALSE,vict,obj,0,TO_ROOM);
	    break;
	default:
	    if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		obj->obj_flags.value[4] += number(0,6);
	    else
		obj->obj_flags.value[4] += number(0,1);
	    act("$p is damaged",FALSE,vict,obj,0,TO_ROOM);
	    break;
	}
    if (obj->obj_flags.value[4] > 10){
	obj_from_char(obj,0);
	obj_to_room(obj,vict->in_room,FALSE);
	scrap_item(obj);
    }

}
void damage_obj_corpse(struct obj_data *corpse, int spell_type, int dam)
{
    struct obj_data *obj, *obj_next;
    for (obj = corpse->contains;obj_next;obj = obj_next){
	obj_next = obj->next_content;
	obj_from_obj(obj);
	switch (spell_type)
	    {
	    case SPELL_POISON:
		break;
	    case SPELL_FIREBALL:
	    case SPELL_FIRE_BREATH:
	    case SKILL_IGNEM:
		act("$p is burned!",FALSE,0,obj,0,TO_ROOM);
		if (obj->obj_flags.type_flag == ITEM_SCROLL ||
		    obj->obj_flags.type_flag == ITEM_POTION ||
		    obj->obj_flags.type_flag == ITEM_NOTE ||
		    obj->obj_flags.type_flag == ITEM_WAND ||
		    obj->obj_flags.type_flag == ITEM_STAFF)
		    obj->obj_flags.value[4] += number(4,11);
		else
		    obj->obj_flags.value[4] += number(0,5);
		break;
	    case SPELL_ACID_BREATH:
		act("$p is coroded!",FALSE,0,obj,0,TO_ROOM);
		if (obj->obj_flags.type_flag == ITEM_ARMOR ||
		    obj->obj_flags.type_flag == ITEM_WEAPON)
		    obj->obj_flags.value[4] += number(0,5);
		else
		    obj->obj_flags.value[4] += number(0,2);
		break;
	    case SPELL_LIGHTNING_BREATH:
		act("$p is electrified!",FALSE,0,obj,0,TO_ROOM);
		if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		    obj->obj_flags.value[4] += number(0,10);
		else
		    obj->obj_flags.value[4] += number(0,2);
		break;
	    case SPELL_FROST_BREATH:
		act("$p is frozen!",FALSE,0,obj,0,TO_ROOM);
		if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		    obj->obj_flags.value[4] += number(0,10);
		else
		    obj->obj_flags.value[4] += number(0,2);
		break;
	    default:
		act("$p is damaged!",FALSE,0,obj,0,TO_ROOM);		
		if (IS_SET(obj->obj_flags.extra_flags,ITEM_FRAGILE))
		    obj->obj_flags.value[4] += number(0,6);
		else
		    obj->obj_flags.value[4] += number(0,1);
		break;
	    }
	
	if (obj->obj_flags.value[4] > 10){
	    obj_to_room(obj,corpse->in_room, FALSE);
	    scrap_item(obj);
	}
	obj_to_obj(obj,corpse);
	
    }

}

bool aff_by_spell(struct char_data *k, int spell)
{
    struct affected_type *af;

    if (k->affected)
	for (af = k->affected;af;af = af->next)
	    if (af->type == spell)
		return(TRUE);

    return(FALSE);
		  

}

int spell_lev(struct char_data *k, int spell)
{
    int level;

    level = spell_info[spell].min_level;
    level *= calc_difficulty(k, spell);
    return level;
}
