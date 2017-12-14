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
#include <string.h>
#include <ctype.h>
#include <time.h>

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
extern int    top_of_world;
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct command_info cmd_info[];
extern struct index_data *mob_index;
extern struct dual_list_type nonreg_plurals[];
extern char     *damage_state[];
extern char	*credits;
extern char	*news;
extern char	*info;
extern char	*wizlist;
extern char	*immlist;
extern char	*policies;
extern char	*handbook;
extern char	*building;
extern char	*motd;
extern char	*imotd;
extern char	*dirs[];
extern char	*where[];
extern char	*where_animal[];
extern char	*where_plant[];
extern char	*where_equine[];
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

int compute_room_light_value(struct room_data *room);
char    *report_cost(int gold);
char    *report_gold(int gold);
int     report_money_weight(int amount);
int     report_highest_value_for_weight(int weight);
int     find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);
void    parse_text(struct char_data *c,struct char_data *vict, int mode, char *text);
int     report_crowns(int amount);
int     report_groats(int amount);
int     report_pennies(int amount);
char    *make_plural(char *arg);
char    *first_name(char *arg);
char    *pluralise_string(char *arg);
char *find_spell_name(int spl);
char *find_skill_name(int spl);
char buf_crwn[240];
/* intern functions & vars*/
int	num_of_cmds;
void	list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, bool show);
void	show_char_to_char(struct char_data *i, struct char_data *ch, int mode);
int     can_see_hidden(struct char_data *sub, struct char_data *obj);
void    show_cart_to_char(struct obj_data *object, struct char_data *ch, int mode);
void   format_desc(char *string, char *buf_p, int max_l);
void  process_room_descr(char *in, char *out, byte llev);
/* Procedures related to 'look' */
void format_desc(char *string, char *buf_p, int max_lin)
{
  char *p, *pp, line[100];
  int i, min_lin=100;
  
  bzero(line,100);
  strcpy(line,"   ");
  for (i = 0;*(string + i) == ' ';i++)
    ;
  *buf_p = '\0';
  p = string +i;
  pp = line + 3;
  while((*p != '\0') &&  (p <= string + strlen(string)))
    {
      if ((*p == ' ') && (*(p + 1) == ' ') && (*(p -1) == ' ' || (*(p + 2) == ' '))) {
	for (;((*p == ' ') && (*p != '\0'));p++)	  
	  ;
	if ((pp - line) < min_lin)
	  min_lin = pp - line;
	strcat(line, "\r\n");
	strcat(buf_p, line);
	bzero(line,100);
	strcpy(line,"   ");
	pp = line + 3;
	continue;
      }
      if (((*p == '\r') && (*(p+1)  == '\n')) ||
	  (((*p == '\n') && (*(p+1)  == '\r')))){
	if ((pp - line > 75) && (*(p+2) == '\0')) {
	  p += 2;
	  if ((pp - line) < min_lin)
	    min_lin = pp - line;
	  strcat(line, "\r\n");
	  strcat(buf_p, line);
	  bzero(line,100);
	  pp = line;
	  continue;
	}
	if (*(p-1) != ' ') 
	  *pp++ = ' ';
	if ((*(p-1) == '.') || (*(p-1) == '?') || (*(p-1) == '!'))
	  *pp++ = ' ';
	p += 2;
	for (i=1 ;((*(p+i) != ' ') && (*(p+i) != '\n') && (*(p+i) != '\r')
		   &&  (*(p+i) != '\0'));i++)
	  ; 
	if ((pp - line + i) > max_lin) {
	  for (;((*p == ' ')  && (*p != '\0'));p++)
	    ;
	  if ((pp - line) < min_lin)
	    min_lin = pp - line;
	  strcat(line, "\r\n");
	  strcat(buf_p, line);
	  bzero(line,100);
	  pp = line;
	  continue;
	}
      }
      *pp++ = *p++;
      if (*p == ' ') {
	if ((*(p-1) != '.') && ((*p+1) == ' '))
	  for (;((*p == ' ') && (*p != '\0'));p++)
	    ;	  
	for (i=1 ;((*(p+i) != ' ') && (*(p+i) != '\n') && (*(p+i) != '\r')
		   &&  (*(p+i) != '\0'));i++)
	  ; 
	if ((pp - line + i) >= max_lin) {
	  for (;((*p == ' ') && (*p != '\0'));p++)
	    ;
	  if ((pp - line) < min_lin)
	    min_lin = pp - line;
	  strcat(line, "\r\n");
	  strcat(buf_p, line);
	  bzero(line,100);
	  pp = line;
	}
      }
    }
  if ((pp - line) < min_lin && (pp > line))
    min_lin = pp - line;
  strcat(buf_p, line);
  if ((*(buf_p + strlen(buf_p) -2) != '\r')
      && (*(buf_p + strlen(buf_p) -2) != '\n'))
    strcat(buf_p,"\r\n");
  if ((min_lin < 9) && (max_lin > 71))
    format_desc(string, buf_p, max_lin -2);
}

void	argument_split_2(char *argument, char *first_arg, char *second_arg)
{
   int	look_at, found, begin;
   found = begin = 0;

   if (!argument)
       return;
   /* Find first non blank */
   for ( ; *(argument + begin ) == ' ' ; begin++)
      ;
 
   /* Find length of first word */
   for (look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)

      /* Make all letters lower case, AND copy them to first_arg */
      *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
   *(first_arg + look_at) = '\0';
   begin += look_at;

   /* Find first non blank */
   for ( ; *(argument + begin ) == ' ' ; begin++)
      ;

   /* Find length of second word */
   for ( look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)

      /* Make all letters lower case, AND copy them to second_arg */
      *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
   *(second_arg + look_at) = '\0';
   begin += look_at;
}



char	*find_ex_description(char *word, struct extra_descr_data *list)
{
   struct extra_descr_data *i =0;

   for (i = list; i; i = i->next)
       
      if (i->keyword && isname(word, i->keyword))
	 return(i->description);

   return(0);
}
void	show_scabbard_to_char(struct obj_data *scabbard, struct char_data *ch, int mode)
{

  if (IS_OBJ_STAT(scabbard, ITEM_INVISIBLE)) 
    strcat(buf, "(invisible)");
  if (!scabbard->contains)
    strcat(buf, "..empty.");
  else {
    strcat (buf, "..containing\r\n                        ... ");
    if (!CAN_SEE_OBJ(ch, scabbard->contains))
      strcat (buf, "something.");
    else
      strcat (buf, ((scabbard->contains->short_description) ? scabbard->contains->short_description : "a weapon."));
  }
    if (*buf)
      page_string(ch->desc, buf, 1);    
}

void	show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
   bool found;
   int where;
   
   *buf = '\0';
   if (mode == 0){
       if (object->description)
	   strcat(buf,(CAN_SEE_OBJ(ch, object) ?
		       object->description : "something"));
   }
   else if (object->short_description
	    && ((mode == 1) || 
		(mode == 2) ||
		(mode == 3) || (mode == 4)))
       strcat(buf, (CAN_SEE_OBJ(ch, object) ? object->short_description: "something"));
   else if (mode == 5) {
      if (object->obj_flags.type_flag == ITEM_NOTE) {
	 if (object->action_description) {
	    strcpy(buf, "There is something written upon it:\r\n\r\n");
	    strcat(buf, object->action_description);
	    page_string(ch->desc, buf, 1);
	 } else
	    act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      } else if ((object->obj_flags.type_flag != ITEM_DRINKCON)) {
	 strcpy(buf, "You see nothing special..");
      } else /* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
	 strcpy(buf, "It looks like a drink container.");
   }
   if (mode != 3) {
     if (GET_ITEM_TYPE(object) == ITEM_SCABBARD) {
       show_scabbard_to_char(object, ch, mode);
       return;
     }
      found = FALSE;
      if (IS_OBJ_STAT(object, ITEM_EVIL) && IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	 strcat(buf, "..reeking of evil!");
	 found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
	 strcat(buf, "..it smells of magic!");
	 found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_GLOW) && (object->obj_flags2.light != 0)) {
	  if ((object->obj_flags2.light+3)/3 > 5)
	      strcat(buf, "..brilliantly shining!");
	  else
	      switch((object->obj_flags2.light+3)/4){
	      case 5:
		  strcat(buf, "..brilliantly shining!");
		  break;
	      case 4:
		  strcat(buf, "..shining!");
		  break;
	      case 3:
		  strcat(buf, "..glowing strongly.");
		  break;
	      case 2:
		  strcat(buf, "..glowing.");
		  break;
	      case 1:
		  strcat(buf, "..glowing faintly.");
	      case 0:
		  break;
	      case -1:
		  strcat(buf, "..casting faint shadows.");
		  break;
	      case -2:
		  strcat(buf, "..cloaked in shadow.");
		  break;
	      case -3:
		  strcat(buf, "..wreathed in darkness.");
		  break;
	      case -4:
		  strcat(buf, "..exuding tendrils of darkness!");
		  break;
	      case -5:
		  strcat(buf, "..enveloped in blackness!");
		  break;	      	      
	  }
	 found = TRUE;
      }
      if (IS_OBJ_STAT(object, ITEM_HUM)) {
	 strcat(buf, "..faintly humming.");
	 found = TRUE;
      }
      where = find_eq_pos(ch, object, 0);
      if (where >0 && where != HOLD && where != WEAR_SCABBARD )
	  if (!PRF_FLAGGED(ch, PRF_NOCOND)){
	      if (object->obj_flags.value[4] >= 0)
		  strcat(buf,damage_state[object->obj_flags.value[4]]);
	      else
		  strcat(buf,"..in stunning condition.");
	  }
      if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
	 strcat(buf, "(invisible)");
	 found = TRUE;
      }
   }
   if (*buf)
       page_string(ch->desc, buf, 1);
}


void	list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, 
bool show)
{
   bool found;
   struct obj_data *i,*ii,*iii,*iv,*v;

   /*
     sort the list putting objs with the same
     virtual number next to each other
     */
   for (i = list; i; i = i->next_content)
       {
	   if(i->next_content)
	       for (ii = i->next_content; ii; ii = ii->next_content)
		   if (i->description && ii->description &&
		       (ii != i->next_content) && 
		       !(strcmp(ii->description,i->description)))
		       {
			   iii = ii->next_content;
			   iv = i->next_content;
			   for (v = i;v->next_content != ii;v = v->next_content)
			       ;
			   v->next_content = iii;
			   i->next_content = ii;
			   ii->next_content = iv;
			   break;
			   
		       }
       }

   found = FALSE;
   for ( i = list ; i ; i = i->next_content ) {
       if (CAN_SEE_OBJ(ch, i)){
	 
	 if (i->pulled_by)
	   show_cart_to_char(i, ch, mode);
	 else if ((GET_ITEM_TYPE(i) == ITEM_CONTAINER) && (HASROOM(i)) &&
		  (ch->in_room == real_room(i->obj_flags.value[3])))
	   continue;
	 else
	   show_obj_to_char(i, ch, mode);
	 found = TRUE;
       }
   }

   if ((!found) && (show))
       send_to_char("Nothing.\r\n", ch);
}

