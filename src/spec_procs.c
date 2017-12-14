/* ************************************************************************
 *   File: spec_procs.c                                  Part of CircleMUD *
 *  Usage: implementation of special procedures for mobiles/objects/rooms  *
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
#include <ctype.h>
#include <sys/types.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"


 /*   external vars  */
extern struct spell_info_type spell_info[];
extern  struct list_index_type *guild_list[];
extern  struct list_index_type am_skills[];
extern struct str_app_type str_app[];
extern struct obj_data *obj_proto;
extern struct index_data *obj_index;	
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern int hyper_soc[];
extern int happy_soc[];
extern int cheery_soc[];
extern int neutral_soc[];
extern int sad_soc[];
extern int grouchy_soc[];
extern int homicidal_soc[];
extern int opp_sex_soc[];
/* extern functions */
SPECIAL(cityguard);
bool check_guard(struct char_data *thief, struct char_data *victim);
int	add_follower(struct char_data *ch, struct char_data *leader);
bool    circle_follow(struct char_data *ch, struct char_data * victim);
void	stop_follower_quiet(struct char_data *ch);    
struct char_data *give_find_vict(struct char_data *ch, char *arg1);
int     report_money_weight(int amount);
char    *report_cost(int amount);
void    drop_excess_gold(struct char_data *ch, int amount);
int     report_highest_value_for_weight(int weight);
bool    is_maxed(int num, int mode);
void    add_event(int plse, int event, int inf1, int inf2, int inf3, int inf4, char *arg, void *subj, void *vict);
int     report_crowns(int amount);
int     report_groats(int amount);
int     report_pennies(int amount);
void poison(struct char_data *ch, struct char_data *victim, struct obj_data *obj);
ACMD(do_wake);
struct char_data *find_npc_world(struct char_data *ch);
void reimund_story(struct char_data *me);
struct char_data *find_vict_world(struct char_data *ch);
 struct social_type {
     char	*cmd;
     int	next_line;
 };


 /* ********************************************************************
 *  Special procedures for rooms                                       *
 ******************************************************************** */


SPECIAL(pet_shop)
{
   char	buf[MAX_STRING_LENGTH], pet_name[256];
   int	pet_room, nfols=0;
   struct follow_type *k;
   struct char_data *pet, *keeper;

   if (IS_AFFECTED((struct char_data *) me, AFF_PARALYSIS))
      return(FALSE);
   keeper = (struct char_data *) me;
   
   if (mob_index[keeper->nr].virtual == 2069) /* tegl */
     pet_room = real_room(1882);
   else 
     pet_room = 0;
   if (cmd == 59) { /* List */
     if (!CAN_SEE(keeper, ch)){
       act("$n says, 'I don't trade with folks I can't see.'",
	   FALSE, keeper, 0, 0, TO_ROOM);
       return;
     }
     send_to_char("Available mounts are:\n\r", ch);
      for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
	 sprintf(buf, "%s (%s)- %s\n\r",pet->player.short_descr,
		 SIZE(GET_SIZE(pet)), report_cost(50 * GET_MAX_MOVE(pet)));
	 send_to_char(buf, ch);
      }
      return(TRUE);
   } else if (cmd == 56) { /* Buy */

     if (!CAN_SEE(keeper, ch)){
       act("$n says, 'I don't trade with folks I can't see.'",
	   FALSE, keeper, 0, 0, TO_ROOM);
       return;
     }
      arg = one_argument(arg, buf);
      arg = one_argument(arg, pet_name);
      /* Pet_Name is for later use when I feel like it */

      if (!(pet = get_char_room(buf, pet_room))) {
	 send_to_char("There is no such animal!\n\r", ch);
	 return(TRUE);
      }

      if (GET_GOLD(ch) < (GET_MAX_MOVE(pet) * 50)) {
	 send_to_char("You don't have enough gold!\n\r", ch);
	 return(TRUE);
      }
      
      for (k = ch->followers;k;k = k->next)
	nfols++;
      
      if (nfols > MAX(1,GET_CHR(ch)/3)){
	send_to_char("You have too many followers.\r\n",ch);
	return(TRUE);
      }
      change_gold(ch, -GET_MAX_MOVE(pet)*50);

      /*      pet = read_mobile(pet->nr, REAL); */
      char_from_room(pet);
      char_to_room(pet, ch->in_room, FALSE);
      GET_EXP(pet) = 0;
      SET_BIT(pet->specials.affected_by, AFF_CHARM);
      /*      GET_HEIGHT(pet) = GET_HEIGHT(ch);*/
      if (*pet_name) {
	 sprintf(buf, "%s %s", pet->player.name, pet_name);
	 /* free(pet->player.name); don't free the prototype! */
	 pet->player.name = str_dup(buf);

	 sprintf(buf, "%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
	     pet->player.description, pet_name);
	 /* free(pet->player.description); don't free the prototype! */
	 pet->player.description = str_dup(buf);
      }

      add_follower(pet, ch);

      act("$n buys $N.", FALSE, ch, 0, pet, TO_ROOM);
      act("$n says, 'May you enjoy your mount.'",FALSE,keeper,0,0, TO_ROOM);

      return(TRUE);
   }

   /* All commands except list and buy */
   return(FALSE);
}

/* SPECIAL(ferry) */
/* { */
/*     int curr_room; */

    
/*     if (cmd >= 0) */
/* 	return(FALSE); */
    
/*     curr_room = ch->in_room; */
/*     char_from_room(ch); */
/*     char_to_room(ch,curr_room + 1, FALSE); */
/*     send_to_char("\r\n",ch); */
/*     send_to_char(world[ch->in_room].description,ch); */
/*     return(FALSE); */
/* } */

/* SPECIAL(to_arg_isle) */
/* { */
/*     ACMD(do_look); */

/*     if (cmd >= 0) */
/* 	return(FALSE); */
    
/*     char_from_room(ch); */
/*     char_to_room(ch,14004, TRUE); */
/*     act("$n disembarks from the Lamman Ferry.",TRUE,ch,0,0,TO_ROOM); */
/*     act("You disembark from the Lamman Ferry.",TRUE,ch,0,0,TO_CHAR); */
/*     do_look(ch,"",0,0); */
/*     return(FALSE); */
/* } */

SPECIAL(leviathan)
{
    ACMD(do_look);
    struct char_data *vict=0;

    if (cmd >= 0)
	return(0);        
    if(!AWAKE(ch))
       return(FALSE);
       
    if (ch->specials.fighting)
	vict = ch->specials.fighting;
    else {
	for (vict = world[ch->in_room].people;
	     vict;
	     vict = vict->next_in_room  )
	    if (vict != ch && !number(0,5))
		break;
    }
    if (!vict)
	return(FALSE);
    if(vict->specials.fighting){
	if (vict->specials.fighting->specials.fighting == vict)
	    stop_fighting(vict->specials.fighting);
	stop_fighting(vict);
    }
    char_from_room(vict);
    char_to_room(vict,5001, TRUE);
    act("Suddenly $n sucks in a huge amount of water.",FALSE,ch,0,0,TO_ROOM);
    act("You are picked up and hurled about like a feather.",FALSE,ch,0,vict,TO_VICT);
    act("$N is sucked into $n's gaping maw.",FALSE,ch,0,vict,TO_NOTVICT);
    act("There is a massive influx of water.",FALSE,vict,0,0,TO_ROOM);
    act("$n is dumped right in front of you.",FALSE,vict,0,0,TO_ROOM);     
    do_look(vict,"",0,0);
    GET_POS(vict) = POSITION_SITTING;
    return(FALSE);

}

