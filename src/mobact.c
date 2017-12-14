/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
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

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"

/* external structs */
extern struct list_index_type mob_beneficial[];
extern struct list_index_type combat_self[];
extern struct list_index_type combat_other[];
extern int top_of_world;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
extern struct spell_info_type spell_info[];
char *first_name(char *buf);
ACMD(do_move);
ACMD(do_get);
ACMD(do_wear);
ACMD(do_gen_com);
ACMD(do_open);
ACMD(do_close);
ACMD(do_flee);
ACMD(do_leave);
ACMD(do_enter);
void hunt_victim(struct char_data *ch);
void remember_id(struct char_data *ch, long id_num);
bool hates(struct char_data *ch, struct char_data *vict);
bool hates_id(struct char_data *ch, long id_num);
int  can_see_hidden(struct char_data *sub, struct char_data *obj);
void reimund_story(struct char_data *ch);
void add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
void mob_cast(struct char_data *mob);
void mobile_activity(void)
{
  SPECIAL(citizen);
  SPECIAL(cityguard);  
  SPECIAL(moods);
  ACMD(do_stand);
  register struct char_data *ch;
  struct char_data *tmp_ch, *vict, *tch;
  struct obj_data *obj,*i, *best_obj;
  int	door, found, max;
  bool present = TRUE,found_door, ctzn = FALSE;
  char buffer[100];
  memory_rec * names;
  struct affected_type *affect=0;

  extern int	no_specials;

  for (ch = character_list; ch; ch = ch->next)
    if (IS_MOB(ch)) {
      if (MOB_FLAGGED(ch,MOB_SPELL_CASTER))
	mob_cast(ch);
      if (CAN_SPEAK(ch) && is_afflicted(ch) && (GET_ALIGNMENT(ch) > 500)
	  && !number(0,1))
	do_bless(ch, "",0,SCMD_PRAY);      
      if ((GET_POS(ch) < POSITION_STANDING) &&
	  (GET_POS(ch) > POSITION_SLEEPING) &&
	  (ch->specials.default_pos == POSITION_STANDING))
	do_stand(ch, "",0,0);

      if ((affect = affected_by_spell(ch, SPELL_ENCASE_IN_ICE))
	  && ((number(0,31) + 2*(affect->duration))  < GET_STR(ch))){
	act("$n shatters $s casing of ice.",TRUE,ch,0,0, TO_ROOM);
	act("You shatter your casing of ice.",TRUE,ch,0,0, TO_CHAR);
	affect_from_char(ch, SPELL_ENCASE_IN_ICE);
      }
      if ((affect = affected_by_spell(ch, SPELL_WEB))
	  && ((number(0,31) + 2*(affect->duration))  < GET_STR(ch))){
	act("$n rips the webs binding $m.",TRUE,ch,0,0, TO_ROOM);
	act("You tear the webs holding you to shreds.",TRUE,ch,0,0, TO_CHAR);
	affect_from_char(ch, SPELL_WEB);
      }
      if (IS_AFFECTED(ch, AFF_PARALYSIS))
	continue;
      if (isname("reimund",ch->player.name) && !number(0,5)){
	reimund_story(ch);
      }
      if (MOB_FLAGGED(ch,MOB_CITIZEN) && !number(0,65))
	citizen(ch, ch, SPEC_MOBACT, "");
      else if (MOB_FLAGGED(ch,MOB_MOODS) && (number(0,3000) < abs(GET_MOOD(ch))
		   || !number(0,160)))
	moods(ch, ch, SPEC_MOBACT, "");
      /* Examine call for special procedure */
      if (IS_SET(ch->specials2.act, MOB_SPEC) && !no_specials) {
	if (!mob_index[ch->nr].func) {
	  sprintf(buf, "%s (#%d): Attempting to call non-existing mob func",
		  GET_NAME(ch), (int)mob_index[ch->nr].virtual);
	  logg(buf);
	  REMOVE_BIT(ch->specials2.act, MOB_SPEC);
	} else {
	  if ((*mob_index[ch->nr].func)(ch, ch, SPEC_MOBACT, ""))
	    continue; /* go to next char */
	}
      }
      if (isname("zoog",ch->player.name)){
	if (!number(0,20))
	  act("You see a strange pair of eyes looking at you."
	      ,FALSE,ch,0,0,TO_ROOM);
	else if (!number(0,40))
	  act("$n makes a low fluttering noise."
	      ,TRUE,ch,0,0,TO_ROOM);
      }
	    
      if (ch->specials.hunting)
	if (AWAKE(ch) && !ch->specials.fighting && !number(0,4))
	  {
	    hunt_victim(ch);
	  }		
      ctzn = FALSE;
      if (IS_ANIMAL(ch) && !number(0,20)){
	for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
	  if (!number(0,3))
	    break;
	
	if (tch)
	  sprintf(buffer,"%s", GET_NAME(tch));
	else
	  *buffer = '\0';
	
	if (isname("dog",ch->player.name) || isname("puppy",ch->player.name)){
	  switch(number(0,3)){
	  case 0:
	    do_action(ch,buffer,196,0);
	    break;
	  case 1:
	    do_action(ch,buffer,352,0);
	    break;
	  case 2:
	    do_action(ch,buffer,265,0);
	    break;
	  default:
	    break;
	  }
	  if (isname("mad", ch->player.name)){
	    switch(number(0,1)){
	    case 0:
	      do_action(ch,buffer,391,0);
	      break;
	    case 1:
	      do_action(ch,buffer,390,0);
	      break;
	    }
	  }
	}
	else if(isname("cat",ch->player.name) || isname("kitten",ch->player.name)){
	  switch(number(0,2)){
	  case 0:
	    do_action(ch,buffer,411,0);
	    break;
	  case 1:
	    do_action(ch,buffer,125,0);
	    break;
	  default:
	    break;
	  }
	}
	else if(isname("snake",ch->player.name)){
	  switch(number(0,2)){
	  case 0:
	    act("$n hisses.",FALSE,ch,0,0,TO_ROOM);
	    break;
	  case 1:
	    act("$n slithers.",FALSE,ch,0,0,TO_ROOM);
	    break;
	  default:
	    break;
	  }
	}
	else if(isname("cow",ch->player.name) || isname("ox",ch->player.name) && !number(0,2))
	  do_action(ch,buffer,412,0);
	else if(isname("squirrel",ch->player.name) && !number(0,2))
	  do_action(ch,buffer,370,0);	  
      }
      
      if (ch->specials.fighting)
	{
	  if (CAN_SPEAK(ch) && is_afflicted(ch) && (GET_ALIGNMENT(ch) > 350)
	      && !number(0,2))
	    do_bless(ch, "",0,SCMD_PRAY);
	  if (MOB_FLAGGED(ch,MOB_CITIZEN) && !number(0,6)
	      && ch->specials.fighting->specials.fighting == ch)
	    {
	      ctzn = TRUE;
	      act("$n yells 'HELP!! MURDER!! Someone help me!'",TRUE,ch,0,0,TO_ROOM);
	      sprintf(buffer,"Help, I'm being attacked by %s!",GET_NAME(ch->specials.fighting));
	      do_gen_com(ch,buffer,0,SCMD_SHOUT);
	      for (tch = character_list;tch;tch= tch->next) {
		if ((world[tch->in_room].zone == world[ch->in_room].zone)
		     && (mob_index[tch->nr].func == cityguard)
		    && !number(0,10)) {
		  tch->specials.hunting = ch;
		  hunt_victim(tch);
		}
	      }
	    }
	  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) 
	    if (IS_NPC(tch) && AWAKE(tch) && (tch != ch) &&
		abs(GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch)) < 200
		&& !number(0,4) && !(tch->specials.fighting)
		&& ((ctzn && MOB_FLAGGED(tch, MOB_CITIZEN))
		    || (GET_INT(tch) > 10
			|| MOB_FLAGGED(tch,MOB_HELPER))))
	      {
		act("$n enters the fray!",TRUE,tch,0,0,TO_ROOM);
		if (ch->specials.fighting) {
		  if (ch->specials.fighting == tch->master)
		    hit(tch,ch,TYPE_UNDEFINED);
		  else
		    hit(tch,ch->specials.fighting,TYPE_UNDEFINED);
		  break;
		}
		continue;
	      }
	}
      if (AWAKE(ch) && !(ch->specials.fighting)) {
	if (IS_SET(ch->specials2.act, MOB_WILL_LOOT)){
	  for (i=world[ch->in_room].contents;i;i = i->next_content){
	    if (GET_ITEM_TYPE(i) == ITEM_CONTAINER &&
		i->obj_flags.value[3] < 0)
	      add_event(number(200,299),EVENT_LOOT,0,0,0,0,0,ch,i);
	    continue;
	  }
	}
	if (IS_SET(ch->specials2.act, MOB_SCAVENGER)) { /* if scavenger */
	  if (world[ch->in_room].contents && !number(0, 5)) {
	    for (max = 1, best_obj = 0, obj = world[ch->in_room].contents; 
		 obj; obj = obj->next_content) {
	      if (CAN_GET_OBJ(ch, obj)) {
		if (obj->obj_flags.cost > max) {
		  best_obj = obj;
		  max = obj->obj_flags.cost;
		}
	      }
	    } /* for */
		     
	    if (best_obj &&
		(CAN_CARRY_W(ch)>(IS_CARRYING_W(ch)+GET_OBJ_WEIGHT(best_obj)))
		&& (IS_CARRYING_N(ch) < CAN_CARRY_N(ch))) {
	      obj_from_room(best_obj);
	      obj_to_char(best_obj, ch,0);
	      act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
	    }
	  }
	}/* Scavenger */

	if (!IS_SET(ch->specials2.act, MOB_SENTINEL) &&
	    (!IS_AFFECTED(ch, AFF_PARALYSIS)) &&
	    (GET_POS(ch) > POSITION_SITTING) && (GET_MOVE(ch) > 25) &&
	    (!ch->specials.fighting) && (!ch->master)){
	  if (((door = number(0, 60)) < NUM_OF_DIRS)
	      && CAN_GO(ch,door) && 
	      !IS_SET(world[real_room(EXIT(ch, door)->to_room)].room_flags, NO_MOB) && 
	      !IS_SET(world[real_room(EXIT(ch, door)->to_room)].room_flags, DEATH)) {
	    if (ch->specials.last_direction == door) 
	      ch->specials.last_direction = -1;
	    else if ((IS_SET(ch->specials2.act, MOB_STAY_ZONE) &&
		      (world[real_room(EXIT(ch, door)->to_room)].zone ==
		       world[ch->in_room].zone)) ||
		     (IS_SET(ch->specials2.act, MOB_STAY_SECTOR) &&
		      (world[real_room(EXIT(ch, door)->to_room)].sector_type ==
		       world[ch->in_room].sector_type)))
	      {
		ch->specials.last_direction = door;
		do_move(ch, "", ++door, 0);
	      }
	    else
	      {
		if (!IS_SET(ch->specials2.act, MOB_STAY_ZONE)
		    && !IS_SET(ch->specials2.act,MOB_STAY_SECTOR)){
		  ch->specials.last_direction = door;
		  do_move(ch, "", ++door, 0);
		}
	      }
	  }
	  else if (world[ch->in_room].obj &&
		   !IS_SET(world[ch->in_room].obj->obj_flags.value[1]
			   , CONT_ONEWAY) &&
		   !IS_SET(world[ch->in_room].obj->obj_flags.value[1]
			   , CONT_CLOSED) && !number(0,20))
	    do_leave(ch,"",0,0);
	  else {
	    for (obj = world[ch->in_room].contents
		   ;obj;obj = obj->next_content){
	      if (obj 
		  && (obj->description
		      && *obj->description != '\0')
		  && HASROOM(obj) && !number(0,20)
		  &&(!IS_SET(obj->obj_flags.value[1]
			     ,CONT_CLOSED))
		  &&(!IS_SET(obj->obj_flags.extra_flags
			     ,ITEM_DARK))){
		strcpy(buf,first_name(obj->name));
		do_enter(ch,buf,0,0);
		break;
	      }
	    }
	  }
	}
	if (!IS_SET(ch->specials2.act, MOB_SENTINEL) &&
	    (GET_POS(ch) == POSITION_STANDING)
	    && CAN_SPEAK(ch) && !ch->specials.fighting
	    && !ch->specials.hunting){
	  found_door = FALSE;
	  for (door=0;door < NUM_OF_DIRS;door++)
	    if(EXIT(ch,door) && EXIT(ch,door)->keyword ){
	      found_door = TRUE;
	      break;}
	  if (found_door && !number(0,40)){
	    half_chop(EXIT(ch,door)->keyword, buffer,buf);
	    if (!IS_SET(EXIT(ch,door)->exit_info,EX_SECRET)
		&& !IS_SET(EXIT(ch,door)->exit_info,EX_CLOSED))
	      do_close(ch,buffer,0,0);
	    else if (!IS_SET(EXIT(ch,door)->exit_info,EX_SECRET)
		     && IS_SET(EXIT(ch,door)->exit_info,EX_CLOSED))
	      do_open(ch,buffer,0,0);
	  }
	}
      }
	     
		   
      if (IS_SET(ch->specials2.act, MOB_AGGRESSIVE) && AWAKE(ch) &&
	 !ch->specials.fighting ) {
	found = FALSE;
	for (tmp_ch = world[ch->in_room].people; tmp_ch && !found; 
	     tmp_ch = tmp_ch->next_in_room) {
	  if (!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch)
	      && !PRF_FLAGGED(tmp_ch, PRF_NOHASSLE)
	      && !(IS_AFFECTED(tmp_ch,AFF_HIDE) &&
		   !can_see_hidden(ch,tmp_ch)) ) {
	    if (!IS_SET(ch->specials2.act, MOB_WIMPY) || !AWAKE(tmp_ch)) {
	      if ((IS_SET(ch->specials2.act, MOB_AGGRESSIVE_EVIL) && 
		   IS_EVIL(tmp_ch)) || 
		  (IS_SET(ch->specials2.act, MOB_AGGRESSIVE_GOOD) && 
		   IS_GOOD(tmp_ch)) || 
		  (IS_SET(ch->specials2.act, MOB_AGGRESSIVE_NEUTRAL) && 
		   IS_NEUTRAL(tmp_ch)) || 
		  (!IS_SET(ch->specials2.act, MOB_AGGRESSIVE_EVIL) && 
		   !IS_SET(ch->specials2.act, MOB_AGGRESSIVE_NEUTRAL) && 
		   !IS_SET(ch->specials2.act, MOB_AGGRESSIVE_GOOD) ))
		{
		  if (GET_POS(ch) < POSITION_STANDING)
		    do_stand(ch, "",0,0);
		  add_event(MAX(1,(number(30,40)-GET_PER(ch))/2)
			    ,EVENT_ATTACK, 0,0,0,0,0,ch, tmp_ch);
		  found = TRUE;
		}
	    }
	  }
	}
      } /* if aggressive */
      if (IS_SET(ch->specials2.act, MOB_MEMORY)
	  && ch->specials.memory && MOB_FLAGGED(ch,MOB_CITIZEN)
	  && AWAKE(ch) && !number(0,9))
	{
	  for (tmp_ch = world[ch->in_room].people;
	       tmp_ch; tmp_ch = tmp_ch->next_in_room)
	    if (IS_NPC(tmp_ch) && MOB_FLAGGED(tmp_ch,MOB_CITIZEN) &&
		IS_SET(tmp_ch->specials2.act, MOB_MEMORY) &&
		tmp_ch != ch && AWAKE(tmp_ch)){
	      present = TRUE;
	      if (tmp_ch->specials.memory){
		for (names = ch->specials.memory
		       ;names;names=names->next)
		  if (!hates_id(tmp_ch, names->id)){
		    present = FALSE;
		    remember_id(tmp_ch,names->id);}
	      }
	      else{
		for (names = ch->specials.memory; names; names = names->next){
		  remember_id(tmp_ch, names->id);
		  present = FALSE;}
	      }
	      if (!present)
		act("$n whispers to $N.",FALSE,ch,0,tmp_ch,TO_ROOM);
			    
	    }
			
	}
      if(IS_SET(ch->specials2.act, MOB_MEMORY) && ch->specials.memory) {
	for (vict = 0, tmp_ch = world[ch->in_room].people; 
	     tmp_ch && !vict; tmp_ch = tmp_ch->next_in_room)
	  if (hates(ch,tmp_ch) && CAN_SEE(ch, tmp_ch))
	    vict = tmp_ch;
	if (vict) {
	  if ((IS_SET(ch->specials2.act, MOB_WIMPY) &&
	       (number(0,30) < GET_PER(ch)))
	      && (GET_HIT(ch) < GET_MAX_HIT(ch)/5)){
	    if (CAN_SPEAK(ch) && AWAKE(ch))
	      act("$n screams 'Leave me alone!!'",
		  FALSE, ch, 0, 0, TO_ROOM);
	    else if (AWAKE(ch))
	      act("$n whimpers.",FALSE, ch, 0, 0, TO_ROOM);
	    do_flee(ch,"",0,0);
	  }
	  else {
	    if (AWAKE(ch) && !ch->specials.fighting){
	      if (!IS_SET(world[ch->in_room].room_flags, PEACEFULL)){
		if (CAN_SEE(ch, vict))
		  add_event(MAX(1,(number(30,40)-GET_PER(ch))/2)
			    ,EVENT_ATTACK, 0,0,0,0,0,ch, vict);
		else if (CAN_SPEAK(ch))
		  act("$n growls, 'Where are you $N. I know you're here somewhere!'", FALSE, ch, 0, vict, TO_ROOM);
		else
		  act("$n growls!", FALSE, ch, 0, vict, TO_ROOM);
	      }
	      else{
		if (CAN_SEE(ch, vict))		
		  act("$n growls, 'Step outside $N. Let's settle this!'",
		      FALSE, ch, 0, vict, TO_ROOM);
		else if (CAN_SPEAK(ch))
		  act("$n growls, 'Where are you $N. I know you're here somewhere!'", FALSE, ch, 0, vict, TO_ROOM);
		else
		  act("$n growls!", FALSE, ch, 0, vict, TO_ROOM);
	      }
	    }
	  }
	}
      } /* mob memory */
    } /* If IS_MOB(ch)  */
}