void show_cart_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
    struct follow_type *k;
    char buf_loc[256];
    char tmp_loc[256];
    int n = 0;
    bzero(buf_loc,256);
    for (k = object->pulled_by;k;k = k->next){
	show_char_to_char(k->follower,ch,mode);
	strcat(buf_loc,
	       first_name(k->follower->player.name ? k->follower->player.name :
		   k->follower->player.short_descr));
	strcat(buf_loc, " ");
	n++;}
    if (n > 1)
	sprintf(tmp_loc,"The %s are hitched to $p.",pluralise_string(buf_loc));
    else{	
	half_chop(buf_loc,buf,buf1);
	sprintf(tmp_loc,"The %s is hitched to $p.",buf);}
    act(tmp_loc,TRUE,ch,object,0,TO_CHAR);

}
char *pluralise_string(char *arg_loc)
{
    char buf1[256], buf2[256], buf2p[256], result[256];
    char *pt, *tmpp;
    int i;
    pt = arg_loc;
    if (!pt)
      return(pt);
    bzero(result,256);    
    while(TRUE){
	if (!pt || *pt == '\0')
	    break;
	half_chop(pt,buf2,buf1);
	pt = buf1;
	strcpy(buf2p,make_plural(buf2));
	if (!strstr(result,buf2) && !strstr(result, buf2p)){
	    i=0;
	    tmpp = pt;
	    while ((tmpp = strstr(tmpp,buf2))){
		for(;*tmpp != ' ' && *tmpp != '\0';tmpp++)
		    ;
		i++;
	    }
	    if (i > 0)
		strcat(result,buf2p);
	    else
		strcat(result,buf2);
	    strcat(result, " and ");
	}
    }
    if (result && strlen(result) > 5) {
      bzero(buf1,256);
      strncpy(buf1,result, strlen(result) -5);
      strcpy(buf,buf1);
      return(buf);
    }
    return(0);
}  


void	diag_char_to_char(struct char_data *i, struct char_data *ch)
{
   int	percent;

   if (GET_MAX_HIT(i) > 0)
      percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
   else
      percent = -1; /* How could MAX_HIT be < 1?? */

   if (CAN_SEE(ch,i))
     strcpy(buf, GET_NAME(i));
   else {
     if (CAN_SPEAK(i))
       strcpy(buf, "someone");
     else
       strcpy(buf, "something");
   }
   CAP(buf);

   if (percent >= 100)
      strcat(buf, " is unhurt.\r\n");
   else if (percent >= 90)
      strcat(buf, " has a few scratches.\r\n");
   else if (percent >= 75)
      strcat(buf, " has some small wounds and bruises.\r\n");
   else if (percent >= 50)
      strcat(buf, " has quite a few wounds.\r\n");
   else if (percent >= 30)
      strcat(buf, " has some big nasty wounds and scratches.\r\n");
   else if (percent >= 15)
      strcat(buf, " looks pretty hurt.\r\n");
   else if (percent >= 0)
      strcat(buf, " is in awful condition.\r\n");
   else
      strcat(buf, " is bleeding awfully from big wounds.\r\n");

   send_to_char(buf, ch);

   if (GET_MAX_MOVE(i) > 0)
      percent = (100 * GET_MOVE(i)) / GET_MAX_MOVE(i);
   else
      percent = -1; /* How could MAX_HIT be < 1?? */

   if (CAN_SEE(ch,i))
     strcpy(buf, GET_NAME(i));
   else if (CAN_SPEAK(i))
     strcpy(buf, "someone");
   else
     strcpy(buf, "something");
   CAP(buf);
   if (percent >= 90)
     strcat(buf, " looks fully rested.\r\n");
   else if (percent >= 75)
     strcat(buf, " seems a little weary.\r\n");
   else if (percent >= 50)
     strcat(buf, " looks tired.\r\n");
   else if (percent >= 30 )
     strcat(buf, " looks very tired.\r\n");
   else if ( percent >= 15 )
     strcat(buf, " looks extremely tired.\r\n");
   else if ( percent >= 0 )
     strcat(buf, " looks nearly exhausted.\r\n");
   else
     strcat(buf, " is completely exhausted!\r\n");
   
   send_to_char(buf, ch);
}


void	show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
   int	j,jj, found;
   struct obj_data *tmp_obj;
   bool has_boat=FALSE;
   const char *pos;
   const int loc[]={6,18,19,20,21,3,4,5,22,10,1,2,14,15,9,
		    11,23,0,16,17,12,13,7,8};
   for (tmp_obj = i->inventory; tmp_obj; tmp_obj= tmp_obj->next_content)
       if (tmp_obj->obj_flags.type_flag == ITEM_BOAT)
	   has_boat = TRUE;
   
   if (mode == 0) {

      if ((IS_AFFECTED(i, AFF_HIDE) && !can_see_hidden(ch,i))
	  || !CAN_SEE(ch, i)) {
	 if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
	    send_to_char("You catch a glimpse of something or someone.\r\n", ch);
	 return;
      }

      if (!(i->player.long_descr)
	  || (GET_POS(i) != i->specials.default_pos)
	  || affected_by_spell(i, SPELL_ENCASE_IN_ICE)
	  || affected_by_spell(i, SPELL_WEB)
	  || i->specials.fighting
	  || i->specials.mount
	  || i->specials.carrying
	  || MOB_FLAGGED(i,MOB_TETHERED)
	  || i->specials.rider) {
	 /* A player char or a mobile without long descr,
	    or not in default position, or fighting */
	  if (!IS_NPC(i)){
	    if (!i->specials.mount){
	      parse_text(i,ch,0,buf2);
	      sprintf(buf, "%s", buf2);
	    }
	    else
	      sprintf(buf, "%s", i->player.name);	      
	  }	      
	  else {
	      strcpy(buf, i->player.short_descr);
	      CAP(buf);
	      if (MOB_FLAGGED(i,MOB_CITIZEN))
		  strcat(buf, " (citizen)");
	      if (MOB_FLAGGED(i,MOB_DOCILE))
		  strcat(buf, " (tame)");	      
	      if (IS_AFFECTED(i, AFF_HIDE))
		  strcat(buf," (hiding)");
	  }
	  if (MOB_FLAGGED(i, MOB_TETHERED)) {
	    strcat(buf, " is tethered here.");
	  } 
	  else if (i->specials.mount
		   || i->specials.carrying
		   || affected_by_spell(i, SPELL_ENCASE_IN_ICE)
		   || affected_by_spell(i, SPELL_WEB)) {
	    strcat(buf, " is here");
	  }
	  else {
	    switch (GET_POS(i)) {
	    case POSITION_STUNNED  :
	      strcat(buf, " is lying here, stunned.");
	      break;
	    case POSITION_INCAP    :
	      strcat(buf, " is lying here, incapacitated.");
	      break;
	    case POSITION_MORTALLYW:
	      strcat(buf, " is lying here, mortally wounded.");
	      break;
	    case POSITION_DEAD     :
	      strcat(buf, " is lying here, dead.");
	      break;
	    case POSITION_STANDING :
	      if (world[i->in_room].sector_type
		  == SECT_UNDER_WATER
		  || world[i->in_room].sector_type
		  == SECT_WATER_SWIM
		  || (world[i->in_room].sector_type
		      == SECT_WATER_NOSWIM && !has_boat))
		strcat(buf, " is swimming here.");
	      else if (IS_AFFECTED(i, AFF_FLY))
		strcat(buf, " is hovering here.");
	      else
		strcat(buf, " is standing here.");
	      break;
	    case POSITION_SITTING  :
	      strcat(buf, " is sitting here.");
	      break;
	    case POSITION_RESTING  :
	      strcat(buf, " is lying here.");
	      break;
	    case POSITION_SLEEPING :
	      strcat(buf, " is sleeping here.");
	      break;
	    default :
	      strcat(buf, " is floating here.");
	      break;
	    }
	  }
	  if (i->specials.rider)
	    sprintf(buf, "%s (%sridden%s)",buf,
		    CCBBLU(ch,C_NRM),CCNRM(ch,C_NRM));
	  if (IS_AFFECTED(i, AFF_INVISIBLE))
	    sprintf(buf, "%s (%sinvis%s)",buf,CCWHT(ch,C_NRM),CCNRM(ch,C_NRM));
	  if (IS_AFFECTED(i, AFF_HIDE))
	    sprintf(buf, "%s (%shide%s)",buf,CCWHT(ch,C_NRM),CCNRM(ch,C_NRM));
	  if (!IS_NPC(i) && !i->desc)
	    sprintf(buf, "%s (%sLD%s)", buf, CCWHT(ch,C_NRM),CCNRM(ch,C_NRM));
	  if (PLR_FLAGGED(i, PLR_WRITING))
	    sprintf(buf, "%s (%swriting%s)", buf, CCGRN(ch,C_NRM),
		    CCNRM(ch,C_NRM));	      
	  if (PLR_FLAGGED(i, PLR_BUILDING))
	    sprintf(buf, "%s (%sbuilding%s)", buf, CCYEL(ch,C_NRM),
		    CCNRM(ch,C_NRM));	      
	  if (PLR_FLAGGED(i, PLR_AFK))
	    sprintf(buf,"%s (%saway%s)",buf,CCCYN(ch,C_NRM),CCNRM(ch,C_NRM));
	  if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) 
	    sprintf(buf,"%s (%sevil%s)",buf,CCRED(ch,C_NRM),CCNRM(ch,C_NRM));
	  if (i->specials.mount) {
	    strcat(buf, " riding on ");
	    if (!IS_NPC(i->specials.mount))
	      strcat(buf, i->specials.mount->player.name);
	    else
	      strcat(buf, i->specials.mount->player.short_descr);
	    strcat(buf, "'s back.\r\n");
	  }
	  else if (i->specials.carrying) {
	    strcat(buf, " carrying ");	    
	    if (!IS_NPC(i->specials.carrying))
	      strcat(buf, i->specials.carrying->player.name);
	    else
	      strcat(buf, i->specials.carrying->player.short_descr);
	    strcat(buf, ".\r\n");
	  }
	  else if(affected_by_spell(i, SPELL_ENCASE_IN_ICE))
	    strcat(buf, " encased in a block of ice.\r\n");
	  else if(affected_by_spell(i, SPELL_WEB))
	    strcat(buf, " entangled in sticky webs.\r\n");
	  else
	    strcat(buf, "\r\n");
      }
      else { /* npc with long */
	*buf = '\0';
	strcat(buf, i->player.long_descr);
	if (IS_AFFECTED(i, AFF_INVISIBLE))
	  sprintf(buf, "%s%s*%s\r\n",strtok(buf,"\r\n"),CCBWHT(ch,C_NRM),
		  CCNRM(ch,C_NRM));
	if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	  if (IS_EVIL(i))
	    sprintf(buf,"%s%s+%s\r\n",strtok(buf,"\r\n"),CCRED(ch,C_NRM),
		    CCNRM(ch,C_NRM));
	}
	if (IS_NPC(i) && MOB_FLAGGED(i,MOB_DOCILE))
	  strcat(strtok(buf,"\r\n"), " (tame)\r\n");
	if (IS_NPC(i) && MOB_FLAGGED(i,MOB_CITIZEN))
	  strcat(strtok(buf,"\r\n"), " (citizen)\r\n");
	strcat(strtok(buf,"\r\n"), "\r\n");
      }
      if (i->specials.fighting) {
	  strcat(strtok(buf,"\r\n"), "..fighting ");
	  if (i->specials.fighting == ch)
	      strcat(buf, "YOU!\r\n");
	  else {
	      if (i->in_room == i->specials.fighting->in_room){
		  strcat(buf, ((i->specials.fighting->player.short_descr)
		      ? i->specials.fighting->player.short_descr:
		      ((i->specials.fighting->player.name) ?
		       i->specials.fighting->player.name: "something")));
		  strcat(buf, ".\r\n");
	      }
	      else
		  strcat(buf, "someone who has already left.\r\n");
	  }
      }
      send_to_char(buf, ch);
      if (i->specials.mount)
	show_char_to_char(i->specials.mount,ch,mode);
      if (!i->specials.rider) {
	if (IS_AFFECTED(i, AFF_SANCTUARY))
	  act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
      if (!IS_NPC(i)){
	  if (IS_AFFECTED(i, AFF_RESIST_HEAT))
	      act("$n glows with a pink light!", FALSE, i, 0, ch, TO_VICT);
	  if (IS_AFFECTED(i, AFF_RESIST_COLD))
	      act("$n glows with a blue light!", FALSE, i, 0, ch, TO_VICT);}
      }
      
   } else if (mode == 1) {
       
       if (i->player.description)
	   send_to_char(i->player.description, ch);
       else {
	   act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
       }

       /* Show a character to another */
       
       diag_char_to_char(i, ch);
       
       found = FALSE;
       if (i->specials.carrying)
	 found = TRUE;
       for (j = 0; j < MAX_WEAR; j++) {
	 if (found)
	   break;
	 if (i->equipment[j]) 
	   if (CAN_SEE_OBJ(ch, i->equipment[j]) )
	     found = TRUE;
       }
       if (found) {
/* 	switch (GET_RACE(i)){ */
/* 	case CLASS_ANIMAL: */
/* 	  pos = where_animal[0]; */
/* 	  break; */
/* 	case CLASS_PLANT: */
/* 	  pos = where_plant[0]; */
/* 	  break; */
/* 	case CLASS_EQUINE: */
/* 	  pos = where_equine[0]; */
/* 	  break;	   */
/* 	case CLASS_OTHER: */
/* 	case CLASS_HUMANOID: */
/* 	case CLASS_UNDEAD: */
/* 	default: */
/* 	  pos = where[0]; */
/* 	} */
	act("\r\n$n has:", FALSE, i, 0, ch, TO_VICT);
	jj = 1;
	for (j = 0; j < MAX_WEAR; j++) {
	  if ((loc[j] == HOLD) && i->specials.carrying) {
	    sprintf(buf, "[%2i] <carried>         %s\r\n",jj,
	       (IS_NPC(i->specials.carrying) ?
		i->specials.carrying->player.short_descr :
		i->specials.carrying->player.name));
	    send_to_char(buf,ch);
	  }
	  if (i->equipment[loc[j]]) {
	    if (CAN_SEE_OBJ(ch, i->equipment[loc[j]])) {
	      if ((loc[j] == HOLD &&
		   (is_held_obj_twohanded(i,i->equipment[loc[j]]))) ||
		  (loc[j] == WIELD &&
		   (is_two_handed(i,i->equipment[loc[j]]))))
		sprintf(buf, "[%2i]%s ",jj,"<both hands>      ");
	      else
		sprintf(buf,"[%2i]%s ",jj,where[loc[j]]);
	      send_to_char(buf, ch);
	      show_obj_to_char(i->equipment[loc[j]], ch, 1);
	      jj++;
	    }
	  }
	}
      }
      if (((GET_PER(ch) >= number(1,30)) || GET_LEVEL(ch) >= LEVEL_BUILDER) && (ch != i)) {
	found = FALSE;
	send_to_char("\r\nYou attempt to peek at the inventory:\r\n", ch);
	for (tmp_obj = i->inventory; tmp_obj; tmp_obj = tmp_obj->next_content) {
	  if (number(0, 120) < (GET_LEVEL(ch) + 3*GET_PER(ch))) {
	    show_obj_to_char(tmp_obj, ch, 1);
	    found = TRUE;
	  }
	}
	if (!found)
	  send_to_char("You can't see anything.\r\n", ch);
      }
      
   } else if (mode == 2) {

       /* Lists inventory */
       act("$n has in $s inventory:", FALSE, i, 0, ch, TO_VICT);
       list_obj_to_char(i->inventory, ch, 1, TRUE);
   }
}