SPECIAL (blacksmith)
{
    ACMD(do_tell);
    ACMD(do_say);
    struct obj_data *obj=0;
    struct char_data *vict=0;
    int cost, npts;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buff[100];

    if (cmd <0)
	return(0);
    
    strcpy(arg3,arg);
    if (cmd != 72)
	return(FALSE);

    half_chop(arg3,arg1,arg2);
    
    if (is_number(arg1))
	return(FALSE);
    if (!(vict = give_find_vict( ch, arg3)))
	return(FALSE);
    if (IS_AFFECTED(vict, AFF_PARALYSIS))
	return(FALSE);

    /* Stumpy Dumpy blacksmith and tailor */
    if ((mob_index[vict->nr].virtual != 2051) &&
	(mob_index[vict->nr].virtual != 2052) && 
	(mob_index[vict->nr].virtual != 11020)&&
	(mob_index[vict->nr].virtual != 11211)&&	
	(mob_index[vict->nr].virtual != 11087) 	 
	)
	return(FALSE);
    else {
	half_chop(arg3,arg1,arg2);
    
	if (!(obj= get_obj_in_list_vis(ch,arg1,ch->inventory))){
	    send_to_char("You don't seem to have that item.\r\n",ch);
	    return(TRUE);}

	
	
	act("You give $p to $N.",FALSE,ch,obj,vict,TO_CHAR);
	act("$n gives $p to $N.",FALSE,ch,obj,vict,TO_NOTVICT);
	act("$N looks at $p.",FALSE,ch,obj,vict,TO_NOTVICT);
    
	if ((obj->obj_flags.type_flag != ITEM_ARMOR
	     && obj->obj_flags.type_flag != ITEM_WORN)
	    && mob_index[vict->nr].virtual == 2051)
	  do_say(vict,"I only fix clothing and armours.",0,0);
	else if (obj->obj_flags.type_flag != ITEM_WEAPON
		 && obj->obj_flags.type_flag != ITEM_SCABBARD
		 && mob_index[vict->nr].virtual == 2052)
	    do_say(vict,"I only fix weapons and scabbards.",0,0);
	else if ((obj->obj_flags.type_flag != ITEM_ARMOR		 
		  && obj->obj_flags.type_flag != ITEM_SCABBARD
		  && obj->obj_flags.type_flag != ITEM_WEAPON) &&
		 mob_index[vict->nr].virtual == 11020)
	  do_say(vict,"I only fix weapons and armours.",0,0);
	else if (obj->obj_flags.type_flag != ITEM_WORN &&
		 mob_index[vict->nr].virtual == 11087)
	  do_say(vict,"I only fix clothing.",0,0);
	else if (obj->obj_flags.type_flag != ITEM_TREASURE &&
		 mob_index[vict->nr].virtual == 11211)	  
	  do_say(vict,"I only fix treasure.",0,0);
	else if (obj->obj_flags.value[4] == 0)
	  do_say(vict,"Looks fine to me.",0,0);
	else {
	    npts = obj->obj_flags.value[4];
	    cost = (float)(npts*obj->obj_flags.cost)/10.;
	    if (GET_GOLD(ch) < cost){
	      sprintf(buff,"$N tells you, 'It will cost you %s to fix it.'\r\n"
		      "$N tells you, '...which I see you can't afford.'",report_cost(cost));
	      act(buff,FALSE,ch,0,vict,TO_CHAR);
	    }
	    else {
	      sprintf(buff,"$N tells you, 'It will cost you %s to fix it'",report_cost(cost));
	      act(buff,FALSE,ch,0,vict,TO_CHAR);
	      change_gold(ch, -cost);
	      obj->obj_flags.value[4] = 0; 
	      act("$N fiddles with $p for a while.",FALSE,ch,obj,vict,TO_ROOM);
	      act("$N fiddles with $p for a while.",FALSE,ch,obj,vict,TO_CHAR);
	      do_say(vict,"All fixed",0,0);
	    }
	}
	if (!number(0,99)){
	  obj->obj_flags.value[4] -= number(0,5);
	  do_say(vict,"I did a superb job, I really ought to charge you extra",0,0);}
	act("$N gives $p to $n.",FALSE,ch,obj,vict,TO_NOTCHAR);
	act("$N gives you $p.",FALSE,ch,obj,vict,TO_CHAR);
	return(TRUE);
    }
    
}
SPECIAL(animate_book)
{
    char arg3[MAX_INPUT_LENGTH];
    struct obj_data *obj=0, *book=0;
    struct char_data *mob=0;

    if (cmd < 0)
	return(0);
    
    strcpy(arg3,arg);

    book = (struct obj_data *)me;
    
    if (cmd != 10 && cmd != 167)
	return(FALSE);
    
    if (!strstr(arg3,"book") && !strstr(arg3,"all"))
	return(FALSE);
    if (!(obj = get_obj_in_list_vis(ch, "book", world[ch->in_room].contents)))
	return(FALSE);
    if (obj != book)
	return(FALSE); 
    extract_obj(book,0);
    mob = read_mobile(2127,VIRTUAL);
    char_to_room(mob,ch->in_room, FALSE);
    act("$p suddenly animates!",FALSE,ch,obj,0,TO_ROOM);
    act("$p suddenly animates!",FALSE,ch,obj,0,TO_CHAR);
    hit(mob,ch,0);
    return(FALSE);
}
SPECIAL (armourer)
{
    ACMD(do_tell);
    ACMD(do_say);
    struct obj_data *obj=0;
    struct char_data *vict=0;
    int cost, objs, chars, npts, factor;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buff[100];

    if (cmd < 0)
	return(0);
    strcpy(arg3,arg);
    if (cmd != 72)
	return(FALSE);

    half_chop(arg3,arg1,arg2);
    
    if (is_number(arg1))
	return(FALSE);

    if (!(vict = give_find_vict( ch, arg3)))
	return(FALSE);
    if (IS_AFFECTED(vict, AFF_PARALYSIS))
	return(FALSE);
    switch (mob_index[vict->nr].virtual)
	{
	case 1832:
	    factor = 5;
	    break;
	default:
	    break;
    }
    if ((mob_index[vict->nr].virtual != 1832)) /* Trona */
	return(FALSE);
    else {
	half_chop(arg3,arg1,arg2);
    
	if (!(obj= get_obj_in_list_vis(ch,arg1,ch->inventory))){
	    send_to_char("You don't seem to have that item.\r\n",ch);
	    return(TRUE);}

	
	
	act("You give $p to $N.",FALSE,ch,obj,vict,TO_CHAR);
	act("$n gives $p to $N.",FALSE,ch,obj,vict,TO_ROOM);
	act("$N looks at $p.",FALSE,ch,obj,vict,TO_NOTVICT);
    
	if (mob_index[vict->nr].virtual == 1832 &&
	    (obj->obj_flags.type_flag != ITEM_ARMOR
	     && obj->obj_flags.type_flag != ITEM_SCABBARD
	     && obj->obj_flags.type_flag != ITEM_WORN))
	    do_say(vict,"I can't resize that for you, sorry.",0,0);	    
	else if(mob_index[vict->nr].virtual == 11086 
		&& obj->obj_flags.type_flag != ITEM_SCABBARD &&
		obj->obj_flags.type_flag != ITEM_ARMOR)
	    do_say(vict,"I only resize armours, sorry.",0,0);	    
	else if (mob_index[vict->nr].virtual == 11013 &&
		 obj->obj_flags.type_flag != ITEM_WORN)
	    do_say(vict,"I only resize clothing, sorry.",0,0);
	else if (mob_index[vict->nr].virtual == 7154 &&
		 obj->obj_flags.type_flag != ITEM_WORN)
	    do_say(vict,"I only resize clothing, sorry.",0,0);
	else if (((mob_index[vict->nr].virtual == 7036) ||
		     (mob_index[vict->nr].virtual == 26072))
		 && obj->obj_flags.type_flag != ITEM_TREASURE)
	    do_say(vict,"I only resize jewlery and treasures, sorry.",0,0);	
	else if (IS_SET(obj->obj_flags.extra_flags, ITEM_RESIZED))
	    {
		do_say(vict,"This object has already been worked on.",0,0);
		do_say(vict,"I can't resize it for you, sorry.",0,0);
	    }
	else if (obj->obj_flags.value[5] == GET_SIZE(ch))
	    do_say(vict,"Seems to fit just fine.",0,0);
	else {
	    objs = obj->obj_flags.value[5];
	    chars = GET_SIZE(ch);
	    if (chars < objs && abs(chars - objs) > 3){
		do_say(vict,"Sorry it is too small to make fit.",0,0);
		return(TRUE);}
	    else
		npts = abs(chars-objs);
	    cost = (float)(npts*factor*obj->obj_flags.cost)/100.;
	    if (GET_GOLD(ch) < cost){
		sprintf(buff,"$N tells you, 'It will cost you %s to size it.'\r\n"
			"$N tells you, '...which I see you can't afford.'",report_cost(cost));
		act(buff,FALSE,ch,0,vict,TO_CHAR);
	    }
	    else {
		sprintf(buff,"$N tells you, 'It will cost you %s to size it.'",report_cost(cost));
		act(buff,FALSE,ch,0,vict,TO_CHAR);
		change_gold(ch, -cost);

		IS_CARRYING_W(ch) -= GET_OBJ_WEIGHT(obj);
		obj->obj_flags.value[5] = GET_SIZE(ch);
		GET_OBJ_WEIGHT(obj)
		  = (GET_OBJ_WEIGHT(obj)*(9 - GET_OBJ_SIZE(obj)))/7;
		IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj);
		SET_BIT(obj->obj_flags.extra_flags,ITEM_RESIZED);
		act("$N sizes $n.",FALSE,ch,obj,vict,TO_ROOM);
		act("$N sizes you.",FALSE,ch,obj,vict,TO_CHAR);	
		act("$N fiddles with $p for a while.",FALSE,ch,obj,vict,TO_ROOM);
		act("$N fiddles with $p for a while.",FALSE,ch,obj,vict,TO_CHAR);
		do_say(vict,"All done",0,0);
	    }
	}

	act("$N gives $p to $n.",FALSE,ch,obj,vict,TO_NOTCHAR);
	act("$N gives you $p.",FALSE,ch,obj,vict,TO_CHAR);
	return(TRUE);
    }
    
}

 SPECIAL(dump)
 {
     struct obj_data *k=0;
     struct char_data *tmp_char=0;
     int	value = 0;

     ACMD(do_drop);
     char	*fname(char *namelist);
     if (cmd < 0)
	 return(0);
     for (k = world[ch->in_room].contents; k ; k = world[ch->in_room].contents) {
	 sprintf(buf, "The %s vanishes in a puff of smoke.\r\n" , fname(k->name));
	 for (tmp_char = world[ch->in_room].people; tmp_char; 
						    tmp_char = tmp_char->next_in_room)
	     if (CAN_SEE_OBJ(tmp_char, k))
		 send_to_char(buf, tmp_char);
	 extract_obj(k,0);
     }

     if (cmd != 60)
	 return(FALSE);

     do_drop(ch, arg, cmd, 0);

     value = 0;

     for (k = world[ch->in_room].contents; k ; k = world[ch->in_room].contents) {
	 sprintf(buf, "The %s vanishes in a puff of smoke.\r\n", fname(k->name));
	 for (tmp_char = world[ch->in_room].people; tmp_char; 
						    tmp_char = tmp_char->next_in_room)
	     if (CAN_SEE_OBJ(tmp_char, k))
		 send_to_char(buf, tmp_char);
	 value += MAX(1, MIN(50, k->obj_flags.cost / 10));

	 extract_obj(k,0);
     }

     if (value) {
	 act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
	 act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

	 if (GET_LEVEL(ch) < 3)
	     gain_exp(ch, value,1);
	 else
	     drop_excess_gold(ch,value);
     }

     return(TRUE);
 }


 /* ********************************************************************
 *  General special procedures for mobiles                             *
 ******************************************************************** */

 /* SOCIAL GENERAL PROCEDURES

 If first letter of the command is '!' this will mean that the following
 command will be executed immediately.

 "G",n      : Sets next line to n
 "g",n      : Sets next line relative to n, fx. line+=n
 "m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
 "w",n      : Wake up and set standing (if possible)
 "c<txt>",n : Look for a person named <txt> in the room
 "o<txt>",n : Look for an object named <txt> in the room
 "r<int>",n : Test if the npc in room number <int>?
 "s",n      : Go to sleep, return false if can't go sleep
 "e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
	      contents of the **thing
 "E<txt>",n : Send <txt> to person pointed to by thing
 "B<txt>",n : Send <txt> to room, except to thing
 "?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
	      Will as usual advance one line upon sucess, and change
	      relative n lines upon failure.
 "O<txt>",n : Open <txt> if in sight.
 "C<txt>",n : Close <txt> if in sight.
 "L<txt>",n : Lock <txt> if in sight.
 "U<txt>",n : Unlock <txt> if in sight.    */

 /* Execute a social command.                                        */
    void	exec_social(struct char_data *npc, char *cmd, int next_line,
		     int *cur_line, void **thing)
	{
	    bool ok;

	    ACMD(do_move);
	    ACMD(do_open);
	    ACMD(do_lock);
	    ACMD(do_unlock);
	    ACMD(do_close);


	    if(npc->specials.fighting)
		return;
	    
	    ok = TRUE;

	    switch (*cmd) {

	    case 'G' :
		*cur_line = next_line;
		return;

	    case 'g' :
		*cur_line += next_line;
		return;

	    case 'e' :
		act(cmd + 1, FALSE, npc, *thing, *thing, TO_ROOM);
		break;

	    case 'E' :
		act(cmd + 1, FALSE, npc, 0, *thing, TO_VICT);
		break;

	    case 'B' :
		break;

	    case 'm' :
		do_move(npc, "", *(cmd + 1) - '0' + 1, 0);
		break;

	    case 'w' :
		if (GET_POS(npc) != POSITION_SLEEPING)
		    ok = FALSE;
		else
		    GET_POS(npc) = POSITION_STANDING;
		break;

	    case 's' :
		if (GET_POS(npc) <= POSITION_SLEEPING)
		    ok = FALSE;
		else
		    GET_POS(npc) = POSITION_SLEEPING;
		break;

	    case 'c' :  /* Find char in room */
		*thing = get_char_room_vis(npc, cmd + 1);
		ok = (*thing != 0);
		break;

	    case 'o' : /* Find object in room */
		*thing = get_obj_in_list_vis(npc, cmd + 1, world[npc->in_room].contents);
		ok = (*thing != 0);
		break;

	    case 'r' : /* Test if in a certain room */
		ok = (npc->in_room == atoi(cmd + 1));
		break;

	    case 'O' : /* Open something */
		do_open(npc, cmd + 1, 0, 0);
		break;

	    case 'C' : /* Close something */
		do_close(npc, cmd + 1, 0, 0);
		break;

	    case 'L' : /* Lock something  */
		do_lock(npc, cmd + 1, 0, 0);
		break;

	    case 'U' : /* UnLock something  */
		do_unlock(npc, cmd + 1, 0, 0);
		break;

	    case '?' : /* Test a random number */
		if (atoi(cmd + 1) <= number(1, 100))
		    ok = FALSE;
		break;

	    default:
		break;
	    }  /* End Switch */

	    if (ok)
		(*cur_line)++;
	    else
		(*cur_line) += next_line;
	}





SPECIAL(snake)
{
    if (cmd >=0)
	return(FALSE);

    if (!ch->specials.fighting)
	return FALSE;
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);

    if (ch->specials.fighting && 
	(ch->specials.fighting->in_room == ch->in_room) && 
	(number(0, 42 - GET_LEVEL(ch)) == 0)) {
	act("$N bites $n!", 1, ch->specials.fighting, 0, ch, TO_ROOM);
	act("$n bites you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
	poison(ch, ch->specials.fighting, 0);
	return TRUE;
    }
    return FALSE;
}
void poison(struct char_data *ch, struct char_data *victim, struct obj_data *obj)
{
   struct affected_type af, af2;
   assert(victim || obj);
   
   if (IS_AFFECTED(victim, AFF_POISON))
       return;
       
   if (victim) {
      if (!saves_spell(victim, SAVING_PARA,GET_LEVEL(ch))) {
	 af.duration = GET_LEVEL(ch)/10 + number(0,GET_LEVEL(ch))/10 + 5;
	 af.modifier = -8;
	 af.location = APPLY_STR;
	 af.bitvector = AFF_POISON;
	 affect_join(victim, &af, FALSE, FALSE);

	 af2.duration = GET_LEVEL(ch)/10 + number(0,GET_LEVEL(ch))/10 + 5;
	 af2.modifier = number(0,3);
	 af2.location = APPLY_NONE;
	 af2.bitvector = AFF_POISON;
	 affect_join(victim, &af2, FALSE, FALSE);
	 send_to_char("You feel very sick.\r\n", victim);
	 poison_vict(victim);
      }

   } else { /* Object poison */
      if ((obj->obj_flags.type_flag == ITEM_DRINKCON) || 
          (obj->obj_flags.type_flag == ITEM_FOUNTAIN) || 
          (obj->obj_flags.type_flag == ITEM_FOOD)) {
	 obj->obj_flags.value[3] = 1;
      }
   }
   
}

