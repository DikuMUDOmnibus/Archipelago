/* ************************************************************************
*   File: zoneedit.c                                  Part of Archipelago *
*  Usage: online object editing commands. A Neil - July 1996              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1994 A Neil                                              *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

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
#include "limits.h"
#include "screen.h"
#include "zoneedit.h"

extern int	top_of_zone_table;
extern struct zone_data *zone_table;

void copy_zone(struct zone_data *from, struct zone_data *to);
int save_zone(int zone);
void free_zone(struct zone_data *zone);
void print_zonedata(struct descriptor_data *d);


void zoneedit(struct descriptor_data *d, char *str)
{
  char bufme[MAX_STRING_LENGTH];
  bool ok = FALSE;
  int zn, points, num, ja_flag, max_top;
  for (zn=0;zn <= top_of_zone_table; zn++)
    if( zone_table[zn].number == d->zone_edit->number){
      ok = TRUE;
      break;
    }
  if (ok){
    if (zn == top_of_zone_table)
      max_top = (d->zone_edit->number + 10)*100 -1;
    else
      max_top = (zone_table[zn+1].number)*100 -1;      
    }
  
  switch(d->zedit_mode) {
  case MAIN_MODE:
    for (; isspace(*str); str++)
      ;
    if (!*str){
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      break;
    }
    *str = LOWER(*str);
    if (*str == 'q'){
      if (!ok){
	SEND_TO_Q("Yikes zone entry doesn't exist.\r\nReport this!\r\n",d);
	free_zone(d->zone_edit);
	d->zone_edit = 0;
	d->virtual = 0;
	d->prompt_mode = 1;
	d->iedsc = 0;
	d->ex_i_dir = 0;
	d->ia_flag = 0;
	d->r_dir = 0;
	d->to_room = 0;
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
	act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
	return;
      }
      copy_zone(d->zone_edit, zone_table + zn);
	free_zone(d->zone_edit);
	d->zone_edit = 0;
	d->virtual = 0;
	d->prompt_mode = 1;
	d->iedsc = 0;
	d->ex_i_dir = 0;
	d->ia_flag = 0;
	d->r_dir = 0;
	d->to_room = 0;
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
	act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
	return;
    }
    else
      switch (*str){
      case 't':
	d->str = &(d->zone_edit->name);
	free(*d->str);
	*d->str = 0;
	d->max_str = 50;
	SEND_TO_Q("Enter new Zone Title.  No crlf.  End with @@\r\n",d);
	return;
      case 'l':
	sprintf(buf, "Lifespan:%s%d%s.\r\nEnter new lifespan: [10,100]:\r\n",cy,d->zone_edit->lifespan,nrm);
	send_to_char(buf, d->character);
	d->zedit_mode = ZL_EDIT;
	return;
      case 'p':
	sprintf(buf, "Zone Top:%s%d%s.\r\nEnter new Zone Top: [%d,%d]:\r\n",cy,d->zone_edit->top,nrm,d->zone_edit->number*100 +1, max_top);
	send_to_char(buf, d->character);
	d->zedit_mode = ZT_EDIT;
	return;
      case 'r':
	sprintf(buf, "Reset Mode:%s%d%s.\r\nEnter new Zone Top: [0,1,2]:\r\n",cy,d->zone_edit->reset_mode,nrm);
	send_to_char(buf, d->character);
	sprintf(buf, "0 = never, 1 = only when no pcs in zone, 2 = always.\r\n");
	send_to_char(buf, d->character);
	d->zedit_mode = ZR_EDIT;
	return;
      case 'o':
	if (GET_LEVEL(d->character) < LEVEL_ASS_IMPL){
	  send_to_char("You are not holy enough to open zones.\r\n",d->character);
	  return;
	}
	sprintf(buf, "Open: %s%s%s.\r\nToggle?: [y]:\r\n",cy,
		(d->zone_edit->open ? "Yes": "No"),nrm);
	send_to_char(buf, d->character);
	d->zedit_mode = ZO_EDIT;
	return;
      default:
	break;
      }
  case ZL_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Lifespan unchanged\r\n",d);
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      return;
    }
    points = atoi(str);
    if (points < 10 || points > 100) {
      sprintf(buf,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(buf,d);
      sprintf(buf,"Lifespan: %s%d%s.\r\nEnter new lifespan: [10,100]:",
	      cy,d->zone_edit->lifespan,nrm);
      send_to_char(buf,d->character);
      return;}
    else {
      d->zone_edit->lifespan = points;
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      return;
    }
  case ZT_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Zone Top unchanged\r\n",d);
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      return;
    }
    points = atoi(str);
    if (points < (d->zone_edit->number*100 + 1) || points > max_top) {
      sprintf(buf,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(buf,d);
      sprintf(buf, "Zone Top:%s%d%s.\r\nEnter new Zone Top: [%d,%d]:\r\n",cy,d->zone_edit->top,nrm,d->zone_edit->number*100 +1, max_top);
      send_to_char(buf,d->character);
      return;}
    else {
      d->zone_edit->top = points;
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      return;
    }
  case ZR_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Zone Top unchanged\r\n",d);
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      return;
    }
    points = atoi(str);
    if (points != 0 && points != 1 && points != 2){
      sprintf(buf,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(buf,d);
      sprintf(buf, "Reset Mode:%s%d%s.\r\nEnter new Zone Top: [0,1,2]:\r\n",cy,d->zone_edit->reset_mode,nrm);
      send_to_char(buf, d->character);
      sprintf(buf, "0 = never, 1 = only when no pcs in zone, 2 = always.\r\n");
      send_to_char(buf,d->character);
      return;
    }
    else {
      d->zone_edit->reset_mode = points;
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      return;
    }
  case ZO_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str || *str != 'y'){
      d->zedit_mode = MAIN_MODE;
      print_zonedata(d);
      return;
    }
    if (d->zone_edit->open){
      sprintf(buf,"%sSetting Zone closed%s\r\n",rd,nrm);
      d->zone_edit->open = 0;
    }
    else{
      sprintf(buf,"%sSetting Zone open%s\r\n",rd,nrm);
      d->zone_edit->open = 1;
    }
    d->zedit_mode = MAIN_MODE;
    print_zonedata(d);
    return;
  default:
    break;
      
  } 
}

void print_zonedata(struct descriptor_data *d){
  sprintf(cy,"%s",CCBBLU(d->character,C_NRM));
  sprintf(wh,"%s",CCWHT(d->character,C_NRM));
  sprintf(ma,"%s",CCMAG(d->character,C_NRM));
  sprintf(nrm,"%s",CCNRM(d->character,C_NRM));
  sprintf(gn,"%s",CCGRN(d->character,C_NRM));
  sprintf(yl,"%s",CCYEL(d->character,C_NRM));
  sprintf(rd,"%s",CCRED(d->character,C_NRM));
  sprintf(bgn,"%s",CCBGRN(d->character,C_NRM));
  sprintf(byl,"%s",CCBYEL(d->character,C_NRM));
  sprintf(brd,"%s",CCBRED(d->character,C_NRM));
  sprintf(bwht,"%s",CCBWHT(d->character,C_NRM));  

  sprintf(buf,"%sZone Number: %d%s\r\n",bgn, d->zone_edit->number,nrm);
  send_to_char(buf, d->character);
  sprintf(buf,"%s(T)itle: %s%s%s\r\n",byl,bwht, d->zone_edit->name,nrm);
  send_to_char(buf, d->character);
  sprintf(buf,"%s(L)ifespan: %s%d%s\r\n",byl,bwht, d->zone_edit->lifespan,nrm);
  send_to_char(buf, d->character);
  sprintf(buf,"%s(R)eset: %s%d%s\r\n",byl,bwht, d->zone_edit->reset_mode,nrm);
  send_to_char(buf, d->character);
  sprintf(buf,"%sto(P): %s%d%s\r\n",byl,bwht, d->zone_edit->top,nrm);
  send_to_char(buf, d->character);  
  sprintf(buf,"%s(O)pen: %s%s%s\r\n",byl,bwht, (d->zone_edit->open ? "Yes": "No"),nrm);
  send_to_char(buf, d->character);
}


