/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int top_of_world;
extern struct   index_data *mob_index;
/* extern functions */
void	raw_kill(struct char_data *ch);
int	do_simple_move(struct char_data *ch, int cmd, int following);
int     can_see_hidden(struct char_data *sub, struct char_data *obj);
char    *first_name(char *buf);
void    botch_kick(struct char_data *ch, struct char_data *vict);
bool    throw_rider(struct char_data *ch);
ACMD(do_assist)
{
   struct char_data *helpee, *opponent;

   if (ch->specials.fighting) {
      send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
      return;
   }

   one_argument(argument, arg);
   
   if (IS_NPC(ch) && ch->specials.timer)
     return;
   
   if (!(helpee = get_char_room_vis(ch, arg)))
      send_to_char("Whom do you wish to assist?\r\n", ch);
   else if (helpee == ch)
      send_to_char("You can't help yourself any more than this!\r\n", ch);
   else
    {
      for (opponent = world[ch->in_room].people; opponent && 
          (opponent->specials.fighting != helpee); 
          opponent = opponent->next_in_room)
	 ;

      if (!opponent)
	 act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
      else if (!CAN_SEE(ch, opponent)
	       || (IS_AFFECTED(opponent,AFF_HIDE) &&
		   !can_see_hidden(ch,opponent)))
	 act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
      else {
	 send_to_char("You join the fight!\r\n", ch);
	 act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
	 act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
	 hit(ch, opponent, TYPE_UNDEFINED);
      }
   }
}


ACMD(do_hit)
{
   struct char_data *victim;

   one_argument(argument, arg);

   if (IS_SET(world[ch->in_room].room_flags, PEACEFULL)) {
     act("You feel too peaceful to contemplate violence."
	 ,FALSE,ch,0,0,TO_CHAR);
     return;
   } 
   if (IS_NPC(ch) && ch->specials.timer)
     return;
   if (*arg) {
      victim = get_char_room_vis(ch, arg);
      if (victim) {
	 if (victim == ch) {
	    send_to_char("You hit yourself...OUCH!.\r\n", ch);
	    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
	    return;
	 }
	 if (!IS_NPC(victim) && !IS_NPC(ch) && (subcmd != SCMD_MURDER)) {
	    send_to_char("To avoid accidental flagging, you have to use the MURDER command to\r\n"
	        "hit another player.\r\n", ch);
	    return;
	 }
	 if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(victim))
	    return; /* you can't order a charmed pet to attack a player */
	 if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, victim, TO_CHAR);
	    return;
	 }
	 if (victim != ch->specials.fighting) {
	    hit(ch, victim, TYPE_UNDEFINED);
	    if (IS_NPC(ch))
	      ch->specials.timer = 2;
	    WAIT_STATE(ch, PULSE_VIOLENCE + 2); /* HVORFOR DET?? */
	 } else
	    send_to_char("You are doing the best you can!\r\n", ch);
      } else
	 send_to_char("They aren't here.\r\n", ch);
   } else
      send_to_char("Hit who?\r\n", ch);
}



ACMD(do_kill)
{
   struct char_data *victim;

   if ((GET_LEVEL(ch) < LEVEL_ASS_IMPL) || IS_NPC(ch)) {
      do_hit(ch, argument, cmd, subcmd);
      return;
   }

   one_argument(argument, arg);
   if (IS_NPC(ch) && ch->specials.timer)
     return;
   if (!*arg) {
      send_to_char("Kill who?\r\n", ch);
   } else {
      if (!(victim = get_char_room_vis(ch, arg)))
	 send_to_char("They aren't here.\r\n", ch);
      else if (ch == victim)
	  send_to_char("Your mother would be so sad.. :(\r\n", ch);
      else {
	  act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, victim, TO_CHAR);
	  act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
	  act("$n brutally slays $N!", FALSE, ch, 0, victim, TO_NOTVICT);
	  raw_kill(victim);
      }
   }
}



