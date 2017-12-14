/* ************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************* ********************************************** */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "spells.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct color_list_type colors[];
extern struct char_data *character_list;
int find_track_length(short src, short target, int max);
bool search_info_keyword(struct char_data *target,int *index,char *keywd1,char *keywd2,int mode);
void perform_tell(struct char_data *ch, struct char_data *vict, char *arg);
ACMD(do_respond)
{
  struct char_data *tch = character_list;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char("You have no-one to respond to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your response?\r\n", ch);
  else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */

    while (tch != NULL && GET_IDNUM(tch) != GET_LAST_TELL(ch))
      tch = tch->next;

    if (tch == NULL)
      send_to_char("They are no longer playing.\r\n", ch);
    else
      perform_tell(ch, tch, argument);
  }
}


ACMD(do_say)
{
   int	i;
   char *len;

   for (i = 0; *(argument + i) == ' '; i++)
      ;

   if (ch->equipment[WEAR_MOUTH]){
       send_to_char("you can talk with something in your mouth?\r\n", ch);
       return;
   }   

   
   if (!*(argument + i))
      send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
   else {
       for (len = argument +i; *(len) != '\0'; len++)
	   ;
       len--;
       switch (*len){
       case '?':
	   sprintf(buf, "$n asks '%s'", argument + i);
	   sprintf(buf1, "You ask '%s'\r\n", argument + i);
	   break;
       case '!':
	   sprintf(buf, "$n exclaims '%s'", argument + i);
	   sprintf(buf1, "You exclaim '%s'\r\n", argument + i);
	   break;
       default:
	   sprintf(buf, "$n says '%s'", argument + i);
	   sprintf(buf1, "You say '%s'\r\n", argument + i);
	   break;
       }
       act(buf, FALSE, ch, 0, 0, TO_ROOM);
       if (!PRF_FLAGGED(ch, PRF_NOREPEAT)) {
	   send_to_char(buf1, ch);
       } else
	   send_to_char("Ok.\r\n", ch);
   }
}


ACMD(do_gsay)
{
   int	i;
   struct char_data *k;
   struct follow_type *f;

   for (i = 0; *(argument + i) == ' '; i++)
      ;

   if (!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are not the member of a group!\r\n", ch);
      return;
   }

   if (!*(argument + i))
      send_to_char("Yes, but WHAT do you want to group-say?\r\n", ch);
   else {
      if (ch->master)
	 k = ch->master;
      else
	 k = ch;
      sprintf(buf, "%s group-says '%s'\r\n", GET_NAME(ch), argument + i);
      if (IS_AFFECTED(k, AFF_GROUP) && (k != ch))
	 send_to_char(buf, k);
      for (f = k->followers; f; f = f->next)
	 if (IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower != ch))
	    send_to_char(buf, f->follower);
      if (!PRF_FLAGGED(ch, PRF_NOREPEAT)) {
	 sprintf(buf, "You group-say '%s'\r\n", argument + i);
	 send_to_char(buf, ch);
      } else
	 send_to_char("Ok.\r\n", ch);
   }
}



ACMD(do_tell)
{
   struct char_data *vict;

   half_chop(argument, buf, buf2);

   if (!*buf || !*buf2)
     send_to_char("Who do you wish to tell what??\r\n", ch);
   else if (!(vict = get_char_vis(ch, buf)))
     send_to_char("No-one by that name here..\r\n", ch);
   else if (ch == vict)
     send_to_char("You try to tell yourself something.\r\n", ch);
   else if (IS_MOB(vict))
     send_to_char("No-one by that name here..\r\n", ch);
   else 
     perform_tell(ch, vict, buf2);

}

