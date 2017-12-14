/* ************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* act.informative split into 2 files act.inform1 and act.inform2         */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "screen.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct command_info cmd_info[];
extern struct index_data *mob_index;

extern char	*credits;
extern char	*news;
extern char	*info;
extern char	*wizlist;
extern char	*immlist;
extern char	*policies;
extern char	*handbook;
extern char	*building;
extern char	*dirs[];
extern char	*where[];
extern char	*color_liquid[];
extern char	*fullness[];
extern char	*connected_types[];
extern char	*command[];
extern char	*race_abbrevs[];
extern char	*room_bits[];
extern char	*spells[];
extern char	*skills_list[];
extern char	*equipment_types[];
extern char	*affected_bits[];
extern char	*apply_types[];
extern char	*pc_race_types[];
extern char	*npc_class_types[];
extern char	*action_bits[];
extern char	*position_types[];
extern int      slave_socket;
extern int      port;
char *format_inet_addr( char *dest, long addr );
char *report_cost(int gold);
char *report_gold(int gold);
void	show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode);
/* intern functions & vars*/
int	num_of_cmds;
void	list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, bool show);
int can_see_hidden(struct char_data *sub, struct char_data *obj);
void	diag_char_to_char(struct char_data *i, struct char_data *ch);
#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-o] [-p]\r\n"
ACMD(do_user_check)
{
  char sbuf[4097], *pbuf;
  struct descriptor_data *d=0;
  struct char_data *victim = 0;

  half_chop(argument, buf1, buf2);
  if (!*buf1) {
    send_to_char("User check on who or what?\r\n", ch);
    return;
  }
  if (!(victim = get_player_vis(ch, buf1))) {
    send_to_char("No such player around.\r\n",ch);
    return;
  }
  if (!victim->desc) {
    send_to_char("Your subject is not connected.\r\n",ch);
    return;
  }
  d= victim->desc;
  if (slave_socket != -1) {
    sbuf[0] = 'i';
    pbuf = format_inet_addr(sbuf+1, d->addr);
    pbuf += sprintf(pbuf, ",%d,%d\n", d->port, port);
    if (write(slave_socket, sbuf, pbuf - sbuf + 1) != (pbuf - sbuf + 1)) {
      logg("[SLAVE] loosing slave on write:");
      close(slave_socket);
      slave_socket = -1;
    }
    
  }
  send_to_char("Check queued.\r\n",ch);

}