int can_see_hidden(struct char_data *sub, struct char_data *obj)
{
  if(!obj)
    return(FALSE);
  
  if ((GET_PER(sub) >= (GET_LEVEL(obj) - GET_LEVEL(sub))/20 + 20)
      && number(0,60) <= GET_PER(sub))
    return(TRUE);
  else
    return(FALSE);
}

void	list_char_to_char(struct char_data *list, struct char_data *ch, 
int mode)
{
   struct char_data *i,*ii,*iii,*iv,*v;

   /*
     sort the list putting mobs with the same
     virtual number next to each other
     */
   for (i = list; i; i = i->next_in_room)
     {
       if(IS_MOB(i))
	 if(i->next_in_room)
	   for (ii = i->next_in_room; ii; ii = ii->next_in_room)
	     if (i->player.long_descr && ii->player.long_descr
		 && (ii != i->next_in_room) &&
		 !(strcmp(ii->player.long_descr,i->player.long_descr)))
	       {
		 iii = ii->next_in_room;
		 iv = i->next_in_room;
		 for (v = i;v->next_in_room != ii;v = v->next_in_room)
		   ;
		 v->next_in_room = iii;
		 i->next_in_room = ii;
		 ii->next_in_room = iv;
		 break;
		 
	       }
     }
   
   for (i = list; i; i = i->next_in_room)
     if (ch != i) {
       if ((CAN_SEE(ch, i) && !IS_PULLING(i)
	    && (!i->specials.carried_by)
	    && (!(i->specials.rider) || (i->specials.rider == ch)) &&	      
            ((IS_AFFECTED(ch, AFF_SENSE_LIFE) || !IS_AFFECTED(i, AFF_HIDE))
	     || (IS_AFFECTED(i, AFF_HIDE) && can_see_hidden(ch,i)))))
	 show_char_to_char(i, ch, 0);
       else if ((IS_DARK(ch->in_room)) && (IS_AFFECTED(i, AFF_INFRARED)))
	 send_to_char("You see a pair of glowing red eyes looking your way.\r\n", ch);
     }
}
ACMD(do_motd)
{
    one_argument(argument,arg);
    if (GET_LEVEL(ch) < LEVEL_BUILDER)
	page_string(ch->desc,motd,1);
    else if (!(*arg))
	page_string(ch->desc,imotd,1);
    else if (!strcmp("mortal",arg) || !strcmp("motd",arg))
	page_string(ch->desc,motd,1);
    else
	page_string(ch->desc,imotd,1);
}

