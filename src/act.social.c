/* ************************************************************************
*   File: act.social.c                                  Part of CircleMUD *
*  Usage: Functions to handle socials                                     *
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
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
 extern int hyper_soc[];
 extern int happy_soc[];
 extern int cheery_soc[];
 extern int neutral_soc[];
 extern int sad_soc[];
 extern int grouchy_soc[];
 extern int homicidal_soc[];
/* extern functions */
    void	gain_exp(struct char_data *ch, int gain,int mode);
void	gain_exp_regardless(struct char_data *ch, int gain, int mode);
void	parse_string(char *input, char *output, struct char_data *ch1,
struct char_data *ch2, struct char_data *to);
int	action(int cmd);
char	*fread_action(FILE *fl, int nr);
void   drop_excess_gold(struct char_data *ch, int amount);
SPECIAL(moods);
/* local globals */
static int	list_top = -1;
int    get_mood_type(int cmd, int mood);
struct social_messg {
   int	act_nr;
   int	hide;
   int	min_victim_position; /* Position of victim */

   /* No argument was supplied */
   char	*char_no_arg;
   char	*others_no_arg;

   /* An argument was there, and a victim was found */
   char	*char_found;		/* if NULL, read no further, ignore args */
   char	*others_found;
   char	*vict_found;

   /* An argument was there, but no victim was found */
   char	*not_found;

   /* The victim turned out to be the character */
   char	*char_auto;
   char	*others_auto;
} *soc_mess_list = 0;


char	*fread_action(FILE *fl, int nr)
{
   char	buf[MAX_STRING_LENGTH], *rslt;

   fgets(buf, MAX_STRING_LENGTH, fl);
   if (feof(fl)) {
      sprintf(buf, "SYSERR: fread_action - unexpected EOF near action #%d", nr);
      logg(buf);
      exit(0);
   }

   if (*buf == '#')
      return(0);
   else {
      *(buf + strlen(buf) - 1) = '\0';
      CREATE(rslt, char, strlen(buf) + 1);
      strcpy(rslt, buf);
      return(rslt);
   }
}


void	boot_social_messages(void)
{
   FILE * fl;
   int	nr, tmp, hide, min_pos;

   if (!(fl = fopen(SOCMESS_FILE, "r"))) {
      perror("boot_social_messages");
      exit(0);
   }

   for (; ; ) {
      fscanf(fl, " %d ", &tmp);
      if ((nr = tmp) < 0)
	 break;
      fscanf(fl, " %d ", &hide);
      fscanf(fl, " %d \n", &min_pos);

      /* alloc a new cell */
      if (!soc_mess_list) {
	 CREATE(soc_mess_list, struct social_messg, 1);
	 list_top = 0;
      } else if (!(soc_mess_list = (struct social_messg *)
          realloc(soc_mess_list, sizeof (struct social_messg ) * 
          (++list_top + 1)))) {
	 perror("boot_social_messages. realloc");
	 exit(1);
      }

      if (!tmp) {
	 if (!list_top)
	    fprintf(stderr, "Format error near beginning of socials file.\n");
	 else
	    fprintf(stderr, "Format error in social file near social #%d.\n",
	        soc_mess_list[list_top-1].act_nr);
	 exit(1);
      }

      if (list_top && soc_mess_list[list_top-1].act_nr > tmp) {
	 fprintf(stderr, "Format error or out-of-order social in socials file near #%d.\n",
	     soc_mess_list[list_top-1].act_nr);
	 exit(1);
      }

      /* read the stuff */
      soc_mess_list[list_top].act_nr = tmp;
      soc_mess_list[list_top].hide = hide;
      soc_mess_list[list_top].min_victim_position = min_pos;

      soc_mess_list[list_top].char_no_arg = fread_action(fl, nr);
      soc_mess_list[list_top].others_no_arg = fread_action(fl, nr);
      soc_mess_list[list_top].char_found = fread_action(fl, nr);

      /* if no char_found, the rest is to be ignored */
      if (!soc_mess_list[list_top].char_found)
	 continue;

      soc_mess_list[list_top].others_found = fread_action(fl, nr);
      soc_mess_list[list_top].vict_found = fread_action(fl, nr);
      soc_mess_list[list_top].not_found = fread_action(fl, nr);
      soc_mess_list[list_top].char_auto = fread_action(fl, nr);
      soc_mess_list[list_top].others_auto = fread_action(fl, nr);
   }
   fclose(fl);
}




int	find_action(int cmd)
{
   int	bot, top, mid;

   bot = 0;
   top = list_top;

   if (top < 0)
      return(-1);

   for (; ; ) {
      mid = (bot + top) / 2;

      if (soc_mess_list[mid].act_nr == cmd)
	 return(mid);
      if (bot >= top)
	 return(-1);

      if (soc_mess_list[mid].act_nr > cmd)
	 top = --mid;
      else
	 bot = ++mid;
   }
}





