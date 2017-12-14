/* ************************************************************************
*   File: act.wizard2.c                                 Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "screen.h"
#include "shop.h"

/*   external vars  */
extern struct   time_info_data time_info;	/* In db.c */
extern FILE *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[36];
extern struct wis_app_type wis_app[36];
extern struct zone_data *zone_table;
extern struct player_index_element *player_table;
extern struct obj_limit_list_type *obj_limit_table;
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern struct shop_data *shop_index;
extern struct event_type *events[];
extern int      number_of_shops;
extern int	top_of_zone_table;
extern int	restrict;
extern int	top_of_world;
extern int	top_of_mobt;
extern int	top_of_objt;
extern int	top_of_p_table;
extern int      top_of_ol_table;
/* for objects */
extern char	*item_types[];
extern char	*wear_bits[];
extern char	*extra_bits[];
extern char	*drinks[];
/* for rooms */
extern char	*dirs[];
extern char	*room_bits[];
extern char	*exit_bits[];
extern char	*sector_types[];

/* for chars */
extern char	*spells[];
extern char	*equipment_types[];
extern char	*affected_bits[];
extern char	*apply_types[];
extern char	*pc_race_types[];
extern char	*npc_class_types[];
extern char	*action_bits[];
extern char	*player_bits[];
extern char	*preference_bits[];
extern char	*position_types[];
extern char	*connected_types[];
extern char     *where[];

/* external functs */
void	reset_zone(int zone);
void    parse_text(struct char_data *c,struct char_data *vict, int mode, char *text);
void    do_stat_room(struct char_data *ch);
void    do_stat_character(struct char_data *ch, struct char_data *vict);
void    do_stat_object(struct char_data *ch, struct obj_data *vict);
void    do_stat_shop(struct char_data *ch,   struct shop_data *shop);
int     report_money_weight(int amount);
int     save_zone(int zone);
int	save_mobiles(int zone);
int	save_objects(int zone);
int	save_rooms(int zone);
bool    is_maxed(int num, int mode);
void    drop_excess_gold(struct char_data *ch, int amount);
int	load_char_bynum(int player_i, struct char_file_u *char_element);

ACMD(do_look);
int is_goditem(struct obj_data *j);


ACMD(do_stat)
{
   struct char_data *victim = 0;
   struct obj_data *object = 0;
   struct char_file_u tmp_store;
   int num = -1;
   
   if (IS_NPC(ch))
      return;

   half_chop(argument, buf1, buf2);

   if (!*buf1) {
      send_to_char("Stats on who or what?\r\n", ch);
      return;
   } else if (is_abbrev(buf1, "room")) {
      do_stat_room(ch);
   }
   else if (is_abbrev(buf1, "mob")) {
      if (!*buf2) {
	 send_to_char("Stats on which mobile?\r\n", ch);
      } else {
	 if ((victim = get_char_vis(ch, buf2)))
	    do_stat_character(ch, victim);
	 else
	    send_to_char("No such mobile around.\r\n", ch);
      }
   }
   else if (is_abbrev(buf1, "player")) {
      if (!*buf2) {
	 send_to_char("Stats on which player?\r\n", ch);
      } else {
	 if ((victim = get_player_vis(ch, buf2)))
	    do_stat_character(ch, victim);
	 else
	    send_to_char("No such player around.\r\n", ch);
      }
   } else if (is_abbrev(buf1, "file")) {
      if (!*buf2) {
	 send_to_char("Stats on which player?\r\n", ch);
      } else {
	 CREATE(victim, struct char_data, 1);
	 clear_char(victim);
	 if (is_number(buf)){
	     num = atoi(buf2);
	     load_char_bynum(num, &tmp_store);
	 }
	 else
	     num = load_char(buf2, &tmp_store);
	 if(num > -1) {
	    store_to_char(&tmp_store, victim);
	    if (GET_LEVEL(victim) > GET_LEVEL(ch))
	       send_to_char("Sorry, you can't do that.\r\n", ch);
	    else
	       do_stat_character(ch, victim);
	    if (GET_LEVEL(ch) >= LEVEL_IMPL) {
	      sprintf(buf, "Encrypted Password: %s\r\n", tmp_store.pwd);
	      send_to_char(buf,ch);
	    }
	    free_char(victim);
	 } else {
	    send_to_char("There is no such player.\r\n", ch);
	    free(victim);
	 }
      }
   } else if (is_abbrev(buf1, "object")) {
      if (!*buf2) {
	 send_to_char("Stats on which object?\r\n", ch);
      } else {
	 if ((object = get_obj_vis(ch, buf2)))
	    do_stat_object(ch, object);
	 else
	    send_to_char("No such object around.\r\n", ch);
      }
   } else {
      if ((victim = get_char_vis(ch, buf1)))
	 do_stat_character(ch, victim);
      else if ((object = get_obj_vis(ch, buf1)))
	 do_stat_object(ch, object);
      else
	 send_to_char("Nothing around by that name.\r\n", ch);
   }
}


ACMD(do_shutdown)
{
  extern int	circle_shutdown, circle_reboot;
  int time;
  int i;
  struct event_type *tmp;
  
   if (IS_NPC(ch))
      return;

   if (subcmd != SCMD_SHUTDOWN) {
      send_to_char("If you want to shut something down, say so!\r\n", ch);
      return;
   }

   half_chop(argument, arg, buf1);
   if ((time = atoi(buf1)) <= 0)
     time = 1;

   if (!*arg) {
     sprintf(buf, "\007\007(GC) Shutdown by %s in %d %s.", GET_NAME(ch), time, ((time == 1) ? "minute" : "minutes"));
     send_to_all(buf);
     logg(buf);
     add_event(250, EVENT_REBOOT, time, 0,0,0,"",ch,0);
   }
   else if (!str_cmp(arg, "reboot")) {
     sprintf(buf, "\007\007(GC) Reboot by %s in %d %s.", GET_NAME(ch), time,
	     ((time == 1) ? "minute" : "minutes"));
     send_to_all(buf);
     logg(buf);
     add_event(250, EVENT_REBOOT, time, 1,0,0,"",ch,0);
     system("touch ../.fastboot");
   }
   else if (!str_cmp(arg, "die")) {
     sprintf(buf, "\007\007(GC) Shutdown by %s in %d %s.", GET_NAME(ch), time,
	     ((time == 1) ? "minute" : "minutes"));
     send_to_all(buf);
     logg(buf);
     add_event(250, EVENT_REBOOT, time, 0,0,0,"",ch,0);
     system("touch ../.killscript");
   }
   else if (!str_cmp(arg, "pause")) {
      sprintf(buf, "\007\007(GC) Shutdown by %s in %d %s.", GET_NAME(ch), time,((time == 1) ? "minute" : "minutes"));
      send_to_all(buf);
      logg(buf);
      system("touch ../pause");
      add_event(250, EVENT_REBOOT, time, 0,0,0,"",ch,0);      
   }
   else if (!str_cmp(arg, "cancel")) {
      sprintf(buf, "\007\007(GC) Shutdown cancelled by %s.", GET_NAME(ch));
      send_to_all(buf);
      logg(buf);
      
      for (i=0;i< 300; i++)
	if ((tmp = events[i]) != NULL)
	  while(tmp){
	    if(tmp->event == EVENT_REBOOT)
	      tmp->event  = EVENT_IGNORE;
	    tmp = tmp->next;
	  }
   }
   else
      send_to_char("Unknown shutdown option.\r\n", ch);
}




ACMD(do_snoop)
{
   struct char_data *victim;

   if (!ch->desc)
      return;

   one_argument(argument, arg);

   if (!*arg) {
      send_to_char("Snoop who?\r\n", ch);
      return;
   }

   if (!(victim = get_char_vis(ch, arg))) {
      send_to_char("No such person around.\r\n", ch);
      return;
   }

   if (!victim->desc) {
      send_to_char("There's no link.. nothing to snoop.\r\n", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("Ok, you just snoop yourself.\r\n", ch);
      if (ch->desc->snoop.snooping) {
	  if (ch->desc->snoop.snooping->desc)
	      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
	  ch->desc->snoop.snooping = 0;
      }
      return;
   }

   if (victim->desc->snoop.snoop_by) {
      send_to_char("Busy already. \r\n", ch);
      return;
   }

   if (GET_LEVEL(victim) >= GET_LEVEL(ch)) {
      send_to_char("You failed.\r\n", ch);
      return;
   }

   send_to_char("Ok. \r\n", ch);

   if (ch->desc->snoop.snooping)
      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

   ch->desc->snoop.snooping = victim;
   victim->desc->snoop.snoop_by = ch;

   if (GET_LEVEL(ch) < LEVEL_IMPL){
      sprintf(buf, "(GC) %s started snooping %s",GET_NAME(ch),GET_NAME(victim));
      mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
   }
   return;
}



ACMD(do_switch)
{
   struct char_data *victim;

/*   if (IS_NPC(ch))
      return; */

   one_argument(argument, arg);

   if (!*arg) {
      send_to_char("Switch with who?\r\n", ch);
   } else {
      if (!(victim = get_char(arg)))
	 send_to_char("They aren't here.\r\n", ch);
      else {
	 if (ch == victim) {
	    send_to_char("He he he... We are jolly funny today, eh?\r\n", ch);
	    return;
	 }
	 if (GET_LEVEL(ch) <= GET_LEVEL(victim)){
	     send_to_char("He he he... Nice try!\r\n",ch);
	     return;}
	 if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) {
	    send_to_char("Mixing snoop & switch is bad for your health.\r\n", ch);
	    return;
	 }

	 if (victim->desc) {
	    send_to_char("You can't do that, the body is already in use!\r\n", ch);
	 } else {
	    send_to_char("Ok.\r\n", ch);

	    ch->desc->character = victim;
	    ch->desc->original = ch;

	    victim->desc = ch->desc;
	    ch->desc = 0;
            if(GET_LEVEL(ch) < LEVEL_IMPL && GET_LEVEL(victim) < LEVEL_BUILDER){
               sprintf(buf, "(GC) %s switched into %s", GET_NAME(ch),GET_NAME(victim));
               mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
            }
	 }
      }
   }
}



ACMD(do_return)
{
   if (!ch->desc)
      return;

   if (!ch->desc->original) {
      send_to_char("Yeah, right...\r\n", ch);
      return;
   } else {
      send_to_char("You return to your original body.\r\n", ch);

      ch->desc->character = ch->desc->original;
      ch->desc->original = 0;

      ch->desc->character->desc = ch->desc;
      ch->desc = 0;
   }
}



ACMD(do_load)
{
   struct char_data *mob;
   struct obj_data *obj;
   int	number, r_num;

   if (IS_NPC(ch))
      return;

   argument_interpreter(argument, buf, buf2);

   if (!*buf || !*buf2 || !isdigit(*buf2)) {
      send_to_char("Usage: load { obj | mob } <number>\r\n", ch);
      return;
   }

   if ((number = atoi(buf2)) < 0) {
      send_to_char("A NEGATIVE number??\r\n", ch);
      return;
   }
   if (is_abbrev(buf, "mob")) {
      if ((r_num = real_mobile(number)) < 0) {
	 send_to_char("There is no monster with that number.\r\n", ch);
	 return;
      }
      mob = read_mobile(r_num, REAL);
      char_to_room(mob, ch->in_room, FALSE);

      act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
          0, 0, TO_ROOM);
      act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
      act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
   } else if (is_abbrev(buf, "obj")) {
      if ((r_num = real_object(number)) < 0) {
	 send_to_char("There is no object with that number.\r\n", ch);
	 return;
      }
      
      obj = read_object(r_num, REAL, GET_LEVEL(ch) == LEVEL_IMPL);
      if ((GET_LEVEL(ch) != LEVEL_IMPL) && (obj->obj_flags.value[6] > 0) &&
              is_maxed(r_num, REAL)){
	  extract_obj(obj,1);
	  send_to_char("Sorry That object is maxed\r\n",ch);
	  return;}
      if ((GET_LEVEL(ch) < LEVEL_GOD)
	  && ((!IS_SET(obj->obj_flags.extra_flags, ITEM_TEST_ONLY))))
	  SET_BIT(obj->obj_flags.extra_flags, ITEM_TEST_ONLY);
      obj_to_char(obj, ch, 0);
      act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
      act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
      act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
   } else
      send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}