ACMD(do_attributes)
{
   int	nsp, bonus;
   char *name;
   struct affected_type *aff;
   extern struct str_app_type str_app[];
   if (IS_MOB(ch))
       return;
   bonus = combat_bonus(ch);
   sprintf(buf,"You are: ");
   send_to_char(buf,ch);
   if (ch->player.title){
     parse_text(ch,ch,0,buf2);
     sprintf(buf, "%s.\r\n", buf2);
   }
   else
       sprintf(buf, " %s.\r\n",ch->player.name); 
   send_to_char(buf, ch);
   if (GET_LEVEL(ch) < LEVEL_BUILDER){
       if (!ch->specials.fighting){
	   sprinttype(GET_POS(ch), position_types, buf2);
	   sprintf(buf, "You are %s.\r\n", buf2);}
       else
	   sprintf(buf, "%sYou are fighting!%s\r\n",  CCRED(ch, C_NRM), CCNRM(ch, C_NRM));
       send_to_char(buf, ch);
       
       strcpy(buf, "Race: ");
       sprinttype(ch->player.race, pc_race_types, buf2);
       strcat(buf, buf2);
       send_to_char(buf,ch);
       if (GET_ALIGNMENT(ch) > 950)
	 sprintf(buf,"Saintly");
       else if (GET_ALIGNMENT(ch) > 850)
	 sprintf(buf,"Pious");
       else if (GET_ALIGNMENT(ch) > 600)
	 sprintf(buf,"Righteous");
       else if (GET_ALIGNMENT(ch) > 450)
	 sprintf(buf,"Upright");
       else if (GET_ALIGNMENT(ch) > 350)
	 sprintf(buf,"Moral");
       else if (GET_ALIGNMENT(ch) > 250)
	 sprintf(buf,"Wayward");
       else if (GET_ALIGNMENT(ch) > 0)
	 sprintf(buf,"Perverse");
       else if (GET_ALIGNMENT(ch) > -250)
	 sprintf(buf,"Bad");
       else if (GET_ALIGNMENT(ch) > -350)
	 sprintf(buf,"Amoral");
       else if (GET_ALIGNMENT(ch) > -450)
	 sprintf(buf,"Cruel");
       else if (GET_ALIGNMENT(ch) > -600)
	 sprintf(buf,"Debauched");
       else if (GET_ALIGNMENT(ch) > -850)
	 sprintf(buf,"Wicked");
       else if (GET_ALIGNMENT(ch) > -950)
	 sprintf(buf,"Evil");       
       else 
	 sprintf(buf,"Diabolic");       
       
       sprintf(buf2, ", XP: [%s%7d%s], Align: %s%s%s, Pracs [%s%5d%s]\r\n",
	       CCYEL(ch, C_NRM), GET_EXP(ch), CCNRM(ch, C_NRM),
	       CCYEL(ch, C_NRM), buf,CCNRM(ch, C_NRM),
	       CCYEL(ch, C_NRM), SPELLS_TO_LEARN(ch),CCNRM(ch, C_NRM));
       send_to_char(buf2, ch);
       sprintf(buf,"Height: %s%2d'%2d''%s, Weight: %s%4d lb%s, ",
	       CCYEL(ch, C_NRM), (int) (100*GET_HEIGHT(ch)/254)/12,
	        (int)(100*GET_HEIGHT(ch)/254)-12*((100*GET_HEIGHT(ch)/254)/12),
	       CCNRM(ch, C_NRM),
	       CCYEL(ch, C_NRM), GET_WEIGHT(ch)/10, CCNRM(ch, C_NRM) 
       );
       send_to_char(buf, ch);
       sprintf(buf, "Carried Weight: %s%4d%s, items: %s%3d%s.\r\n",
	       CCYEL(ch, C_NRM), IS_CARRYING_W(ch)/10,CCNRM(ch, C_NRM),
	       CCYEL(ch, C_NRM), IS_CARRYING_N(ch),CCNRM(ch, C_NRM)
	   );
       send_to_char(buf,ch);
       switch(encumberance_level(ch)){
       case 0:
	 sprintf(buf, "%sYou are unencumbered.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;
       case 1:
	 sprintf(buf, "%sYou are lightly loaded.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;
       case 2:
	 sprintf(buf, "%sYou are slightly encumbered.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;	 
       case 3:
	 sprintf(buf, "%sYou are moderately encumbered.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;	 
       case 4:
	 sprintf(buf, "%sYou are encumbered.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;	 	 
       case 5:
	 sprintf(buf, "%sYou are heavily loaded.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;	 	 
       case 6:
	 sprintf(buf, "%sYou are close to your carrying limit.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;	 	 
       default:
	 sprintf(buf, "%sMinor Bug please report this.\r\n%s",
		 CCBBLU(ch, C_NRM),CCNRM(ch, C_NRM));
	   break;	 	 
       }
       send_to_char(buf,ch);
       send_to_char(CCBGRN(ch,C_NRM), ch);   
       send_to_char("\r\n                         Current/Base Stats:\r\n",ch);
       send_to_char(CCNRM(ch,C_NRM), ch);
       sprintf(buf, " \r\n  Str: [%s%2d/%2d%s]  Int: [%s%2d/%2d%s]  Wis: [%s%2d/%2d%s]  Dex: [%s%2d/%2d%s]  Foc: [%s%2d/%2d%s]\r\n",
	       CCBCYN(ch, C_NRM), GET_STR(ch), GET_RAW_STR(ch), CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM), GET_INT(ch), GET_RAW_INT(ch), CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM), GET_WIS(ch), GET_RAW_WIS(ch), CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM), GET_DEX(ch), GET_RAW_DEX(ch), CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM), GET_FOC(ch), GET_RAW_FOC(ch), CCNRM(ch, C_NRM));
       send_to_char(buf, ch);
       sprintf(buf, "  Con: [%s%2d/%2d%s]  Chr: [%s%2d/%2d%s]  Per: [%s%2d/%2d%s]  Gui: [%s%2d/%2d%s] \r\n",
	       CCBCYN(ch, C_NRM), GET_CON(ch), GET_RAW_CON(ch), CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM), GET_CHR(ch), GET_RAW_CHR(ch), CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM), GET_PER(ch), GET_RAW_PER(ch), CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM), GET_GUI(ch), GET_RAW_GUI(ch), CCNRM(ch, C_NRM));
       send_to_char(buf, ch);
       sprintf(buf, " \r\n          Hit  p.: [%s%4d/%4d%s]     Move p.: [%s%4d/%4d%s]\r\n \r\n",
	  CCBGRN(ch, C_NRM), GET_HIT(ch), GET_MAX_HIT(ch), CCNRM(ch, C_NRM),
	  CCBGRN(ch, C_NRM), GET_MOVE(ch), GET_MAX_MOVE(ch), CCNRM(ch, C_NRM));
       send_to_char(buf, ch);     
       sprintf(buf, "           Body AC/St: [%s%d/%d%s]       Arm  AC/St: [%s%d/%d%s]\r\n",
       CCBWHT(ch, C_NRM),GET_BODY_AC(ch),GET_BODY_STOPPING(ch),CCNRM(ch,C_NRM),
       CCBWHT(ch, C_NRM),GET_ARMS_AC(ch),GET_ARMS_STOPPING(ch),CCNRM(ch,C_NRM));
       send_to_char(buf, ch);   
       sprintf( buf,"           Head AC/St: [%s%d/%d%s]       Leg  AC/St: [%s%d/%d%s]\r\n",
       CCBWHT(ch, C_NRM),GET_HEAD_AC(ch),GET_HEAD_STOPPING(ch),CCNRM(ch,C_NRM),
       CCBWHT(ch, C_NRM),GET_LEGS_AC(ch),GET_LEGS_STOPPING(ch),CCNRM(ch,C_NRM));
       send_to_char(buf, ch);
       sprintf(buf,"\r\nPrimary:   Hitroll: [%s%2d%s/%s%2d%s]  Damroll: [%s%2d%s/%s%2d%s]\r\n",
	       CCBCYN(ch, C_NRM),ch->points.hitroll,CCNRM(ch, C_NRM),	       
	       CCBYEL(ch, C_NRM),ch->points.hitroll+bonus,CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM),ch->points.damroll,CCNRM(ch, C_NRM),      
	       CCBYEL(ch, C_NRM),ch->points.damroll+bonus,CCNRM(ch, C_NRM));
       send_to_char(buf, ch);
       sprintf(buf,"\r\nSecondary: Hitroll: [%s%2d%s/%s%2d%s]  Damroll: [%s%2d%s/%s%2d%s]\r\n",
	       CCBCYN(ch, C_NRM),ch->specials.hitroll,CCNRM(ch, C_NRM),	       
	       CCBYEL(ch, C_NRM),ch->specials.hitroll+bonus,CCNRM(ch, C_NRM),
	       CCBCYN(ch, C_NRM),ch->specials.damroll,CCNRM(ch, C_NRM), 
	       CCBYEL(ch, C_NRM),ch->specials.damroll+bonus,CCNRM(ch, C_NRM));
       send_to_char(buf, ch);
   }
   else{
     parse_text(ch,ch,1,buf2);
     sprintf(buf,"%sPoofIn: %s%s\r\n",CCCYN(ch, C_NRM),CCNRM(ch, C_NRM), buf2);
     send_to_char(buf,ch);
     parse_text(ch,ch,2,buf2);     
     sprintf(buf,"%sPoofOut: %s%s\r\n",CCCYN(ch, C_NRM),CCNRM(ch, C_NRM),buf2);
     send_to_char(buf,ch);
     sprintf(buf,"\r\n%sYou load in room: %s%d%s\r\n"
	     ,CCGRN(ch, C_NRM),CCRED(ch, C_NRM), ch->specials2.load_room,CCNRM(ch, C_NRM));
     send_to_char(buf,ch);
     if (GET_LEVEL(ch) >= LEVEL_BUILDER){
       if (ch->specials2.edit_zone > 0)
	       sprintf(buf,"\r\n%sYou are editing zone: %s%ld%s\r\n"
		       ,CCGRN(ch, C_NRM),CCRED(ch, C_NRM), ch->specials2.edit_zone,CCNRM(ch, C_NRM));
	   else
	       sprintf(buf,"\r\n%sYou are editing zone: %sNone%s\r\n"
		       ,CCGRN(ch, C_NRM),CCRED(ch, C_NRM),CCNRM(ch, C_NRM));
	   send_to_char(buf,ch);
       }
       if (GET_LEVEL(ch) > LEVEL_BUILDER
	   && (ch->specials2.edit_zone2 > 0
	       || ch->specials2.edit_zone3 > 0)){
	   if (ch->specials2.edit_zone2 > 0 && ch->specials2.edit_zone3 <= 0)
	       sprintf(buf,"%sYou can also edit zone:%s%ld%s"
		       ,CCGRN(ch, C_NRM),CCRED(ch, C_NRM)
		       , ch->specials2.edit_zone2,CCNRM(ch, C_NRM));
	   else if (ch->specials2.edit_zone2 <= 0 && ch->specials2.edit_zone3 > 0)
	       sprintf(buf,"%sYou can also edit zone:%s%ld%s"
		       ,CCGRN(ch, C_NRM),CCRED(ch, C_NRM)
		       , ch->specials2.edit_zone3,CCNRM(ch, C_NRM));
	   else  if (ch->specials2.edit_zone2 > 0 && ch->specials2.edit_zone3 > 0)
	       sprintf(buf,"%sYou can also edit zones: %s%ld and %ld %s\r\n"
		       ,CCGRN(ch, C_NRM),CCRED(ch, C_NRM)
		       ,ch->specials2.edit_zone2
		       ,ch->specials2.edit_zone3
		       ,CCNRM(ch, C_NRM));
	   send_to_char(buf,ch);
       }
   }
   /* Routine to show what spells a char is affected by */
   if (ch->affected) {
       send_to_char(CCYEL(ch,C_NRM), ch);   
       send_to_char("\r\n                         Spells Affecting:\r\n",ch);
       send_to_char(CCNRM(ch,C_NRM), ch);   
       nsp = 0;
       *buf = 0;
       for (aff = ch->affected; aff; aff = aff->next) {
	 if (aff->type != SKILL_SNEAK){
	   if ((name = find_spell_name(aff->type))){
	     sprintf(buf, "%sSPL: %s%20s%s ",buf,
		     CCGRN(ch, C_NRM), name, CCNRM(ch, C_NRM));
	     nsp++;
	   }
	   else if ((name = find_skill_name(aff->type))){
	     sprintf(buf, "%sSKL: %s%20s%s ",buf,
		     CCGRN(ch, C_NRM), name, CCNRM(ch, C_NRM));
	     nsp++;
	   }
	 }
	 if (nsp > 1 && !(nsp%2))
	   sprintf(buf, "%s\r\n",buf);
       }
       sprintf(buf, "%s\r\n",buf);
       send_to_char(buf, ch);
   }
}
int report_highest_value_for_weight(int weight)
{
    int amount=0;
    
    amount += 1000*weight/4;
    weight -= 4*(weight/4);
    amount += 100*weight/3;
    weight -= 3*(weight/3);
    amount += 10*weight/2;
    return amount;
}
int report_money_weight(int amount)
{
    int weight=0;

    weight += report_crowns(amount)*4;
    weight += report_groats(amount)*3;
    weight += report_pennies(amount)*2;
    return(weight);
}

int report_crowns(int gold)
{
    int crowns;
    
    crowns = gold/1000;
    return(crowns);
}

int report_groats(int gold)
{
    int crowns,groats;
    
    crowns = gold/1000;
    groats = gold/100 - crowns*10;
    return(groats);
}
int report_pennies(int gold)
{
    int crowns,groats,pennies;
    
    crowns = gold/1000;
    groats = gold/100 - crowns*10;
    pennies = gold/10 - crowns*100 - groats*10;
    return(pennies);
}

char *report_cost(int gold)
{
    char buf_loc[240];
    
    int crowns, groats, pennies;

    if (gold <10){
	sprintf(buf_crwn,"nothing");
	return(&buf_crwn[0]);
    }
    
    crowns = gold/1000;
    groats = gold/100 - crowns*10;
    pennies = gold/10 - crowns*100 -groats*10;
    
    if (crowns)
	if (crowns > 1)
	    sprintf(buf_crwn,"%d crowns", crowns);
	else
	    sprintf(buf_crwn,"1 crown");
	
    if (groats)
	if ( crowns > 0 && pennies == 0 ){
	    if (groats == 1)
		strcat(buf_crwn," and 1 groat");
	    else {
		sprintf(buf_loc," and %d groats", groats);
		strcat(buf_crwn,buf_loc);
	    }
	}
	else if (crowns > 0 && pennies > 0){
	    if (groats == 1)
		strcat(buf_crwn,", 1 groat");
	    else {
		sprintf(buf_loc,", %d groats", groats);
		strcat(buf_crwn,buf_loc);
	    }
	}
	else if (crowns == 0){
	    if (groats == 1)
		sprintf(buf_crwn,"1 groat");
	    else {
		sprintf(buf_crwn,"%d groats", groats);
	    }
	}
    if (pennies)
	if ( crowns > 0 || groats > 0 )
	    {
		if (pennies == 1)
		    strcat(buf_crwn," and 1 penny");
		else {
		    sprintf(buf_loc," and %d pennies", pennies);
		    strcat(buf_crwn,buf_loc);
		}
	    }
	else
	    {
		if (pennies == 1)
		    sprintf(buf_crwn,"1 penny");
		else {
		    sprintf(buf_crwn,"%d pennies", pennies);
		}
	    }


    
    return(&buf_crwn[0]);
}

char *report_gold(int gold)
{
    char buf_loc[240];
    
    int crowns, groats, pennies;
    crowns = gold/1000;
    groats = gold/100 - crowns*10;
    pennies = gold/10 - crowns*100 -groats*10;
    if (crowns)
	if (crowns > 1)
	    sprintf(buf_crwn,"%d cr", crowns);
	else
	    sprintf(buf_crwn,"1 cr");
    else
	buf_crwn[0] = '\0';
	
    if (groats)
	if ( crowns > 0){
	    if (groats == 1)
		strcat(buf_crwn,"/1 gr");
	    else {
		sprintf(buf_loc,"/%d gr", groats);
		strcat(buf_crwn,buf_loc);
	    }
	}
	else if ( crowns == 0 && pennies >0){
	    if (groats == 1)
		sprintf(buf_crwn,"1 gr");
	    else {
		sprintf(buf_crwn,"%d gr", groats);
	    }
	}
    if (pennies)
	if ( crowns > 0 || groats > 0 )
	    {
		if (pennies == 1)
		    strcat(buf_crwn,"/1 p");
		else {
		    sprintf(buf_loc,"/%d p", pennies);
		    strcat(buf_crwn,buf_loc);
		}
	    }
	else
	    {
		if (pennies == 1)
		    sprintf(buf_crwn,"1 penny");
		else {
		    sprintf(buf_crwn,"%d pennies", pennies);
		}
	    }
    return(&buf_crwn[0]);
}



