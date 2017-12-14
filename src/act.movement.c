/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct str_app_type str_app[];
extern int	rev_dir[];
extern char	*dirs[],*revdirs[];
extern int	movement_loss[];
extern int      top_of_world;
/* external functs */
char    *first_name(char *arg);
int	special(struct char_data *ch, int cmd, char *arg);
void	death_cry(struct char_data *ch);
void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
void	rmdamage(struct char_data *victim, int dam);
bool    fall_down(struct char_data *ch);

bool    fall_down(struct char_data *ch)
{
  int fall, down_room;
  
  fall = number(0,30);
  fall -= (IS_CARRYING_W(ch)+GET_WEIGHT(ch))/100;
  fall += GET_DEX(ch)/2;              
  fall += GET_STR(ch)/2;
  fall += GET_FOC(ch)/2;
  fall += GET_SKILL(ch,SKILL_CLIMB);
  down_room = NOWHERE;
  if (CAN_GO(ch,5))
    down_room = EXIT(ch,5)->to_room;
  else
    return(FALSE);
  if (down_room != NOWHERE){
    if (IS_SET(world[ch->in_room].room_flags, CLIMB_MODERATE))
      {
	if (fall < 3 && !IS_AFFECTED(ch, AFF_FLY))
	  {
	    if (!number(0,5)){
	      act("Suddenly you nearly fall! but you catch yourself."
		  ,TRUE,ch,0,0,TO_CHAR);
	      act("Suddenly $n nearly falls! but catches $mself."
		  ,TRUE,ch,0,0,TO_ROOM);
	      if (!IS_AFFECTED(ch,AFF_FLY)){
		act("You learn a new climbing move.",TRUE,ch,0,0,TO_CHAR);
		SET_SKILL(ch,SKILL_CLIMB,MIN(30,GET_SKILL(ch,SKILL_CLIMB)
					     + number (0,1)));}
	      return(FALSE);
	    }
	    
	    act("\r\nSuddenly you fall!!",TRUE,ch,0,0,TO_CHAR);
	    act("\r\n$n falls!!",TRUE,ch,0,0,TO_ROOM);
	    if (down_room)
	      {
		char_from_room(ch);
		char_to_room(ch,down_room, TRUE);
		act("\r\n$n falls down from above!",TRUE,ch,0,0,TO_ROOM);
		do_look(ch,"",0,0);
		GET_POS(ch) = POSITION_SITTING;
	      }
	    SET_SKILL(ch,SKILL_CLIMB,GET_SKILL(ch,SKILL_CLIMB)
		      - number(0,1));
	    rmdamage(ch,4*GET_MAX_HIT(ch)/10 + number(5,20));
	    act("you lose some confidence",TRUE,ch,0,0,TO_CHAR);
	  }
      }
    if (IS_SET(world[ch->in_room].room_flags, CLIMB_HARD))
      {
	if (fall < 35 && !IS_AFFECTED(ch, AFF_FLY))
	  {
	    if (!number(0,5)){
	      act("Suddenly you nearly fall! but you catch yourself."
		  ,TRUE,ch,0,0,TO_CHAR);
	      act("Suddenly $n nearly falls! but catches $mself."
		  ,TRUE,ch,0,0,TO_ROOM);
	      if (!IS_AFFECTED(ch,AFF_FLY)){
		act("You learn a new climbing move.",TRUE,ch,0,0,TO_CHAR);
		SET_SKILL(ch,SKILL_CLIMB,MIN(30,GET_SKILL(ch,SKILL_CLIMB)
					     + number (0,1)));}
	      return(FALSE);
	    }
	    
	    act("\r\nSuddenly you fall!!",TRUE,ch,0,0,TO_CHAR);
	    act("\r\n$n falls!!",TRUE,ch,0,0,TO_ROOM);
	    if (down_room)
	      {
		char_from_room(ch);
		char_to_room(ch,down_room, TRUE);
		act("\r\n$n falls down from above!",TRUE,ch,0,0,TO_ROOM);
		do_look(ch,"",0,0);
		GET_POS(ch) = POSITION_SITTING;
	      }
	    SET_SKILL(ch,SKILL_CLIMB,GET_SKILL(ch,SKILL_CLIMB)
		      - number(0,1));
	    rmdamage(ch,6*GET_MAX_HIT(ch)/10 + number(20,50));
	    act("you lose some confidence",TRUE,ch,0,0,TO_CHAR);
	  }
      }
    if (IS_SET(world[ch->in_room].room_flags, CLIMB_SEVERE))
      {
	if (fall < 50 && !IS_AFFECTED(ch, AFF_FLY))
	  {
	    if (!number(0,5)){
	      act("Suddenly you nearly fall! but you catch yourself."
		  ,TRUE,ch,0,0,TO_CHAR);
	      act("Suddenly $n nearly falls! but catches $mself."
		  ,TRUE,ch,0,0,TO_ROOM);
	      if (!IS_AFFECTED(ch,AFF_FLY)){
	      act("You learn a new climbing move.",TRUE,ch,0,0,TO_CHAR);
	      SET_SKILL(ch,SKILL_CLIMB,MIN(30,GET_SKILL(ch,SKILL_CLIMB)
					   + number (0,1)));}
	      return (FALSE);
	    }
	    
	    act("\r\nSuddenly you fall!!",TRUE,ch,0,0,TO_CHAR);
	    act("\r\n$n falls!!",TRUE,ch,0,0,TO_ROOM);
	    if (down_room)
	      {
		char_from_room(ch);
		char_to_room(ch,down_room, TRUE);
		act("\r\n$n falls down from above!",TRUE,ch,0,0,TO_ROOM);
		do_look(ch,"",0,0);
		GET_POS(ch) = POSITION_SITTING;
	      }
	    SET_SKILL(ch,SKILL_CLIMB,GET_SKILL(ch,SKILL_CLIMB)
		      - number(0,1));
	    rmdamage(ch,9*GET_MAX_HIT(ch)/10 + number(50,100));
	    act("you lose some confidence",TRUE,ch,0,0,TO_CHAR);
	  }
      }
    if (IS_SET(world[ch->in_room].room_flags, CLIMB_EXTREME))
      {
	if (fall < 75 && !IS_AFFECTED(ch, AFF_FLY))
	  {
	    if (!number(0,5)){
	      act("Suddenly you nearly fall! but you catch yourself."
		  ,TRUE,ch,0,0,TO_CHAR);
	      act("Suddenly $n nearly falls! but catches $mself."
		  ,TRUE,ch,0,0,TO_ROOM);
	      if (!IS_AFFECTED(ch,AFF_FLY)){
		act("You learn a new climbing move.",TRUE,ch,0,0,TO_CHAR);
		SET_SKILL(ch,SKILL_CLIMB,MIN(30,GET_SKILL(ch,SKILL_CLIMB)
					     + number (0,2)));}
	      return(FALSE);
	    }
	    act("\r\nSuddenly you fall!!",TRUE,ch,0,0,TO_CHAR);
	    act("\r\n$n falls!!",TRUE,ch,0,0,TO_ROOM);
	    if (down_room)
	      {
		char_from_room(ch);
		char_to_room(ch,down_room, TRUE);
		act("\r\n$n falls down from above!",TRUE,ch,0,0,TO_ROOM);
		do_look(ch,"",0,0);
		GET_POS(ch) = POSITION_SITTING;
	      }
	    SET_SKILL(ch,SKILL_CLIMB,GET_SKILL(ch,SKILL_CLIMB)
		      - number(1,2));
	    rmdamage(ch,49*GET_MAX_HIT(ch)/50 + number(100,200));
	    act("you lose some confidence",TRUE,ch,0,0,TO_CHAR);
	  }
      }
    return(TRUE);
  }
  return(FALSE);   
}