bool npc_steal_coins(struct char_data *ch, struct char_data *victim)
{
  int	gold;
  bool status=TRUE;
  int total = 0;
  
  if (IS_NPC(victim))
    return(FALSE);
  
  total = number(0, GET_LEVEL(ch) + GET_DEX(ch));

  if (AWAKE(victim) 
      && ( total < GET_PER(victim))) 
    {
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
      act("$N tries to steal gold from $n.",TRUE,victim,0,ch,TO_ROOM);
      status=FALSE;
    }
  else if(!AWAKE(victim) && ( total < GET_PER(victim)/5))
    {
      do_wake(victim, "", 0, 0);
      act("You wake to discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
      act("$N tries to steal gold from $n.",TRUE,victim,0,ch,TO_ROOM);
      status=FALSE;
    }
  else if((AWAKE(victim) && ( total < 2 * GET_PER(victim))) 
           || (!AWAKE(victim) && ( total < 2 * GET_PER(victim)/5)))
    {
    /*do nothing...*/
    status = FALSE;
    }
  else
    {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
      if (gold > 0) 
	{
	  change_gold(ch, gold);
	  change_gold(victim, -gold);
	}
    }
  return(status);
}

bool npc_steal_inv(struct char_data *ch, struct char_data *victim)
{
  struct obj_data *max_obj=0;
  struct obj_data *vict_obj=0;
  int target;
  bool status=TRUE;
  
  if (IS_NPC(victim))
    return(FALSE);
  
  for(vict_obj = victim->inventory,max_obj = victim->inventory;
      vict_obj;
      vict_obj = vict_obj->next_content)
    {
      if(vict_obj->obj_flags.cost > max_obj->obj_flags.cost)
	max_obj = vict_obj;
    }
  if (!max_obj)
    return(0);
  
  target = GET_LEVEL(ch)+GET_DEX(ch)+GET_STR(ch)-GET_OBJ_WEIGHT(max_obj)/2;
  if (AWAKE(victim) && ((number(0, MAX(1,target)) < GET_PER(victim)))) {
    act("You discover that $n has $s hands on your possessions.",
	FALSE, ch, 0, victim, TO_VICT);
    act("$N tries to steal something from $n.",TRUE,victim,0,ch,TO_ROOM);
    status=FALSE;
  }
  else if (!AWAKE(victim)&& ((number(0, MAX(1,target)) < GET_PER(victim)/5))) {
    do_wake(victim, "", 0, 0);
    act("You are startled awake by $n going through your belongings.",
	FALSE, ch, 0, victim, TO_VICT);
    act("$n quickly wakes and jumps to $s feet when $N tries to steal something from $m.",TRUE,victim,0,ch,TO_ROOM);
    status=FALSE;
  }
   else if((AWAKE(victim) && (number(0, MAX(1,target)) < 2*GET_PER(victim)))
           || (!AWAKE(victim) && 
              (number(0, MAX(1,target)) < 2*GET_PER(victim)/5)))
    {
    /*do nothing...*/
    status = FALSE;
    } else 
    {
      /* Steal the item */
      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	if (IS_GODITEM(max_obj) && GET_LEVEL(ch) < LEVEL_BUILDER){
	  return;}
	if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(max_obj)) < CAN_CARRY_W(ch)) {
	  obj_from_char(max_obj,0);
	  obj_to_char(max_obj, ch,0);
	}
      } 
      else
	return FALSE;
    }
  return(status);
}

bool npc_steal_equiped(struct char_data *ch, struct char_data *victim)
{
  struct obj_data *vict_obj=0;
  int target, pos, difficulty;
  bool status=TRUE;
  
  if (IS_NPC(victim))
    return(FALSE);
  
  switch(number(0,6))
    {
    case 0:
      pos = WEAR_SCABBARD;
      difficulty = 2;
      break;
    case 1:
      pos = WEAR_FINGER_R;
      difficulty = 1;	
      break;
    case 2:
      pos = WEAR_FINGER_L;
      difficulty = 1;	
      break;
    case 3:
      pos = WEAR_WRIST_L;
      difficulty = 1;	
      break;
    case 4:
      pos = WEAR_WRIST_R;
      difficulty = 1;	
      break;
    case 5:
      pos = WEAR_EARS_R;
      difficulty = 1;
      break;
    case 6:
      pos = WEAR_EARS_L;
      difficulty = 1;	
      break;
    default:
      return FALSE;
      break;
    }
  
  if (victim->equipment[pos] && CAN_SEE_OBJ(ch, victim->equipment[pos]))
    vict_obj = victim->equipment[pos];
  
  if(!vict_obj)
    return FALSE;
  else { /* It is equipment */
    target = GET_LEVEL(ch)+GET_DEX(ch)+GET_STR(ch)-(difficulty*GET_OBJ_WEIGHT(vict_obj))/2;
    if (AWAKE(victim) && ((number(0, MAX(1,target)) < GET_PER(victim))))
      {
	act("You discover that $n has $s hands on your possessions.",
	    FALSE, ch, 0, victim, TO_VICT);
	act("$N tries to steal something from $n.",TRUE,victim,0,ch,TO_ROOM);
	status=FALSE;
      }
    else if(!AWAKE(victim) && ((number(0, MAX(1,target)) < GET_PER(victim)/5)))
      {
	do_wake(victim, "", 0, 0);
	act("You are startled awake by $n going through your belongings.", FALSE, ch, 0, victim, TO_VICT);
	act("$n quickly wakes and jumps to $s feet when $N tries to steal something from $m.",TRUE,victim,0,ch,TO_ROOM);
	status=FALSE;
      }
 else if((AWAKE(victim) && (number(0, MAX(1,target)) < 2*GET_PER(victim)))
           || (!AWAKE(victim) && 
              (number(0, MAX(1,target)) < 2*GET_PER(victim)/5)))
    {
    /*do nothing...*/
    status = FALSE;
    }
    else 
      {
	/* Steal the item */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  if (IS_GODITEM(vict_obj) && GET_LEVEL(ch) < LEVEL_BUILDER)
	    return(0);
	  if ((IS_CARRYING_W(ch)+GET_OBJ_WEIGHT(vict_obj)) < CAN_CARRY_W(ch)) 
	    obj_to_char(unequip_char(victim, pos), ch,0);
	} 
	else
	  return FALSE;
      }
  }
  return(status);
}

SPECIAL(thief)
{
  struct char_data *cons, *thief;
  thief = (struct char_data *) me;

  if (cmd >= 0)
    return(FALSE);
  
  if (thief->specials.fighting)
    return(FALSE);
  
  if (IS_AFFECTED(thief, AFF_PARALYSIS))
      return(FALSE);

  if (IS_SET(world[thief->in_room].room_flags, PEACEFULL))
    return(FALSE);
  
  if (GET_POS(thief) < POSITION_STANDING)
    return (FALSE);
  
  switch(number(0,5)){
  case 0:
    for (cons = world[thief->in_room].people;
	 cons; cons = cons->next_in_room ) {
      if ((!IS_NPC(cons)) && CAN_SEE(thief, cons) && (number(1, 5) == 1)
	  && (GET_LEVEL(cons) < LEVEL_BUILDER)) {
	npc_steal_coins(thief, cons);
	if(check_guard(thief, cons))
	  return(0);
      }
    }
    break;
  case 1:
    for (cons = world[thief->in_room].people;
	 cons; cons = cons->next_in_room ) {
      if ((!IS_NPC(cons)) && CAN_SEE(thief, cons) && (number(1, 5) == 1)
	  && (GET_LEVEL(cons) < LEVEL_BUILDER)) {
	npc_steal_inv(thief, cons);
	if(check_guard(thief, cons))
	  return(0);
      }
    }
    break;
  case 2:
    for (cons = world[thief->in_room].people;
	 cons; cons = cons->next_in_room ) {
      if ((!IS_NPC(cons)) && CAN_SEE(thief, cons) && (number(1, 5) == 1)
	  && (GET_LEVEL(cons) < LEVEL_BUILDER)){
	npc_steal_equiped(thief, cons);
	if(check_guard(thief, cons))
	  return(0);
      }
    }
    break;
  default:
    break;
  }
  return(1);
}
bool check_guard(struct char_data *thief, struct char_data *victim)
{
  struct char_data *guard;
  
  for (guard = world[thief->in_room].people;guard;guard = guard->next_in_room){
    if (IS_NPC(guard) && (mob_index[guard->nr].func == cityguard))
      if ((number(0,31) + GET_LEVEL(thief)/10)
	  < (GET_PER(guard) + GET_LEVEL(guard)/10)){
	if (!IS_NPC(thief))
	  return(0);
	sprintf(buf, "%s notices $N trying to steal from you", GET_NAME(guard));
	act(buf,FALSE,victim,0,thief,TO_CHAR);
	sprintf(buf, "%s notices $N trying to steal from $n", GET_NAME(guard));
	act(buf,FALSE,victim,0,thief,TO_NOTCHAR);	  
	act("$n screams, 'STOP THIEF!'",TRUE,guard,0,0,TO_ROOM);
	if (MOB_FLAGGED(thief, MOB_CITIZEN))
	  REMOVE_BIT(MOB_FLAGS(thief), MOB_CITIZEN);
	hit(guard, thief, TYPE_UNDEFINED);
	return(1);
      }
  }
  return(0);
}

SPECIAL(tax)
{
    ACMD(do_say);
    ACMD(do_gen_com);
    struct char_data *cons=0;
    char buffr[256];
    
    if (cmd >= -1)
	return(FALSE);
    if (!AWAKE(ch) || ch->specials.fighting)
	return(FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);
    for (cons = world[ch->in_room].people;
	 cons; cons= cons->next_in_room )
	if (!IS_NPC(cons) && !number(0,1))
	    break;
    
    switch (number(0, 20)) {
    case 0:
	if(cons)
	    if(npc_steal_coins(ch, cons))
	      {
		sprintf(buffr,"Thanks for the taxes %s!",GET_NAME(cons));
		do_gen_com(ch,buffr,0,5);
		return(TRUE);
	      }
    case 1:
	if(cons)
	  sprintf(buffr,"Have you paid the King's taxes %s?",GET_NAME(cons));
	else
	  sprintf(buffr,"Have you paid the King's taxes citizen?");
	do_say(ch,buffr,0,0);
	break;
    case 2:
      break;
    case 3:
      break;
    default:
      return(FALSE);
    }
    return(FALSE);
}



SPECIAL(guild_guard)
{
    struct list_index_type *this_list;
    struct char_data *guard=0;
    char buf[256], buf2[256];
    int total=0, num=0, j, i;
    
    guard = (struct char_data *)me;
    if (!guard)
	return FALSE;
    if (IS_AFFECTED(guard, AFF_PARALYSIS) || !AWAKE(guard))
	return(FALSE);    

    if (cmd != 1)
      return(FALSE);
    
    if (GET_LEVEL(ch) >= LEVEL_BUILDER)
      return(FALSE);
    
    if (IS_NPC(ch))

      return(FALSE);

    for (j=0;j<=12;j++){
      this_list = guild_list[j];
      for (i = 0; *(this_list[i].entry) != '\n';i++)
	if (spell_info[this_list[i].index].spll_pointer)
	  total += GET_SKILL(ch,this_list[i].index);
    }

    this_list = am_skills;
    for (i = 0; *(this_list[i].entry) != '\n';i++)
      total += GET_SKILL(ch,this_list[i].index);
    
    if (total < 150)
      return(FALSE);
	
    act("$n says, 'Look $N, you know too much magic already.'",
	FALSE, guard, 0, ch, TO_ROOM);
    act("$n blocks $N's way.", FALSE, guard, 0, ch, TO_NOTVICT);
    act("$n blocks your way.", FALSE, guard, 0, ch, TO_VICT);

    return TRUE;
    
}

