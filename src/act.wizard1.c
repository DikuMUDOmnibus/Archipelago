/* ************************************************************************
*   File: act.wizard1.c                                 Part of CircleMUD *
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
#include <errno.h>

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
extern FILE *player_fl;
extern struct shop_data *shop_index;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern struct index_data *obj_index;
extern struct int_app_type int_app[36];
extern struct wis_app_type wis_app[36];
extern struct list_index_type npc_races[];
extern struct zone_data *zone_table;
extern struct player_index_element *player_table;
extern struct obj_limit_list_type *obj_limit_table;
extern struct obj_data *obj_proto;
extern int	top_of_zone_table;
extern int	restrict;
extern int	top_of_world;
extern int	top_of_mobt;
extern int	top_of_objt;
extern int	top_of_p_table;
extern int      top_of_ol_table;
extern struct str_app_type str_app[];
/* for objects */
extern char	*item_types[];
extern char	*wear_bits[];
extern char	*extra_bits[];
extern char	*drinks[];
extern char     *sizes[];
extern char     *damage_state[];
/* for rooms */
extern char	*dirs[];
extern char	*room_bits[];
extern char	*exit_bits[];
extern char	*sector_types[];

/* for chars */
extern char	*spells[];
extern char	*skills_list[];
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
extern struct dual_list_type attack_hit_text[];
extern struct list_index_type TechForms[];
/* external functs */
void	reset_zone(int zone);
char *find_spell_name(int spl);
char *find_skill_name(int spl);
char    *rev_search_list(int num, struct list_index_type *list);
void zone_save_equipment(int zone,struct char_data *ch,struct char_data *victm);
void    zone_save_put_obj(int zone,struct char_data *ch, struct obj_data *obj);
ACMD(do_look);
void    parse_text(struct char_data *c,struct char_data *vict,int mode,char *text);
void    print_obj(struct descriptor_data *d);
void    print_mob(struct descriptor_data *d);
void    print_room(struct descriptor_data *d);
void    print_shop(struct descriptor_data *d);
void    save_all_object_limit_data(void);
int     save_zone(int zone);
int	save_mobiles(int zone);
int	save_objects(int zone);
int	save_shops(int zone);
int	save_rooms(int zone);
int     create_zreset(int zn);
int     destroy_zreset(int zn, int no);
int     create_zone(int zone);
int     create_mob(int zone);
int     create_obj(int zone);
int     create_room(int zone);
int     create_shop(int zone);
void    make_dummy_mob(int i);
void    make_dummy_obj(int i);
void    make_dummy_room(int i);
void    clone_mob(int from, int to);
void    clone_room(int from, int to);
void    copy_room(struct room_data *from, struct room_data *to);
void    copy_shop(struct shop_data *from, struct shop_data *to);
void    copy_mob(struct char_data *from, struct char_data *to);
void    copy_obj(struct obj_data *from, struct obj_data *to, bool copy_extra);
void    do_stat_shop(struct char_data *ch,   struct shop_data *shop);

ACMD(do_emote)
{
   int	i;

   if (IS_NPC(ch))
      return;

   for (i = 0; *(argument + i) == ' '; i++)
      ;

   if (!*(argument + i))
      send_to_char("Yes.. But what?\r\n", ch);
   else {
      sprintf(buf, "$n %s", argument + i);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	 send_to_char("Ok.\r\n", ch);
      else
	 act(buf, FALSE, ch, 0, 0, TO_CHAR);
   }
}


ACMD(do_send)
{
   struct char_data *vict;

   half_chop(argument, arg, buf);

   if (!*arg) {
      send_to_char("Send what to who?\r\n", ch);
      return;
   }

   if (!(vict = get_char_vis(ch, arg))) {
      send_to_char("No such person around.\r\n", ch);
      return;
   }

   send_to_char(buf, vict);
   send_to_char("\r\n", vict);
   if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char("Sent.\r\n", ch);
   else {
      sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
      send_to_char(buf2, ch);
   }
}



ACMD(do_echo)
{
   int	i;

   if (IS_NPC(ch))
      return;

   for (i = 0; *(argument + i) == ' '; i++)
      ;

   if (!*(argument + i))
      send_to_char("That must be a mistake...\r\n", ch);
   else {
      sprintf(buf, "%s\r\n", argument + i);
      send_to_room_except(buf, ch->in_room, ch, FALSE);
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


/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
short find_target_room(struct char_data *ch, char *rawroomstr)
{
   int	tmp;
   short location;
   struct char_data *target_mob;
   struct obj_data *target_obj;
   char	roomstr[MAX_INPUT_LENGTH];

   one_argument(rawroomstr, roomstr);

   if (!*roomstr) {
      send_to_char("You must supply a room number or name.\r\n", ch);
      return NOWHERE;
   }

   if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
      tmp = atoi(roomstr);
      if ((location = real_room(tmp)) < 0) {
	 send_to_char("No room exists with that number.\r\n", ch);
	 return NOWHERE;
      }
   } else if ((target_mob = get_char_vis(ch, roomstr)))
      location = target_mob->in_room;
   else if ((target_obj = get_obj_vis(ch, roomstr))) {
      if (target_obj->in_room != NOWHERE)
	 location = target_obj->in_room;
      else {
	 send_to_char("That object is not available.\r\n", ch);
	 return NOWHERE;
      }
   } else {
      send_to_char("No such creature or object around.\r\n", ch);
      return NOWHERE;
   }

   /* a location has been found. */
   if (IS_SET(world[location].room_flags, GODROOM) && GET_LEVEL(ch) < LEVEL_GRGOD) {
      send_to_char("You are not godly enough to use that room!\r\n", ch);
      return NOWHERE;
   }
   if (IS_SET(world[location].room_flags, UNFINISHED) &&
       (GET_LEVEL(ch) < LEVEL_BUILDER)){
       send_to_char("That room is not ready yet.\r\n", ch);
       return NOWHERE;
   }
   if (!zone_table[world[location].zone].open  &&
       (GET_LEVEL(ch) < LEVEL_BUILDER)){
       send_to_char("That zone is closed.\r\n", ch);
       return NOWHERE;
   }
   if (IS_SET(world[location].room_flags, PRIVATE) && GET_LEVEL(ch) < LEVEL_GRGOD)
      if (world[location].people && world[location].people->next_in_room) {
	 send_to_char("There's a private conversation going on in that room.\r\n", ch);
	 return NOWHERE;
      }

   return location;
}



ACMD(do_at)
{
   char	command[MAX_INPUT_LENGTH];
   int	location, original_loc;

   if (IS_NPC(ch))
      return;

   half_chop(argument, buf, command);
   if (!*buf) {
      send_to_char("You must supply a room number or a name.\r\n", ch);
      return;
   }

   if ((location = find_target_room(ch, buf)) < 0)
      return;

   /* a location has been found. */
   original_loc = ch->in_room;
   char_from_room(ch);
   char_to_room(ch, location, FALSE);
   command_interpreter(ch, command);

   /* check if the guy's still there */
   if (ch->in_room == location) {
      char_from_room(ch);
      char_to_room(ch, original_loc, FALSE);
   }
}


ACMD(do_goto)
{
   short location;
   struct char_data *tch;

   if (IS_NPC(ch))
      return;
   if ((GET_LEVEL(ch) < LEVEL_BUILDER) && ch->specials.fighting){
     send_to_char("No Way!  You are fighting for your life!\r\n",ch);
     return;
   }
   if ((location = find_target_room(ch, argument)) < 0)
      return;
   for (tch=world[ch->in_room].people;tch;tch = tch->next_in_room){
     parse_text(ch,tch,2,buf2);
     sprintf(buf, "%s", buf2);
     if (!(GET_INVIS_LEV(ch) > GET_LEVEL(tch) && GET_LEVEL(ch) >= LEVEL_ASS_IMPL))
       act(buf, TRUE, ch, 0, tch, TO_VICT);}
   char_from_room(ch);
   char_to_room(ch, location, FALSE);
   
   for (tch=world[location].people;tch;tch = tch->next_in_room){
     parse_text(ch,tch,1,buf2);     
     sprintf(buf, "%s", buf2);
     if (!(GET_INVIS_LEV(ch) > GET_LEVEL(tch) && GET_LEVEL(ch) >= LEVEL_ASS_IMPL))
       act(buf, TRUE, ch, 0, tch, TO_VICT);}
   
   do_look(ch, "", 0, 0);
}



ACMD(do_trans)
{
   struct descriptor_data *i;
   struct char_data *victim;

   if (IS_NPC(ch))
      return;

   one_argument(argument, buf);
   if (!*buf)
      send_to_char("Whom do you wish to transfer?\r\n", ch);
   else if (str_cmp("all", buf)) {
      if (!(victim = get_char_vis(ch, buf)))
	 send_to_char("No-one by that name around.\r\n", ch);
      else if (victim == ch)
	 send_to_char("That doesn't make much sense, does it?\r\n", ch);
      else {
	 if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	    send_to_char("Go transfer someone your own size.\r\n", ch);
	    return;
	 }
	 act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	 char_from_room(victim);
	 char_to_room(victim, ch->in_room, FALSE);
	 act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	 act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	 do_look(victim, "", 0, 0);
         sprintf(buf, "(GC) %s transed %s to %d",GET_NAME(ch),GET_NAME(victim), world[ch->in_room].number);
         if (GET_LEVEL(ch) < LEVEL_IMPL && (GET_LEVEL(victim) < LEVEL_BUILDER) )
            mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)+1), TRUE);
      }
   } else { /* Trans All */
      for (i = descriptor_list; i; i = i->next)
	 if (!i->connected && i->character && i->character != ch) {
	    victim = i->character;
            if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
                send_to_char("Go transfer someone your own size.\r\n", ch);
            }
            else {
	        act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	        char_from_room(victim);
	        char_to_room(victim, ch->in_room, FALSE);
	        act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	        act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	        do_look(victim, "", 0, 0);
                sprintf(buf, "(GC) %s transed all to %d",GET_NAME(ch),world[ch->in_room].number);
                mudlog(buf, CMP, MAX(LEVEL_GRGOD, GET_LEVEL(ch)), TRUE);
            }
	 }

      send_to_char("Ok.\r\n", ch);
   }
}



