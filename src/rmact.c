/* ************************************************************************
*   File: rmact.c                                     Part of Archipelago *
*  Usage: Functions for generating interesting room bhaviour              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*  AJN April 94                                                             *
*  Archipelago is based on CircleMud                                      *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*  Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"

/* external structs */
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
ACMD(do_move);
ACMD(do_look);
ACMD(do_get);
ACMD(do_wear);
ACMD(do_flee);
ACMD(do_stand);
void	update_pos( struct char_data *victim );
void	die(struct char_data *ch);

void	rmdamage(struct char_data *victim, int dam);
bool    fall_down(struct char_data *ch);
void	room_activity(void)
{
   register struct char_data *ch;
   int	cmmd = 0, heat_dam, drown_dam, cold_dam;

   char *arg;

   for (ch = character_list; ch; ch = ch->next)
     if (!IS_MOB(ch)){
       if(world[ch->in_room].funct)
	 (*world[ch->in_room].funct)(ch, &world[ch->in_room], &cmmd, arg);
       if (IS_CLIMB(ch, 5))
	 fall_down(ch);
       if ((world[ch->in_room].sector_type == SECT_UNDER_WATER)
	   && ((ubyte) GET_LEVEL(ch) < LEVEL_BUILDER))
	 if (!IS_AFFECTED(ch, AFF_WATER_BREATH)){
	   act("$n panics and clutches $s throat.",FALSE,ch,0,0,TO_ROOM);
	   act("$n is drowning!!!",FALSE,ch,0,0,TO_ROOM);
	   act("Glub, Splutter, Cough! You are drowning!!",FALSE,ch,0,0,TO_CHAR);
	   drown_dam = (GET_HIT(ch)/4 + number(5,10));
	   rmdamage(ch,drown_dam);
	 }
       if (world[ch->in_room].sector_type == SECT_FIRE && GET_LEVEL(ch) < LEVEL_BUILDER){
	 if (IS_AFFECTED(ch, AFF_RESIST_HEAT)){
	   act("It's hot here! Sweat drips off you",FALSE,ch,0,0,TO_CHAR);
	   GET_MOVE(ch) -= number(2,10);}
	 else {
	   act("You are being scorched!",FALSE,ch,0,0,TO_CHAR);
	   heat_dam = number(3,18);
	   rmdamage(ch,heat_dam);
	   GET_MOVE(ch) -= number(4,20);}
       }
       if (world[ch->in_room].sector_type == SECT_ICE && GET_LEVEL(ch) < LEVEL_BUILDER){
	 if (IS_AFFECTED(ch, AFF_RESIST_COLD)){
	   act("Brrr its chilly here.",FALSE,ch,0,0,TO_CHAR);
	   GET_MOVE(ch) -= number(2,10);}
	 else {
	   act("You are freezing!",FALSE,ch,0,0,TO_CHAR);
	   heat_dam = number(3,18);
	   rmdamage(ch,heat_dam);
	   GET_MOVE(ch) -= number(4,20);}
       }	   
       if ((world[ch->in_room].sector_type == SECT_DESERT_HOT)
	   && (GET_LEVEL(ch) < LEVEL_BUILDER)){
	 if (IS_AFFECTED(ch, AFF_RESIST_HEAT)){
	   act("It's pretty warm here.",FALSE,ch,0,0,TO_CHAR);
	   GET_MOVE(ch) -= number(1,5);}
	 else {
	   act("Boy is it hot",FALSE,ch,0,0,TO_CHAR);
	   GET_MOVE(ch) -= number(2,10);}
       }	   
       if (world[ch->in_room].sector_type == SECT_DESERT_COLD && GET_LEVEL(ch) < LEVEL_BUILDER){
	 if (IS_AFFECTED(ch, AFF_RESIST_COLD)){
	   act("It's cool here.",FALSE,ch,0,0,TO_CHAR);
	   GET_MOVE(ch) -= number(1,5);}
	 else {
	   act("It's c-c-c-cold.",FALSE,ch,0,0,TO_CHAR);
	   GET_MOVE(ch) -= number(2,10);}
       }	   
       if (IS_SET(world[ch->in_room].room_flags, HOT)
	   && GET_LEVEL(ch) <LEVEL_BUILDER)
	 if (IS_AFFECTED(ch, AFF_RESIST_HEAT)){
	   act("Phew it's hot here!",FALSE,ch,0,0,TO_CHAR);
	   heat_dam = number(5,15);
	   rmdamage(ch,heat_dam);
	   GET_MOVE(ch) -= number(5,50);}
	 else {
	   act("AAARRGGHHH! You are burning!",FALSE,ch,0,0,TO_CHAR);
	   heat_dam = number(15,90);
	   rmdamage(ch,heat_dam);
	   GET_MOVE(ch) -= number(50,100);}	   
	  if (IS_SET(world[ch->in_room].room_flags, COLD)
	      && GET_LEVEL(ch) <LEVEL_BUILDER)
	    if (IS_AFFECTED(ch, AFF_RESIST_COLD)){
	      act("Brrr it's cold here!",FALSE,ch,0,0,TO_CHAR);
	      cold_dam = number(5,15);
	      rmdamage(ch,cold_dam);
	      GET_MOVE(ch) -= number(5,50);}
	    else {
	      act("Urrrggg you are freezing...",FALSE,ch,0,0,TO_CHAR);
	      cold_dam =  number(15,90);
	      rmdamage(ch,cold_dam);
	      GET_MOVE(ch) -= number(50,100);}
	  if (IS_SET(world[ch->in_room].room_flags, WARM)
	      && GET_LEVEL(ch) <LEVEL_BUILDER)
	    if (IS_AFFECTED(ch, AFF_RESIST_HEAT))
	      act("Ahh it's a bit warm here here",FALSE,ch,0,0,TO_CHAR);
	    else {
	      act("It's warm here!",FALSE,ch,0,0,TO_CHAR);
	      heat_dam = number(5,15);
	      rmdamage(ch,heat_dam);
	      GET_MOVE(ch) -= number(1,10);}
	  if (IS_SET(world[ch->in_room].room_flags, COOL)
	      && GET_LEVEL(ch) <LEVEL_BUILDER)
	    if (IS_AFFECTED(ch, AFF_RESIST_COLD))
	      act("Hmmm it's cool here!",FALSE,ch,0,0,TO_CHAR);
	    else {
	      cold_dam = number(5,15);
	      rmdamage(ch,cold_dam);
	      act("It's chilly here...",FALSE,ch,0,0,TO_CHAR);
	      GET_MOVE(ch) -= number(1,10);}
	  if (IS_SET(world[ch->in_room].room_flags, PAIN)
	      && GET_LEVEL(ch) < LEVEL_BUILDER){
	    cold_dam = number(20,60);
	    rmdamage(ch,cold_dam);
	    act("Pain wracks every fibre of your body!",FALSE,ch,0,0,TO_CHAR);
	    act("Pain wracks every fibre of your body!",FALSE,ch,0,0,TO_CHAR);
	    GET_MOVE(ch) -= number(1,30);
	  }
     }   
   
}
void	rmdamage(struct char_data *victim, int dam)
{ 
   
   if (GET_POS(victim) <= POSITION_DEAD) {
       logg("SYSERR: Attempt to damage a corpse.");
      return;   /* -je, 7/7/92 */
   }

   assert(GET_POS(victim) > POSITION_DEAD);

   /* You can't damage an immortal! */
   if ((GET_LEVEL(victim) >= LEVEL_BUILDER) && !IS_NPC(victim))
      dam = 0;
   dam = MAX(dam, 0);
   GET_HIT(victim) -= dam;
   update_pos(victim);

   /* Use send_to_char -- act() doesn't send message if you are DEAD. */
   switch (GET_POS(victim)) {
   case POSITION_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
      break;
   case POSITION_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", victim);
      break;
   case POSITION_STUNNED:
      act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
      break;
   case POSITION_DEAD:
      act("$n is dead!  R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are dead!  Sorry...\r\n", victim);
      break;

   default:  /* >= POSITION SLEEPING */
      if (dam > (GET_MAX_HIT(victim) / 5))
	 act("That Really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

      if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 5)) {
	 act("You wish that your wounds would stop BLEEDING so much!", FALSE, victim, 0, 0, TO_CHAR);
	 if (IS_NPC(victim)) {
	   if (IS_SET(victim->specials2.act, MOB_WIMPY)){
	     if (GET_POS(victim) <= POSITION_STANDING)
	       do_stand(victim, "", 0, 0);
	     if (GET_POS(victim) == POSITION_STANDING)	 
	       do_flee(victim, "", 0, 0);
	   }
	 }
      }

      if (!IS_NPC(victim) && WIMP_LEVEL(victim) && 
          GET_HIT(victim) < WIMP_LEVEL(victim)) {
	 send_to_char("You panic, and attempt to flee!\r\n", victim);
	 if (GET_POS(victim) <= POSITION_STANDING)
	   do_stand(victim, "", 0, 0);
	 if (GET_POS(victim) == POSITION_STANDING)	 
	   do_flee(victim, "", 0, 0);
      }

      break;
   }

   if (GET_POS(victim) <= POSITION_DEAD){
       if (!IS_NPC(victim)) {
	   sprintf(buf2, "%s killed by room damage at %s", GET_NAME(victim),  world[victim->in_room].name);
	   mudlog(buf2, BRF, LEVEL_BUILDER, TRUE);
       }
   
       die(victim);
   }

}