SPECIAL(ticket_collector)
{
    ACMD(do_say);
    struct obj_data *obj=0;
    struct char_data *collector;
    int mode;
    int dirn[4];
    
    collector = (struct char_data *)me;
    if (!collector)
	return FALSE;
    
    if (IS_AFFECTED(collector, AFF_PARALYSIS) || !AWAKE(collector))
	return(FALSE);
    
    if (mob_index[collector->nr].virtual == 2009){
	mode = 0;
	dirn[0] = 3;
    }
    else
	return FALSE;
	    
    if (cmd == SPEC_ARRIVE){
      act("$N looks at you.",FALSE,ch,0,collector,TO_CHAR);
      act("$N looks at $n.",FALSE,ch,0,collector,TO_ROOM);    	
      return(0);}
    else if (cmd <0)
	return(0);
    if (cmd != dirn[mode])
	return FALSE;
    act("$N looks at you.",FALSE,ch,0,collector,TO_CHAR);
    act("$N looks at $n.",FALSE,ch,0,collector,TO_ROOM);    
    switch (mode){
    case 0:
      obj = get_obj_in_list_vis(collector, "arg", ch->inventory);
      /* not in inventory but is holding something */
      if(!obj && (ch->equipment[HOLD]))
	if (isname("arg",ch->equipment[HOLD]->name))
	  obj = unequip_char(ch,HOLD);
      break;
    case 1:
      obj = get_obj_in_list_vis(collector, "rum", ch->inventory);
      /* not in inventory but is holding something */
      if(!obj && (ch->equipment[HOLD]))
	if (isname("rum",ch->equipment[HOLD]->name))
	  obj = unequip_char(ch,HOLD);
      break;
    case 2:
      for (obj= ch->inventory; obj; obj = obj->next_content)
	if ((obj_index[obj->item_number].virtual == 29004) &&
	    CAN_SEE_OBJ(collector, obj))
	  break;
      if (!obj && ch->equipment[HOLD])
	if (obj_index[ch->equipment[HOLD]->item_number].virtual == 29004)
	  obj = unequip_char(ch,HOLD);
      break;
    case 3:
      for (obj= ch->inventory; obj; obj = obj->next_content)
	if ((obj_index[obj->item_number].virtual == 29005) &&
	    CAN_SEE_OBJ(collector, obj))
	  break;
      if (!obj && ch->equipment[HOLD])
	if (obj_index[ch->equipment[HOLD]->item_number].virtual == 29005)
	  obj = unequip_char(ch,HOLD);
      break;
    }
    if (!obj) {
      switch (mode){
      case 0:
	do_say(collector,"No ticket - no sail.",0,0);
	do_say(collector,"You can buy tickets from the Harbourmaster.",0,0);
	return TRUE;
      case 1:
	do_say(collector, "Aye!  Not so fast!",0,0);
	do_say(collector, "Where are you sneakin' off to?",0,0);
	do_say(collector, "Who do you be?  Hows about a swig o' me rum?",0,0);
	do_say(collector, "Arggh! Me rum do be gone",0,0);	    
	return TRUE;
      case 2:
      case 3:
	do_say(collector,"No ticket - no sail.",0,0);
	do_say(collector,"You can buy tickets from the Harbourmaster.",0,0);
	return TRUE;
      }
    }
    switch(mode){
    case 0:
      act("$N helps $n onto the ship.",FALSE,ch,0,collector,TO_ROOM);
      act("$N helps you onto the ship.",FALSE,ch,0,collector,TO_CHAR);
      act("$N takes $p.",FALSE,ch,obj,collector,TO_CHAR);
      act("$N takes $p from $n.",FALSE,ch,obj,collector,TO_ROOM);
      break;
    case 1:
      if (GET_SEX(ch) == SEX_FEMALE)
	do_say(collector, "Ah, my sweet pretty!",0,0);
      else
	do_say(collector, "Ah, 'pon my soul!",0,0);
      do_say(collector, "There ain't no finer drink than one as sweet as rum.",0,0);
      act("$N takes $p.",FALSE,ch,obj,collector,TO_CHAR);
      act("$N takes $p from $n.",FALSE,ch,obj,collector,TO_ROOM);
      break;
    case 2:
    case 3:
      act("$N helps $n onto the ship.",FALSE,ch,0,collector,TO_ROOM);
      act("$N helps you onto the ship.",FALSE,ch,0,collector,TO_CHAR);
      act("$N takes $p.",FALSE,ch,obj,collector,TO_CHAR);
      act("$N takes $p from $n.",FALSE,ch,obj,collector,TO_ROOM);
      break;
    }
    extract_obj(obj,1);
    switch(mode){
    case 0:
      char_from_room(ch);
      char_to_room(ch,900, TRUE);
      break;
    case 1:
      char_from_room(ch);
      char_to_room(ch,11464, TRUE);
      break;
    case 2:
      char_from_room(ch);
      char_to_room(ch,29106, TRUE);
      break;
    case 3:
      char_from_room(ch);
      char_to_room(ch,29119, TRUE);
      break;
    }
    send_to_char("\r\n",ch);
    send_to_char(world[ch->in_room].description,ch);
    return(TRUE);
    
}



SPECIAL(puff)
{
    ACMD(do_say);

    if (cmd >=0)
	return(0);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);    
    switch (number(0, 60)) {
    case 0:
	do_say(ch, "My god!  It's full of stars!", 0, 0);
	return(1);
    case 1:
	do_say(ch, "How'd all those fish get up here?", 0, 0);
	return(1);
    case 2:
	do_say(ch, "I'm a very female dragon.", 0, 0);
	return(1);
    case 3:
	do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
	return(1);
    default:
	return(0);
    }
}

SPECIAL(gaspode)
    
{
    ACMD(do_say);
    ACMD(do_gen_com);
    ACMD(do_stand);
    void	perform_remove(struct char_data *ch, int pos);
    struct char_data *victim=0;
    struct char_data *dog=0;
    struct obj_data *obj=0;
    char bufr[256];
    int j;
    dog = (struct char_data *) me;
    if (IS_AFFECTED(dog, AFF_PARALYSIS))
	return(FALSE);    
    if (cmd >=0)
	return(0);

    *bufr = '\0';
    
    if (number(0,3))
	return(0);
    
    if (dog->equipment[WEAR_MOUTH] && !number(0,2))
	perform_remove(dog, WEAR_MOUTH);
	
    if (GET_POS(dog) == POSITION_SLEEPING && !number(0,2)){
	do_wake(dog,"",0,0);
	act("$n yawns.",TRUE,dog,0,0,TO_ROOM);
	do_stand(dog,"",0,0);
    }
    
    switch (number(0,400)) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:	
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      switch (number(0,6)){
      case 0:
	do_say(dog, "Yo! Got a bone?", 0, 0);
	break;
      case 1:
	do_say(dog, "Go on, pet me.", 0, 0); 
	break;
      case 2:
	do_say(dog, "I like your leg", 0, 0);
	break;
      case 3:
	do_say(dog, "Alright men! Gather 'round!",0,0);
	break;
      case 4:
	do_say(dog, "Put that chicken away missus!",0,0);
	break;
      case 5:
	act("$n lets off a real rip-roarer! ",FALSE, dog, 0, 0, TO_ROOM);
	do_say(dog, "Damn! Pardon me. Must be something I ate.", 0, 0);
	break;
      case 6:
	act("$n piddles on your leg.",FALSE, dog, 0, 0, TO_ROOM);
	break;
      case 7:
	act("$n slobbers on you.",FALSE, dog, 0, 0, TO_ROOM);
	break;
      default:
	break;
      }
      return(1);
    case 16:
    case 17:
    case 18:
    case 19:	
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:	
    case 25:
    case 26:
	if (number(0,1))
	    victim = find_vict_world(dog);
	else
	    victim = find_npc_world(dog);
	if (victim && !number(0,2)){
	    switch(number(0,6)){
	    case 0:
		act("$n tells you, 'I have a cunning plan!'",TRUE,dog,0,victim,TO_VICT);
		break;
	    case 1:
		if (!IS_NPC(victim)){
		    sprintf(buf,"%s",GET_NAME(victim));
		    sprintf(bufr,"Hey there %s!",CAP(buf));}
		else{
		    sprintf(buf,"%s",GET_NAME(victim));
		    sprintf(bufr,"Look! %s!",CAP(buf));
		    do_gen_com(dog,bufr,0,0);
		}
		break;
	    case 2:
	    case 3:
		sprintf(buf,"%s",GET_NAME(victim));		
		sprintf(bufr,"Anyone seen %s anywhere?",buf);
		do_gen_com(dog,bufr,0,0);
		do_gen_com(dog,"I mean are we talkin' Thicko City or what?",0,0);		
		break;		
	    case 4:
		if (!IS_NPC(victim))
		    act("$n tells you, 'Can you keep a secret?'",TRUE,dog,0,victim,TO_VICT);
		break;
	    case 5:
	      for (obj= victim->inventory; obj; obj = obj->next_content)
		if (obj_index[obj->item_number].virtual == 1237)
		  break;
	      if (!obj)
		for (j=0;j < MAX_WEAR ;j++)
		  if (victim->equipment[j])
		    if (obj_index[victim->equipment[j]->item_number].virtual == 1237){
		      obj = victim->equipment[j];
		      break;
		    }
	      if (!obj)
		return(1);
	      sprintf(buf,"%s",GET_NAME(victim));
	      sprintf(bufr,"J'accuse! %s stole my bone!",CAP(buf));
	      do_gen_com(dog,bufr,0,0);
	      break;
	    case 6:
	      for (obj= victim->inventory; obj; obj = obj->next_content)
		if (obj->obj_flags.value[6] > 0 && !number(0,2))
		  break;
	      if (!obj)
		for (j=0;j < MAX_WEAR ;j++)
		  if (victim->equipment[j])
		    if (victim->equipment[j]->obj_flags.value[6] > 0
			&& !number(0,1))
		      {
			obj = victim->equipment[j];
			break;
		      }
	      if (!obj)
		return(1);
	      sprintf(buf,"%s",GET_NAME(victim));
	      sprintf(buf2,"%s",obj->short_description);
	      sprintf(bufr,"Hey %s has %s!",CAP(buf),buf2);
	      do_gen_com(dog,bufr,0,0);
	      do_gen_com(dog,"I wouldn't give it to a dog, and I am one.",0,0);
	      break;		
	    }
	}
	else
	    switch(number(0,10)){
	    case 0:
		do_gen_com(dog,"Anyone got any flea repellant to sell?"
			   ,0,SCMD_AUCTION);
		break;
	    case 2:
		do_gen_com(dog,"I was jus' tryin' to save the world.",0,SCMD_GOSSIP);
		break;
	    case 3:
		do_gen_com(dog,"There's nothin' wrong with bein' a son of a bitch.!",0, 0);
		break;	    
	    case 4:
		do_gen_com(dog, "Woof boody woof!",0,SCMD_GOSSIP);
		break;
	    case 5:
		do_gen_com(dog,"Woof?",0,0);
		break;
	    case 6:
		do_gen_com(dog,"Woof. In tones of low menace.",0, SCMD_GOSSIP);
		break;
	    case 7:
		do_gen_com(dog,"Yelp!",0,0);
		break;
	    case 8:
		do_gen_com(dog,"Ouch! That Mad Monk's an evil bugger!", 0, SCMD_GOSSIP);
		break;
	    case 9:
		do_gen_com(dog,"Anyone seen my bone?",0,0);
		break;
	    case 10:
		do_gen_com(dog,"I'm in the market for a good bone.",0,SCMD_AUCTION);
		do_gen_com(dog,"Preferably mutton or beef - will pay well.",0,SCMD_AUCTION);
		break;
		
	    }
	return(1);
	break;
    case 27:
    case 28:	
	if (number(0,1))
	    victim = find_vict_world(dog);
	else
	    victim = find_npc_world(dog);	
	if (victim){
	    void update_pos(struct char_data *victim);
	    if ((GET_LEVEL(victim) >= LEVEL_BUILDER))
		return(1); 
	    GET_HIT(victim) = GET_MAX_HIT(victim);
	    GET_MOVE(victim) = GET_MAX_MOVE(victim);	    
	    update_pos(victim);
	    act("You have been fully healed by $N!",FALSE,victim,0,dog,TO_CHAR);
	    if ((!IS_NPC(victim) &&
		 (GET_LEVEL(victim)) <= 15 &&
		 (GET_LEVEL(victim) >= 4)) || GET_LEVEL(victim) < 30)
		{
		act("$N! has forced you to yell.",FALSE,victim,0,dog,TO_CHAR);
		do_gen_com(victim,"My God! I'm such a plonker!",0,0);}
	}
	    return(1);
	default:
	    return(0);
	}
    return(0);
}

SPECIAL(garm)
{
  struct char_data *mme;
  mme = (struct char_data *) me;
    if (cmd >= 0)
	return FALSE;
    if (IS_AFFECTED(mme, AFF_PARALYSIS))
	return(FALSE);
    if (!number(0,5))
	{
	    switch(number(0,2)){
	    case 0:
		if (cmd == SPEC_ARRIVE){
		    act("$n growls at $N with hatred, his hackles rising.",
		    TRUE,me,0,ch,TO_NOTVICT);
		    act("$n growls you with hatred, his hackles rising.",
		    TRUE,me,0,ch,TO_VICT);
		}
		else
		    act("$n suddenly growls with hatred, his hackles rising.",
		    TRUE,me,0,0,TO_ROOM);
		break;
	    case 1:
		act("$n bays, a terrible blood-curdling sound.",
		    TRUE,me,0,0,TO_ROOM);
		break;
	    case 2:
		act("$n paces about restlessly.",TRUE,me,0,0,TO_ROOM);
		break;		
	    default:
		break;
	    }
	}
   return(FALSE);
}
SPECIAL(satyrguard)
{
    ACMD(do_say);
    if (cmd >= 0)
	return FALSE;
    if (cmd == SPEC_ARRIVE)
	do_say(me,"Who goes there?",0,0);
    if (!number(0,20))
	act("$n paces slowly back and forth ever watchful.",
	    TRUE,me,0,0,TO_ROOM);
    return(FALSE);
}

SPECIAL(fido)
{
    
    struct obj_data *i=0, *temp=0, *next_obj=0;

    if (cmd <0)
	return(0);    
    if (cmd || !AWAKE(ch))
	return(FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);    
    for (i = world[ch->in_room].contents; i; i = i->next_content) {
	if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && (i->obj_flags.value[3] < 0)) {
	    act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
	    for (temp = i->contains; temp; temp = next_obj) {
		next_obj = temp->next_content;
		obj_from_obj(temp);
		obj_to_room(temp, ch->in_room, FALSE);
	    }
	    extract_obj(i,0);
	    return(TRUE);
	}
    }
     return(FALSE);
}