ACMD(do_users)
{
   extern char	*connected_types[];
   char	line[200], line2[220], idletime[10], classname[20];
   char	state[30], *timeptr;

   struct char_data *tch;
   char	name_search[80];
   char	host_search[80];
   char	mode, *format;
   int	low = 0, high = LEVEL_IMPL;
   int	outlaws = 0, num_can_see = 0, playing = 0, deadweight = 0;

   struct descriptor_data *d;

   name_search[0] = '\0';
   host_search[0] = '\0';

   strcpy(buf, argument);
   while (*buf) {
      half_chop(buf, arg, buf1);
      if (*arg == '-') {
	 mode = *(arg + 1); /* just in case; we destroy arg in the switch */
	 switch (mode) {
	 case 'o':
	 case 'k':
	    outlaws = 1;
	    playing = 1;
	    strcpy(buf, buf1);
	    break;
	 case 'p':
	    playing = 1;
	    strcpy(buf, buf1);
	    break;
	 case 'd':
	    deadweight = 1;
	    strcpy(buf, buf1);
	    break;
	 case 'l':
	    playing = 1;
	    half_chop(buf1, arg, buf);
	    sscanf(arg, "%d-%d", &low, &high);
	    break;
	 case 'n':
	    playing = 1;
	    half_chop(buf1, name_search, buf);
	    break;
	 case 'h':
	    playing = 1;
	    half_chop(buf1, host_search, buf);
	    break;
	 default:
	    send_to_char(USERS_FORMAT, ch);
	    return;
	    break;
	 } /* end of switch */

      } else { /* endif */
	 send_to_char(USERS_FORMAT, ch);
	 return;
      }
   } /* end while (parser) */
   strcpy(line,
       "\r\nNum Name         State          Idl Login@   Remote info\r\n");
   strcat(line,
       "--- ------------ -------------- --- -------- --------------------------------\r\n");
   send_to_char(line, ch);

   one_argument(argument, arg);

   for (d = descriptor_list; d; d = d->next) {
      if (d->connected && playing)
	 continue;
      if (!d->connected && deadweight)
	 continue;
      if (!d->connected) {
	 if (d->original)
	    tch = d->original;
	 else if (!(tch = d->character))
	    continue;

	 if (*host_search && !strstr(d->host, host_search))
	    continue;
	 if (*name_search && str_cmp(GET_NAME(tch), name_search) && 
	     !strstr(GET_TITLE(tch), name_search))
	    continue;
	 if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	    continue;
	 if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) && 
	     !PLR_FLAGGED(tch, PLR_THIEF))
	    continue;
	 if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
	    continue;
	 if ((*d->user_id != '\0') && (GET_LEVEL(ch) >= 220))
	   sprintf(classname, "%8s@", d->user_id);
	 else
	   sprintf(classname, "%9s","");
      }
      else
	sprintf(classname, "%9s","");
      timeptr = asctime(localtime(&d->login_time));
      timeptr += 11;
      *(timeptr + 8) = '\0';
      if (!d->connected && d->original)
	 strcpy(state, "Switched");
      else
	 strcpy(state, connected_types[d->connected]);

      if (d->character && !d->connected && GET_LEVEL(d->character) < LEVEL_IMPL)
	 sprintf(idletime, "%3d",
	     d->character->specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
      else
	 strcpy(idletime, "");

      format = "%3d %-12s %-12s %-3s   %-8s ";

      if (d->character && d->character->player.name) {
	 if (d->original)
	    sprintf(line, format, d->desc_num,
		    d->original->player.name, state, idletime, timeptr);
	 else
	    sprintf(line, format, d->desc_num,
		    d->character->player.name, state, idletime, timeptr);
      } else
	 sprintf(line, format, d->desc_num, "UNDEFINED",
		 state, idletime, timeptr);

      if (d->host && *d->host)
	 sprintf(line + strlen(line), "[%s%s]\r\n",classname, d->host);
      else
	 strcat(line, "[Hostname unknown]\r\n");

      if (d->connected) {
	 sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
	 strcpy(line, line2);
      }

      if (d->connected || (!d->connected && CAN_SEE(ch, d->character))) {
	 send_to_char(line, ch);
	 num_can_see++;
      }
   }

   sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
   send_to_char(line, ch);
}



ACMD(do_inventory)
{
   send_to_char("You have in your inventory:\r\n", ch);
   list_obj_to_char(ch->inventory, ch, 1, TRUE);
}


ACMD(do_equipment)
{
   int	j,jj;
   bool found;
   const int loc[]={
       6,18,19,20,21,3,4,5,22,10,1,2,14,15,9,11,23,0,16,17,12,13,7,8};       
   send_to_char("You are using:\r\n", ch);
   found = FALSE;
   jj = 1;
   for (j = 0; j < MAX_WEAR; j++) {
     if ((loc[j] == HOLD) && ch->specials.carrying) {
       sprintf(buf, "[%2i] <carried>         %s\r\n",jj,
	       (IS_NPC(ch->specials.carrying) ?
		ch->specials.carrying->player.short_descr :
		ch->specials.carrying->player.name));
       found = TRUE;
       send_to_char(buf, ch);
     }
     if (ch->equipment[loc[j]]) {
       if (CAN_SEE_OBJ(ch, ch->equipment[loc[j]])) {
	 if ((loc[j] == HOLD &&
	      (is_held_obj_twohanded(ch,ch->equipment[loc[j]]))) ||
	     (loc[j] == WIELD &&
	      (is_two_handed(ch,ch->equipment[loc[j]]))))
	   sprintf(buf, "[%2i]%s ",jj,"<both hands>      ");
	 else
	   sprintf(buf,"[%2i]%s ",jj,where[loc[j]]);
	 send_to_char(buf, ch);
	 show_obj_to_char(ch->equipment[loc[j]], ch, 1);
	 found = TRUE;
       } else {
	 if ((loc[j] == HOLD &&
	      (is_held_obj_twohanded(ch,ch->equipment[loc[j]]))) ||
	     (loc[j] == WIELD &&
	      (is_two_handed(ch,ch->equipment[loc[j]]))))
	   sprintf(buf, "[%2i]%s ",jj,"<both hands>      ");
	 else	     
	   sprintf(buf,"[%2i] %s",jj,where[loc[j]]);
	 send_to_char(buf, ch);
	 send_to_char("Something.\r\n", ch);
	 found = TRUE;
       }
       jj++;
     }
   }
   if (!found) {
     send_to_char(" Nothing.\r\n", ch);
   }
}