ACMD(do_vstat)
{
   struct char_data *mob;
   struct obj_data *obj;
   int	number, r_num, i;

   if (IS_NPC(ch))
      return;

   argument_interpreter(argument, buf, buf2);

   if (!*buf || !*buf2 || !isdigit(*buf2)) {
      send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
      return;
   }

   if ((number = atoi(buf2)) < 0) {
      send_to_char("A NEGATIVE number??\r\n", ch);
      return;
   }
   if (is_abbrev(buf, "mob")) {
      if ((r_num = real_mobile(number)) < 0) {
	 send_to_char("There is no monster with that number.\r\n", ch);
	 return;
      }
      mob = read_mobile(r_num, REAL);
      do_stat_character(ch, mob);
      char_to_room(mob,0, FALSE);
      extract_char(mob,FALSE);
   }
   else if (is_abbrev(buf, "obj")) {
      if ((r_num = real_object(number)) < 0) {
	 send_to_char("There is no object with that number.\r\n", ch);
	 return;
      }
      obj = read_object(r_num, REAL,0);
      do_stat_object(ch, obj);
      extract_obj(obj,0);
   }
   else if (is_abbrev(buf, "shop")) {
      if ((r_num = real_shop(number)) < 0) {
	 send_to_char("There is no shop with that number.\r\n", ch);
	 return;
      }
      do_stat_shop(ch, &shop_index[r_num]);
   }
   else
      send_to_char("That'll have to be either 'obj' or 'mob' or 'shop'.\r\n", ch);
}





/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
   struct char_data *vict, *next_v;
   struct obj_data *obj, *next_o;

   if (IS_NPC(ch))
      return;

   one_argument(argument, buf);

   if (*buf)  /* argument supplied. destroy single object or char */ {
      if ((vict = get_char_room_vis(ch, buf))) {
	 if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	    send_to_char("Fuuuuuuuuu!\r\n", ch);
	    return;
	 }
	 act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	 act("You disintegrate $N.", FALSE, ch, 0, vict, TO_CHAR);	 

	 if (IS_NPC(vict)) {
	    extract_char(vict, FALSE);
	 } else {
	    sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	    mudlog(buf, BRF, LEVEL_GOD, TRUE);
	    if (vict->desc) {
	       close_socket(vict->desc);
	       vict->desc = 0;
	    }
	    extract_char(vict, TRUE);
	 }
      } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
	  act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
	  act("You destroy $p.", FALSE, ch, obj, 0, TO_CHAR);
	  extract_obj(obj,0);
      } else {
	 send_to_char("I don't know anyone or anything by that name.\r\n", ch);
	 return;
      }

   } else { /* no argument. clean out the room */
      if (IS_NPC(ch)) {
	 send_to_char("Don't... You would only kill yourself..\r\n", ch);
	 return;
      }

      act("$n gestures... You are surrounded by scorching flames!", 
          FALSE, ch, 0, 0, TO_ROOM);
      send_to_room("The world seems a little cleaner.\r\n", ch->in_room, FALSE);

      for (vict = world[ch->in_room].people; vict; vict = next_v) {
	 next_v = vict->next_in_room;
	 if (IS_NPC(vict))
	     extract_char(vict, FALSE);
      }

      for (obj = world[ch->in_room].contents; obj; obj = next_o) {
	 next_o = obj->next_content;
	 extract_obj(obj,0);
      }
   }
}




void	do_start(struct char_data *ch)
{
   void	advance_level(struct char_data *ch);
   struct obj_data *obj, *scabb;
   int rnum;

   GET_LEVEL(ch) = 1;
   GET_SUB_LEVEL(ch) = 0;
   GET_EXP(ch) = 750;

   set_title(ch);

   ch->points.max_hit  = (2*GET_STR(ch) + 6*GET_CON(ch))/10;
   ch->points.max_hit  += 15;
   ch->points.max_move = (GET_STR(ch) + GET_DEX(ch) + GET_CON(ch))*10/5;
   SPELLS_TO_LEARN(ch) = (number(1, GET_RAW_LUC(ch))*(number(1,3)));
   advance_level(ch);

   ch->specials2.edit_zone = -1;

   GET_HIT(ch) = GET_MAX_HIT(ch);
   GET_MOVE(ch) = GET_MAX_MOVE(ch);

   GET_COND(ch, THIRST) = 24;
   GET_COND(ch, FULL) = 24;
   GET_COND(ch, DRUNK) = 0;

   ch->player.time.played = 0;
   ch->player.time.logon = time(0);

/* give him/her some equipment */
   if (!ch->equipment[WEAR_BODY]){
     rnum = real_object(2036); /* jacket */
     obj = read_object(rnum, REAL,0);
     obj->obj_flags.value[5] = GET_SIZE(ch);
     equip_char(ch, obj, WEAR_BODY);
   }
   if (!ch->equipment[WEAR_HEAD]){   
   rnum = real_object(2037); /* cap */
   obj = read_object(rnum, REAL,0);
   obj->obj_flags.value[5] = GET_SIZE(ch);
   equip_char(ch, obj, WEAR_HEAD);
   }
   if (!ch->equipment[WEAR_SHIELD]){   
     rnum = real_object(2038); /* shield */
     obj = read_object(rnum, REAL,0);
     obj->obj_flags.value[5] = GET_SIZE(ch);
     equip_char(ch, obj, WEAR_SHIELD);
   }
   if (!ch->equipment[WEAR_SCABBARD]){   
     rnum = real_object(2029); /* scabbard */
     obj = read_object(rnum, REAL,0);
     scabb = obj;
     obj->obj_flags.value[5] = GET_SIZE(ch);      
     rnum = real_object(2017); /* sword */
     obj = read_object(rnum, REAL,0);
     obj->obj_flags.value[5] = GET_SIZE(ch);   
     obj_to_obj(obj,scabb);
     equip_char(ch, scabb, WEAR_SCABBARD);
   }
   rnum = real_object(2013); /* bread */
   obj = read_object(rnum, REAL,0);
   obj_to_char(obj,ch,1);
   obj = read_object(rnum, REAL,0);
   obj_to_char(obj,ch,1);      
}


ACMD(do_advance)
{
   struct char_data *victim;
   char	name[100], level[100];
   int adv,newlevel;

   void	gain_exp(struct char_data *ch, int gain,int mode);

   if (IS_NPC(ch))
      return;

   half_chop(argument, name, buf);
   one_argument(buf, level);

   if (*name) {
      if (!(victim = get_char_vis(ch, name))) {
	 send_to_char("That player is not here.\r\n", ch);
	 return;
      }
   } else {
      send_to_char("Advance who?\r\n", ch);
      return;
   }

   if (ch == victim) {
      send_to_char("Maybe that's not such a great idea.\r\n", ch);
      return;
   }

   if (IS_NPC(victim)) {
      send_to_char("NO!  Not on NPC's.\r\n", ch);
      return;
   }

   if (GET_LEVEL(victim) == 0)
      adv = 1;
   else if (!*level) {
      send_to_char("You must supply a level number.\r\n", ch);
      return;
   } else {
      if (!isdigit(*level)) {
	 send_to_char("Second argument must be a positive integer.\r\n", ch);
	 return;
      }
      if ((newlevel = atoi(level)) <= 3) {
	 do_start(victim);
	 GET_LEVEL(victim) = newlevel;
      }

      adv = newlevel - GET_LEVEL(victim);
   }

   if (((adv + GET_LEVEL(victim)) > 1) && (GET_LEVEL(ch) < LEVEL_ASS_IMPL)) {
      send_to_char("Thou art not godly enough.\r\n", ch);
      return;
   }

   if ((adv + GET_LEVEL(victim)) > LEVEL_IMPL) {
      send_to_char("227  is the highest possible level.\r\n", ch);
      return;
   }

   act("$n makes some strange gestures.\r\nA strange feeling comes upon you,"
       "\r\nLike a giant hand, light comes down from\r\nabove, grabbing your"
       "body, that begins\r\nto pulse with colored lights from inside.\r\nYo"
       "ur head seems to be filled with demons\r\nfrom another plane as your"
       " body dissolves\r\nto the elements of time and space itself.\r\nSudde"
       "nly a silent explosion of light snaps\r\nyou back to reality.  You fee"
       "l slightly\r\ndifferent.", FALSE, ch, 0, victim, TO_VICT);

   send_to_char("Ok.\r\n", ch);

   if (GET_LEVEL(victim) == 0) {
      do_start(victim);
   } 
   else {
      if (GET_LEVEL(victim) < LEVEL_IMPL) {
	 sprintf(buf, "(GC) %s has advanced %s to level %d (from %d)",
	     GET_NAME(ch), GET_NAME(victim), newlevel, GET_LEVEL(victim));
	 logg(buf);
	 gain_exp_regardless(victim, (levels_table[GET_LEVEL(victim) + adv]) - GET_EXP(victim),0);
	 save_char(victim, NOWHERE);
      } 
      else {
	 send_to_char("Some idiot just tried to advance your level.\r\n", victim);
	 send_to_char("IMPOSSIBLE!  IDIOTIC!\r\n", ch);
      }
   }
}



ACMD(do_restore)
{
   struct char_data *victim;
   int	i;

   void	update_pos( struct char_data *victim );

   if (IS_NPC(ch))
      return;

   one_argument(argument, buf);
   if (!*buf)
      send_to_char("Whom do you wish to restore?\r\n", ch);
   else if (!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\r\n", ch);
   else {
      GET_HIT(victim) = GET_MAX_HIT(victim);
      GET_MOVE(victim) = GET_MAX_MOVE(victim);

      if ((GET_LEVEL(ch) >= LEVEL_GRGOD) && (GET_LEVEL(victim) >= LEVEL_BUILDER)) {
	 for (i = 0; i < MAX_SKILLS; i++)
	    SET_SKILL(victim, i, 30);

	 if (GET_LEVEL(victim) >= LEVEL_GRGOD) {
	    victim->abilities.str = 30;
	    victim->abilities.intel = 30;
	    victim->abilities.wis = 30;
	    victim->abilities.dex = 30;
	    victim->abilities.str = 30;
	    victim->abilities.con = 30;
	    victim->abilities.chr = 30;
	    victim->abilities.per = 30;
	    victim->abilities.gui = 30;
	    victim->abilities.foc = 30;
	    victim->abilities.dev = 30;	    
	    victim->abilities.luc = 30;
	 }
	 victim->tmpabilities = victim->abilities;
      }
      update_pos(victim);
      send_to_char("Done.\r\n", ch);
      act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
   }
}



ACMD(do_invis)
{
   int	level;

   if (IS_NPC(ch)) {
      send_to_char("Yeah.. like a mob knows how to bend light.\r\n", ch);
      return;
   }

   one_argument (argument, arg);
   if (!*arg) {
      if (GET_INVIS_LEV(ch) > 0) {
	 GET_INVIS_LEV(ch) = 0;
	 sprintf(buf, "You are now fully visible.\r\n");
      } else {
	 GET_INVIS_LEV(ch) = GET_LEVEL(ch);
	 sprintf(buf, "Your invisibility level is %d.\r\n", GET_LEVEL(ch));
      }
   } else {
      level = atoi(arg);
      if (level > GET_LEVEL(ch)) {
	 send_to_char("You can't go invisible above your own level.\r\n", ch);
	 return;
      } else if (level < 1) {
	 GET_INVIS_LEV(ch) = 0;
	 sprintf(buf, "You are now fully visible.\r\n");
      } else {
	 GET_INVIS_LEV(ch) = level;
	 sprintf(buf, "Your invisibility level is now %d.\r\n", level);
      }
   }
   send_to_char(buf, ch);
}


ACMD(do_gecho)
{
   int	i;
   struct descriptor_data *pt;

   if (IS_NPC(ch))
      return;

   for (i = 0; *(argument + i) == ' '; i++)
      ;

   if (!*(argument + i))
      send_to_char("That must be a mistake...\r\n", ch);
   else {
      sprintf(buf, "%s\r\n", argument + i);
      for (pt = descriptor_list; pt; pt = pt->next)
	 if (!pt->connected && pt->character && pt->character != ch)
	    act(buf, FALSE, ch, 0, pt->character, TO_VICT);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	 send_to_char("Ok.\r\n", ch);
      else
	 send_to_char(buf, ch);
      if(GET_LEVEL(ch) < LEVEL_IMPL){
          sprintf(buf,"(GC) %s echoed %s in %d",GET_NAME(ch),argument + i, world[ch->in_room].number);
          mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
      }
   }
}


ACMD(do_poofset)
{
   char	**msg;
   int	i;

   switch (subcmd) {
   case SCMD_POOFIN : msg = &(ch->specials.poofIn); break;
   case SCMD_POOFOUT: msg = &(ch->specials.poofOut); break;
   default: return; break;
   }

   for (i = 0; *(argument + i) == ' '; i++)
      ;

   if (*msg)
      free(*msg);

   if (!*(argument + i))
      *msg = NULL;
   else
      *msg = str_dup(argument + i);

   send_to_char("Ok.\r\n", ch);
}





ACMD(do_dc)
{
   struct descriptor_data *d;
   int	num_to_dc;

   if (IS_NPC(ch)) {
      send_to_char("Monsters can't cut connections... leave me alone.\r\n", ch);
      return;
   }

   if (!(num_to_dc = atoi(argument))) {
      send_to_char("Usage: DC <connection number> (type USERS for a list)\r\n", ch);
      return;
   }

   for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next)
      ;

   if (!d) {
      send_to_char("No such connection.\r\n", ch);
      return;
   }

   if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
      send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
      return;
   }

   close_socket(d);
   sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
   send_to_char(buf, ch);
   sprintf(buf, "(GC) Connection closed by %s.", GET_NAME(ch));
   logg(buf);
}



