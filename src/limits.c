/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include "structs.h"
#include "limits.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern struct weather_data weather_info;
extern struct time_info_data time_info;	/* the infomation about the time   */
void    unhitch(struct char_data *ch, struct obj_data *cart, struct char_data *vict, char quiet);
void    rem_healing_bon(struct char_data *i);
void    poison_vict(struct char_data *i);
void    gain_social_standing(struct char_data *ch, struct char_data *vict,int mode);
/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int	graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

   if (age < 15)
      return(p0);						/* < 15   */
   else if (age <= 29)
      return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
   else if (age <= 44)
      return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
   else if (age <= 59)
      return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
   else if (age <= 79)
      return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
   else
      return(p6);						/* >= 80 */
}
int *ilev,il;


/* The hit_limit, mana_limit, and move_limit functions are gone.  They
   added an unnecessary level of complexity to the internal structure,
   weren't particularly useful, and led to some annoying bugs.  From the
   players' point of view, the only difference the removal of these
   functions will make is that a character's age will now only affect
   the HMV gain per tick, and _not_ the HMV maximums.
*/


void assign_levels()
{
ilev = &levels_table[0];
*ilev = 0;
ilev++;
for(il=1;il<=5;il++) {
     *ilev= (1 << (il-1))*750;
     ilev++;}
for(il=6;il<=25;il++){
     *ilev = 4000*((il-4)*(il-4)) + 9000;
     ilev++;}
for(il=26;il<=250;il++){
     *ilev = 7813*((il-10)*(il-10));
     ilev++;}
}


int	hit_gain(struct char_data *ch)
/* Hitpoint gain pr. game hour */
{
  int	gain;
  struct affected_type *aff;
  
  if (IS_NPC(ch)) {
      gain = GET_LEVEL(ch);
      /* Neat and fast */
   } else {

      gain = graf(age(ch).year, 8, 12, 20, 32, 16, 10, 4);

      /* Class/Level calculations */

      /* Skill/Spell calculations */

      /* Position calculations    */
      gain += (GET_CON(ch) - GET_RAW_CON(ch));
      
      switch (GET_POS(ch)) {
      case POSITION_SLEEPING:
	 gain += (gain/2); /* Divide by 2 */
	 break;
      case POSITION_RESTING:
	 gain += (gain/4);  /* Divide by 4 */
	 break;
      case POSITION_SITTING:
	 gain += (gain/8); /* Divide by 8 */
	 break;
      }
      if (GET_CON(ch) < 15)
	 gain /= 2;
      if (GET_CON(ch) > 20)
	 gain += gain/2;
   }
   
   if (GET_LEVEL(ch) > 10){
       if (((float)GET_HIT(ch))/ ((float)(MAX(1,GET_MAX_HIT(ch)))) > .7)
	   gain += gain/2;
       if (((float)GET_HIT(ch))/ ((float)(MAX(1,GET_MAX_HIT(ch)))) < .4)
	   gain -= gain/2;
       if (((float)GET_HIT(ch))/ ((float)(MAX(1,GET_MAX_HIT(ch)))) < .2)
	   gain -= gain/2;
       if (((float)GET_HIT(ch))/ ((float)(MAX(1,GET_MAX_HIT(ch)))) < .1)
	   gain -= gain/2;
   }

   if (IS_AFFECTED(ch, AFF_POISON))
      gain /= 4;

   if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;

   if (IS_SET(world[ch->in_room].room_flags, FAST_HP_GAIN))
       gain <<= 1;
   if (ch->specials.fighting && GET_LEVEL(ch) > 10)
       gain = 0;
   
   if (IS_AFFECTED(ch, AFF_HEALING))
     gain *= 2;
   for (aff = ch->affected; aff; aff = aff->next)
     if (aff->type == SPELL_BODY_MADE_WHOLE)
       gain = MAX(gain,(GET_MAX_HIT(ch) - GET_HIT(ch))/5);
   return (gain);
}