ACMD(do_look)
{
  static char	arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char buff[100];
  int	keyword_no, to_room;
  int	j,iexits2, bits, temp;
  byte llev;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char	*tmp_desc;
  ACMD(do_track);
  static char	*keywords[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "room",
    "out",
    "\n"          };
  int	door,iexits;
  char	*exits[] = 
  {
    "North",
    "East",
    "South",
    "West",
    "Up",
    "Down"
  };


  if (!ch->desc)
    return;

  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\r\n", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\r\n", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damned thing, you're blinded!\r\n", ch);
  else {
    argument_split_2(argument, arg1, arg2);
    keyword_no = search_block(arg1, keywords, FALSE); /* Partiel Match */

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
    }

    found = FALSE;
    tmp_object = 0;
    tmp_char	 = 0;
    tmp_desc	 = 0;

    switch (keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 :
      if (EXIT(ch, keyword_no)) {
	if (EXIT(ch, keyword_no)->general_description
	    && !IS_SET(EXIT(ch,keyword_no)->exit_info,EX_SECRET))
	  send_to_char(EXIT(ch, keyword_no)->general_description, ch);
	else if (!IS_SET(EXIT(ch,keyword_no)->exit_info,EX_SECRET) ||
		 (!IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) &&
		  IS_SET(EXIT(ch,keyword_no)->exit_info,EX_SECRET)))
	  send_to_char("You see nothing special.\r\n", ch);
	else
	  send_to_char("Nothing special there...\r\n", ch);
	    
		
	if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && 
	    (EXIT(ch, keyword_no)->keyword) &&
	    !IS_SET(EXIT(ch,keyword_no)->exit_info,EX_SECRET) &&
	    !IS_SET(EXIT(ch,keyword_no)->exit_info,EX_DARK)) {
	  sprintf(buf, "The %s is closed.\r\n",
		  fname(EXIT(ch, keyword_no)->keyword));
	  send_to_char(buf, ch);
	} else {
	  if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) && 
	      EXIT(ch, keyword_no)->keyword &&
	      !IS_SET(EXIT(ch,keyword_no)->exit_info,EX_SECRET) &&
	      !IS_SET(EXIT(ch,keyword_no)->exit_info,EX_DARK)) {
	    sprintf(buf, "The %s is open.\r\n",
		    fname(EXIT(ch, keyword_no)->keyword));
	    send_to_char(buf, ch);
	  }
	}
      } else {
	send_to_char("Nothing special there...\r\n", ch);
      }
      break;

      /* look 'in'	*/
    case 6:
      if (*arg2) {
	/* Item carried */

	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | 
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if (bits) { /* Found something */
	  if ((GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) || 
	      (GET_ITEM_TYPE(tmp_object) == ITEM_FOUNTAIN)) {
	    if (tmp_object->obj_flags.value[1] <= 0) {
	      act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
	    } else {
	      temp = ((tmp_object->obj_flags.value[1] * 3)
		      / tmp_object->obj_flags.value[0]);
	      sprintf(buf, "It's %sfull of a %s liquid.\r\n",fullness[temp],
		      color_liquid[tmp_object->obj_flags.value[2]]);
	      send_to_char(buf, ch);
	    }
	  } else if ((GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER)
		     || (GET_ITEM_TYPE(tmp_object) == ITEM_SCABBARD)) {
	    if (!IS_SET(tmp_object->obj_flags.value[1], CONT_CLOSED)) {
	      if (HASROOM(tmp_object)) {
		to_room = real_room(tmp_object->obj_flags.value[3]);
		if (tmp_object->obj_flags.value[0] > 0)
		  act("Looking in $p you see:",TRUE,ch,tmp_object,0,TO_CHAR);
		else
		  act("Looking through $p you see:",TRUE
		      ,ch,tmp_object,0,TO_CHAR);
		compute_room_light_value(&world[to_room]);
		llev = illumination_level(world[to_room].light, ch);
		if (PRF_FLAGGED(ch, PRF_HOLYLIGHT))
		  llev = 3;
		if ((llev < 0))
		  strcpy(buf2,"It is pitch black you can't see anything.\r\n");
		else if (llev == 5)
		  strcpy(buf2,"It is dazzlingly bright you can't see anything.\r\n");      
		else {
		  sprintf(buf2,"%s\r\n%s.",
			  CCCYN(ch, C_CMP),
			  (world[to_room].name ? world[to_room].name : "something"));
		  if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
		    sprintbit((long) world[to_room].room_flags,
			      room_bits,buf);
		    sprintf(buf2, "%s (#%d) [ %s]",buf2,
			    world[to_room].number, buf);
		  }
		  strcat(buf2, CCNRM(ch, C_CMP));
		  strcat(buf2, "\r\n");
		  send_to_char(buf2,ch);
		  if (world[to_room].description &&
		      !PRF_FLAGGED(ch, PRF_BRIEF)){
		    process_room_descr(world[to_room].description, buf1,llev);
		    if(!IS_SET(world[to_room].room_flags,
			       NO_AUTOFORMAT)) {
		      if (PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
			if (*buf1)
			  *(buf1 + strlen(buf1) -2) = '\0';
			strcat(buf1, "  The area is magically lit.\r\n");
		      }
		      else {
			switch (llev) {
			case 0:
			  if (*buf1)
			    *(buf1 + strlen(buf1) -2) = '\0';
			  strcat(buf1, "  It's quite dark here.\r\n");
			  break;
			case 1:
			  if (*buf1)			
			    *(buf1 + strlen(buf1) -2) = '\0';
			  strcat(buf1, "  It's a bit gloomy here.\r\n");
			  break;
			case 3:
			  if (*buf1)			
			    *(buf1 + strlen(buf1) -2) = '\0';
			  strcat(buf1, "  It's bright here.\r\n");
			  break;	    
			case 4:
			  if (*buf1)			
			    *(buf1 + strlen(buf1) -2) = '\0';
			  strcat(buf1, "  It's glaringly bright here.\r\n");
			  break;
			default:
			  break;
			}
		      }
		      if (*buf1) {
			format_desc(buf1, buf,79);
			send_to_char(buf, ch);
		      }
		    }
		    else 
		      send_to_char(world[to_room].description,ch);
		  }
		  list_obj_to_char(world[to_room].contents
				   ,ch,0,FALSE);
		  list_char_to_char(world[to_room].people,ch,0);
		}
	      }
	      else {
		send_to_char(fname(tmp_object->name), ch);
		switch (bits) {
		case FIND_OBJ_INV :
		  send_to_char(" (carried) : \r\n", ch);
		  break;
		case FIND_OBJ_ROOM :
		  send_to_char(" (here) : \r\n", ch);
		  break;
		case FIND_OBJ_EQUIP :
		  send_to_char(" (worn) : \r\n", ch);
		  break;
		}
		list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
	      }
	    } else
	      send_to_char("It is closed.\r\n", ch);
	  } else 
	    send_to_char("That is not a container.\r\n", ch);
	} else  /* wrong argument */
	  send_to_char("You do not see that item here.\r\n", ch);
      } else  /* no argument */
	send_to_char("Look in what?!\r\n", ch);
      break;

      /* look 'at'	*/
    case 7 :
      if (*arg2) {

	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | 
			    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);

	if (tmp_char) {
	  show_char_to_char(tmp_char, ch, 1);
	  if (ch != tmp_char) {
	    if (CAN_SEE(tmp_char, ch))
	      act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	    act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	  }
	  return;
	}

	/* Search for Extra Descriptions in room and items */

	/* Extra description in room?? */
	if (!found) {
	  tmp_desc = find_ex_description(arg2, 
					 world[ch->in_room].ex_description);
	  if (tmp_desc) {
	    page_string(ch->desc, tmp_desc, 0);
	    return; /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
	    /* Old system was: found = TRUE; */
	  }
	}

	/* Search for extra descriptions in items */

	/* Equipment Used */

	if (!found) {
	  for (j = 0; j < MAX_WEAR && !found; j++) {
	    if (ch->equipment[j]) {
	      if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
		tmp_desc = find_ex_description(arg2, 
					       ch->equipment[j]->ex_description);
		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}
	      }
	    }
	  }
	}

	/* In inventory */

	if (!found) {
	  for (tmp_object = ch->inventory; 
	       tmp_object && !found; 
	       tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}

	/* Object In room */

	if (!found) {
	  for (tmp_object = world[ch->in_room].contents; 
	       tmp_object && !found; 
	       tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* wrong argument */

	if (bits) { /* If an object was found */
	  if (!found)
	    show_obj_to_char(found_object, ch, 5); /* Show no-description */
	  else
	    show_obj_to_char(found_object, ch, 6); /* Find hum, glow etc */
	} else if (!found) {
	  send_to_char("You do not see that here.\r\n", ch);
	}
      } else {
	/* no argument */

	send_to_char("Look at what?\r\n", ch);
      }

      break;
      /* look ''		*/
      /* and look 'room'       */
    case 8:
    case 9 :
      compute_room_light_value(&world[ch->in_room]);
      llev = illumination_level(world[ch->in_room].light, ch);
      if (PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	llev = 3;
      if ((llev < 0))
	strcpy(buf2,"It is pitch black...\r\n");
      else if (llev == 5)
	strcpy(buf2,"It is dazzlingly bright...\r\n");      
      else {
	strcpy(buf2, CCCYN(ch, C_CMP));
	strcat(buf2, (world[ch->in_room].name ?world[ch->in_room].name :"Null" ));
	if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
	  sprintbit((long) world[ch->in_room].room_flags, room_bits, buf);
	  sprintf(buf2,"%s (#%d) [ %s]",buf2,world[ch->in_room].number, buf);
	}

	strcat(buf2, CCNRM(ch, C_CMP));
	strcat(buf2, "\r\n");
      }
      if ( world[ch->in_room].description && !PRF_FLAGGED(ch, PRF_BRIEF)) {
	process_room_descr(world[ch->in_room].description, buf1, llev);
	if(!IS_SET(world[ch->in_room].room_flags,NO_AUTOFORMAT)) {
	  if (PRF_FLAGGED(ch, PRF_HOLYLIGHT)) {
	    if (*buf1)
	      *(buf1 + strlen(buf1) -2) = '\0';
	    strcat(buf1, "  The area is magically lit.\r\n");
	  }
	  else {
	    switch (llev) {
	    case 0:
	      if (*buf1)
		*(buf1 + strlen(buf1) -2) = '\0';
	      strcat(buf1, "  It's quite dark here.\r\n");
	      break;
	    case 1:
	      if (*buf1)	    
		*(buf1 + strlen(buf1) -2) = '\0';
	      strcat(buf1, "  It's a bit gloomy here.\r\n");
	      break;
	    case 3:
	      if (*buf1)	    
		*(buf1 + strlen(buf1) -2) = '\0';
	      strcat(buf1, "  It's bright here.\r\n");
	      break;	    
	    case 4:
	      if (*buf1)	    
		*(buf1 + strlen(buf1) -2) = '\0';
	      strcat(buf1, "  It's glaringly bright here.\r\n");
	      break;
	    default:
	      break;
	    }
	  }
	  if (*buf1) {	  
	    format_desc(buf1, buf,79);
	    strcat (buf2,  buf);
	  }
	}
	else 
	  strcat (buf2,  buf1);
      }
      send_to_char(buf2, ch);
      if (world[ch->in_room].obj)
	if (GET_ITEM_TYPE(world[ch->in_room].obj) == ITEM_CONTAINER
	    && !IS_SET(world[ch->in_room]
		       .obj->obj_flags.extra_flags, ITEM_DARK)
	    && !IS_SET(world[ch->in_room]
		       .obj->obj_flags.value[1], CONT_ONEWAY)
	    && CAN_SEE_OBJ(ch,world[ch->in_room].obj)){
	  if (!IS_SET(world[ch->in_room].obj->obj_flags.value[1], CONT_CLOSED))
	    sprintf(buff,"The %s is open.",
		    first_name(world[ch->in_room].obj->name));
	  else
	    sprintf(buff,"The %s is closed.",
		    first_name(world[ch->in_room].obj->name));
	  act(buff,FALSE,ch,0,0,TO_CHAR);
	}
      /* this is a hack to add exits at end of room descriptions ajn 15th Dec 1993 */
      if (!IS_SET(world[ch->in_room].room_flags, NO_EXITS)){
	*buf = '\0';
	sprintf(buf1,"%sExits: ",CCGRN(ch, C_NRM));
	iexits=0;
	iexits2=0;
	for (door = 0; door <=5; door++){
	  if (EXIT(ch, door))
	    if (((EXIT(ch, door)->to_room) > 0) &&
		!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
		!IS_SET(EXIT(ch, door)->exit_info, EX_DARK))
	      iexits++;}
	for (tmp_object = world[ch->in_room].contents
	       ; tmp_object;tmp_object = tmp_object->next_content)
	  if (HASROOM(tmp_object)
	      && !IS_SET(tmp_object->obj_flags.extra_flags,ITEM_DARK)   
	      && !IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)
	      && CAN_SEE_OBJ(ch,tmp_object ))
	    iexits++;
	if (world[ch->in_room].obj)
	  if (GET_ITEM_TYPE(world[ch->in_room].obj) == ITEM_CONTAINER 
	      && !IS_SET(world[ch->in_room].obj->obj_flags.extra_flags,ITEM_DARK)
	      && !IS_SET(world[ch->in_room]
			 .obj->obj_flags.value[1], CONT_CLOSED)
	      && !IS_SET(world[ch->in_room]
			 .obj->obj_flags.value[1], CONT_ONEWAY)
	      && CAN_SEE_OBJ(ch,world[ch->in_room].obj))
	    iexits++;
	for (door = 0; door <= 5; door++)
	  {
	    if (EXIT(ch, door))
	      if ((EXIT(ch, door)->to_room > 0) && 		  
		  !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
		  !IS_SET(EXIT(ch, door)->exit_info, EX_DARK))
		{
		  strcat(buf, exits[door]);
		  iexits2++;
		  if (*buf)
		    {
		      if ((iexits >=1) && (iexits2 <iexits))
			strcat(buf,", ");
		    }
		}
	  }
	for (tmp_object = world[ch->in_room].contents
	       ; tmp_object;tmp_object = tmp_object->next_content)
	  if (HASROOM(tmp_object)
	      && !IS_SET(tmp_object->obj_flags.extra_flags,ITEM_DARK)  
	      && !IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)
	      && CAN_SEE_OBJ(ch,tmp_object))
	    {
	      strcpy(buff,first_name(tmp_object->name));
	      buff[0] = UPPER(buff[0]);
	      strcat(buf, buff);
	      iexits2++;
	      if(*buf)
		{
		  if ((iexits >=1) && (iexits2 < iexits))
		    strcat(buf,", ");
		}
	    }
	if (world[ch->in_room].obj)
	  if (GET_ITEM_TYPE(world[ch->in_room].obj) == ITEM_CONTAINER
	      && !IS_SET(world[ch->in_room].obj->obj_flags.extra_flags,ITEM_DARK)
	      && !IS_SET(world[ch->in_room]
			 .obj->obj_flags.value[1], CONT_CLOSED)
	      && !IS_SET(world[ch->in_room]
			 .obj->obj_flags.value[1], CONT_ONEWAY)
	      && CAN_SEE_OBJ(ch,world[ch->in_room].obj)){
	    iexits2++;
	    strcat(buf,"Leave");
	    if ((iexits >=1) && (iexits2 < iexits))
	      strcat(buf,", ");
	  }
	if (*buf)
	  strcat(buf,".");
	else
	  strcat(buf,"None.");
	strcat(buf1,buf);
	sprintf(buf,"%s\r\n",CCNRM(ch,C_NRM));
	strcat(buf1,buf);
	send_to_char(buf1,ch);
      }
      /*end of Hack*/


      list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
      list_char_to_char(world[ch->in_room].people, ch, 0);
      if (!IS_NPC(ch) && ch->specials.hunting)
	do_track(ch,"",0,0);
      break;
      /* look 'out' (from a cart e.g. ...) */
    case 10:
      if (world[ch->in_room].obj)
	{

	  if (IS_SET(world[ch->in_room].obj->obj_flags.value[1],
		     CONT_CLOSED)){
	    sprintf(buf,"The %s seems to be closed.",
		    first_name(world[ch->in_room].obj->name));
	    act(buf,TRUE,ch,0,0,TO_CHAR);
	    return;
	  }
		  
	  else if (IS_DARK(world[ch->in_room].obj->in_room)
		   && !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	    send_to_char("Nothing.  It is pitch black.\r\n",ch);
	  else{
	    sprintf(buf1,"Looking out of the %s you see:%s\r\n\r\n%s.",
		    first_name(world[ch->in_room].obj->name),
		    CCCYN(ch, C_CMP),
		    (world[world[ch->in_room].obj->in_room].name ?
		     world[world[ch->in_room].obj->in_room].name:
		     "something"));
	    if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
	      sprintbit((long) world[world[ch->in_room].obj->in_room].room_flags,
			room_bits, buf);
	      sprintf(buf1, "%s (#%d) [ %s]",
		      buf1, world[world[ch->in_room].obj->in_room].number, buf);
	    }
			 
	    strcat(buf1, CCNRM(ch, C_CMP));
	    strcat(buf1, "\r\n");
	    send_to_char(buf1,ch);
	    if (world[world[ch->in_room].obj->in_room].description
		&& !PRF_FLAGGED(ch, PRF_BRIEF)){
	      strcpy(buf2,
		     world[world[ch->in_room].obj->in_room].description);
	      send_to_char(buf2, ch);}
	    list_obj_to_char(world[world[ch->in_room].obj->in_room].contents,ch,0,FALSE);
	    list_char_to_char(world[world[ch->in_room].obj->in_room].people,ch,0);
	    return;
	  }
	}
      do_look(ch,"",0,0);
      return;
      /* wrong arg	*/
    case -1 :
      send_to_char("Sorry, I didn't understand that!\r\n", ch);
      break;
    }
  }
}