ACMD(do_teleport)
{
   struct char_data *victim;
   short target;

   if (IS_NPC(ch))
      return;

   half_chop(argument, buf, buf2);

   if (!*buf)
      send_to_char("Who do you wish to teleport?\r\n", ch);
   else if (!(victim = get_char_vis(ch, buf)))
      send_to_char("No-one by that name around.\r\n", ch);
   else if (victim == ch)
      send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
   else if (!*buf2)
      send_to_char("Where do you wish to send this person?\r\n", ch);
   else if ((target = find_target_room(ch, buf2)) >= 0) {
       if (GET_LEVEL(victim) > GET_LEVEL(ch) && !IS_NPC(victim)){
	   send_to_char("Go teleport someone your sown size.\r\n",ch);
	   return;
       }
       act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
       char_from_room(victim);
       char_to_room(victim, target, FALSE);
       act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
       act("$n has teleported you!", FALSE, ch, 0, (char *)victim, TO_VICT);
       do_look(victim, "", 0, 0);
   }
}
ACMD(do_savezone)
{
    int zone,saved;
    
    if (ch->specials2.edit_zone <0 ){
	send_to_char("You are not authorised to edit any zones.\r\n",ch);
	return;}
    
    zone= ch->specials2.edit_zone;
    
    saved = save_zone(zone);
    sprintf(buf,"%s stored %d zone resets for zone %d.",
	    GET_NAME(ch),saved,zone);
    mudlog(buf , CMP, LEVEL_BUILDER, TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_ZONE);    

}
ACMD(do_savemobs)
{
    int zone,saved;
    
    if (ch->specials2.edit_zone <0 ){
	send_to_char("You are not authorised to edit any zones.\r\n",ch);
	return;}
    
    zone= ch->specials2.edit_zone;

    saved = save_mobiles(zone);
    sprintf(buf,"%s stored %d mobiles for zone %d.",
	    GET_NAME(ch),saved,zone);
    mudlog(buf , CMP, LEVEL_BUILDER, TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_MOBS);
}
void saveshops(struct char_data *ch)
{
    int zone,saved;
    
    if (ch->specials2.edit_zone <0 ){
	send_to_char("You are not authorised to edit any zones.\r\n",ch);
	return;}
    
    zone= ch->specials2.edit_zone;

    saved = save_shops(zone);
    sprintf(buf,"%s stored %d shops for zone %d.",
	    GET_NAME(ch),saved,zone);
    mudlog(buf , CMP, LEVEL_IMPL, TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_SHOPS);        
}

ACMD(do_saveobjs)
{
    int zone,saved;
    
    if (ch->specials2.edit_zone <0 ){
	send_to_char("You are not authorised to edit any zones.\r\n",ch);
	return;}
    
    zone= ch->specials2.edit_zone;

    saved = save_objects(zone);
    sprintf(buf,"%s stored %d objects for zone %d.",
	    GET_NAME(ch),saved,zone);
    mudlog(buf , CMP, LEVEL_BUILDER, TRUE);

}
ACMD(do_saverooms)
{
    int zone,saved;
    
    if (ch->specials2.edit_zone <0 ){
	send_to_char("You are not authorised to edit any zones.\r\n",ch);
	return;}
    
    zone= ch->specials2.edit_zone;

    saved = save_rooms(zone);
    sprintf(buf,"%s stored %d rooms for zone %d.",
	    GET_NAME(ch),saved,zone);
    mudlog(buf , CMP, LEVEL_BUILDER, TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_ROOMS);        
}

ACMD(do_zpurge)
{
    char value[100];
    char buffer[100];
    int zn,cmd_no,no;
    bool tidyup=FALSE;
    
    one_argument(argument, value);
    
    if (ch->specials2.edit_zone <0 ){
	send_to_char("You are not authorised to edit any zones.\r\n",ch);
	return;}

    for (zn = 0; zone_table[zn].number != ch->specials2.edit_zone &&
		       zn <= top_of_zone_table; zn++)
	if (zn > top_of_zone_table)
	    {
	    send_to_char("Zone does not exist, ask an imp to create it.\r\n",ch);
	    return;
	    }
    if (!is_number(value)){
	send_to_char("What sort of a zone reset number is that? - do show zone.\r\n",ch);
	return;}
    no = atoi(value);
    for (cmd_no=0;;cmd_no++)
	if (zone_table[zn].cmd[cmd_no].command == 'S')
	    break;
    if (no > cmd_no || no < 0){
	send_to_char("What sort of a zone reset number is that? - do show zone.\r\n",ch);
	return;}
    if (zone_table[zn].cmd[no].command == 'm' || zone_table[zn].cmd[no].command == 'o' )
	tidyup = TRUE;
    sprintf(buffer,"%s destroying zreset %d in zone %ld.",GET_NAME(ch),no,ch->specials2.edit_zone);
    mudlog(buffer, CMP, LEVEL_BUILDER, TRUE);
    destroy_zreset(zn,no);
    no++;
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
	SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
    if (tidyup)
	while(zone_table[zn].cmd[no].command != 'S'){
	    if (zone_table[zn].cmd[no].command == 'm'
		|| zone_table[zn].cmd[no].command == 'o'
		|| zone_table[zn].cmd[no].command == 'd'
		|| zone_table[zn].cmd[no].command == 'S')
		break;
	    else{
		destroy_zreset(zn,no);
		no++;
	    }
	}
}
ACMD(do_zset)
{
    char num[100],amount[100];
    int zn,cmd_no,no,val, next_zone, zone_top;

    half_chop(argument, buf, buf2);
    one_argument(buf, num);
    half_chop(buf2, buf1, amount);

    if (!ch->desc)
	return;
    if (!*buf || !*buf1 || (!is_abbrev(buf1, "if") && !is_abbrev(buf1, "number") && !is_abbrev(buf1, "percent"))){
      send_to_char("Usage: zset reset# {if | [number | percent] value }\r\n",ch);
      return;
    }
    if (ch->specials2.edit_zone <0 ){
	send_to_char("You are not authorised to edit any zones.\r\n",ch);
	return;}

    for (zn = 0; zone_table[zn].number != ch->specials2.edit_zone &&
		       zn <= top_of_zone_table; zn++)
	if (zn > top_of_zone_table){
	  send_to_char("Zone does not exist, ask an imp to create it.\r\n",ch);
	  return;
	}
    if (!is_number(num)){
	send_to_char("What sort of a zone reset number is that?\r\n",ch);
      return;
    }
    no = atoi(num);
    for (cmd_no=0;;cmd_no++)
      if (zone_table[zn].cmd[cmd_no].command == 'S')
	break;
    cmd_no--;
    if (no > cmd_no || no < 0){
      send_to_char("What sort of a zone reset number is that? - do show zone.\r\n",ch);
      return;
    }
    if(is_abbrev(buf1, "if")){
      if (zone_table[zn].cmd[no].if_flag == 1){
	sprintf(buf,"Zone reset #%d will now always repop.\r\n",no);
	send_to_char(buf,ch);
	zone_table[zn].cmd[no].if_flag = 0;
      }
      else{
	sprintf(buf,"Zone reset #%d will now repop only if preceeding reset repops.\r\n",no);
	send_to_char(buf,ch);
	zone_table[zn].cmd[no].if_flag = 1;
      }
      if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
	SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
      return;
    }
    else if (is_abbrev(buf1,"number")){
      if (zone_table[zn].cmd[no].command != 'S'){
	val = atoi(amount);
	if (val < 0 || val > 40){
	  send_to_char("What sort of a number is that?\r\n",ch);
	  return;
	}
	if (zone_table[zn].cmd[no].command != 'd'
	    || zone_table[zn].cmd[no].command != 'S'
	    || zone_table[zn].cmd[no].command != '*')
	  zone_table[zn].cmd[no].arg2 = val;
	sprintf(buf,"Setting the max loadable for zonerest #%d to %d.\r\n",no,val);
	send_to_char(buf,ch);
	
	if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
	  SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
      }    
    }
    else if (is_abbrev(buf1, "percent")){
      if (zone_table[zn].cmd[no].command != 'S'){
	val = atoi(amount);
	if (val < 0 || val > 100){
	  send_to_char("What sort of a percentage is that?\r\n",ch);
	  return;
	}
	zone_table[zn].cmd[no].percent = val;
	sprintf(buf,"Setting the load percent for zonerest #%d to %d%%.\r\n",no,val);
	send_to_char(buf,ch);
	
	if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
	  SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
      }    
    }
}
ACMD(do_zsave)
{
  int zone,comm,dirn,state;
  struct char_data *victm;
  struct follow_type *f, *ff;
  struct obj_data *victo;
  static char	*keywords[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n"};
  
  half_chop(argument,buf,buf2);
  
  if (!*buf || !*buf2){
    send_to_char("Usage: zsave [mob|obj name | door direction].\r\n",ch);
    return;}
  if (ch->specials2.edit_zone <0 ){
    send_to_char("You are not authorised to edit any zones.\r\n",ch);
    return;}
  for (zone = 0; zone_table[zone].number != ch->specials2.edit_zone &&
	 zone <= top_of_zone_table; zone++)
    if (zone > top_of_zone_table)
      {
	send_to_char("Zone does not exist, ask an imp to create it.\r\n",ch);
	return;
      }
  
  if (world[ch->in_room].zone != zone){
    send_to_char("You are not authorised to edit this zone.\r\n",ch);
    return;}
  if (!strcmp("mob",buf)){
    if (!(victm = get_char_vis(ch,buf2)))
      {
	send_to_char("No such mobile about.\r\n",ch);
	return;
      }
    
    comm = create_zreset(zone);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
      SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);	
    sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);
    send_to_char(buf,ch);
    zone_table[zone].cmd[comm].command = 'm';
    zone_table[zone].cmd[comm].if_flag = 0;
    zone_table[zone].cmd[comm].arg1    = victm->nr;
    zone_table[zone].cmd[comm].arg2    = 1;        
    zone_table[zone].cmd[comm].arg3    = ch->in_room;
    zone_table[zone].cmd[comm].percent = 100;
    zone_table[zone].cmd[comm].num     = 0;            
    zone_save_equipment(zone,ch,victm);
    for (f = victm->followers; f; f = f->next)
      if (IS_MOB(f->follower))
	{
	  comm = create_zreset(zone);
	  sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);
	  if (f->follower->specials.rider &&
	      (f->follower->specials.rider == victm))
	    zone_table[zone].cmd[comm].command = 'r';
	  else if (f->follower->specials.carried_by &&
		   (f->follower->specials.carried_by == victm))
	    zone_table[zone].cmd[comm].command = 'c';
	  else
	    zone_table[zone].cmd[comm].command = 'f';
	  zone_table[zone].cmd[comm].if_flag = 1;
	  zone_table[zone].cmd[comm].arg1    = f->follower->nr;
	  zone_table[zone].cmd[comm].arg2    = 1;
	  zone_table[zone].cmd[comm].percent = 100;
	  zone_table[zone].cmd[comm].num     = 0;            
	  zone_save_equipment(zone,ch,f->follower);
	}
  }
  else if (!strcmp("obj",buf)){
    if (!(victo = get_obj_vis(ch,buf2)))
      {
	send_to_char("No such object about.\r\n",ch);
	return;
      }
    comm = create_zreset(zone);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
      SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
    sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);
    send_to_char(buf,ch);
    zone_table[zone].cmd[comm].command = 'o';
    zone_table[zone].cmd[comm].if_flag = 0;
    zone_table[zone].cmd[comm].arg1    = victo->item_number;
    zone_table[zone].cmd[comm].arg2    = 1;
    zone_table[zone].cmd[comm].arg3    = ch->in_room;
    zone_table[zone].cmd[comm].percent = 100;
    zone_table[zone].cmd[comm].num     = 0;                
    if (victo->contains)
      zone_save_put_obj(zone,ch,victo->contains);
    if (victo->pulled_by){
      for (f = victo->pulled_by; f; f = f->next){
	comm = create_zreset(zone);
	sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);    
	zone_table[zone].cmd[comm].command = 'h';
	zone_table[zone].cmd[comm].if_flag = 1;
	zone_table[zone].cmd[comm].arg1    = f->follower->nr;
	zone_table[zone].cmd[comm].arg2    = 1;
	zone_table[zone].cmd[comm].percent = 100;
	zone_table[zone].cmd[comm].num     = 0;            	  
	zone_save_equipment(zone,ch,f->follower);
	if (f->follower->followers){
	  for (ff = f->follower->followers; ff;ff = ff->next)
	    if(IS_MOB(ff->follower)) {
	      comm = create_zreset(zone);
	      sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);    
	      zone_table[zone].cmd[comm].command = 'f';
	      zone_table[zone].cmd[comm].if_flag = 1;
	      zone_table[zone].cmd[comm].arg1    = ff->follower->nr;
	      zone_table[zone].cmd[comm].arg2    = 1;
	      zone_table[zone].cmd[comm].percent = 100;
	      zone_table[zone].cmd[comm].num     = 0;            	  
	      zone_save_equipment(zone,ch,ff->follower);
	    }
	}
      }
    }
  }
  else if (!strcmp("door", buf))
    {
      dirn = search_block(buf2,keywords,FALSE);
      if (dirn == -1){
	send_to_char("What sort of direction is that?\r\n",ch);
	return;}
      if (!EXIT(ch,dirn)){
	send_to_char("No exit in that direction.\r\n",ch);
	return;}
      else if (!IS_SET(EXIT(ch, dirn)->exit_info, EX_ISDOOR)){
	send_to_char("No door in that direction.\r\n",ch);
	return;}
      
      if (IS_SET(EXIT(ch, dirn)->exit_info, EX_CLOSED) &&
	  !IS_SET(EXIT(ch, dirn)->exit_info, EX_LOCKED) &&
	  !IS_SET(EXIT(ch, dirn)->exit_info, EX_SECRET))
	state = 1;
      else if (IS_SET(EXIT(ch, dirn)->exit_info, EX_CLOSED) &&
	       IS_SET(EXIT(ch, dirn)->exit_info, EX_LOCKED) &&
	       !IS_SET(EXIT(ch, dirn)->exit_info, EX_SECRET))
	state = 2;
      else if (IS_SET(EXIT(ch, dirn)->exit_info, EX_CLOSED) &&
	       !IS_SET(EXIT(ch, dirn)->exit_info, EX_LOCKED) &&
	       IS_SET(EXIT(ch, dirn)->exit_info, EX_SECRET))
	state = 3;
      else if (IS_SET(EXIT(ch, dirn)->exit_info, EX_CLOSED) &&
	       IS_SET(EXIT(ch, dirn)->exit_info, EX_LOCKED) &&
	       IS_SET(EXIT(ch, dirn)->exit_info, EX_SECRET))
	state = 4;
      else
	state = 0;
      
      comm = create_zreset(zone);
      if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
	SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
      sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);
      send_to_char(buf,ch);
      send_to_char("Now check the other side of the door.\r\n",ch);
      zone_table[zone].cmd[comm].command = 'd';
      zone_table[zone].cmd[comm].if_flag = 0;
      zone_table[zone].cmd[comm].arg1    = ch->in_room;
      zone_table[zone].cmd[comm].arg2    = dirn;
      zone_table[zone].cmd[comm].arg3    = state;
      zone_table[zone].cmd[comm].percent = 100;
    }
  
}
void zone_save_equipment(int zone,struct char_data *ch,struct char_data *victm)
{
    int j, comm;
    struct obj_data *tmp;
    
    for (j=0;j < MAX_WEAR; j++)
	if (victm->equipment[j])
	    {
		comm = create_zreset(zone);
		if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
		    SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
		sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);
		send_to_char(buf,ch);
		zone_table[zone].cmd[comm].command = 'e';
		zone_table[zone].cmd[comm].if_flag = 1;
		zone_table[zone].cmd[comm].arg1    = victm->equipment[j]->item_number;
		zone_table[zone].cmd[comm].arg2    = 20;        
		zone_table[zone].cmd[comm].arg3    = j;
		zone_table[zone].cmd[comm].num     = 0;                  
		zone_table[zone].cmd[comm].percent    = 100;                 
		if (victm->equipment[j]->contains)
		    zone_save_put_obj(zone,ch,victm->equipment[j]->contains);
	    }
    
    for (tmp= victm->inventory;tmp;tmp = tmp->next_content)
	{
	    comm = create_zreset(zone);
	    if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
		SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
	    sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);
	    send_to_char(buf,ch);
	    zone_table[zone].cmd[comm].command = 'g';
	    zone_table[zone].cmd[comm].if_flag = 1;
	    zone_table[zone].cmd[comm].arg1    = tmp->item_number;
	    zone_table[zone].cmd[comm].arg2    = 20;
	    zone_table[zone].cmd[comm].num     = 0;                  
	    zone_table[zone].cmd[comm].percent = 100;         	    
	    
	    /* ok now object contents */
	    if (tmp->contains)
		zone_save_put_obj(zone,ch,tmp->contains);
	}
}
void zone_save_put_obj(int zone,struct char_data *ch, struct obj_data *obj)
{
    struct obj_data *tmp;
    int comm;
    
    if (!obj)
	return;
    for (tmp = obj;tmp;tmp = tmp->next_content){
	comm = create_zreset(zone);
	if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
	    SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
	sprintf(buf,"Creating zone reset #%d in Zone: %d.\r\n",comm,zone);
	send_to_char(buf,ch);
	zone_table[zone].cmd[comm].command = 'p';
	zone_table[zone].cmd[comm].if_flag = 1;
	zone_table[zone].cmd[comm].arg1    = tmp->item_number;
	zone_table[zone].cmd[comm].arg2    = 1;
	zone_table[zone].cmd[comm].arg3   = tmp->in_obj->item_number;
	zone_table[zone].cmd[comm].num     = 0;                  
	zone_table[zone].cmd[comm].percent = 100;         	    	
	if (tmp->contains)
	    zone_save_put_obj(zone,ch,tmp->contains);
    }
}