int	move_gain(struct char_data *ch)
/* move gain pr. game hour */
{
  int	gain;
  struct affected_type *aff;
  
  if (IS_NPC(ch)) {
    return(GET_LEVEL(ch));
    /* Neat and fast */
  } else {
      gain = graf(age(ch).year, 16, 20, 24, 20, 16, 12, 10);

      /* Class/Level calculations */

      /* Skill/Spell calculations */

      gain += 2*(GET_CON(ch) - GET_RAW_CON(ch));
      /* Position calculations    */
      switch (GET_POS(ch)) {
      case POSITION_SLEEPING:
	 gain += (gain/2); /* Divide by 2 */
	 break;
      case POSITION_RESTING:
	 gain += (gain/4);  /* Divide by 4 */
	 break;
      case POSITION_SITTING:
	 gain += (gain/8); /* Divide by 8 */
	 break;
      }
   }

   if (GET_LEVEL(ch) > 10){
       if (((float)GET_MOVE(ch))/ ((float)(MAX(1,GET_MAX_MOVE(ch)))) > .7)
	   gain += gain/2;
       if (((float)GET_MOVE(ch))/ ((float)(MAX(1,GET_MAX_MOVE(ch)))) < .3)
	   gain -= gain/8;
       if (((float)GET_MOVE(ch))/ ((float)(MAX(1,GET_MAX_MOVE(ch)))) < .2)
	 gain -= gain/6;
       if (((float)GET_MOVE(ch))/ ((float)(MAX(1,GET_MAX_MOVE(ch)))) < .1)
	   gain -= gain/4;
   }
   if ((GET_MOVE(ch) < 0 ) && (GET_POS(ch) == POSITION_SLEEPING))
     gain *= 3;
   
   if (IS_AFFECTED(ch, AFF_POISON))
      gain /= 4;

   if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;

   if (IS_SET(world[ch->in_room].room_flags, FAST_MOVE_GAIN))
       gain *= 2;
   if (ch->specials.fighting && GET_LEVEL(ch) > 10)
       gain = 0;   
   if (IS_AFFECTED(ch, AFF_HEALING))
     gain *= 2;
   for (aff = ch->affected; aff; aff = aff->next)
     if (aff->type == SPELL_BODY_MADE_WHOLE)
       gain = MAX(gain,(GET_MAX_MOVE(ch) - GET_MOVE(ch))/5);
   return (gain);
}