ACMD(do_wizlock)
{
   int	value;
   char *when;

   one_argument(argument, arg);
   if (*arg) {
      value = atoi(arg);
      if (value < 0 || value > LEVEL_IMPL) {
         send_to_char("Invalid wizlock value.\r\n", ch);
         return;
      }
      restrict = value;
      when = "now";
   } else
      when = "currently";

   switch (restrict) {
   case 0 :
      sprintf(buf, "The game is %s completely open.\r\n", when);
      break;
   case 1 :
      sprintf(buf, "The game is %s closed to new players.\r\n", when);
      break;
   default :
      sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
          restrict, when);
      break;
   }
   send_to_char(buf, ch);
}


ACMD(do_date)
{
   long	ct;
   char	*tmstr;

   if (IS_NPC(ch))
      return;

   ct = time(0);
   tmstr = (char *)asctime(localtime(&ct));
   *(tmstr + strlen(tmstr) - 1) = '\0';
   sprintf(buf, "Current machine time: %s\r\n", tmstr);
   send_to_char(buf, ch);
}



ACMD(do_uptime)
{
   char	*tmstr;
   long	uptime;
   int	d, h, m;

   extern long	boot_time;

   if (IS_NPC(ch))
      return;

   tmstr = (char *)asctime(localtime(&boot_time));
   *(tmstr + strlen(tmstr) - 1) = '\0';

   uptime = time(0) - boot_time;
   d = uptime / 86400;
   h = (uptime / 3600) % 24;
   m = (uptime / 60) % 60;

   sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
       ((d == 1) ? "" : "s"), h, m);

   send_to_char(buf, ch);
}



ACMD(do_last)
{
   struct char_file_u chdata;
   extern char	*race_abbrevs[];

   if (IS_NPC(ch))
      return;

   if (!*argument) {
      send_to_char("For whom do you wish to search?\r\n", ch);
      return;
   }

   one_argument(argument, arg);
   if (load_char(arg, &chdata) < 0) {
      send_to_char("There is no such player.\r\n", ch);
      return;
   }
   if ((chdata.level > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LEVEL_GRGOD)) {
      send_to_char("You are not sufficiently godly for that!\r\n", ch);
      return;
   }

   sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
       chdata.specials2.idnum, chdata.level, race_abbrevs[(int)chdata.race],
       chdata.name, chdata.host, ctime(&chdata.last_logon));
   send_to_char(buf, ch);
}


ACMD(do_force)
{
   struct descriptor_data *i;
   struct char_data *vict;
   char	name[100], to_force[MAX_INPUT_LENGTH+2];

   if (IS_NPC(ch)) {
      send_to_char("Umm.... no.\r\n", ch );
      return;
   }

   half_chop(argument, name, to_force);

   sprintf(buf1, "%s has forced you to %s.\r\n", GET_NAME(ch), to_force);
   sprintf(buf2, "Someone has forced you to %s.\r\n", to_force);

   if (!*name || !*to_force)
      send_to_char("Whom do you wish to force do what?\r\n", ch);
   else if (str_cmp("all", name) && str_cmp("room", name)) {
      if (!(vict = get_char_vis(ch, name)) || !CAN_SEE(ch, vict))
	 send_to_char("No-one by that name here...\r\n", ch);
      else {
	 if (GET_LEVEL(ch) > GET_LEVEL(vict)) {
	    send_to_char("Ok.\r\n", ch);
	    if (CAN_SEE(vict, ch) && GET_LEVEL(ch) < LEVEL_IMPL)
	       send_to_char(buf1, vict);
	    else if (GET_LEVEL(ch) < LEVEL_IMPL) {
	       send_to_char(buf2, vict);
	    }
	    if (GET_LEVEL(ch) < LEVEL_IMPL) {
	       sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), name, to_force);
	       logg(buf);
                                   /*added to syslog, 1/10/95 -Soraya*/
               mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
	    }
	    command_interpreter(vict, to_force);
	 } else
	    send_to_char("No, no, no!\r\n", ch);
      }
   } else if (str_cmp("room", name)) {
      send_to_char("Okay.\r\n", ch);
      if (GET_LEVEL(ch) < LEVEL_IMPL) {
	 sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), name, to_force);
	 logg(buf);
                                   /*added to syslog, 1/10/95 -Soraya*/
         mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
      }
      for (i = descriptor_list; i; i = i->next)
	 if (i->character != ch && !i->connected) {
	    vict = i->character;
	    if (GET_LEVEL(ch) > GET_LEVEL(vict)) {
	       if (CAN_SEE(vict, ch) && GET_LEVEL(ch) < LEVEL_IMPL)
		  send_to_char(buf1, vict);
	       else if (GET_LEVEL(ch) < LEVEL_IMPL)
		  send_to_char(buf2, vict);
	       command_interpreter(vict, to_force);
	    }
	 }
   } else {
      send_to_char("Okay.\r\n", ch);
      if (GET_LEVEL(ch) < LEVEL_IMPL) {
	 sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), name, to_force);
	 logg(buf);
                                   /*added to syslog, 1/10/95 -Soraya*/
         mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
      }
      for (i = descriptor_list; i; i = i->next)
	 if (i->character != ch && !i->connected && 
	     i->character->in_room == ch->in_room) {
	    vict = i->character;
	    if (GET_LEVEL(ch) > GET_LEVEL(vict)) {
	       if (CAN_SEE(vict, ch) && GET_LEVEL(ch) < LEVEL_IMPL)
		  send_to_char(buf1, vict);
	       else if (GET_LEVEL(ch) < LEVEL_IMPL)
		  send_to_char(buf2, vict);
	       command_interpreter(vict, to_force);
	    }
	 }
   }
}

ACMD(do_wiznet)
{
   struct descriptor_data *d;
   char	any = FALSE;
   int	level = LEVEL_BUILDER;

/*   if (IS_NPC(ch)) {
      send_to_char("Yeah - like the Gods are interested in listening to mobs.\r\n", ch);
      return;
   }
   */
   for ( ; *argument == ' '; argument++)
      ;

   if (!*argument) {
      send_to_char("Usage: wiznet [<text>|#<level> <text>|@|-|+]\r\n", ch);
      return;
   }

   switch (*argument) {
   /* patched 9-24-94 by Scarrow -- removed dull * case and fixed bad */
   /* pointer incrementing */
   case '#':
      one_argument(argument, buf1);
      if (is_number(buf1 + 1)) {
	 half_chop(argument, buf1, argument);
	 level = MAX(atoi(buf1 + 1), LEVEL_BUILDER);
	 if (level > GET_LEVEL(ch)) {
	    send_to_char("You can't wizline above your own level.\r\n", ch);
	    return;
	 }
      }
      break;
   case '@':
      for (d = descriptor_list; d; d = d->next) {
	 if (!d->connected && GET_LEVEL(d->character) >= LEVEL_BUILDER && 
	     !PRF_FLAGGED(d->character, PRF_NOWIZ) && 
	     (CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LEVEL_IMPL) ) {
	    if (!any) {
	       sprintf(buf1, "Gods online:\r\n");
	       any = TRUE;
	    }
	    sprintf(buf1, "%s  %s", buf1, GET_NAME(d->character));
	    if (PLR_FLAGGED(d->character, PLR_WRITING))
	       sprintf(buf1, "%s (Writing)\r\n", buf1);
	    else if (PLR_FLAGGED(d->character, PLR_MAILING))
	       sprintf(buf1, "%s (Writing mail)\r\n", buf1);
	    else
	       sprintf(buf1, "%s\r\n", buf1);

	 }
      }
      any = FALSE;
      for (d = descriptor_list; d; d = d->next) {
	 if (!d->connected && GET_LEVEL(d->character) >= LEVEL_BUILDER && 
	     PRF_FLAGGED(d->character, PRF_NOWIZ) && 
	     CAN_SEE(ch, d->character) ) {
	    if (!any) {
	       sprintf(buf1, "%sGods offline:\r\n", buf1);
	       any = TRUE;
	    }
	    sprintf(buf1, "%s  %s\r\n", buf1, GET_NAME(d->character));
	 }
      }
      send_to_char(buf1, ch);
      return;
      break;
   case '-':
      if (PRF_FLAGGED(ch, PRF_NOWIZ))
	 send_to_char("You are already offline!\r\n", ch);
      else {
	 send_to_char("You will no longer hear the wizline.\r\n", ch);
	 SET_BIT(PRF_FLAGS(ch), PRF_NOWIZ);
      }
      return;
      break;
   case '+':
      if (!PRF_FLAGGED(ch, PRF_NOWIZ))
	 send_to_char("You are already online!\r\n", ch);
      else {
	 send_to_char("You can now hear the wizline again.\r\n", ch);
	 REMOVE_BIT(PRF_FLAGS(ch), PRF_NOWIZ);
      }
      return;
      break;
   case '\\':
      ++argument;
      break;
   default:
      break;
   }
   if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
      send_to_char("You are offline!\r\n", ch);
      return;
   }

   for ( ; *argument == ' '; argument++)
      ;
   if (!*argument) {
      send_to_char("Don't bother the gods like that!\r\n", ch);
      return;
   }

   if (level > LEVEL_BUILDER) 
      sprintf(buf1, "<%d> %s\r\n", level, argument);
   else
      sprintf(buf1, "%s\r\n", argument);
   

   for (d = descriptor_list; d; d = d->next) {
      if ( (!d->connected) && (GET_LEVEL(d->character) >= level) &&
	  (!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
          (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING))
           && (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
	 sprintf(buf2,"%s%s:%s ",CCBGRN(d->character, C_NRM),((CAN_SEE(d->character, ch)) ? GET_NAME(ch):"Someone"), CCGRN(d->character, C_NRM));
	 send_to_char(buf2, d->character);
	 send_to_char(buf1, d->character);
	 send_to_char(CCNRM(d->character, C_NRM), d->character);
      }
   }

   if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char("Ok.\r\n", ch);
}