void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  if (PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc) /* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now, try again later.",
	FALSE, ch, 0, vict, TO_CHAR);
  else if (PLR_FLAGGED(vict, PLR_BUILDING))
    act("$E's making part of the universe right now, try again later.",
	FALSE, ch, 0, vict, TO_CHAR);
  else if ((PRF_FLAGGED(vict, PRF_NOTELL)))
    act("$E refuses to hear you.", FALSE, ch, 0, vict, TO_CHAR);
  else if (!AWAKE(vict) && GET_MOVE(vict) > 0)
    act("$E can't hear you, $E's sleeping.", FALSE, ch, 0, vict, TO_CHAR);
  else if (!AWAKE(vict) && GET_MOVE(vict) < 0)
    act("$E can't hear you, $E's in an exhausted slumber.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    sprintf(buf, "%s tells you '%s'%s\r\n", GET_NAME(ch), arg,
	    CCNRM(vict, C_NRM));
    CAP(buf);
    send_to_char(CCRED(vict, C_NRM), vict);
    send_to_char(buf, vict);
    if (!PRF_FLAGGED(ch, PRF_NOREPEAT)) {
      sprintf(buf, "%sYou tell %s '%s'%s\r\n", CCRED(ch, C_CMP),
	      GET_NAME(vict), arg, CCNRM(ch, C_CMP));
      send_to_char(buf, ch);
    } else
      send_to_char("Ok.\r\n", ch);
    vict->specials2.last_tell_id = GET_IDNUM(ch);
  }
}


ACMD(do_whisper)
{
   struct char_data *vict;

   half_chop(argument, buf, buf2);

   if (!*buf || !*buf2)
      send_to_char("Who do you want to whisper to.. and what??\r\n", ch);
   else if (!(vict = get_char_room_vis(ch, buf)))
      send_to_char("No-one by that name here..\r\n", ch);
   else if (vict == ch) {
      act("$n whispers quietly to $mself.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You can't seem to get your mouth close enough to your ear...\r\n", ch);
   } else {
      if (!(vict = get_char_room_vis(ch, buf)))
          send_to_char("No-one by that name here..\r\n", ch);
      
      else {
          sprintf(buf, "$n whispers to you, '%s'", buf2);
          act(buf, FALSE, ch, 0, vict, TO_VICT);
          sprintf(buf, "You whisper '%s' in $N's ear.", buf2);
          act(buf, FALSE, ch, 0, vict, TO_CHAR);      
          act("$N whispers something to $n.", FALSE, vict, 0, ch, TO_NOTVICT);
      }
   }
}


ACMD(do_ask)
{
   struct char_data *vict;

   half_chop(argument, buf, buf2);

   if (ch->equipment[WEAR_MOUTH]){
       send_to_char("you can talk with something in your mouth?\r\n", ch);
       return;
   }   

   if (!*buf2){
       act("Ok ask sure, but ask about what?",FALSE,ch,0,0,TO_CHAR);
       return;}
   if (!(vict = get_char_room_vis(ch, buf)))
      send_to_char("No-one by that name here..\r\n", ch);
   else if (vict == ch) {
      act("$n quietly asks $mself a question.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You think about it for a while...\r\n", ch);
   }
   else {
     if (!(vict = get_char_room_vis(ch, buf)))
       send_to_char("How do you expect an answer when you aren't in the room?\r\n", ch);
     else { 
       sprintf(buf, "$n asks you '%s'", buf2);
       act(buf, FALSE, ch, 0, vict, TO_VICT);
       send_to_char("Ok.\r\n", ch);
       act("$n asks $N a question.", FALSE, ch, 0, vict, TO_NOTVICT);
     }
   }
}


#define MAX_NOTE_LENGTH 1000      /* arbitrary */

ACMD(do_write)
{
   struct obj_data *paper = 0, *pen = 0;
   char	*papername, *penname;

   papername = buf1;
   penname = buf2;

   argument_interpreter(argument, papername, penname);

   if (!ch->desc)
      return;

   if (!*papername)  /* nothing was delivered */ {
      send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
      return;
   }
   if (*penname) /* there were two arguments */ {
      if (!(paper = get_obj_in_list_vis(ch, papername, ch->inventory))) {
	 sprintf(buf, "You have no %s.\r\n", papername);
	 send_to_char(buf, ch);
	 return;
      }
      if (!(pen = get_obj_in_list_vis(ch, penname, ch->inventory))) {
	 sprintf(buf, "You have no %s.\r\n", papername);
	 send_to_char(buf, ch);
	 return;
      }
   } else /* there was one arg.let's see what we can find */	 {
      if (!(paper = get_obj_in_list_vis(ch, papername, ch->inventory))) {
	 sprintf(buf, "There is no %s in your inventory.\r\n", papername);
	 send_to_char(buf, ch);
	 return;
      }
      if (paper->obj_flags.type_flag == ITEM_PEN)  /* oops, a pen.. */ {
	 pen = paper;
	 paper = 0;
      } else if (paper->obj_flags.type_flag != ITEM_NOTE) {
	 send_to_char("That thing has nothing to do with writing.\r\n", ch);
	 return;
      }

      /* one object was found. Now for the other one. */
      if (!ch->equipment[HOLD]) {
	 sprintf(buf, "You can't write with a %s alone.\r\n", papername);
	 send_to_char(buf, ch);
	 return;
      }
      if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD])) {
	 send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
	 return;
      }

      if (pen)
	 paper = ch->equipment[HOLD];
      else
	 pen = ch->equipment[HOLD];
   }

   /* ok.. now let's see what kind of stuff we've found */
   if (pen->obj_flags.type_flag != ITEM_PEN) {
      act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
   } else if (paper->obj_flags.type_flag != ITEM_NOTE) {
      act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
   } else if (paper->action_description)
      send_to_char("There's something written on it already.\r\n", ch);
   else {
      /* we can write - hooray! */
      send_to_char("Ok.. go ahead and write.. end the note with a @.\r\n", ch);
      act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
      ch->desc->str = &paper->action_description;
      ch->desc->max_str = MAX_NOTE_LENGTH;
      act("$n finishes jottting down a note.", TRUE, ch, 0, 0, TO_ROOM);
   }
}