ACMD(do_ocreate)
{
    int zone,num;
    
    
    if (ch->specials2.edit_zone < 0){
	send_to_char ("You are not authorised to edit any zone..\r\n",ch);
	return;}

    zone = ch->specials2.edit_zone;

    num = create_obj(zone);
    if (num == 0){
	sprintf(buf,"No slots available in zone: %d. Object not created\r\n",zone);
	send_to_char(buf,ch);
	return;}
    else
	sprintf(buf,"Object %d created.\r\n",obj_index[num].virtual);
    send_to_char(buf,ch);
    sprintf(buf,"Object %d created by %s.",obj_index[num].virtual,GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
}
ACMD(do_screate)
{
  int num;
  if (GET_LEVEL(ch) < LEVEL_ASS_IMPL)
    return;
  if (ch->specials2.edit_zone < 0){
    send_to_char ("You are not authorised to edit any zone..\r\n",ch);
    return;}

  num = create_shop(ch->specials2.edit_zone);
  sprintf(buf,"Shop #%d created\r\nSaving shops in zone: %d\r\n",
	  num,ch->specials2.edit_zone);
  send_to_char (buf,ch);
  save_shops(ch->specials2.edit_zone);
}

ACMD(do_rcreate)
{
    int zone,num;
    
    if (ch->specials2.edit_zone < 0){
	send_to_char ("You are not authorised to edit any zone..\r\n",ch);
	return;}

    zone = ch->specials2.edit_zone;

    num = create_room(zone);
    if (num == 0){
	sprintf(buf,"No slots available in zone: %d. room not created\r\n",zone);
	send_to_char(buf,ch);
	return;}
    else
	sprintf(buf,"room %d created.\r\n",world[num].number);
    send_to_char(buf,ch);
    sprintf(buf,"room %d created by %s.",world[num].number,GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ROOMS))
	SET_BIT(ch->specials2.builder_flag,BUILD_ROOMS);
}
ACMD(do_zcreate)
{
  int zone,num, zn;

  if (GET_LEVEL(ch) < LEVEL_IMPL){
    send_to_char ("You are not authorised to create zones..\r\n",ch);
    return;}
  if (!*argument){
    send_to_char ("Usage: zcreate zone_number.\r\n",ch);
    return;
  }
  one_argument(argument,buf);
  if (!is_number(buf)){
    send_to_char("What sort of a zone number is that?\r\n",ch);
    return;
  }
  zone = atoi(buf);
  if (zone <= 0){
    send_to_char("What sort of a zone number is that?\r\n",ch);
    return;
  }
  for (zn = 0;zn <= top_of_zone_table && zone_table[zn].number < zone;zn++)
    ;
  if (zn <= top_of_zone_table){
    if (zone_table[zn-1].top > (zone*100 -1)){
      sprintf(buf,"Zone: %d would overlap Zone: %d\r\n",zone, zone_table[zn-1].number);    
      sprintf(buf,"%sZone not created\r\n", buf,zone);
      send_to_char(buf,ch);
      return;
    }
    
  }
  num = create_zone(zone);
  if (num >= 0){
    sprintf(buf,"Zone %d created.\r\n",zone_table[num].number);
    send_to_char(buf,ch);
    sprintf(buf,"Zone %d created by %s.",zone_table[num].number,GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
    ch->specials2.edit_zone = zone;
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
      SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);
    
  }
  else{
    sprintf(buf,"Problem allocating memory for Zone %d.\r\n",zone_table[num].number);
    send_to_char(buf,ch);
  }
}
ACMD(do_mcreate)
{
    int zone,num;
    
    if (ch->specials2.edit_zone < 0){
	send_to_char ("You are not authorised to edit any zone..\r\n",ch);
	return;}

    zone = ch->specials2.edit_zone;

    num = create_mob(zone);
    if (num == 0){
	sprintf(buf,"No slots available in zone: %d. Mobile not created\r\n",zone);
	send_to_char(buf,ch);
	return;}
    else
	sprintf(buf,"Mobile %d created.\r\n",mob_index[num].virtual);
    send_to_char(buf,ch);
    sprintf(buf,"Mobile %d created by %s.",mob_index[num].virtual,GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_MOBS))
	SET_BIT(ch->specials2.builder_flag,BUILD_MOBS);
}