bool throw_rider(struct char_data *ch) {
  int num;
  struct char_data *tmp;
  
  if (!ch->specials.mount)
    return(FALSE);
  
  if (GET_SKILL(ch, SKILL_RIDING) < (num = number(0,31)) && !number(0,31)) 
    if (GET_SKILL(ch, SKILL_RIDING) < (num = number(0,31))) {
      act("$N slips and throws $n!", FALSE,ch,0,ch->specials.mount,TO_ROOM);
      act("$N slips and throws you!", FALSE,ch,0,ch->specials.mount,TO_CHAR);
      act("You slip and throws $n!", FALSE,ch,0,ch->specials.mount,TO_VICT);
      GET_POS(ch) = POSITION_SITTING;
      rmdamage(ch,GET_HEIGHT(ch->specials.mount)/number(5,20));
      if (num == 0 && (number(0,31) == 0)
	  && (number(0,31) > GET_SKILL(ch, SKILL_RIDING))
	  && (GET_SKILL(ch, SKILL_RIDING) < 30)) {
	send_to_char("You realise how you can improve your riding technique\r\n.",ch);
	SET_SKILL(ch, SKILL_RIDING,(GET_SKILL(ch,SKILL_RIDING) + 1));
	rmdamage(ch,GET_HEIGHT(ch->specials.mount)/number(5,20));
      }
      tmp = ch->specials.mount;
      unmount(ch);
      if (!number(0,10) && IS_MOB(tmp) && (GET_INT(tmp) < 10))
	do_flee(tmp,"",0,0);
      return(TRUE);
    }
  return(FALSE);
}

int	do_simple_move(struct char_data *ch, int cmd, int following)