ACMD(do_backstab)
{
   struct char_data *victim;
   byte percent, prob, num;

   one_argument(argument, buf);
   
   if (IS_NPC(ch) && ch->specials.timer)
     return;
   
   if (ch->specials.mount) {
     send_to_char("You had better dismount first.\r\n", ch);
     return;}
   if (ch->specials.carried_by) {
     act("Better ask $N to drop you first.",FALSE, ch,0,ch->specials.carried_by, TO_CHAR);
     return;}   
  
   if (IS_SET(world[ch->in_room].room_flags, PEACEFULL)) 
       {
	   act("You feel too peaceful to contemplate violence.",
	       FALSE,ch,0,0,TO_CHAR);
	   return;
       } 
   if (!(victim = get_char_room_vis(ch, buf))) {
      send_to_char("Backstab who?\r\n", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("How can you sneak up on yourself?\r\n", ch);
      return;
   }

   if (!ch->equipment[WIELD]) {
      send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
      return;
   }

   if (ch->equipment[WIELD]->obj_flags.value[3] != 2) {
      send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
      return;
   }

   if (victim->specials.fighting) {
      send_to_char("You can't backstab a fighting person, too alert!\r\n", ch);
      return;
   }

   percent = number(1, 31); /* 101% is a complete failure */

   prob = GET_SKILL(ch, SKILL_BACKSTAB);
   if (IS_AFFECTED(ch, AFF_SNEAK))
     prob += 5;
   else if (IS_AFFECTED(ch, AFF_HIDE))
     prob += 15;   
   if (encumberance_level(ch) > 0)
     prob /= (encumberance_level(ch) + 2);
   if (AWAKE(victim) && (percent > prob))
      damage(ch, victim, 0, SKILL_BACKSTAB,-1,1);
   else{
       num = GET_SKILL(ch, SKILL_BACKSTAB);
	if (!number(0,(num*num)/10 + 1) && percent < 10)
	    if (num < 30){
		SET_SKILL(ch, SKILL_BACKSTAB, num + 1);
		send_to_char("You seem to improve your backstabbing ability.\r\n",ch);
	    }
       
	hit(ch, victim, SKILL_BACKSTAB);
   }
   if (IS_NPC(ch))
     ch->specials.timer = 4;
   WAIT_STATE(ch, PULSE_VIOLENCE*4);
}



ACMD(do_order)
{
   char	name[100], message[256];
   char	buf[256];
   bool found = FALSE;
   int	org_room;
   struct char_data *victim;
   struct follow_type *k;

   half_chop(argument, name, message);

   if (!*name || !*message)
      send_to_char("Order who to do what?\r\n", ch);
   else if (!(victim = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
      send_to_char("That person isn't here.\r\n", ch);
   else if (ch == victim)
      send_to_char("Talking to yourself again?\r\n", ch);

   else {
      if (IS_AFFECTED(ch, AFF_CHARM)) {
	 send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
	 return;
      }

      if (victim) {
	 sprintf(buf, "$N orders you to '%s'", message);
	 act(buf, FALSE, victim, 0, ch, TO_CHAR);
	 act("$n gives $N an order.", FALSE, ch, 0, victim, TO_NOTVICT);

         /* patched 9-24-94 by Scarrow so that an object which sets the */
         /* AFF_CHARM flag will not allow the user to be ordered by the */
         /* person they are following if they are immortal              */
	 if ( (victim->master != ch) || !IS_AFFECTED(victim, AFF_CHARM) ||
               (!IS_NPC(victim) && GET_LEVEL(victim) >= LEVEL_BUILDER))
	    act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	 else {
	    send_to_char("Ok.\r\n", ch);
	    command_interpreter(victim, message);
	 }
      } else {  /* This is order "followers" */
	 sprintf(buf, "$n issues the order '%s'.", message);
	 act(buf, FALSE, ch, 0, victim, TO_ROOM);

	 org_room = ch->in_room;

	 for (k = ch->followers; k; k = k->next) {
	    if (org_room == k->follower->in_room)
	       if (IS_AFFECTED(k->follower, AFF_CHARM)) {
		  found = TRUE;
		  command_interpreter(k->follower, message);
	       }
	 }
	 if (found)
	    send_to_char("Ok.\r\n", ch);
	 else
	    send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
      }
   }
}

ACMD(do_abort)
{
  send_to_char("Input buffer purged.\r\n", ch);
  return;
}

ACMD(do_flee)
{
  ACMD(do_stand);
  ACMD(do_enter);
  ACMD(do_leave);    
  struct obj_data *tmp_obj;
  int	i, attempt, loose, die;

  void	gain_exp(struct char_data *ch, int gain, int mode);
  int	special(struct char_data *ch, int cmd, char *arg);
  
  if (GET_POS(ch) < POSITION_RESTING){
    send_to_char("Don't be silly - just lie still and die!\r\n", ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_PARALYSIS)){
    send_to_char("You are unable to move!\r\n", ch);
    return;
  }
  if (GET_POS(ch) < POSITION_STANDING){
    send_to_char("Try getting on your feet first!\r\n", ch);
    return;
  }
  if (IS_NPC(ch) && ch->specials.timer)
    return;
  if (ch->specials.carried_by) {
    if (!number(0,10)) {
      act("You wriggle and squirm in $N's grasp."
	  ,FALSE,ch,0,ch->specials.carried_by,TO_CHAR);
      act("$n wriggles and squirms in $N's grasp."
	  ,FALSE,ch,0,ch->specials.carried_by,TO_NOTVICT);
      act("$n wriggles and squirms in your grasp."
	  ,FALSE,ch,0,ch->specials.carried_by,TO_VICT);
      do_drop_mob(ch->specials.carried_by);
    }
    else {
      act("Impossible $N has too tight a grip of you."
	  ,FALSE,ch,0,ch->specials.carried_by,TO_CHAR);
      act("$n wriggles and squirms, trying to escape $N's grasp."
	  ,FALSE,ch,0,ch->specials.carried_by,TO_NOTVICT);
      act("$n wriggles and squirms, trying to escape your grasp."
	  ,FALSE,ch,0,ch->specials.carried_by,TO_VICT);
      return;
    }
  }
  if (!(ch->specials.fighting)) {
    for (i = 0; i < 6; i++) {
      attempt = number(0, NUM_OF_DIRS-1);  /* Select a random direction */
      if (!number(0,2) && CAN_GO(ch, attempt) && 
	  !IS_SET(world[real_room(EXIT(ch, attempt)->to_room)].room_flags, DEATH)) {
	act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
	if ((die = do_simple_move(ch, attempt, FALSE)) == 1) {
	  /* The escape has succeded */
	  send_to_char("You flee head over heels.\r\n", ch);
	} else {
	  if (!die)
	    act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
	}
	return;
      }
    }
    /* No exits was found */
    /* try for cart */
    for (tmp_obj = world[ch->in_room].contents
	   ;tmp_obj;tmp_obj = tmp_obj->next_content)
      if (HASROOM(tmp_obj) && !number(0,3) &&
	  !IS_SET(tmp_obj->obj_flags.value[1]
		  , CONT_CLOSED)){
	act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
	send_to_char("You flee head over heels.\r\n", ch);
	strcpy(buf,first_name(tmp_obj->name));
	do_enter(ch,buf,0,0);
	return;
      }  
      else if (world[ch->in_room].obj  &&
	       !IS_SET(world[ch->in_room].obj->obj_flags.value[1]
		       , CONT_ONEWAY) &&
	       !IS_SET(world[ch->in_room].obj->obj_flags.value[1]
		       , CONT_CLOSED)){
	act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);	      
	send_to_char("You flee head over heels.\r\n", ch);
	do_leave(ch,"",0,0);
	return;}
    act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);      
    send_to_char("PANIC!  You couldn't escape!\r\n", ch);
    return;
  }

  for (i = 0; i < 6; i++) {
    if (GET_POS(ch) < POSITION_STANDING){
      if (IS_NPC(ch)){
	act("$n madly scrambles to get to $s feet.",FALSE,ch,0,0,TO_ROOM);
	do_stand(ch,"",0,0);
      }
      else
	act("You had better get on your feet first.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
    attempt = number(0, NUM_OF_DIRS-1);  /* Select a random direction */
    if (CAN_GO(ch, attempt) && 
	!IS_SET(world[real_room(EXIT(ch, attempt)->to_room)].room_flags,DEATH))
      {
	act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
	if ((die = do_simple_move(ch, attempt, FALSE)) == 1) {
	  /* The escape has succeded */
	  loose = GET_MAX_HIT(ch->specials.fighting) - GET_HIT(ch->specials.fighting);
	  loose *= GET_LEVEL(ch);
	  
	  if (!IS_NPC(ch)){
	    gain_exp(ch, -loose,0);
	    gain_social_standing(ch, ch->specials.fighting, MODE_FLEE);
	  }
	  
	  send_to_char("You flee head over heels.\r\n", ch);
	  
	  /* hunting system on */
	  /* only hunt pc's */
	  if (!IS_NPC(ch) && IS_NPC(ch->specials.fighting) && !number(0,2)
	      && IS_SET(ch->specials.fighting->specials2.act, MOB_MEMORY)
	      && !IS_SET(ch->specials.fighting->specials2.act, MOB_SENTINEL))
	    ch->specials.fighting->specials.hunting = ch;
	  
	  if (ch->specials.fighting->specials.fighting == ch)
	    stop_fighting(ch->specials.fighting);
	  stop_fighting(ch);
	  return;
	} else {
	  if (!die)
	    act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
	  return;
	}
      }
  } /* for */
  
  /* No exits was found */
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}
ACMD(do_bash)
{
  struct affected_type af;
  struct char_data *victim;
  int base = 0, height_difference = 0;
  byte percent, prob, turns, num;
  

  if (IS_SET(world[ch->in_room].room_flags, PEACEFULL)) 
    {
      act("You feel too peaceful to contemplate violence.",
	  FALSE,ch,0,0,TO_CHAR);
      return;
    } 
  if (IS_NPC(ch) && ch->specials.timer)
    return;
  if (ch->specials.mount && !GET_SKILL(ch, SKILL_CAVALRY)) {
    send_to_char("You can't bash whilst mounted.\r\n",ch);
    return;
  }
  one_argument(argument, arg);
  if (!(victim = get_char_room_vis(ch, arg))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Bash whom?\r\n", ch);
      return;
    }
  }
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (GET_WEIGHT(ch) > 3*GET_WEIGHT(victim)/2)
    turns = 2;
  else if (GET_WEIGHT(ch) > GET_WEIGHT(victim))
    turns = 1;
  else if (GET_WEIGHT(ch) > (2*GET_WEIGHT(victim))/3)
    turns = 0;
  else 
    turns = -1;
  
  if (3*GET_WEIGHT(ch)  < GET_WEIGHT(victim))
    base = 7;
  else if(2*GET_WEIGHT(ch)  < GET_WEIGHT(victim))
    base = 5;
  else if (3*GET_WEIGHT(ch)/2 < GET_WEIGHT(victim))
       base = 5;
  else if (GET_WEIGHT(ch) < GET_WEIGHT(victim))
    base = 2;
  else if(GET_WEIGHT(ch) > GET_WEIGHT(victim)    
	  && GET_WEIGHT(ch) < 3*GET_WEIGHT(victim)/2)
    base = -2;
  else if(GET_WEIGHT(ch) > 3*GET_WEIGHT(victim)/2
	  &&  GET_WEIGHT(ch) <  2*GET_WEIGHT(victim))
    base = -5;
  else 
    base = -7;
  
  height_difference = GET_HEIGHT(ch) - GET_HEIGHT(victim);
  if (height_difference < 0)
    height_difference *= -1;
  height_difference = 100*height_difference/GET_HEIGHT(ch);
  
  while(height_difference > 100)
    {
      base += 2;
      height_difference -= 10;
    }

  base = MAX( 1, base);
  base = MIN( base, 31);
  
  if (ch->specials.mount)
    turns += MAX(1,GET_SKILL(ch, SKILL_CAVALRY)/10);
  
  percent = number(base, 31); /* 31 is a complete failure */
  
  if (!IS_MOB(ch))
    prob = GET_SKILL(ch, SKILL_BASH);
  else 
    prob = (15*GET_LEVEL(ch))/30;
  
  if (victim->specials.mount && !IS_MOB(victim)) {
    prob -=  GET_SKILL(victim, SKILL_CAVALRY);
    prob -=  GET_SKILL(victim, SKILL_RIDING)/2;
  }
  else if (victim->specials.mount)
    prob -= (15*GET_LEVEL(victim))/30;
  
  if (GET_LEVEL(victim) >= LEVEL_BUILDER)
    percent = 31;
  if (IS_AFFECTED(ch, AFF_SLIPPY))
    prob /= 2;
  if (IS_AFFECTED(victim, AFF_SLIPPY))
    prob *= 2;
  if (IS_AFFECTED(victim, AFF_PARALYSIS))
    percent = 0;
  if (GET_POS(victim) < POSITION_STANDING)
    percent = 31;
  
  af.type = SKILL_BASH;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_BASH;
  
  if (percent > prob ||  percent > 30) {
    if (ch->specials.mount)
      if (!(throw_rider(ch))) {
	act("$n try to bash $N and nearly falls from $s mount.",FALSE,
	    ch,0,victim,TO_ROOM);
	act("$n try to bash you and nearly falls from $s mount.",FALSE,
	    ch,0,victim,TO_VICT);	  
	act("You try to bash $N and nearly falls from your mount.",FALSE,
	    ch,0,victim,TO_CHAR);
	return;
      }
    damage(ch, victim, 0, SKILL_BASH,-1,1);    
    if ((number(0,31) > GET_DEX(ch)) &&
	(GET_LEVEL(ch) < LEVEL_BUILDER)){
      GET_POS(ch) = POSITION_SITTING;
      af.duration = 1;
      if (GET_LEVEL(victim) < LEVEL_BUILDER){
	act("$n falls over.",FALSE,ch,0,0,TO_NOTCHAR);
	act("You are sent sprawling.",FALSE,ch,0,0,TO_CHAR);
      }
      if (number(0,2) && (!IS_UNDEAD(ch))){
	affect_join(ch, &af, FALSE, FALSE);
	act("$n is winded.",FALSE,ch,0,0,TO_NOTCHAR);
	act("You have the wind knocked out of you.",FALSE,ch,0,0,TO_CHAR);
      }
    }
    else{
      act("$n catches $mself before falling.",FALSE,ch,0,0,TO_NOTCHAR);
      act("You almost fall but catch yourself.",FALSE,ch,0,0,TO_CHAR);
    }
    return;
  }
  else { /* success */
    num = GET_SKILL(ch, SKILL_BASH);
    if (!number(0,(num*num)/10 + 1) && percent < 10)
      if (num < 30){
	  SET_SKILL(ch, SKILL_BASH, num + 1);
	  send_to_char("You feel more proficient at bashing.\r\n",ch);
      }
    if (victim->specials.mount)
      if (!(throw_rider(victim))) {
	act("$n trys to bash $N nearly knocking $M from $S mount.",FALSE,
	      ch,0,victim,TO_ROOM);
	act("$n trys to bash you. You nearly fall from your mount.",FALSE,
	    ch,0,victim,TO_VICT);	  
	act("You try to bash $N nearly knocking $M from $S mount.",FALSE,
	    ch,0,victim,TO_CHAR);
	return;
      }
    if (affected_by_spell(victim, SPELL_ENCASE_IN_ICE) && !number(0,3)){
      act("$n's bash shatters the ice encasing $N!",TRUE,ch,0,victim,TO_NOTVICT);
      act("$n's bash shatters the ice encasing you!",TRUE,ch,0,victim,TO_VICT);
      act("Your bash shatters the ice encasing $N.",TRUE,ch,0,victim,TO_CHAR);
      affect_from_char(victim, SPELL_ENCASE_IN_ICE);
    }
    if (ch->specials.mount && ch->specials.fighting) 
      damage(ch->specials.mount, victim, 1, SKILL_BASH,-1,0);
    else
      damage(ch, victim, 1, SKILL_BASH,-1,0);
    if (number(0,31) > GET_DEX(victim) &&
	(GET_LEVEL(victim) < LEVEL_BUILDER) ){
      GET_POS(victim) = POSITION_SITTING;
      af.duration = 1 + number(0,1)*turns;
      if (GET_LEVEL(victim) < LEVEL_BUILDER){
	act("$n falls over.",FALSE,victim,0,0,TO_NOTCHAR);
	act("You are sent sprawling.",FALSE,victim,0,0,TO_CHAR);
      }
      if (number(0,2) && !IS_UNDEAD(ch)){
	affect_join(victim, &af, FALSE, FALSE);	
	act("$n is winded.",FALSE,victim,0,0,TO_NOTCHAR);
	act("You have the wind knocked out of you.",FALSE,victim,0,0,TO_CHAR);
      }
    }
    else{
      act("$n nearly falls down but catches $mself.",FALSE,victim,0,0,TO_NOTCHAR);
      act("You almost fall but catch yourself.",FALSE,victim,0,0,TO_CHAR);
    }
  }
  
  if (IS_NPC(ch))
    ch->specials.timer = 1;
  
  WAIT_STATE(ch, PULSE_VIOLENCE);
}





ACMD(do_rescue)
{
   struct char_data *victim, *tmp_ch;
   byte	percent, prob, num;

   one_argument(argument, arg);
   if (IS_NPC(ch) && ch->specials.timer)
     return;
   
   if (!(victim = get_char_room_vis(ch, arg))) {
      send_to_char("Who do you want to rescue?\r\n", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("What about fleeing instead?\r\n", ch);
      return;
   }

   if (ch->specials.fighting == victim) {
      send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
      return;
   }

   for (tmp_ch = world[ch->in_room].people; tmp_ch && 
       (tmp_ch->specials.fighting != victim); tmp_ch = tmp_ch->next_in_room)
      ;

   if (!tmp_ch) {
      act("But nobody is fighting $M!", FALSE, ch, 0, victim, TO_CHAR);
      return;
   }



   percent = number(1, 31); /* 101% is a complete failure */
   if (!IS_MOB(ch))
       prob = GET_SKILL(ch, SKILL_RESCUE);
   else
       	prob = (15*GET_LEVEL(ch))/30;
   
   if (IS_AFFECTED(ch, AFF_SLIPPY))
       prob /= 2;
   if (percent > prob || percent > 30) {
     send_to_char("You fail the rescue!\r\n", ch);
     return;
   }
   num = GET_SKILL(ch, SKILL_RESCUE);
   if (!number(0,(num*num)/10 + 1) && percent < 10)
       if (num < 30){
	   SET_SKILL(ch, SKILL_RESCUE, num + 1);
	   send_to_char("You seem to be getting better at this hero lark.\r\n",ch);
       }

   send_to_char("Banzai!  To the rescue...\r\n", ch);
   act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
   act("$n heroically rescues $N!", FALSE, ch, 0, victim, TO_NOTVICT);
   
   if (victim->specials.fighting == tmp_ch)
     stop_fighting(victim);
   if (tmp_ch->specials.fighting)
     stop_fighting(tmp_ch);
   if (ch->specials.fighting)
     stop_fighting(ch);
   
   set_fighting(ch, tmp_ch);
   set_fighting(tmp_ch, ch);
   
    if (!IS_NPC(ch))
      ch->specials.timer = 2;
    WAIT_STATE(ch, PULSE_VIOLENCE*2);

}



ACMD(do_kick)
{
   struct char_data *victim;
   byte percent, prob, num;

   if (IS_SET(world[ch->in_room].room_flags, PEACEFULL)) 
     {
       act("You feel too peaceful to contemplate violence.",
	   FALSE,ch,0,0,TO_CHAR);
       return;
     } 

   one_argument(argument, arg);
   if (IS_NPC(ch) && ch->specials.timer)
     return;

   if (ch->specials.mount && !GET_SKILL(ch, SKILL_CAVALRY)) {
     send_to_char("You can't kick whilst mounted.\r\n",ch);
     return;
   }
   if (!(victim = get_char_room_vis(ch, arg))) {
      if (ch->specials.fighting) {
	 victim = ch->specials.fighting;
      } else {
	 send_to_char("Kick who?\r\n", ch);
	 return;
      }
   }

   if (victim == ch) {
      send_to_char("Aren't we funny today...\r\n", ch);
      return;
   }

   percent = ((10 - (GET_BODY_AC(victim) / 10))/2) + number(1, 31);
/* 31 is a complete failure */

   if (!IS_MOB(ch))
       prob = GET_SKILL(ch, SKILL_KICK);
   else
       prob = (15*GET_LEVEL(ch))/30;
   if (IS_AFFECTED(ch, AFF_SLIPPY))
       prob /= 2;
   
    if ((GET_POS(victim) < POSITION_SLEEPING)
	|| IS_AFFECTED(victim, AFF_PARALYSIS))
      prob = 30;
   
   if (percent > 30)
       botch_kick(ch,victim);
   else if (percent > prob) {
      damage(ch, victim, 0, SKILL_KICK,-1,1);
   } else{
       	num = GET_SKILL(ch, SKILL_KICK);
	if (!number(0,(num*num)/10 + 1) && percent < 10)
	    if (num < 30){
		SET_SKILL(ch, SKILL_KICK, num + 1);
		send_to_char("Woah that was a great kick!\r\n",ch);
	    }
	if (ch->specials.mount && ch->specials.fighting) 
	  damage(ch->specials.mount, victim,
		 number(GET_LEVEL(ch->specials.mount)/10,
			GET_LEVEL(ch->specials.mount)/2)
		 + GET_SKILL(ch, SKILL_CAVALRY)
		 , SKILL_KICK,-1,0);
	else
	  damage(ch, victim, number(GET_LEVEL(ch)/10, GET_LEVEL(ch)/2), SKILL_KICK,-1,0);
      if (affected_by_spell(victim, SPELL_ENCASE_IN_ICE) && !number(0,3)){
	act("$n's kick shatters the ice encasing $N!",TRUE,ch,0,victim,TO_NOTVICT);
	act("$n's kick shatters the ice encasing you!",TRUE,ch,0,victim,TO_VICT);
	act("Your kick shatters the ice encasing $N.",TRUE,ch,0,victim,TO_CHAR);
	affect_from_char(victim, SPELL_ENCASE_IN_ICE);
      }
	
   }
    if (!IS_NPC(ch))
      ch->specials.timer = 3;
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
}