ACMD(do_zreset)
{
   int	i, j;

   if IS_NPC(ch) {
      send_to_char("Homie don't play that!\r\n", ch);
      return;
   }
   if (!*argument) {
      send_to_char("You must specify a zone.\r\n", ch);
      return;
   }
   one_argument(argument, arg);
   if (*arg == '*') {
      for (i = 0; i <= top_of_zone_table; i++)
	 reset_zone(i);
      send_to_char("Reset world.\r\n", ch);
      return;
   } else if (*arg == '.')
      i = world[ch->in_room].zone;
   else {
      j = atoi(arg);
      for (i = 0; i <= top_of_zone_table; i++)
	 if (zone_table[i].number == j)
	    break;
   }
   if (i >= 0 && i <= top_of_zone_table) {
      reset_zone(i);
      sprintf(buf, "Reset zone %d: %s.\r\n", i, zone_table[i].name);
      send_to_char(buf, ch);
      sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
      mudlog(buf, NRM, MAX(LEVEL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
   } else
      send_to_char("Invalid zone number.\r\n", ch);
}


/* 
  General fn for wizcommands of the sort: cmd <player>
*/

ACMD(do_wizutil)
{
   struct char_data *vict;
   char	name[40];
   long	result;

   if (IS_NPC(ch)) {
      send_to_char("You're just an unfrozen caveman NPC.\r\n", ch);
      return;
   }
   one_argument(argument, name);
   if (!*name) {
      send_to_char("Yes, but for whom?!?\r\n", ch);
      return;
   }
   if (!(vict = get_char_vis(ch, name))) {
      send_to_char("There is no such player.\r\n", ch);
      return;
   }
   if (IS_NPC(vict)) {
      send_to_char("You can't do that to a mob!\r\n", ch);
      return;
   }
   if (GET_LEVEL(vict) > GET_LEVEL(ch)) {
      send_to_char("Hmmm...you'd better not.\r\n", ch);
      return;
   }

   switch (subcmd) {
   case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF | PLR_KILLER)) {
	 send_to_char("Your victim is not flagged.\r\n", ch);
	 return;
      }
      REMOVE_BIT(PLR_FLAGS(vict), PLR_THIEF | PLR_KILLER);
      send_to_char("Pardoned.\r\n", ch);
      send_to_char("You have been pardoned by the Gods!\r\n", vict);
      sprintf(buf, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LEVEL_GOD, GET_INVIS_LEV(ch)), TRUE);
      break;
   case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
          GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX(LEVEL_GOD, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
   case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
          GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LEVEL_GOD, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
   case SCMD_FREEZE:
      if (ch == vict) {
	 send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
	 return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	 send_to_char("Your victim is already pretty cold.\r\n", ch);
	 return;
      }
      SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      vict->specials2.freeze_level = GET_LEVEL(ch);
      send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n",
                                         vict);
      send_to_char("Frozen.\r\n", ch);
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LEVEL_GOD, GET_INVIS_LEV(ch)), TRUE);
      break;
   case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	 send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
	 return;
      }
      if (vict->specials2.freeze_level > GET_LEVEL(ch)) {
	 sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
	     vict->specials2.freeze_level,
	     GET_NAME(vict),
	     HMHR(vict));
	 send_to_char(buf, ch);
	 return;
      }
      sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LEVEL_GOD, GET_INVIS_LEV(ch)), TRUE);
      REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
      send_to_char("Thawed.\r\n", ch);
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
   case SCMD_UNAFFECT:
      if (vict->affected) {
	 while (vict->affected)
	    affect_remove(vict, vict->affected);
	 send_to_char("All spells removed.\r\n", ch);
	 send_to_char("There is a brief flash of light!\r\n"
	     "You feel slightly different.\r\n", vict);
      } else {
	 send_to_char("Your victim does not have any affections!\r\n", ch);
	 return;
      }
      break;
   case SCMD_REROLL:
      send_to_char("disabled...\r\n", ch);
      break;
   }
   save_char(vict, NOWHERE);
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void	print_zone_to_buf(char *bufptr, int zone)
{
   sprintf(bufptr, "%3d %-30.30s Age: %3d Reset: %3d (%1d) Top: %5d %s\r\n",
       zone_table[zone].number, zone_table[zone].name,
       zone_table[zone].age, zone_table[zone].lifespan,
       zone_table[zone].reset_mode, zone_table[zone].top,
	   ((zone_table[zone].open) ? "Open":"Closed"));
}

#define ZCMD zone_table[zone].cmd[cmd_no]