/* end of look */




ACMD(do_read)
{
  /* This is just for now - To be changed later.! */
  sprintf(buf1, "at %s", argument);
  do_look(ch, buf1, 15, 0);
}



ACMD(do_examine)
{
  char	name[100], buf[100];
  int	bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  sprintf(buf, "at %s", argument);
  do_look(ch, buf, 15, 0);

  one_argument(argument, name);

  if (!*name) {
    send_to_char("Examine what?\r\n", ch);
    return;
  }

  bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM | 
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) || 
	(GET_ITEM_TYPE(tmp_object) == ITEM_FOUNTAIN) || 
	(GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\r\n", ch);
      sprintf(buf, "in %s", argument);
      do_look(ch, buf, 15, 0);
    }
  }
}


ACMD(do_scan)
{
  char buf[256],buf1[2048];
  struct char_data *list,*i; 
  static char *dirn_info[] = {
    "A little to the North.",
    "A little to the East.",
    "A little to the South.",
    "A little to the West.",
    "A little above your head.",
    "A little below you."};
  static char *dirn_info_far[] = {
    "To the North.",
    "To the East.",
    "To the South.",
    "To the West.",
    "Above your head.",
    "Below you."};
  int door, rnum, rrnum,rrrnum;
  
  act("$n looks about furtively.",FALSE,ch,0,0,TO_ROOM);
  send_to_char("You check the area.\r\n",ch);
  if (IS_SET(world[ch->in_room].room_flags,NO_SCAN))
    {
      send_to_char("You can't see anything.\r\n",ch);
      return;}
  *buf1 = '\0';
  *buf = '\0';
  list = world[ch->in_room].people;
  if (!(IS_DARK(ch->in_room) && !IS_AFFECTED(ch,AFF_INFRARED)))
    for (i=list;i;i=i->next_in_room)   /* this room */
      {
	if (ch != i) 
	  {
	    if ((CAN_SEE(ch,i) && 
		 (IS_AFFECTED(ch, AFF_SENSE_LIFE) || !IS_AFFECTED(i, AFF_HIDE))))
	      if (!(IS_NPC(i)))
		sprintf(buf, "\t%s...%s",i->player.name, "Right here!\r\n");
	      else
		sprintf(buf, "\t%s...%s",i->player.short_descr, "Right here!\r\n");
	    strcat(buf1,buf);  
	    *buf = '\0';
	  }
      }
    
  for (door=0;door<=5; door++) /* other exits */
    {        
      if (!EXIT(ch,door))
	continue;
      else if ((EXIT(ch, door)->to_room > 0)  && 
	       !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	       !(IS_DARK(real_room(EXIT(ch,door)->to_room))
		 && !IS_AFFECTED(ch,AFF_INFRARED)))
	{
	  
	  rnum = EXIT(ch,door)->to_room;
	  rrnum = real_room(rnum);
	  list = world[rrnum].people;
	  for (i=list;i;i=i->next_in_room)
	    { 
	      if (ch != i) 
		{
		  if ((CAN_SEE(ch,i) && 
		       (IS_AFFECTED(ch, AFF_SENSE_LIFE) || !IS_AFFECTED(i, AFF_HIDE))
		       && (number(0,10) > 5)))
		    if (!(IS_NPC(i)))
		      sprintf(buf, "\t%s...%s\r\n",i->player.name, dirn_info[door]);
		    else
		      sprintf(buf, "\t%s...%s\r\n",i->player.short_descr, dirn_info[door]);
		  strcat(buf1,buf);
		  *buf = '\0';
		}
	    }
	  if (!(world[rrnum].dir_option[door]))
	    continue;
	  if (((world[rrnum].dir_option[door])->to_room) != NOWHERE && 
	      !IS_SET((world[rrnum].dir_option[door])->exit_info, EX_CLOSED) && 
	      !(IS_DARK(real_room(world[rrnum].dir_option[door]->to_room)) &&
		!IS_AFFECTED(ch,AFF_INFRARED)))
	    {
	      rrrnum = world[rrnum].dir_option[door]->to_room;
	      rrnum = real_room(rrrnum);
	      list = world[rrnum].people;
	      for (i=list;i;i=i->next_in_room)
		{ 
		  if (ch != i) 
		    {
		      if ((CAN_SEE(ch,i) && 
			   (IS_AFFECTED(ch, AFF_SENSE_LIFE) || !IS_AFFECTED(i, AFF_HIDE))
			   && (number(0,10) > 7)))
			if (!(IS_NPC(i)))
			  sprintf(buf, "\t%s...%s\r\n",i->player.name, dirn_info_far[door]);
			else
			  sprintf(buf, "\t%s...%s\r\n",i->player.short_descr, dirn_info_far[door]);
		      strcat(buf1,buf);
		      *buf = '\0';
		    }
		}
	    }
	}
    }
    
    
  if (!(*buf1))
    send_to_char("hmmm, all clear - nobody about.\r\n",ch); 
  else
    { send_to_char("You see...\r\n", ch);
    send_to_char(buf1,ch);}
}