/* Assumes, 
	1. That there is no master and no followers.
	2. That the direction exists. 

   Returns :
   1 : If succes.
   0 : If fail
  -1 : If dead.
*/
{
  int	special(struct char_data *ch, int cmd, char *arg);
  int	need_movement, target_sect, curr_sect, nteam, total_weight;
  int delay,num;
  struct char_data *tmp;
  struct obj_data *obj, *cart;
  struct room_data *target_room;
  struct follow_type *k;
  bool has_boat, cart_move;

  
  if (special(ch, cmd + 1, ""))  /* Check for special routines (North is 1) */
    return(FALSE);
  if (ch->specials.mount)
    if ((GET_POS(ch->specials.mount) != POSITION_STANDING)
	|| IS_AFFECTED(ch->specials.mount, AFF_PARALYSIS)
	|| MOB_FLAGGED(ch->specials.mount, MOB_TETHERED)
	){
      send_to_char("Your mount seems unable to move.\r\n",ch);
      return(FALSE);
    }
  if (IS_MOB(ch) && MOB_FLAGGED(ch,MOB_TETHERED))
    return(FALSE);
  
  if (IS_SET(world[real_room(world[ch->in_room].dir_option[cmd]->to_room)].room_flags,UNFINISHED) && GET_LEVEL(ch) < LEVEL_BUILDER){
    send_to_char("A strange force bars your movement in that direction.\r\n",ch);
    return(FALSE);
  }
  if ((world[ch->in_room].dir_option[cmd]->to_room == 0) &&
      world[ch->in_room].dir_option[cmd]->general_description){
    send_to_char( world[ch->in_room].dir_option[cmd]->general_description,ch);
    return(FALSE);
  }
  else if (world[ch->in_room].dir_option[cmd]->to_room == 0){
    send_to_char("A strange force bars your movement in that direction.\r\n",ch);
    return(FALSE);
  }
  target_room =&world[real_room(world[ch->in_room].dir_option[cmd]->to_room)];
  target_sect= target_room->sector_type;
  curr_sect = world[ch->in_room].sector_type;
  if (IS_PULLING(ch)
      && (target_sect != SECT_CITY
	  && target_sect != SECT_FIELD
	  && target_sect != SECT_HILLS
	  && target_sect != SECT_FOREST)){
    send_to_char("You can't drag cart there.\r\n",ch);
    return(FALSE);}
  if (ch->specials.mount && (target_sect == SECT_INSIDE)) {
    send_to_char("You can't ride indoors.\r\n",ch);
    return(FALSE);
  }
  if (IS_AFFECTED(ch, AFF_SLIPPY) && !number(0,1)){
    act("$n slips and falls down.", TRUE, ch, 0,0,TO_ROOM);
    send_to_char("You slip and fall down.\r\n",ch);
    GET_POS(ch) = POSITION_SITTING;
    if (ch->specials.rider)
      if(throw_rider(ch->specials.rider))
	;
    if (ch->specials.carrying)
      do_drop_mob(ch);
    return(FALSE);
  }
  need_movement = (movement_loss[curr_sect] + movement_loss[target_sect])/ 2;
  if (IS_AFFECTED(ch,AFF_FLY) &&
      ((curr_sect != SECT_UNDER_WATER) || (target_sect != SECT_UNDER_WATER)))
    need_movement = 1;
  /* 9-22-94: patch by Scarrow to make water rooms easier with a boat */
  has_boat = FALSE;
  /* See if char is carrying a boat */
  for (obj = ch->inventory; obj; obj = obj->next_content)
    if (obj->obj_flags.type_flag == ITEM_BOAT)
      has_boat = TRUE;
  /* if it's water (swim or noswim) and we have a boat and aren't flying */
  /* halve the computed cost of movement -- Scarrow (9-22-94) */
  if (((world[ch->in_room].sector_type == SECT_WATER_SWIM) || 
       (world[ch->in_room].sector_type == SECT_WATER_NOSWIM) ||
       (target_room->sector_type == SECT_WATER_SWIM) ||
       (target_room->sector_type == SECT_WATER_NOSWIM)) &&
      has_boat && (!IS_AFFECTED(ch, AFF_FLY) ||
		   (ch->specials.mount && !IS_AFFECTED(ch->specials.mount, AFF_FLY)))) {
    need_movement /= 2;
  }
  if ((world[ch->in_room].sector_type == SECT_WATER_NOSWIM) || 
      (target_room->sector_type == SECT_WATER_NOSWIM)) {
    if (!has_boat && !IS_AFFECTED(ch,AFF_FLY) && !ch->specials.carried_by &&
	!IS_AFFECTED(ch, AFF_WATER_BREATH)
	&& !(ch->specials.mount &&(IS_AFFECTED(ch->specials.mount,AFF_FLY) ||
				   IS_AFFECTED(ch->specials.mount, AFF_WATER_BREATH)))) {
      send_to_char("You thrash about trying to stay afloat.\r\n", ch);
      need_movement *= 2;
    }
  }
  
  if ((world[ch->in_room].sector_type == SECT_FLY) || 
      (target_room->sector_type == SECT_FLY)) {
    if (!IS_AFFECTED(ch,AFF_FLY) && !ch->specials.carried_by
	&& !(ch->specials.mount && IS_AFFECTED(ch->specials.mount,AFF_FLY))) {
      send_to_char("You would need wings to go there.\r\n", ch);
      return(FALSE);
    }
  }
  if ((world[ch->in_room].sector_type == SECT_THICKET) || 
      (world[real_room(world[ch->in_room].dir_option[cmd]->to_room)].sector_type == SECT_THICKET)) {
    if (GET_LEVEL(ch) < LEVEL_BUILDER  && GET_SIZE(ch) < 6) {
      send_to_char("Ouch! you are raked by briar thorns.\r\n", ch);
      if (ch->specials.mount) {
	rmdamage(ch,number(4,32));
	rmdamage(ch->specials.mount,number(4,32));	     
      }
      else
	rmdamage(ch,number(2,16));
      
    }
    else
      send_to_char("The briar thorns bounce harmlessly off your skin.\r\n", ch);	
  }
   
  if ((world[ch->in_room].sector_type == SECT_UNDER_WATER) || 
      (target_room->sector_type == SECT_UNDER_WATER)) {
    if (!IS_AFFECTED(ch,AFF_WATER_BREATH) && !ch->specials.carried_by &&
	!(ch->specials.mount
	  && IS_AFFECTED(ch->specials.mount,AFF_WATER_BREATH))) {
      send_to_char("You thrash about. You are Drowning!\r\n", ch);
      need_movement *= 2;

    }
  }
  
  if (IS_SET(target_room->room_flags,SMALL))
    if ((GET_SIZE(ch) < 4) ||
	(ch->specials.mount && (GET_SIZE(ch->specials.mount) < 4))){
      send_to_char("You bump your head.\r\n",ch);
      send_to_char("You can't go there, you are too big.\r\n",ch);
      return(FALSE);}
    else if ( (GET_SIZE(ch) < 5) ||
	      (ch->specials.mount && (GET_SIZE(ch->specials.mount) < 5)))
      send_to_char("You bump your head.\r\n",ch);
  if (IS_SET(target_room->room_flags,TINY))
    if ( (GET_SIZE(ch) < 5) ||
	 (ch->specials.mount && (GET_SIZE(ch->specials.mount) < 5))){
      send_to_char("You bump your head.\r\n",ch);	   
      send_to_char("You can't go there, you are too big.\r\n",ch);
      return(FALSE);}
    else if ((GET_SIZE(ch) < 6) ||
	     (ch->specials.mount && (GET_SIZE(ch->specials.mount) < 6)))
      send_to_char("You bump your head.\r\n",ch);

  if ((cmd == 4 || cmd ==5) && !ch->specials.carried_by &&
      !(IS_AFFECTED(ch,AFF_FLY) ||
	(ch->specials.mount && IS_AFFECTED(ch->specials.mount,AFF_FLY))) &&
      (((IS_SET(world[TO_RM(ch,cmd)].room_flags,CLIMB_MODERATE) ||
	 IS_SET(world[ch->in_room].room_flags, CLIMB_MODERATE))
	&& (GET_SKILL(ch,SKILL_CLIMB) < 8) && number(0,3)) ||
       ((IS_SET(world[TO_RM(ch,cmd)].room_flags,CLIMB_HARD) ||
	 IS_SET(world[ch->in_room].room_flags, CLIMB_HARD))
	&& (GET_SKILL(ch,SKILL_CLIMB) < 18) && number(0,6)) ||
       ((IS_SET(world[TO_RM(ch,cmd)].room_flags,CLIMB_SEVERE) ||
	 IS_SET(world[ch->in_room].room_flags, CLIMB_SEVERE))
	&& (GET_SKILL(ch,SKILL_CLIMB) < 26) && number(0,8)) ||
       ((IS_SET(world[TO_RM(ch,cmd)].room_flags,CLIMB_EXTREME) ||
	 IS_SET(world[ch->in_room].room_flags, CLIMB_EXTREME))
	&& (GET_SKILL(ch,SKILL_CLIMB) < 29) && number(0,15))))
    {
      if (ch->specials.mount) {
	act("$N cannot manage the climb.",FALSE,ch,0,ch->specials.mount,TO_CHAR);
	return(FALSE);
      }
      send_to_char("You aren't a good enough climber.\r\n",ch);
      if (IS_CLIMB(ch, cmd))
	 fall_down(ch);
      if (cmd != 5){
	send_to_char("You fall back onto your backside.\r\n",ch);
	GET_POS(ch) = POSITION_SITTING;
	if (!IS_AFFECTED(ch,AFF_FLY) && !ch->specials.carried_by){
	  if ((IS_SET(world[ch->in_room].room_flags, CLIMB_MODERATE) && (number(0,20) <= 1)) || (IS_SET(world[ch->in_room].room_flags, CLIMB_HARD) && (number(0,20) <= 3)) || (IS_SET(world[ch->in_room].room_flags, CLIMB_SEVERE) && (number(0,20) <= 5)) || (IS_SET(world[ch->in_room].room_flags, CLIMB_EXTREME) && (number(0,20) <= 8))){
	    act("You learn a new climbing move.",TRUE,ch,0,0,TO_CHAR);
	    SET_SKILL(ch,SKILL_CLIMB,MIN(30,GET_SKILL(ch,SKILL_CLIMB) + number (0,2)));
	  }
	}
      }
      return(FALSE);
    }
   
  if (IS_PULLING(ch)){
    total_weight = 0;
    for (k = IS_PULLING(ch)->pulled_by;k;k = k->next)
      total_weight += GET_WEIGHT(k->follower);
    need_movement += (need_movement*GET_OBJ_WEIGHT(IS_PULLING(ch))/
		      total_weight);}
  need_movement *= encumberance_level(ch);
  if (ch->specials.mount) 
    if ((GET_MOVE(ch->specials.mount) < 2*need_movement) &&
	(GET_HIT(ch->specials.mount) < GET_MAX_HIT(ch->specials.mount)/4)) {
      send_to_char("Your mount is too exhausted.\r\n", ch);
      return(FALSE);
    }
  if ((GET_MOVE(ch) < need_movement) &&
      (GET_HIT(ch)  < GET_MAX_HIT(ch)/4)
      ) {
    if (following && !ch->master)
      send_to_char("You are too exhausted to follow.\r\n", ch);
    else
      send_to_char("You are too exhausted.\r\n", ch);

    return(FALSE);
  }
  if (!ch->specials.carried_by) {
    if (ch->specials.mount) {
      GET_MOVE(ch) -= need_movement/4;
      GET_MOVE(ch->specials.mount) -=
	need_movement*(2*CAN_CARRY_W(ch->specials.mount)
		       /(GET_WEIGHT(ch)+ IS_CARRYING_W(ch)));
      if (GET_MOVE(ch) < 0)
	GET_HIT(ch) -= MAX(1,GET_HIT(ch)/4);
    }
    else {
      if (GET_LEVEL(ch) < LEVEL_BUILDER || IS_NPC(ch)){
	GET_MOVE(ch) -= need_movement;
	if (GET_MOVE(ch) < 0){
	  send_to_char("You are wracked with pain as you overexert yourself.\r\n",ch);
	  act("Though exhausted $n manages to move.",FALSE,ch,0,0,TO_NOTCHAR);
	  GET_HIT(ch) -= MAX(1,GET_HIT(ch)/4);
	}
      }
    }
  }
  if (ch->specials.mount) 
    delay = (need_movement*GET_MAX_MOVE(ch->specials.mount))
      /(MAX(1,GET_MOVE(ch->specials.mount))
	*(10 + GET_STR(ch->specials.mount)/2));
  else
    delay = need_movement/10;
  if (ch->specials.mount)
    if (throw_rider(ch))
      return(FALSE);
   
  if ((cart = IS_PULLING(ch))){
    /* add leave events for all hitched  */
    cart_move = TRUE;
    nteam = 0;
    for (k = cart->pulled_by;k;k = k->next)
      if (GET_MOVE(k->follower) < 0) {
	nteam++;
	cart_move = FALSE;
      }
    if(cart_move){
      for (k = cart->pulled_by;k;k = k->next){
	WAIT_STATE(k->follower,delay);
	add_event(0, EVENT_LEAVE, cmd,has_boat,0,0,0,k->follower,0);}
      /* and the cart itself */
      add_event(0, EVENT_CART_LEAVE,cmd,cart->in_room,0,0,0,cart,0);
    }
    else{
      if (nteam > 1){
	act("The team strains to move $p."
	    ,TRUE,cart->pulled_by->follower,cart,0,TO_ROOM);
	act("But they are too exhausted."
	    ,TRUE,cart->pulled_by->follower,cart,0,TO_ROOM);}
      else{
	act("$n strains to move $p."
	    ,TRUE,cart->pulled_by->follower,cart,0,TO_ROOM);
	act("But $e is too exhausted."
	    ,TRUE,cart->pulled_by->follower,cart,0,TO_ROOM);}
    }
  }
  else
    add_event(0, EVENT_LEAVE, cmd,has_boat,0,0,0,ch,0);

  WAIT_STATE(ch,delay);
  if (ch->desc)
    ch->desc->prompt_mode =0;
  if ((cart = IS_PULLING(ch))){
    if(cart_move){
      for (k = cart->pulled_by;k;k = k->next)
	add_event(delay, EVENT_ARRIVE,
		  cmd,world[ch->in_room].dir_option[cmd]->to_room
		  ,0,0,0,k->follower,0);
      /* and the cart itself */
      add_event(delay, EVENT_CART_ARRIVE,cmd
		,world[ch->in_room].dir_option[cmd]->to_room
		,0,0,0,cart,0);
    }
  }
  else   
    add_event(delay, EVENT_ARRIVE, cmd,world[ch->in_room].dir_option[cmd]->to_room,0,0,0,ch,0);
  return(1);
}