ACMD(do_rclone)
{
    int zone,num,numberl,numberu,r_numu,r_num,ntoclone,i;

    if (ch->specials2.edit_zone < 0){
	send_to_char ("You are not authorised to edit any zone..\r\n",ch);
	return;}
    
    half_chop(argument,buf,buf2);
    if (!*buf){
	send_to_char ("Usage rclone < lower room vnum upper room vnum | room vnum>.\r\n",ch);
	return;}
    if (!*buf2){
	r_num = -1;
	if (is_number(buf))
	  {
	    if ((numberl = atoi(buf)) < 0)
	      {
		send_to_char("A negative room number .... an interesting concept.\r\n",ch);
		return;
	      }
	    if ((r_num = real_room(numberl)) < 0){
	      send_to_char("There is no such room with that number.\r\n",ch);
	      return;}
	    ntoclone = 1;
	  }
	else{
	  send_to_char("Huh?\r\n",ch);
	  return;}
	
    }
    else{
      r_num = -1;
      if (is_number(buf2) && is_number(buf2))
	{
	  if ((numberl = atoi(buf)) < 0 || (numberu = atoi(buf2)) < 0 )
	    {
	      send_to_char("A negative room number .... an interesting concept.\r\n",ch);
	      return;
	    }
	  if ((r_num = real_room(numberl)) < 0 || (r_numu = real_room(numberu)) < 0){
	    send_to_char("There is no such room with that number.\r\n",ch);
	    return;}
	  if (r_numu < r_num)
	    {
	      r_num = r_numu;
	      r_numu = real_room(numberl);
	    }
	  ntoclone = r_numu - r_num +1;
	}
      else{
	send_to_char("Huh?\r\n",ch);
	return;}
    }
    
    zone = ch->specials2.edit_zone;
    for (i=0; i< ntoclone;i++){
      num = create_room(zone);
      if (num == 0){
	sprintf(buf,"No more slots available in zone: %d. Cloning only %d rooms\r\n",zone,i);
	send_to_char(buf,ch);
	break;
      }
      if (r_num >= num)
	r_num++;
      sprintf(buf,"Rooms # %d cloned to # %d.\r\n",world[r_num+i].number,world[num].number);
      send_to_char(buf,ch);
      sprintf(buf,"Room # %d cloned to # %d by %s.",world[r_num+1].number,world[num].number,GET_NAME(ch));
      mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
      clone_room(r_num+i,num);
    }
    if (i == 0)
      return;
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ROOMS))
	SET_BIT(ch->specials2.builder_flag,BUILD_ROOMS);
    
}
ACMD(do_mclone)
{
    int zone,num,number,r_num,nr;

    if (ch->specials2.edit_zone < 0){
	send_to_char ("You are not authorised to edit any zone..\r\n",ch);
	return;}
    
    one_argument(argument,buf);
    if (!*buf){
	send_to_char ("Usage mclone < mob virtual number | mob name >.\r\n",ch);
	return;}
    /* ok mob by number */
    r_num = -1;
    if (is_number(buf))
	{
	    if ((number = atoi(buf)) < 0)
		{
		    send_to_char("A negative mob number .... an interesting concept.\r\n",ch);
		    return;
		}
	    if ((r_num = real_mobile(number)) < 0){
		send_to_char("Error: There is no such mob with than number.\r\n",ch);
		return;}
	    ch->desc->mob_edit = mob_proto + r_num;
	}
    /* mob by name */
    else {
	for (nr = 0; nr <= top_of_mobt;nr++)
	    if (isname(buf, mob_proto[nr].player.name)){
		ch->desc->mob_edit = mob_proto + nr ;
		break;}
	r_num = nr;}    
    if(!(ch->desc->mob_edit)){
	send_to_char("No such mobile! sorry.\r\n",ch);
	return;}
    ch->desc->mob_edit = 0;


    zone = ch->specials2.edit_zone;

    num = create_mob(zone);
    if (num == 0){
	sprintf(buf,"No slots available in zone: %d. Mobile not created\r\n",zone);
	send_to_char(buf,ch);
	return;}
    if (r_num >= num)
	r_num++;
    sprintf(buf,"Mobile # %d cloned to # %d.\r\n",mob_index[r_num].virtual,mob_index[num].virtual);
    send_to_char(buf,ch);
    sprintf(buf,"Mobile # %d cloned to # %d by %s.",mob_index[r_num].virtual,mob_index[num].virtual,GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
    clone_mob(r_num,num);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_MOBS))
	SET_BIT(ch->specials2.builder_flag,BUILD_MOBS);
}
ACMD(do_oclone)
{
    struct obj_data *from, *to;
    int zone,num,number,r_num,nr, toitemnr;

    if (ch->specials2.edit_zone < 0){
	send_to_char ("You are not authorised to edit any zone..\r\n",ch);
	return;}
    
    one_argument(argument,buf);
    if (!*buf){
	send_to_char ("Usage oclone < mob virtual number | mob name >.\r\n",ch);
	return;}
    /* ok object by number */
    r_num = -1;
    if (is_number(buf))
	{
	    if ((number = atoi(buf)) < 0)
		{
		    send_to_char("A negative obj number .... an interesting concept.\r\n",ch);
		    return;
		}
	    if ((r_num = real_object(number)) < 0){
		send_to_char("There is no such obj with than number.\r\n",ch);
		return;}
	    from = obj_proto + r_num;
	}
    /* object by name */
    else {
	for (nr = 0; nr <= top_of_objt;nr++)
	    if (isname(buf, obj_proto[nr].name)){
		from = obj_proto +nr ;
		break;}
	r_num = nr;}    
    if(!(from)){
	send_to_char("No such object! sorry.\r\n",ch);
	return;}

    zone = ch->specials2.edit_zone;

    num = create_obj(zone);
    if (num == 0){
	sprintf(buf,"No slots available in zone: %d. Object not created\r\n",zone);
	send_to_char(buf,ch);
	return;}
/*    if (r_num >= num)
	r_num++;
    from = obj_proto + r_num; */
    to = obj_proto + num;
    sprintf(buf,"Object # %d cloned to # %d.\r\n",obj_index[r_num].virtual,obj_index[num].virtual);
    send_to_char(buf,ch);
    sprintf(buf,"Object # %d cloned to # %d by %s.",obj_index[r_num].virtual,obj_index[num].virtual,GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
    toitemnr = to->item_number;
    copy_obj(from,to, TRUE);
    to->item_number = toitemnr;
}
ACMD(do_medit)
{
    struct char_data *edit_mob;
    int r_num,number,nr,zone,zn,next_zone;

    if (!ch->desc)
	return;

    if (GET_LEVEL(ch) < LEVEL_BUILDER){
    	send_to_char ("You aren't holy enough to edit mobiles.\r\n",ch);
	return;}
    
    one_argument(argument,buf);
    if (!*buf){
	send_to_char ("Usage medit < mob virtual number | mob name >.\r\n",ch);
	return;}
    /* ok mob by number */
    zone = ch->specials2.edit_zone;
    if (zone>0){
	for (zn = 0; zone_table[zn].number != zone && zn <= top_of_zone_table; zn++)
	    ;
	if (zn +1 <= top_of_zone_table)
	    next_zone = zone_table[zn+1].number;
	else
	    next_zone = zone +10;
    }
    else
	{
	    send_to_char("You aren't authorised to edit any zones.\r\n",ch);
	    return;}
    if (is_number(buf))
	{
	    if ((number = atoi(buf)) < 0)
		{
		    send_to_char("A negative mob number .... an interesting concept.\r\n",ch);
		    return;
		}
	    if ((r_num = real_mobile(number)) < 0){
		send_to_char("There is no such mob with than number.\r\n",ch);
		return;}
	    if ((number < ch->specials2.edit_zone*100
		 ||  number > next_zone*100)){
		send_to_char ("You are not authorised to edit that zone.\r\n",ch);
		ch->desc->virtual = 0;
		ch->desc->mob_edit = 0;
		return;}
		
	    ch->desc->mob_edit = mob_proto + r_num;
	    ch->desc->virtual = number;
	}
    else /* mob by name */
	for (nr = 0; nr <= top_of_mobt;nr++)
	    if (isname(buf, mob_proto[nr].player.name)){
		ch->desc->mob_edit = mob_proto +nr;
		ch->desc->virtual = mob_index[nr].virtual;
		break;}
    
    if(!(ch->desc->mob_edit)){
	send_to_char("No such mobile! sorry.\r\n",ch);
	return;}
    if(!(ch->desc->virtual)){
	send_to_char("Bogus virtual number! sorry.\r\n",ch);
	return;}    
    number = mob_index[ch->desc->mob_edit->nr].virtual;
    if ((number < ch->specials2.edit_zone*100
	 ||  number >= next_zone*100)){
	send_to_char ("You are not authorised to edit that zone.\r\n",ch);
	ch->desc->mob_edit = 0;
	ch->desc->virtual = 0;
	return;}

    act("$n wanders off to create part of the world.",TRUE,ch,0, 0,TO_ROOM);
    
    CREATE(edit_mob, struct char_data, 1);
    copy_mob(ch->desc->mob_edit, edit_mob);
    ch->desc->mob_edit = edit_mob;
    print_mob(ch->desc);
    send_to_char("\r\n**Enter Q to Quit**\r\n",ch);
    ch->desc->prompt_mode = 0;
    REMOVE_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_MOBS))
	SET_BIT(ch->specials2.builder_flag,BUILD_MOBS);
}
ACMD(do_oedit)
{
    struct obj_data *edit_obj;
    int r_num,number,nr,zone,zn,next_zone;
    
    if (!ch->desc)
	return;

    if (GET_LEVEL(ch) < LEVEL_BUILDER){
    	send_to_char ("You aren't holy enough to edit objects.\r\n",ch);
	return;}
    
    one_argument(argument,buf);
    if (!*buf){
	send_to_char ("Usage oedit < obj virtual number | obj name >.\r\n",ch);
	return;}
    zone = ch->specials2.edit_zone;
    if (zone>0){
	for (zn = 0; zone_table[zn].number != zone && zn <= top_of_zone_table; zn++)
	    ;
/*	if (zn +1 <= top_of_zone_table)
	    next_zone = zone_table[zn+1].number;
	else
	    next_zone = zone +10; */
	next_zone = (zone_table[zn].top +1) / 100;
    }
    else
	{
	    send_to_char("You aren't authorised to edit any zones.\r\n",ch);
	    return;}
    if (is_number(buf))
	{
	    if ((number = atoi(buf)) < 0)
		{
		    send_to_char("A negative obj number .... an interesting concept.\r\n",ch);
		    return;
		}
	    if ((r_num = real_object(number)) < 0){
		send_to_char("There is no such obj with than number.\r\n",ch);
		return;}
	    if ((number < zone*100 ||  number > next_zone*100)){	    
		send_to_char ("You are not authorised to edit that zone.\r\n",ch);
		ch->desc->obj_edit = 0;
		ch->desc->virtual = 0;		
		return;}
		
	    ch->desc->obj_edit = obj_proto + r_num;
	    ch->desc->virtual = obj_index[r_num].virtual;			    
	}
    else /* obj by name */
	for (nr = 0; nr <= top_of_objt;nr++)
	    if (isname(buf, obj_proto[nr].name)){
		ch->desc->obj_edit = obj_proto +nr ;
		ch->desc->virtual = obj_index[ch->desc->obj_edit->item_number].virtual;		
		break;}
    
    if(!(ch->desc->obj_edit)){
	send_to_char("No such object! sorry.\r\n",ch);
	return;}
    if(!(ch->desc->virtual)){
	send_to_char("Bogus virtual number! sorry.\r\n",ch);
	return;}    
    number = obj_index[ch->desc->obj_edit->item_number].virtual;
    if ((number < zone*100 ||  number > next_zone*100)){	    
	send_to_char ("You are not authorised to edit that zone.\r\n",ch);
	ch->desc->obj_edit = 0;
	ch->desc->virtual = 0;		
	return;}
    
    act("$n wanders off to create part of the world.",TRUE,ch,0, 0,TO_ROOM);
    CREATE(edit_obj, struct obj_data, 1);
    copy_obj(ch->desc->obj_edit, edit_obj, TRUE);
    ch->desc->obj_edit = edit_obj;
    print_obj(ch->desc);
    send_to_char("\r\n**Enter Q to Quit**\r\n",ch);
    ch->desc->prompt_mode = 0;
    REMOVE_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_OBJS))
      SET_BIT(ch->specials2.builder_flag,BUILD_OBJS);

}
ACMD(do_redit)
{
    int r_num,number,zone,j;
    struct room_data *edit_room;

    if (!ch->desc)
	return;

    if (GET_LEVEL(ch) < LEVEL_BUILDER){
    	send_to_char ("You aren't holy enough to edit rooms.\r\n",ch);
	return;}
    
    one_argument(argument,buf);
    if (!*buf){
	send_to_char ("Usage redit room number >.\r\n",ch);
	return;}

    j = ch->specials2.edit_zone;
   for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
       ;
       if (zone > top_of_zone_table)
	       return;

    if (is_number(buf))
	{
	    if ((number = atoi(buf)) < 0)
		{
		    send_to_char("A negative room number .... an interesting concept.\r\n",ch);
		    return;
		}
	    if ((r_num = real_room(number)) < 0){
		send_to_char("There is no such room with than number.\r\n",ch);
		return;}
	    
	    if (world[r_num].zone != zone){
		send_to_char ("You are not authorised to edit that zone.\r\n",ch);
		ch->desc->room_edit = 0;
		return;}
		
	    ch->desc->room_edit = world + r_num;
	}
    else{
	send_to_char("Sorry have to give a room number.\r\n",ch);
	return;}
	
    act("$n wanders off to create part of the world.",TRUE,ch,0, 0,TO_ROOM);
    CREATE(edit_room, struct room_data, 1);
    copy_room(ch->desc->room_edit, edit_room);
    ch->desc->room_edit = edit_room;
    print_room(ch->desc);
    send_to_char("\r\n**Enter Q to Quit**\r\n",ch);
    ch->desc->prompt_mode = 0;
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ROOMS))
	SET_BIT(ch->specials2.builder_flag,BUILD_ROOMS);    
    REMOVE_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);
}
ACMD(do_sedit)
{
    int s_num,number,zone,j;
    struct shop_data *edit_shop;
    struct char_data *keeper;
    if (!ch->desc)
	return;

    if (GET_LEVEL(ch) < LEVEL_ASS_IMPL){
    	send_to_char ("You aren't holy enough to edit shops.\r\n",ch);
	return;}
    
    one_argument(argument,buf);
    if (!*buf){
	send_to_char ("Usage sedit shop number >.\r\n",ch);
	return;}

    j = ch->specials2.edit_zone;

    if (is_number(buf))
	{
	    if ((number = atoi(buf)) < 0)
		{
		    send_to_char("A negative shop number .... an interesting concept.\r\n",ch);
		    return;
		}
	    if ((s_num = real_shop(number)) < 0){
		send_to_char("There is no such shop with than number.\r\n",ch);
		return;}
	    
	    if (number/100 != j){
		send_to_char ("You are not authorised to edit that zone.\r\n",ch);
		ch->desc->shop_edit = 0;
		return;}
		
	    ch->desc->shop_edit = shop_index + s_num;
	}
    else{
	send_to_char("Sorry have to give a shop number.\r\n",ch);
	return;}
	
    act("$n wanders off to create part of the world.",TRUE,ch,0, 0,TO_ROOM);
    CREATE(edit_shop, struct shop_data, 1);
    copy_shop(ch->desc->shop_edit, edit_shop);
    ch->desc->shop_edit = edit_shop;
    if (edit_shop->keeper > 0) 
      if (mob_index[edit_shop->keeper].func)
	mob_index[edit_shop->keeper].func = 0;
    print_shop(ch->desc);
    send_to_char("\r\n**Enter Q to Quit**\r\n",ch);
    ch->desc->prompt_mode = 0;
    REMOVE_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    if (!IS_SET(ch->specials2.builder_flag,BUILD_SHOPS))
      SET_BIT(ch->specials2.builder_flag,BUILD_SHOPS);

}