ACMD(do_gen_ps)
{
   extern char	circlemud_version[];

   switch (subcmd) {
   case SCMD_CREDITS : page_string(ch->desc, credits, 0); break;
   case SCMD_NEWS    : page_string(ch->desc, news, 0); break;
   case SCMD_INFO    : page_string(ch->desc, info, 0); break;
   case SCMD_WIZLIST : page_string(ch->desc, wizlist, 0); break;
   case SCMD_IMMLIST : page_string(ch->desc, immlist, 0); break;
   case SCMD_HANDBOOK: page_string(ch->desc, handbook, 0); break;
   case SCMD_BUILDH  : page_string(ch->desc, building, 0); break;       
   case SCMD_POLICIES: page_string(ch->desc, policies, 0); break;
   case SCMD_CLEAR   : send_to_char("\033[H\033[J", ch); break;
   case SCMD_VERSION : send_to_char(circlemud_version, ch); break;
   case SCMD_WHOAMI  : send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch); break;
   default: return; break;
   }
}


void perform_mortal_where(struct char_data *ch, char *arg)
{
   register struct char_data *i;
   register struct descriptor_data *d;

   if (!*arg) {
      send_to_char("Players in your Zone\r\n--------------------\r\n", ch);
      for (d = descriptor_list; d; d = d->next)
	 if (!d->connected) {
	    i = (d->original ? d->original : d->character);
            if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) &&
	     (world[ch->in_room].zone == world[i->in_room].zone)) {
		sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
		send_to_char(buf, ch);
	    }
	 }
   } else { /* print only FIRST char, not all. */
       for (d = descriptor_list; d; d = d->next)
	   if (!d->connected){
	       i = (d->original ? d->original : d->character);
	       if (world[i->in_room].zone == world[ch->in_room].zone && CAN_SEE(ch, i) &&
		   (i->in_room != NOWHERE) && isname(arg, i->player.name)) {
		   sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
		   send_to_char(buf, ch);
		   return;
	       }
	   }
       send_to_char("No-one around by that name.\r\n", ch);
   }
}