ACMD(do_move)
{
  int	was_in,ex,nexits,which;
  struct follow_type *k, *next_dude;
  
  
  if (ch->specials.carried_by) {
    act("Ask $N to drop you first.",
	FALSE,ch,0,ch->specials.carried_by,TO_CHAR);
    return;
  }
  if (IS_SET(world[ch->in_room].room_flags, SPIN)){
    nexits=0;       
    for (ex=0;ex < NUM_OF_DIRS;ex++)
      {
	if (EXIT(ch,ex))
	  nexits++;
      }
    if (nexits >1) {
      which = number(1,nexits);
      nexits=0;
      for (ex=0;ex < NUM_OF_DIRS && nexits < which;ex++)
	{
	  if (EXIT(ch,ex))
	    nexits++;
	}
      cmd = ex;
    }
  }
  --cmd;
   
  if (!world[ch->in_room].dir_option[cmd]) {
    send_to_char("Alas, you cannot go that way...\r\n", ch);
  } else {          /* Direction is possible */
    if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED)
	&& !IS_SET(EXIT(ch,cmd)->exit_info,EX_SECRET)
	&& !IS_SET(EXIT(ch,cmd)->exit_info,EX_DARK)) {
      if (EXIT(ch, cmd)->keyword) {
	sprintf(buf2, "The %s seems to be closed.\r\n",
	        fname(EXIT(ch, cmd)->keyword));
	send_to_char(buf2, ch);
      }
      else
	send_to_char("It seems to be closed.\r\n", ch);
    }
    else if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED)
	     && (IS_SET(EXIT(ch,cmd)->exit_info,EX_SECRET)
		 || IS_SET(EXIT(ch,cmd)->exit_info,EX_DARK)))
      send_to_char("Alas, you cannot go that way...\r\n", ch);
    else if (EXIT(ch, cmd)->to_room == NOWHERE)
      send_to_char("Alas, you cannot go that way...\r\n", ch);
    else if (!ch->followers && !ch->master)
      do_simple_move(ch, cmd, FALSE);
    else {
      was_in = ch->in_room;
      if (do_simple_move(ch, cmd, TRUE) == 1) { /* Move the character */
	if (ch->followers) { /* If succes move followers */
	  for (k = ch->followers; k; k = next_dude) {
	    next_dude = k->next;
	    if ((was_in == k->follower->in_room) && 
		(GET_POS(k->follower) >= POSITION_STANDING) &&
		!k->follower->specials.fighting){
	      if(IS_NPC(ch) && !number(0,3) && GET_LEVEL(k->follower) < LEVEL_BUILDER && !IS_NPC(k->follower)){
		act("You lost track of $n.",TRUE,ch,0,k->follower,TO_VICT);
		break;}
	      else {
		act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
		do_move(k->follower, argument, cmd + 1, 0);
	      }
	    }
	  }
	}
      }
    }
  }
}



int	find_door(struct char_data *ch, char *type, char *dir)
{
   int	door;
   char	*dirs[] = 
    {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "\n"
   };

   if (*dir) /* a direction was specified */ {
      if ((door = search_block(dir, dirs, FALSE)) == -1) /* Partial Match */ {
	 send_to_char("That's not a direction.\r\n", ch);
	 return(-1);
      }

      if (EXIT(ch, door))
	 if (EXIT(ch, door)->keyword)
	    if (isname(type, EXIT(ch, door)->keyword))
	       return(door);
	    else {
	       sprintf(buf2, "I see no %s there.\r\n", type);
	       send_to_char(buf2, ch);
	       return(-1);
	    }
	 else
	    return(door);
      else {
	 send_to_char("I really don't see how you can close anything there.\r\n", ch);
	 return(-1);
      }
   }
   else /* try to locate the keyword */	 {
       for (door = 0; door < NUM_OF_DIRS; door++)
	   if (EXIT(ch, door))
	       if (EXIT(ch, door)->keyword)
		   if (isname(type, EXIT(ch, door)->keyword))
		       return(door);
       
       sprintf(buf2, "I see no %s here.\r\n", type);
       send_to_char(buf2, ch);
       return(-1);
   }
}