ACMD(do_page)
{
   struct descriptor_data *d;
   struct char_data *vict;

   if (IS_NPC(ch)) {
      send_to_char("Monsters can't page.. go away.\r\n", ch);
      return;
   }

   if (!*argument) {
      send_to_char("Whom do you wish to page?\r\n", ch);
      return;
   }
   half_chop(argument, buf, buf2);
   if (!str_cmp(buf, "all")) {
      if (GET_LEVEL(ch) > LEVEL_GOD) {
	 sprintf(buf, "\007\007*%s* %s\r\n", GET_NAME(ch), buf2);
	 for (d = descriptor_list; d; d = d->next)
	    if (!d->connected)
	       SEND_TO_Q(buf, d);
      } else
	 send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
   }

   if ((vict = get_char_vis(ch, buf))) {
      sprintf(buf, "\007\007*%s* %s\r\n", GET_NAME(ch), buf2);
      send_to_char(buf, vict);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	 send_to_char("Ok.\r\n", ch);
      else
	 send_to_char(buf, ch);
      return;
   } else
      send_to_char("There is no such person in the game!\r\n", ch);
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_com)
{
   extern int	level_can_shout;
   extern int	holler_move_cost;
   struct descriptor_data *i;
   char	buf3[MAX_STRING_LENGTH];
   char	buf4[MAX_STRING_LENGTH];
   char	buf5[MAX_STRING_LENGTH];
   char	buf6[MAX_STRING_LENGTH];   
   char	name[MAX_NAME_LENGTH+10];
   int col, dist;

   static int	channels[] = {
      0,
      PRF_DEAF,
      PRF_NOGOSS,
      PRF_NOAUCT,
      PRF_NOGRATZ,
      PRF_NOBRAG
   };

   static char	*com_msgs[][4] =  {
      { "You cannot yell!!\r\n",
      "yell",
      "",
      "\7" },
      { "You cannot shout!!\r\n",
      "shout",
      "Turn off your noshout flag first!\r\n",
      "\6" },
      { "You cannot gossip!!\r\n",
      "gossip", 
      "You aren't even on the channel!\r\n",
      "\3" },
      { "You cannot auction!!\r\n", 
      "auction",
      "You aren't even on the channel!\r\n",
      "\3" },
      { "You cannot congratulate!\r\n",
      "congratulate",
      "You aren't even on the channel!\r\n",
      "\5" },
      { "You cannot brag!\r\n",
      "brag",
      "You aren't even on the channel!\r\n",
      "\1" }
   };  


 /*  if (!ch->desc)
      return;
*/
   if (ch->equipment[WEAR_MOUTH]){
       send_to_char("You can talk with something in your mouth?\r\n", ch);
       return;
   }   

   
   if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
      send_to_char(com_msgs[subcmd][0], ch);
      return;
   }
   if (GET_LEVEL(ch) < level_can_shout) {
      sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
          level_can_shout, com_msgs[subcmd][1]);
      send_to_char(buf1, ch);
      return;
   }

   if (subcmd == SCMD_HOLLER) {
      if (GET_MOVE(ch) < holler_move_cost) {
	 send_to_char("You're too exhausted to yell.\r\n", ch);
	 return;
      } else
	 GET_MOVE(ch) -= holler_move_cost;
   }


   if (PRF_FLAGGED(ch, channels[subcmd])) {
      send_to_char(com_msgs[subcmd][2], ch);
      return;
   }

   for (; *argument == ' '; argument++)
      ;
   if (!(*argument)) {
      sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
          com_msgs[subcmd][1], com_msgs[subcmd][1]);
      send_to_char(buf1, ch);
      return;
   }