void perform_immort_where(struct char_data *ch, char *arg)
{
   register struct char_data *i;
   register struct obj_data *k, *tmp;
   struct descriptor_data *d;
   int	num = 0, found = 0;

   if (!*arg) {
      send_to_char("Players\r\n-------\r\n", ch);
      for (d = descriptor_list; d; d = d->next)
	 if (!d->connected) {
	    i = (d->original ? d->original : d->character);
            if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
	       if (d->original)
		  sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
		     GET_NAME(i), world[d->character->in_room].number,
		     world[d->character->in_room].name, GET_NAME(d->character));
	       else
		  sprintf(buf, "%-20s - [%5d] %s\r\n", GET_NAME(i),
		     world[i->in_room].number, world[i->in_room].name);
	       send_to_char(buf, ch);
	    }
	 }
   }
   else {
     half_chop(arg,buf1,buf2);
     for (i = character_list; i; i = i->next)
       if (CAN_SEE(ch, i) && i->in_room != NOWHERE &&
	   isname(buf1, i->player.name))
	 if (!*buf2 || isname(buf2, i->player.name)){
	   found = 1;
	   sprintf(buf, "%3d. %-25s - [%5d] %s\r\n", ++num, GET_NAME(i),
		   world[i->in_room].number, world[i->in_room].name);
	   send_to_char(buf, ch);
	 }
     
     for (num = 0, k = object_list; k; k = k->next)
       if (CAN_SEE_OBJ(ch, k) && isname(buf1, k->name))
	 if (!*buf2 || isname(buf2, k->name)){
	   found = 1;
	   if (k->in_room != NOWHERE){
	     sprintf(buf, "%3d. %-25s - [%5d] %s\r\n", ++num,
		     k->short_description, world[k->in_room].number,
		     world[k->in_room].name);
	      send_to_char(buf, ch);}
	   else if (k->carried_by){
	     sprintf(buf, "%3d. %-25s - [%5d] carried by %s\r\n", ++num,
		     k->short_description,
		     world[k->carried_by->in_room].number,
		     (!(IS_NPC(k->carried_by))
		      ? k->carried_by->player.name
		      : k->carried_by->player.short_descr));
	     send_to_char(buf, ch);}
	    else if (k->worn_by){
	      sprintf(buf, "%3d. %-25s - [%5d] worn by %s\r\n", ++num,
		      k->short_description,
		      world[k->worn_by->in_room].number,
		       (!(IS_NPC(k->worn_by))
			? k->worn_by->player.name
			: k->worn_by->player.short_descr));
	      send_to_char(buf, ch);}
	   else if (k->in_obj){
	     sprintf(buf, "%3d. %-25s - inside %s\r\n", ++num,
		     k->short_description,
		     k->in_obj->short_description);
	     send_to_char(buf, ch);
	     tmp = k->in_obj;
	     	     while(tmp->in_obj){
	       sprintf(buf, "%13s%4s%13s   - inside %s\r\n"," "
		       , "...."," ",
		       tmp->in_obj->short_description);
	       send_to_char(buf, ch);
	       tmp=tmp->in_obj;
	     }
	     if (tmp->worn_by){
	       sprintf(buf, "%13s%4s%13s   - [%5d] worn by %s\r\n",
			 " ", "...."," ",
		       world[tmp->worn_by->in_room].number,
		       (!(IS_NPC(tmp->worn_by))
			? tmp->worn_by->player.name
			: tmp->worn_by->player.short_descr));
	       send_to_char(buf, ch);
	       } 
	     else if (tmp->carried_by){
	       sprintf(buf, "%13s%4s%13s   - [%5d] carried by %s\r\n",
		       " ", "...."," ",
		       world[tmp->carried_by->in_room].number,
		       (!(IS_NPC(tmp->carried_by))
			? tmp->carried_by->player.name
			: tmp->carried_by->player.short_descr));
	       send_to_char(buf, ch);
	     }
	     else if (tmp->in_room != NOWHERE){
	       sprintf(buf, "%13s%4s%13s   - [%5d] %s\r\n",
		       " ", "...."," ",
		       world[tmp->in_room].number,
		       world[tmp->in_room].name);
	       send_to_char(buf, ch);
	     }
	   }
	 }
     
     if (!found)
       send_to_char("Couldn't find any such thing.\r\n", ch);
   }
}
   


ACMD(do_where)
{
  /*   one_argument(argument, arg); */

   if (GET_LEVEL(ch) >= LEVEL_BUILDER)
      perform_immort_where(ch, argument);
   else
      perform_mortal_where(ch, argument);
}