ACMD(do_show)
{
   struct char_file_u vbuf;
   char bufa[40],bufb[40];
   char *bffzm=0;
   int zone, cmd_no,next_zone;
   int	i, jj, j=-1, k, l, con, room=-1,rroom=-1;
   char	*tmstr;
   long	uptime;
   int	d, h, m, iloc= -1, ibot=0, itop=10000;
   char	self = 0, mode=0;
   struct zone_obj_list *zol;
   struct char_data *vict;
   struct obj_data *obj;
   char	field[40], value[40], birth[80];
   extern char	*race_abbrevs[];
   extern char	*genders[];
   extern int buf_switches, buf_largecount, buf_overflows;
   extern long	boot_time;
   bool twinkie = FALSE, all=FALSE, last_cmd=FALSE;

   struct show_struct {
      char	*cmd;
      short	level;
   } fields[] = {
      { "nothing", 	0 },
      { "zone", 	LEVEL_BUILDER },
      { "players", 	LEVEL_GOD },
      { "rent", 	LEVEL_GOD },
      { "stats", 	1 },
      { "limited", 	LEVEL_BUILDER },
      { "death", 	LEVEL_BUILDER },
      { "godrooms", 	LEVEL_GOD },
      { "player",        LEVEL_GOD},      
      { "zones",        1},      
      { "rooms",        LEVEL_BUILDER},
      { "mobs",        LEVEL_BUILDER},
      { "objs",        LEVEL_BUILDER},
      { "teleports",   LEVEL_BUILDER},
      { "goditems",    LEVEL_BUILDER},
      { "shops",       LEVEL_MBUILDER},
      { "noformat",       LEVEL_BUILDER},      
      { "\n", 0 }
   };   

   if IS_NPC(ch) {
      send_to_char("Oh no!  None of that stuff!\r\n", ch);
      return;
   }
   if (!*argument) {
      strcpy(buf, "Show options:\r\n");
      for (j = 0, i = 1; fields[i].level; i++)
	 if (fields[i].level <= GET_LEVEL(ch))
	    sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      return;
   }
   half_chop(argument, field, arg);
   half_chop(arg, value, arg);

   for (l = 0; *(fields[l].cmd) != '\n'; l++)
      if (!strncmp(field, fields[l].cmd, strlen(field)))
	 break;

   if (GET_LEVEL(ch) < fields[l].level) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      return;
   }
   if (!strcmp(value, "."))
      self = 1;
   buf[0] = '\0';
   if (l == 16) {
     mode = 1;
     l = 6;
   }
   switch (l) {
   case 1: /* zone */
     rroom = ch->in_room;
     j = ch->specials2.edit_zone;
     if (*value && is_number(value))
       j = atoi(value);
     else{
       sprintf(buf,"%s %s",value, arg);
       sprintf(arg,"%s",buf);
     }
     if (*arg){
       half_chop(arg, buf1, buf2);
       if (!strcmp("all", buf1)){
	 sprintf(arg,"%s",buf2);
	 all = TRUE;
       }
       else if (!strcmp("at", buf1)){
	 room = atoi(buf2);
	 if ((rroom = real_room(room)) < 0 ){
	   send_to_char("No such room.\r\n",ch);
	   return;
	 }
       }
       else
	 sprintf(arg,"%s %s",buf1, buf2);
       half_chop(arg, buf1, buf2);       
       if (!strcmp("about", buf1)){
	 iloc = atoi(buf2);
	 if (iloc < 0){
	   send_to_char("Illegal reset number.\r\n",ch);
	   return;}
       }
       else if (!strcmp("from", buf1)){
	 ibot = atoi(buf2);
	 iloc = -1;
	 if (ibot < 0){
	   send_to_char("Illegal reset number.\r\n",ch);
	   return;}
       }	       
     }
     for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
       ;
     if (zone > top_of_zone_table)
       {
	 send_to_char("Illegal zone number.\r\n",ch);
	 return;
       }
     for (cmd_no = 0; ;cmd_no++) {
       if (ZCMD.command == 'S')
	 break;}
     if (iloc >= 0 ){
       if (iloc > cmd_no)
	 iloc = cmd_no;
       ibot = MAX(iloc - 10, 0);
       itop = iloc + 10;}
     else{
       if (ibot > cmd_no)
	 ibot  = MAX(cmd_no - 20, 0);
     }
     print_zone_to_buf(buf, zone);
     send_to_char(buf,ch);
     send_to_char("\r\n",ch);
     if (!all){
       sprintf(buf, "Resets for room: %s%s%s, %s#%d%s.\r\n",
	       CCBBLU(ch, C_NRM), world[rroom].name, CCNRM(ch, C_NRM),
	       CCGRN(ch, C_NRM),world[rroom].number, CCNRM(ch, C_NRM));
       send_to_char(buf,ch);     
     }
     *buf2 = 0;
     for (cmd_no = ibot; cmd_no <= itop;cmd_no++){
       if (ZCMD.command == 'S')
	 break;
       else if (ZCMD.command == 'M' || ZCMD.command == 'm' ){
	 if (!all && ZCMD.arg3 != rroom){
	   last_cmd = FALSE;
	   continue;
	 }
	 else
	   last_cmd = TRUE;
	 if (ZCMD.arg1 > top_of_mobt)
	   printf("error in Zone: %d at reset: %d, %d > top of mob table\n",zone, cmd, ZCMD.arg1);
	 else{
	     sprintf(bufa,"%sM%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	     sprintf(bufb,"%s %d %s",CCRED(ch,C_NRM),ZCMD.arg1,CCNRM(ch,C_NRM));
	     sprintf(buf,"[%3d] M: %-25.25s in %-25.25s %2d %2d %3d\r\n"
		     ,cmd_no
		     ,((mob_proto[ZCMD.arg1].player.short_descr) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		     ,((world[ZCMD.arg3].name) ? world[ZCMD.arg3].name : bufb)
		     ,ZCMD.arg2, ZCMD.num, ZCMD.percent);}
       }
       else if (ZCMD.command == 'F' || ZCMD.command == 'f' ){
	 if (!all && !last_cmd)
	   continue;
       if (ZCMD.arg1 > top_of_mobt)
	   printf("error in Zone: %d at reset: %d, %d > top of mob table\n",zone, cmd, ZCMD.arg1);
	 else{
	   sprintf(bufa,"%sF%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	   sprintf(bufb,"%s %d %s",CCRED(ch,C_NRM),ZCMD.arg1,CCNRM(ch,C_NRM));
	   sprintf(buf,"[%3d] F: %-25.25s %-28.28s %2d %2d %3d\r\n"
		   ,cmd_no
		   ,((mob_proto[ZCMD.arg1].player.name) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		   ,"",ZCMD.arg2, ZCMD.num, ZCMD.percent);}
       }	   
       else if (ZCMD.command == 'R' || ZCMD.command == 'r'){
	 if (!all && !last_cmd)
	   continue;	 
	 if (ZCMD.arg1 > top_of_mobt)
	   printf("error in Zone: %d at reset: %d, %d > top of mob table\n",zone, cmd, ZCMD.arg1);
	 else{
	   sprintf(bufa,"%sR%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	   sprintf(bufb,"%s %d %s",CCRED(ch,C_NRM),ZCMD.arg1,CCNRM(ch,C_NRM));
	   sprintf(buf,"[%3d] Mt: %-23.23s %2d %2d %3d\r\n"
		   ,cmd_no
		   ,((mob_proto[ZCMD.arg1].player.name) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		   ,ZCMD.arg2, ZCMD.num, ZCMD.percent);}
       }	   
       else if (ZCMD.command == 'C' || ZCMD.command == 'c'){
	 if (!all && !last_cmd)
	   continue;	 
	 if (ZCMD.arg1 > top_of_mobt)
	   printf("error in Zone: %d at reset: %d, %d > top of mob table\n",zone, cmd, ZCMD.arg1);
	 else{
	   sprintf(bufa,"%sC%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	   sprintf(bufb,"%s %d %s",CCRED(ch,C_NRM),ZCMD.arg1,CCNRM(ch,C_NRM));
	   sprintf(buf,"[%3d] Cy: %-23.23s %2d %2d %3d\r\n"
		   ,cmd_no
		   ,((mob_proto[ZCMD.arg1].player.name) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		   ,ZCMD.arg2, ZCMD.num, ZCMD.percent);}
       }	   
       else if (ZCMD.command == 'O' || ZCMD.command == 'o'){
	 if (!all && ZCMD.arg3 != rroom){
	   last_cmd = FALSE;
	   continue;
	 }
	 else
	   last_cmd = TRUE;
	 
	 if (ZCMD.arg1 > top_of_objt)
	   printf("error in Zone: %d at reset: %d, %d > top of obj table\n",zone, cmd, ZCMD.arg1);
	 else{
	   sprintf(bufa,"%sO%d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	   sprintf(bufb,"%s %d %s",CCRED(ch,C_NRM),ZCMD.arg1,CCNRM(ch,C_NRM));
	   sprintf(buf,"[%3d] O: %-25.25s in %-25.25s %2d %2d %3d\r\n"
		   ,cmd_no
		   ,((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		   ,((world[ZCMD.arg3].name) ? world[ZCMD.arg3].name : bufb)
		   ,ZCMD.arg2, ZCMD.num, ZCMD.percent);
	 }
       }
       else if (ZCMD.command == 'P' || ZCMD.command == 'p'){
	 if (!all && !last_cmd)
	   continue;	 
	 if (ZCMD.arg1 > top_of_objt)
	   printf("error in Zone: %d at reset: %d, %d > top of obj_table\n",zone, cmd, ZCMD.arg1);
	 else if (ZCMD.arg3 > top_of_objt)
	   printf("error in Zone: %d at reset: %d, %d > top of obj_table\n",zone, cmd, ZCMD.arg3);
	 else{
	   sprintf(bufa,"%s %d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	   sprintf(bufb,"%s %d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg3].virtual,CCNRM(ch,C_NRM));
	   sprintf(buf,"[%3d] P: %-25.25s in %-25.25s %2d %2d %3d\r\n"
		   ,cmd_no
		   ,((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		   ,((obj_proto[ZCMD.arg3].short_description) ? obj_proto[ZCMD.arg3].short_description : bufb)
		   ,ZCMD.arg2, ZCMD.num, ZCMD.percent);
	 }
       }
       else if (ZCMD.command == 'G' || ZCMD.command == 'g'){
	 if (!all && !last_cmd)
	   continue;	 
	 sprintf(bufa,"%s %d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	 sprintf(buf,"[%3d] G: %-25.25s    %-25.25s %2d %2d %3d\r\n",
		 cmd_no,
		 ((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		 ,"",ZCMD.arg2, ZCMD.num, ZCMD.percent);}
       else if (ZCMD.command == 'H' || ZCMD.command == 'h'){
	 if (!all && !last_cmd)
	   continue;	 
	 sprintf(bufa,"%s %d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	 sprintf(buf,"[%3d] C: %-25.25s    %-25.25s %2d %2d %3d\r\n",
		 cmd_no,
		 ((mob_proto[ZCMD.arg1].player.name) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		 ,"",ZCMD.arg2, ZCMD.num, ZCMD.percent);}
       else if (ZCMD.command == 'E' || ZCMD.command == 'e'){
	 if (!all && !last_cmd)
	   continue;
	 sprintf(bufa,"%s %d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,CCNRM(ch,C_NRM));
	 sprintf(buf,"[%3d] E: %-25.25s    %-25.25s %2d %2d %3d \r\n",
		 cmd_no,
		 ((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		 ,where[ZCMD.arg3], ZCMD.arg2, ZCMD.num, ZCMD.percent);}
       else if (ZCMD.command == 'D' || ZCMD.command == 'd'){
	 if (!all && ZCMD.arg1 != rroom){
	   last_cmd = FALSE;
	   continue;
	 }
	 else
	   last_cmd = FALSE;

	 sprintf(bufb,"%s %d %s",CCRED(ch,C_NRM),ZCMD.arg1,CCNRM(ch,C_NRM));
	 sprintf(buf,"[%3d] D: %-25.25s in %-23.23s %-7.7s %3d\r\n",cmd_no, dirs[ZCMD.arg2],((world[ZCMD.arg1].name) ? world[ZCMD.arg1].name : bufb) ,exit_bits[ZCMD.arg3], ZCMD.percent);
       }
       else
	 *buf = '\0';
       
       if (strlen(buf2) + strlen(buf) +1 < MAX_STRING_LENGTH)
	 strcat(buf2,buf);
       else{
	 send_to_char("Too many Zone resets to display at once!\r\n",ch);
	 send_to_char("Use show zone # ( from # | about #).\r\n",ch);
	 break;
       }

     }
     if (buf2)
       page_string(ch->desc,buf2,1);
     break;      

   case 2: /* players */
     *buf = 0;
     *buf2 = 0;
     if (!*value){
       for (i=0;i <= top_of_p_table;i++){
	 sprintf(buf," Player#%4d:  %s%-11s%s%s"
		 ,i,CCGRN(ch,C_NRM),(player_table +i)->name
		 ,CCNRM(ch,C_NRM)
		 ,((!((i+1)%3) && i) ? "\r\n" : ""));
	 if (strlen(buf2) + strlen(buf) +1 < MAX_STRING_LENGTH)
	   strcat(buf2,buf);
	 else{
	   send_to_char("Too many players to list at once!\r\n",ch);
	   break;
	 }
       }
     }
     else{
       if (!*arg || (!is_abbrev(value, "from") && !is_abbrev(value,"about"))){
	 send_to_char("Usage: show players [from|about #]\r\n", ch);
	 return;
       }
       if (is_abbrev(value, "from")){
	 ibot = atoi(arg);
	 itop = top_of_p_table;
	 if (ibot < 0)
	   ibot  = 0;
	 if (ibot >= top_of_p_table)
	   ibot  = top_of_p_table - 62;
	 itop = top_of_p_table;	 
       }
       else if (is_abbrev(value,"about")){
	 ibot = atoi(arg) -31;
	 if (ibot < 0)
	   ibot  = 0;
	 if (ibot >= top_of_p_table)
	   ibot  = top_of_p_table - 62;
	 itop = ibot + 62;
       }
       for (i=ibot;i <= itop ;i++){
	 sprintf(buf," Player#%4d:  %s%-11s%s%s"
		 ,i,CCGRN(ch,C_NRM),(player_table +i)->name
		 ,CCNRM(ch,C_NRM)
		 ,((!((i+1)%3) && i) ? "\r\n" : ""));
	 if (strlen(buf2) + strlen(buf) +1 < MAX_STRING_LENGTH)
	   strcat(buf2,buf);
	 else{
	   send_to_char("Too many players to list at once!\r\n",ch);
	   break;
	 }
       }
     }
     if (buf2)
       page_string(ch->desc,buf2,1);
     return;
     break;
   case 3:
     Crash_listrent(ch, value);
      break;
   case 4:
       tmstr = (char *)asctime(localtime(&boot_time));
      *(tmstr + strlen(tmstr) - 1) = '\0';

      uptime = time(0) - boot_time;
      d = uptime / 86400;
      h = (uptime / 3600) % 24;
      m = (uptime / 60) % 60;

      i = 0;
      j = 0;
      k = 0;
      con = 0;
      for (vict = character_list; vict; vict = vict->next) {
	 if (IS_NPC(vict))
	    j++;
	 else if (CAN_SEE(ch, vict)) {
	    i++;
	    if (vict->desc)
	       con++;
	 }
      }
       for (obj = object_list; obj; obj = obj->next)
	 k++;
       jj = 0;
       for (l=0;l<=top_of_world;l++)
	  if (IS_SET(world[l].room_flags, UNFINISHED))
	      jj++;
      sprintf(buf, "Current stats:\r\n");
      sprintf(buf, "%sUp since %s: %d day%s, %d:%02d\r\n",buf, tmstr, d,
	      ((d == 1) ? "" : "s"), h, m);
      sprintf(buf, "%s  %5d players in game  %5d connected\r\n", buf, i, con);
      sprintf(buf, "%s  %5d registered\r\n", buf, top_of_p_table + 1);
      sprintf(buf, "%s  %5d mobiles          %5d prototypes\r\n",
	      buf, j, top_of_mobt + 1);
      sprintf(buf, "%s  %5d objects          %5d prototypes\r\n",
          buf, k, top_of_objt + 1);
      sprintf(buf, "%s  %5d rooms            %5d zones\r\n", 
          buf, top_of_world + 1, top_of_zone_table + 1);
      sprintf(buf, "%s %s %5d %sunfinished rooms\r\n", 
          buf, CCBRED(ch, C_NRM),jj,CCNRM(ch,C_NRM));       
      sprintf(buf, "%s  %5d large bufs\r\n", buf, buf_largecount);
      sprintf(buf, "%s  %5d buf switches     %5d overflows\r\n", buf,
	  buf_switches, buf_overflows);
      send_to_char(buf, ch);
      break;
   case 5:
       strcpy(buf,"Limited Items\r\n-------------\r\n");
       send_to_char(buf,ch);
       for ( i = 0 ; i <= top_of_ol_table ; i++ ) {
	   j = real_object(obj_limit_table[i].obj_num);
	   if (j > 0)
	       sprintf(buf,"%s%-35s%s, Vnum: %5d, Max: %3d, In Rent: %3d\r\n"
		       ,CCCYN(ch,C_NRM),obj_proto[j].short_description,CCNRM(ch,C_NRM)
		       ,obj_limit_table[i].obj_num
		       ,obj_proto[j].obj_flags.value[6]
		       ,obj_limit_table[i].no_stored);
	   if (bffzm){
	       RECREATE(bffzm, char, (strlen(bffzm) + strlen(buf) +1));
	       strcat(bffzm,buf);}
	   else{
	       CREATE(bffzm,char,strlen(buf) +1);
	       strcpy(bffzm,buf);}
       }
       if (bffzm)
	   page_string(ch->desc,bffzm,1);
       free(bffzm);       
      
       break;
   case 6:
       if (!*value && ch->specials2.edit_zone != -1)
	   jj = ch->specials2.edit_zone;
       else if ( is_number(value))
	   jj = atoi(value);
       else
	   jj = -1;
       if (*value && jj < 0)
	   if (strcmp(value,"all")){
	     if (!mode)
	       send_to_char("Usage: show death [ zone# | all ]\r\n",ch);
	     else
	       send_to_char("Usage: show noformat [ zone# | all ]\r\n",ch);
	     return;}
       if (jj > -1){
	 for (zone = 0; zone_table[zone].number != jj && zone <= top_of_zone_table; zone++)
	   ;
       if (zone > top_of_zone_table)
	   {
	       send_to_char("Illegal zone number - type show zones.\r\n",ch);
	       return;
	   }
       }
       if (jj >-1 && zone <= top_of_zone_table)
	 {
	   sprintf(buf,"%s for Zone: %3d\r\n-------------------------\r\n",
		   ((!mode) ? "Death Traps":"No-Format Rooms"),jj);
	   send_to_char(buf,ch);
	   for (i = 0, j = 0; i < top_of_world; i++)
	     if (world[i].zone == zone)
	       if ((IS_SET(world[i].room_flags,DEATH) && !(mode)) ||
		   (IS_SET(world[i].room_flags,NO_AUTOFORMAT) && mode)){
		 sprintf(buf, "%3d: [%5d] %s\r\n", ++j,
			       world[i].number, world[i].name);
		       if (bffzm){
			   RECREATE(bffzm, char, (strlen(bffzm) + strlen(buf) +1));
			   strcat(bffzm,buf);}
		       else{
			   CREATE(bffzm,char,strlen(buf) +1);
			   strcpy(bffzm,buf);}
		   }
	   }
       else
	   {
	     if (!mode)
	       send_to_char("Death Traps\r\n-----------\r\n",ch);
	     else
	       send_to_char("No-Format Rooms\r\n-----------\r\n",ch);
	       for (i = 0, j = 0; i < top_of_world; i++)
		   if ((IS_SET(world[i].room_flags, DEATH) && !mode) ||
		       (IS_SET(world[i].room_flags, NO_AUTOFORMAT) && mode)){
		       sprintf(buf, "%4d: [%5d] %s\r\n", ++j,
			       world[i].number, world[i].name);
		       if (bffzm){
			   RECREATE(bffzm, char, (strlen(bffzm) + strlen(buf) +1));
			   strcat(bffzm,buf);}
		       else{
			   CREATE(bffzm,char,strlen(buf) +1);
			   strcpy(bffzm,buf);}
		   }
	   }
       if (bffzm)
	   page_string(ch->desc,bffzm,1);
       free(bffzm);       
      break;
   case 7:
#define GOD_ROOMS_ZONE 2
      send_to_char("Godrooms\r\n---------\r\n",ch);
      for (i = 0, j = 0; i < top_of_world; i++)
	  if (world[i].zone == GOD_ROOMS_ZONE){
	      sprintf(buf, "%2d: [%5d] %s\r\n", j++, world[i].number,
	        world[i].name);
       	       if (bffzm){
		   RECREATE(bffzm, char, (strlen(bffzm) + strlen(buf) +1));
		   strcat(bffzm,buf);}
	       else{
		   CREATE(bffzm,char,strlen(buf) +1);
		   strcpy(bffzm,buf);}
	  }
       if (bffzm)
       page_string(ch->desc,bffzm,1);
       free(bffzm);       
      break;
   case 8: /* player */
      if (load_char(value, &vbuf) < 0) {
	 send_to_char("There is no such player.\r\n", ch);
	 return;
      }
      sprintf(buf, "Player: %-12s (%s) [%2d %s]\r\n", vbuf.name,
          genders[(int)vbuf.sex], vbuf.level, race_abbrevs[(int)vbuf.race]);
      sprintf(buf,
          "%sAu: %-8d  Bal: %-8d  Exp: %-8d  Align: %-5d  Lessons: %-3d\r\n",
          buf, vbuf.points.gold, vbuf.points.bank_gold, vbuf.points.exp,
          vbuf.specials2.alignment, vbuf.specials2.spells_to_learn);
      sprintf(birth, "%d", ctime(&vbuf.birth));
      sprintf(buf,
          "%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
          buf, birth, ctime(&vbuf.last_logon), (int)(vbuf.played / 3600),
	  (int)(vbuf.played / 60 % 60));
      send_to_char(buf, ch);
      break;
   case 9: /* show zone */
     if (GET_LEVEL(ch) >= LEVEL_BUILDER) {
       if (self)
	 print_zone_to_buf(buf, world[ch->in_room].zone);
       else if (is_number(value)) {
	 for (j = atoi(value), i = 0;
	      zone_table[i].number != j && i <= top_of_zone_table;
	      i++)
	   ;
	 if (i <= top_of_zone_table){
	   print_zone_to_buf(buf, i);
	   send_to_char(buf, ch);
	   return;}
	 else {
	   send_to_char("That is not a valid zone.\r\n", ch);
	   return;
	 }
       } else
	 for (i = 0; i <= top_of_zone_table; i++){
	   print_zone_to_buf(buf, i);
	   if (bffzm){
	     RECREATE(bffzm, char, (strlen(bffzm) + strlen(buf) +1));
	     strcat(bffzm,buf);}
	   else{
	     CREATE(bffzm,char,strlen(buf) +1);
	     strcpy(bffzm,buf);}
	 }
       if (bffzm)
	 page_string(ch->desc,bffzm,1);
       free(bffzm);
     }
     else { /* mortal version */
       j = 1;
       sprintf(buf,"Open Zones:\r\n");
       for (i = 0; i <= top_of_zone_table; i++){
	 if (zone_table[i].open) {
	   sprintf(buf,"%s[%2d] %-30.30s%s",buf,j,zone_table[i].name,
		   (!(j%2) ? "\r\n":" "));
	   j++;
	 }
       }
       page_string(ch->desc,buf,1);
     }
     break;
   case 10: /* show rooms */
     ibot = 0;
     itop = top_of_world;
     iloc = -1;
     j = ch->specials2.edit_zone;
     if (*value && is_number(value))
       j = atoi(value);
     else{
       sprintf(buf,"%s %s",value, arg);
       sprintf(arg,"%s",buf);
     }
     if (*arg){
       half_chop(arg, buf1, buf2);
       if (!strcmp("about", buf1)){
	 iloc = atoi(buf2);
	 if (iloc < 0){
	   send_to_char("Illegal room number.\r\n",ch);
	   return;}
       }
       else if (!strcmp("from", buf1)){
	 ibot = atoi(buf2);
	 iloc = -1;
	 if (ibot < 0){
	   send_to_char("Illegal room number.\r\n",ch);
	   return;}
       }	       
     }
     for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
       ;
     if (zone > top_of_zone_table)
       {
	 send_to_char("Illegal zone number.\r\n",ch);
	 return;
       }
     for (i = 0,j=0; world[i].zone != zone && i <= top_of_world; i++)
       j++;
     for (i=j ; world[i].zone == zone && i <= top_of_world; i++)
       ;
     if (iloc >= 0 ){
       if (iloc > i-j)
	 iloc = i-j;
       ibot = MAX(iloc - 10, 0);
       itop = MIN(top_of_world,iloc + 10 + j);}
     else{
       if (ibot > i-j)
	 ibot  = MAX(i-j - 21,0) ;
     }
     *buf2 = '\0';
     for (i=ibot + j ; world[i].zone == zone && i <= itop; i++)
       {
	 if (IS_SET(world[i].room_flags, UNFINISHED))
	   sprintf (buf,"[%3d] Number: %5d  %s*%s%-30.30s \r\n"
		    ,i-j,world[i].number,
		    CCBRED(ch, C_NRM),CCNRM(ch, C_NRM),world[i].name);
	 else
	   sprintf (buf,"[%3d] Number: %5d   %-30.30s \r\n"
		    ,i-j,world[i].number,
		    world[i].name);
	 
	 if (strlen(buf2) + strlen(buf) +1 < MAX_STRING_LENGTH)
	   strcat(buf2,buf);
	 else{
	   send_to_char("Too many rooms to list at once!\r\n",ch);
	   send_to_char("Use the from or about keywords.\r\n",ch);		   
	   break;
	 }
       }
     if (*buf2)
       page_string(ch->desc,buf2,1);
     else
       send_to_char("0 rooms in this zone.\r\n",ch);
     break;
   case 11:
     ibot  = 0;
     itop = top_of_mobt;
     iloc = -1;
     j = ch->specials2.edit_zone;
     if (*value && is_number(value))
       j = atoi(value);
     else{
       sprintf(buf,"%s %s",value, arg);
       sprintf(arg,"%s",buf);
     }
     if (*arg){
       half_chop(arg, buf1, buf2);
       if (!strcmp("about", buf1)){
	 iloc = atoi(buf2);
	 if (iloc < 0){
	   send_to_char("Illegal mob number.\r\n",ch);
	   return;}
       }
       else if (!strcmp("from", buf1)){
	 ibot = atoi(buf2);
	 iloc = -1;
	 if (ibot < 0){
	   send_to_char("Illegal mob number.\r\n",ch);
	   return;}
       }	       
     }
     for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
	   ;
/*       if (zone +1 <= top_of_zone_table)
	   next_zone = zone_table[zone+1].number;
       else 
       next_zone = zone_table[zone].number +10; */
       next_zone = (zone_table[zone].top +1)/100;
       if (zone > top_of_zone_table)
	   {
	       send_to_char("Illegal zone number.\r\n",ch);
	       return;
	   }
       for (i = 0,jj=0; mob_index[i].virtual < j*100 && i <= top_of_mobt; i++)
	   jj++;
       for (i = jj; mob_index[i].virtual < next_zone*100 && i <= top_of_mobt; i++)
	   ;       
       if (iloc >=0){
	   if (iloc > i - jj)
	       iloc = i-jj;
	   ibot = MAX(iloc - 10, 0);
	   itop = MIN(top_of_mobt, iloc + 10 + jj);
       }
       else{
	   if (ibot > i-jj)
	       ibot = MAX(i-j-21,0);
       }
       *buf2 = '\0';
       for (i=ibot +jj
	      ; ((mob_index[i].virtual < next_zone*100) && (i <= itop))
	      ; i++)
	   {
	       sprintf (buf,"[%3d] Number: %5d    %-30.30s  #loaded: %3d\r\n"
			,i,mob_index[i].virtual,
			mob_proto[i].player.short_descr,
			mob_index[i].number);
	       if (strlen(buf2) + strlen(buf) +1 < MAX_STRING_LENGTH)
		   strcat(buf2,buf);
	       else{
		   send_to_char("Too many mobs to list at once!\r\n",ch);
		   send_to_char("Use the from or about keywords.\r\n",ch);
		   break;
	       }
	   }
       if (*buf2)
	 page_string(ch->desc,buf2,1);
       else
	 send_to_char("0 rooms in this zone.\r\n",ch);
       
       break;
   case 12: /* show objects */
     ibot  = 0;
     itop = top_of_objt;
     iloc = -1;
     j = ch->specials2.edit_zone;
     if (*value && is_number(value))
       j = atoi(value);
     else{
       sprintf(buf,"%s %s",value, arg);
       sprintf(arg,"%s",buf);
     }
     if (*arg){
       half_chop(arg, buf1, buf2);
       if (!strcmp("about", buf1)){
	 iloc = atoi(buf2);
	 if (iloc < 0){
	   send_to_char("Illegal object number.\r\n",ch);
	   return;}
       }
       else if (!strcmp("from", buf1)){
	 ibot = atoi(buf2);
	 iloc = -1;
	 if (ibot < 0){
	   send_to_char("Illegal object number.\r\n",ch);
	   return;}
       }	       
     }
     for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
       ;
     next_zone = (zone_table[zone].top +1) / 100;
     if (zone > top_of_zone_table)
       {
	 send_to_char("Illegal zone number.\r\n",ch);
	 return;
       }
     for (jj=0,zol = zone_table[zone].list;zol;zol = zol->next)
       jj++;
     sprintf(buf,"%d objects in this zone.\r\n",jj);
     send_to_char(buf,ch);
     
     if (iloc > 0){
       if (iloc > jj)
	 iloc = jj;
       ibot = MAX( iloc - 10, 0);
       itop = MIN(jj, iloc + 10);
     }
     else
       if (ibot > jj){
	 ibot = MAX(jj - 21, 0);
	 itop = jj;
       }
     *buf2 = 0;
     jj = 0;
     for (zol = zone_table[zone].list;zol && (jj < ibot) ;zol = zol->next)
       jj++;
     for(i=0; zol && ( i <= itop); i++, zol = zol->next){
       {
	 jj++;
	 if (!zol->obj)
	   continue;
	 if (IS_SET(zol->obj->obj_flags.extra_flags, ITEM_GODONLY))
	   sprintf (buf,"[%3d] Number: %5d  %s*%s%-30.30s  #loaded: %3d\r\n"
		    ,i,zol->vnum,
		    CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM),
		    zol->obj->short_description,
		    obj_index[real_object(zol->vnum)].number);
	 else
	   sprintf (buf,"[%3d] Number: %5d   %-30.30s  #loaded: %3d\r\n"
		    ,i,zol->vnum,
		    zol->obj->short_description,
		    obj_index[real_object(zol->vnum)].number);
	 if (strlen(buf2) + strlen(buf) +1 < MAX_STRING_LENGTH)
	   strcat(buf2,buf);
	 else{
	   send_to_char("Too many objects to list at once!\r\n",ch);
	   send_to_char("Use the from or about keywords.\r\n",ch);
	   break;	      
	 }
       }
     }
     if (buf2)
       page_string(ch->desc,buf2,1);
     break;
   case 13:
     if (!*value && ch->specials2.edit_zone != -1)
       j = ch->specials2.edit_zone;
     else if (!is_number(value)){
       send_to_char("Illegal zone number.\r\n",ch);
       return;}
     else
       j = atoi(value);
     for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
       ;
     if (zone > top_of_zone_table)
       {
	 send_to_char("Illegal zone number.\r\n",ch);
	 return;
       }
     for (i = 0,j=0; world[i].zone != zone && i <= top_of_world; i++)
       j++;
     sprintf(buf,"\r\nTeleports for zone:%3d\r\n----------------------\r\n",zone);
     send_to_char(buf,ch);
       for (i=j ; world[i].zone == zone && i <= top_of_world; i++)
	 {
	   if (world[i].tele_delay >= 0){
	     sprintf (buf,"From: %5d  %-22.22s to: %5d  %-22.22s Delay: %3d\r\n"
		      ,world[i].number,
		      world[i].name,world[real_room(world[i].tele_to_room)].number,
		      world[real_room(world[i].tele_to_room)].name,world[i].tele_delay);
	     if (strlen(buf2) + strlen(buf) +1 < MAX_STRING_LENGTH)
	       strcat(buf2,buf);
	     else{
	       send_to_char("Too many teleports to list at once!\r\n",ch);
	       send_to_char("Use the from or about keywords.\r\n",ch);
	       break;	      
	     }	       
	   }
	 }
       if (buf2)
	 page_string(ch->desc,buf2,1);
       break;
   case 14: /* show GOD objects*/
     if (!*value && ch->specials2.edit_zone != -1)
       j = ch->specials2.edit_zone;
     else if (!is_number(value)){
       send_to_char("Illegal zone number.\r\n",ch);
       return;}
     else
       j = atoi(value);
     for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
       ;
     if (zone +1 <= top_of_zone_table)
       next_zone = zone_table[zone+1].number;
     else
       next_zone = zone_table[zone].number +10;
     if (zone > top_of_zone_table)
       {
	 send_to_char("Illegal zone number.\r\n",ch);
	 return;
       }
     for(i = 0; i <= top_of_objt; i++){
       if((obj_index[i].virtual >= j * 100) && (obj_index[i].virtual <  next_zone * 100) && is_goditem(obj_proto + i)){
	 sprintf (buf,"[%3d] Number: %5d    %-30.30s  #loaded: %3d\r\n"
		  ,i,obj_index[i].virtual,
		  obj_proto[i].short_description,
		  obj_index[i].number);
	 if (bffzm){
	   RECREATE(bffzm, char, (strlen(bffzm) + strlen(buf) +1));
	   strcat(bffzm,buf);}
	 else{
	   CREATE(bffzm,char,strlen(buf) +1);
	   strcpy(bffzm,buf);}
       }
     }
     if (bffzm)
       page_string(ch->desc,bffzm,1);
     free(bffzm);
     break;
   case 15: /* show shops */
     if (!*value && ch->specials2.edit_zone != -1)
       j = ch->specials2.edit_zone;
     else if (!is_number(value)) {
       send_to_char("Illegal zone number.\r\n",ch);
       return;
     }
     else
       j = atoi(value);
     *buf2 = '\0';
     for(i=0;i<= number_of_shops;i++) {
       if (shop_index[i].virtual/100 == j) {
	 if (shop_index[i].in_room) {
	   sprintf(buf, "[%3d] Number: %5d  in room: %5d  %-30.30s \r\n"
		   ,i,shop_index[i].virtual,shop_index[i].in_room,
		   ((shop_index[i].keeper > 0) ?
		    mob_proto[shop_index[i].keeper].player.short_descr :
		    "None set"));
	   strcat(buf2,buf);
	 }
	 else {
	   sprintf(buf, "[%3d] Number: %5d  in room: Wander  %-30.30s \r\n"
		   ,i,shop_index[i].virtual,
		   ((shop_index[i].keeper > 0) ?
		    mob_proto[shop_index[i].keeper].player.short_descr :
		    "None set"));
	   strcat(buf2,buf);
	 }
       }
     }
     if (*buf2)
       page_string(ch->desc, buf2,1);
     else
       send_to_char("0 shops in this zone.\r\n",ch);
     break;
   default:
      send_to_char("Sorry, I don't understand that.\r\n", ch);
      break;
   }
}


#define PC   1
#define NPC  2
#define BOTH 3
#define NEITHER 0

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT(flagset, flags); \
	else if (off) REMOVE_BIT(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))

ACMD(do_link_load)
{
    struct char_data *cbuf;
    struct char_file_u tmp_store;
    int player_i, load_result;
    char name[100];

    one_argument(argument,name);

    CREATE(cbuf, struct char_data,1);
    clear_char(cbuf);
    if ((player_i = load_char(name, &tmp_store)) > -1)
	store_to_char(&tmp_store,cbuf);
    else {
	free_char(cbuf);
	send_to_char("There is no such player.\r\n", ch);
	return;	
    }
    reset_char(cbuf);
    cbuf->next = character_list;
    character_list = cbuf;
    load_result = Crash_load(cbuf);
    char_to_room(cbuf,ch->in_room, FALSE);
    sprintf(buf,"%s link loaded by %s. load_result = %d",GET_NAME(cbuf),GET_NAME(ch), load_result);
    mudlog(buf, CMP, LEVEL_IMPL, TRUE);
    act("$n slowly materialises.",FALSE,cbuf,0,0,TO_ROOM);

    
}
ACMD(do_rentchar)
{
    int j;
    struct char_data *vict;
    char name[100];
    void	Crash_extract_objs(struct obj_data *obj);
    one_argument(argument,name);

    if (!(vict = get_char_vis(ch,name))){
	send_to_char("No such player\r\n",ch);
	return;
    }
    else{
	if (GET_LEVEL(vict) > GET_LEVEL(ch)){
	    send_to_char("Uh Uh...\r\n",ch);
	    return;}
	if (vict->in_room != NOWHERE)
	    char_from_room(vict);
	char_to_room(vict,3, FALSE);
	if (vict->desc)
	    close_socket(vict->desc);
	vict->desc = 0;
	save_char(vict,NOWHERE); 
	
	Crash_crashsave(vict);

	for (j = 0; j < MAX_WEAR; j++)
	  if (vict->equipment[j]) 
	    obj_to_char(unequip_char(vict, j), vict, 1);
	Crash_extract_objs(vict->inventory);

	sprintf(buf,"%s force rented by %s.",GET_NAME(vict),GET_NAME(ch));
	mudlog(buf, CMP, LEVEL_IMPL, TRUE);
	extract_char(vict, FALSE);
    }
    
}

ACMD(do_set)
{
   int	i,ii, l, amount;
   struct char_data *vict;
   struct char_data *cbuf;
   struct char_file_u tmp_store;
   char	field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH],
   val_arg[MAX_INPUT_LENGTH], orig_name[MAX_NAME_LENGTH + 10];
   int	on = 0, off = 0, value = 0;
   char	is_file = 0, is_mob = 0, is_player = 0;
   int	player_i = -1;
   char	*pc_races[] = {
       "UNDEFINED",
       "human",
       "elf",
       "halfling",
       "giant",
       "gnome",
       "halfelf",
       "ogier",
       "dwarf",
       "selkie",
       "pixie",
       "amarya",
       "troll",
       "\n"
   };

   struct set_struct {
      char	*cmd;
      int	level;
      char	pcnpc;
      char	type;
   } fields[] = 
    {
      { "brief", 	LEVEL_GOD, 	PC, 	BINARY },
      { "invstart", 	LEVEL_GOD, 	PC, 	BINARY },
      { "title", 	LEVEL_GOD, 	PC, 	MISC },
      { "nosummon", 	LEVEL_GRGOD, 	PC, 	BINARY },
      { "maxhit", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "maxmana", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "maxmove", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "hit", 		LEVEL_ASS_IMPL, BOTH, 	NUMBER },
      { "mana", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "move", 	LEVEL_ASS_IMPL,	BOTH, 	NUMBER },
      { "align", 	LEVEL_GOD, 	BOTH, 	NUMBER },
      { "str", 		LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "stradd", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "int", 		LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "wis", 		LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "dex", 		LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "con", 		LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "sex", 		LEVEL_GRGOD, 	BOTH, 	MISC },
      { "ac", 		LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "gold", 	LEVEL_GOD, 	BOTH, 	NUMBER },
      { "bank", 	LEVEL_GOD, 	PC, 	NUMBER },
      { "exp", 		LEVEL_ASS_IMPL,  BOTH, 	NUMBER },
      { "hitroll", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "damroll", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "invis", 	LEVEL_IMPL, 	PC, 	NUMBER },
      { "nohassle", 	LEVEL_GRGOD, 	PC, 	BINARY },
      { "frozen", 	LEVEL_FREEZE, 	PC, 	BINARY },
      { "practices", 	LEVEL_ASS_IMPL, PC, 	NUMBER },
      { "lessons", 	LEVEL_ASS_IMPL,	PC, 	NUMBER },
      { "drunk", 	LEVEL_GOD, 	BOTH, 	MISC },
      { "hunger", 	LEVEL_GOD, 	BOTH, 	MISC },
      { "thirst", 	LEVEL_GOD, 	BOTH, 	MISC },
      { "killer", 	LEVEL_GOD, 	PC, 	BINARY },
      { "thief", 	LEVEL_GOD, 	PC, 	BINARY },
      { "level", 	LEVEL_ASS_IMPL, BOTH, 	NUMBER },
      { "roomflag", 	LEVEL_GRGOD, 	PC, 	BINARY },
      { "room", 	LEVEL_IMPL, 	BOTH, 	NUMBER },
      { "siteok", 	LEVEL_IMPL, 	PC, 	BINARY },
      { "deleted", 	LEVEL_IMPL, 	PC, 	BINARY },
      { "race", 	LEVEL_IMPL, 	BOTH, 	MISC },
      { "nowizlist", 	LEVEL_GOD, 	PC, 	BINARY },
      { "quest", 	LEVEL_GOD, 	PC, 	BINARY },
      { "loadroom", 	LEVEL_GRGOD, 	PC, 	MISC },
      { "color", 	LEVEL_GOD, 	PC, 	BINARY },
      { "idnum", 	LEVEL_IMPL, 	PC, 	NUMBER },
      { "passwd", 	LEVEL_IMPL, 	PC, 	MISC },
      { "nodelete", 	LEVEL_GOD, 	PC, 	BINARY },
      { "chr",          LEVEL_ASS_IMPL,    BOTH,   NUMBER},
      { "per",          LEVEL_ASS_IMPL,    BOTH,   NUMBER},
      { "gui",          LEVEL_ASS_IMPL,    BOTH,   NUMBER},
      { "luc",          LEVEL_ASS_IMPL,    BOTH,   NUMBER},
      { "maxpower", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "power", 	LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "height",       LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "weight",       LEVEL_GRGOD, 	BOTH, 	NUMBER },
      { "foc",          LEVEL_ASS_IMPL,     BOTH,   NUMBER},
      { "dev",          LEVEL_IMPL,    BOTH,   NUMBER},
      { "zone",         LEVEL_ASS_IMPL, BOTH,   NUMBER},
      { "stopping",     LEVEL_ASS_IMPL, BOTH,   NUMBER},
      { "zone2",        LEVEL_ASS_IMPL, PC,   NUMBER},
      { "zone3",        LEVEL_ASS_IMPL, PC,   NUMBER},
      { "debug",        LEVEL_ASS_IMPL,     PC, BINARY},
      { "idle",         LEVEL_IMPL,     PC, NUMBER},
      { "name",         LEVEL_IMPL,     PC, MISC},
      { "time",         LEVEL_IMPL,     BOTH, NUMBER},
      { "holylight",    LEVEL_IMPL,     PC, BINARY},
      { "\n", 0, NEITHER, MISC }
    };   

    if (!*argument)
      {
	sprintf(buf, "The following set commands are available to you:\r\n");
	buf2[0] = '\0';
	for (ii=0,i=0;*(fields[i].cmd) != '\n';i++)
	  if (fields[i].level <= GET_LEVEL(ch)){
	    sprintf(buf2,"%s%-15s%s",buf2,fields[i].cmd, (!(++ii % 5) ? "\r\n": ""));
	  }
	page_string(ch->desc, buf2, 1);
	return;
      }
    sprintf(buf2,"%s", argument);
    half_chop(argument, name, buf);
    if (!strcmp(name, "file")) {
      is_file = 1;
      half_chop(buf, name, buf);
    } else if (!str_cmp(name, "player")) {
      is_player = 1;
      half_chop(buf, name, buf);
    } else if (!str_cmp(name, "mob")) {
      is_mob = 1;
      half_chop(buf, name, buf);
    }
    
    half_chop(buf, field, buf);
   strcpy(val_arg, buf);

   if (!*name || !*field) {
      send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
      return;
   }
   if (IS_NPC(ch)) {
      send_to_char("None of that!\r\n", ch);
      return;
   }
   for (l = 0; *(fields[l].cmd) != '\n'; l++)
      if (!strncmp(field, fields[l].cmd, strlen(field)))
	 break;

   if (!is_file) {
      if (is_player) {
	 if (!(vict = get_player_vis(ch, name))) {
	    send_to_char("There is no such player.\r\n", ch);
	    return;
	 }
      } else {
	if (!(vict = get_char_vis(ch, name)) && fields[l].pcnpc != NEITHER) {
	  send_to_char("There is no such creature.\r\n", ch);
	  return;
	}
	else if (fields[l].pcnpc != NEITHER)
	  vict == ch;
      }
   }
   else if (is_file) {
     CREATE(cbuf, struct char_data, 1);
      clear_char(cbuf);
      if (is_number(name)){
	  player_i = atoi(name);
	  load_char_bynum(player_i, &tmp_store);
      }
      else
	player_i= load_char(name, &tmp_store);
      if(player_i > -1) {
	store_to_char(&tmp_store, cbuf);
	if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)){
	  free_char(cbuf);
	  send_to_char("Sorry, you can't do that.\r\n", ch);
	  return;
	}
	vict = cbuf;
      }
      else {
	 free(cbuf);
	 send_to_char("There is no such player.\r\n", ch);
	 return;
      }
   }

   if (GET_LEVEL(ch) != LEVEL_IMPL && (fields[l].pcnpc != NEITHER)) {
      if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
	 send_to_char("Maybe that's not such a great idea...\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
   }


   if (GET_LEVEL(ch) < fields[l].level) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      if (is_file)
	free_char(cbuf);
      return;
   }
   if (IS_NPC(vict) && (fields[l].pcnpc != NEITHER)
       && (!fields[l].pcnpc && NPC)) {
      send_to_char("You can't do that to a beast!\r\n", ch);
      if (is_file)
	free_char(cbuf);
      return;
   } else if (!IS_NPC(vict) && (fields[l].pcnpc != NEITHER)
	      && (!fields[l].pcnpc && PC)) {
      send_to_char("That can only be done to a beast!\r\n", ch);
      if (is_file)
	free_char(cbuf);
      return;
   }

   if (fields[l].type == BINARY) {
      if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
	 on = 1;
      else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
	 off = 1;
      if (!(on || off)) {
	 send_to_char("Value must be on or off.\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
   } else if (fields[l].type == NUMBER) {
      value = atoi(val_arg);
   }

   strcpy(buf, "Okay.");
   if (!IS_NPC(vict)){
     sprintf(buf,"(GC) %s set %s.",GET_NAME(ch),buf2);
     mudlog(buf, CMP,GET_LEVEL(ch), TRUE);
   }
   switch (l) {
   case 0:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
      break;
   case 1:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
      break;
   case 2:
      if (GET_TITLE(vict))
	 RECREATE(GET_TITLE(vict), char, strlen(val_arg) + 1);
      else
	 CREATE(GET_TITLE(vict), char, strlen(val_arg) + 1);
      strcpy(GET_TITLE(vict), val_arg);
      parse_text(vict,ch,0,buf2);
      sprintf(buf, "%s is now: %s", GET_NAME(vict), buf2);
      break;
   case 3:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
      on = !on; /* so output will be correct */
      break;
   case 4:
      vict->points.max_hit = RANGE(1, 5000);
      affect_total(vict);
      break;
   case 5:
      break;
   case 6:
      vict->points.max_move = RANGE(1, 5000);
      affect_total(vict);
      break;
   case 7:
      vict->points.hit = RANGE(-9, vict->points.max_hit);
      affect_total(vict);
      break;
   case 8:
      affect_total(vict);
      break;
   case 9:
      vict->points.move = RANGE(0, vict->points.max_move);
      affect_total(vict);
      break;
   case 10:
       GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
      affect_total(vict);
      break;
   case 11:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.str = value;
      vict->abilities.str_add = 0;
      affect_total(vict);
      break;
   case 12:
      vict->abilities.str_add = RANGE(0, 100);
/*      if (value > 0)
	 vict->abilities.str = 18; */
      affect_total(vict);
      break;
   case 13:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.intel = value;
      affect_total(vict);
      break;
   case 14:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.wis = value;
      affect_total(vict);
      break;
   case 15:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.dex = value;
      affect_total(vict);
      break;
   case 16:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.con = value;
      affect_total(vict);
      break;
   case 17:
      if (!str_cmp(val_arg, "male"))
	 vict->player.sex = SEX_MALE;
      else if (!str_cmp(val_arg, "female"))
	 vict->player.sex = SEX_FEMALE;
      else if (!str_cmp(val_arg, "neutral"))
	 vict->player.sex = SEX_NEUTRAL;
      else {
	 send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      break;
   case 18:
      vict->points.armor[0] = RANGE(-100, 100);
      vict->points.armor[1] = RANGE(-100, 100);
      vict->points.armor[2] = RANGE(-100, 100);
      vict->points.armor[3] = RANGE(-100, 100);      
      affect_total(vict);
      break;
   case 19:
     amount = -1*GET_GOLD(vict);
     change_gold(vict, amount);
     drop_excess_gold(vict,RANGE(0, 100000));
     break;
   case 20:
      GET_BANK_GOLD(vict) = RANGE(0, 100000);
      break;
   case 21:
      vict->points.exp = RANGE(0, 500000000);
      break;
   case 22:
      vict->points.hitroll = RANGE(-100, 100);
      affect_total(vict);
      break;
   case 23:
      vict->points.damroll = RANGE(-200, 200);
      affect_total(vict);
      break;
   case 24:
      if (GET_LEVEL(ch) < LEVEL_IMPL && ch != vict) {
	 send_to_char("You aren't godly enough for that!\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
      break;
   case 25:
      if (GET_LEVEL(ch) < LEVEL_IMPL && ch != vict) {
	 send_to_char("You aren't godly enough for that!\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
      break;
   case 26:
      if (ch == vict) {
	 send_to_char("Better not -- could be a long winter!\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
      break;
   case 27:
   case 28:
       SPELLS_TO_LEARN(vict) = value; 
      break;
   case 29:
   case 30:
   case 31:
      if (!str_cmp(val_arg, "off")) {
	 GET_COND(vict, (l - 29)) = (char) -1;
	 sprintf(buf, "%s's %s now off.", GET_NAME(vict),
	     fields[l].cmd);
      } else if (is_number(val_arg)) {
	 value = atoi(val_arg);
	 RANGE(0, 24);
	 GET_COND(vict, (l - 29)) = (char) value;
	 sprintf(buf, "%s's %s set to %d.", GET_NAME(vict),
	     fields[l].cmd, value);
      } else {
	 send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      break;
   case 32:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
      break;
   case 33:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
      break;
   case 34:
      if ((GET_LEVEL(ch) < LEVEL_IMPL || value > GET_LEVEL(ch))) {
	 send_to_char("You cannot raise someone above your own level!\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      if (value > LEVEL_IMPL +1){
	 if (is_file)
	   free_char(cbuf);
	  return;
      }
      vict->player.level = (int) value;
      break;
   case 35:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
      break;
   case 36:
      if ((i = real_room(value)) < 0) {
	 send_to_char("No room exists with that number.\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      char_from_room(vict);
      char_to_room(vict, i, FALSE);
      break;
   case 37:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
      break;
   case 38:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
      break;
   case 39:
       if ((value = search_block(val_arg,pc_races, FALSE)))
	   vict->player.race = value;
       else{
	   send_to_char("Hmm don't know about that race sorry.\r\n",ch);
	 if (is_file)
	   free_char(cbuf);
	   return;
       }
       break;
   case 40:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
      break;
   case 41:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
      break;
   case 42:
      if (!str_cmp(val_arg, "on"))
	 SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
      else if (!str_cmp(val_arg, "off"))
	 REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
      else {
	 if (real_room(i = atoi(val_arg)) > -1) {
	    GET_LOADROOM(vict) = i;
	    sprintf(buf, "%s will enter at %d.", GET_NAME(vict),
	        GET_LOADROOM(vict));
	 } else
	    sprintf(buf, "That room does not exist!");
      }
      break;
   case 43:
      SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
      break;
   case 44:
      if (!IS_NPC(vict))
	 if (is_file)
	   free_char(cbuf);
	 return;
      vict->specials2.idnum = value;
      break;
   case 45:
      if (!is_file)
	 return;
      if (GET_LEVEL(vict) > GET_LEVEL(ch)) {
	 send_to_char("You cannot change that.\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      strncpy(tmp_store.pwd, CRYPT(val_arg, tmp_store.name), MAX_PWD_LENGTH);
      tmp_store.pwd[MAX_PWD_LENGTH] = '\0';
      sprintf(buf, "Password changed to '%s'.", val_arg);
      break;
   case 46:
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
      break;
   case 47:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.chr = value;
      affect_total(vict);
      break;
   case 48:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.per = value;
      affect_total(vict);
      break;
   case 49:
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	 RANGE(3, 30);
      else
	 RANGE(3, 30);
      vict->abilities.gui = value;
      affect_total(vict);
      break;
   case 50:
       if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	   RANGE(3, 30);
       else
	   RANGE(3, 30);
      vict->abilities.luc = value;
      affect_total(vict);
      break;
   case 51:
      break;
   case 52:
      break;
   case 53:
       vict->player.height = RANGE(15,300);
       affect_total(vict);
      break;
   case 54:
       vict->player.weight = RANGE(150,4000);
       affect_total(vict);
      break;      
   case 55:
       if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	   RANGE(3, 30);
       else
	   RANGE(3, 30);
      vict->abilities.foc = value;
      affect_total(vict);
      break;
   case 56:
       if (IS_NPC(vict) || GET_LEVEL(vict) >= LEVEL_GRGOD)
	   RANGE(3, 30);
       else
	   RANGE(3, 30);
      vict->abilities.dev = value;
      affect_total(vict);
      break;
   case 57:
       if (GET_LEVEL(vict) >= LEVEL_BUILDER)
	   RANGE(-1, 999);
       else{
	 if (is_file)
	   free_char(cbuf);
	   return;
       }
      if (IS_SET(ch->specials2.builder_flag,BUILD_ZONE) && ch->specials2.edit_zone > 0){
	  i = save_zone(ch->specials2.edit_zone);
	  sprintf(buf,"(%s) Autosaving %d zone resets for zone: %ld",
		  GET_NAME(ch),i,ch->specials2.edit_zone);
	  mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	  REMOVE_BIT(ch->specials2.builder_flag,BUILD_ZONE);        	  
      }
      if (IS_SET(ch->specials2.builder_flag,BUILD_SHOPS) && ch->specials2.edit_zone > 0){
	  i = save_shops(ch->specials2.edit_zone);
	  sprintf(buf,"(%s) Autosaving %d shops for zone: %ld",
		  GET_NAME(ch),i,ch->specials2.edit_zone);
	  mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	  REMOVE_BIT(ch->specials2.builder_flag,BUILD_SHOPS);        	  
      }
      if (IS_SET(ch->specials2.builder_flag,BUILD_MOBS) && ch->specials2.edit_zone > 0){
	  i = save_mobiles(ch->specials2.edit_zone);
	  sprintf(buf,"(%s) Autosaving %d mobiles for zone: %ld",
		  GET_NAME(ch),i,ch->specials2.edit_zone);
	  mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	  REMOVE_BIT(ch->specials2.builder_flag,BUILD_MOBS);        	  
      }
      if (IS_SET(ch->specials2.builder_flag,BUILD_ROOMS) && ch->specials2.edit_zone > 0){
	  i = save_rooms(ch->specials2.edit_zone);
	  sprintf(buf,"(%s) Autosaving %d rooms for zone: %ld",
		  GET_NAME(ch),i,ch->specials2.edit_zone);
	  mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	  REMOVE_BIT(ch->specials2.builder_flag,BUILD_ROOMS);        	  
      }         
      sprintf(buf,"(%s) Setting zone for %s to: %ld",GET_NAME(ch),
	      GET_NAME(vict), value);
      mudlog(buf,  NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
      vict->specials2.edit_zone = value; 
      break;
   case 58:
      vict->points.stopping[0] = RANGE(0, 20);
      vict->points.stopping[1] = RANGE(0, 20);
      vict->points.stopping[2] = RANGE(0, 20);
      vict->points.stopping[3] = RANGE(0, 20);      
      affect_total(vict);
      break;
   case 59:
       if (GET_LEVEL(vict) >= LEVEL_BUILDER)
	   RANGE(-1, 999);
       else{
	 if (is_file)
	   free_char(cbuf);	 
	   return;
       }
      sprintf(buf,"(%s) Setting zone2 for %s to: %ld",GET_NAME(ch),
	      GET_NAME(vict), value);
      mudlog(buf,  NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);       
      vict->specials2.edit_zone2 = value;
      break;
   case 60:
       if (GET_LEVEL(vict) >= LEVEL_BUILDER)
	   RANGE(-1, 999);
       else{
	 if (is_file)
	   free_char(cbuf);	 
	   return;
       }
      sprintf(buf,"(%s) Setting zone3 for %s to: %ld",GET_NAME(ch),
	      GET_NAME(vict), value);
      mudlog(buf,  NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);       
      vict->specials2.edit_zone3 = value;
      break;
   case 61:
       SET_OR_REMOVE(PRF_FLAGS(vict), PRF_DEBUG);
      break;
   case 62:
       vict->specials.timer = value;
      break;
   case 63:
      if (GET_LEVEL(ch) < LEVEL_IMPL) {
	 send_to_char("You cannot change that.\r\n", ch);
	 if (is_file)
	   free_char(cbuf);
	 return;
      }
      strcpy(orig_name, vict->player.name);
      for (i=0;i <= top_of_p_table;i++)
	  if (!str_cmp((player_table + i)->name, orig_name))
	      break;
      RECREATE((player_table + i)->name, char, strlen(val_arg) +1);
      RECREATE(vict->player.name, char, strlen(val_arg) +1);
      strcpy(vict->player.name, val_arg);
      strcpy(orig_name, val_arg);
      for (ii=0;(*(val_arg +ii) = LOWER(*(orig_name +ii))); ii++)
	  ;
      strcpy((player_table +i)->name, val_arg);
      sprintf(buf, "Name changed to '%s'.", val_arg);
      break;
   case 64:
     if (value > 23 || value < 0) {
       send_to_char("Time out of range...\r\n",ch);
	 if (is_file)
	   free_char(cbuf);
       return;
     }
     time_info.hours = value;
     break;
   case 65:
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_HOLYLIGHT);
      break;
   default:
      sprintf(buf, "Can't set that!");
      break;
   }

   if (fields[l].type == BINARY) {
      sprintf(buf, "%s %s for %s.\r\n", fields[l].cmd, ONOFF(on),
          GET_NAME(vict));
      CAP(buf);
   } else if (fields[l].type == NUMBER) {
      sprintf(buf, "%s's %s set to %d.\r\n", GET_NAME(vict),
          fields[l].cmd, value);
   } else
      strcat(buf, "\r\n");
   send_to_char(buf, ch);

   if (!is_file && !IS_NPC(vict) && (fields[l].pcnpc != NEITHER))
      save_char(vict, NOWHERE);

   if (is_file) {
      char_to_store(vict, &tmp_store);
      fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
      fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
      free_char(cbuf);
      send_to_char("Saved in file.\r\n", ch);
   }
   if((GET_LEVEL(ch) < LEVEL_IMPL)){
      sprintf(buf,"(GC) %s set %s",GET_NAME(ch), argument);
      mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
   }
}





