/* Mob Memory Routines */

/* make ch remember victim */
void	remember (struct char_data *ch, struct char_data *victim)
{
    memory_rec * tmp;
    bool present = FALSE;
    
    if (!IS_NPC(ch) || IS_NPC(victim) || GET_LEVEL(victim) >= LEVEL_BUILDER) 
	return;

    for (tmp = ch->specials.memory; tmp && !present; tmp = tmp->next)
	if (tmp->id == GET_IDNUM(victim))
	    present = TRUE;

    if (!present) {
	CREATE(tmp, memory_rec, 1);
	tmp->next = ch->specials.memory;
	tmp->id = GET_IDNUM(victim);
	ch->specials.memory = tmp;
    }
}
void	remember_id(struct char_data *ch, long id)
{
    memory_rec * tmp;
    bool present = FALSE;
    
    if (!IS_NPC(ch)) 
	return;
    if (id <= 0)
	return;
    
    for (tmp = ch->specials.memory; tmp && !present; tmp = tmp->next)
	if (tmp->id == id)
	    present = TRUE;

    if (!present) {
	CREATE(tmp, memory_rec, 1);
	tmp->next = ch->specials.memory;
	tmp->id = id;
	ch->specials.memory = tmp;
    }
}

/* make all mobs forget victim */
void	forget (struct char_data *victim)
{
  struct char_data *ch;
  memory_rec *curr, *prev;
  
  for (ch = character_list;ch;ch = ch->next){
    if (IS_MOB(ch) && ch->specials.hunting == victim)
      ch->specials.hunting =0;
    if (IS_MOB(ch) && IS_SET(ch->specials2.act, MOB_MEMORY))
      if (ch->specials.memory){
	curr = ch->specials.memory;
	while (curr && curr->id != GET_IDNUM(victim)) {
	  prev = curr;
	  curr = curr->next;
	}	
	if(curr){
	  if(curr == ch->specials.memory)
	    ch->specials.memory = curr->next;
	  else
	    prev->next = curr->next;
	  free(curr);
	}
      }
  }
}

