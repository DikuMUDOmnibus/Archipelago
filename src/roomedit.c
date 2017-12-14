/* ************************************************************************
*   File: roomedit.c                                  Part of Archipelago *
*  Usage: online room editing commands. A Neil - July 1994                *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Archipelago is based on                                                *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*  Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 * 
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
#include "roomedit.h"
#include "spells.h"

void make_exit_twoway(struct descriptor_data *d);
void roomedit(struct descriptor_data *d, char *str);
void print_room(struct descriptor_data *d);
int print_sectors(struct descriptor_data *d);
int print_rflags(struct descriptor_data *d);
void print_rstr(struct descriptor_data *d);
void print_exit(struct descriptor_data *d);
int print_rex(struct descriptor_data *d);
void print_rexd(struct descriptor_data *d);
int print_exbits(struct descriptor_data *d);
void print_tele(struct descriptor_data *d);
void copy_room(struct room_data *from, struct room_data *to);
void copy_stringinfo(char **to, char **from);
void replace_text (char **text, char *replace, char *with);
void free_room(struct room_data *room);
extern char     *room_bits[];
extern char     *sector_types[];
extern char     *door_bits[];
extern char     *dirs[];
extern int      rev_dir[];
extern int      top_of_world;
extern struct room_data *world;

char bufme[MAX_STRING_LENGTH];

void roomedit(struct descriptor_data *d, char *str)
{

    int ja_flag,points,idesc,room,rroom,delay;
    struct extra_descr_data *desc=0, *new_desc=0, *tmp_d=0;
    struct char_data *vict;

    switch (d->redit_mode){
    case MAIN_MODE:
	for (; isspace(*str); str++)
	    ;
	if (!*str){
	    d->redit_mode = MAIN_MODE;
	    print_room(d);	    
	    break;}
	*str = LOWER(*str);
	if (*str == 'q'){
	    if ((rroom = real_room(d->room_edit->number)) < 0){
		SEND_TO_Q("Yikes room doesn't exist.\r\nReport this!\r\n",d);
		free_room(d->room_edit);
		free(d->replace);
		free(d->with);
		d->replace = 0;
		d->with = 0;		
		d->room_edit = 0;
		d->virtual = 0;
		d->prompt_mode = 1;
		d->iedsc = 0;
		d->ex_i_dir = 0;
		d->ia_flag = 0;
		d->r_dir = 0;
		d->to_room = 0;
		REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
                act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
		return;}
	    copy_room(d->room_edit, world + rroom);
	    free_room(d->room_edit);
	    free(d->replace);
	    free(d->with);
	    d->replace = 0;
	    d->with = 0;			    
	    d->room_edit = 0;
	    d->virtual = 0;
	    d->iedsc = 0;
	    d->ex_i_dir = 0;
	    d->ia_flag = 0;
	    d->r_dir = 0;
	    d->to_room = 0;	    
	    d->prompt_mode = 1;	    
	    REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
            act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
	    return;}
	else
	    switch (*str){
	    case 't':
		d->redit_mode = RTEXT_EDIT;
		print_rstr(d);
		return;
	    case 'r':
		d->redit_mode = RFLAG_EDIT;
		d->ia_flag = print_rflags(d);
		return;
	    case 'c':
		d->redit_mode = RSECT_EDIT;
		d->ia_flag = print_sectors(d);
		return;
	    case 'l':
		d->redit_mode = RTELE_EDIT;
		print_tele(d);
		return;
	    case 'x':
		d->redit_mode = RED_EDIT;
		d->ia_flag = print_rex(d);
		return;
	    case 'n':
		d->ex_i_dir = 0;
		if (d->room_edit->dir_option[d->ex_i_dir]){
		    d->redit_mode = REXIT_EDIT;
		    print_exit(d);
		    return;}
		else{
		  CREATE(d->room_edit->dir_option[d->ex_i_dir], struct room_direction_data,1);
		  d->redit_mode = REXIT_EDIT;
		  print_exit(d);
		  return;}
	    case 'e':
	      d->ex_i_dir = 1;
	      if (d->room_edit->dir_option[d->ex_i_dir]){
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	      else{
		CREATE(d->room_edit->dir_option[d->ex_i_dir], struct room_direction_data,1);
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	    case 's':
	      d->ex_i_dir = 2;
	      if (d->room_edit->dir_option[d->ex_i_dir]){
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	      else{
		CREATE(d->room_edit->dir_option[d->ex_i_dir], struct room_direction_data,1);
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	    case 'w':
	      d->ex_i_dir = 3;
	      if (d->room_edit->dir_option[d->ex_i_dir]){
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	      else{
		CREATE(d->room_edit->dir_option[d->ex_i_dir], struct room_direction_data,1);
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	    case 'u':
	      d->ex_i_dir = 4;
	      if (d->room_edit->dir_option[d->ex_i_dir]){
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	      else{
		CREATE(d->room_edit->dir_option[d->ex_i_dir], struct room_direction_data,1);
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	    case 'd':
	      d->ex_i_dir = 5;
	      if (d->room_edit->dir_option[d->ex_i_dir]){
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}
	      else{
		CREATE(d->room_edit->dir_option[d->ex_i_dir], struct room_direction_data,1);
		d->redit_mode = REXIT_EDIT;
		print_exit(d);
		return;}		
	    default:
	      SEND_TO_Q("Illegal Entry, try again\r\n",d);
	      print_room(d);
	      SEND_TO_Q("\r\nEnter a letter a-w or  Q to Quit\r\n",d);
	      return;
	    }
	break;
    case RTELE_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	*str = LOWER(*str);
	if (*str != 't' && *str != 'd' && *str != '1' && *str != '2' && *str != '3' && *str !='q'){
	    SEND_TO_Q("No such item.",d);
	    print_tele(d);
	    break;}
	switch(*str){
	case 't':
	    sprintf(bufme,"%sEnter new destination room -1 to delete teleport:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->redit_mode = RTTO_EDIT;
	    return;
	case 'd':
	    sprintf(bufme,"%sEnter teleport delay:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->redit_mode = RTDELAY_EDIT;
	    return;
	case '1':
	    d->str = &(d->room_edit->tele_mesg1);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 240;
	    SEND_TO_Q("Enter teleport message to teleportee.\r\nTerminate with a @@.\r\n",d);
	    return;
	case '2':	    
	    d->str = &(d->room_edit->tele_mesg2);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 240;
	    SEND_TO_Q("Enter teleport message to room.\r\nTerminate with a @@.\r\n",d);
	    return;
	case '3':	    
	    d->str = &(d->room_edit->tele_mesg3);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 240;
	    SEND_TO_Q("Enter teleport message to arrival room.\r\nTerminate with a @@.\r\n",d);
	    return;
	case 'q':
	    d->redit_mode = MAIN_MODE;
	    print_room(d);
	    return;	    
	}
    case REXIT_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	*str = LOWER(*str);
	if (*str != 't' && *str != 'd' && *str != 'y' && *str != 'k' && *str != 'p' && *str != 'e' && *str !='q'){
	    SEND_TO_Q("No such item.",d);
	    print_exit(d);
	    break;}
	switch(*str) {
	case 't':
	    sprintf(bufme,"%sEnter new destination room:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->redit_mode = REXTR_EDIT;
	    return;
	case 'd':
	    d->str = &(d->room_edit->dir_option[d->ex_i_dir]->general_description);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter a description. Carriage return at the end,\r\n terminate with a @@:\r\n",d);
	    return;
	case 'k':
	    d->str = &(d->room_edit->dir_option[d->ex_i_dir]->keyword);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter a new keywords. No carriage return at the end,\r\n terminate with a @@:\r\n",d);
	    return;
	case 'e':
	    d->redit_mode = EXBIT_EDIT;
	    d->ia_flag = print_exbits(d);
	    return;
	case 'y':
	    d->redit_mode = EXKEY_EDIT;
	    sprintf(bufme,"%sEnter new key number:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    return;	    
	case 'p':
	    d->redit_mode = EXPU_EDIT;
	    sprintf(bufme,"%sReally purge this exit? (y/n):%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    return;
	case 'q':
	    d->redit_mode = MAIN_MODE;
	    if (d->room_edit->dir_option[d->ex_i_dir]->to_room > 0)
	      make_exit_twoway(d);
	    print_room(d);
	    return;
	}
    case EXPU_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (*str != 'Y' && *str != 'y'){
	    d->redit_mode = MAIN_MODE;
	    print_room(d);
	    return;}
	else{
	    d->to_room = real_room(d->room_edit->dir_option[d->ex_i_dir]->to_room);
	    d->r_dir = rev_dir[d->ex_i_dir];
	    if (d->room_edit->dir_option[d->ex_i_dir]->keyword)
		free(d->room_edit->dir_option[d->ex_i_dir]->keyword);
	    if (d->room_edit->dir_option[d->ex_i_dir]->general_description)
		free(d->room_edit->dir_option[d->ex_i_dir]->general_description);
	    free(d->room_edit->dir_option[d->ex_i_dir]);
	    d->room_edit->dir_option[d->ex_i_dir] = 0;
	    if (world[d->to_room].dir_option[d->r_dir])
		if (world[d->to_room].dir_option[d->r_dir]->to_room
		    == d->room_edit->number)
		    {
		    	    d->redit_mode = EXPUF_EDIT;
			    sprintf(bufme,"%sPurge other side of exit too? (y/n):%s",rd,nrm);
			    SEND_TO_Q(bufme,d);
			    return;
		    }
	    d->redit_mode = MAIN_MODE;
	    print_room(d);
	    return;}
    case EXPUF_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (*str != 'Y' && *str != 'y'){
	    d->redit_mode = MAIN_MODE;
	    print_room(d);
	    return;}
	else{
	    if (world[d->to_room].dir_option[d->r_dir]->keyword)
		free(world[d->to_room].dir_option[d->r_dir]->keyword);
	    if (world[d->to_room].dir_option[d->r_dir]->general_description)
		free(world[d->to_room].dir_option[d->r_dir]->general_description);
	    free((world[d->to_room].dir_option[d->r_dir]));
	    world[d->to_room].dir_option[d->r_dir] = 0;
	    d->redit_mode = MAIN_MODE;
	    print_room(d);
	    return;}
    case EXBIT_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	print_exbits(d);
	break;}
      if (is_number(str)){
	ja_flag = atoi(str);
	if (ja_flag >= 0 && ja_flag <= d->ia_flag )
	  {
	    if (!IS_SET(d->room_edit->dir_option[d->ex_i_dir]->exit_info, 1<<ja_flag))
	      SET_BIT(d->room_edit->dir_option[d->ex_i_dir]->exit_info, 1<<ja_flag);
	    else
	      REMOVE_BIT(d->room_edit->dir_option[d->ex_i_dir]->exit_info, 1<<ja_flag);
	    d->ia_flag =  print_exbits(d);
	    break;
	  }
	else{
	  SEND_TO_Q("Illegal Flag.\r\n",d);
	  d->ia_flag =  print_exbits(d);
	  break;
	}
      }
      else if (*str == 'Q'){
	print_exit(d);	
	d->redit_mode = REXIT_EDIT;
	break;}
      else{
	SEND_TO_Q("Illegal Flag.\r\n",d);
	d->ia_flag =  print_exbits(d);
	break;}
    case RTDELAY_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	d->redit_mode = RTELE_EDIT;
	sprintf(bufme,"\r\n%sTeleport delay unchanged.%s",rd,nrm);
	print_tele(d);
	SEND_TO_Q(bufme,d);	    
	return;}
      delay = atoi(str);
	if (delay > 300){
	    sprintf(bufme,"%sIllegal delay.%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    sprintf(bufme,"%sEnter new delay < 300:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->redit_mode = RTDELAY_EDIT;
	    break;}
	else{
	    d->room_edit->tele_delay = delay;
	    d->redit_mode = RTELE_EDIT;
	    print_tele(d);
	    return;}
    case RTTO_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->redit_mode = RTELE_EDIT;
	    sprintf(bufme,"\r\n%sTeleport To Room unchanged.%s",rd,nrm);
	    print_tele(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	room = atoi(str);
	if (!(rroom = real_room(room))){
	    sprintf(bufme,"%sNo such Room.%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    sprintf(bufme,"%sEnter new destination room -1 to delete teleport:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->redit_mode = RTTO_EDIT;
	    break;}
	else{
	  if (room == -1) {
	    SEND_TO_Q("Deleting room teleport.\r\n",d);
	    d->room_edit->tele_to_room = -1;
	    d->room_edit->tele_delay = -1;
	    free(d->room_edit->tele_mesg1);
	    free(d->room_edit->tele_mesg2);	    
	    free(d->room_edit->tele_mesg3);
	    d->room_edit->tele_mesg3=0;
	    d->room_edit->tele_mesg2=0;
	    d->room_edit->tele_mesg1=0;	    	    
	    print_room(d);
	    d->redit_mode = MAIN_MODE;
	    return;
	  }
	  d->room_edit->tele_to_room = room;
	  d->redit_mode = RTELE_EDIT;
	  print_tele(d);
	  return;}
    case REXTR_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->redit_mode = REXIT_EDIT;
	    sprintf(bufme,"\r\n%sExit To Room unchanged.%s",rd,nrm);
	    print_exit(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	room = atoi(str);
	if (!(rroom = real_room(room)) && room != 0){
	  
	    sprintf(bufme,"%sNo such Room.%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    sprintf(bufme,"%sEnter new destination room:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->redit_mode = REXTR_EDIT;
	    break;}
	else{
	    d->room_edit->dir_option[d->ex_i_dir]->to_room = room;
	    d->redit_mode = REXIT_EDIT;
	    print_exit(d);
	    return;}
    case EXKEY_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->redit_mode = REXIT_EDIT;
	    sprintf(bufme,"\r\n%sKey number unchanged.%s",rd,nrm);
	    print_exit(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	room = atoi(str);
	if (!(rroom = real_object(room))){
	    sprintf(bufme,"%sNo such object.%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    sprintf(bufme,"%sEnter new key number:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->redit_mode = REXTR_EDIT;
	    break;}
	else{
	    d->room_edit->dir_option[d->ex_i_dir]->key = room;
	    d->redit_mode = REXIT_EDIT;
	    print_exit(d);
	    return;}
    case RED_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->redit_mode = RED_EDIT;
	    sprintf(bufme,"%sNo such extra description.%s",rd,nrm);
	    d->ia_flag = print_rex(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	else if (*str == 'q' || *str == 'Q'){
	    print_room(d);
	    d->redit_mode = MAIN_MODE;
	    return;}
	else if (*str == 'c' || *str == 'C'){
	    CREATE(new_desc, struct extra_descr_data, 1);
	    new_desc->keyword =0;
	    new_desc->description =0;
	    new_desc->next = 0;
	    d->iedsc =0;
	    tmp_d = d->room_edit->ex_description;
	    if (tmp_d){
		while (tmp_d->next){
		    tmp_d = tmp_d->next;
		    d->iedsc++;}
		tmp_d->next = new_desc;
	    }
	    else
		 d->room_edit->ex_description = new_desc;
	    d->redit_mode = REDD_EDIT;
	    print_rexd(d);
	    return;
	}
	points = atoi(str);
	if(!d->room_edit->ex_description){
	    SEND_TO_Q("\r\nDon't be silly, press return to continue:",d);
	    break;
	}
	if (points < 0 || points > d->ia_flag){
	    SEND_TO_Q("\r\nIllegal description #.\r\nEnter new extra description #:",d);
	    break;
	}
	else{
	    d->iedsc = points;
	    d->redit_mode = REDD_EDIT;
	    print_rexd(d);
	    return;}
    case REDD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->redit_mode = REDD_EDIT;
	    print_rexd(d);
	    return;}
	*str = LOWER(*str);
	if (*str != 'k' && *str != 'd' && *str != 'p' && *str != 'q'){
	    SEND_TO_Q("No such member.",d);
	    print_rexd(d);
	    break;}
	switch(*str){
	case 'k':
	    desc = d->room_edit->ex_description;
	    idesc = 0;
	    while(idesc < d->iedsc && desc->next){
		desc = desc->next;
		++idesc;}
	    d->str = &(desc->keyword);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter a new keyword. No carriage return at the end,\r\n terminate with a @@:\r\n",d);
	    return;
	case 'd':
	    desc = d->room_edit->ex_description;
	    idesc = 0;
	    while(idesc < d->iedsc && desc->next){
		desc = desc->next;
		++idesc;}
	    d->str = &(desc->description);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 1440;
	    SEND_TO_Q("Enter a new keyword. No carriage return at the end\r\n, terminate with a @@,\r\n",d);
	    return;
	case 'p':
	    desc = d->room_edit->ex_description;
	    idesc = 0;
	    while(idesc < d->iedsc && desc->next){
		desc = desc->next;
		++idesc;}
	    if (desc == d->room_edit->ex_description){
		tmp_d = desc->next;
		if (desc->description)
		    free(desc->description);
		if (desc->keyword)
		    free(desc->keyword);
		free(desc);
		d->room_edit->ex_description = tmp_d;}
	    else{
		for (tmp_d = d->room_edit->ex_description;
		     tmp_d;tmp_d = tmp_d->next)
		    if (tmp_d->next == desc)
			break;
		tmp_d->next = desc->next;
		if (desc->description)
		    free(desc->description);
		if (desc->keyword)
		    free(desc->keyword);
		free(desc);
	    }
	case 'q':
	    d->ia_flag = print_rex(d);
	    d->redit_mode = RED_EDIT;
	    return;
	}
    
    case RSECT_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    print_sectors(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		    d->room_edit->sector_type = ja_flag;
		    d->ia_flag =  print_sectors(d);
		    break;
		}
	    else{
		SEND_TO_Q("Illegal sector type.\r\n",d);
		d->ia_flag =  print_sectors(d);
		break;
	    }
	}
	else if (*str == 'Q'){
	    print_room(d);	
	    d->redit_mode = MAIN_MODE;
	    break;}
	else{
	    SEND_TO_Q("Illegal sector type.\r\n",d);
	    d->ia_flag =  print_sectors(d);
	    break;}

    case RFLAG_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    print_rflags(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		    if (!IS_SET(d->room_edit->room_flags, 1<<ja_flag))
			SET_BIT(d->room_edit->room_flags, 1<<ja_flag);
		    else
			REMOVE_BIT(d->room_edit->room_flags, 1<<ja_flag);
		    d->ia_flag =  print_rflags(d);
		    break;
		}
	    else{
		SEND_TO_Q("Illegal Flag.\r\n",d);
		d->ia_flag =  print_rflags(d);
		break;
	    }
	}
	else if (*str == 'Q'){
	    print_room(d);	
	    d->redit_mode = MAIN_MODE;
	    break;}
	else{
	    SEND_TO_Q("Illegal Flag.\r\n",d);
	    d->ia_flag =  print_rflags(d);
	    break;}
    case WITH_MODE:
      /*      d->str = &(d->with);
      free(*d->str);
      *d->str = 0;
      d->max_str = 29; */
      d->redit_mode = REPCON_MODE;
      return;
    case REPCON_MODE:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	sprintf(bufme, "Confirm replace %s%s%s with %s%s%s (y/n): ",
		gn,d->replace,nrm,gn,d->with,nrm);
	SEND_TO_Q(bufme,d);
	break;}	    
      *str = LOWER(*str);
      if (*str != 'n' && *str != 'y'){
	sprintf(bufme, "Confirm replace %s%s%s with %s%s%s (y/n): ",
		gn,d->replace,nrm,gn,d->with,nrm);
	SEND_TO_Q(bufme,d);
	break;}
      switch(*str){
      case 'y':
	replace_text(&(d->room_edit->description),d->replace, d->with);
      default:
	free(d->replace);
	free(d->with);
	d->replace = 0;
	d->with = 0;			
	d->redit_mode = RTEXT_EDIT;
	print_rstr(d);
	return;
      }
    case RTEXT_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	d->redit_mode = RTEXT_EDIT;
	print_rstr(d);
	break;}	    
      *str = LOWER(*str);
      if (*str != 'n' && *str != 'd' && *str != 'r' && *str != 'q'){
	SEND_TO_Q("No such mode.",d);
	break;}
      switch(*str){
      case 'n':
	d->str = &(d->room_edit->name);
	free(*d->str);
	*d->str = 0;
	d->max_str = 80;
	SEND_TO_Q("Enter a room name list. No carriage return, terminate with a @@.\r\n",d);
	return;
      case 'd':
	d->str = &(d->room_edit->description);
	free(*d->str);
	*d->str = 0;
	d->max_str = 1440;
	SEND_TO_Q("Enter a new description. Remember the carriage return at the end\r\n, terminate with a @@,\r\n",d);
	return;
      case 'r':
	d->str = &(d->replace);
	free(*d->str);
	*d->str = 0;
	d->max_str = 29;
	SEND_TO_Q("Terminate with a @@,\r\n",d);
	SEND_TO_Q("Replace: ",d);
	d->redit_mode = WITH_MODE;
	return;	  
      case 'q':
	d->redit_mode = MAIN_MODE;
	print_room(d);
	return;	    
      }
    }
}
int print_exbits(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];    
    struct room_data *j;
    int i;
    
    j = d->room_edit;
    
    sprintbit(j->dir_option[d->ex_i_dir]->exit_info, door_bits, bufm);
    sprintf(bufme,"%sExit Bits: %s%s%s\r\n\r\n",cy,gn,bufm,nrm);
    SEND_TO_Q(bufme,d);
    i = 0;
    while (*door_bits[i] != '\n')
	{
	    sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,door_bits[i],nrm);
	    i +=1;
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter Bit #to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
int print_sectors(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];    
    struct room_data *j;
    int i;
    
    j = d->room_edit;
    
    sprinttype(j->sector_type, sector_types, bufm);
    sprintf(bufme,"%sFlags: %s%s%s\r\n\r\n",cy,gn,bufm,nrm);
    SEND_TO_Q(bufme,d);
    i = 0;
    while (*sector_types[i] != '\n')
	{
	    if (*sector_types[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,sector_types[i],nrm,i+1,gn,sector_types[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,sector_types[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter Sector # or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
int print_rflags(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];    
    struct room_data *j;
    int i;
    
    j = d->room_edit;
    
    sprintbit(j->room_flags, room_bits, bufm);
    sprintf(bufme,"%sFlags: %s%s%s\r\n\r\n",cy,gn,bufm,nrm);
    SEND_TO_Q(bufme,d);
    i = 0;
    while (*room_bits[i] != '\n')
	{
	    if (*room_bits[i+2] != '\n' && *room_bits[i+1] != '\n'){
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,room_bits[i],nrm,i+1,gn,room_bits[i+1],nrm,i+2,gn,room_bits[i+2],nrm);
		i +=3;}
	    else if (*room_bits[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,room_bits[i],nrm,i+1,gn,room_bits[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,room_bits[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter flag # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
void print_rstr(struct descriptor_data *d)
{
    struct room_data *j;
    
    j = d->room_edit;
    
    sprintf(bufme,"%sDescriptions:%s (%sQ%s)uit\r\n",cy,nrm,cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sN%s)ame: %s%s%s\r\n",cy,nrm,wh,
	    ((j->name) ? j->name: "<None>"),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sD%s)escription:\r\n%s%s%s",cy,nrm,wh,
	    ((j->description) ? j->description: "<None>"),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme, "(%sR%s)eplace\r\n",gn,nrm);
    send_to_char(bufme,d->character);    
}
void print_rexd(struct descriptor_data *d)
{
    struct room_data *j;
    struct extra_descr_data *desc=0;
    int i =0;
    
    j = d->room_edit;

    desc = j->ex_description;
    while(i < d->iedsc && desc->next){
	desc = desc->next;
	++i;}
    SEND_TO_Q(cy,d);
    SEND_TO_Q("Extra Desc:\r\n",d);
    SEND_TO_Q(nrm,d);
    sprintf(bufme,"(K) Keywords:%s %s%s\r\n",gn,
	    ((desc->keyword) ? desc->keyword: "<None>") ,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"(D) Description:%s\r\n %s%s\r\n",gn,
	    ((desc->description) ? desc->description: "<None>") ,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"(%sP%s) Purge.\r\n",rd,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"(Q) Quit.\r\n");
    SEND_TO_Q(bufme,d);
}
	
int print_rex(struct descriptor_data *d)
{
    struct room_data *j;
    struct extra_descr_data *desc=0;
    int i =0;
    
    
    j = d->room_edit;
    
    send_to_char("Extra Desc:\r\n", d->character);
    send_to_char(yl,d->character);
    *bufme = '\0';
    if (j->ex_description)
	for (desc = j->ex_description; desc; desc = desc->next){
	    sprintf(bufme,"(%d) %s\r\n",i,
		    ((desc->keyword) ? desc->keyword: "<None>"));
	    i++;
	    send_to_char(bufme,d->character);
	}
    send_to_char("(C) Create new extra description.\r\n",d->character);
    send_to_char("(Q) Quit.\r\n",d->character);    
    send_to_char(nrm,d->character);
    return(i);
}
void print_tele(struct descriptor_data *d)
{
   struct room_data *j;

   j = d->room_edit;
   sprintf(bufme,"%sTeleport%s\r\n\r\n",cy,nrm);
   SEND_TO_Q(bufme,d);
   sprintf(bufme,"(T)o Room: %s%d%s\r\n",gn,j->tele_to_room,nrm);
   SEND_TO_Q(bufme,d);
   sprintf(bufme,"(D)elay:   %s%d%s\r\n",gn,j->tele_delay,nrm);
   SEND_TO_Q(bufme,d);
   if (j->tele_mesg1)
       sprintf(bufme,"(1) Message to char: \r\n%s%s%s\r\n",gn,j->tele_mesg1,nrm);
   else
       sprintf(bufme,"(1) Message to char: %s%s%s\r\n",yl,"Not Set",nrm);
   SEND_TO_Q(bufme,d);
   if (j->tele_mesg2)
       sprintf(bufme,"(2) Message to room: \r\n%s%s%s\r\n",gn,j->tele_mesg2,nrm);
   else
       sprintf(bufme,"(2) Message to room: %s%s%s\r\n",yl,"Not Set",nrm);
   SEND_TO_Q(bufme,d);
   if (j->tele_mesg3)
       sprintf(bufme,"(3) Message to destination: \r\n%s%s%s\r\n",gn,j->tele_mesg3,nrm);
   else
       sprintf(bufme,"(3) Message to destination: %s%s%s\r\n",yl,"Not Set",nrm);
   SEND_TO_Q(bufme,d);
   sprintf(bufme,"(Q)uit.\r\n");
   SEND_TO_Q(bufme,d);
}

void print_exit(struct descriptor_data *d)
{
    struct room_data *j;
    int dir;
    char bufm[MAX_STRING_LENGTH];
    j = d->room_edit;
    dir = d->ex_i_dir;
    sprintf(bufme,"%sExit %s%s%s\r\n\r\n",cy,gn,dirs[dir],nrm);
    SEND_TO_Q(bufme,d);
    if (real_room(j->dir_option[dir]->to_room) > 0 && real_room(j->dir_option[dir]->to_room) < top_of_world)
	sprintf(bufme,"(T)o Room: %s%d%s\r\n",gn,j->dir_option[dir]->to_room,nrm);
    else
	sprintf(bufme,"(T)o Room: %s%s%s\r\n",yl,"Not Set",nrm);
    SEND_TO_Q(bufme,d);
    if (j->dir_option[dir]->general_description)
	sprintf(bufme,"(D)esc:\r\n%s%s%s\r\n",gn,j->dir_option[dir]->general_description,nrm);
    else
	sprintf(bufme,"(D)esc: %s%s%s\r\n",yl,"Not Set",nrm);
    SEND_TO_Q(bufme,d);
    if (j->dir_option[dir]->keyword)
	sprintf(bufme,"(K)eywords: %s%s%s\r\n",gn,j->dir_option[dir]->keyword,nrm);
    else
	sprintf(bufme,"(K)eywords: %s%s%s\r\n",yl,"Not Set",nrm);	
    SEND_TO_Q(bufme,d);
    sprintbit(j->dir_option[dir]->exit_info, door_bits,bufm);
    sprintf(bufme,"(E)xit bits: %s%s%s\r\n",gn,bufm,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"Ke(Y): %s%d%s\r\n",gn,j->dir_option[dir]->key,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"(P)urge Exit.\r\n");
    SEND_TO_Q(bufme,d);    
    sprintf(bufme,"(Q)uit.\r\n");
    SEND_TO_Q(bufme,d);

}
void print_room(struct descriptor_data *d)

{
    struct room_data *j;
    char bufm[MAX_STRING_LENGTH];
    struct extra_descr_data *desc=0;
    

    j = d->room_edit;
    
    sprintf(cy,"%s",CCBBLU(d->character,C_NRM));
    sprintf(wh,"%s",CCWHT(d->character,C_NRM));
    sprintf(ma,"%s",CCMAG(d->character,C_NRM));
    sprintf(nrm,"%s",CCNRM(d->character,C_NRM));
    sprintf(gn,"%s",CCGRN(d->character,C_NRM));
    sprintf(yl,"%s",CCYEL(d->character,C_NRM));
    sprintf(rd,"%s",CCRED(d->character,C_NRM));                
    
    sprintf(bufme,"%sRoom Number: %d%s\r\n",cy,j->number,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sT%s)ext:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Name: %s%s%s\r\n",wh,((j->name) ? j->name: "<None>"),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Descr:\r\n%s%s%s",wh,
	    ((j->description) ? j->description: "<None>"),nrm);
    send_to_char(bufme,d->character);

    sprintbit(j->room_flags, room_bits, bufm);
    sprintf(bufme,"(%sR%s)oom Flags: %s%s%s\r\n",cy,nrm,gn,bufm,nrm);
    send_to_char(bufme,d->character);

    sprinttype(j->sector_type, sector_types, bufm);
    sprintf(bufme,"Se(%sC%s)tor Type: %s%s%s\r\n",cy,nrm,gn,bufm,nrm);
    send_to_char(bufme,d->character);

    sprintf(bufme,"Te(%sL%s)eport ",cy,nrm);
    send_to_char(bufme,d->character);
    if (j->tele_delay > -1)
	sprintf(bufm,"%sTo room: %s%s%s (%s%d%s)\r\n",wh,cy,
		((world[j->tele_to_room].name) ? world[real_room(j->tele_to_room)].name:
		 "No room name"),nrm,gn,
		j->tele_to_room,nrm);
    else
	sprintf(bufm,"%sNot Set%s\r\n",wh,nrm);
    send_to_char(bufm,d->character);
    sprintf(bufme,"(%sN%s)orth ",cy,nrm);
    send_to_char(bufme,d->character);
    if (j->dir_option[0])
	{sprintf(bufm,"%s%d%s\r\n",gn,j->dir_option[0]->to_room,nrm);
	sprintf(bufme,"%s%s%s",wh,(j->dir_option[0]->general_description ? j->dir_option[0]->general_description : bufm ),nrm);
	}
    else
	sprintf(bufme,"Not set\r\n");
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sE%s)ast  ",cy,nrm);
    send_to_char(bufme,d->character);
    if (j->dir_option[1])
	{sprintf(bufm,"%s%d%s\r\n",gn,j->dir_option[1]->to_room,nrm);
	sprintf(bufme,"%s%s%s",wh,(j->dir_option[1]->general_description ? j->dir_option[1]->general_description : bufm ),nrm);
	}
    else
	sprintf(bufme,"Not set\r\n");
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sS%s)outh ",cy,nrm);
    send_to_char(bufme,d->character);
    if (j->dir_option[2])
	{sprintf(bufm,"%s%d%s\r\n",gn,j->dir_option[2]->to_room,nrm);
	sprintf(bufme,"%s%s%s",wh,(j->dir_option[2]->general_description ? j->dir_option[2]->general_description : bufm ),nrm);
	}
    else
	sprintf(bufme,"Not set\r\n");
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sW%s)est  ",cy,nrm);
    send_to_char(bufme,d->character);
    if (j->dir_option[3])
	{sprintf(bufm,"%s%d%s\r\n",gn,j->dir_option[3]->to_room,nrm);
	sprintf(bufme,"%s%s%s",wh,(j->dir_option[3]->general_description ? j->dir_option[3]->general_description : bufm ),nrm);
	}
    else
	sprintf(bufme,"Not set\r\n");
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sU%s)p    ",cy,nrm);
    send_to_char(bufme,d->character);
    if (j->dir_option[4])
	{sprintf(bufm,"%s%d%s\r\n",cy,j->dir_option[4]->to_room,nrm);
	sprintf(bufme,"%s%s%s",wh,(j->dir_option[4]->general_description ? j->dir_option[4]->general_description : bufm ),nrm);
	}    
    else
	sprintf(bufme,"Not set\r\n");
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sD%s)own  ",cy,nrm);
    send_to_char(bufme,d->character);
    if (j->dir_option[5])
	{sprintf(bufm,"%s%d%s\r\n",cy,j->dir_option[5]->to_room,nrm);
	sprintf(bufme,"%s%s%s",wh,(j->dir_option[5]->general_description ? j->dir_option[5]->general_description : bufm ),nrm);
	}
    else
	sprintf(bufme,"Not set\r\n");
    send_to_char(bufme,d->character);	     
    sprintf(bufme,"\r\nE(%sX%s)tra Desc:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    send_to_char(yl,d->character);
    if (j->ex_description)
	for (desc = j->ex_description; desc; desc = desc->next){
	    SEND_TO_Q(((desc->keyword) ? desc->keyword: "<None>"),d);
	    if (desc->next)
		SEND_TO_Q(", ",d);}
    else
	    SEND_TO_Q("<None Set>",d);

    sprintf(bufme,".%s\r\n",nrm);    
    send_to_char(bufme,d->character);
}

void make_exit_twoway(struct descriptor_data *d)
{
    int to_room, dir;

    dir = rev_dir[d->ex_i_dir];
    to_room = real_room(d->room_edit->dir_option[d->ex_i_dir]->to_room);

    if (world[to_room].dir_option[dir]){
	sprintf(bufme,"%sCannot make exit two ways, exit already exists.\r\n%s",rd,nrm);
	SEND_TO_Q(bufme,d);
	return;}
    else{
	CREATE(world[to_room].dir_option[dir], struct room_direction_data,1);
	world[to_room].dir_option[dir]->to_room = d->room_edit->number;
	if (d->room_edit->dir_option[d->ex_i_dir]->general_description){
	    CREATE(world[to_room].dir_option[dir]->general_description, char,
		     strlen(d->room_edit->dir_option[d->ex_i_dir]->general_description) + 1);
	    strcpy(world[to_room].dir_option[dir]->general_description,
		   d->room_edit->dir_option[d->ex_i_dir]->general_description);}
	if (d->room_edit->dir_option[d->ex_i_dir]->keyword){
	    CREATE(world[to_room].dir_option[dir]->keyword, char,
		     strlen(d->room_edit->dir_option[d->ex_i_dir]->keyword)+1);
	    strcpy(world[to_room].dir_option[dir]->keyword,
		   d->room_edit->dir_option[d->ex_i_dir]->keyword);}
	world[to_room].dir_option[dir]->exit_info
	    = d->room_edit->dir_option[d->ex_i_dir]->exit_info;
	world[to_room].dir_option[dir]->key
	    = d->room_edit->dir_option[d->ex_i_dir]->key;	
    }		 
}
void replace_text(char **text, char *replace, char *with)
{
  char *p = 0;
  
  if (!(*text))
    return;
  if ((strlen(*text) + strlen(with) - strlen(replace) +1) > MAX_STRING_LENGTH)
    return;
  bzero(bufme,strlen(*text) + strlen(with) - strlen(replace) +1);
  strcpy(buf,*text);
  if (!(p = strstr(buf,replace)))
    return;
  if (p != buf)
    strncpy(bufme,buf,p-buf);
  strcat(bufme,with);
  p += strlen(replace);
  strcat(bufme,p);
  RECREATE(*text, char, strlen(bufme) + 1);
  strcpy(*text, bufme);
}