/*   strcpy(colon, com_msgs[subcmd][3]); */
   col =  *com_msgs[subcmd][3];
   if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char("Ok.\r\n", ch);
   else {
      if (COLOR_LEV(ch) >= C_CMP)
         sprintf(buf1, "%sYou %s,%s '%s'%s\r\n", colors[col].high, com_msgs[subcmd][1],colors[col].high,
	     argument, KNRM);
      else
	 sprintf(buf1, "You %s, '%s'\r\n", com_msgs[subcmd][1], argument);
      send_to_char(buf1, ch);
   }

   strcpy(name, GET_NAME(ch));
   *name = UPPER(*name);

   sprintf(buf1, "%s %ss, '%s'\r\n", name, com_msgs[subcmd][1], argument);
   sprintf(buf2, "Someone %ss, '%s'\r\n", com_msgs[subcmd][1], argument);
   sprintf(buf3, "%s%s %ss,%s '%s'%s\r\n", colors[col].high, name,com_msgs[subcmd][1],colors[col].low , argument,
       KNRM);
   sprintf(buf4, "%sSomeone %ss,%s '%s'%s\r\n",colors[col].high, com_msgs[subcmd][1],colors[col].low, argument, KNRM);


   for (i = descriptor_list; i; i = i->next) {
       if (!i->connected && i != ch->desc && 
	   !PRF_FLAGGED(i->character, channels[subcmd]) && 
	   !PLR_FLAGGED(i->character, PLR_WRITING) &&
	   !PLR_FLAGGED(i->character, PLR_BUILDING)) {
	   
	 if (subcmd == SCMD_SHOUT && 
	     (world[ch->in_room].zone != world[i->character->in_room].zone))
	   continue;
	 if (subcmd == SCMD_SHOUT) {
	   dist = find_track_length(ch->in_room, i->character->in_room, 10);
	   if (dist >= 10 || dist < 0)
	       continue;
	   else if (dist > 7 && dist < 10) {
	     sprintf(buf1, "You hear someone shouting in the distance.\r\n");
	     sprintf(buf2, "You hear someone shouting in the distance.\r\n");
	     sprintf(buf3, "%sYou hear someone shouting in the distance. %s\r\n", colors[col].high, KNRM);
	     sprintf(buf4, "%sYou hear someone shouting in the distance. %s\r\n", colors[col].high, KNRM);	     
	   }
	   else if ((dist >=  4) && (dist <= 7)) {
	     if (!strncmp(name,"A ",2) || !strncmp(name,"The ",4))
	       *name = LOWER(*name);
	     sprintf(buf1, "You hear %s %sing in the distance.\r\n", name, com_msgs[subcmd][1]);
	     sprintf(buf2, "You hear someone %sing in the distance.\r\n", com_msgs[subcmd][1]);
	     sprintf(buf3, "%sYou hear %s %sing in the distance.%s\r\n", colors[col].high, name,com_msgs[subcmd][1], KNRM);
	     sprintf(buf4, "%sYou hear someone %sing in the distance.%s\r\n", colors[col].high, com_msgs[subcmd][1], KNRM);
	   }
	 }
	 if (subcmd != SCMD_GOSSIP
	     && (GET_POS(i->character) < POSITION_RESTING))
	   continue;

	 if (CAN_SEE(i->character, ch)) {
	   if (COLOR_LEV(i->character) >= C_NRM)
	     send_to_char(buf3, i->character);
	   else
	     send_to_char(buf1, i->character);
	 }
	 else {
	   if (COLOR_LEV(i->character) >= C_NRM)
	     if (GET_LEVEL(ch) > LEVEL_BUILDER)
	       send_to_char(buf4, i->character);
	     else{
	       sprintf(buf4, "%sSomebody %ss,%s '%s'%s\r\n",colors[col].high, com_msgs[subcmd][1],colors[col].low, argument, KNRM);
	       send_to_char(buf4, i->character);
	     }
	   else
	     if (GET_LEVEL(ch) > LEVEL_BUILDER)
	       send_to_char(buf2, i->character);
	     else{
	       sprintf(buf2, "Somebody %ss, '%s'\r\n", com_msgs[subcmd][1], argument);
	       send_to_char(buf2, i->character);
	     }
	 }
       }
   }
}


ACMD(do_qsay)
{
   struct descriptor_data *i;

   if (!PRF_FLAGGED(ch, PRF_QUEST)) {
      send_to_char("You aren't even part of the quest!\r\n", ch);
      return;
   }

   for (; *argument == ' '; argument++)
      ;
   if (!(*argument))
      send_to_char("Quest-say?  Yes!  Fine!  Quest-say we must, but WHAT??\r\n", ch);
   else {
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	 send_to_char("Ok.\r\n", ch);
      else {
	 sprintf(buf1, "You quest-say, '%s'\r\n", argument);
	 send_to_char(buf1, ch);
      }

      sprintf(buf1, "$n quest-says, '%s'", argument);
      for (i = descriptor_list; i; i = i->next)
	 if (!i->connected && i != ch->desc && 
	     PRF_FLAGGED(i->character, PRF_QUEST) && 
	     !PLR_FLAGGED(i->character, PLR_WRITING))
	    act(buf1, 0, ch, 0, i->character, TO_VICT);
   }
}