ACMD(do_open)
{
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;

   argument_interpreter(argument, type, dir);

   if (!*type)
      send_to_char("Open what?\r\n", ch);
   else if (generic_find(argument,FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP
			 ,ch, &victim, &obj))

      /* this is an object */

      if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	 send_to_char("That's not a container.\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	 send_to_char("But it's already open!\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
	 send_to_char("You can't do that.\r\n", ch);
      else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	 send_to_char("It seems to be locked.\r\n", ch);
      else
       {
	 REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	 act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
	 act("you open $p.", FALSE, ch, obj, 0, TO_CHAR);	 
      }
   else if ((door = find_door(ch, type, dir)) >= 0)

      /* perhaps it is a door */

      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's impossible, I'm afraid.\r\n", ch);
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("It's already open!\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	 send_to_char("It seems to be locked.\r\n", ch);
      else {
	 REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	 if (EXIT(ch, door)->keyword ){
	     if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET))
		 act("$n opens a hidden passage.", FALSE, ch, 0,0, TO_ROOM);
	     else
		 act("$n opens the $F.", FALSE, ch, 0,
		     EXIT(ch, door)->keyword, TO_ROOM);
	 act("You open the $F.", FALSE, ch, 0,
	     EXIT(ch, door)->keyword, TO_CHAR);
	 }
	 else if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET)){
	     act("$n opens a hidden passage.", FALSE, ch, 0,0, TO_ROOM);
	     act("you open the door.", FALSE, ch, 0,0, TO_CHAR);
	 }
	 else{
	    act("$n opens a door.", FALSE, ch, 0, 0, TO_ROOM);
	    act("you open the door.", FALSE, ch, 0,0, TO_CHAR);
	 }
	 /* now for opening the OTHER side of the door! */
	 if ((other_room = real_room(EXIT(ch, door)->to_room)) != NOWHERE)
	    if ((back = world[other_room].dir_option[rev_dir[door]]))
	       if (world[ch->in_room].number == back->to_room) {
		  REMOVE_BIT(back->exit_info, EX_CLOSED);
		  if (back->keyword) {
		      if (IS_SET(back->exit_info, EX_SECRET))
			  sprintf(buf, "A hidden passage is opened from the other side.\r\n");
		      else
			  sprintf(buf, "The %s is opened from the other side.\r\n",fname(back->keyword));
		      send_to_room(buf, EXIT(ch, door)->to_room, TRUE);
		  }
		  else if (IS_SET(back->exit_info, EX_SECRET))
		      send_to_room("A hidden passage is opened from the other side.\r\n", EXIT(ch, door)->to_room, TRUE);
		  else
		      send_to_room("The door is opened from the other side.\r\n",
		         EXIT(ch, door)->to_room, TRUE);
	       }
      }
}


ACMD(do_close)
{
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;

   argument_interpreter(argument, type, dir);

   if (!*type)
      send_to_char("Close what?\r\n", ch);
   else if (generic_find(argument,FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP
			 ,ch, &victim, &obj))

      /* this is an object */

      if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	 send_to_char("That's not a container.\r\n", ch);
      else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	 send_to_char("But it's already closed!\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
	 send_to_char("That's impossible.\r\n", ch);
      else {
	 SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	 act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
	 act("You close $p.", FALSE, ch, obj, 0, TO_CHAR);	 
      }
   else if ((door = find_door(ch, type, dir)) >= 0)

      /* Or a door */

      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's absurd.\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("It's already closed!\r\n", ch);
      else {
	 SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	 if (EXIT(ch, door)->keyword){
	     if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET))
		 act("$n closes a hidden passage.", 0, ch, 0,0,TO_ROOM);
	     else
		 act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
		     TO_ROOM);
	     act("You close the $F.", 0, ch, 0, EXIT(ch, door)->keyword,TO_CHAR);
	 }
	 else if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET)){
	     act("$n closes a hidden passage.", 0, ch, 0,0,TO_ROOM);
	     act("You close a hidden passage.", 0, ch, 0,0,TO_CHAR);	     
	 }
	 else{
	     act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
	     act("You close the door.", FALSE, ch, 0, 0, TO_CHAR);
	 }
	 /* now for closing the other side, too */
	 if ((other_room = real_room(EXIT(ch, door)->to_room)) != NOWHERE)
	    if ((back = world[other_room].dir_option[rev_dir[door]]))
	       if (world[ch->in_room].number == back->to_room) {
		  SET_BIT(back->exit_info, EX_CLOSED);
		  if (back->keyword) {
		      if (IS_SET(back->exit_info, EX_SECRET))
			  sprintf(buf, "A hidden passage closes quietly.\r\n");
		      else
			  sprintf(buf, "The %s closes quietly.\r\n", fname(back->keyword));
		     send_to_room(buf, EXIT(ch, door)->to_room, TRUE);
		  } else
		      if (IS_SET(back->exit_info, EX_SECRET))
			  send_to_room("A hidden passage closes quietly.\r\n", EXIT(ch, door)->to_room, TRUE);
		      else
			  send_to_room("The door closes quietly.\r\n", EXIT(ch, door)->to_room, TRUE);
	       }
      }
}


int	has_key(struct char_data *ch, int key)
{
   struct obj_data *o;

   for (o = ch->inventory; o; o = o->next_content)
      if (obj_index[o->item_number].virtual == key)
	 return(1);

   if (ch->equipment[HOLD])
      if (obj_index[ch->equipment[HOLD]->item_number].virtual == key)
	 return(1);

   return(0);
}


ACMD(do_lock)
{
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;


   argument_interpreter(argument, type, dir);
  if (ch->specials.mount) {
    send_to_char("You had better dismount first.\r\n", ch);
    return;}    
   if (!*type)
      send_to_char("Lock what?\r\n", ch);
   else if (generic_find(argument,FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP
			 ,ch, &victim, &obj))

      /* this is an object */

      if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	 send_to_char("That's not a container.\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	 send_to_char("Maybe you should close it first...\r\n", ch);
      else if (obj->obj_flags.value[2] < 0)
	 send_to_char("That thing can't be locked.\r\n", ch);
      else if (!has_key(ch, obj->obj_flags.value[2]))
	 send_to_char("You don't seem to have the proper key.\r\n", ch);
      else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	 send_to_char("It is locked already.\r\n", ch);
      else {
	 SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	 send_to_char("*Cluck*\r\n", ch);
	 act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
      }
   else if ((door = find_door(ch, type, dir)) >= 0)

      /* a door, perhaps */

      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's absurd.\r\n", ch);
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("You have to close it first, I'm afraid.\r\n", ch);
      else if (EXIT(ch, door)->key < 0)
	 send_to_char("There does not seem to be any keyholes.\r\n", ch);
      else if (!has_key(ch, EXIT(ch, door)->key))
	 send_to_char("You don't have the proper key.\r\n", ch);
      else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	 send_to_char("It's already locked!\r\n", ch);
      else {
	 SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	 if (EXIT(ch, door)->keyword)
	     if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET))
		 act("$n locks a hidden door.", FALSE, ch, 0,0, TO_ROOM);
	     else
		 act("$n locks the $F.", 0, ch, 0,  EXIT(ch, door)->keyword,
		     TO_ROOM);
	 else if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET))
	     act("$n locks a hidden door.", FALSE, ch, 0,0, TO_ROOM);
	 else
	     act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
	 send_to_char("*Click*\r\n", ch);
	 /* now for locking the other side, too */
	 if ((other_room = real_room(EXIT(ch, door)->to_room)) != NOWHERE)
	    if ((back = world[other_room].dir_option[rev_dir[door]]))
	       if (back->to_room == world[ch->in_room].number)
		  SET_BIT(back->exit_info, EX_LOCKED);
      }
}