ACMD(do_levels)
{
   int	i,lower,upper;

   if (IS_NPC(ch)) {
      send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
      return;
   }

   *buf = '\0';
   half_chop(argument,buf1,buf2);
   if (!*argument){
       send_to_char("Usage: levels {#from #to}\r\n",ch);
       return;}

   
   if (!*buf2 && atoi(buf1) <= 228 && atoi(buf1) > 0){
       i = atoi(buf1);
       sprintf(buf,"[%3d] %9d\r\n",i,levels_table[i]);
       return;}
   if (*buf2 && *buf1){
       lower = MIN(atoi(buf1),atoi(buf2));
       upper = MAX(atoi(buf1),atoi(buf2));
       if (lower < 0)
	   lower = 0;
       if (upper > LEVEL_IMPL)
	   upper = LEVEL_IMPL;
       for (i = lower; i <= upper; i++) {
	   sprintf(buf + strlen(buf), "[%3d] %9d-%-9d : \r\n",
		   i, levels_table[i],levels_table[i + 1]);}
       send_to_char(buf, ch);}

}
int compute_dam(struct char_data *ch, int type)
{
  struct obj_data *wielded;
  int dam=0, dam2=0, bonus=0, damg, w_type, prof, num;
  extern struct str_app_type str_app[];

  wielded = ch->equipment[WIELD];
  if (type == SKILL_DUAL_WIELD)
    if (wielded = ch->equipment[HOLD])
      if (GET_ITEM_TYPE(wielded) != ITEM_WEAPON)
	return(0); 
  
  prof = get_prof(ch, &w_type, type);
  if (prof >= PROF_SWORD && prof <= PROF_CLAW)
    num = GET_SKILL(ch,prof);
  else if (!IS_MOB(ch) && !wielded)
    num = GET_SKILL(ch,SKILL_FISTICUFFS);
  else
    num =0;
  
  if (!affected_by_spell(ch, SPELL_ENDURANCE))   
    bonus = combat_bonus(ch);

  dam  = str_app[GET_STR(ch)].todam;
  dam += bonus;
  if (type == SKILL_DUAL_WIELD)
    dam += GET_DAMROLL2(ch);
  else
    dam += GET_DAMROLL(ch);    
  if (!wielded) {
    if (IS_NPC(ch) && ch->specials.damsizedice){
      dam += ((ch->specials.damnodice*ch->specials.damsizedice)
	      + ch->specials.damnodice)/2;
    }
    else 
      dam +=GET_SKILL(ch, SKILL_FISTICUFFS)/4;
  }
  else{
    damg = MAX(0,wielded->obj_flags.value[4]);
    if (wielded->obj_flags.value[2])
      dam += ((wielded->obj_flags.value[1]*wielded->obj_flags.value[2]
	       + wielded->obj_flags.value[1])/2)*(10-damg)/10;
    dam *= (num + 120);
    dam /= 150;
  }
  dam *= compute_multi_attacks(ch,wielded,type);
  return(dam);
}