SPECIAL(lorshroompest)
{
    ACMD(do_say);
    struct char_data *vict=0;
    char *pest[]={
	"Theirs not to reason why. Theirs but to do or die.",
	"Into that ominous tract which, all agree, hides the Dark Tower.",
	"Mary had a little lamb whose fleece was white as snow.",
	"Tell me what thy lordly name is on the Night's Plutonian shore!",
	"Quoth the raven, 'Nevermore.'",
	"For he on honey-dew hath fed, and drunk the milk of Paradise",
	"O, rest ye, brother mariners, we will not wander more.",
	"The splendor falls on castle walls and snowy summits old in story.",
	"I hate the dreadful hollow behind the little wood.",
	"The itsy-bitsy spider went up the water spout.",
	"I am of old and young, of foolish as much as the wise.",
	"A line in long array where they wind betwixt green islands.",
	"Beautiful that war and all its deeds of carnage must in time be utterly lost.",
	"Some say the world will end in fire, some say in ice.",
	"The woods are lovely, dark and deep, but I have promises to keep.",
    };

    if (cmd >=0)
	return(0);    
    if( !AWAKE(ch) || ch->specials.fighting)
	return(FALSE);

    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);
    
    if (ch->master && number(0,9)){
	do_say(ch,pest[number(0,14)],0,0);
	return(0);}
    else{
	for (vict = world[ch->in_room].people;
	     vict;
	     vict = vict->next_in_room )
	    if ((!IS_NPC(vict) || IS_SET(vict->specials2.act, MOB_CAN_SPEAK)) && !number(0,1) && GET_LEVEL(vict) > 3)
	    break;
	if(!vict)
	    return(0);
	if (ch->master){
	    if (vict == ch->master)
		return(0);
	    stop_follower_quiet(ch);}
	else
	    if (!circle_follow(ch,vict))
		add_follower(ch,vict);
    }
    return(0);
}
SPECIAL(pigeon)
{
  if (IS_AFFECTED(ch, AFF_PARALYSIS))
    return(FALSE);
  if(!AWAKE(ch) || ch->specials.fighting || number(0,30))
	return(FALSE);
    switch(number(0,8)){
    case 0:
	act("$n coos softly.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 1:
	act("$n bobs $s head back and forth.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 2:
	act("$n flutters $s wings.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 3:
	act("$n poops.",FALSE,ch,0,0,TO_ROOM);
	break;
    default:
	act("$n struts about.",FALSE,ch,0,0,TO_ROOM);
	break;
    }
    return(FALSE);
}
SPECIAL(puppy)
{
    struct affected_type af;

    struct char_data *vict=0;

    if (cmd >= -1)
	return(0);        
    if(!AWAKE(ch) || ch->specials.fighting)
	return(FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);
    for (vict = world[ch->in_room].people;
	 vict;
	 vict = vict->next_in_room )
	if ((!IS_NPC(vict) || IS_SET(vict->specials2.act, MOB_CAN_SPEAK)) && !number(0,1))
	    break;
    
    switch(number(0,150))
	{
	case 0:
	    act("$n wags $s tail.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 3:
	    act("$n notices $s tail.",FALSE,ch,0,0,TO_ROOM);
	    act("$n spins around chasing $s tail.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 6:
	    act("$n yips happily.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 9:
	    if (vict){
		act("$N sniffs $n.",FALSE,vict,0,ch,TO_ROOM);
		act("$n sniffs you.",FALSE,ch,0,vict,TO_VICT);}
	    else
		act("$n sniffs you.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 12:
	    if (ch->master)
		break;
	    else if (vict){
	    act("$n growls at you.",FALSE,ch,0,vict,TO_VICT);	    
	    act("$N growls at $n.",FALSE,vict,0,ch,TO_ROOM);}
	    else
		act("$n growls at you.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 13:
	    if (vict){
		act("$N slobbers on $n.",FALSE,vict,0,ch,TO_ROOM);
		act("$n slobbers on you.",FALSE,ch,0,vict,TO_VICT);}
	    else
		act("$n slobbers on you.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 14:
	  if (cmd == SPEC_ARRIVE)
	    return;
	    if(!vict)
		break;
	    if (ch->master){
		if (vict == ch->master)
		    break;
		act("The puppy decides to follow you instead of $n.",FALSE,ch->master,0,vict,TO_VICT);
		act("The puppy decides to follow $N instead of you.",FALSE,ch->master,0,vict,TO_CHAR);
		stop_follower_quiet(ch);}
	    else if ( (ch != vict) && !circle_follow(ch,vict))
		{
		    act("The puppy decides to follow $n.",FALSE,vict,0,0,TO_ROOM);
		    act("The puppy decides to follow you home.",FALSE,vict,0,0,TO_CHAR);
		}
	    add_follower(ch,vict);
	    af.type = SPELL_CHARM_PERSON;
	    af.duration = 24;
	    af.modifier = 0;
	    af.location = 0;
	    af.bitvector = AFF_CHARM;
	    affect_to_char(ch,&af);
	    break;
	case 15:
	    act("$n licks $mself thoroughly.",FALSE,ch,0,0,TO_ROOM);
	    break;	    	    	    
	default:
	    break;
	}
    return(0);
}

SPECIAL(kitten)
{
    ACMD(do_flee);
    struct char_data *vict=0;

    if (cmd >= -1)
	return(0);        
    if(!AWAKE(ch) || ch->specials.fighting)
	return(FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);
    for (vict = world[ch->in_room].people;
	 vict;
	 vict = vict->next_in_room )
	if ((!IS_NPC(vict) || IS_SET(vict->specials2.act, MOB_CAN_SPEAK)) && !number(0,1))
	    break;
    

    switch(number(0,150))
	{
	case 0:
	    if (vict){
		act("$N rubs up against $n's leg.",FALSE,vict,0,ch,TO_ROOM);
		act("$n rubs up against your leg.",FALSE,ch,0,vict,TO_VICT);}
	    else
		act("$n rubs up against your leg.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 3:
	  act("$n decides $e would rather be somewhere else right now!",FALSE,ch,0,0,TO_ROOM);
	  do_flee(ch,"",0,0);
	  break;
	case 6:
	  act("$n purrs contentedly.",FALSE,ch,0,0,TO_ROOM);
	  break;
	case 9:
	  if (vict){
	    act("$N sniffs $n.",FALSE,vict,0,ch,TO_ROOM);		
	    act("$n sniffs you.",FALSE,ch,0,vict,TO_VICT);}
	  else
	    act("$n sniffs you.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 12:
	    if (vict){
		act("$N hisses at $n.",FALSE,vict,0,ch,TO_ROOM);
		act("$n hisses at you.",FALSE,ch,0,vict,TO_VICT);}
	    else
		act("$n hisses at you.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 13:
	    if (vict){
		act("$N pounces on $n's foot.",FALSE,vict,0,ch,TO_ROOM);
		act("$n pounces on your foot.",FALSE,ch,0,vict,TO_VICT);}
	    else
		act("$n pounces on your foot.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 14:
	    act("$n chases invisible dust motes.",FALSE,ch,0,0,TO_ROOM);
	    break;
	case 15:
	    act("$n washes $mself throroughly.",FALSE,ch,0,0,TO_ROOM);
	    break;	    	    
	default:
	    break;
	}
    return(0);
}

SPECIAL(janitor)
{
    struct obj_data *i=0;
    
    if (cmd >= -1)
	return(0);        
    if (!AWAKE(ch))
	return(FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);    
    switch(number(0,150)){
    case 0:
	break;
    case 1:
	break;
    case 2:
	break;
    case 3:
	break;
    case 4:
    case 5:
    case 6:
	for (i = world[ch->in_room].contents; i; i = i->next_content) {
	    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) &&
		!IS_GODITEM(i) && 
		((i->obj_flags.type_flag == ITEM_DRINKCON) || 
		 (i->obj_flags.cost <= 10))) {
	      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
		
	      obj_from_room(i);
	      obj_to_char(i, ch, 1);
	      return(1);
	    }
	}
    case 7:
	break;
    case 8:
    case 9:
	act("$n mumbles incoherently.", FALSE, ch, 0, 0, TO_ROOM);
	return(1);;
    default:
	break;
    }
    return(0);
}
	     
SPECIAL(tomcat)
{
    struct char_data *subj=0;
    subj = (struct char_data *) me;

    if (cmd >= -1)
	return(0);    
    if (!AWAKE(subj) || subj->specials.fighting)
	return (FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);
    if (isname("kitten",ch->player.name)){
	    hit(subj, ch, TYPE_CLAW);
	    return(TRUE);
	}
    else if (ch != subj && (isname("tom",GET_NAME(ch)))){
	    hit(subj, ch, TYPE_CLAW);
	    return(TRUE);
	}
    else if ((!strcmp("a large dog",GET_NAME(ch))
	      || !strcmp("the small dog",GET_NAME(ch)))){
	    act("$n arches $s back.",FALSE,me,0,ch,TO_ROOM);
	    act("$n hisses at $N.",FALSE,me,0,ch,TO_ROOM);
	}
    else if (!strcmp("a black cat",GET_NAME(ch))){
	    act("$n pounces on $N.",FALSE,me,0,ch,TO_ROOM);
	}
    
    switch (number(0,150))
	    {
	    case 0:
	    case 1:
	    case 2:
		act("$n grins evilly.",FALSE,me,0,0,TO_ROOM);
		break;
	    case 3:
	    case 4:
		act("$n starts at something.",FALSE,me,0,0,TO_ROOM);
		break;
	    case 5:
	    case 6:
	    case 7:
		act("$n marks $s territory.",FALSE,me,0,0,TO_ROOM);
	    case 8:
	    case 9:
	    case 10:
		act("$n snickers softly.",FALSE,me,0,0,TO_ROOM);
		break;
	    case 11:
		break;
	    case 12:
	    case 13:
	    case 14:
		act("$n struts proudly.",FALSE,me,0,0,TO_ROOM);
		break;
	    case 15:
	    case 16:
	    case 17:
		act("$n yowels loudly.",FALSE,me,0,0,TO_ROOM);		
		break;
	    case 18:
	    case 19:
	    case 20:
		act("$n glares around balefully.",FALSE,me,0,0,TO_ROOM);
		break;
	    default:
		break;
	    }
    return(FALSE);
    
}

SPECIAL(bianca)
{
    ACMD(do_tell);
    ACMD(do_say);
    ACMD(do_action);
    
    if (cmd >= 0)
	return FALSE;
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);    
    if (!number(0,20))
	{

	    switch(number(0,3)){
	    case 0:
		do_say(me,"Madam Bianca knows all.",0,0);
		break;
	    case 1:
		if (cmd == SPEC_ARRIVE)
		    act("$n tells you 'I can see your future in the stars.'"
			,TRUE,me,0,ch,TO_VICT);
		break;
	    case 2:
		do_say(me,"I sense that you are looking for someone.",0,0);
		break;
	    case 3:
		do_action(me,"",335,0);
		do_say(me,"I can feel death very close by.",0,0);
		break;
	    default:
		break;		
	    }
	}
    return(0);
}

SPECIAL(satyr_toadie)
{
    ACMD(do_tell);
    ACMD(do_say);
    ACMD(do_action);
    struct char_data *vict=0;

    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);    
    for (vict = world[ch->in_room].people;
	 vict;
	 vict = vict->next_in_room )
	if ((!IS_NPC(vict) || IS_SET(vict->specials2.act, MOB_CAN_SPEAK)) && !number(0,1))
	    break;

    
    if (cmd >= 0 || !AWAKE(ch))
	return FALSE;
    if (!number(0,20))
	{

	    switch(number(0,8)){
	    case 0:
		act("$n adjusts his hair.",FALSE,ch,0,0,TO_ROOM);
		break;
	    case 1:
		if (cmd == SPEC_ARRIVE)
		    act("$n says 'Ughh! here comes that unbearable $N.'"
			,TRUE,me,0,ch,TO_ROOM);
		break;
	    case 2:
		do_say(me,"You know the King relies on my council.",0,0);
		break;
	    case 3:
		do_action(me,"",28,0);
		do_say(me,"Now where has the King gone?",0,0);
		break;
	    case 4:
		if (vict)
		    do_action(me,GET_NAME(vict),89,0);
		else
		    do_action(me,"",89,0);
		break;
	    case 5:
		do_say(me,"Will you dance?",0,0);
		if (vict)
		    do_action(me,GET_NAME(vict),24,0);
		break;
	    case 6:
		do_say(me,"Why, that's astounding!",0,0);
		break;
	    case 7:
		act("$n exclaims 'Yes! Of course!'",TRUE,me,0,0,TO_ROOM);
		break;
	    case 8:
		do_say(me,"Hear the latest?",0,0);
		break;		
	    default:
		break;		
	    }
	}
    return(0);
}

SPECIAL(librarian)
{

    ACMD(do_say);
    
    if (cmd >= 0 || !AWAKE( (struct char_data *) me))
	return FALSE;
    if (IS_AFFECTED((struct char_data *) me, AFF_PARALYSIS))
	return(FALSE);    
    if (!number(0,20))
	{

	    switch(number(0,1)){
	    case 0:
		do_say(me,"Don't remove books from the library.",0,0);
		break;
	    case 1:
		if (cmd == SPEC_ARRIVE)
		    act("$n whispers to you, 'Shhhhhhhhhhhhh!'"
			,TRUE,me,0,ch,TO_VICT);
		break;
	    default:
		break;
	    }

	}
    return(0);
}
SPECIAL(mime)
{
    struct char_data *tmp=0, *vict=0;
    struct affected_type af;
    tmp = (struct char_data *)me;
    if (cmd >= -1 || !AWAKE(tmp))
	return(0);
    if (IS_AFFECTED(tmp, AFF_PARALYSIS))
	return(FALSE);        
    if (number(0,5))
	return(0);
    if (cmd == SPEC_MOBACT){
	for (vict = world[ch->in_room].people;
	     vict;
	     vict = vict->next_in_room )
	    if ((!IS_NPC(vict) || IS_SET(vict->specials2.act, MOB_CAN_SPEAK)) && vict != ch)
		break;
    }
    else
	if (tmp)
	    vict = tmp;

    
    if (!vict)
	return(0);
    if (vict == ch)
    	return(0);
    if (ch->master){
	if (vict == ch->master)
	    return(FALSE);
	act("$N decides to follow $n.",FALSE,vict,0,ch,TO_ROOM);
	act("$N decides to follow you.",FALSE,vict,0,ch,TO_CHAR);
	act("$n stops following you.",FALSE,ch,0,ch->master,TO_VICT);
	stop_follower_quiet(ch);}
    else if((ch != vict) && !circle_follow(ch,vict)){
	act("$N decides to follow $n.",FALSE,vict,0,ch,TO_ROOM);
	add_follower(ch,vict);
	af.type = SPELL_CHARM_PERSON;
	af.duration = 24;
	af.modifier = 0;
	af.location = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char(ch,&af);}
    return(TRUE);
}
SPECIAL(magician)
{
    ACMD(do_tell);
    ACMD(do_say);
    
    if (IS_AFFECTED((struct char_data *) me, AFF_PARALYSIS))
	return(FALSE);        
    
    if (cmd >= 0)
	return FALSE;
    if (!number(0,20))
	{

	    switch(number(0,3)){
	    case 0:
		do_say(me,"Watch me very closely now.",0,0);
		break;
	    case 1:
		act("$n cries out, 'Everyone stand well back!'"
		    ,TRUE,me,0,0,TO_ROOM);
		break;
	    case 2:
		do_say(me,"ABRAPOCUS!",0,0);
		break;
	    case 3:
		do_say(me,"HOCUS CADABRA!",0,0);
		break;
	    default:
		break;
	    }
	}
    return(0);
}
    
SPECIAL(citizen)
{
    ACMD(do_say);
    ACMD(do_bash);
    ACMD(do_kick);        
    ACMD(do_gen_com);    
    SPECIAL(moods);
    struct char_data *vict=0;
    
    if (cmd >= 0)
	return(0);        
    if(!AWAKE(ch) || ch->specials.fighting)
	return(FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);        

    for (vict = world[ch->in_room].people;
	 vict;
	 vict = vict->next_in_room )
	if ((!IS_NPC(vict)
	     || IS_SET(vict->specials2.act, MOB_CAN_SPEAK))
	     && !number(0,1) && vict != ch)
	    break;
    
    if(isname("match",ch->player.name) && !number(0,1)){
	switch(number(0,10)){
	case 0:
	case 1:
	case 2:
	case 3:
	    if (vict){
		act ("$N looks at $n sadly.",TRUE,vict,0,ch,TO_ROOM);
		act ("$n looks at you sadly.",TRUE,ch,0,vict,TO_VICT);
	    }
	    else
		act ("$n sniffs sadly.",TRUE,ch,0,vict,TO_ROOM);
	    break;
	case 4:
	case 5:
	case 6:
	    act ("A tear rolls down $n's cheek.",TRUE,ch,0,vict,TO_ROOM);
	    break;
	case 7:
	case 8:
	case 9:
	case 10:
	    do_say(ch,"Please buy my matches.",0,0);
	    break;
	}
    }	
    
    if (isname("opal",ch->player.name) && !number(0,1)){
	switch(number(0,10)){
	case 0:
	case 1:
	case 2:
	case 3:
	    if (vict){
		act ("$N blows a kiss to $n.",TRUE,vict,0,ch,TO_ROOM);
		act ("$n blows you a kiss.",TRUE,ch,0,vict,TO_VICT);
	    }
	    else
		act ("$n pouts.",TRUE,ch,0,vict,TO_ROOM);
	    break;
	case 4:
	case 5:
	case 6:
	case 7:
	    if (vict){
		act ("$N winks at $n mischieviously.",TRUE,vict,0,ch,TO_ROOM);
		act ("$n winks at you salaciously.",TRUE,ch,0,vict,TO_VICT);
	    }
	    else
		act("$n sighs deeply.",TRUE,ch,0,vict,TO_ROOM);
	    break;
	case 8:
	case 9:
	case 10:
	    do_say(ch,"OOh! he's cute.",0,0);
	    break;
	}
	
    }
	
    if (mob_index[ch->nr].virtual == 11030 && !number(0,1)){		
	switch(number(0,2)){
	case 0:
	    do_gen_com(ch,"I have silk stockings for sale!",0,1);
	    break;
	case 1:
	    do_gen_com(ch,"perfumes and ribbons all for a penny!",0,1);
	    break;
	case 2:
	    do_gen_com(ch,"Pretty silver earrings to give to your darling!",0,1);
	    break;
	}
    } 

    if (isname("rogan",ch->player.name) && !number(0,1)){		
	switch(number(0,2)){
	case 0:
	    do_gen_com(ch,"Lovely hot pies!",0,1);
	    break;
	case 1:
	    do_gen_com(ch,"Get yer hot pies here!",0,1);
	    break;
	case 2:
	    do_gen_com(ch,"Hot pies! only 4 pennies each!",0,1);
	    break;
	}
    } 
    if (isname("gawin",ch->player.name) && !number(0,1)){
	switch(number(0,2)){
	case 0:
	    do_gen_com(ch,"Muffins! Get yer fresh hot muffins!",0,1);
	    break;
	    case 1:
		do_gen_com(ch,"Fresh muffins! Get 'em while they're hot!",0,1);
	    break;
	case 2:
	    do_gen_com(ch,"Have you seen the muffin man?",0,1);
	    break;
	}

    }
    if (isname("vorg",ch->player.name) && !number(0,1)){
	switch(number(0,2)){
	case 0:
	    do_gen_com(ch,"Fresh fish!  I catch 'em, you buy 'em!",0,1);
	    break;
        case 1:
		do_gen_com(ch,"Catch of the day! Don't pass it up!",0,1);
	    break;
	case 2:
	    do_gen_com(ch,"Fish of all sizes!  Even guppies for the pixies!",0,1);
	    break;
	}

    }
    moods(ch, ch,SPEC_MOBACT, "");
    return 0;
}
SPECIAL(moods)
{
  ACMD(do_action);
  ACMD(do_insult);
  ACMD(do_say);
  ACMD(do_gen_com);
  int mood, *mp, which, i;
  struct char_data *vict=0;
  
  vict = (struct char_data *) me;
  
  if (IS_AFFECTED(ch, AFF_PARALYSIS))
    return(FALSE);
  if(GET_POS(ch) < POSITION_RESTING)
    return;
  if (ch->specials.fighting)
    return;
  if (vict == ch){
    for (vict = world[ch->in_room].people;
	 vict;
	 vict = vict->next_in_room )
      if ((!IS_NPC(vict)
	   || IS_SET(vict->specials2.act, MOB_CAN_SPEAK))
	  && !number(0,1) && vict != ch)
	break;
  }

  if (GET_MOOD(ch) > 900)
    mp = hyper_soc;
  else if (GET_MOOD(ch) > 750)
    {
      if (number(0,1))
	mp = happy_soc;
      else
	mp = hyper_soc;
    }
  else if (GET_MOOD(ch) > 500)
    {
      switch(number(0,2))
	{
	case 0:
	  mp = happy_soc;
	  break;
	case 1:
	  mp = cheery_soc;
	  break;
	case 2:
	  mp = hyper_soc;
	  break;
	}
    }
  else if (GET_MOOD(ch) > 250)
    {
      switch(number(0,2))
	{
	case 0:
	  mp = cheery_soc;
	  break;
	case 1:
	  mp = happy_soc;
	  break;
	case 2:
	  mp = neutral_soc;
	  break;
	}
    }	
  else if (GET_MOOD(ch) > 0){
    switch(number(0,2)){
    case 0:
      mp = cheery_soc;
      break;
    case 1:
      mp = sad_soc;
      break;
    case 2:
      mp = neutral_soc;
      break;
    }
  }
  else if (GET_MOOD(ch) > -250){
    switch(number(0,2)){
    case 0:
      mp = grouchy_soc;
      break;
    case 1:
      mp = sad_soc;
      break;
    case 2:
      mp = neutral_soc;
      break;
    }
  }
  else if (GET_MOOD(ch) > -500){
    switch(number(0,2)){
    case 0:
      mp = grouchy_soc;
      break;
    case 1:
      mp = sad_soc;
      break;
    case 2:
      mp = homicidal_soc;
      break;
    }
  }
  else if (GET_MOOD(ch) > -900){
    switch(number(0,1)){
    case 0:
      mp = grouchy_soc;
      break;
    case 1:
      mp = homicidal_soc;
      break;
    }
  }
  else
    mp = homicidal_soc;
  
  mood = 0;
  while (*(mp + mood) > 0)
	mood++;
  which = number(0,mood);
  if (vict && GET_SEX(vict) == GET_SEX(ch)){
    i = 0;
    while(opp_sex_soc[i] > 0){
      if (opp_sex_soc[i] == *(mp +which))
	return(0);
      i++;
    }
  }
  if (vict
      && GET_LEVEL(vict) < GET_LEVEL(ch)
      && ((*(mp +which) == 183 || *(mp +which) == 98) || *(mp +which) == 197))
	return(0);
    
  if (!vict && (*(mp +which) == 183 || *(mp +which) == 98) )
    return(0);
    
  if (*(mp +which) == 110 &&	GET_SEX(ch) != SEX_FEMALE)
    return(0);
  if (*(mp +which) == 98 &&	GET_SEX(ch) != SEX_MALE)
    return(0);
  
  if (vict)
    do_action(ch,vict->player.name,*(mp + which),0);
  else
    do_action(ch,"",*(mp + which),0);

  if (GET_MOOD(ch) < 0 && GET_ALIGNMENT(ch) < 200 && !number(0,5) && vict)
    do_insult(ch,vict->player.name,0,0);
  return 0;
}

SPECIAL(warrior){
    ACMD(do_insult);
    ACMD(do_bash);
    ACMD(do_kick);    
    if (cmd >=0)
	return(0);        
    if (!AWAKE(ch))
	return (FALSE);
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return(FALSE);        

    if (ch->specials.fighting){
	switch(number(0,10)){
	case 0:
	case 1:
	    do_insult(ch,GET_NAME(ch->specials.fighting),0,0);	    
	case 2:		   		   
	case 3:
	  if (ch->specials.fighting &&
	      (GET_POS(ch->specials.fighting) >= POSITION_STANDING))
	    do_bash(ch,"",0,0);
	  break;
	case 4:
	    do_insult(ch,GET_NAME(ch->specials.fighting),0,0);
	case 5:
	    do_insult(ch,GET_NAME(ch->specials.fighting),0,0);
	case 6:
	    do_kick(ch,"",0,0);
		   break;
	case 7:
	case 8:
	    do_insult(ch,GET_NAME(ch->specials.fighting),0,0);
	case 9:
	case 10:		   		   
	default:
	    break;}
    }
    return (0);
    
}
SPECIAL(princess)
{
  struct char_data *subj=0, *tch=0, *kitty=0, *winner=0;
  struct obj_data *obj;
  ACMD(do_say);
  ACMD(do_get);
  ACMD(do_action);
  ACMD(do_gen_com);
  if(cmd >= 0){
    return(FALSE);
  }
  subj = (struct char_data *)me;
  if (subj->specials.carrying &&
      (mob_index[subj->specials.carrying->nr].virtual == 2078)){
    if (!number(0,3))
      act("$n plays with $N.",FALSE,ch, 0, subj->specials.carrying, TO_ROOM);
    else if (!number(0,5)){
      kitty = subj->specials.carrying;
      if (!IS_MOB(subj) || !IS_MOB(kitty))
	return(0);
      char_from_room(subj);
      char_from_room(kitty);
      extract_char(subj, FALSE);
      extract_char(kitty, FALSE);
      mudlog("Extracting Princess and Kitty.", NRM,LEVEL_BUILDER, TRUE);
    }
    return(0);
  }
  if (cmd == SPEC_ARRIVE){
    for (tch = world[subj->in_room].people;
	 tch; tch = tch->next_in_room) {
      if (tch == subj)
	continue;
      else if (IS_MOB(tch) && (mob_index[tch->nr].virtual == 2078)
	       ){
	do_say(subj,"Kitty!!",0,0);
	if (!tch->specials.carried_by){
	  do_get(subj,"kitty",0,0);
	  act("$n cuddles $N.",FALSE,subj,0,tch,TO_ROOM);
	  return(0);
	}
	else
	  continue;
      }
      else if  (!tch->specials.carrying || !IS_MOB(tch->specials.carrying)) 
	continue;
      else if (mob_index[tch->specials.carrying->nr].virtual == 2078){
	kitty =  tch->specials.carrying;
	break;
      }
    }
    if (!kitty)
      {
	switch(number(0,5))
	  {
	  case 0:
	  case 1:
	    do_gen_com(subj, "Please can anyone help me find my kitty?"
		       ,0,SCMD_GOSSIP);
	    break;
	  case 2:
	    do_action(subj,"",53,0);
	  case 3:
	    do_say(subj, "Please help me I've lost my kitty.",0,0);
	    break;
	  case 4:
	    do_gen_com(subj, "Please help, my kitty is missing."
		       ,0,SCMD_GOSSIP);
	    do_gen_com(subj, "I'm just certain he'll be so frightened."
		       ,0,SCMD_GOSSIP);
	  case 5:
	    do_action(subj,"",135,0);	    
	    break;
	  }
	return(0);
      }
    else /* kitty found YAY! lets reward the player */
      {
	act("$n squirms in $N's arms.",FALSE,kitty,0,kitty->specials.carried_by,TO_NOTVICT);
	act("$n squirms in your arms.",FALSE,kitty,0,kitty->specials.carried_by,TO_VICT);
	winner = kitty->specials.carried_by;
	do_drop_mob(winner);
	do_get(subj,"kitty",0,0);
	if (obj = subj->inventory){
	  act("$n says, '$N accept this gift with my blessing.'",FALSE,subj,0,winner, TO_VICT);
	  act("$n says, '$N accept this gift with my blessing.'",FALSE,subj,0,winner, TO_ROOM);
	  perform_give(subj, winner, obj);
	}
	act("$n rewards you for returning her kitty.",FALSE,subj,0,winner,TO_VICT);
	act("$n rewards $N for returning her kitty.",FALSE,subj,0,winner,TO_NOTVICT);
	sprintf(buf,"Princess rewarding %s.",GET_NAME(winner));
	mudlog(buf, NRM,LEVEL_BUILDER, TRUE);
	gain_exp(winner, 5000/GET_LEVEL(winner),1);
	if (GET_FAME(winner) < 100)
	  GET_FAME(winner) += number(10,20);
      }
  }
}


SPECIAL(cityguard)
{
    ACMD(do_bash);
    ACMD(do_kick);
    ACMD(do_say);    
    ACMD(do_gen_com);
    struct char_data *tch=0,*cmbat_list=0,*tmp=0;
    struct char_data *victim=0, *evil=0, *subj=0;
    int	max_evil;
    char	victim_name[240];
    char	obj_name[240];
   
    /* cityguards don't trap normal commands, just events/time checks */
    if(cmd >= 0 && cmd != 156){
      return(FALSE);
    }
    subj = (struct char_data *)me;
    if (IS_AFFECTED(subj, AFF_PARALYSIS))
      return(FALSE);        
    
    if (!AWAKE(subj))
      return (FALSE);
    
    if ((ch == subj) && (cmd == SPEC_ARRIVE)){
      for (tch = world[ch->in_room].people;
	   tch; tch = tch->next_in_room) {
	    if (tch == subj)
	      continue;
	    if (IS_MOB(tch) && (mob_index[tch->nr].func
				== cityguard)){
	      if (GET_LEVEL(subj) > GET_LEVEL(tch)){
		act("$n salutes $N.",FALSE,tch,0,subj,TO_ROOM);
		act("$n returns the salute.",FALSE,subj,0,0,TO_ROOM);
		return FALSE;}
	      else if (GET_LEVEL(subj) < GET_LEVEL(tch)) {
		act("$n salutes $N.",TRUE,subj,0,tch,TO_ROOM);
		act("$n returns the salute.",TRUE,tch,0,0,TO_ROOM);
		return FALSE;}
	    }
      }
    }
    if(cmd == 156 && !number(0,3) && !subj->specials.fighting){
      strcpy(buf, arg);
      one_argument(one_argument(buf, obj_name), victim_name);
      if (!(victim = get_char_room_vis(ch, victim_name)))
	return(FALSE);
      if (!IS_NPC(victim))
	return(FALSE);
      if (IS_NPC(victim) && !MOB_FLAGGED(victim, MOB_CITIZEN))
	return(FALSE);
      else if (ch == victim){
	sprintf(buf1,"Don't be such a pratt %s.",GET_NAME(ch));
	do_say(subj,buf1,0,0);
	return(TRUE);
      }
      else {
	act("$n screams, 'STOP THIEF!'",TRUE,subj,0,0,TO_ROOM);
	hit(subj, ch, TYPE_UNDEFINED);
	return(FALSE);
      }
    }


    max_evil = 1000;
    evil = 0;
    
    if (subj->specials.fighting){
	switch(number(0,10)){
	case 0:
	case 1:
	case 2:		   		   
	case 3:
	  if (ch->specials.fighting &&
	      (GET_POS(ch->specials.fighting) >= POSITION_STANDING))	  
	    do_bash(subj,"",0,0);
	    return (FALSE);
	case 4:
	case 5:
	case 6:
	    do_kick(subj,"",0,0);
	    return (FALSE);
	case 7:
	case 8:
	case 9:
	case 10:		   		   
	default:
	    return(0);}
    }
    

    for (tch = world[subj->in_room].people; tch; tch = tch->next_in_room) {
      if (IS_UNDEAD(tch)){
	do_gen_com(subj,"DIE UNCLEAN CREATURE!!!!",0,SCMD_SHOUT);
	hit(subj,tch, TYPE_UNDEFINED);
      }
      if (!ch->specials.fighting && !IS_NPC(tch) && (IS_SET(PLR_FLAGS(tch), PLR_KILLER) || IS_SET(PLR_FLAGS(tch),PLR_THIEF))) {
	act("$n screams 'DIE CRIMINAL!!!'", FALSE, subj, 0, 0, TO_ROOM);
	hit(subj, tch, TYPE_UNDEFINED);
	return(TRUE);
      }
    }
    for (tch = world[subj->in_room].people;tch; tch = tch->next_in_room) {
      if (tch->specials.fighting && IS_NPC(tch)
	  && MOB_FLAGGED(tch, MOB_CITIZEN) && (tch != subj)) {
	cmbat_list = tch->next_fighting;
	for (tmp = cmbat_list;tmp;tmp= tmp->next_fighting)
	  if (!IS_NPC(tmp) && (tmp->specials.fighting == tch)){
	    evil = tmp;
	    break;
	  }
      }
    }
    if (evil) {
      act("$n screams 'UNHAND THAT CITIZEN'", FALSE, ch, 0, 0, TO_ROOM);
      hit(subj, evil, TYPE_UNDEFINED);
      return(TRUE);
    }
    return(FALSE);
}



/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */
   SPECIAL(bank)
       {
	   int	amount;
	   if (cmd <0)
	       return FALSE;
	   switch (cmd) {
	   case 233: /* balance */
	       if (GET_BANK_GOLD(ch) > 0)
		   sprintf(buf, "Your current balance is %s.\r\n",
			   report_cost(GET_BANK_GOLD(ch)));
	       else
		   sprintf(buf, "You currently have no money deposited.\r\n");
	       send_to_char(buf, ch);
	       return(1);
	       break;
	   case 234: /* deposit */
	       half_chop(arg,buf,buf2);
	       if ((amount = atoi(buf)) <= 0) {
		   send_to_char("How much do you want to deposit?\r\n", ch);
		   return(1);
	       }
	       if (!*buf2)
		   if (amount > report_pennies(GET_GOLD(ch))){
		       send_to_char("You don't have that many pennies.\r\n",ch);
		       return(1);}
		   else
		       amount *= 10;
	       else if( !strcmp("penny",buf2) ||
			!strcmp("pennies",buf2) ||
			!strcmp("coin",buf2) ||  !strcmp("coins",buf2)) 
		   if (amount > report_pennies(GET_GOLD(ch))){
		       send_to_char("You don't have that many pennies.\r\n",ch);
		       return(1);}
		   else
		       amount *= 10;
	       else if( !strcmp("groat",buf2) ||  !strcmp("groats",buf2))
		   if (amount > report_groats(GET_GOLD(ch))){
		       send_to_char("You don't have that many groats.\r\n",ch);
		       return(1);}
		   else
		       amount *= 100;
	       else if( !strcmp("crown",buf2) ||  !strcmp("crowns",buf2))
		   if (amount > report_crowns(GET_GOLD(ch))){
		       send_to_char("You don't have that many crowns .\r\n",ch);
		       return(1);}
		   else
		       amount *= 1000;      
	       change_gold(ch, -amount);
	       GET_BANK_GOLD(ch) += amount;
	       sprintf(buf, "You deposit %s.\r\n", report_cost(amount));
	       send_to_char(buf, ch);
	       sprintf(buf, "Your balance is now %s.\r\n", report_cost(GET_BANK_GOLD(ch)));
	       send_to_char(buf, ch);
	       act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
	       return(1);
	       break;
	   case 235: /* withdraw */
	       half_chop(arg,buf,buf2);	      
	       if ((amount = atoi(buf)) <= 0) {
		   send_to_char("How much do you want to withdraw?\r\n", ch);
		   return(1);
	       }
	       if (!*buf2)
		   if (amount > report_pennies(GET_BANK_GOLD(ch))){
		       send_to_char("You don't have that many pennies in the bank .\r\n",ch);
		       return(1);}
		   else
		       amount *= 10;
	       else if( !strcmp("penny",buf2)
	       ||!strcmp("pennies",buf2) ||
	       !strcmp("coin",buf2) ||  !strcmp("coins",buf2)) 
		   if (amount > report_pennies(GET_BANK_GOLD(ch))){
		       send_to_char("You don't have that many pennies.\r\n",ch);
		       return(1);}
		   else
		       amount *= 10;
	       else if( !strcmp("groat",buf2) ||  !strcmp("groats",buf2))
		   if (amount > report_groats(GET_BANK_GOLD(ch))){
		       send_to_char("You don't have that many groats.\r\n",ch);
		       return(1);}
		   else
		       amount *= 100;
	       else if( !strcmp("crown",buf2) ||  !strcmp("crowns",buf2))
		   if (amount > report_crowns(GET_BANK_GOLD(ch))){
		       send_to_char("You don't have that many crowns .\r\n",ch);
		       return(1);}
		   else
		       amount *= 1000;      
	       
	       drop_excess_gold(ch,amount);
	       GET_BANK_GOLD(ch) -= amount;
	       sprintf(buf, "You withdraw %s.\r\n", report_cost(amount));
	       send_to_char(buf, ch);
	       sprintf(buf, "Your balance is now %s.\r\n", report_cost(GET_BANK_GOLD(ch)));
	       send_to_char(buf, ch);		
	       act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
	       return(1);
	       break;
	   default:
	       return(0);
	       break;
	   }
       }

struct char_data *find_npc_world(struct char_data *ch)
{
    struct char_data *tch=0;
    int num=0, choice;

    for(tch = character_list; tch; tch = tch->next)
	if (IS_NPC(tch) && tch != ch)
	    num++;
    choice = number(1,num);
    num =0;
    for(tch = character_list; tch, num < choice; tch = tch->next)
	if (IS_NPC(tch) && tch != ch )
	    num++;
    
    return(tch);
    
}

struct char_data *find_vict_world(struct char_data *ch)
{
    struct descriptor_data *d;
    int ivict, ivict1, found=0;
    struct char_data *victim=0;
    
/* select a player from the world */
    
    ivict=0;
    for (d=descriptor_list;d;d=d->next){
	if (d->connected)
	    continue;
	if (GET_INVIS_LEV(d->character))
	    continue;	    
	if(d->original)
	    victim = d->original;
	else if (!(victim = d->character))
	    continue;
	ivict++;}
    found = FALSE;
    if (ivict > 0){
	ivict1 = number(1,ivict);
	ivict=0;
	for (d=descriptor_list;d;d=d->next){
	    if (d->connected)
		continue;
	    if (GET_INVIS_LEV(d->character))
		continue;
	    if(d->original)
		victim = d->original;
	    else if (!(victim = d->character))
		continue;
	    ivict++;
	    if (ivict == ivict1){
		found = TRUE;
		break;
	    }
	}
    }
    if (found)
	return(victim);
    else
	return(0);
	    
}

void reimund_story(struct char_data *me)
{
    ACMD(do_tell);
    ACMD(do_say);
    ACMD(do_action);
    static int current = 0;
    

    switch(current){
    case 0:
	do_say(me,"In my younger years, I was a stalwart adventurer known\r\n wide and far across the realms.",0,0);
	break;
    case 1:
	do_say(me,"But my heroic exploits and valiant deeds ended rather\r\n abruptly almost forty years ago.",0,0);
	break;
    case 2:
	do_say(me,"I heard tell of a great treasure to be found far to\r\n the north across the frozen wastelands.",0,0);
	break;
    case 3:
	do_say(me,"Several days out of camp, my band of fortune hunters\r\n were crossing a desolate stretch in the frozen northern wastelands.",0,0);
	break;
    case 4:
	do_say(me,"All that fateful morning the weather looked ominous,\r\n but our hearts were consumed by greed and we foolishly pressed onwards.",0,0);
	break;
    case 5:
	do_say(me,"We thought the storm too far off in the west, but by\r\n noon the heavy snow clouds had almost overtaken us.",0,0);
	break;
    case 6:
	do_say(me,"We were trapped in the middle of a fearsome blizzard!",0,0);
	break;
    case 7:
	do_say(me,"The blizzard raged madly round blinding us with\r\n swirling fingers of icy snow.  We pressed on, fearing that we would either\r\n freeze or be buried alive beneath the snow.",0,0);
	break;
    case 8:
	do_say(me,"The snow was catapulted by blinding gusts of wind and\r\n we could not see anything, including each other.",0,0);
	break;
    case 9:
	do_say(me,"When the wind died down and the snow settled, there I\r\n was alone.",0,0);
	break;
    case 10:
	do_say(me,"I was cut off from the rest of my band and was lost.\r\n Freezing, I headed towards the setting sun hoping to find warm shelter.",0,0);
	break;
    case 11:
	do_say(me,"At dusk I found myself crossing a frozen ravine, when\r\n I was ambushed by a slither of enormous devouring snow wyrms!",0,0);
	break;
    case 12:
	do_say(me,"Terrible beasts, massive milk-white serpents who hunt\r\n their food by sensing their quarry's body heat.",0,0);
	break;
    case 13:
	do_say(me,"A devouring wyrm's hunger is insatiable and never does\r\n the beast's stomach fill.",0,0);
	break;
    case 14:
	do_say(me,"Despite my weariness, I drew Incendiary, my mighty\r\n broadsword, from its jeweled scabbard and prepared to defend myself from\r\n these vile creatures.",0,0);
	break;
    case 15:
	do_say(me,"A terrible battle ensued!  With a powerful slash I\r\n burned Incendiary into the soft underside of the lead wyrm.",0,0);
	break;
    case 16:
	do_say(me,"Icy blue blood geysered up from the wound, chilling me\r\n to the bone.  The wound, however, was fatal and with one massive convulsion\r\n the serpent was dead.",0,0);
	break;
    case 17:
	do_say(me,"The other two devouring snow wyrms roared in anger\r\n upon witnessing the death of their kin and lunged viciously at me. ",0,0);
	break;
    case 18:
	do_say(me,"I parried the gaping snout of the first wyrm with my\r\n shield and violently slashed Incendiary at its neck.",0,0);
	break;
    case 19:
	do_say(me,"As the second wyrm made a lunge for my legs, I let\r\n loose a blood curdling war whoop and buried Incendiary between its evil \r\n eyes.",0,0);
	break;
    case 20:
	do_say(me,"Writhing in pain, the wyrm shook its head back and\r\n forth trying to shake loose the burning blade from its skull.",0,0);
	break;
    case 21:
	do_say(me,"With each mighty toss of the beast's head, I risked\r\n being flung far across the snow bound ravine.",0,0);
	break;
    case 22:
	do_say(me,"Bravely I held tight my grip and as I drove my fiery\r\n blade deep into the skull of the skull of the beast, the other wyrm \r\n attacked!",0,0);
	break;
    case 23:
	do_say(me,"In a blinding flash that cowardly beast, that evil\r\n beyond compare, struck at my armored legs.",0,0);
	break;
    case 24:
	do_say(me,"'With one bone crunching chomp, that foul wyrm bit off\r\n my left leg and swallowed it whole.",0,0);
	break;
    case 25:
	do_say(me,"My mind screamed in pain in what once was my leg.",0,0);
	break;		
    case 26:
	do_say(me,"Collapsing down to the snow in a red pool of my own\r\n blood, I fought desperately to stay alive.",0,0);
	break;
    case 27:
	do_say(me,"I hurled Incendiary at the wyrm with all the might I\r\n could muster.  The fiery blade flew swift and true to its target and plunged\r\n into the belly of the devouring wyrm.",0,0);
	break;
    case 28:
	do_say(me,"The wyrm moaned in torment and, in a desperate move,\r\n lunged again at me.  At the last moment I rolled away!",0,0);
	break;
    case 29:
	do_say(me,"Then that great coward wyrm slithered away in pain,\r\n escaping across the wastes with me cursing my revenge upon its frosty hide.",0,0);
	break;
    case 30:
	do_say(me,"Although profusely bleeding, I dragged myself across\r\n the snowy wastes sometimes nearly passing into a coma.",0,0);
	break;
    case 31:
	do_say(me,"After what seemed like hours, I miraculously chanced\r\n upon a lonely yurt and called out for help and then passed out unconscious.",0,0);
	break;
    case 32:
	do_say(me,"When I came to, I found myself in the care of \r\n hermit. This reclusive dwarf took compassion on me and nursed me back to\r\n health.",0,0);
	break;
    case 33:
	do_say(me,"As I slowly healed from my near fatal wound, my\r\n thinking began to change.  No longer did I desire revenge against my\r\n amputator.",0,0);
	break;
    case 34:
	do_say(me,"I began to reflect upon my violent way of life and\r\n through the guidance of my hermit master, began to learn the teachings of\r\n passivity and compassion.",0,0);
	break;
    case 35:
	do_say(me,"Long I lived with my master studying his teachings and\r\n cleansing my soul of hate and anger.",0,0);
	break;
    case 36:
	do_say(me,"Then after seven years under his tutelage, my master\r\n told me to return to civilization to spread all the lessons he had taught.",0,0);
	break;
    case 37:
	do_say(me,"So my good person, I beg of you to learn by my example\r\n and give up your violent ways of greed and anger.",0,0);
	break;
    case 38:
	do_say(me,"Be kind to your fellow beings and donate your\r\n belongings to others.  Because those items you perceive you so dearly need\r\n can surely serve others better.",0,0);
	break;
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
	break;
    default:
	current = 0;
	break;
    }
    current ++;
    if(current > 49)
	current = 0;
    return;
}

SPECIAL(water_dragon)
{
    if (cmd >=0)
        return(FALSE);

    if (!ch->specials.fighting)
        return FALSE;

    if (ch->specials.fighting &&
        (ch->specials.fighting->in_room == ch->in_room) &&
        (number(0, (230 - GET_LEVEL(ch))/4) == 0)) {
        act("$N leans back $S head and aims $S mouth towards $n!", 1, ch->specials.fighting, 0, ch, TO_ROOM);
        act("$n opens $s mouth wide to breathe on you!  Look at those tonsils!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        add_event(10, EVENT_SPELL, 0, 115, 0, 321,"", ch, ch->specials.fighting);
        return TRUE;
    }
    return FALSE;
}
SPECIAL(dragon)
{
  int breath_type;
  
  if (cmd >=0)
    return(FALSE);

  if ((mob_index[ch->nr].virtual == 22026) ||
      (mob_index[ch->nr].virtual == 7004) ||
      (mob_index[ch->nr].virtual == 7188))
    breath_type = 402; /* fire */
  else if ((mob_index[ch->nr].virtual == 11905) ||
	   (mob_index[ch->nr].virtual == 11187) ||
	   (mob_index[ch->nr].virtual == 11186))
    breath_type = 403; /* acid */
  else if ((mob_index[ch->nr].virtual == 11184))
    breath_type = 405; /* lighning */
  else if ((mob_index[ch->nr].virtual == 22010))
    breath_type = 404; /* gas */
  else
    return(FALSE);
  
  if (!ch->specials.fighting)
    return FALSE;
  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room) &&
      (number(0, 220) < GET_LEVEL(ch))) {
    act("$n raises $s head!", 1, ch, 0,0, TO_ROOM);
    act("$n opens $s mouth wide to breathe on you!", 1, ch, 0, 0, TO_ROOM);
    add_event(number(4,10),EVENT_ROOM_DAMAGE,GET_LEVEL(ch),
	      world[ch->in_room].number,
	      breath_type,FALSE,"$n breaths!",ch,ch);
    return TRUE;
  }
  return FALSE;
}

SPECIAL(yurian_shop)
{
    ACMD(do_say);
    if (!AWAKE(ch))
        return (FALSE);
    if (cmd == SPEC_ARRIVE)
        {
	do_say(ch,"A customer!  It seems like an eternity since I last did any business!",0,0);
        do_say(ch,"What fine armor may I interest you in today?",0,0);
        }
    return(FALSE);
}

SPECIAL(diels)
{
   struct obj_data *coffin=0, *corpse=0;

   if (!AWAKE(ch))
     return (FALSE);
   for (corpse=world[ch->in_room].contents;corpse;corpse=corpse->next_content) 
       {
       if (GET_ITEM_TYPE(corpse) == ITEM_CONTAINER 
              && corpse->obj_flags.value[3] < 0)
           {
           coffin = read_object(11239, VIRTUAL, 0);
           obj_from_room(corpse);
           obj_to_obj(corpse, coffin);
	   obj_to_room(coffin, ch->in_room, FALSE);
	   SET_BIT(coffin->obj_flags.value[1], CONT_CLOSED);
           act("$n gets $p.", FALSE, me, corpse, 0, TO_ROOM); 
           act("$n places $p in $P.", FALSE, me, corpse, coffin, TO_ROOM); 
           act("$n nails $p shut.", FALSE, me, coffin, 0, TO_ROOM); 
           act("$n drops $p onto the floor.", FALSE, me, coffin, 0, TO_ROOM); 
           }
       }
   return(FALSE);
}
SPECIAL(echo_room)
{
   int  i;
   char *len;
   ACMD(do_say);
   ACMD(do_gen_com);
   
   if(cmd != 17 && cmd != 18 && cmd != 169)
     return(0);
   else if (cmd == 17 || cmd == 169) 
     do_say(ch,arg,0,0);
   else 
     do_gen_com(ch,arg,0,SCMD_SHOUT);
   return(0);
   /*    {
      for (i = 0; *(arg + i) == ' '; i++)
	;
      if (ch->equipment[WEAR_MOUTH])
	return;
      if (*(arg + i))
	{
	  for (len = arg +i; *(len) != '\0'; len++)
	    ;
	  len--;
	  switch (*len){
	  case '?':
	    sprintf(buf, "$n asks '%s'", arg + i);
	    sprintf(buf1, "You ask '%s'\r\n", arg + i);
	    break;
	  case '!':
	    sprintf(buf, "$n exclaims '%s'", arg + i);
	    sprintf(buf1, "You exclaim '%s'\r\n", arg + i);
	    break;
	  default:
	    sprintf(buf, "$n says '%s'", arg + i);
	    sprintf(buf1, "You say '%s'\r\n", arg + i);
	    break;
	  }
I would really like to replace this bit right here with an add_event
but I didn't see one that would work.  I'd like it to be able to delay
number(1,10) and to have a 1/20 chance of just losing the echo altogether
           act(buf, FALSE, ch, 0, 0, TO_ROOM);
           send_to_char(buf1, ch);
       } 
   }*/
}


SPECIAL(banana)
{
  
  struct affected_type af;
  
    
  if (cmd == SPEC_ARRIVE){
    if((GET_DEX(ch) + number(1,10)) < 30){
      af.bitvector = AFF_SLIPPY;
      affect_to_char(ch, &af);
    }
  }
  return(0);
}
SPECIAL(blue_caterpillar)
{
    ACMD(do_say);

   if (cmd >= 0)
     return(0);
   if(!AWAKE(ch) || ch->specials.fighting || number(0,10))
	return(FALSE);
    switch(number(0,15)){
    case 0:
	act("$n slowly yawns.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 1:
	act("$n blows a smoke in your face.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 2:
	act("$n draws smokes from a hookah.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 3:
	act("$n languidly looks about.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 4:
	act("$n blows a smoke ring.",FALSE,ch,0,0,TO_ROOM);
	break;
    case 5:
	do_say(ch,"Who are you?",0,0);
	break;
    case 6:
	do_say(ch,"What are you doing?",0,0);
	break;
    case 7:
	do_say(ch,"When are you going to disappear?",0,0);
	break;
    case 8:
	do_say(ch,"Where are you from?",0,0);
	break;
    case 9:
	do_say(ch,"Why are you here?",0,0);
	break;
    case 10:
	do_say(ch,"How do you feel?",0,0);
	break;
    case 11:
	do_say(ch,"How do you feel?",0,0);
	break;
    default:
        break;
    }
/*Mag wants him to cast spells as well...wasn't sure what to do with the 
new spells.  He wants him to cast healing and rejuvenation type spells.*/
    return(FALSE);
}
SPECIAL(satyr_skald)
{
    ACMD(do_tell);
    ACMD(do_say);
    ACMD(do_action);

   if (cmd >= 0)
     return(0);
   if(!AWAKE(ch) || ch->specials.fighting || number(0,10))
	return(FALSE);

   switch(number(0,8)){
   case 0:
     do_action(ch,"",26,0);
     break;
   case 1:
     act("$n juggles $s daggers, tossing each high in the air."
	 ,TRUE,ch,0,me,TO_ROOM);
     break;
   case 2:
     do_action(ch,"",98,0);
     break;
   case 3:
     act("$n peers at you, an irreverent smirk lighting his face.", TRUE,ch,0,me,TO_ROOM);
     break;
   default:
     break;		
   }
   return(0);
}