ACMD(do_unlock)
{
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;


   argument_interpreter(argument, type, dir);
   
  if (ch->specials.mount) {
    send_to_char("You had better dismount first.\r\n", ch);
    return;}    

   if (!*type)
      send_to_char("Unlock what?\r\n", ch);
   else if (generic_find(argument,FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP
			 ,ch, &victim, &obj))

      /* this is an object */

      if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	 send_to_char("That's not a container.\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	 send_to_char("Silly - it ain't even closed!\r\n", ch);
      else if (obj->obj_flags.value[2] < 0)
	 send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
      else if (!has_key(ch, obj->obj_flags.value[2]))
	 send_to_char("You don't seem to have the proper key.\r\n", ch);
      else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	 send_to_char("Oh.. it wasn't locked, after all.\r\n", ch);
      else {
	 REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	 send_to_char("*Click*\r\n", ch);
	 act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
   else if ((door = find_door(ch, type, dir)) >= 0)

      /* it is a door */

      if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	 send_to_char("That's absurd.\r\n", ch);
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	 send_to_char("Heck.. it ain't even closed!\r\n", ch);
      else if (EXIT(ch, door)->key < 0)
	 send_to_char("You can't seem to spot any keyholes.\r\n", ch);
      else if (!has_key(ch, EXIT(ch, door)->key))
	 send_to_char("You do not have the proper key for that.\r\n", ch);
      else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	 send_to_char("It's already unlocked, it seems.\r\n", ch);
      else {
	 REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	 if (EXIT(ch, door)->keyword)
	     if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET))
		 act("$n unlocks a hidden door.", FALSE, ch, 0,0, TO_ROOM);
	     else
		 act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
	        TO_ROOM);
	 else if (IS_SET(EXIT(ch,door)->exit_info, EX_SECRET))
	     act("$n unlocks a hidden door.", FALSE, ch, 0,0, TO_ROOM);
	 else
	     act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
	 send_to_char("*click*\r\n", ch);
	 /* now for unlocking the other side, too */
	 if ((other_room = real_room(EXIT(ch, door)->to_room)) != NOWHERE)
	    if ((back = world[other_room].dir_option[rev_dir[door]]))
	       if (back->to_room == world[ch->in_room].number)
		  REMOVE_BIT(back->exit_info, EX_LOCKED);
      }
}





ACMD(do_pick)
{
   byte percent;
   int	door, other_room;
   char	type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
   struct room_direction_data *back;
   struct obj_data *obj;
   struct char_data *victim;

   argument_interpreter(argument, type, dir);

   percent = number(1, 31); /* 101% is a complete failure */

   if ((IS_NPC(ch) && percent > GET_LEVEL(ch)/10) ||
       (!IS_NPC(ch) && (percent > GET_SKILL(ch, SKILL_PICK_LOCK)))) {
     send_to_char("You failed to pick the lock.\r\n", ch);
     return;
   }
   if (ch->specials.mount) {
     send_to_char("Pick locks while riding?  Fat chance.\r\n", ch);
     return;}    
   
   if (!*type)
     send_to_char("Pick what?\r\n", ch);
   else if (generic_find(argument,FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP
			 , ch, &victim, &obj))
     
     /* this is an object */
     
     if (obj->obj_flags.type_flag != ITEM_CONTAINER)
       send_to_char("That's not a container.\r\n", ch);
     else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
       send_to_char("Silly - it isn't even closed!\r\n", ch);
     else if (obj->obj_flags.value[2] < 0)
       send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
     else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
       send_to_char("Oho! This thing is NOT locked!\r\n", ch);
     else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
       send_to_char("It resists your attempts at picking it.\r\n", ch);
     else {
       REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
       send_to_char("*Click*\r\n", ch);
       act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
     }
   else if ((door = find_door(ch, type, dir)) >= 0)
     if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
       send_to_char("That's absurd.\r\n", ch);
     else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
       send_to_char("You realize that the door is already open.\r\n", ch);
     else if (EXIT(ch, door)->key < 0)
       send_to_char("You can't seem to spot any lock to pick.\r\n", ch);
     else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
       send_to_char("Oh.. it wasn't locked at all.\r\n", ch);
     else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
       send_to_char("You seem to be unable to pick this lock.\r\n", ch);
     else {
       REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
       if (EXIT(ch, door)->keyword)
	 act("$n skillfully picks the lock of the $F.", 0, ch, 0,
	        EXIT(ch, door)->keyword, TO_ROOM);
       else
	    act("$n picks the lock of the door.", TRUE, ch, 0, 0, TO_ROOM);
       send_to_char("The lock quickly yields to your skills.\r\n", ch);
       /* now for unlocking the other side, too */
       if ((other_room = real_room(EXIT(ch, door)->to_room)) != NOWHERE)
	 if ((back = world[other_room].dir_option[rev_dir[door]]))
	   if (back->to_room == world[ch->in_room].number)
	     REMOVE_BIT(back->exit_info, EX_LOCKED);
     }
}