ACMD(do_zedit)
{
    int zn,number=-1,zone,j;
    bool ok = FALSE;
    struct zone_data *zone_edit;
    if (!ch->desc)
	return;

    one_argument(argument,buf);
    if (buf)
      if (is_number(buf)) {
	if ((number = atoi(buf)) < 0){
	  send_to_char("A negative zone number .... an interesting concept.\r\n",ch);
	  return;
	}
      }
    if (number <= -1)
      number = ch->specials2.edit_zone;
    for (zn =0;zn <= top_of_zone_table; zn++)
      if (zone_table[zn].number == number){
	ok = TRUE;
	break;
      }
    if (!ok){
      send_to_char("There is no such zone with than number.\r\n",ch);
      return;
    }
	
    if (GET_LEVEL(ch) < LEVEL_ASS_IMPL && number != ch->specials2.edit_zone){
      send_to_char ("You are not authorised to edit that zone.\r\n",ch);
      ch->desc->zone_edit = 0;
      return;}
    
    act("$n wanders off to create part of the world.",TRUE,ch,0, 0,TO_ROOM);
    CREATE(zone_edit, struct zone_data, 1);
    copy_zone(zone_table + zn, zone_edit);
    ch->desc->zone_edit = zone_edit;
    print_zonedata(ch->desc);
    send_to_char("\r\n**Enter Q to Quit**\r\n",ch);
    ch->desc->prompt_mode = 0;
    REMOVE_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    SET_BIT(PLR_FLAGS(ch), PLR_BUILDING);
    ch->specials2.edit_zone = number;
    if (!IS_SET(ch->specials2.builder_flag,BUILD_ZONE))
      SET_BIT(ch->specials2.builder_flag,BUILD_ZONE);

}


ACMD(do_vnum)
{
   if (IS_NPC(ch)) {
      send_to_char("What would a monster do with a vnum?\r\n", ch);
      return;
   }

   half_chop(argument, buf, buf2);

   if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj") && !is_abbrev(buf, "zone"))) {
      send_to_char("Usage: vnum { obj | mob | zone } <name>\r\n", ch);
      return;
   }

   if (is_abbrev(buf, "mob"))
      if (!vnum_mobile(buf2, ch))
	 send_to_char("No mobiles by that name.\r\n", ch);

   if (is_abbrev(buf, "obj"))
      if (!vnum_object(buf2, ch))
	 send_to_char("No objects by that name.\r\n", ch);
   
   if (is_abbrev(buf, "zone"))
     if (!vnum_zone(buf2, ch))
       	 send_to_char("No matching zresets in this zone.\r\n", ch);
}

