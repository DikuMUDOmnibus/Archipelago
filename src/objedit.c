/* ************************************************************************
*   File: objedit.c                                   Part of Archipelago *
*  Usage: online object editing commands. A Neil - July 1994              *
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
#include "objedit.h"
#include "spells.h"

char *rev_search_list(int num, struct list_index_type *list);
void objedit(struct descriptor_data *d, char *str);
void print_obj(struct descriptor_data *d);
int print_oflags(struct descriptor_data *d);
void print_ostr(struct descriptor_data *d);
void print_flags(struct descriptor_data *d);
int print_owear(struct descriptor_data *d);
int print_extras(struct descriptor_data *d);
void print_stats(struct descriptor_data *d);
int print_types(struct descriptor_data *d);
void print_aff(struct descriptor_data *d);
int print_affs(struct descriptor_data *d);
int print_ex(struct descriptor_data *d);
void print_exd(struct descriptor_data *d);
void print_osiz(struct descriptor_data *d);
int print_weaps(struct descriptor_data *d);
int print_cont_flags(struct descriptor_data *d);
int print_drinks(struct descriptor_data *d);
struct list_index_type *print_spells_n(struct descriptor_data *d, int ii, int type);
bool good_spell(int spl, int tf);
int print_odams(struct descriptor_data *d);
int print_oaff(struct descriptor_data *d);
void print_poisons(struct descriptor_data *d);
int calc_min_level_scpo(struct descriptor_data *d);
int is_goditem(struct obj_data *j);
void    copy_obj(struct obj_data *from, struct obj_data *to, bool cpyextras);
void    free_objstruct(struct obj_data *obj);
int assess_item(struct obj_data *j);

extern struct room_data *world;
extern struct obj_data *obj_proto;
extern int      top_of_world;
extern char	*wear_bits[];
extern char	*extra_bits[];
extern char	*item_types[];
extern char	*apply_types[];
extern char	*drinks[];
extern char	*sizes[];
extern char	*affected_bits[];
extern char     *damage_state[];
extern char     *spells[];
extern struct list_index_type CrIg[];
extern struct list_index_type Water[];
extern struct list_index_type CrCo[];
extern struct list_index_type ReCo[];
extern struct list_index_type ReTe[];
extern struct list_index_type MuCo[];
extern struct list_index_type Animal[];
extern struct list_index_type PeCo[];
extern struct list_index_type PeIm[];
extern struct list_index_type InIm[];
extern struct list_index_type TechForms[];
extern struct list_index_type Techs[];
extern struct list_index_type Forms[];
extern struct index_data *obj_index;
extern struct spell_info_type spell_info[];
extern char *weapon_type[];
extern char     *container_bits[];

void objedit(struct descriptor_data *d, char *str)
{

    int ja_flag,points,idesc, robj, room,rroom;
    char bufme[MAX_STRING_LENGTH];
    struct extra_descr_data *desc=0, *new_desc=0, *tmp_d=0;
    
    switch (d->oedit_mode){
    case MAIN_MODE:
	for (; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);	    
	    break;}
	*str = LOWER(*str);
	if (*str == 'q'){
	    if (!IS_SET(d->obj_edit->obj_flags.extra_flags, ITEM_GODONLY)
		&& is_goditem(d->obj_edit))
		SET_BIT(d->obj_edit->obj_flags.extra_flags, ITEM_GODONLY);
	    else if (IS_SET(d->obj_edit->obj_flags.extra_flags, ITEM_GODONLY)
		&& !(is_goditem(d->obj_edit)))
		REMOVE_BIT(d->obj_edit->obj_flags.extra_flags, ITEM_GODONLY);
	    if ((robj = real_object(d->virtual)) < 0){
		SEND_TO_Q("Yikes object doesn't exist.\r\nReport this!\r\n",d);
		free_objstruct(d->obj_edit);
		d->obj_edit = 0;
		d->virtual = 0;		
		d->ia_flag = 0;
		d->iaff = 0;
		d->level = 0;
		d->iedsc = 0;
		d->cpyextras = 0;		
		d->prompt_mode = 1;
		REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
                act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
		return;}
	    d->obj_edit->item_number = robj;
	    copy_obj(d->obj_edit, obj_proto + robj, d->cpyextras);
	    SEND_TO_Q("Type saveobjs to save object.",d);    
	    free_objstruct(d->obj_edit);
	    d->obj_edit = 0;
	    d->virtual = 0;
	    d->ia_flag = 0;
	    d->iaff = 0;
	    d->level = 0;
	    d->iedsc = 0;
	    d->cpyextras = 0;
	    d->prompt_mode = 1;
	    REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
            act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
	    return;}
	else
	    switch (*str){
	    case 'f':
		d->oedit_mode = OF_EDIT;
		print_flags(d);
		return;
	    case 's':
		d->oedit_mode = OS_EDIT;
		print_stats(d);
		return;
	    case 'a':
		d->oedit_mode = OA_EDIT;
		print_aff(d);
		return;
	    case 'e':
		if (obj_index[d->obj_edit->item_number].number != 0)
		    {
			SEND_TO_Q("Cannot modify string info on loaded objects.\r\n",d);
			SEND_TO_Q("Press return to continue.\r\n",d);
			d->oedit_mode = MAIN_MODE;
			return;
		    }		
		d->oedit_mode =  OED_EDIT;
		d->ia_flag = print_ex(d);
		return;
	    case 'i':
	      if (d->obj_edit->obj_flags.type_flag == ITEM_LIGHT
		  || d->obj_edit->obj_flags.type_flag == ITEM_SRTLIGHT) {
		print_obj(d);
		SEND_TO_Q("Use the light duration for LIGHT objects\r\n",d);
		return;
	      }
	      sprintf(bufme,"%s Illumination level %s%d%s\r\n -ve numbers reduces room illumination, +ve increases room illumination.%s\r\n ",cy, byl,d->obj_edit->obj_flags2.light,gn,nrm);
	      SEND_TO_Q(bufme,d);
	      d->oedit_mode = OIL_EDIT;
	      return;
	    case 'v':
		switch (d->obj_edit->obj_flags.type_flag){
		case ITEM_SCABBARD:
		  d->oedit_mode = OSCABV_EDIT;
		  sprintf(bufme,"%sSpecific Weapon vnum for scabbard:%s",cy,nrm);
		  SEND_TO_Q(bufme,d);
		  return;
		case ITEM_SRTLIGHT:
		case ITEM_LIGHT:
		    d->oedit_mode = OVL_EDIT;
		    sprintf(bufme,"%s1/4 Hours of light:%s ",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    return;
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_WAND:
		case ITEM_STAFF:
		  print_obj(d);
		  return;
		case ITEM_PHILTRE:	
		case ITEM_CANTRIP:
		case ITEM_ROD:
		    d->oedit_mode = OVMTF_EDIT;
		    sprintf(bufme,"%sChoose a Technique and Form:%s\r\n",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    d->list = print_spells_n(d, 0,-1);
		    return;
		case ITEM_WEAPON:
		    d->oedit_mode = OVWDD_EDIT;
		    sprintf(bufme,"%sNumber of Damage Dice:%s",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    return;
		case ITEM_ARMOR:
		    d->oedit_mode = OVAC_EDIT;
		    sprintf(bufme,"%sEnhances A/C by (Negative A/C bad):%s ",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    return;
		case ITEM_VIS:
		  d->oedit_mode = OVTECH_EDIT;
		  sprintf(bufme, "%sTechnique No. for vis, 0 to clear:%s\r\n",cy,nrm);
		  SEND_TO_Q(bufme,d);
		  d->list = print_spells_n(d,1,-2);	      		  
		  return;		  
		case ITEM_CONTAINER:
		    d->oedit_mode = OVCONTC_EDIT;
		    sprintf(bufme,"%sContainer Capacity (> %d):%s ",cy,d->obj_edit->obj_flags.weight,nrm);
		    SEND_TO_Q(bufme,d);
		    return;		    		    
		case ITEM_DRINKCON:
		case ITEM_FOUNTAIN:		    
		    d->oedit_mode = OVDRINK_EDIT;
		    sprintf(bufme,"%sContainer Capacity:%s ",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    return;
		case ITEM_FOOD:
		    d->oedit_mode = OVFOOD_EDIT;
		    sprintf(bufme,"%sHours to fill stomache:%s ",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    return;
		case ITEM_KEY:
		    d->oedit_mode = OVKEY_EDIT;
		    sprintf(bufme,"%sKey-type (must match lock type for door):%s ",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    return;
		case ITEM_MONEY:
		    d->oedit_mode = OVGOLD_EDIT;
		    sprintf(bufme,"%sGold amount (crowns*1000) > 0:%s ",cy,nrm);
		    SEND_TO_Q(bufme,d);
		    return;		    
		default:
		    print_obj(d);
		    return;
		}
	    case 't':
		if (obj_index[d->obj_edit->item_number].number != 0)
		    {
			SEND_TO_Q("Cannot modify string info on loaded objects.\r\n",d);
			SEND_TO_Q("Press return to continue.\r\n",d);
			d->oedit_mode = MAIN_MODE;
			return;
		    }		
		d->oedit_mode = OSTR_EDIT;
		print_ostr(d);
		return;
	    case 'd':
		d->ia_flag = print_odams(d);
		d->oedit_mode = OVDS_EDIT;
		return;
	    case 'z':
		print_osiz(d);
		d->oedit_mode = OVS_EDIT;
		return;
	    case 'l':
		sprintf(bufme,"%sObject Limit: %s%d%s\r\n",cy,gn,d->obj_edit->obj_flags.value[6],nrm);
		SEND_TO_Q(bufme,d);	    
		sprintf(bufme,"Enter new object limit:");
		SEND_TO_Q(bufme,d); 
		d->oedit_mode = OVLM_EDIT;
		return;				
	    default:
		SEND_TO_Q("Illegal Entry, try again\r\n",d);
		print_obj(d);
		SEND_TO_Q("\r\nEnter a letter a-w or  Q to Quit\r\n",d);
		return;
	    }
	break;
    case OSCABV_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sScabbard weapon unchanged.\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sEnter Scabbard weapon type:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);	    
	    d->oedit_mode = OSCABV2_EDIT;
	    d->ia_flag = print_weaps(d);
	    return;}
	points = atoi(str);			
	if (points <= 0 || (real_object(points) < 0)){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter weapon vnum:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else if (obj_proto[real_object(points)].obj_flags.type_flag
		 != ITEM_WEAPON) {
	  sprintf(bufme,"\r\n%sThat's not a weapon.%s\r\nEnter weapon vnum:",rd,nrm);
	  SEND_TO_Q(bufme,d);
	  break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    d->obj_edit->obj_flags.value[3] = 0;
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);	    
	    return;
	}
    case OSCABV2_EDIT:
      	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    print_weaps(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		    d->obj_edit->obj_flags.value[3] = ja_flag;
		    d->ia_flag =  print_weaps(d);
		    d->obj_edit->obj_flags.value[0] = 0;
		    break;

		}
	    else{
		SEND_TO_Q("Illegal weapon type.\r\n",d);
		d->ia_flag =  print_weaps(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    break;}
	else{
	    SEND_TO_Q("Illegal weapon type.\r\n",d);
	    d->ia_flag =  print_weaps(d);
	    break;
	}
    case OVGOLD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sGold amount unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;}
	points = atoi(str);			
	if (points <= 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter gold amount (crowns*1000) > 0:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;}
    case OVKEY_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sKey type unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_obj(d) ;
	    d->oedit_mode = MAIN_MODE;
	    return;}
	points = atoi(str);			
	if (points < 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter key type >= 0:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    print_obj(d) ;
	    d->oedit_mode = MAIN_MODE;
	    return;}
    case OVFOOD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sFood value unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sPoisoned (y/n)?:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVPOIS_EDIT;
	    return;}
	points = atoi(str);			
	if (points < 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter food value >= 0:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    sprintf(bufme,"%sPoisoned (y/n)?:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVPOIS_EDIT;
	    return;}
    case OVDRINK_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sCapacity unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sDrinks Left:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVDRINKSL_EDIT;
	    return;}
	points = atoi(str);			
	if (points <= 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter container capacity > 0:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    sprintf(bufme,"%sDrinks Left:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVDRINKSL_EDIT;
	    return;}
    case OVDRINKSL_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sDrink left unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    d->ia_flag = print_drinks(d);
	    d->oedit_mode = OVDRINKTY_EDIT;
	    return;}
	points = atoi(str);			
	if (points < 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter drinks left >= 0:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[1] = points;
	    d->ia_flag = print_drinks(d);
	    d->oedit_mode = OVDRINKTY_EDIT;
	    return;}
    case OVDRINKTY_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OVDRINKTY_EDIT;
	    d->ia_flag = print_drinks(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		    d->obj_edit->obj_flags.value[2] = ja_flag;
		    d->ia_flag =  print_drinks(d);
		    break;

		}
	    else{
		SEND_TO_Q("Illegal drink type.\r\n",d);
		d->ia_flag =  print_drinks(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    d->oedit_mode = OVPOIS_EDIT;
	    sprintf(bufme,"%sPoisoned (y/n)?:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    break;}
	else{
	    SEND_TO_Q("Illegal drink type.\r\n",d);
	    d->ia_flag =  print_drinks(d);
	    break;}
    case OVPOIS_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sPoisoned state unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sEnter Poison level.\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_poisons(d);
	    d->oedit_mode = OVPOISLEV_EDIT;
	    return;}
	else if (*str == 'y' || *str == 'Y'){
	    d->obj_edit->obj_flags.value[3] = 1;
	    sprintf(bufme,"%sEnter Poison level.\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_poisons(d);
	    d->oedit_mode = OVPOISLEV_EDIT;
	    return;}
	else {
	    d->obj_edit->obj_flags.value[3] = 0;
	    print_obj(d) ;
	    d->oedit_mode = MAIN_MODE;
	    return;}
    case OVPOISLEV_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sPoison unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_obj(d);
	    d->oedit_mode = MAIN_MODE;
	}
	points = atoi(str);			
	if (points < 0 || points > 5){
	    sprintf(bufme,"\r\n%sIllegal number.%s Enter Poison > 0 <=5.\r\n",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_poisons(d);
	    break;;}
	else{
	    d->obj_edit->obj_flags.value[7] = points;
	    print_obj(d);
	    d->oedit_mode = MAIN_MODE;
	    break;}
    case OVCONTC_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sCapacity unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    d->ia_flag = print_cont_flags(d);
	    d->oedit_mode = OVCFLAGS_EDIT;
	    return;}
	points = atoi(str);			
	if (points < d->obj_edit->obj_flags.weight){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter container capacity >  %d:",rd,nrm,d->obj_edit->obj_flags.weight);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    d->oedit_mode = OVCFLAGS_EDIT;
	    d->ia_flag = print_cont_flags(d);
	    return;}
    case OVCFLAGS_EDIT:
	    for ( ; isspace(*str); str++)
		;
	if (!*str){
		d->oedit_mode = OVCFLAGS_EDIT;
		d->ia_flag = print_cont_flags(d);
		break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		    {
			if (!IS_SET(d->obj_edit->obj_flags.value[1], 1<<ja_flag))
			    SET_BIT(d->obj_edit->obj_flags.value[1], 1<<ja_flag);
			else
			    REMOVE_BIT(d->obj_edit->obj_flags.value[1], 1<<ja_flag);
			d->ia_flag = print_cont_flags(d);
			break;
			
		    }
	    else{
		SEND_TO_Q("Illegal Flag.\r\n",d);
		d->ia_flag = print_cont_flags(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    sprintf(bufme,"Item number of key for lock (-1 == no lockability): %s%d%s",cy,d->obj_edit->obj_flags.value[2],nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVCKEY_EDIT;
	    break;}
	else{
	    SEND_TO_Q("Illegal Flag.\r\n",d);
	    d->ia_flag = print_cont_flags(d);
	    break;}
    case OVCKEY_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sKey number unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    d->oedit_mode = OVROOM_EDIT;
	    sprintf(bufme,"%sContainer room: %s%d%s\r\n",cy,rd,
		(d->obj_edit->obj_flags.value[3] > 0 ?
		d->obj_edit->obj_flags.value[3] : 0), nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sEnter new container room:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
	points = atoi(str);			
	if (points > 99999){
	    SEND_TO_Q("\r\nIllegal number.\r\nEnter new key number, <99999:",d);
	    break;
	}
	else{
	    if (points < 0)
		points = -1;
	    d->obj_edit->obj_flags.value[2] = points;
	    d->oedit_mode = OVROOM_EDIT;
	    sprintf(bufme,"%sContainer room: %s%d%s\r\n",cy,rd,
		(d->obj_edit->obj_flags.value[3] > 0 ?
		d->obj_edit->obj_flags.value[3] : 0), nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sEnter new container room:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
    case OVROOM_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"\r\n%sContainer To Room unchanged.%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;}
	room = atoi(str);
	if ((rroom = real_room(room)) == NOWHERE){
	    sprintf(bufme,"%sNo such Room.%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    sprintf(bufme,"%sEnter new container room:%s ",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVROOM_EDIT;
	    break;}
	else{
	    d->obj_edit->obj_flags.value[3] = room;
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;}
    case OVTECH_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->list = print_spells_n(d,1,-2);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag == 0){
		  d->obj_edit->obj_flags.value[1] = 0;	      
		  d->list = print_spells_n(d,1,-2);
		  break;
	    }
	    if (rev_search_list(ja_flag, d->list))
		{
		  d->obj_edit->obj_flags.value[1] = ja_flag;
		  d->list = print_spells_n(d,1,-2);
		  break;
		}
	    else{
	      SEND_TO_Q("Illegal technique No. 0 to clear\r\n",d);
	      d->list = print_spells_n(d,1,-2);
	      break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	  sprintf(bufme,"%sEnter Form No. 0 to clear:%s\r\n ",cy,nrm);
	  SEND_TO_Q(bufme,d);
	  d->list = print_spells_n(d,2,-3);	  
	  d->oedit_mode = OVFORMS_EDIT;
	  break;}
	else{
	  SEND_TO_Q("Illegal technique No. 0 to clear\r\n",d);
	  d->list =  print_spells_n(d,1,-2);
	  break;}
    case OVFORMS_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	d->list = print_spells_n(d,2,-3);
	break;}
      if (is_number(str)){
	ja_flag = atoi(str);
	if (ja_flag == 0){
	  d->obj_edit->obj_flags.value[2] = 0;	      
	  d->list = print_spells_n(d,2,-3);
	  break;
	}
	if (rev_search_list(ja_flag, d->list))
	  {
	    d->obj_edit->obj_flags.value[2] = ja_flag;
	    d->list =print_spells_n(d,2,-3);
	    break;
	  }
	else{
	  SEND_TO_Q("Illegal Form.\r\n",d);
	  d->list = print_spells_n(d,2,-3);
	  break;
	}
      }
      else if (*str == 'Q' || *str == 'q'){
	if (d->obj_edit->obj_flags.value[2] == 0 &&
	    d->obj_edit->obj_flags.value[1] == 0){
	  d->obj_edit->obj_flags.value[3] = 0;  
	  print_obj(d);
	  d->oedit_mode = MAIN_MODE;
	  return;
	}
	sprintf(bufme,"%sNumber of pawns: %d%s "
		,cy,d->obj_edit->obj_flags.value[3],nrm);
	SEND_TO_Q(bufme,d);	  
	sprintf(bufme,"%sEnter new number of pawns >0:%s ",cy,nrm);
	SEND_TO_Q(bufme,d);
	d->oedit_mode = OVPAWNS_EDIT;
	break;	      
	break;}
      else{
	SEND_TO_Q("Illegal Form No. 0 to clear:\r\n",d);
	d->list =  print_spells_n(d,2,-3);
	break;}
    case OVPAWNS_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str) {
	if (d->obj_edit->obj_flags.value[3] < 1){
	  sprintf(bufme,"%sSetting pawns to default = 1.%s", rd, nrm);
	  d->obj_edit->obj_flags.value[3] = 1;
	  SEND_TO_Q(bufme,d);
	  print_obj(d);
	  d->oedit_mode = MAIN_MODE;
	}
	sprintf(bufme,"%sPawns unchanged.\r\n\r\n%s",rd,nrm);
	SEND_TO_Q(bufme,d);
	print_obj(d);
	d->oedit_mode = MAIN_MODE;
	return;}
      points = atoi(str);			
      if (points > 10 || points < 1){
	sprintf(bufme,"\r\n%sIllegal number.%s\r\nNumber of pawns >= 1 <= 10:",rd,nrm);
	SEND_TO_Q(bufme,d);
	break;
      }
      else{
	d->obj_edit->obj_flags.value[3] = points;
	print_obj(d);
	d->oedit_mode = MAIN_MODE;
	return;}      
    case OVAC_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	sprintf(bufme,"%sA/C enhancement unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sStopping power:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVWAST_EDIT;
	    return;}
	points = atoi(str);			
	if (points > 40 || points < -40){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter A/C enhancement >= -40 <= 40:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    d->oedit_mode = OVWAST_EDIT;
	    sprintf(bufme,"%sStopping power:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
    case OVWAST_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sStopping power unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_obj(d);
	    d->oedit_mode = MAIN_MODE;
	    return;}
	points = atoi(str);			
	if (points > 20 || points < 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter A/C enhancement >= 0 <= 20:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[1] = points;
	    print_obj(d);
	    d->oedit_mode = MAIN_MODE;
	    return;}
    case OVWDD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sNumber of damage dice unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sSize of Damage Dice:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVWSDD_EDIT;
	    return;}
	points = atoi(str);			
	if (points < 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter number of damage dice > 0 <= 40:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[1] = points;
	    d->oedit_mode = OVWSDD_EDIT;
	    sprintf(bufme,"%sSize of Damage Dice:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
    case OVWSDD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sSize of damage dice unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sWeapon type:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->ia_flag = print_weaps(d);
	    d->oedit_mode = OVWT_EDIT;
	    return;}
	points = atoi(str);			
	if (points < 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter number of damage dice > 0 <= 20:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[2] = points;
	    sprintf(bufme,"%sWeapon type:%s",cy,nrm);
	    SEND_TO_Q(bufme,d);
	    d->ia_flag = print_weaps(d);
	    d->oedit_mode = OVWT_EDIT;
	    return;}
    case OVWT_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OVWT_EDIT;
	    print_weaps(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		  d->obj_edit->obj_flags.value[3] = ja_flag;
		  d->ia_flag =  print_weaps(d);
		  break;
		  
		}
	    else{
	      SEND_TO_Q("Illegal weapon type.\r\n",d);
	      d->ia_flag =  print_weaps(d);
	      break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	  d->oedit_mode = MAIN_MODE;
	  print_obj(d);
	  break;}
	else{
	  SEND_TO_Q("Illegal weapon type.\r\n",d);
	  d->ia_flag =  print_weaps(d);
	  break;}
    case OVWSC_EDIT:
      for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    sprintf(bufme,"%sMax Charges unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;}
	points = atoi(str);			
	if (points > 20 || points <= 0){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter Max Charges > 0 <= 20:",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[2] = points;
	    d->obj_edit->obj_flags.value[3] = points;	    
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;}
    case OVMTF_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OVMTF_EDIT;
	    d->list = print_spells_n(d,0,-1);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (rev_search_list(ja_flag, d->list))
		{
		    d->obj_edit->obj_flags.value[0] = ja_flag;
		    d->list =  print_spells_n(d,0,-1);
		    break;

		}
	    else{
		SEND_TO_Q("Illegal technique and form No.\r\n",d);
		d->list =  print_spells_n(d,0,-1);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    sprintf(bufme,"%sChoose a Spell:%s ",cy,nrm);
	    d->list = print_spells_n(d,1,d->obj_edit->obj_flags.value[0]);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = OVMSPL_EDIT;
	    break;}
	else{
	    SEND_TO_Q("Illegal technique and form No.\r\n",d);
	    d->list =  print_spells_n(d,0, -1);
	    break;}
    case OVMSPL_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OVMSPL_EDIT;
	    d->list = print_spells_n(d,1,d->obj_edit->obj_flags.value[0]);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (rev_search_list(ja_flag, d->list))
		{
		  if (!spell_info[ja_flag].spll_pointer) {
		    SEND_TO_Q("Spell not implimented yet choose again.\r\n",d);
		    d->list
		      =print_spells_n(d,1,d->obj_edit->obj_flags.value[0]);
		    break;
		  }
		  d->obj_edit->obj_flags.value[1] = ja_flag;
		  d->list =print_spells_n(d,1,d->obj_edit->obj_flags.value[0]);
		  break;
		}
	    else{
	      SEND_TO_Q("Illegal Spell No.\r\n",d);
	      d->list = print_spells_n(d,1,d->obj_edit->obj_flags.value[0]);
	      break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	  if (d->obj_edit->obj_flags.type_flag == ITEM_CANTRIP ||
	      d->obj_edit->obj_flags.type_flag == ITEM_PHILTRE) {
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;
	  }
	  sprintf(bufme,"%sMax Charges > 0 <= 20:%s ",cy,nrm);
	  SEND_TO_Q(bufme,d);
	  d->oedit_mode = OVWSC_EDIT;
	  break;}
	else{
	  SEND_TO_Q("Illegal spell No.\r\n",d);
	  d->list =  print_spells_n(d,1,d->obj_edit->obj_flags.value[0]);
	  break;}
    case OVSPLEV_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = MAIN_MODE;
	    sprintf(bufme,"%sSpell Level unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    print_obj(d) ;
	    return;}
	points = atoi(str);			
	if (points < d->level || points > LEVEL_IMPL){
	    sprintf(bufme,"\r\n%sIllegal number.%s\r\nEnter new Spell level >= %d <= 227:",rd,nrm,d->level);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[0] = points;
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d) ;
	    return;}
    case OIL_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = MAIN_MODE;
	    sprintf(bufme,"%sLight duration unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);	    
	    print_obj(d) ;
	    return;}
	points = atoi(str);			
	if (points > 30 || points < -30){
	    SEND_TO_Q("\r\nIllegal number.\r\nEnter new illumination value >= -30  and <= 30:",d);
	    break;
	}
	d->obj_edit->obj_flags2.light = points;
	d->oedit_mode = MAIN_MODE;
	print_obj(d) ;
	return;
    case OVL_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	d->oedit_mode = MAIN_MODE;
	sprintf(bufme,"%sLight duration unchanged.\r\n\r\n%s",rd,nrm);
	SEND_TO_Q(bufme,d);	    
	print_obj(d) ;
	return;}
      points = atoi(str);			
      if (points > 160 || points < 0){
	SEND_TO_Q("\r\nIllegal number.\r\nEnter new light duration 1/4 hours, <= 160:",d);
	break;
      }
      d->obj_edit->obj_flags.value[2] = points;
      d->oedit_mode = MAIN_MODE;
      print_obj(d) ;
      return;
    case OVDS_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	d->oedit_mode = OVDS_EDIT;
	d->ia_flag = print_odams(d);
	break;}
      if (*str == 'Q' || *str == 'q'){
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);	
	    break;}
	else if ((ja_flag = atoi(str)) >= -3){
	    if (ja_flag >= -3 && ja_flag <= d->ia_flag )
		{
		    d->obj_edit->obj_flags.value[4] = ja_flag;
		    d->oedit_mode = MAIN_MODE;
		    print_obj(d);
		    break;
		}
	    else{
		SEND_TO_Q("Illegal damage state.\r\n",d);
		d->oedit_mode = OVDS_EDIT;
		d->ia_flag =  print_odams(d);
		break;
	    }
	}
	else{
	    SEND_TO_Q("Illegal damage state.\r\n",d);
	    d->oedit_mode = OVDS_EDIT;
	    d->ia_flag =  print_odams(d);
	    break;}
    case OVS_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = MAIN_MODE;
	    sprintf(bufme,"%sSize unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_obj(d);
	    return;}
	points = atoi(str);			
	if (points > 6 || points < 0){
	    SEND_TO_Q("\r\nIllegal size.\r\nEnter new size,>= 1 <= 6:",d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[5] = points;
	    print_obj(d);
	    d->oedit_mode = MAIN_MODE;
	    return;}
    case OVLM_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = MAIN_MODE;
	    sprintf(bufme,"%sObject limit unchanged.\r\n\r\n%s",rd,nrm);
	    SEND_TO_Q(bufme,d);
	    print_obj(d);
	    return;}
	points = atoi(str);			
	if (points > 200 || points < 0){
	    SEND_TO_Q("\r\nIllegal limit.\r\nEnter new limit,>= 0 <= 200:",d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.value[6] = points;
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;}
    case OED_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OED_EDIT;
	    sprintf(bufme,"%sNo such extra description.%s",rd,nrm);
	    d->ia_flag = print_ex(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	else if (*str == 'q' || *str == 'Q'){
	    print_obj(d);
	    d->oedit_mode = MAIN_MODE;
	    return;}
	else if (*str == 'c' || *str == 'C'){
	    if (obj_index[d->obj_edit->item_number].number != 0)
		{
		    SEND_TO_Q("Cannot modify extra descriptions on loaded objects.\r\n",d);
		    d->ia_flag = print_ex(d);
		    return;
		}
	    d->cpyextras = TRUE;
	    CREATE(new_desc, struct extra_descr_data, 1);
	    new_desc->keyword =0;
	    new_desc->description =0;
	    new_desc->next = 0;
	    d->iedsc = 0;
	    tmp_d = d->obj_edit->ex_description;
	    if (tmp_d){
		while (tmp_d->next){
		    tmp_d = tmp_d->next;
		    d->iedsc++;}
		tmp_d->next = new_desc;
	    }
	    else
		 d->obj_edit->ex_description = new_desc;
	    d->oedit_mode = OEDD_EDIT;
	    print_exd(d);
	    return;
	}
	points = atoi(str);
	if (obj_index[d->obj_edit->item_number].number != 0)
	    {
		SEND_TO_Q("Cannot modify extra descriptions on loaded objects.\r\n",d);
		d->ia_flag = print_ex(d);
		return;
	    }
	if(!d->obj_edit->ex_description){
	    SEND_TO_Q("\r\nDon't be silly, press return to continue:",d);
	    break;
	}
	if (points < 0 || points > d->ia_flag){
	    SEND_TO_Q("\r\nIllegal description #.\r\nEnter new extra description #:",d);
	    break;
	}
	else{
	    d->cpyextras = TRUE;	
	    d->iedsc = points;
	    d->oedit_mode = OEDD_EDIT;
	    print_exd(d);
	    return;}
    case OEDD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OEDD_EDIT;
	    print_exd(d);
	    return;}
	*str = LOWER(*str);
	if (*str != 'k' && *str != 'd' && *str != 'p' && *str != 'q'){
	    SEND_TO_Q("No such member.",d);
	    print_exd(d);
	    break;}
	switch(*str){
	case 'k':
	    desc = d->obj_edit->ex_description;
	    idesc = 0;
	    while(idesc < d->iedsc && desc->next){
		desc = desc->next;
		++idesc;}
	    d->str = &(desc->keyword);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter new keywords. No carriage return at the end\r\n, terminate with a @@,\r\n",d);
	    return;
	case 'd':
	    desc = d->obj_edit->ex_description;
	    idesc = 0;
	    while(idesc < d->iedsc && desc->next){
		desc = desc->next;
		++idesc;}
	    d->str = &(desc->description);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 3072;
	    SEND_TO_Q("Enter new keywords. No carriage return at the end\r\n, terminate with a @@,\r\n",d);
	    return;
	case 'p':
	    desc = d->obj_edit->ex_description;
	    idesc = 0;
	    while(idesc < d->iedsc && desc->next){
		desc = desc->next;
		++idesc;}
	    if (desc == d->obj_edit->ex_description){
		tmp_d = desc->next;
		if(desc->keyword)		
		    free(desc->keyword);
		if(desc->description)		
		free(desc->description);		
		free(desc);
		d->obj_edit->ex_description = tmp_d;}
	    else{
		for (tmp_d = d->obj_edit->ex_description;
		     tmp_d;tmp_d = tmp_d->next)
		    if (tmp_d->next == desc)
			break;
		tmp_d->next = desc->next;
		if(desc->keyword)
		    free(desc->keyword);
		if(desc->description)				
		    free(desc->description);				
		free(desc);
	    }
	case 'q':
	    d->ia_flag = print_ex(d);
	    d->oedit_mode = OED_EDIT;
	    return;
	}
    case OA_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OS_EDIT;
	    sprintf(bufme,"%sNo such affect number.%s",rd,nrm);
	    print_aff(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	else if (*str == 'q' || *str == 'Q'){
	    print_obj(d);
	    d->oedit_mode = MAIN_MODE;
	    return;}
	points = atoi(str);			
	if (points < 0 || points > MAX_OBJ_AFFECT){
	    SEND_TO_Q("\r\nIllegal affect #.\r\nEnter new obj affect #:",d);
	    break;
	}
	else{
	    d->iaff = points;
	    d->oedit_mode = OAL_EDIT;
	    d->ia_flag  = print_affs(d);
	    return;}
    case OAL_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OAL_EDIT;
	    d->ia_flag = print_affs(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		    d->obj_edit->affected[d->iaff].location = ja_flag;
		    sprintf(bufme,"%sModifier:%s %d %s\r\n",cy,gn,d->obj_edit->affected[d->iaff].modifier,nrm);
		    SEND_TO_Q(bufme,d);
		    sprintf(bufme,"Enter new modifier: ");
		    SEND_TO_Q(bufme,d);
		    d->oedit_mode = OAM_EDIT;
		    break;
		}
	    else{
		SEND_TO_Q("Illegal location.\r\n",d);
		d->ia_flag =  print_affs(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){

	  sprintf(bufme,"%sModifier:%s %d %s\r\n",cy,gn,d->obj_edit->affected[d->iaff].modifier,nrm);
	  SEND_TO_Q(bufme,d);
	  sprintf(bufme,"Enter new modifier: ");
	  SEND_TO_Q(bufme,d);
	  d->oedit_mode = OAM_EDIT;
	  break;}
	else{
	    SEND_TO_Q("Illegal type.\r\n",d);
	    d->ia_flag =  print_types(d);
	    break;}
    case OAM_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OA_EDIT;
	    sprintf(bufme,"%sModifier unchanged.\r\n\r\n%s",rd,nrm);
	    print_aff(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	points = atoi(str);			
	if (points < -100 || points > 100){
	    SEND_TO_Q("\r\nIllegal modifier.\r\nEnter new modifier, >= -100, <= 100:",d);
	    break;
	}
	else{
	    d->obj_edit->affected[d->iaff].modifier = atoi(str);
	    d->oedit_mode = OA_EDIT;
	    print_aff(d);
	    return;}
    case OS_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OS_EDIT;
	    print_stats(d);
	    break;}	    
	*str = LOWER(*str);
	if (*str != 't' && *str != 'w' && *str != 'v' && *str != 'c' && *str != 'q'){
	    SEND_TO_Q("No such stat.",d);
	    print_stats(d);
	    break;}
	switch(*str){
	case 't':
	    d->ia_flag = print_types(d);
	    d->oedit_mode = OET_EDIT;
	    return;
	case 'w':
	    sprintf(bufme,"%sWeight: %s%d\r\n",cy,gn,d->obj_edit->obj_flags.weight);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sEnter new obj weight:%s ",cy,gn);
	    SEND_TO_Q(bufme,d);	    
	    d->oedit_mode = OWE_EDIT;
	    return;	    
	case 'v':
	    sprintf(bufme,"%sValue: %s%d\r\n",cy,gn,d->obj_edit->obj_flags.cost);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sEnter new obj value (crowns*1000):%s ",cy,gn);
	    SEND_TO_Q(bufme,d);	    
	    d->oedit_mode = OC_EDIT;
	    return;	    
	case 'c':
	    sprintf(bufme,"%sCost/day: %s%d\r\n",cy,gn,d->obj_edit->obj_flags.cost_per_day);
	    SEND_TO_Q(bufme,d);
	    sprintf(bufme,"%sEnter new obj cost/day (crowns*1000):%s ",cy,gn);
	    SEND_TO_Q(bufme,d);	    
	    d->oedit_mode = OCD_EDIT;
	    return;	    
	case 'q':
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;
	}
    case OCD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OS_EDIT;
	    sprintf(bufme,"%sCost/day unchanged.\r\n\r\n%s",rd,nrm);
	    print_stats(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	points = atoi(str);			
	if (points < 0 || points > 1000000){
	    SEND_TO_Q("\r\nIllegal Cost/day.\r\nEnter new obj cost/day, >= 0, <= 1000000:",d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.cost_per_day = atoi(str);
	    d->oedit_mode = OS_EDIT;
	    print_stats(d);
	    return;}
    case OC_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OS_EDIT;
	    sprintf(bufme,"%sValue unchanged.\r\n\r\n%s",rd,nrm);
	    print_stats(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	points = atoi(str);			
	if (points < 0 || points > 1000000){
	    SEND_TO_Q("\r\nIllegal Value.\r\nEnter new obj value, >= 0, <= 1000000:",d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.cost = atoi(str);
	    d->oedit_mode = OS_EDIT;
	    print_stats(d);
	    return;}
    case OWE_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OS_EDIT;
	    sprintf(bufme,"%sWeight unchanged.\r\n\r\n%s",rd,nrm);
	    print_stats(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	points = atoi(str);			
	if (points < 0 || points > 100000){
	    SEND_TO_Q("\r\nIllegal weight.\r\nEnter new obj weight, >= 0, <= 100000:",d);
	    break;
	}
	else{
	    d->obj_edit->obj_flags.weight = atoi(str);
	    d->oedit_mode = OS_EDIT;
	    print_stats(d);
	    return;}
    case OF_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OF_EDIT;
	    print_flags(d);
	    break;}	    
	*str = LOWER(*str);
	if (*str != 'w' && *str != 'e' && *str != 'a' && *str != 'q'){
	    SEND_TO_Q("No such flag.",d);
	    break;}
	switch(*str){
	case 'a':
	    if (GET_ITEM_TYPE(d->obj_edit) == ITEM_WAND ||
		GET_ITEM_TYPE(d->obj_edit) == ITEM_STAFF ||
		GET_ITEM_TYPE(d->obj_edit) == ITEM_TRASH){
		SEND_TO_Q("Sorry no affect bitvector allowed for this object type.\r\n",d);
		print_flags(d);
		return;
	    }	    
	    d->oedit_mode = OAFF_EDIT;
	    d->ia_flag = print_oaff(d);
	    return;
	case 'w':
	    d->oedit_mode = OW_EDIT;
	    d->ia_flag = print_owear(d);
	    return;
	case 'e':
	    d->oedit_mode = OEX_EDIT;
	    d->ia_flag = print_extras(d);
	    return;	    
	case 'q':
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;
	}
    case OEX_EDIT:
	    for ( ; isspace(*str); str++)
		;
	if (!*str){
		d->oedit_mode = OEX_EDIT;
		print_extras(d);
		break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		    {
			if (!IS_SET(d->obj_edit->obj_flags.extra_flags, 1<<ja_flag))
			    SET_BIT(d->obj_edit->obj_flags.extra_flags, 1<<ja_flag);
			else{
			    if ((1<<ja_flag) != ITEM_GODONLY)
				REMOVE_BIT(d->obj_edit->obj_flags.extra_flags, 1<<ja_flag);
			    else
				SEND_TO_Q("You are not holy enough to remove this flag.\r\n",d);
			}
			d->ia_flag =  print_extras(d);
			break;
			
		    }
	    else{
		SEND_TO_Q("Illegal Flag.\r\n",d);
		d->ia_flag =  print_extras(d);
		break;
	    }
	    }
	else if (*str == 'Q' || *str == 'q'){
	    print_flags(d);	
	    d->oedit_mode = OF_EDIT;
	    break;}
	else{
	    SEND_TO_Q("Illegal Flag.\r\n",d);
	    d->ia_flag =  print_extras(d);
	    break;}
    case OET_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OET_EDIT;
	    print_types(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		  if (GET_ITEM_TYPE(d->obj_edit) != ja_flag){
		    GET_ITEM_TYPE(d->obj_edit) = ja_flag;
		    for (points=0;points<8;points++)
		      d->obj_edit->obj_flags.value[points] = 0;
		    d->obj_edit->obj_flags.extra_flags = 0;
		    d->obj_edit->obj_flags.bitvector = 0;
		    d->obj_edit->obj_flags.wear_flags = 0;
		    for (points = 0; points < MAX_OBJ_AFFECT; points++){
		      d->obj_edit->affected[points].location = 0;
		      d->obj_edit->affected[points].modifier = 0;
		    }
		  }
		  d->ia_flag =  print_types(d);
		  break;
		}
	    else{
		SEND_TO_Q("Illegal Flag.\r\n",d);
		d->ia_flag =  print_types(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    print_stats(d);	
	    d->oedit_mode = OS_EDIT;
	    break;}
	else{
	    SEND_TO_Q("Illegal type.\r\n",d);
	    d->ia_flag =  print_types(d);
	    break;}
    case OW_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OW_EDIT;
	    print_owear(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		  if (!IS_SET(d->obj_edit->obj_flags.wear_flags, 1<<ja_flag))
		    SET_BIT(d->obj_edit->obj_flags.wear_flags, 1<<ja_flag);
		  else
		    REMOVE_BIT(d->obj_edit->obj_flags.wear_flags, 1<<ja_flag);
		  d->ia_flag =  print_owear(d);
		  break;
		}
	    else{
	      SEND_TO_Q("Illegal Flag.\r\n",d);
	      d->ia_flag =  print_owear(d);
	      break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	  print_flags(d);	
	    d->oedit_mode = OF_EDIT;
	    break;}
	else{
	    SEND_TO_Q("Illegal Flag.\r\n",d);
	    d->ia_flag =  print_owear(d);
	    break;}
    
    case OAFF_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OAFF_EDIT;
	    print_oaff(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		    if (!IS_SET(d->obj_edit->obj_flags.bitvector, 1<<ja_flag))
			SET_BIT(d->obj_edit->obj_flags.bitvector, 1<<ja_flag);
		    else
			REMOVE_BIT(d->obj_edit->obj_flags.bitvector, 1<<ja_flag);
		    d->ia_flag =  print_oaff(d);
		    break;

		}
	    else{
		SEND_TO_Q("Illegal Flag.\r\n",d);
		d->ia_flag =  print_oaff(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    if (d->obj_edit->obj_flags.bitvector == 0){
		print_flags(d);	
		d->oedit_mode = OF_EDIT;
		break;}
	    else{
		sprintf(bufme,"Affect duration: %s%d%s\r\nNew Duration: ",
			cy,d->obj_edit->obj_flags2.aff_dur,nrm);
		SEND_TO_Q(bufme,d);
		d->oedit_mode = OAFDUR_EDIT;
		break;}
	    }
	else{
	    SEND_TO_Q("Illegal Flag.\r\n",d);
	    d->ia_flag =  print_oaff(d);
	    break;}
    case OAFDUR_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    SEND_TO_Q("Affect Duration unchanged.\r\n",d);
	    sprintf(bufme,"No Use duration: %s%d%s\r\nNew Duration: ",
			cy,d->obj_edit->obj_flags2.no_use_dur,nrm);
	    SEND_TO_Q(bufme
,d);
	    d->oedit_mode = ONUDUR_EDIT;
		break;}
	points = atoi(str);
	if (points < 1 || points > 14){
	    sprintf(bufme,"%sIllegal  duration%s: Range (1-14)\r\nNew Duration:",
		    rd,nrm);
	    SEND_TO_Q(bufme,d);
	    break;}
	else{
	    d->obj_edit->obj_flags2.aff_dur = points;
	     sprintf(bufme,"No Use duration: %s%d%s\r\nNew Duration: ",
			cy,d->obj_edit->obj_flags2.no_use_dur,nrm);
	    SEND_TO_Q(bufme,d);
	    d->oedit_mode = ONUDUR_EDIT;
	    break;}
    case ONUDUR_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    SEND_TO_Q("No Use Duration unchanged.\r\n",d);
	    d->obj_edit->obj_flags2.no_use_dur =
		2*d->obj_edit->obj_flags2.aff_dur;
	    print_flags(d);	
	    d->oedit_mode = OF_EDIT;
	    break;}
	points = atoi(str);
	if (points < 2*d->obj_edit->obj_flags2.aff_dur){
	    sprintf(bufme,"%sIllegal  duration%s: Range (>%d)\r\nNew Duration:",
		    rd,nrm, 2*d->obj_edit->obj_flags2.aff_dur);
	    SEND_TO_Q(bufme,d);
	    break;}
	else{
	    d->obj_edit->obj_flags2.no_use_dur = points;
	    print_flags(d);	
	    d->oedit_mode = OF_EDIT;	     
	    break;}
    case OSTR_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->oedit_mode = OSTR_EDIT;
	    print_ostr(d);
	    break;}	    
	*str = LOWER(*str);
	if (*str != 'n' && *str != 's' && *str != 'a' && *str != 'd' && *str != 'q'){
	    SEND_TO_Q("No such string.\r\n",d);
	    break;}
	switch(*str){
	case 'n':
	    d->str = &(d->obj_edit->name);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter a new name list. No carriage return, terminate with a @@.\r\n",d);
	    return;
	case 's':
	    d->str = &(d->obj_edit->short_description);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter a new short description. No carriage return, terminate with a @@.\r\n",d);
	    return;
	case 'a':
	    d->str = &(d->obj_edit->action_description);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter a new action description. Carriage return at the end\r\n, terminate with a @@.\r\n",d);
	    return;
	case 'd':
	    d->str = &(d->obj_edit->description);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 80;
	    SEND_TO_Q("Enter a new description. No carriage return at the end\r\n, terminate with a @@,\r\n",d);
	    return;
	case 'q':
	    d->oedit_mode = MAIN_MODE;
	    print_obj(d);
	    return;	    
	}

    }
}
void print_poisons(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;


    j = d->obj_edit;
    sprintf(bufm,"Current Poison: %s%d%s\r\n\r\n",rd,j->obj_flags.value[7],nrm);
    sprintf(bufm,"%s%s0%s) Mild     1-12    pts per tic\r\n",bufm, bgn,nrm);
    sprintf(bufm,"%s%s1%s) Mild     6-24    pts per tic\r\n",bufm, yl,nrm);
    sprintf(bufm,"%s%s2%s) Moderate 12-48   pts per tic\r\n",bufm, yl,nrm);
    sprintf(bufm,"%s%s3%s) Strong   24-96   pts per tic\r\n",bufm, yl,nrm);
    sprintf(bufm,"%s%s4%s) V Strong 48-192  pts per tic\r\n",bufm, rd,nrm);
    sprintf(bufm,"%s%s5%s) Deadly   200-500 pts per tic\r\n",bufm, rd,nrm);
    SEND_TO_Q(bufm,d);
}
int print_drinks(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    i = j->obj_flags.value[2];
    sprintf(bufm ,"\r\n%sDrink Type:%s %s%s%s\r\n\r\n",cy,nrm,gn,drinks[i],nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*drinks[i] != '\n')
	{
	    if (*drinks[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,drinks[i],nrm,i+1,gn,drinks[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,drinks[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter Drink Type or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
int print_cont_flags(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];
    char bufme[MAX_STRING_LENGTH];    
    struct obj_data *j;
    int i;
    
    j = d->obj_edit;
    
    sprintbit(j->obj_flags.value[1], container_bits, bufme);
    sprintf(bufm ,"\r\n%sAffected by Flags:%s %s%s%s\r\n\r\n",cy,nrm,gn,bufme,nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*container_bits[i] != '\n')
	{
	    sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,container_bits[i],nrm);
	    i +=1;
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter flag # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
void print_osiz(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];    
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    i = j->obj_flags.value[5];
    sprintf(bufme ,"\r\n%sObject is: %s%s%s\r\n\r\n",cy,gn, sizes[i],nrm);
    i = 0;
    while (*sizes[i] != '\n'){
	sprintf(bufme,"%s(%s%d%s) %s%s%s\r\n", bufme,cy,i,nrm,gn,sizes[i],nrm);
	i++;
    }
    SEND_TO_Q(bufme,d);
}
int print_odams(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    i = j->obj_flags.value[4];
    sprintf(bufm ,"\r\n%sObject is: %s%s%s\r\n\r\n",cy,gn,damage_state[i],nrm);
    SEND_TO_Q(bufm,d);
    
    i = 0;
    while (*damage_state[i] != '\n')
	{
	    sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,damage_state[i],nrm);
	    SEND_TO_Q(bufm,d);        
	    i++;
	}
    
    sprintf(bufm ,"\r\n%sEnter damage state or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    sprintf(bufm ,"\r\n%s-ve state == stunning condition.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);
}
int print_weaps(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    i = j->obj_flags.value[3];
    sprintf(bufm ,"\r\n%sWeapon Type:%s %s%s%s\r\n\r\n",cy,nrm,gn,weapon_type[i],nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*weapon_type[i] != '\n')
	{
	    if (*weapon_type[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,weapon_type[i],nrm,i+1,gn,weapon_type[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,weapon_type[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter Weapon Type or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}

struct list_index_type *print_spells_n(struct descriptor_data *d, int ii, int type)
{
    char bufm[MAX_STRING_LENGTH],*spl;
    struct obj_data *j;
    struct list_index_type *list;

    int i;
    switch(type){
    case -3:
      list = Forms;
      break;
    case -2:
      list = Techs;
      break;
    case -1:
      list = TechForms;
      break;
    case CRIG_GUILD:
      list = CrIg;
      break;
    case CRAQ_GUILD:
      list = Water;
      break;
    case CRCO_GUILD:
      list = CrCo;
      break;
    case RECO_GUILD:
      list = ReCo;
      break;
    case MUCO_GUILD:
      list = MuCo;
      break;
    case RETE_GUILD:
      list = ReTe;
      break;
    case PECO_GUILD:
      list = PeCo;
      break;
    case PEIM_GUILD:
      list = PeIm;
      break;
    case INIM_GUILD:
      list = InIm;
      break;
    case CRAN_GUILD:
      list = Animal;
      break;
    default:
      SEND_TO_Q("Error in spell selection - report this\r\n",d);
      print_obj(d);
      d->oedit_mode = MAIN_MODE;
      return 0;
      break;
    }
    
    j = d->obj_edit;
    bzero(bufm,MAX_STRING_LENGTH);
    for (i = 0; *(list[i].entry) != '\n';i++) 
	sprintf(bufm, "%s[%s%3d%s]  %s%-40s%s\r\n",bufm,cy,list[i].index,nrm
		,gn,list[i].entry,nrm);
    SEND_TO_Q(bufm,d);
    spl = rev_search_list(j->obj_flags.value[ii], list);
    sprintf(bufm ,"\r\n\r\n%s%s:%s%s %s.%s ",cy,
	    ((type == -1) ? "Technique & Form" :
	     ((type == -2) ? "Technique" :
	      ((type == -3) ? "Form" :"Spell"))),nrm,gn
	    ,(spl? spl: "None Set") ,nrm);
    SEND_TO_Q(bufm,d);
    sprintf(bufm ,"\r\n%sEnter %s # or Q to quit.%s ",cy,
	    ((type == -1) ? "Technique & Form" :
	     ((type == -2) ? "Technique" :
	      ((type == -3) ? "Form" :"Spell"))),nrm,gn
	    ,(spl? spl: "None Set") ,nrm);

    SEND_TO_Q(bufm,d);
    return list;
}

int print_affs(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;

    sprinttype(j->affected[d->iaff].location, apply_types, bufme);
    sprintf(bufm ,"\r\n%sAffect location:%s %s%s%s\r\n\r\n",cy,nrm,gn,bufme,nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*apply_types[i] != '\n')
	{
	    if (*apply_types[i+2] != '\n' && *apply_types[i+1] != '\n' ){
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,apply_types[i],nrm,i+1,gn,apply_types[i+1],nrm,i+2,gn,apply_types[i+2],nrm);
		i +=3;}
	    else if (*apply_types[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,apply_types[i],nrm,i+1,gn,apply_types[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,apply_types[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter location # or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
int print_oaff(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    
    sprintbit(j->obj_flags.bitvector, affected_bits, bufme);
    sprintf(bufm ,"\r\n%sAffected by Flags:%s %s%s%s\r\n",cy,nrm,gn,bufme,nrm);
    SEND_TO_Q(bufm,d);
    sprintf(bufm, "Affect duration: %s%d%s, No-Use duration: %s%d%s\r\n\r\n",
	    cy,j->obj_flags2.aff_dur, nrm,cy,j->obj_flags2.no_use_dur,nrm);
    i = 0;
    while (*affected_bits[i] != '\n')
	{
	    if (*affected_bits[i+2] != '\n' && *affected_bits[i+1] != '\n' ){
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,affected_bits[i],nrm,i+1,gn,affected_bits[i+1],nrm,i+2,gn,affected_bits[i+2],nrm);
		i +=3;}
	    else if (*affected_bits[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,affected_bits[i],nrm,i+1,gn,affected_bits[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,affected_bits[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter flag # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
void print_stats(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];    
    struct obj_data *j;
    
    j = d->obj_edit;
    sprintf(bufme,"%sStats:%s\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprinttype(GET_ITEM_TYPE(j), item_types, bufm);
    sprintf(bufme,"Item (T)ype: %s%s%s, ",gn,bufm,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme, "(W)eight: %s%d%s, (V)alue: %s%d%s, (C)ost/day: %s%d%s.\r\n",
       gn,j->obj_flags.weight,nrm,gn,j->obj_flags.cost,nrm,
       gn,j->obj_flags.cost_per_day,nrm);
    send_to_char(bufme, d->character);
}
int print_types(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    sprintf(bufme ,"\r\n%sWARNING changing item type will clear all values and flags!%s\r\n\r\n",brd,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme ,"%sPress Q to quit!%s\r\n\r\n",brd,nrm);
    SEND_TO_Q(bufme,d);
    sprinttype(GET_ITEM_TYPE(j), item_types, bufm);
    sprintf(bufme ,"\r\n%sType:%s %s%s%s\r\n\r\n",cy,nrm,gn,bufm,nrm);
    SEND_TO_Q(bufme,d);
    i = 0;
    while (*item_types[i] != '\n')
	{
	    if (*item_types[i+2] != '\n' && *item_types[i+1] != '\n'){
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,item_types[i],nrm,i+1,gn,item_types[i+1],nrm,i+2,gn,item_types[i+2],nrm);
		i +=3;}
	    else if (*item_types[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,item_types[i],nrm,i+1,gn,item_types[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,item_types[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter Item type # or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
int print_extras(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    
    sprintbit(j->obj_flags.extra_flags, extra_bits, bufme);
    sprintf(bufm ,"\r\n%sWorn:%s %s%s%s\r\n\r\n",cy,nrm,gn,bufme,nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*extra_bits[i] != '\n')
	{
	    if (*extra_bits[i+2] != '\n' && *extra_bits[i+1] != '\n'){
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,extra_bits[i],nrm,i+1,gn,extra_bits[i+1],nrm,i+2,gn,extra_bits[i+2],nrm);
		i +=3;}
	    else if (*extra_bits[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,extra_bits[i],nrm,i+1,gn,extra_bits[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,extra_bits[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter flag # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
int print_owear(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    struct obj_data *j;
    int i;

    j = d->obj_edit;
    
    sprintbit(j->obj_flags.wear_flags, wear_bits, bufme);
    sprintf(bufm ,"\r\n%sWorn:%s %s%s%s\r\n\r\n",cy,nrm,gn,bufme,nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*wear_bits[i] != '\n')
	{
	    if (*wear_bits[i+2] != '\n' && *wear_bits[i+1] != '\n'){
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,wear_bits[i],nrm,i+1,gn,wear_bits[i+1],nrm,i+2,gn,wear_bits[i+2],nrm);
		i +=3;}
	    else if (*wear_bits[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,wear_bits[i],nrm,i+1,gn,wear_bits[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,gn,wear_bits[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter flag # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}

void print_flags(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];    
    struct obj_data *j;
    
    j = d->obj_edit;
    
    sprintf(bufme,"%sFlags:%s\r\n",cy,nrm);
    send_to_char(bufme,d->character);

    sprintbit(j->obj_flags.wear_flags, wear_bits, bufm);
    sprintf(bufme,"(W) Worn: %s%s%s\r\n",gn,bufm,nrm);
    send_to_char(bufme,d->character);                
    sprintbit(j->obj_flags.bitvector, affected_bits, bufm);
    sprintf(bufme,"(A) Apply: %s%s%s\r\n",gn,bufm,nrm);
    send_to_char(bufme,d->character);                
    sprintbit(j->obj_flags.extra_flags, extra_bits, bufm);
    sprintf(bufme,"(E) Extra Flags: %s%s%s\r\n",gn,bufm,nrm);    
    send_to_char(bufme,d->character);            	


}
void print_aff(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];    
    struct obj_data *j;
    int i;
    
    j = d->obj_edit;
    
    send_to_char(cy, d->character);
    send_to_char("Affections:\r\n", d->character);

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
	if (j->affected[i].modifier) {
	    sprinttype(j->affected[i].location, apply_types, bufm);
	    sprintf(bufme, "%s(%d)%s %+d to %s ",nrm,i,gn,j->affected[i].modifier, bufm);
	    send_to_char(bufme, d->character);
	}
	else {
	    sprintf(bufme, "%s(%d)%s +0 to <none> ",nrm,i,gn);
	    send_to_char(bufme, d->character);}
    
    send_to_char(nrm,d->character);
    send_to_char("\r\nSelect number of affect to modify:",d->character);
}
void print_ostr(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    struct obj_data *j;
    
    j = d->obj_edit;
    
    sprintf(bufme,"%sDescriptions:%s\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(N) Name:%s %s%s%s,  ",cy,nrm,wh,j->name,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(S) Short Descr:%s %s%s%s\r\n",cy,nrm,wh,
	    ((j->short_description) ? j->short_description: "<None>"),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(D) Description:%s%s%s\r\n",cy,wh,
	    ((j->description) ? j->description: "<None>"),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(A) Action Description:%s %s%s%s\r\n",cy,nrm,wh,
	    ((j->action_description) ? j->action_description: "<None>"),nrm);
    send_to_char(bufme,d->character);            

}
void print_exd(struct descriptor_data *d)
{
    struct obj_data *j;
    char bufme[MAX_STRING_LENGTH];
    struct extra_descr_data *desc=0;
    int i =0;
    
    j = d->obj_edit;

    desc = j->ex_description;
    while(i < d->iedsc && desc->next){
	desc = desc->next;
	++i;}
    SEND_TO_Q(cy,d);
    SEND_TO_Q("Extra Desc:\r\n",d);
    SEND_TO_Q(nrm,d);
    sprintf(bufme,"(K) Keywords:%s %s%s\r\n",gn,((desc->keyword) ? desc->keyword : "None") ,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"(D) Description:%s \r\n%s%s\r\n",gn,((desc->description) ? desc->description : "None"),nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"(%sP%s) Purge\r\n",rd,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(Q) Quit.\r\n");
    SEND_TO_Q(bufme,d);
}

int print_ex(struct descriptor_data *d)
{
    struct obj_data *j;
    char bufme[MAX_STRING_LENGTH];
    struct extra_descr_data *desc=0;
    int i =0;
    
    j = d->obj_edit;
    
    send_to_char("Extra Desc:\r\n", d->character);
    send_to_char(yl,d->character);
    *bufme = '\0';
    if (j->ex_description)
	for (desc = j->ex_description; desc; desc = desc->next){
	    sprintf(bufme,"(%d) %s\r\n",i,desc->keyword);
	    i++;
	    send_to_char(bufme,d->character);
	}
    send_to_char("(C) Create new extra description.\r\n",d->character);
    send_to_char("(Q) Quit.\r\n",d->character);    
    send_to_char(nrm,d->character);
    return(i);
}

void print_obj(struct descriptor_data *d)

{
    int i;
    struct obj_data *j;
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    struct extra_descr_data *desc=0;
    

    j = d->obj_edit;
    
    sprintf(cy,"%s",CCBBLU(d->character,C_NRM));
    sprintf(wh,"%s",CCWHT(d->character,C_NRM));
    sprintf(ma,"%s",CCMAG(d->character,C_NRM));
    sprintf(nrm,"%s",CCNRM(d->character,C_NRM));
    sprintf(gn,"%s",CCGRN(d->character,C_NRM));
    sprintf(yl,"%s",CCYEL(d->character,C_NRM));
    sprintf(rd,"%s",CCRED(d->character,C_NRM));                
    sprintf(bgn,"%s",CCGRN(d->character,C_NRM));
    sprintf(byl,"%s",CCBYEL(d->character,C_NRM));
    sprintf(brd,"%s",CCBRED(d->character,C_NRM));
    
    sprintf(bufme,"%sObj Number: %d%s, %sCreation Points: %s%d%s\r\n",cy,d->virtual,nrm,byl,brd,assess_item(j),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sT%s)ext:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Name: %s%s%s  ",wh,j->name,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Short Descr: %s%s%s\r\n",wh,
	    ((j->short_description) ? j->short_description: "<None>"),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Description: %s%s%s\r\n",wh,
	    ((j->description) ? j->description: "<None>"),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Action Description: %s%s%s\r\n",wh,
	    ((j->action_description) ? j->action_description: "<None>"),nrm);
    send_to_char(bufme,d->character);            

    sprintf(bufme,"(%sF%s)lags:\r\n",cy,nrm);
    send_to_char(bufme,d->character);

    sprintbit(j->obj_flags.wear_flags, wear_bits, bufm);
    sprintf(bufme,"Worn: %s%s%s\r\n",gn,bufm,nrm);
    send_to_char(bufme,d->character);                
    sprintbit(j->obj_flags.bitvector, affected_bits, bufm);
    sprintf(bufme,"Apply: %s%s%s\r\n",gn,bufm,nrm);
    send_to_char(bufme,d->character);
    if (j->obj_flags.bitvector){
	sprintf(bufme,"Aff time: %s%d%s, No Use time: %s%d%s\r\n",gn,j->obj_flags2.aff_dur,nrm, rd,j->obj_flags2.no_use_dur,nrm);
	send_to_char(bufme,d->character);
    }
    sprintbit(j->obj_flags.extra_flags, extra_bits, bufm);
    sprintf(bufme,"Extra Flags: %s%s%s\r\n",gn,bufm,nrm);    
    send_to_char(bufme,d->character);            	

    sprintf(bufme,"(%sS%s)tats:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprinttype(GET_ITEM_TYPE(j), item_types, bufm);
    sprintf(bufme,"Item Type: %s%s%s, ",gn,bufm,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme, "Weight: %s%d%s, Value: %s%d%s, Cost/day: %s%d%s.\r\n",
       gn,j->obj_flags.weight,nrm,gn,j->obj_flags.cost,nrm,
       gn,j->obj_flags.cost_per_day,nrm);
    send_to_char(bufme, d->character);
    sprintf(bufme,"(%sI%s)llumination: %s%d%s\r\n",cy,nrm,gn,j->obj_flags2.light, nrm);
    send_to_char(bufme, d->character);
    sprintf(bufme,"(%sV%s)alues: %s%d %d %d %d %d%s\r\n",cy,nrm,gn,
	j->obj_flags.value[0],
	j->obj_flags.value[1],	    
	j->obj_flags.value[2],
	j->obj_flags.value[3],
	j->obj_flags.value[7],cy);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sD%s)am state: %s%s%s, si(%sZ%s)e: %s%s%s\r\n",
	    cy,nrm,gn,damage_state[j->obj_flags.value[4]],nrm,cy,nrm,gn,
	    sizes[j->obj_flags.value[5]],nrm);
    send_to_char(bufme, d->character);
    sprintf(bufme,"Item (%sL%s)imit: %s%d%s\r\n",cy,nrm,yl
	    ,j->obj_flags.value[6],nrm);
    send_to_char(bufme, d->character);
    sprintf(bufme,"(%sA%s)ffections:\r\n",cy,nrm);
    send_to_char(bufme, d->character);
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
	if (j->affected[i].modifier) {
	    sprinttype(j->affected[i].location, apply_types, bufm);
	    sprintf(bufme, "%s(%d)%s %+d to %s ",nrm,i,gn,
		    j->affected[i].modifier, bufm);
	    send_to_char(bufme, d->character);
	}
    	else {
	    sprintf(bufme, "%s(%d)%s  +0 to <none> ",nrm,i,gn);
	    send_to_char(bufme, d->character);}

    sprintf(bufme,"%s\r\n(%sE%s) Extra Desc:\r\n%s",nrm,cy,nrm,yl);
    send_to_char(bufme,d->character);
    if (j->ex_description)
	for (desc = j->ex_description; desc; desc = desc->next){
	    SEND_TO_Q(desc->keyword,d);
	    if (desc->next)
		SEND_TO_Q(", ",d);}
    else
	    SEND_TO_Q("<None Set>",d);

    sprintf(bufme,".%s\r\n",nrm);    
    send_to_char(bufme,d->character);
}

int calc_min_level_scpo(struct descriptor_data *d)
{
 int i,il,level;

 level = 0;
 for (i = 1;i<3;i++){
     il = d->obj_edit->obj_flags.value[i];
     if (spell_info[il].min_level > level)
	 level =  spell_info[il].min_level;
 }
 return(level);
}

int is_goditem(struct obj_data *j)
{
    int tohitdam = 0, points = 0, i;
    
    switch(GET_ITEM_TYPE(j)){
    case ITEM_WEAPON:
	if ((j->obj_flags.value[1]*j->obj_flags.value[2] 
	     > j->obj_flags.weight/7)
	    || (j->obj_flags.value[1] > j->obj_flags.weight/15))
	    return TRUE;
	for (i = 0; i < MAX_OBJ_AFFECT; i++)
	    if (j->affected[i].modifier) {
		switch (j->affected[i].location){
		case APPLY_HITROLL:
		case APPLY_HITROLL2:		  
		case APPLY_DAMROLL:
		case APPLY_DAMROLL2:		  
		    tohitdam += j->affected[i].modifier;
		}
	    }
	if (tohitdam > 7)
	    return(TRUE);
    case ITEM_CONTAINER:
	if ((j->obj_flags.value[0] - j->obj_flags.weight) > 4000) 
	    return TRUE;
	break;
    default:
	break;
    }
    points = assess_item(j);
    if (points > 125)
	return(TRUE);
    else
	{
            if(points <= 20)
                return (FALSE);
	    if(j->obj_flags.cost < points*100)
		return(TRUE);
	    if(j->obj_flags.cost_per_day < points*10) 
		return(TRUE);
	    return FALSE;
	}    
    
}
int assess_item(struct obj_data *j)
{
  int i, totpoints = 0;

  switch(GET_ITEM_TYPE(j)){
  case ITEM_WEAPON:
    totpoints = (j->obj_flags.value[1]*j->obj_flags.value[2] 
		 + j->obj_flags.value[1]) * 2;
    break;
  case ITEM_VIS:
    if (j->obj_flags.value[1] > 0)
    totpoints = 15*j->obj_flags.value[3];
    if (j->obj_flags.value[2] > 0)
    totpoints += 15*j->obj_flags.value[3];
    break;
  case ITEM_ARMOR:
    if(j->obj_flags.value[0] > 0)
      totpoints += j->obj_flags.value[0] * 2;
    else
      totpoints += j->obj_flags.value[0];
    if(j->obj_flags.value[1] > 0)
      totpoints += j->obj_flags.value[1] * 15;
    else
      totpoints += j->obj_flags.value[1] * 5;
    break;
  default:
    break;
  }
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      switch (j->affected[i].location){
      case APPLY_HITROLL:
      case APPLY_DAMROLL:
      case APPLY_HITROLL2:
      case APPLY_DAMROLL2:	      
	if(j->affected[i].modifier > 0)
	  totpoints += j->affected[i].modifier * 12;
	else
	  totpoints += j->affected[i].modifier * 4;
	break;
      case APPLY_STR:
      case APPLY_DEX:
      case APPLY_INT:
      case APPLY_WIS:
      case APPLY_CON:
      case APPLY_FOCUS:
	if(j->affected[i].modifier > 0)
	  totpoints += j->affected[i].modifier * 20; else
	    totpoints += j->affected[i].modifier * 10;
	break;
      case APPLY_PER:
      case APPLY_GUI:
      case APPLY_CHR:
      case APPLY_LUC:
	if(j->affected[i].modifier > 0)
	  totpoints += j->affected[i].modifier * 20; else
	    totpoints += j->affected[i].modifier * 2;
	break;
      case APPLY_DEVOTION:
      case APPLY_MANA:
      case APPLY_POWER:
	break;
      case APPLY_MOVE:
      case APPLY_HIT:
	if(j->affected[i].modifier > 0)
	  totpoints += j->affected[i].modifier * 3; else
	    totpoints += j->affected[i].modifier;
	break;
      case APPLY_AC:
      case APPLY_LEGS_AC:
      case APPLY_ARMS_AC:
      case APPLY_HEAD_AC:
	if(j->affected[i].modifier < 0)
	  totpoints -= j->affected[i].modifier * 2; else
	    totpoints -= j->affected[i].modifier;
	break;
      case APPLY_ALL_AC:
	if(j->affected[i].modifier < 0)
	  totpoints -= j->affected[i].modifier * 15; else
	    totpoints -= j->affected[i].modifier * 5;
	break;
      case APPLY_BODY_STOPPING:
      case APPLY_LEGS_STOPPING:
      case APPLY_ARMS_STOPPING:
      case APPLY_HEAD_STOPPING:
	if(j->affected[i].modifier > 0)
	  totpoints += j->affected[i].modifier * 15; else
	    totpoints += j->affected[i].modifier * 5;
	break;
      case APPLY_ALL_STOPPING:
	if(j->affected[i].modifier > 0)
	  totpoints += j->affected[i].modifier * 60; else
	    totpoints += j->affected[i].modifier * 20;
	break;
      case APPLY_SAVING_PARA:
      case APPLY_SAVING_ROD:
      case APPLY_SAVING_PETRI:
      case APPLY_SAVING_BREATH:
      case APPLY_SAVING_SPELL:
	if(j->affected[i].modifier < 0)
	  totpoints -= j->affected[i].modifier * 7; else
	    totpoints -= j->affected[i].modifier * 2;
	break;
      case APPLY_ALL_SAVE:
	if(j->affected[i].modifier < 0)
	  totpoints -= j->affected[i].modifier * 20; else
	    totpoints -= j->affected[i].modifier * 7;
	break;
      }
    }

  if(j->obj_flags.bitvector & AFF_BLIND)
    totpoints += -20;
  if(j->obj_flags.bitvector & AFF_INVISIBLE) 
    totpoints += 50;
  if(j->obj_flags.bitvector & AFF_DETECT_EVIL) 
    totpoints += 50;
  if(j->obj_flags.bitvector & AFF_DETECT_INVISIBLE) 
    totpoints += 40;
  if(j->obj_flags.bitvector & AFF_DETECT_MAGIC) 
    totpoints += 50;
  if(j->obj_flags.bitvector & AFF_SENSE_LIFE) 
    totpoints += 50;
  if(j->obj_flags.bitvector & AFF_SANCTUARY) 
    totpoints += 200;
  if(j->obj_flags.bitvector & AFF_CURSE)
    totpoints += -30;
  if(j->obj_flags.bitvector & AFF_PROTECT_EVIL) 
    totpoints += 50;
  if(j->obj_flags.bitvector & AFF_PARALYSIS) 
    totpoints += -25;
  if(j->obj_flags.bitvector & AFF_RESIST_HEAT) 
    totpoints += 60;
  if(j->obj_flags.bitvector & AFF_RESIST_COLD) 
    totpoints += 60;
  if(j->obj_flags.bitvector & AFF_SLEEP)
    totpoints += -25;
  if(j->obj_flags.bitvector & AFF_SNEAK)
    totpoints += 150;
  if(j->obj_flags.bitvector & AFF_HIDE)
    totpoints += 150;
  if(j->obj_flags.bitvector & AFF_INFRARED) 
    totpoints += 60;
  if(j->obj_flags.bitvector & AFF_FLY)
    totpoints += 75;
  if(j->obj_flags.bitvector & AFF_WATER_BREATH) 
    totpoints += 100;
  if(j->obj_flags.bitvector & AFF_FREE_ACTION) 
    totpoints += 100;

  if(j->obj_flags.extra_flags & ITEM_FRAGILE) 
    totpoints += -20;
  if ((j->obj_flags.extra_flags & ITEM_MAGIC)
      && GET_ITEM_TYPE(j) == ITEM_LIGHT)
    totpoints += (j->obj_flags.value[2]*3/4);
  if ((j->obj_flags.extra_flags & ITEM_MAGIC)
      && (j->obj_flags.extra_flags & ITEM_GLOW))
    totpoints +=(4*abs(j->obj_flags2.light));    
  if(j->obj_flags.extra_flags & ITEM_NORENT)
    totpoints += -30;

  return(totpoints);


}