ACMD(do_enter)
{
   int	door, was_in;
   struct follow_type *k, *next_dude;
   struct obj_data *cont;

   ACMD(do_move);

   one_argument(argument, buf);

  if (ch->specials.carried_by) {
    act("Ask $N to drop you first.",
	FALSE,ch,0,ch->specials.carried_by,TO_CHAR);
    return;
  }
   
   if (*buf)  /* an argument was supplied, search for door keyword */ {
      for (door = 0; door < NUM_OF_DIRS; door++)
	 if (EXIT(ch, door))
	    if (EXIT(ch, door)->keyword)
	       if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
		  do_move(ch, "", ++door, 0);
		  return;
	       }
      /* ok not a door how about an object */
      if ((cont = get_obj_in_list_vis(ch,buf,world[ch->in_room].contents)))
	  {
	      if (HASROOM(cont))
		  if (!IS_SET(cont->obj_flags.value[1],CONT_CLOSED))
		      {
			  if (GET_OBJ_SIZE(cont) - GET_SIZE(ch) > 1){
			      sprintf(buf1,
				      "The %s is too small you can't fit."
				      ,first_name(cont->name));
			      act(buf1,TRUE,ch,0,0,TO_CHAR);
			      return;
			  }				      
			  if ((GET_WEIGHT(ch)
			       + IS_CARRYING_W(ch) +
			       + GET_OBJ_WEIGHT(cont))
			       > cont->obj_flags.value[0])
			      {
				  sprintf(buf1,
					  "There is no room in the %s."
					  ,first_name(cont->name));
				  act(buf1,TRUE,ch,0,0,TO_CHAR);
				  return;
			      }
                          if(real_room(cont->obj_flags.value[3]) == -1){
		             send_to_char("You can't enter that!\r\n",ch);
                             return;
                          }
			  act("$n enters $p.",TRUE,ch,cont,0,TO_ROOM);
			  act("You enter $p.",TRUE,ch,cont,0,TO_CHAR);
			  was_in = ch->in_room;
			  char_from_room(ch);
			  char_to_room(ch,cont->obj_flags.value[3],
				       TRUE);
			  do_look(ch,"",0,0);
			  if (!IS_SET(world[ch->in_room]
			    .obj->obj_flags.value[1], CONT_ONEWAY))
			      act("$n enters $p.",TRUE,ch,cont,0,TO_ROOM);
			  else
			      act("Suddenly, $n arrives!",TRUE,ch,cont,0,TO_ROOM);
			  if (ch->followers) {
			    for (k = ch->followers; k; k = next_dude) {
			      next_dude = k->next;
			      if ((k->follower->in_room == was_in) &&
				  (GET_POS(k->follower) >= POSITION_STANDING)
				  && !k->follower->specials.fighting){
				if(IS_NPC(ch) && !number(0,3)
				   && GET_LEVEL(k->follower)
				   < LEVEL_BUILDER && !IS_NPC(k->follower)){
				  act("You lost track of $n.",TRUE,ch,0,k->follower,TO_VICT);
				  break;}
				else {
				  act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
				  do_enter(k->follower, argument,0,0);
				}
			      }
			    }
			  }
			  return;
		      }
		  else
		      {
			  sprintf(buf1,"The %s seems to be closed."
				  ,first_name(cont->name));
			  act(buf1,TRUE,ch,cont,0,TO_CHAR);
			  return;
		      }
	      else{
		  send_to_char("You can't enter that!\r\n",ch);
		  return;
	      }
	  }
      sprintf(buf2, "There is no %s here.\r\n", buf);
      send_to_char(buf2, ch);
   } else if (IS_SET(world[ch->in_room].room_flags, INDOORS))
      send_to_char("You are already indoors.\r\n", ch);
   else {
      /* try to locate an entrance */
      for (door = 0; door < NUM_OF_DIRS; door++)
	 if (EXIT(ch, door))
	    if (EXIT(ch, door)->to_room != NOWHERE)
	       if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) && 
	           IS_SET(world[real_room(EXIT(ch, door)->to_room)].room_flags,
	           INDOORS)) {
		  do_move(ch, "", ++door, 0);
		  return;
	       }
      /* ok not a door how about an object */
      if ((cont = get_obj_in_list_vis(ch,buf,world[ch->in_room].contents)))
	  {
	      if (HASROOM(cont))
		  if (!IS_SET(cont->obj_flags.value[1],CONT_CLOSED))
		      {
			  if (GET_OBJ_SIZE(cont) - GET_SIZE(ch) > 1){
			      sprintf(buf1,
				      "The %s is too small you can't fit."
				      ,first_name(cont->name));
			      act(buf1,TRUE,ch,0,0,TO_CHAR);
			      return;
			  }
			  if ((GET_WEIGHT(ch)+IS_CARRYING_W(ch)
			       + GET_OBJ_WEIGHT(cont))
			       > cont->obj_flags.value[0])
			      {
				  sprintf(buf1,
					  "There is no room in the %s."
					  ,first_name(cont->name));
				  act(buf1,TRUE,ch,0,0,TO_CHAR);
				  return;
			      }
                          if(real_room(cont->obj_flags.value[3]) == -1){
		             send_to_char("You can't enter that!\r\n",ch);
                             return;
                          }
/*			  sprintf(buf1,"$n enters the %s."
				  ,first_name(cont->name));*/
			  act("$n enters $p.",TRUE,ch,cont,0,TO_ROOM);
/*			  sprintf(buf1,"You enter the %s."
				  ,first_name(cont->name));  */
			  act("you enter $p.",TRUE,ch,cont,0,TO_CHAR);
			  char_from_room(ch);
			  char_to_room(ch,real_room(cont->obj_flags.value[3])
			      ,FALSE);
			  do_look(ch,"",0,0);
/*			  sprintf(buf1,"$n enters the %s."
				  ,first_name(cont->name)); */
			  act("$n enters $p.",TRUE,ch,cont,0,TO_ROOM);
			  return;
		      }
		  else
		      {
			  sprintf(buf1,"The %s seems to be closed."
				  ,first_name(cont->name));
			  act(buf1,TRUE,ch,cont,0,TO_CHAR);
			  return;
		      }
	      else
		  {
		      send_to_char("You can't enter that!\r\n",ch);
		      return;
		  }
	  }
      send_to_char("You can't seem to find anything to enter.\r\n", ch);
   }
}


ACMD(do_leave)
{
   int	door, to_room, was_in;
   struct follow_type *k, *next_dude;
   struct obj_data *cont;
   
   ACMD(do_move);

   if (ch->specials.carried_by) {
    act("Ask $N to drop you first.",
	FALSE,ch,0,ch->specials.carried_by,TO_CHAR);
    return;
  }

   if (world[ch->in_room].obj)
       {
	   cont = world[ch->in_room].obj;
	   if (GET_ITEM_TYPE(cont) == ITEM_CONTAINER &&
	       !IS_SET(cont->obj_flags.value[1], CONT_ONEWAY) &&
	       !IS_SET(cont->obj_flags.value[1],CONT_CLOSED)){
	       if (GET_OBJ_SIZE(cont) - GET_SIZE(ch) > 1){
		   sprintf(buf1,
			   "The %s is too small you are stuck!"
			   ,first_name(cont->name));
		   act(buf1,TRUE,ch,0,0,TO_CHAR);
		   return;
	       }				      

	       if (cont->in_room != NOWHERE){
		   act("You leave $p.",TRUE,ch,cont,0,TO_CHAR);
		   act("$n leaves $p.",TRUE,ch,cont,0,TO_ROOM); 	   
		   to_room = cont->in_room;
		   was_in = ch->in_room;
		   char_from_room(ch);
		   char_to_room(ch,to_room, FALSE);
		   do_look(ch,"",0,0);
		   act("$n leaves $p.",TRUE,ch,cont,0,TO_ROOM);
		   if (ch->followers) {
		     for (k = ch->followers; k; k = next_dude) {
		       next_dude = k->next;
		       if ((k->follower->in_room == was_in) &&
			   (GET_POS(k->follower) >= POSITION_STANDING)
			   && !k->follower->specials.fighting){
			 if(IS_NPC(ch) && !number(0,3)
			    && GET_LEVEL(k->follower)
			    < LEVEL_BUILDER && !IS_NPC(k->follower)){
			   act("You lost track of $n.",TRUE,ch,0,k->follower,TO_VICT);
			   break;}
			 else {
			   act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
			   do_leave(k->follower, argument,0,0);
			 }
		       }
		     }
		   }
		   return;
	       }
	   }
	   else if (!IS_SET(cont->obj_flags.value[1], CONT_ONEWAY)){
	       sprintf(buf1,"The %s seems to be closed.",first_name(cont->name));
	       act(buf1,TRUE,ch,cont,0,TO_CHAR);
	       return;
	   }
	       
       }
   if (!IS_SET(world[ch->in_room].room_flags, INDOORS))
      send_to_char("You are outside.. where do you want to go?\r\n", ch);
   else {
      for (door = 0; door < NUM_OF_DIRS; door++)
	 if (EXIT(ch, door))
	    if (EXIT(ch, door)->to_room != NOWHERE)
	       if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) && 
	           !IS_SET(world[real_room(EXIT(ch, door)->to_room)].room_flags, INDOORS)) {
		  do_move(ch, "", ++door, 0);
		  return;
	       }
      send_to_char("I see no obvious exits to the outside.\r\n", ch);
   }
}