/* Gain maximum in various points */
void	advance_level(struct char_data *ch)
{
   int	add_hp, i,add_move;

   extern struct con_app_type con_app[];

   add_hp = con_app[GET_RAW_CON(ch)].hitp;


   add_hp += number(GET_RAW_CON(ch)/3,GET_RAW_CON(ch)/2);
   add_move = number((GET_RAW_CON(ch)/8 + GET_RAW_DEX(ch)/8 + GET_RAW_STR(ch)/4),(GET_RAW_CON(ch)/4 + GET_RAW_DEX(ch)/4 + GET_RAW_STR(ch)/2))/4;


   ch->points.max_hit += MAX(5, add_hp);
   if ((GET_LEVEL(ch) != 1) && (ch->points.max_move < 1000)) 
     ch->points.max_move = ch->points.max_move + (short)add_move;

   SPELLS_TO_LEARN(ch) += 20*number(1, MAX(2,(GET_RAW_INT(ch)*2/3 
					+ GET_RAW_WIS(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) += 20*number(1, MAX(2,(GET_RAW_STR(ch)*2/3 
					+ GET_RAW_DEX(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) += 10*number(1, MAX(2,(GET_RAW_GUI(ch)*2/3 
					+ GET_RAW_CHR(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) += 10*number(1, MAX(2,(GET_RAW_PER(ch)*2/3 
					+ GET_RAW_CON(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) += 20*number(1, MAX(2,(GET_RAW_DEV(ch)*2/3 
					+ GET_RAW_FOC(ch)*2/3)/10));   
   SPELLS_TO_LEARN(ch) += number(1,GET_RAW_LUC(ch))/2;

   if (GET_LEVEL(ch) >= LEVEL_BUILDER)
      for (i = 0; i < 3; i++)
	 GET_COND(ch, i) = (char) -1;

   save_char(ch, NOWHERE);

   sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
   mudlog(buf, BRF, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
}
void	loose_level(struct char_data *ch)
{
   int	add_move, add_hp, i;

   extern struct con_app_type con_app[];

   add_hp = con_app[GET_RAW_CON(ch)].hitp;


   add_hp += number( GET_RAW_CON(ch)/3,GET_RAW_CON(ch)/2);
   add_move = number((GET_RAW_CON(ch)/8 + GET_RAW_DEX(ch)/8 + GET_RAW_STR(ch)/4),(GET_RAW_CON(ch)/4 + GET_RAW_DEX(ch)/4 + GET_RAW_STR(ch)/2))/4;

   SPELLS_TO_LEARN(ch) -= 20*number(1, MAX(2,(GET_RAW_INT(ch)*2/3 
					+ GET_RAW_WIS(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) -= 20*number(1, MAX(2,(GET_RAW_STR(ch)*2/3 
					+ GET_RAW_DEX(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) -= 10*number(1, MAX(2,(GET_RAW_GUI(ch)*2/3 
					+ GET_RAW_CHR(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) -= 10*number(1, MAX(2,(GET_RAW_PER(ch)*2/3 
					+ GET_RAW_CON(ch)*2/3)/10));
   SPELLS_TO_LEARN(ch) -= 20*number(1, MAX(2,(GET_RAW_DEV(ch)*2/3 
					+ GET_RAW_FOC(ch)*2/3)/10));   
   SPELLS_TO_LEARN(ch) -= number(1,GET_RAW_LUC(ch))/2;

   ch->points.max_hit -= MAX(5, add_hp);
   ch->points.max_hit = MAX(10,ch->points.max_hit);
   ch->points.max_move = ch->points.max_move - (short)add_move;
   ch->points.max_move = MAX(50,ch->points.max_move);

   if (GET_LEVEL(ch) >= LEVEL_BUILDER)
      for (i = 0; i < 3; i++)
	 GET_COND(ch, i) = (char) -1;

   save_char(ch, NOWHERE);

   sprintf(buf, "%s reduced to level %d", GET_NAME(ch), GET_LEVEL(ch));
   mudlog(buf, BRF, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
}
void	advance_sub_level(struct char_data *ch, int sublev)
{
   int	add_hp, i;
   int	add_move,add_pracs;

   extern struct con_app_type con_app[];

   add_hp = con_app[GET_RAW_CON(ch)].hitp;
   add_hp += number(GET_RAW_CON(ch)/3,GET_RAW_CON(ch)/2);
   add_move = number((GET_RAW_CON(ch)/8 + GET_RAW_DEX(ch)/8 + GET_RAW_STR(ch)/4),(GET_RAW_CON(ch)/4 + GET_RAW_DEX(ch)/4 + GET_RAW_STR(ch)/2))/4;

   add_pracs = 20*number(1, MAX(2,(GET_RAW_INT(ch)*2/3 
					+ GET_RAW_WIS(ch)*2/3)/10));
   add_pracs += 20*number(1, MAX(2,(GET_RAW_STR(ch)*2/3 
					+ GET_RAW_DEX(ch)*2/3)/10));
   add_pracs += 10*number(1, MAX(2,(GET_RAW_GUI(ch)*2/3 
					+ GET_RAW_CHR(ch)*2/3)/10));
   add_pracs += 10*number(1, MAX(2,(GET_RAW_PER(ch)*2/3 
					+ GET_RAW_CON(ch)*2/3)/10));
   add_pracs += 20*number(1, MAX(2,(GET_RAW_DEV(ch)*2/3 
					+ GET_RAW_FOC(ch)*2/3)/10));   
   add_pracs += number(1,GET_RAW_LUC(ch))/2;
   if (sublev%2){
   ch->points.max_hit += MAX(1, add_hp/4);
   ch->points.max_move += MAX(2,(short)add_move/4);
   }
   else 
     SPELLS_TO_LEARN(ch) += add_pracs/5;

   if (GET_LEVEL(ch) >= LEVEL_BUILDER)
      for (i = 0; i < 3; i++)
	 GET_COND(ch, i) = (char) -1;

   save_char(ch, NOWHERE);

}
void	loose_sub_level(struct char_data *ch, int sublev)
{
   int	add_hp, i;
   int	add_move,add_pracs;

   extern struct con_app_type con_app[];

   add_hp = con_app[GET_RAW_CON(ch)].hitp;
   add_hp += number(GET_RAW_CON(ch)/3,GET_RAW_CON(ch)/2);
   add_move = number((GET_RAW_CON(ch)/8 + GET_RAW_DEX(ch)/8 + GET_RAW_STR(ch)/4),(GET_RAW_CON(ch)/4 + GET_RAW_DEX(ch)/4 + GET_RAW_STR(ch)/2))/4;

   add_pracs = 20*number(1, MAX(2,(GET_RAW_INT(ch)*2/3 
					+ GET_RAW_WIS(ch)*2/3)/10));
   add_pracs += 20*number(1, MAX(2,(GET_RAW_STR(ch)*2/3 
					+ GET_RAW_DEX(ch)*2/3)/10));
   add_pracs += 10*number(1, MAX(2,(GET_RAW_GUI(ch)*2/3 
					+ GET_RAW_CHR(ch)*2/3)/10));
   add_pracs += 10*number(1, MAX(2,(GET_RAW_PER(ch)*2/3 
					+ GET_RAW_CON(ch)*2/3)/10));
   add_pracs += 20*number(1, MAX(2,(GET_RAW_DEV(ch)*2/3 
					+ GET_RAW_FOC(ch)*2/3)/10));   
   add_pracs += number(1,GET_RAW_LUC(ch))/2;
   if (sublev%2){
     ch->points.max_hit -= MAX(1, add_hp/4);
     ch->points.max_move -= MAX(2,(short)add_move/4);
   }
   else 
     SPELLS_TO_LEARN(ch) -= add_pracs/5;

   save_char(ch, NOWHERE);

}


void	set_title(struct char_data *ch)
{
   if (GET_TITLE(ch))
     return;
   else
      CREATE(GET_TITLE(ch), char, strlen("%n the newbie %c visits %r") +1);

   strcpy(GET_TITLE(ch), "%n the newbie %c visits %r");
}


void check_autowiz(struct char_data *ch)
{
   char	buf[100];
   extern int	use_autowiz;
   extern int	min_wizlist_lev;

   if (use_autowiz && GET_LEVEL(ch) >= LEVEL_BUILDER) {
      sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
         WIZLIST_FILE, LEVEL_BUILDER, IMMLIST_FILE, getpid());
      mudlog("Initiating autowiz.", CMP, LEVEL_BUILDER, FALSE);
      system(buf);
   }
}

void    gain_social_standing(struct char_data *ch, struct char_data *vict,int mode){
  
  if (IS_NPC(ch))
    return;
  switch(mode){
  case MODE_KILL:
    if (IS_NPC(vict))
      GET_FAME(ch) -= (GET_FAME(vict)*GET_LEVEL(vict))/MAX(1,GET_LEVEL(ch));
    else 
      if ((GET_LEVEL(vict) <= 10) && (GET_LEVEL(ch) > 10))
	GET_FAME(ch) -= 500;
      else
	GET_FAME(ch) -= (GET_FAME(vict)*GET_LEVEL(vict))/MAX(1,GET_LEVEL(ch));
    break;
  case MODE_FLEE:
    GET_FAME(ch) -= abs(GET_FAME(vict)*GET_LEVEL(vict))/MAX(1,5*GET_LEVEL(ch));
    break;
  case MODE_STEAL:
    GET_FAME(ch) -= abs(GET_FAME(vict)*GET_LEVEL(vict))/MAX(1,5*GET_LEVEL(ch));
    break;    
    
    
  }
}

void	gain_exp(struct char_data *ch, int gain,int mode)
{
  int	i, sub_level, incr, sincr;
   
  if (!IS_NPC(ch)
      && ((GET_LEVEL(ch) < LEVEL_BUILDER-1) && (GET_LEVEL(ch) > 0))) {
    if (gain > 0) {
      gain = MIN(3000*(MAX(1,GET_LEVEL(ch))), gain);
      i = GET_LEVEL(ch);
      sub_level = GET_SUB_LEVEL(ch);
      sincr = (levels_table[i+1] - levels_table[i])/10;
      while (gain){
	 incr = MIN(gain, sincr);
	 GET_EXP(ch) += incr;
	 gain -= incr;
	 
	 while(GET_EXP(ch) >= (levels_table[i] + (sub_level + 1)*sincr)){
	   advance_sub_level(ch, sub_level);
	   sub_level++;
	   GET_SUB_LEVEL(ch) += 1;
	   if (sub_level > 9){
	     sub_level = 0;
	     GET_SUB_LEVEL(ch) = 0;	     
	     i++;
	     if (GET_LEVEL(ch) < LEVEL_BUILDER -1)
	       GET_LEVEL(ch) += 1;
	     sincr = (levels_table[i+1] - levels_table[i])/10;
	   }
	 }
      }
    }
    if (gain < 0) {
      gain = MAX(-6000*GET_LEVEL(ch), gain);
      i = GET_LEVEL(ch);
      sub_level = GET_SUB_LEVEL(ch);
      sincr = (levels_table[i+1] - levels_table[i])/10;
      while (gain < 0){
	incr = MAX(gain, -1*sincr);
	GET_EXP(ch) += incr;
	gain -= incr;
	if (GET_EXP(ch) < 750){
	  GET_EXP(ch) = 750;
	  GET_SUB_LEVEL(ch) = 0;
	  return;
	}
	while (GET_EXP(ch) <= (levels_table[i] + sub_level*sincr)){
	  loose_sub_level(ch, sub_level);
	  sub_level--;
	   GET_SUB_LEVEL(ch) -= 1;
	   if (sub_level < 0){
	     sub_level = 9;
	     GET_SUB_LEVEL(ch) = 9;	     
	     i--;
	     GET_LEVEL(ch) -= 1;
	     if (GET_LEVEL(ch) < 1)
	       GET_LEVEL(ch) = 1;
	     sincr = (levels_table[i+1] - levels_table[i])/10;
	   }
	}
      }
    }
    
  }
}


void	gain_exp_regardless(struct char_data *ch, int gain, int mode)
{
   int	i, incr, sincr, sub_level;

   if (!IS_NPC(ch)) {
      if (gain > 0) {

       i = GET_LEVEL(ch);
       sub_level = GET_SUB_LEVEL(ch);
       sincr = (levels_table[i+1] - levels_table[i])/10;
       while (gain){
	 incr = MIN(gain, sincr);
	 GET_EXP(ch) += incr;
	 gain -= incr;
	 
	 while(GET_EXP(ch) >= (levels_table[i] + (sub_level + 1)*sincr)){
	   advance_sub_level(ch, sub_level);
	   sub_level++;
	   GET_SUB_LEVEL(ch) += 1;
	   if (sub_level > 9){
	     sub_level = 0;
	     GET_SUB_LEVEL(ch) = 0;	     
	     i++;
	     GET_LEVEL(ch) += 1;
	     sincr = (levels_table[i+1] - levels_table[i])/10;
	   }
	 }
       }
	
      }
      else if (gain < 0){
	i = GET_LEVEL(ch);
	sub_level = GET_SUB_LEVEL(ch);
	sincr = (levels_table[i+1] - levels_table[i])/10;
	while (gain < 0){
	  incr = MAX(gain, -1*sincr);
	  GET_EXP(ch) += incr;
	  gain -= incr;
	  if (GET_EXP(ch) < 750){
	    GET_EXP(ch) = 750;
	    GET_SUB_LEVEL(ch) = 0;
	    return;
	  }
	  while (GET_EXP(ch) <= (levels_table[i] + sub_level*sincr)){
	    loose_sub_level(ch, sub_level);
	    sub_level--;
	    GET_SUB_LEVEL(ch) -= 1;
	    if (sub_level < 0){
	      sub_level = 9;
	      GET_SUB_LEVEL(ch) = 9;	     
	      i--;
	      GET_LEVEL(ch) -= 1;
	      sincr = (levels_table[i+1] - levels_table[i])/10;
	    }
	  }
	}
      }
   }
}


void	gain_condition(struct char_data *ch, int condition, int value)
{
   bool intoxicated;

   if (GET_COND(ch, condition) == -1) /* No change */
      return;

   intoxicated = (GET_COND(ch, DRUNK) > 0);

   GET_COND(ch, condition)  += value;

   GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
   GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

   if (PLR_FLAGGED(ch, PLR_WRITING))
      return;

   switch (condition) {
   case FULL:
     if (GET_COND(ch, FULL) == 0)
       send_to_char("You are starving.\r\n",ch);
     else if (GET_COND(ch, FULL) <= 4 )
       send_to_char("You are famished.\r\n",ch);
     else if (GET_COND(ch, FULL) <= 8 )
       send_to_char("You are very hungry.\r\n",ch);
     else if (GET_COND(ch, FULL) <= 12 )
       send_to_char("You are hungry.\r\n",ch);
     else if (GET_COND(ch, FULL) < 16 )
       send_to_char("Your stomach is growling.\r\n",ch);
     return;
   case THIRST :
     if (GET_COND(ch, THIRST) == 0)
       send_to_char("You are parched.\r\n",ch);
     else if (GET_COND(ch, THIRST) <= 4)
       send_to_char("Your throat is dry and your lips cracked.\r\n",ch);
     else if (GET_COND(ch, THIRST) <= 8)
       send_to_char("Your throat is very dry.\r\n",ch);
     else if (GET_COND(ch, THIRST) <= 12)
       send_to_char("You are thirsty.\r\n",ch);
     else if (GET_COND(ch, THIRST) < 16)
       send_to_char("Your feel a little thirsty.\r\n",ch);
      return;
   case DRUNK :
      if (intoxicated)
	 send_to_char("You are now sober.\r\n", ch);
      return;
   default :
      break;
   }

}


void	check_idling(struct char_data *ch)
{
    
    if (++(ch->specials.timer) > 8)
	if (GET_LEVEL(ch) < LEVEL_BUILDER)
	    if (ch->specials.was_in_room == NOWHERE
		&& ch->in_room != NOWHERE) {
		ch->specials.was_in_room = ch->in_room;
		if (ch->specials.fighting) {
		    stop_fighting(ch->specials.fighting);
		    stop_fighting(ch);
		}
		act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
		send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
		save_char(ch, NOWHERE);
		Crash_crashsave(ch);
		char_from_room(ch);
		char_to_room(ch, 1, FALSE);
	    }
	    else if (ch->specials.timer > 48) {
		if (ch->in_room != NOWHERE) 
		    char_from_room(ch);
		char_to_room(ch, 3, FALSE);
		if (ch->desc)
		    close_socket(ch->desc);
		ch->desc = 0;
		Crash_idlesave(ch);
		sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
		mudlog(buf, CMP, LEVEL_GOD, TRUE);
		extract_char(ch, FALSE);
	    }
}
void rem_healing_bon(struct char_data *i)
{
  struct affected_type *aff, *next;

  
  for (aff = i->affected;aff;aff = next){
    next = aff->next;
    if ((aff->type == SPELL_HEALING_TOUCH) ||
	(aff->type == SPELL_BODY_MADE_WHOLE))
      affect_remove(i, aff);
  }
}

void poison_vict(struct char_data *i)
{
   struct affected_type *aff;

   for (aff = i->affected;aff;aff = aff->next)
	if (aff->type == SPELL_POISON && aff->location == APPLY_NONE)
	    break;
    if (aff)
	switch (aff->modifier){
	case 0:
	    damage(i, i, number(1,12), SPELL_POISON,-1,0);
	    break;
	case 1:
	    damage(i, i, number(6,24), SPELL_POISON,-1,0);
	    break;
	case 2:
	    damage(i, i, number(12,48), SPELL_POISON,-1,0);
	    break;
	case 3:
	    damage(i, i, number(24,96), SPELL_POISON,-1,0);
	    break;
	case 4:
	    damage(i, i, number(96,192), SPELL_POISON,-1,0);
	    break;
	case 5:
	    damage(i, i, number(200,500), SPELL_POISON,-1,0);
	    break;
	default:
	    damage(i, i, number(96,192), SPELL_POISON,-1,0);
	}
    else
	damage(i, i, number(96,192), SPELL_POISON,-1,0);

}

/* Update both PC's & NPC's and objects*/
void	point_update( void )
{
   void	update_char_objects( struct char_data *ch ); /* handler.c */
   void	extract_obj(struct obj_data *obj, int mode); /* handler.c */
   struct char_data *i, *next_dude;
   struct obj_data *j, *next_thing, *jj, *next_thing2;
   /* characters */
   for (i = character_list; i; i = next_dude) {
      next_dude = i->next;
      if (i->specials.fighting && (i->specials.fighting->specials.fighting
				   == i)
	  && IS_AFFECTED(i, AFF_HEALING))
	  rem_healing_bon(i);
      if (GET_POS(i) >= POSITION_STUNNED) {
	 GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  GET_MAX_HIT(i));
	 GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
	 if (IS_AFFECTED(i, AFF_HEALING) && (GET_HIT(i) == GET_MAX_HIT(i)))
	     rem_healing_bon(i);
	 if (IS_AFFECTED(i, AFF_POISON))
	     poison_vict(i);
	 if (GET_POS(i) == POSITION_STUNNED)
	    update_pos(i);
      } else if (GET_POS(i) == POSITION_INCAP)
	 damage(i, i, 1, TYPE_SUFFERING,-1,0);
      else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW))
	 damage(i, i, 2, TYPE_SUFFERING,-1,0);
      if (i->specials.mount && (i->specials.mount ->in_room != i->in_room))
	unmount(i);
      if (i->specials.rider && (i->specials.rider->in_room != i->in_room))
	unmount(i->specials.rider);
      if (i->specials.carrying &&(i->specials.carrying->in_room != i->in_room))
	do_drop_mob(i);
      if (i->specials.carried_by &&
	  (i->specials.carried_by->in_room != i->in_room))
	do_drop_mob(i->specials.carried_by);
      if (IS_PULLING(i) && (IS_PULLING(i)->in_room != i->in_room))
	  unhitch(i,IS_PULLING(i),i,1);
	   /* if carts get separated from their */
           /* owners then they should be unhitched*/
      if (!IS_NPC(i)) {
	 update_char_objects(i);
	 if (GET_LEVEL(i) < LEVEL_IMPL)
	    check_idling(i);
      }
      gain_condition(i, FULL, -1);
      gain_condition(i, DRUNK, -1);
      gain_condition(i, THIRST, -1);
   } /* for */
   for (j = object_list; j ; j = next_thing) {
     next_thing = j->next; /* Next in object list */
     if (GET_ITEM_TYPE(j) == ITEM_SRTLIGHT) {
       if (((time_info.hours >= 21) || (time_info.hours  <= 4))
	   && !IS_SET(j->obj_flags.extra_flags, ITEM_GLOW)) {
	 SET_BIT(j->obj_flags.extra_flags, ITEM_GLOW);
	 act("$p starts to glow.", FALSE,0,j,0,TO_ROOM);
       }
       else if (((time_info.hours >= 5) && (time_info.hours <= 20))
		&& IS_SET(j->obj_flags.extra_flags, ITEM_GLOW)) {
	 REMOVE_BIT(j->obj_flags.extra_flags, ITEM_GLOW);
	 act("$p goes out.", FALSE,0,j,0,TO_ROOM);
       }
     }
     if (j->obj_flags.timer > 0)
       j->obj_flags.timer--;
     if ((j->obj_flags2.light > 0) && j->in_room
	 && !IS_OBJ_STAT(j, ITEM_MAGIC) && IS_OBJ_STAT(j, ITEM_GLOW)){
       j->obj_flags2.light--;
       if ((j->obj_flags2.light == 0)){
	 REMOVE_BIT(j->obj_flags.extra_flags, ITEM_GLOW);
	 act("$p stops glowing.",FALSE,0,j,0,TO_ROOM);
	if (j->obj_flags.type_flag == ITEM_LIGHT)
	  extinguish_light(j);
       }
       else if ((j->obj_flags2.light <= 4) && (j->obj_flags2.light > 0))
	 act("$p is growing dim.",FALSE,0,j,0,TO_CHAR);
     }
     
    /* If this is a corpse */
     if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3] < 0)){
       /* timer count down */
       if (j->obj_flags.timer == 0) {
	 
	 if (j->carried_by)
	   act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	 else if ((j->in_room != NOWHERE) && (world[j->in_room].people)){
	   if (world[j->in_room].sector_type == SECT_UNDER_WATER) {
	     if (j->obj_flags.value[3] == -1){
	       act("A shoal of fish devour $p."
		   ,TRUE, world[j->in_room].people, j,0,TO_ROOM);
	       act("A shoal of fish devour $p."
		   ,TRUE, world[j->in_room].people, j,0,TO_CHAR);
	     }
	   }
	   else {
	     if (j->obj_flags.value[3] == -1){
	       act("$p crumbles to dust.",TRUE, world[j->in_room].people, j,0,TO_ROOM);
	       act("$p crumbles to dust.",TRUE, world[j->in_room].people, j,0,TO_CHAR);
	       j->obj_flags.timer = 1 ;
	       j->obj_flags.value[3] = -2;
	       sprintf(buf2,"dust pile");
	       free(j->name);
	       j->name = str_dup(buf2);
	       sprintf(buf2, "A pile of dust is lying here.");
	       free(j->description);
	       j->description = str_dup(buf2);
	       sprintf(buf2, "a pile of dust");
	       free(j->short_description);
	       j->short_description = str_dup(buf2);
	       free(j->action_description);
	       j->action_description = 0;
	       continue;
	     }
	     send_to_room("A gust of wind blows the dust away.\r\n",
			  j->in_room, FALSE);
	   }
	 }
	 for (jj = j->contains; jj; jj = next_thing2) {
	   next_thing2 = jj->next_content; /* Next in inventory */
	   obj_from_obj(jj);
	   
	   if (j->in_obj)
	     obj_to_obj(jj, j->in_obj);
	   else if (j->carried_by)
	     obj_to_room(jj, j->carried_by->in_room, FALSE);
	   else if (j->in_room != NOWHERE)
	     obj_to_room(jj, j->in_room, FALSE);
	   else
	     assert(FALSE);
	 }
	 extract_obj(j,0);
       }
     }
     else {
       if (j->obj_flags.timer == 0) {
	 if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
	   act("$p fades out of existance."
	       ,TRUE, world[j->in_room].people, j, 0, TO_NOTCHAR);
	   act("$p fades out of existance."
	       ,TRUE, world[j->in_room].people, j, 0, TO_CHAR);
	 }
	 for (jj = j->contains; jj; jj = next_thing2) {
	   next_thing2 = jj->next_content; /* Next in inventory */
	   obj_from_obj(jj);
	   
	   if (j->in_obj)
	     obj_to_obj(jj, j->in_obj);
	   else if (j->carried_by)
	     obj_to_room(jj, j->carried_by->in_room, FALSE);
	   else if (j->in_room != NOWHERE)
	     obj_to_room(jj, j->in_room, FALSE);
	   else
	     assert(FALSE);
	 }
	 extract_obj(j,0);
       }
     }
   }
   
}