ACMD(do_action)
{
    int	act_nr, act_mood, old_mood;
    struct social_messg *action;
    struct char_data *vict;
    if (subcmd > 0)
      cmd = subcmd;
   if ((act_nr = find_action(cmd)) < 0) {
      send_to_char("That action is not supported.\r\n", ch);
      return;
   }
   if ((cmd == 244)
       && GET_LEVEL(ch) >= LEVEL_BUILDER && !PLR_FLAGGED(ch,PLR_AFK))
       SET_BIT(PLR_FLAGS(ch), PLR_AFK);
   else if ((cmd == 244)
	    && GET_LEVEL(ch) >= LEVEL_BUILDER && PLR_FLAGGED(ch,PLR_AFK))
       {
	   act("$n has returned.",FALSE,ch,0,0,TO_ROOM);
	   send_to_char("You have returned.\r\n",ch);
	   REMOVE_BIT(PLR_FLAGS(ch), PLR_AFK);
	   return;
       }
   action = &soc_mess_list[act_nr];

   if (action->char_found)
      one_argument(argument, buf);
   else
      *buf = '\0';

   if (!*buf) {
      send_to_char(action->char_no_arg, ch);
      send_to_char("\r\n", ch);
      act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);
      return;
   }
   if (!(vict = get_char_room_vis(ch, buf))) {
      send_to_char(action->not_found, ch);
      send_to_char("\r\n", ch);
   } else if (vict == ch) {
      send_to_char(action->char_auto, ch);
      send_to_char("\r\n", ch);
      act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
   } else {
      if (GET_POS(vict) < action->min_victim_position) {
	 act("$N is not in a proper position for that.", FALSE, ch, 0, vict, TO_CHAR);
      } else {
	 act(action->char_found, 0, ch, 0, vict, TO_CHAR);
	 act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT); 
	 act(action->vict_found, action->hide, ch, 0, vict, TO_VICT);
      }
      if (IS_NPC(vict)){ /* change mood a bit */
	  act_mood = get_mood_type(cmd, GET_MOOD(vict));
	  old_mood = GET_MOOD(vict);
	  if (IS_MOB(ch))
	      GET_MOOD(vict) += GET_MOOD(ch)/200;
	  GET_MOOD(vict) += act_mood;
	  GET_MOOD(vict) = MIN(1000, GET_MOOD(vict));
	  GET_MOOD(vict) = MAX(-1000, GET_MOOD(vict)); 

	  if (GET_MOOD(vict) < old_mood){
	      if (old_mood <= -900){
		  act("$n seems really upset.",TRUE,vict,0,0,TO_ROOM);
		  if (GET_MOOD(vict) < -950 && !number(0,6)){
		      act("$n gets really enraged at $N.",TRUE,vict,0,ch,TO_NOTVICT);
		      if (!IS_SET(ch->specials2.act, MOB_SENTINEL) &&
			  !IS_SET(vict->specials2.act, MOB_SENTINEL))
			  hit(vict,ch,TYPE_UNDEFINED);
		      return;
		  }
	      }
	  }
	  if ((MOB_FLAGGED(ch, MOB_MOODS) || MOB_FLAGGED(ch, MOB_CITIZEN)) &&
	      (!IS_NPC(ch) || !number(0,2)) && !ch->specials.fighting)
	    moods(vict,ch,SPEC_MOBACT, "");

      }
   }
}

int get_mood_type(int cmd, int mood)
{
    int i;

    i = 0;
    while(hyper_soc[i] > 0){
	if (cmd == hyper_soc[i])
	    return 40 + mood/100;
	i++;
    }
    i = 0;
    while(happy_soc[i] > 0){
	if (cmd == happy_soc[i])
	    return 30 + mood/100;
	i++;
    }
    i = 0;
    while(cheery_soc[i] > 0){
	if (cmd == cheery_soc[i])
	    return 20 + mood/100;
	i++;
    }
    i = 0;
    while(neutral_soc[i] > 0){
	if (cmd == neutral_soc[i])
	    return number(-10,10) + mood/500;
	i++;
    }
    i = 0;
    while(sad_soc[i] > 0){
	if (cmd == sad_soc[i])
	    return -20 + mood/100;
	i++;
    }
    i = 0;
    while(grouchy_soc[i] > 0){
	if (cmd == grouchy_soc[i])
	    return -28 + mood/100;
	i++;
    }	            
    i = 0;
    while(homicidal_soc[i] > 0){
	if (cmd == homicidal_soc[i])
	    return -33 + mood/100;
	i++;
    }	            
    return(0);
}

ACMD(do_insult)
{
   struct char_data *victim;

   one_argument(argument, arg);

   if (*arg) {
      if (!(victim = get_char_room_vis(ch, arg))) {
	 send_to_char("Can't hear you!\r\n", ch);
      } else {
	 if (victim != ch) {
	    sprintf(buf, "You insult %s.\r\n", GET_NAME(victim) );
	    send_to_char(buf, ch);
	    if (IS_MOB(victim)){
		if (IS_NPC(ch))
		    GET_MOOD(victim) -= number(1,20);
		else
		    GET_MOOD(victim) -= number(1,4);
	    }
	    switch (random() % 3) {
	    case 0 :
	        {
		  if (GET_SEX(ch) == SEX_MALE) {
		     if (GET_SEX(victim) == SEX_MALE)
			act(
			    "$n accuses you of fighting like a woman!", FALSE,
			    ch, 0, victim, TO_VICT);
		     else
			act("$n says that women can't fight.",
			    FALSE, ch, 0, victim, TO_VICT);
		  } else { /* Ch == Woman */
		     if (GET_SEX(victim) == SEX_MALE)
			act("$n accuses you of having the smallest.... (brain?)",
			     			     								FALSE, ch, 0, victim, TO_VICT );
		     else
			act("$n tells you that you'd lose a beauty contest against a troll.",
			     			     								FALSE, ch, 0, victim, TO_VICT );
		  }
	       }
	       break;
	    case 1 :
	        {
		  act("$n calls your mother a bitch!",
		      FALSE, ch, 0, victim, TO_VICT );
	       }
	       break;
	    default :
	        {
		  act("$n tells you to get lost!", FALSE, ch, 0, victim, TO_VICT);
	       }
	       break;
	    } /* end switch */

	    act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
	 } else { /* ch == victim */
	    send_to_char("You feel insulted.\r\n", ch);
	 }
      }
   } else
      send_to_char("I'm sure you don't want to insult *everybody*...\r\n", ch);
}