int compute_thaco(struct char_data *ch, int type)
  {
  struct obj_data *wielded;
  int bonus=0, thaco, prof, prof_factor, w_type, num;
  extern struct str_app_type str_app[];
  
  prof = get_prof(ch, &w_type, type);
  if (prof >= PROF_SWORD && prof <= PROF_CLAW)
    num = GET_SKILL(ch,prof);
  else if (!IS_MOB(ch) && !wielded)
    num = GET_SKILL(ch,SKILL_FISTICUFFS);
  else
    num =0;

  if (!IS_NPC(ch) && num)
    prof_factor = 100 -  (99*num)/30;
  else if (IS_NPC(ch))
    prof_factor = 0;
  else 
    prof_factor = 100;
  
  wielded = ch->equipment[WIELD];
  if (type == SKILL_DUAL_WIELD)
    if (wielded = ch->equipment[HOLD])
      if (GET_ITEM_TYPE(wielded) != ITEM_WEAPON)
	return(0);
  
  thaco = 20;
  if (!affected_by_spell(ch, SPELL_ENDURANCE))   
    bonus = combat_bonus(ch);
  thaco -= str_app[GET_STR(ch)].tohit;
  thaco -= bonus;
  
  if (type == SKILL_DUAL_WIELD)   
    thaco -= GET_HITROLL2(ch);
  else
    thaco -= GET_HITROLL(ch);
  
  thaco -= (2*(GET_DEX(ch) - 20))/3;   
  
  if (!IS_NPC(ch) && (GET_LEVEL(ch) < 5))
      thaco -= (GET_LEVEL(ch)*GET_LEVEL(ch))/3;
  
  thaco -= GET_LEVEL(ch)/20;
  thaco *= 5;
  thaco += prof_factor/8;
  thaco += 25;
  return(thaco);
}
int compute_av_hits(int thaco, int ac){
  int total=0,i,percent_hits;
  
  for (i=0;i<1000;i++)
    if (number(0,50) > (thaco - ac/4))
      total++ ;
  
  total *= 100;
  total /= 1000;
  total = MIN(100,total);
  total = MAX(1,total);
    
  return(total);

}
int compute_av_dam(struct char_data *ch, int type){
  int total=0,i,percent_hits;
  
  for (i=0;i<1000;i++)
    total += compute_dam(ch, type);  
  total /= 1000;
  
  return(total);

}
ACMD(do_consider)
{
   struct char_data *victim;

   int	player_no, vict_no, our_hit_ptg, vict_hit_ptg, vict_n_rounds;
   float ratio,mod_lev,weight_diff,height_diff;
   int rm,nattacks,inatt, vict_av_dam, our_av_dam, our_n_rounds;



   one_argument(argument, buf);
  
   if (!(victim = get_char_room_vis(ch, buf))) {
      send_to_char("Consider killing who?\r\n", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("Your mother would be so sad!\r\n", ch);
      return;
   }

   if (GET_LEVEL(ch) >= LEVEL_BUILDER) {
     send_to_char("Are you not a GOD?\r\n",ch);
   }

   if ((!IS_NPC(victim)) && (GET_LEVEL(victim)>=LEVEL_BUILDER)
       && (GET_LEVEL(victim) > GET_LEVEL(ch))) {
      send_to_char("The Gods might not like that!\r\n", ch);
      return;
   }
   
   our_hit_ptg = compute_av_hits(compute_thaco(ch, 0),GET_AC(victim));
   our_av_dam = our_hit_ptg*compute_av_dam(ch, 0);
   
   if (ch->equipment[HOLD] &&
       (GET_ITEM_TYPE(ch->equipment[HOLD]) == ITEM_WEAPON)){
     our_hit_ptg = compute_av_hits(compute_thaco(ch, SKILL_DUAL_WIELD),
				   GET_AC(victim));     
     
     our_av_dam += our_hit_ptg*compute_av_dam(ch, SKILL_DUAL_WIELD);
   }
   our_av_dam /= 100;
   our_av_dam -= GET_BODY_STOPPING(victim);
   
   
   vict_hit_ptg = compute_av_hits(compute_thaco(victim, 0),GET_AC(ch));
   vict_av_dam = vict_hit_ptg*compute_av_dam(victim, 0);

   if (victim->equipment[HOLD] &&
       (GET_ITEM_TYPE(victim->equipment[HOLD]) == ITEM_WEAPON)){
     vict_hit_ptg = compute_av_hits(compute_thaco(victim, SKILL_DUAL_WIELD),
				   GET_AC(ch));     

     vict_av_dam += vict_hit_ptg*compute_av_dam(victim, SKILL_DUAL_WIELD);
   }
   vict_av_dam /= 100;
   vict_av_dam -= GET_BODY_STOPPING(ch);


   if (vict_av_dam > 0)
     vict_n_rounds = GET_HIT(ch)/vict_av_dam;
   else
     vict_n_rounds = GET_HIT(ch)/number(1,2);
   
   if (our_av_dam > 0)
     our_n_rounds = GET_HIT(victim)/our_av_dam;
   else
     our_n_rounds = GET_HIT(victim)/number(1,2);
   if (GET_LEVEL(ch) < 20)
     our_n_rounds += (20 - GET_LEVEL(ch))/2;
   sprintf(buf, "o: %d %d %d",our_av_dam, our_hit_ptg, our_n_rounds);   
   sprintf(buf, "%s v: %d %d %d.\r\n",buf ,vict_av_dam, vict_hit_ptg, vict_n_rounds);
   if (PRF_FLAGGED(ch, PRF_DEBUG))
     send_to_char(buf,ch);
   if (our_n_rounds < 1)
     send_to_char("Your victim is almost dead already!\r\n", ch);
   else if (((float)vict_n_rounds/(float)our_n_rounds) > 10)
      send_to_char("Too easy to be believed!\r\n", ch);
   else if (((float)vict_n_rounds/(float)our_n_rounds) > 5)
     send_to_char("Trivial.\r\n", ch);
   else if (((float) vict_n_rounds/(float) our_n_rounds) > 3)
     send_to_char("Simple.\r\n", ch);
   else if (((float) vict_n_rounds/(float)our_n_rounds) > 2)
     send_to_char("Shouldn't be a problem.\r\n", ch);
   else if (((float)vict_n_rounds)/((float) our_n_rounds) > 1.5)
     send_to_char("You are a bit more skillfull.\r\n", ch);
   else if (((float)vict_n_rounds)/((float) our_n_rounds) > 1.0)
     send_to_char("An even match.\r\n", ch);   
   else if (((float)vict_n_rounds)/((float)our_n_rounds) > .8)
      send_to_char("You would need some luck!\r\n", ch);
   else if (((float)vict_n_rounds)/((float)our_n_rounds) > .6)
      send_to_char("You would need lots of luck!\r\n", ch);
   else if (((float)vict_n_rounds)/((float)our_n_rounds) > .5)
      send_to_char("You would need lots of luck and great equipment!\r\n", ch);
   else if (((float)vict_n_rounds)/((float)our_n_rounds) > .4)
      send_to_char("You probably won't survive!\r\n", ch);   
   else if (((float)vict_n_rounds)/((float)our_n_rounds) > .3)
      send_to_char("You must be CRAZY!!\r\n", ch);   
   else if (((float)vict_n_rounds)/((float)our_n_rounds) > .2)
      send_to_char("MAD you are totaly MAD!!!\r\n", ch);   
   else 
      act("You ARE mad! $E could kill you 12 times over!!!!",FALSE, ch,0,victim,TO_CHAR);
   if ((vict_av_dam > 0) && (our_av_dam < -5)) 
     act("You probably couldn't even scratch $N.",FALSE,ch,0,victim, TO_CHAR);
   else if ((vict_av_dam < -5) && (our_av_dam > 0))
     act("$N probably can't even scratch you.",FALSE,ch,0,victim, TO_CHAR);


   height_diff = (float) GET_HEIGHT(victim)/(float) (1+GET_HEIGHT(ch));

   if (height_diff <= .5)
       act("$N is tiny compared to you.",FALSE,ch,0,victim,TO_CHAR);
   else if (height_diff <= .75)
       act("$N is small compared to you.",FALSE,ch,0,victim,TO_CHAR);
   else if (height_diff <= .9)
       act("$N is a bit smaller than you.",FALSE,ch,0,victim,TO_CHAR);
   else if (height_diff <= 1.1)
       act("$N is about the same height you.",FALSE,ch,0,victim,TO_CHAR);
   else if (height_diff <= 1.25)
       act("$N is taller than you.",FALSE,ch,0,victim,TO_CHAR);
   else 
       act("$N is huge compared to you.",FALSE,ch,0,victim,TO_CHAR);
   weight_diff = (float) GET_WEIGHT(victim)/(float) (1+GET_WEIGHT(ch));
   if (weight_diff < .5)
       act("$N is a feather compared to you.",FALSE,ch,0,victim,TO_CHAR);
   else if (weight_diff <= .75)
       act("$N is much lighter than you.",FALSE,ch,0,victim,TO_CHAR);
   else if (weight_diff <= .9)
       act("$N is much lighter than you.",FALSE,ch,0,victim,TO_CHAR);
   else if (weight_diff <= 1.1)
       act("$N weighs about the same as you.",FALSE,ch,0,victim,TO_CHAR);
   else if (weight_diff <= 1.25)
       act("$N is heavier than you.",FALSE,ch,0,victim,TO_CHAR);
   else if (weight_diff <= 1.75)
       act("$N is much heavier than you.",FALSE,ch,0,victim,TO_CHAR);
   else 
       act("$N might as well be a mountain.",FALSE,ch,0,victim,TO_CHAR);   
   
}


ACMD(do_toggle)
{
   if (IS_NPC(ch))
      return;
   if (WIMP_LEVEL(ch) == 0)
      strcpy(buf2, "OFF");
   else
      sprintf(buf2, "%-3d", WIMP_LEVEL(ch));

   sprintf(buf,
       "Hit Pnt Display: %-3s    "
       "     Brief Mode: %-3s    "
       "  No Conditions: %-3s    "	   
       " Summon Protect: %-3s\r\n"
       " Move Pnt Disp.: %-3s    "
       "   Compact Mode: %-3s    "
       "       On Quest: %-3s\r\n"
       "   Mana Display: %-3s    "
       "         NoTell: %-3s    "
       "   Repeat Comm.: %-3s\r\n"
       "   Auto Display: %-3s    "
       "           Deaf: %-3s	"
       "     Wimp Level: %-3s\r\n",

       ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
       ONOFF(PRF_FLAGGED(ch, PRF_NOCOND)),	   
       ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
       ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),
       ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
       ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
       YESNO(PRF_FLAGGED(ch, PRF_QUEST)),
       ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
       ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
       YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),
       ONOFF(PRF_FLAGGED(ch, PRF_DISPAUTO)),
       YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
       buf2);

   send_to_char(buf, ch);

}