void	do_stat_room(struct char_data *ch)
{
   struct extra_descr_data *desc=0;
   struct room_data *rm = &world[ch->in_room];
   int	i, found = 0;
   struct obj_data *j = 0;
   struct char_data *k = 0;

   sprintf(buf, "Room name: %s%s%s Light_Level: %s%d%s\r\n",
	   CCCYN(ch, C_NRM), rm->name,CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), compute_room_light_value(rm),CCNRM(ch, C_NRM));
   send_to_char(buf, ch);

   sprinttype(rm->sector_type, sector_types, buf2);
   sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
       rm->zone, CCGRN(ch, C_NRM), rm->number, CCNRM(ch, C_NRM), ch->in_room, buf2);
   send_to_char(buf, ch);

   sprintbit((long) rm->room_flags, room_bits, buf2);
   sprintf(buf, "SpecProc: %s, Flags: %s\r\n", (rm->funct) ? "Exists" : "No",
      buf2);
   send_to_char(buf, ch);

   send_to_char("Description:\r\n", ch);
   if (rm->description)
      send_to_char(rm->description, ch);
   else
      send_to_char("  None.\r\n", ch);
   if (rm->tele_delay > -1){
       sprintf(buf, "Teleport to: %s%d%s, Delay: %s%d%s\r\n",CCYEL(ch,C_NRM),
rm->tele_to_room,CCNRM(ch,C_NRM),CCYEL(ch,C_NRM),rm->tele_delay,
CCNRM(ch,C_NRM));
       send_to_char(buf,ch);
   }
   if (rm->ex_description) {
      sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
      for (desc = rm->ex_description; desc; desc = desc->next) {
	 strcat(buf, " ");
	 strcat(buf, (desc->keyword ? desc->keyword : "no-keyword"));	   
      }
      strcat(buf, CCNRM(ch, C_NRM));
      send_to_char(strcat(buf, "\r\n"), ch);
   }

   sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
   for (found = 0, k = rm->people; k; k = k->next_in_room) {
      if (!CAN_SEE(ch, k))
	 continue;
      sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
	 (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	 if (k->next_in_room)
	    send_to_char(strcat(buf, ",\r\n"), ch);
	 else
	    send_to_char(strcat(buf, "\r\n"), ch);
	 *buf = found = 0;
      }
   }

   if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
   send_to_char(CCNRM(ch, C_NRM), ch);

   if (rm->contents) {
      sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
      for (found = 0, j = rm->contents; j; j = j->next_content) {
	 if (!CAN_SEE_OBJ(ch, j))
	    continue;
	 sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
	 strcat(buf, buf2);
	 if (strlen(buf) >= 62) {
	    if (j->next_content)
	       send_to_char(strcat(buf, ",\r\n"), ch);
	    else
	       send_to_char(strcat(buf, "\r\n"), ch);
	    *buf = found = 0;
	 }
      }

      if (*buf)
	 send_to_char(strcat(buf, "\r\n"), ch);
      send_to_char(CCNRM(ch, C_NRM), ch);
   }


   for (i = 0; i < NUM_OF_DIRS; i++) {
      if (rm->dir_option[i]) {
	 if (rm->dir_option[i]->to_room == NOWHERE)
	    sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
	 else
	    sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
	       rm->dir_option[i]->to_room, CCNRM(ch, C_NRM));
	 sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
	 sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
	     CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	     rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	     buf2);
	 send_to_char(buf, ch);
	 if (rm->dir_option[i]->general_description)
	    strcpy(buf, rm->dir_option[i]->general_description);
	 else
	    strcpy(buf, "  No exit description.\r\n");
	 send_to_char(buf, ch);
      }
   }
}