ACMD(do_exits)
{
  struct obj_data *tmp_object;
  int	door, to_room;
  char	*exits[] = 
  {
    "North",
    "East ",
    "South",
    "West ",
    "Up   ",
    "Down "
  };

  *buf = '\0';
   
  if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damned thing, you're blinded!\r\n", ch);
  else if ( IS_DARK(ch->in_room) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)) 
    send_to_char("It is pitch black...\r\n", ch);
  else{
    for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room > 0 && 
	    !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	    !IS_SET(EXIT(ch, door)->exit_info, EX_DARK)
	    && (!IS_SET(world[real_room(EXIT(ch, door)->to_room)].room_flags,UNFINISHED) || GET_LEVEL(ch) > LEVEL_BUILDER))
	  if (GET_LEVEL(ch) >= LEVEL_BUILDER)
	    sprintf(buf + strlen(buf), "%-5s - [%5d] %s\r\n",
		    exits[door], EXIT(ch, door)->to_room,
		    world[real_room(EXIT(ch, door)->to_room)].name);
	  else if (IS_DARK(real_room(EXIT(ch, door)->to_room)) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	    sprintf(buf + strlen(buf), "%-5s - Too dark to tell\r\n", exits[door]);
	  else
	    sprintf(buf + strlen(buf), "%-5s - %s\r\n", exits[door],
		    world[real_room(EXIT(ch, door)->to_room)].name);
    for (tmp_object = world[ch->in_room].contents
	   ; tmp_object;tmp_object = tmp_object->next_content){
      to_room = real_room(tmp_object->obj_flags.value[3]);
      if (HASROOM(tmp_object)
	  && !IS_SET(tmp_object->obj_flags.extra_flags,ITEM_DARK)
	  && !IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)
	  && CAN_SEE_OBJ(ch,tmp_object))    
	{
	  sprintf(buf1, first_name(tmp_object->name));
	  buf1[0] = UPPER(buf1[0]);
	  if (GET_LEVEL(ch) >= LEVEL_BUILDER)
	    sprintf(buf + strlen(buf),"%-5s - [%5d] %s\r\n",
		    buf1,world[to_room].number,
		    world[to_room].name);
	  else if (IS_DARK(to_room) && !PRF_FLAGGED(ch,PRF_HOLYLIGHT))
	    sprintf(buf + strlen(buf),"%-5s - Too dark to tell\r\n",buf1);
	  else
	    sprintf(buf + strlen(buf),"%-5s - %s\r\n",buf1,
		    world[to_room].name);
	}
    }
    if (world[ch->in_room].obj
	&& !IS_SET(world[ch->in_room].obj->obj_flags.extra_flags,ITEM_DARK)
	&& CAN_SEE_OBJ(ch,world[ch->in_room].obj)
	&& !IS_SET(world[ch->in_room].obj->obj_flags.value[1],CONT_ONEWAY)
	&& !IS_SET(world[ch->in_room].obj->obj_flags.value[1],CONT_CLOSED))
      {
	sprintf(buf1, "Leave");
	if (GET_LEVEL(ch) >= LEVEL_BUILDER)
	  sprintf(buf + strlen(buf),"%-5s - [%5d] %s\r\n",
		  buf1,world[world[ch->in_room].obj->in_room].number,
		  world[world[ch->in_room].obj->in_room].name);
	else if (IS_DARK(world[ch->in_room].obj->in_room)
		 && !PRF_FLAGGED(ch,PRF_HOLYLIGHT))
	  sprintf(buf + strlen(buf),"%-5s - Too dark to tell\r\n",buf1);
	else
	  sprintf(buf + strlen(buf),"%-5s - %s\r\n",buf1,
		  world[world[ch->in_room].obj->in_room].name);
      }

    send_to_char("Obvious exits:\r\n", ch);

    if (*buf)
      send_to_char(buf, ch);
    else
      send_to_char(" None.\r\n", ch);}
}

char *social_standing(struct char_data *ch)
{
  if (GET_FAME(ch) < -500)
    return("Outlaw");
  else if (GET_FAME(ch) < -300)
    return("Cutthroat");
  else if (GET_FAME(ch) < -200)  
    return("Brigand");
  else if (GET_FAME(ch) < -100)  
    return("Thug");
  else if (GET_FAME(ch) < 0)  
    return("Bully");
  else if (GET_FAME(ch) < 100)  
    return("Peasant");
  else if (GET_FAME(ch) < 200)  
    return(((GET_SEX(ch) == 1) ? "Yeoman": "Goodwife"));
  else if (GET_FAME(ch) < 300)
    return(((GET_SEX(ch) == 1) ? "Squire": "Mistress"));    
  else if (GET_FAME(ch) < 400)
    return(((GET_SEX(ch) == 1) ? "Burgher": "Lady"));        
  else if (GET_FAME(ch) < 500)
    return(((GET_SEX(ch) == 1) ? "Knight": "Dame"));            
  else if (GET_FAME(ch) < 600)
    return(((GET_SEX(ch) == 1) ? "Baron": "Baroness"));                
  else if (GET_FAME(ch) < 700)
    return(((GET_SEX(ch) == 1) ? "Viscount": "Viscountess")); 
  else if (GET_FAME(ch) < 850)  
    return(((GET_SEX(ch) == 1) ? "Earl": "Countess")); 
  else if (GET_FAME(ch) < -950)  
    return(((GET_SEX(ch) == 1) ? "Marquess": "Marchioness")); 
  else
    return(((GET_SEX(ch) == 1) ? "Duke": "Duchess"));     
}

ACMD(do_score)
{
  struct time_info_data playing_time;
  struct time_info_data real_time_passed(time_t t2, time_t t1);

  sprintf(buf, "You are %d years old.", GET_AGE(ch));

  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf, "  It's your birthday today.\r\n");
  else
    strcat(buf, "\r\n");


  sprintf(buf, "%sIn your purse you have: %s.\r\n",
	  buf, report_cost(GET_GOLD(ch)));
  sprintf(buf, "%sIn the bank you have:   %s.\r\n",
	  buf, report_cost(GET_BANK_GOLD(ch)));

  playing_time = real_time_passed((time(0) - ch->player.time.logon) + 
				  ch->player.time.played, 0);
  sprintf(buf, "%sYou have been playing for %d days and %d hours.\r\n",
	  buf, playing_time.day, playing_time.hours);
  sprintf(buf, "%sYou have %d social points making you a %s.\r\n",
	  buf, GET_FAME(ch), social_standing(ch));
  sprintf(buf, "%sYou have %d study/training/learning sessions\r\n", buf,
	  SPELLS_TO_LEARN(ch));
  sprinttype(GET_POS(ch), position_types, buf2);
  sprintf(buf, "%sYou are %s and fighting %s.\r\n",buf, buf2,
	  ((ch->specials.fighting)
	   ? GET_NAME(ch->specials.fighting) : "Nobody"));
  if (GET_COND(ch, DRUNK) > 10)
    strcat(buf, "You are intoxicated.\r\n");

  if (GET_COND(ch, FULL) == 0)
    strcat(buf, "You are starving.\r\n");
  if (GET_COND(ch, FULL) == -1)
    strcat(buf, "You are sustained my holy manna.\r\n");
  else if (GET_COND(ch, FULL) <= 4 )
    strcat(buf, "You are famished.\r\n");
  else if (GET_COND(ch, FULL) <= 8 )
    strcat(buf, "You are very hungry.\r\n");
  else if (GET_COND(ch, FULL) <= 12 )
    strcat(buf, "You are hungry.\r\n");
  else if (GET_COND(ch, FULL) < 16 )
    strcat(buf, "Your stomach is growling.\r\n");
  
  if (GET_COND(ch, THIRST) == 0)
    strcat(buf, "You are parched.\r\n");
  if (GET_COND(ch, THIRST) == -1)
    strcat(buf, "Your thirst is quenched by sweet nectar.\r\n");  
  else if (GET_COND(ch, THIRST) <= 4)
    strcat(buf, "Your throat is dry and your lips cracked.\r\n");
  else if (GET_COND(ch, THIRST) <= 8)
    strcat(buf, "Your throat is very dry.\r\n");
  else if (GET_COND(ch, THIRST) <= 12)
    strcat(buf, "You are thirsty.\r\n");
  else if (GET_COND(ch, THIRST) < 16)
    strcat(buf, "Your feel a little thirsty.\r\n");
  
  if (IS_AFFECTED(ch, AFF_BLIND))
    strcat(buf, "You have been blinded!\r\n");

  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    strcat(buf, "You are invisible.\r\n");

  if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE))
    strcat(buf, "You are sensitive to the presence of invisible things.\r\n");

  if (IS_AFFECTED(ch, AFF_POISON))
    strcat(buf, "You are poisoned!\r\n");

  if (IS_AFFECTED(ch, AFF_CHARM))
    strcat(buf, "You have been charmed!\r\n");

  if (affected_by_spell(ch, SPELL_FORTITUDE_BEAR))
    strcat(buf, "You feel protected.\r\n");

  if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
    strcat(buf, "You are summonable by other players.\r\n");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}


ACMD(do_time)
{
  char	*suf;
  int	weekday, day;
  extern struct time_info_data time_info;
  extern const char	*weekdays[];
  extern const char	*month_name[];

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am") );

  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;/* 35 days in a month */

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;   /* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
	  day, suf, month_name[(int)time_info.month], time_info.year);

  send_to_char(buf, ch);
}


ACMD(do_weather)
{
  static char *sky_look[4] = {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning" };

  if (OUTSIDE(ch)) {
    sprintf(buf, 
	    "The sky is %s and %s.\r\n",
	    sky_look[weather_info.sky],
	    (weather_info.change >= 0 ? "you feel a warm wind from south" : 
	     "your foot tells you bad weather is due"));
    send_to_char(buf, ch);
  } else
    send_to_char("You have no feeling about the weather at all.\r\n", ch);
}


ACMD(do_help)
{
  extern int	top_of_helpt;
  extern struct help_index_element *help_index;
  extern FILE *help_fl;
  extern char	*help;

  int	chk, bot, top, mid, minlen;

  if (!ch->desc)
    return;

  for (; isspace(*argument); argument++)
    ;

  if (*argument) {
    if (!help_index) {
      send_to_char("No help available.\r\n", ch);
      return;
    }
    bot = 0;
    top = top_of_helpt;

    for (; ; ) {
      mid = (bot + top) / 2;
      minlen = strlen(argument);

      if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {

	/* trace backwards to find first matching entry. Thanks Jeff Fink! */
	while ((mid > 0) &&
	       (!(chk = strn_cmp(argument, help_index[mid-1].keyword, minlen))))
	  mid--;
	fseek(help_fl, help_index[mid].pos, SEEK_SET);
	*buf2 = '\0';
	for (; ; ) {
	  fgets(buf, 80, help_fl);
	  if (*buf == '#')
	    break;
	  strcat(buf2, buf);
	  strcat(buf2, "\r");
	}
	page_string(ch->desc, buf2, 1);
	return;
      } else if (bot >= top) {
	send_to_char("There is no help on that word.\r\n", ch);
	return;
      } else if (chk > 0)
	bot = ++mid;
      else
	top = --mid;
    }
    return;
  }

  send_to_char(help, ch);
}



#define WHO_FORMAT \
"format: who [-n name] [-d] [-o] [-q] [-r] [-z]\r\n"
#define WHO_FORMAT_IMM \
"format: who [minlev[-maxlev]] [-n name] [-d] [-o] [-q] [-r] [-z]\r\n"