void	sort_commands(void)
{
   int	a, b, tmp;

   ACMD(do_action);

   num_of_cmds = 1;

   while (num_of_cmds < MAX_CMD_LIST && 
       cmd_info[num_of_cmds].command_pointer) {
      cmd_info[num_of_cmds].sort_pos = num_of_cmds - 1;
      cmd_info[num_of_cmds].is_social = 
          (cmd_info[num_of_cmds].command_pointer == do_action);
      num_of_cmds++;
   }

   num_of_cmds--;
   
   for (a = 1; a <= num_of_cmds - 1; a++)
      for (b = a + 1; b <= num_of_cmds; b++)
	 if (strcmp(command[cmd_info[a].sort_pos],
	     command[cmd_info[b].sort_pos]) > 0) {
	    tmp = cmd_info[a].sort_pos;
	    cmd_info[a].sort_pos = cmd_info[b].sort_pos;
	    cmd_info[b].sort_pos = tmp;
	 }
}



ACMD(do_commands)
{
   int	no, i, cmd_num;
   int	wizhelp = 0, socials = 0;
   struct char_data *vict;

   one_argument(argument, buf);

   if (*buf) {
      if (!(vict = get_char_vis(ch, buf)) || IS_NPC(vict)) {
	 send_to_char("Who is that?\r\n", ch);
	 return;
      }
      if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
	 send_to_char("Can't determine commands of people above your level.\r\n", ch);
	 return;
      }
   } else
      vict = ch;

   if (subcmd == SCMD_SOCIALS)
      socials = 1;
   else if (subcmd == SCMD_WIZHELP)
      wizhelp = 1;

   sprintf(buf, "The following %s%s are available to %s:\r\n",
       wizhelp ? "privileged " : "",
       socials ? "socials" : "commands",
       vict == ch ? "you" : GET_NAME(vict));

   for (no = 1, cmd_num = 1; cmd_num <= num_of_cmds; cmd_num++) {
      i = cmd_info[cmd_num].sort_pos;
      if (cmd_info[i+1].minimum_level >= 0 && 
          (cmd_info[i+1].minimum_level >= LEVEL_BUILDER) == wizhelp && 
          GET_LEVEL(vict) >= cmd_info[i+1].minimum_level && 
          (wizhelp || socials == cmd_info[i+1].is_social)) {
	 sprintf(buf + strlen(buf), "%-15s", command[i]);
	 if (!(no % 5))
	    strcat(buf, "\r\n");
	 no++;
      }
   }

   strcat(buf, "\r\n");
   page_string(ch->desc,buf, 1);
}



ACMD(do_diagnose)
{
   struct char_data *vict;

   one_argument(argument, buf);

   if (*buf) {
      if (!(vict = get_char_room_vis(ch, buf))) {
	 send_to_char("No-one by that name here.\r\n", ch);
	 return;
      } else
	 diag_char_to_char(vict, ch);
   } else {
      if (ch->specials.fighting)
	 diag_char_to_char(ch->specials.fighting, ch);
      else
	 send_to_char("Diagnose who?\r\n", ch);
   }
}