void    do_stat_shop(struct char_data *ch,   struct shop_data *shop)
{
  int i;
  
  if (shop->in_room) 
    sprintf(buf, "Number: %s%d%s, Room: %s%d%s\r\n",
	    CCBYEL(ch,C_NRM),shop->virtual,CCNRM(ch, C_NRM),
	    CCBYEL(ch,C_NRM),shop->in_room,CCNRM(ch, C_NRM));  
  else
    sprintf(buf, "Number: %s%d%s, Room: %sWANDERER%s\r\n",
	    CCBYEL(ch,C_NRM),shop->virtual,CCNRM(ch, C_NRM),
	    CCBYEL(ch,C_NRM),CCNRM(ch, C_NRM));
  send_to_char(buf,ch);
  if (shop->keeper > 0)
    sprintf(buf, "Keeper: [%s%d%s] %s%s%s\r\n",
	    CCBBLU(ch,C_NRM),mob_index[shop->keeper].virtual,CCNRM(ch, C_NRM),
	    CCBCYN(ch,C_NRM),mob_proto[shop->keeper].player.short_descr,
	    CCNRM(ch, C_NRM));
  else
    sprintf(buf, "Keeper: None set\r\n");
  send_to_char(buf,ch);
  sprintf(buf, "Temper1: %s%d%s, Temper2: %s%d%s\r\n",
	  CCCYN(ch,C_NRM),shop->temper1,CCNRM(ch, C_NRM),
	  CCCYN(ch,C_NRM),shop->temper2,CCNRM(ch, C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "Open1: %s%d%s, Close1: %s%d%s, ",
	  CCCYN(ch,C_NRM),shop->open1 ,CCNRM(ch, C_NRM),
	  CCCYN(ch,C_NRM),shop->close1,CCNRM(ch, C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "Open2: %s%d%s, Close2: %s%d%s\r\n",
	  CCCYN(ch,C_NRM),shop->open2 ,CCNRM(ch, C_NRM),
	  CCCYN(ch,C_NRM),shop->close2,CCNRM(ch, C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "Sell markup: %s%f%s, Buy markdown: %s%f%s\r\n",
	  CCGRN(ch,C_NRM),shop->profit_buy ,CCNRM(ch, C_NRM),
	  CCGRN(ch,C_NRM),shop->profit_sell,CCNRM(ch, C_NRM));
  send_to_char(buf,ch);
  if (shop->tradetype > 0) {
    sprintbit(shop->tradetype,item_types, buf2);
    sprintf(buf, "Trades: %s%s%s\r\n",CCGRN(ch,C_NRM), buf2, CCNRM(ch, C_NRM));
  }
  else if (shop->tradetype == 0) 
    sprintf(buf, "Trades: %sANYTHING%s\r\n",CCGRN(ch,C_NRM),CCNRM(ch,C_NRM));
  else
    sprintf(buf, "Trades: %sOnly Sells%s\r\n",CCGRN(ch,C_NRM),CCNRM(ch,C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "No such item 1: %s%s%s\r\n",
	  CCBWHT(ch,C_NRM),shop->no_such_item1,CCNRM(ch,C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "No such item 2: %s%s%s\r\n",
	  CCBWHT(ch,C_NRM),shop->no_such_item2,CCNRM(ch,C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "Do not buy    : %s%s%s\r\n",
	  CCBWHT(ch,C_NRM),shop->do_not_buy,CCNRM(ch,C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "Missing cash 1: %s%s%s\r\n",
	  CCBWHT(ch,C_NRM),shop->missing_cash1,CCNRM(ch,C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "Missing cash 2: %s%s%s\r\n",
	  CCBWHT(ch,C_NRM),shop->missing_cash2,CCNRM(ch,C_NRM));
  send_to_char(buf,ch);
  sprintf(buf, "Message buy   : %s%s%s\r\n",
	  CCBWHT(ch,C_NRM),shop->message_buy,CCNRM(ch,C_NRM));
  send_to_char(buf,ch);
  if (shop->tradetype >=0) {
    sprintf(buf, "Message sell  : %s%s%s\r\n",
	    CCBWHT(ch,C_NRM),shop->message_sell,CCNRM(ch,C_NRM));
      send_to_char(buf,ch);

  }
  for ( i = 0; i< MAX_PROD; i++)
    {
      if (shop->producing[i] >= 0) {
	sprintf(buf, "Producing: [%s%d%s] %s%s%s\r\n",
	      CCCYN(ch,C_NRM),
	      obj_index[shop->producing[i]].virtual,
	      CCNRM(ch,C_NRM),CCCYN(ch,C_NRM),
	      obj_proto[shop->producing[i]].short_description,
	      CCNRM(ch,C_NRM));
	send_to_char(buf,ch);
      }
    }
}


void do_stat_object(struct char_data *ch, struct obj_data *j)
{
   bool found;
   int	i;
   struct obj_data *j2;
   struct extra_descr_data *desc=0;
   int	virtual;

   virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
   sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
      ((j->short_description) ? j->short_description : "<None>"),
      CCNRM(ch, C_NRM), j->name);
   send_to_char(buf, ch);
   sprinttype(GET_ITEM_TYPE(j), item_types, buf1);
   if (j->item_number >= 0)
      strcpy(buf2, (obj_index[j->item_number].func ? "Exists" : "None"));
   else
      strcpy(buf2, "None");
   sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
      CCGRN(ch, C_NRM), virtual, CCNRM(ch, C_NRM), j->item_number, buf1, buf2);
   send_to_char(buf, ch);
   sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
   send_to_char(buf, ch);

   if (j->ex_description) {
      sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
      for (desc = j->ex_description; desc; desc = desc->next) {
	 strcat(buf, " ");
	 strcat(buf, (desc->keyword ? desc->keyword : "no-keyword"));
      }
      strcat(buf, CCNRM(ch, C_NRM));
      send_to_char(strcat(buf, "\r\n"), ch);
   }

   send_to_char("Can be worn on: ", ch);
   sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
   strcat(buf, "\r\n");
   send_to_char(buf, ch);

   send_to_char("Setting char bits : ", ch);
   sprintbit(j->obj_flags2.bitvector_aff, affected_bits, buf);
   strcat(buf, "\r\nCan set char bits : ");
   sprintbit(j->obj_flags.bitvector, affected_bits, buf2);
   strcat(buf, buf2);
   strcat(buf, "\r\n");
   send_to_char(buf, ch);
   send_to_char("Timers : ", ch);
   sprintf(buf, "Aff dur: %d, No Use Dur: %d, Aff Time: %d, No Use Time: %d\r\n",j->obj_flags2.aff_dur,j->obj_flags2.no_use_dur,j->obj_flags2.aff_timer,j->obj_flags2.no_use_timer);
   send_to_char(buf,ch);
   send_to_char("Extra flags   : ", ch);
   sprintbit(j->obj_flags.extra_flags, extra_bits, buf);
   strcat(buf, "\r\n");
   send_to_char(buf, ch);

   sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d, Creation Pts: %d\r\n",
       j->obj_flags.weight, j->obj_flags.cost,
       j->obj_flags.cost_per_day,  j->obj_flags.timer, assess_item(j));
   send_to_char(buf, ch);

   strcpy(buf, "In room: ");
   if (j->in_room == NOWHERE)
      strcat(buf, "Nowhere");
   else {
      sprintf(buf2, "%d", world[j->in_room].number);
      strcat(buf, buf2);
   }
   strcat(buf, ", In object: ");
   strcat(buf, j->in_obj ? j->in_obj->short_description: "None");
   strcat(buf, ", Carried by: ");
   strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
   strcat(buf, ", Worn by: ");
   strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");   
   strcat(buf, "\r\n");
   send_to_char(buf, ch);

   switch (j->obj_flags.type_flag) {
   case ITEM_LIGHT :
      sprintf(buf, "Color: [%d], Type: [%d], Hours: [%d]",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2]);
      break;
   case ITEM_SCROLL :
   case ITEM_POTION :
      sprintf(buf, "Spells: %d, %d, %d, %d", j->obj_flags.value[0],
	  j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
      break;
   case ITEM_WAND :
   case ITEM_STAFF :
      sprintf(buf, "Spell: %d, Mana/power: %d", j->obj_flags.value[0],
	 j->obj_flags.value[1]);
      break;
   case ITEM_ROD:
      sprintf(buf, "Spell: %s%s%s, max charges: %d, charges remaining: %d",
	      CCCYN(ch, C_NRM),find_spell_name(j->obj_flags.value[1]),
	      CCNRM(ch, C_NRM),j->obj_flags.value[2],j->obj_flags.value[3] );
      break;
   case ITEM_CANTRIP:
   case ITEM_PHILTRE:
     sprintf(buf, "Spell: %s%s%s", CCCYN(ch, C_NRM),
	     find_spell_name(j->obj_flags.value[1]),CCNRM(ch, C_NRM));
     break;
   case ITEM_FIREWEAPON :
   case ITEM_WEAPON :
      sprintf(buf, "Tohit: %d, Todam: %dd%d, Type: %d", j->obj_flags.value[0],
          j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
      break;
   case ITEM_MISSILE :
      sprintf(buf, "Tohit: %d, Todam: %d, Type: %d", j->obj_flags.value[0],
          j->obj_flags.value[1], j->obj_flags.value[3]);
      break;
   case ITEM_ARMOR :
      sprintf(buf, "AC-apply [%d] Stopping [%d] Damage: [%d] Size: [%d] Limit: [%d]",
          j->obj_flags.value[0], j->obj_flags.value[1],
          j->obj_flags.value[4], j->obj_flags.value[5],
          j->obj_flags.value[6]);
      break;
   case ITEM_TRAP :
      sprintf(buf, "Spell: %d, - Hitpoints: %d",
          j->obj_flags.value[0], j->obj_flags.value[1]);
      break;
   case ITEM_CONTAINER :
      sprintf(buf, "Max-contains: %d, Locktype: %d, Corpse: %s",
          j->obj_flags.value[0], j->obj_flags.value[1],
          j->obj_flags.value[3] < 0 ? "Yes" : "No");
      break;
   case ITEM_DRINKCON :
   case ITEM_FOUNTAIN :
      sprinttype(j->obj_flags.value[2], drinks, buf2);
      sprintf(buf, "Max-contains: %d, Contains: %d, Poisoned: %s, Liquid: %s",
          j->obj_flags.value[0], j->obj_flags.value[1],
          j->obj_flags.value[3] ? "Yes" : "No", buf2);
      break;
   case ITEM_NOTE :
      sprintf(buf, "Tounge: %d", j->obj_flags.value[0]);
      break;
   case ITEM_KEY :
      sprintf(buf, "Keytype: %d", j->obj_flags.value[0]);
      break;
   case ITEM_FOOD :
      sprintf(buf, "Makes full: %d, Poisoned: %d",
          j->obj_flags.value[0], j->obj_flags.value[3]);
      break;
   default :
      sprintf(buf, "Values 0-3: [%d] [%d] [%d] [%d]",
          j->obj_flags.value[0], j->obj_flags.value[1],
          j->obj_flags.value[2], j->obj_flags.value[3]);
      break;
   }
   send_to_char(buf, ch);
   sprintf(buf,"\r\nSize: %s, Damage State: %s, ",
	   sizes[j->obj_flags.value[5]], damage_state[j->obj_flags.value[4]]);
   send_to_char(buf,ch);
   if (j->obj_flags.value[6])
       sprintf(buf,"Limit: [%d]",j->obj_flags.value[6]);
   else
       sprintf(buf,"Limit: [unlimited]");
   send_to_char(buf,ch);
   strcpy(buf, "\r\nEquipment Status: ");
   if (!j->carried_by)
      strcat(buf, "None");
   else {
      found = FALSE;
      for (i = 0; i < MAX_WEAR; i++) {
	 if (j->carried_by->equipment[i] == j) {
	    sprinttype(i, equipment_types, buf2);
	    strcat(buf, buf2);
	    found = TRUE;
	 }
      }
      if (!found)
	 strcat(buf, "Inventory");
   }
   send_to_char(strcat(buf, "\r\n"), ch);

   if (j->contains) {
      sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
      for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
	 sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
	 strcat(buf, buf2);
	 if (strlen(buf) >= 62) {
	    if (j2->next_content)
	       send_to_char(strcat(buf, ",\r\n"), ch);
	    else
	       send_to_char(strcat(buf, "\r\n"), ch);
	    *buf = found = 0;
	 }
      }

      if (*buf)
	 send_to_char(strcat(buf, "\r\n"), ch);
      send_to_char(CCNRM(ch, C_NRM), ch);
   }

   found = 0;
   send_to_char("Light Value:", ch);
   sprintf(buf2," %s%d%s\r\n",CCBYEL(ch,C_NRM),j->obj_flags2.light,CCNRM(ch,C_NRM));
   send_to_char(buf2, ch);
   send_to_char("Affections:", ch);
   for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (j->affected[i].modifier) {
	 sprinttype(j->affected[i].location, apply_types, buf2);
	 sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	    j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
   }
   if (!found)
      send_to_char(" None", ch);

   send_to_char("\r\n", ch);
}


void do_stat_character(struct char_data *ch, struct char_data *k)
{
   int	i, ii, i2, found = 0,attck;
   char *name, *tmp;
   struct obj_data *j;
   struct follow_type *fol;
   struct affected_type *aff;
   memory_rec *curr;

   switch (k->player.sex) {
   case SEX_NEUTRAL	: strcpy(buf, "NEUTRAL-SEX"); break;
   case SEX_MALE	: strcpy(buf, "MALE"); break;
   case SEX_FEMALE	: strcpy(buf, "FEMALE"); break;
   default		: strcpy(buf, "ILLEGAL-SEX!!"); break;
   }

   sprintf(buf2, " %s '%s'  IDNum: [%5ld], In room [%5d], Illum Lev: %d\r\n",
       (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
       GET_NAME(k), GET_IDNUM(k), world[k->in_room].number,
	   illumination_level(world[ch->in_room].light, ch));
   send_to_char(strcat(buf, buf2), ch);
   if (IS_MOB(k)) {
      sprintf(buf, "Alias: %s, VNum: [%5d], RNum: [%5d]\r\n",
	 k->player.name, mob_index[k->nr].virtual, k->nr);
      send_to_char(buf, ch);
   }
   sprintf(buf,"Height: %2d'%2d'', Weight %4d lbs\r\n",
	   (int) (100*GET_HEIGHT(k)/254)/12,
	   (int) (100*GET_HEIGHT(k)/254) - 12*((100*GET_HEIGHT(k)/254)/12),
	   GET_WEIGHT(k)/10);
   send_to_char(buf, ch);
   sprintf(buf, "Title: %s\r\n", (k->player.title ? k->player.title : "<None>"));
   send_to_char(buf, ch);

   sprintf(buf, "L-Des: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));
   send_to_char(buf, ch);

   if (IS_NPC(k)) {
      strcpy(buf, "Monster Class: ");
      tmp = rev_search_list(GET_RACE(k), npc_races);
      sprintf(buf2,"%s%s%s Mob Fame: %s%d%s",CCBBLU(ch, C_NRM),
	      ((tmp) ? tmp : "Unknown Class"),CCNRM(ch, C_NRM),
	      CCBBLU(ch, C_NRM), GET_FAME(ch), CCNRM(ch, C_NRM));
   } else {
      strcpy(buf, "Class: ");
      sprinttype(k->player.race, pc_race_types, buf2);
      strcat(buf, buf2);
      sprintf(buf2," Social Standing: %s%d%s",
	      CCBBLU(ch, C_NRM), GET_FAME(ch), CCNRM(ch, C_NRM));
   }
   strcat(buf, buf2);

   sprintf(buf2, "\r\nLev:[%s%2d%s], XP:[%s%7d%s], Align:[%s%4d%s], Mood:[%s%4d%s]\r\n",
	   CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
	   CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
	   CCYEL(ch, C_NRM), GET_ALIGNMENT(k), CCNRM(ch, C_NRM),
	   CCYEL(ch, C_NRM) , GET_MOOD(k), CCNRM(ch, C_NRM));
   strcat(buf, buf2);
   send_to_char(buf, ch);

   if (!IS_NPC(k)) {
      strcpy(buf1, (char *)asctime(localtime(&(k->player.time.birth))));
      strcpy(buf2, (char *)asctime(localtime(&(k->player.time.logon))));
      buf1[10] = buf2[10] = '\0';

      sprintf(buf, "Created:[%s], Last Logon:[%s], Played[%dh %dm], Age[%d]\r\n", 
	 buf1, buf2, k->player.time.played / 3600,
	 ((k->player.time.played / 3600) % 60), age(k).year);
      send_to_char(buf, ch);

      sprintf(buf, "Edit Zone:[%3ld], Hometown:[%d], Speaks:[%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])\r\n",k->specials2.edit_zone,
	 k->player.hometown, k->player.talks[0], k->player.talks[1],
	 k->player.talks[2], SPELLS_TO_LEARN(k), int_app[GET_INT(k)].learn,
	 wis_app[GET_WIS(k)].bonus);
      send_to_char(buf, ch);
   }
   send_to_char("Current/Base Stats:\r\n",ch);
   sprintf(buf, "Str:[%s%d/%d%s] Int:[%s%d/%d%s] Wis:[%s%d/%d%s] Dex:[%s%d/%d%s] Con:[%s%d/%d%s]\r\n",
	   CCCYN(ch, C_NRM), GET_STR(k),GET_RAW_STR(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_INT(k),GET_RAW_INT(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_WIS(k),GET_RAW_WIS(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_DEX(k),GET_RAW_DEX(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_CON(k),GET_RAW_CON(k), CCNRM(ch, C_NRM));
   send_to_char(buf, ch);
   sprintf(buf, "Foc:[%s%d/%d%s] Chr:[%s%d/%d%s] Per:[%s%d/%d%s] Gui:[%s%d/%d%s] Luc:[%s%d/%d%s]\r\n",
	   CCCYN(ch, C_NRM), GET_FOC(k),GET_RAW_FOC(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_CHR(k),GET_RAW_CHR(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_PER(k),GET_RAW_PER(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_GUI(k),GET_RAW_GUI(k), CCNRM(ch, C_NRM),
	   CCCYN(ch, C_NRM), GET_LUC(k),GET_RAW_LUC(k), CCNRM(ch, C_NRM));
   send_to_char(buf, ch);
   sprintf(buf, "Hit p.:[%s%4d/%4d+%d%s]  Move p.:[%s%4d/%4d+%d%s]\r\n",
       CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
       CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM));
   send_to_char(buf, ch);
   sprintf(buf,"%sAC/Stopping: %s",CCBBLU(ch,C_NRM), CCNRM(ch, C_NRM));
   send_to_char(buf, ch);       
   sprintf(buf,"Head [%s%d/%d%s],  Body [%s%d/%d%s],  ",
	   CCWHT(ch, C_NRM),GET_HEAD_AC(k),GET_HEAD_STOPPING(k),CCNRM(ch,C_NRM),
	   CCWHT(ch, C_NRM),GET_BODY_AC(k),GET_BODY_STOPPING(k),CCNRM(ch,C_NRM));
       send_to_char(buf, ch);
   sprintf(buf,"Arm  [%s%d/%d%s],  Leg [%s%d/%d%s]\r\n",
	   CCWHT(ch, C_NRM),GET_ARMS_AC(k),GET_ARMS_STOPPING(k),CCNRM(ch,C_NRM),
	   CCWHT(ch, C_NRM),GET_LEGS_AC(k),GET_LEGS_STOPPING(k),CCNRM(ch,C_NRM));
       send_to_char(buf, ch);
    if (k->specials.attack_type < TYPE_HIT)
	attck = TYPE_HIT;
    else
	attck = k->specials.attack_type;

   sprintf(buf, "%sHitroll:%s [%2d], %sDamroll:%s [%2d], %sHitroll2:%s [%2d], %sDamroll2:%s [%2d]\r\n%sCombat Bonus:%s [%2d]\r\n%sSaving throws:%s [%d/%d/%d/%d/%d]",
	   CCBYEL(ch, C_NRM),
	   CCNRM(ch, C_NRM),
	   k->points.hitroll,
	   CCBYEL(ch, C_NRM),
	   CCNRM(ch, C_NRM),
	   k->points.damroll,
	   CCBYEL(ch, C_NRM),
	   CCNRM(ch, C_NRM),
	   k->specials.hitroll,
	   CCBYEL(ch, C_NRM),
	   CCNRM(ch, C_NRM),
	   k->specials.damroll,
	   CCBYEL(ch, C_NRM),
	   CCNRM(ch, C_NRM),
	   combat_bonus(k),
	   CCBGRN(ch, C_NRM),
	   CCNRM(ch, C_NRM),
	   k->specials2.apply_saving_throw[0],
	   k->specials2.apply_saving_throw[1],
	   k->specials2.apply_saving_throw[2],
	   k->specials2.apply_saving_throw[3],
	   k->specials2.apply_saving_throw[4]
       );
   sprintf(buf,"%s %sAttack Type:%s %s\r\n",
	   buf,
	   CCBMAG(ch, C_NRM),
	   CCNRM(ch, C_NRM),
	   attack_hit_text[attck-TYPE_HIT].singular);

   send_to_char(buf, ch);
   sprintf(buf, "Coins: [%9d], Bank: [%9d] (Total: %d)\r\n",
       GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));
   send_to_char(buf, ch);

   sprinttype(GET_POS(k), position_types, buf2);
   sprintf(buf, "Pos: %s, Fighting: %s", buf2,
       ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody") );
   if (k->desc) {
      sprinttype(k->desc->connected, connected_types, buf2);
      strcat(buf, ", Connected: ");
      strcat(buf, buf2);
   }
   send_to_char(strcat(buf, "\r\n"), ch);

   strcpy(buf, "Default position: ");
   sprinttype((k->specials.default_pos), position_types, buf2);
   strcat(buf, buf2);

   sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->specials.timer);
   strcat(buf, buf2);
   send_to_char(buf, ch);


   if (IS_MOB(k)) {
      sprintf(buf, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
	(mob_index[k->nr].func ? "Exists" : "None"),
        k->specials.damnodice, k->specials.damsizedice);
      send_to_char(buf, ch);
   }

   sprintf(buf, "Carried/Max: weight: %d/%d, items: %d/%d; ",
	   IS_CARRYING_W(k)/10, CAN_CARRY_W(k)/10,
	   IS_CARRYING_N(k), CAN_CARRY_N(k));

   for (i = 0, j = k->inventory; j; j = j->next_content, i++)
      ;
   sprintf(buf, "%sItems in: inventory: %d, ", buf, i);

   for (i = 0, i2 = 0; i < MAX_WEAR; i++)
      if (k->equipment[i])
	 i2++;
   sprintf(buf2, "eq: %d, ", i2);
   strcat(buf, buf2);
   sprintf(buf2, "Encumberance Level: %d\r\n",encumberance_level(k));
   strcat(buf, buf2);   
   send_to_char(buf, ch);

   sprintf(buf, "Light Value: %s%d%s, Hunger: %d, Thirst: %d, Drunk: %d\r\n",
	   CCBYEL(ch,C_NRM),k->light,CCNRM(ch,C_NRM),
	   GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
   send_to_char(buf, ch);
   sprintf(buf2," ");
   if (IS_MOB(k)){
       sprintf(buf,"Hates:");
       if (!k->specials.memory)
	   sprintf(buf2," no-one");
       else{
	   for(curr=k->specials.memory;curr;curr=curr->next)
	       for (ii=0;ii<=top_of_p_table;ii++){
		   if (player_table[ii].id_num  == curr->id)
		       sprintf(buf2,"%s %s",buf2,player_table[ii].name);}}
       sprintf(buf,"%s %s\r\n",buf,buf2);
       send_to_char(buf,ch);
   }
   sprintf(buf, "Master is: %s, Followers are:",
      ((k->master) ? GET_NAME(k->master) : "<none>"));
   for (fol = k->followers; fol; fol = fol->next) {
      sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	 if (fol->next)
	    send_to_char(strcat(buf, ",\r\n"), ch);
	 else
	    send_to_char(strcat(buf, "\r\n"), ch);
	 *buf = found = 0;
      }
   }

   if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
   if (IS_NPC(k)) {
      sprintbit(MOB_FLAGS(k), action_bits, buf2);
      sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
      send_to_char(buf, ch);
   } else {
      sprintbit(PLR_FLAGS(k), player_bits, buf2);
      sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
      send_to_char(buf, ch);
      sprintbit(PRF_FLAGS(k), preference_bits, buf2);
      sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
      send_to_char(buf, ch);
   }

   /* Showing the bitvector */
   sprintbit(k->specials.affected_by, affected_bits, buf2);
   sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
   send_to_char(buf, ch);

   /* Routine to show what spells a char is affected by */
   if (k->affected) {
      for (aff = k->affected; aff; aff = aff->next) {
	 *buf2 = '\0';
	 if ((name = find_skill_name(aff->type))){
	   sprintf(buf, "SKL: (%3dhr) %s%-21s%s ", aff->duration+1,
		   CCCYN(ch, C_NRM), name, CCNRM(ch, C_NRM));
	   if (aff->modifier && aff->type < 400)
	     sprintf(buf, "%s %+d to %s ", buf, aff->modifier, apply_types[(int)aff->location]);
	   
	 }
	 else if ((name = find_spell_name(aff->type))){
	     sprintf(buf, "SPL: (%3dhr) %s%-21s %slevel: %3d%s ", aff->duration+1,
		     CCCYN(ch, C_NRM), name, 
		     CCGRN(ch,C_NRM),aff->level,CCNRM(ch, C_NRM) );
	     if (aff->modifier && aff->type < 400)
		 sprintf(buf, "%s %+d to %s ", buf, aff->modifier, apply_types[(int)aff->location]);
	 }
	 if (aff->bitvector) {
	    if (*buf2)
	       strcat(buf, ", sets: ");
	    else
	       strcat(buf, "sets: ");
	    sprintbit(aff->bitvector, affected_bits, buf2);
	    strcat(buf, buf2);
	 }
	 send_to_char(strcat(buf, "\r\n"), ch);
      }
   }
}

ACMD(do_whohas)
{
    int vnum, total=0,gtotal=0,i;
    struct rent_info rent;
    char fname[MAX_INPUT_LENGTH];
    FILE *rf;

    if (!(*argument))
	{
	    send_to_char("Usage: whohas object_vnum#.\r\n",ch);
	    return;
	}
    if (IS_NPC(ch)) {
	send_to_char("Why would a monster want to know?\r\n", ch);
	return;
    }
    one_argument(argument, arg);

    vnum = atoi(arg);
    if (real_object(vnum) < 0){
	send_to_char("No such object sorry.\r\n",ch);
	return;}
    sprintf(buf,"Checking object vnum: %d\r\n",vnum);
    send_to_char(buf,ch);
    for (i=0;i <= top_of_p_table;i++){
	if(!Crash_get_filename((player_table +i)->name,fname))
	    return;
	if (!(rf = fopen(fname,"r+b"))){
	    if (errno != ENOENT) { /* if it fails, NOT because of no file */
		sprintf(buf1, "SYSERR: READING OBJECT FILE %s", fname);
		perror(buf1);
		send_to_char(buf1,ch);
	    }
	    continue;
	}
	fread(&rent, sizeof(struct rent_info),1, rf);
	total =0;
	if (rent.version == 0){
	  struct obj_file_elem_0 tmp_p;
	  while (!feof(rf))
	    {
	      fread(&tmp_p, sizeof(struct obj_file_elem_0),1,rf);
	      if (tmp_p.item_number == vnum)
		total++;
	      tmp_p.item_number =0;
	    }
	}
	else if (rent.version == 1) {
	  struct obj_file_elem_1 tmp_p;
	    while (!feof(rf))
		{
		    fseek( rf,1,1);
		    fread(&tmp_p, sizeof(struct obj_file_elem_1),1,rf);
		    if (tmp_p.item_number == vnum)
			total++;
		    tmp_p.item_number =0;
		}
	}
	else if (rent.version == 2) {
	  struct obj_file_elem tmp_p;
	    while (!feof(rf))
		{
		    fseek( rf,1,1);
		    fread(&tmp_p, sizeof(struct obj_file_elem),1,rf);
		    if (tmp_p.item_number == vnum)
			total++;
		    tmp_p.item_number =0;
		}
	}
	fclose(rf);
	if (total >0){
	gtotal += total;
	sprintf(buf,"%s has %d.\r\n",(player_table +i)->name, total);
	send_to_char(buf,ch);
	}

    }
    sprintf(buf,"total in rent %d.\r\n", gtotal);
    send_to_char(buf,ch);
    if (obj_proto[real_object(vnum)].obj_flags.value[6] >0){
	for (i=0;i<top_of_ol_table;i++)
	    if (obj_limit_table[i].obj_num == vnum)
		break;
	sprintf(buf,"Limit data says: %d on chars.\r\n", obj_limit_table[i].no_stored);
	send_to_char(buf,ch);
	if (obj_limit_table[i].no_stored != gtotal){
	    obj_limit_table[i].no_stored = gtotal;
	    send_to_char("Limit data corrected.\r\n",ch);
	    save_all_object_limit_data();}
	
    }
}