ACMD(do_stand)
{
  if (IS_AFFECTED(ch, AFF_PARALYSIS)) {
    send_to_char("Lie still you can't move a muscle.\r\n",ch);
    return;
  }
  if (ch->specials.mount && GET_POS(ch) != POSITION_STANDING) {
    GET_POS(ch) = POSITION_STANDING;
    send_to_char("Stand? Your mount might not like that.\r\n", ch);
    return;}    
    if (GET_POS(ch) == POSITION_STANDING){
	send_to_char("You are already standing.\r\n",ch);
	return;
    }
    if (IS_AFFECTED(ch, AFF_SLIPPY))
      if((number(0,2) && IS_NPC(ch)) || (number(0,1) && !IS_NPC(ch))){
	act("$n tries to stand up but slips and falls!", TRUE, ch,0,0,TO_ROOM);
	send_to_char("You try to stand but you slip and fall.\r\n", ch);
	return;
      }
    if (IS_AFFECTED(ch, AFF_BASH)){
      if((number(0,2) && IS_NPC(ch)) || (number(0,1) && !IS_NPC(ch))){
	act("$n tries to stand up but is too winded!", TRUE, ch,0,0,TO_ROOM);
	act("You try to stand but you are unable to catch your breath.", FALSE, ch,0,0,TO_CHAR);
	return;
      }
      else
	affect_from_char(ch, SKILL_BASH);
    }
    switch (GET_POS(ch)) {
    case POSITION_SITTING	:
      act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
      break;
    case POSITION_RESTING	:
      act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
      break;
    case POSITION_SLEEPING :
	act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
	break;
    default :
      act("You stop floating around, and put your feet on the ground.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and puts $s feet on the ground.",
	  TRUE, ch, 0, 0, TO_ROOM);
      break;
    }
    
}


ACMD(do_sit)
{
  if (ch->specials.mount) {
    send_to_char("Sit where? you are riding?\r\n", ch);
    return;}    
    if (ch->specials.fighting){
	act("Sit down while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
	return;}
   switch (GET_POS(ch)) {
   case POSITION_STANDING :
      act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
   case POSITION_SITTING	:
      send_to_char("You'r sitting already.\r\n", ch);
      break;
   case POSITION_RESTING	:
      act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
   case POSITION_SLEEPING :
      act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
      break;
    default :
      act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
   }
}




ACMD(do_rest)
{
  if (ch->specials.mount) {
    send_to_char("Rest while riding?  Well you can try?\r\n", ch);
    return;}    
    if (ch->specials.fighting){
	act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
	return;}
   switch (GET_POS(ch)) {
   case POSITION_STANDING :
      act("You lie down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n lies down and rests.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
      break;
   case POSITION_SITTING :
      act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
      break;
   case POSITION_RESTING :
      act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
      break;
   case POSITION_SLEEPING :
      act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
      break;
   default :
      act("You stop floating around, and stop to rest your tired bones.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
   }
}


ACMD(do_sleep)
{
  if (ch->specials.mount) {
    send_to_char("Sleep while riding?  Are you MAD?\r\n", ch);
    return;}    
    if (ch->specials.fighting){
	send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
	return;}
   switch (GET_POS(ch)) {
   case POSITION_STANDING :
   case POSITION_SITTING  :
   case POSITION_RESTING  :
      send_to_char("You go to sleep.\r\n", ch);
      act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
      break;
   case POSITION_SLEEPING :
      send_to_char("You are already sound asleep.\r\n", ch);
      break;
   default :
      act("You stop floating around, and lie down to sleep.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and lie down to sleep.",
          TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
      break;
   }
}


ACMD(do_wake)
{
   struct char_data *tmp_char;

   one_argument(argument, arg);
   if (*arg) {
      if (GET_POS(ch) == POSITION_SLEEPING) {
	 act("You can't wake people up if you are asleep yourself!",
	     FALSE, ch, 0, 0, TO_CHAR);
      } else {
	 tmp_char = get_char_room_vis(ch, arg);
	 if (tmp_char) {
	    if (tmp_char == ch) {
	       act("If you want to wake yourself up, just type 'wake'",
	           FALSE, ch, 0, 0, TO_CHAR);
	    } else {
	       if (GET_POS(tmp_char) == POSITION_SLEEPING) {
		  if (IS_AFFECTED(tmp_char, AFF_SLEEP) || GET_MOVE(tmp_char) < 0) {
		     act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
		  } else {
		     act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
		     GET_POS(tmp_char) = POSITION_SITTING;
		     act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
		  }
	       } else {
		  act("$N is already awake.", FALSE, ch, 0, tmp_char, TO_CHAR);
	       }
	    }
	 } else {
	    send_to_char("You do not see that person here.\r\n", ch);
	 }
      }
   } else {
      if (IS_AFFECTED(ch, AFF_SLEEP) || (GET_MOVE(ch) < 0 && number(0,3)) ) {
	 send_to_char("You can't wake up!\r\n", ch);
      } else {
	 if (GET_POS(ch) > POSITION_SLEEPING)
	    send_to_char("You are already awake...\r\n", ch);
	 else {
	    send_to_char("You wake, and sit up.\r\n", ch);
	    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POSITION_SITTING;
	 }
      }
   }
}


ACMD(do_follow)
{
   struct char_data *leader;
   int result;

   void	stop_follower(struct char_data *ch);
   int	add_follower(struct char_data *ch, struct char_data *leader);

   one_argument(argument, buf);

   if (*buf) {
      if (!str_cmp(buf, "self"))
	 leader = ch;
      else if (!(leader = get_char_room_vis(ch, buf))) {
	 send_to_char("I see no person by that name here!\r\n", ch);
	 return;
      }
   } else {
      send_to_char("Whom do you wish to follow?\r\n", ch);
      return;
   }

   if (ch->master == leader) {
      sprintf(buf, "You are already following %s.\r\n", HMHR(leader));
      send_to_char(buf, ch);
      return;
   }

   if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
      act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
   } else { /* Not Charmed follow person */
      if (leader == ch) {
	 if (!ch->master) {
	    send_to_char("You are already following yourself.\r\n", ch);
	    return;
	 }
	 stop_follower(ch);
      } else {
	 if (circle_follow(ch, leader)) {
	    act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
	    return;
	 }

	 if (ch->master)
	    stop_follower(ch);

	 REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);

	 if (( result = add_follower(ch, leader)) == 0)
	   {
	     act("$N tried to follow you but you already have too many followers."
		 ,FALSE,leader,0,ch,TO_CHAR);
	     act("$n already has too many followers."
		 ,FALSE,leader,0,ch,TO_VICT);	       
	   }
      }
   }
}
