ACMD(do_who)
{
  char bff2[25];
  struct char_data *tch;
  char	name_search[80];
  char	mode;
  int	maxl,minl,low = 0, high = LEVEL_IMPL, i, localwho = 0, questwho = 0;
  int	short_list = 0, outlaws = 0, num_can_see = 0, deadw = 0;
  int	who_room = 0;

  /* skip spaces */
  for (i = 0; *(argument + i) == ' '; i++)
    ;
  strcpy(buf, (argument + i));
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      if (GET_LEVEL(ch) < LEVEL_BUILDER){
	send_to_char(WHO_FORMAT, ch);
	return;
      }
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1); /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'd':
	deadw = 1;
	strcpy(buf, buf1);
	break;
      case 'o':
      case 'k':
	outlaws = 1;
	strcpy(buf, buf1);
	break;
      case 'z':
	localwho = 1;
	strcpy(buf, buf1);
	break;
      case 's':
	if (GET_LEVEL(ch) < LEVEL_BUILDER){
	  send_to_char(WHO_FORMAT, ch);
	  return;
	}
	short_list = 1;
	strcpy(buf, buf1);
	break;
      case 'q':
	questwho = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	if (GET_LEVEL(ch) < LEVEL_BUILDER){
	  send_to_char(WHO_FORMAT, ch);
	  return;
	}
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = 1;
	strcpy(buf, buf1);
	break;
      default:
	if (GET_LEVEL(ch) < LEVEL_BUILDER)
	  send_to_char(WHO_FORMAT, ch);
	else
	  send_to_char(WHO_FORMAT_IMM, ch);	  
	return;
	break;
      } /* end of switch */

    } else { /* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  } /* end while (parser) */

  minl = LEVEL_BUILDER;
  maxl = LEVEL_IMPL;
   

  while(TRUE){
    num_can_see=0;
    if (minl >= LEVEL_BUILDER)
      sprintf(buf,"%sImmortals%s\r\n---------\r\n%s",CCBWHT(ch,C_NRM),
	      CCBLU(ch,C_NRM),CCNRM(ch,C_NRM));
    else
      sprintf(buf,"\r\n%sPlayers%s\r\n-------\r\n%s",CCBWHT(ch,C_NRM),
	      CCBLU(ch,C_NRM),CCNRM(ch,C_NRM));
       
    send_to_char(buf,ch);

    for (tch = character_list; tch; tch = tch->next){
      if (IS_NPC(tch))
	continue;
      if (GET_LEVEL(tch) > maxl
	  || GET_LEVEL(tch) < minl)
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search) && 
	  !strstr(GET_TITLE(ch), name_search))
	continue;      
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) && 
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (deadw && tch->desc)
	continue;
      if (!deadw && !tch->desc)
	continue;
      if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
	continue;
      if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
	continue;
      if (who_room && (tch->in_room != ch->in_room))
	continue;
      if (short_list) {
	sprintf(buf, "%s[ %3d %3.3s] %-12.12s%s%s",
		(GET_LEVEL(tch) >= LEVEL_BUILDER ? CCYEL(ch, C_SPR) : ""),
		GET_LEVEL(tch), RACE_ABBR(tch), GET_NAME(tch),
		(GET_LEVEL(tch) >= LEVEL_BUILDER ? CCNRM(ch, C_SPR) : ""),
		((!(++num_can_see % 3)) ? "\r\n" : ""));
	send_to_char(buf, ch);
      } else {
	num_can_see++;
	switch(GET_RACE(tch)){
	case 0:
	  sprintf( bff2," ERROR  ");
	  break;
	case 1:
	  sprintf( bff2," Human  ");
	  break;
	case 2:
	  sprintf( bff2,"  Elf   ");
	  break;
	case 3:
	  sprintf( bff2,"Halfling");
	  break;
	case 4:
	  sprintf( bff2," Giant  ");
	  break;
	case 5:
	  sprintf( bff2," Gnome  ");
	  break;
	case 6:
	  sprintf( bff2,"Half-elf");
	  break;
	case 7:
	  sprintf( bff2," Ogier  ");
	  break;
	case 8:
	  sprintf( bff2," Dwarf  ");
	  break;
	case 9:
	  sprintf( bff2," Selkie ");
	  break;
	case 10:
	  sprintf( bff2," Pixie  ");
	  break;
	case 11:
	  sprintf( bff2," Amarya ");
	  break;
	case 12:
	  sprintf( bff2, "%s", " Troll  ");
	  break;			
	  
	}
	       
	if (GET_LEVEL(tch) >= LEVEL_IMPL)
	  sprintf(buf, "[%s%-15.15s%s] ",CCBCYN(ch, C_SPR),"  Implementor  ",CCNRM(ch, C_SPR));
	else if (GET_LEVEL(tch) >= LEVEL_ASS_IMPL)
	  sprintf(buf, "[%s%-15.15s%s] ",CCBYEL(ch, C_SPR),"    Overlord   ",CCNRM(ch, C_SPR));
	else if (GET_LEVEL(tch) >= LEVEL_GRGOD)
	  sprintf(buf, "[%s%-15.15s%s] ",CCBMAG(ch, C_SPR),"     Djinn     ",CCNRM(ch, C_SPR));
	else if (GET_LEVEL(tch) >= LEVEL_GOD)
	  sprintf(buf, "[%s%-15.15s%s] ",CCBGRN(ch, C_SPR),"     Daemon    ",CCNRM(ch, C_SPR));
	else if (GET_LEVEL(tch) >= LEVEL_MBUILDER)
	  sprintf(buf, "[%s%-15.15s%s] ",CCBGRN(ch, C_SPR),"  Bodhisattva  ",CCNRM(ch, C_SPR));	 
	else if (GET_LEVEL(tch) >= LEVEL_BUILDER)
	  sprintf(buf, "[%s%-15.15s%s] ",CCBWHT(ch, C_SPR),"    Builder    ",CCNRM(ch, C_SPR));	 
	else if (GET_LEVEL(tch) >= 200)
	  sprintf(buf, "[%s%-8.8s%s] ",CCBWHT(ch, C_SPR)," Avatar ",CCNRM(ch, C_SPR));
	else
	  sprintf(buf, "[%-8.8s] ",bff2);	 
	parse_text(tch,ch,0,buf2);
	sprintf(buf, "%s%s",buf, buf2);
	if (GET_INVIS_LEV(tch))
	  sprintf(buf, "%s (i%d)", buf, GET_INVIS_LEV(tch));
	else if (IS_AFFECTED(tch, AFF_INVISIBLE))
	  sprintf(buf, "%s (%sinvis%s)", buf, CCWHT(ch,C_NRM),
		  CCNRM(ch,C_NRM));
	if (PLR_FLAGGED(tch, PLR_MAILING))
	  sprintf(buf, "%s (%smailing%s)", buf, CCGRN(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      		   
	else if (PLR_FLAGGED(tch, PLR_WRITING))
	  sprintf(buf, "%s (%swriting%s)", buf, CCGRN(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      
	else if (PLR_FLAGGED(tch, PLR_BUILDING))
	  sprintf(buf, "%s (%sbuilding%s)", buf, CCYEL(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      
	else if (PLR_FLAGGED(tch, PLR_AFK))
	  sprintf(buf, "%s (%saway%s)", buf, CCCYN(ch,C_NRM),
		  CCNRM(ch,C_NRM));
	if (PRF_FLAGGED(tch, PRF_DEAF))
	  sprintf(buf, "%s (%sdeaf%s)", buf, CCBBLU(ch,C_NRM),
		  CCNRM(ch,C_NRM));
	if (PRF_FLAGGED(tch, PRF_NOTELL))
	  sprintf(buf, "%s (%snotell%s)", buf, CCBBLU(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      		   		   
	if (PRF_FLAGGED(tch, PRF_QUEST))
	  sprintf(buf, "%s (%squest%s)", buf, CCMAG(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      		   
	if (PLR_FLAGGED(tch, PLR_THIEF))
	  sprintf(buf, "%s (%sTHIEF%s)", buf, CCBMAG(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      		   
	if (PLR_FLAGGED(tch, PLR_KILLER))
	  sprintf(buf, "%s (%sKILLER%s)", buf, CCBCYN(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      		   		   
	if (!tch->desc)
	  sprintf(buf, "%s (%sLD%s)", buf, CCWHT(ch,C_NRM),
		  CCNRM(ch,C_NRM));	      
	if (GET_LEVEL(tch) >= LEVEL_BUILDER)
	  strcat(buf, CCNRM(ch, C_SPR));
	strcat(buf, "\r\n");
	send_to_char(buf, ch);
      } /* endif shortlist */
    } /* end of for */
    if (short_list && (num_can_see % 2))
      send_to_char("\r\n", ch);
    sprintf(buf, "\r\n%s%d%s characters displayed.%s\r\n",CCBRED(ch,C_NRM),
	    num_can_see,CCBWHT(ch,C_NRM),CCNRM(ch,C_NRM));
    send_to_char(buf, ch);
    if (maxl < LEVEL_BUILDER)
      break;
    minl = 0;
    maxl = LEVEL_BUILDER -1;
    
  }
}

char *make_plural(char *arg)
{
  char tmp_loc[100], tmp2_loc[100];
  const char *ptr;
  char *ptrr;
  int i;

  bzero(tmp2_loc,100);
  bzero(tmp_loc,100);
  if (!*arg)
    return(arg);
  if (strlen(arg) > 90)
    return(arg);
  for (ptr = arg; *ptr == ' '; ptr++)
    ;
  for (ptrr= tmp2_loc; *ptr != ' ' && *ptr != '\0';ptr++, ptrr++)
    *ptrr = LOWER(*ptr);
  /* first exceptions */
  *(ptrr++) = '\0';
  for(i=0;strcmp(nonreg_plurals[i].singular,"\n");i++)
    {
      if (!strcmp(nonreg_plurals[i].plural,tmp2_loc)) /*already plural*/
	return(nonreg_plurals[i].plural);
      else if (!strcmp(nonreg_plurals[i].singular,tmp2_loc))
	return(nonreg_plurals[i].plural);
    }
    
  for(ptr = arg, ptrr= tmp_loc;*ptr != ' ' && *ptr != '\0'; ptr++, ptrr++)
    *ptrr = *ptr;
  ptr--;
  if (*ptr == 'y')
    {
      ptrr--;
      *ptrr = 'i';
      *(ptrr +1) = 'e';
      *(ptrr +2) = 's';
      *(ptrr +3) = '\0';	    
    }
  else
    {
      *ptrr = 's';
      *(ptrr +1) = '\0';
    }
  strcpy(buf1,tmp_loc);
  return(buf1);
}



char    *first_name(char *arg)
{
  char buf3[256];
  if (strlen(arg) > 256)
    *(arg + 255) = '\0';
  
  half_chop(arg, buf3, buf2);
  strcpy(buf2,buf3);
  return (buf2);
}

void  process_room_descr(char *in, char *out, byte llev)
{
  const char *string, *i;
  char *p;
  bool include = TRUE;
  
  if (llev == 0)
    include = FALSE;

  bzero(buf1,MAX_STRING_LENGTH);
  if ((llev == -1) || (llev == 5)) {
    strcpy(out,buf1);
    return;
  }
  p = buf1;
  string = in;
  while ( *string != '\0') {
    if (*string != '%') {
      if (include)
	*p++ = *string++;
      else
	string++;
      continue;
    }
    ++string;
    if ((*string == '0'))
      {
	string++;
	while (*string != '%' && *string != '\0') {
	  if (llev >=0)
	    *p++ = *string++;
	  else
	    string++;
	  continue;
	}
      }
    else if ((*string == '1'))
      {
	string++;
	while (*string != '%' && *string != '0' ) {
	  if (llev > 0)
	    *p++ = *string++;
	  else
	    string++;
	  continue;
	}
      }
    else if ((*string == '2'))
      {
	string++;
	while (*string != '%' && *string != '\0') {
	  if (llev > 1)
	    *p++ = *string++;
	  else
	    string++;
	  continue;
	}
      }
    else if ((*string == '3'))
      {
	string++;
	while (*string != '%' && *string != '\0') {
	  if (llev > 2)
	    *p++ = *string++;
	  else
	    string++;
	  continue;
	}
      }
    else if ((*string == '4'))
      {
	string++;
	while (*string != '%' && *string != '\0') {
	  if (llev > 3)
	    *p++ = *string++;
	  else
	    string++;
	  continue;
	}
      }
    else if ((*string == '%'))
      *p++ = *string++;

  }
  return;

}