bool hates(struct char_data *ch, struct char_data *vict)
{
    memory_rec *names;
    if (!IS_NPC(ch) || IS_NPC(vict))
	return(FALSE);
    if (!IS_SET(ch->specials2.act, MOB_MEMORY))
	return(FALSE);
    if (!ch->specials.memory)
	return(FALSE);
    for (names = ch->specials.memory;names; names = names->next)
	if (names->id == GET_IDNUM(vict))
	    return(TRUE);
    return(FALSE);
}
bool hates_id(struct char_data *ch, long id)
{
    memory_rec *names;
    if (!IS_NPC(ch))
	return(FALSE);
    if (!IS_SET(ch->specials2.act, MOB_MEMORY))
	return(FALSE);
    if (!ch->specials.memory)
	return(FALSE);
    for (names = ch->specials.memory;names; names = names->next)
	if (names->id == id)
	    return(TRUE);
    return(FALSE);
}


/* erase ch's memory */
void	clearMemory(struct char_data *ch)
{
    memory_rec *curr, *next;

    curr = ch->specials.memory;

    while (curr) {
	next = curr->next;
	free(curr);
	curr = next;
    }

    ch->specials.memory = NULL;
}





void mob_cast(struct char_data *ch)
{
  ACMD(do_invoke);
  ACMD(do_sit);
  ACMD(do_stand);
  ACMD(do_rest);    
  int spell_num=-1,i;
  char buffer[100];
  struct char_data *target;
  bool sitting=0, resting=0;
  *buffer = '\0';
  
  if (ch->specials.timer){
    ch->specials.timer--;
    return;
  }
  if (GET_LEVEL(ch) < 10)
    return;
  if (ch->specials.fighting){
    if (!number(0,1)){
      /* offensive spells */
      while (spell_num == -1){
	for (i=0;combat_other[i].index != -1;i++)
	  if (spell_info[combat_other[i].index].min_level <= 2*GET_LEVEL(ch)
	      && !number(0,20)){
	    spell_num = i;
	    break;
	  }
	if (!can_see_char(ch, ch->specials.fighting) && !number(0,2))
	  sprintf(buffer,"'reveal the lost image'");
	else
	  sprintf(buffer,"'%s' %s",combat_other[spell_num].entry,GET_NAME(ch->specials.fighting));
	do_invoke(ch,buffer,0,0);
      }
    }
    else{
	/* heal self spells */
      while (spell_num == -1){	
	for (i=0;combat_self[i].index != -1;i++)
	  if (spell_info[combat_self[i].index].min_level <= 2*GET_LEVEL(ch)
	      && !number(0,20)){
	    spell_num = i;
	    break;
	  }
	if (!can_see_char(ch, ch->specials.fighting) && !number(0,2))
	  sprintf(buffer,"'discern image' self");
	else
	  sprintf(buffer,"'%s' self",combat_self[spell_num].entry);
	do_invoke(ch,buffer,0,0);
      }
    }
  }
  else if (GET_ALIGNMENT(ch) > 500 && !number(0,50))
    {
      /* benevolent spells on chars */
      for (target = world[ch->in_room].people;
	   target;
	   target = target->next_in_room){
	if (can_see_char(ch, target) && (GET_ALIGNMENT(target) > -500)) {
	  while (spell_num == -1){
	    for (i=0;mob_beneficial[i].index != -1;i++)
	      if ((spell_info[mob_beneficial[i].index].min_level
		   <= 2*GET_LEVEL(ch))&& !number(0,20)){
		spell_num = i;
		break;
	      }
	    sprintf(buffer,"'%s' %s",mob_beneficial[spell_num].entry, GET_NAME(target));
	    if (GET_POS(ch) == POSITION_SITTING)
	      sitting = TRUE;
	    if (GET_POS(ch) == POSITION_RESTING)
	      resting = TRUE;
	    do_stand(ch,"",0,0);
	    do_invoke(ch,buffer,0,0);
	    if (sitting || resting)
	      do_sit(ch,"",0,0);
	    if (resting)
	      do_rest(ch,"",0,0);
	  }
	  
	}
      }
    }
}
