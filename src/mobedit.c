/* ************************************************************************
*   File: mobedit.c                                   Part of Archipelago *
*  Usage: online mobile editing commands. A Neil - June 1994              *
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
#include "spells.h"
#include "limits.h"
#include "screen.h"
#include "mobedit.h"

void mobedit(struct descriptor_data *d, char *str);
void print_mob(struct descriptor_data *d);
int print_maff(struct descriptor_data *d);
int print_mflags(struct descriptor_data *d);
void print_mstats(struct descriptor_data *d);
void print_acsvs(struct descriptor_data *d);
void print_ac(struct descriptor_data *d);
void print_st(struct descriptor_data *d);
void print_sv(struct descriptor_data *d);
void print_mstr(struct descriptor_data *d);
void print_matt(struct descriptor_data *d);
int print_mpos(struct descriptor_data *d);
int print_mdpos(struct descriptor_data *d);
void print_msex(struct descriptor_data *d);
void print_mobstats(struct descriptor_data *d);
void    copy_mob(struct char_data *from, struct char_data *to);
void    free_mob(struct char_data *mob);
void print_races(struct descriptor_data *d);
int compute_mob_exp(struct char_data *mob);
int compute_mobproto_exp(struct char_data *mob);

extern struct str_app_type str_app[];
extern struct list_index_type npc_races[];
extern struct char_data *mob_proto;
extern char	*affected_bits[];
extern char	*action_bits[];
extern char	*position_types[];
extern char	*npc_class_types[];
extern struct index_data *mob_index;
extern struct dual_list_type attack_hit_text[];
char    *rev_search_list(int num, struct list_index_type *list);

void mobedit(struct descriptor_data *d, char *str)
{
    
    struct char_data *mob_edit;
    int ja_flag,points, rmob;
    char *ctmp;
    char bufme[MAX_STRING_LENGTH];
    const char *save[] = {
	"Breath",
	"Paralysis",
	"Rods",
	"Petrification",
	"Spells"};
    const char *locs[] = {
	"Breath",
	"Paralysis",
	"Rods",
	"Petrification",
	"Spells"};


    mob_edit = d->mob_edit;
    switch (d->medit_mode){
    case MAIN_MODE:
	for (; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MAIN_MODE;
	    print_mob(d);	    
	    break;}
	*str = LOWER(*str);
	if (*str == 'q'){
	    if ((rmob = real_mobile(d->virtual)) < 0){
		SEND_TO_Q("Yikes mob doesn't exist.\r\nReport this!\r\n",d);
		free_mob(d->mob_edit);
		d->mob_edit = 0;
		d->virtual = 0;
		d->ia_flag = 0;
		d->n_att = 0;
		d->isave =0;
		d->iloc = 0;
		d->prompt_mode = 1;
		REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
                act("$n returns from creating part of the world.",TRUE, d->character,0, 0, TO_ROOM);
		return;}
	    d->mob_edit->nr = rmob;
	    copy_mob(d->mob_edit, mob_proto + rmob);
	    free_mob(d->mob_edit);		    
	    d->mob_edit = 0;
	    d->virtual = 0;	    
	    d->ia_flag = 0;
	    d->n_att = 0;
	    d->isave = 0;
	    d->iloc = 0;
	    d->prompt_mode = 1;
	    REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
            act("$n returns from creating part of the world.",TRUE, d->character,0, 0, TO_ROOM);
	    return;}
	else
	    switch (*str){
	    case 'm':
		d->medit_mode = MFLAG_EDIT;
		d->ia_flag = print_mflags(d);
		return;
	    case 'f':
		d->medit_mode = MAFF_EDIT;
		d->ia_flag = print_maff(d);
		return;
	    case 's':
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
	    case 'p':
		d->medit_mode = MSTAT_EDIT;
		print_mstats(d);
		return;
	    case 'a':
		d->medit_mode = MACS_EDIT;
		print_acsvs(d);
		return;		
	    case 't':
		if (mob_index[mob_edit->nr].number != 0)
		    {
			SEND_TO_Q("Cannot modify string info on active mobs.\r\n",d);
			SEND_TO_Q("Press return to continue.\r\n",d);
			d->medit_mode = MAIN_MODE;
			return;
		    }
		d->medit_mode = MSTRI_EDIT;
		print_mstr(d);
		return;		
	    default:
		SEND_TO_Q("Illegal Entry, try again\r\n",d);
		print_mob(d);
		SEND_TO_Q("\r\nEnter a letter a-w or  Q to Quit\r\n",d);
		return;
	    }
	break;
    case MMSTAT_EDIT:
	for( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MSV_EDIT;
	    print_sv(d);	    
	    break;}

	*str = LOWER(*str);
	if (*str != '1' && *str != '2' && *str != '3' && *str != '4' && *str != '5' && *str != 'q'){
	    SEND_TO_Q("No such option.",d);
	    break;}
	switch(*str){
	case '1':
	    d->medit_mode = MALIGN_EDIT;
	    sprintf(bufme,"%sAlignment: %s%s%d%s\r\n\r\n",cy,nrm,rd,GET_ALIGNMENT(mob_edit),nrm);
	    SEND_TO_Q(bufme,d);
	    SEND_TO_Q("Enter a new alignment >= -1000 <= 1000.\r\n",d);
	    return;
	case '2':
	    d->medit_mode = MNHD_EDIT;
	    sprintf(bufme,"%sHit Dice: %s%d%s\r\n",cy,rd,mob_edit->points.hit,nrm);
	    SEND_TO_Q(bufme,d);
	    SEND_TO_Q("Enter new Hit dice.\r\n",d);
	    return;
	case '3':
	    SEND_TO_Q("Enter new position.\r\n",d);
	    d->ia_flag = print_mpos(d);
	    d->medit_mode = MPOS_EDIT;
	    return;
	case '4':
	    d->medit_mode = MGOLD_EDIT;
	    sprintf(bufme,"%sHas Gold?:%s %s%s%s\r\n\r\n",cy,nrm,rd,(GET_GOLD(mob_edit) ? "yes": "no"),nrm);
	    SEND_TO_Q(bufme,d);
	    return;
	case '5':
	    d->medit_mode = MCLASS_EDIT;
	    print_races(d);
	    return;
	case 'q':
	    d->medit_mode = MAIN_MODE;
	    print_mob(d);
	    return;
	}
	    
    case MALIGN_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MLEVEL_EDIT;
	    sprintf(bufme,"Alignment Unchanged.\r\n\r\n%sLevel: %s%s%d%s\r\nEnter a new Mob Level:\r\n",cy,nrm,rd,GET_LEVEL(mob_edit),nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	points = atoi(str);
	    
	if (points > 1000 || points < -1000){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new Alignment, >= -1000, <= 1000:",d);
	    break;
	}
	    else{
		GET_ALIGNMENT(mob_edit) = points;
		d->medit_mode = MLEVEL_EDIT;
		sprintf(bufme,"%sLevel: %s%s%d%s\r\n\r\n",cy,nrm,rd,GET_LEVEL(mob_edit),nrm);
		SEND_TO_Q(bufme,d);
		SEND_TO_Q("Enter a new Mob Level:",d);
		return;
	    }
    case MLEVEL_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MTHACO_EDIT;
	    sprintf(bufme,"Level Unchanged.\r\n\r\n%sHitroll: %s%s%d%s\r\n\r\nEnter a new Mob Hitroll:\r\n",cy,nrm,rd,mob_edit->points.hitroll,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	points = atoi(str);

	if (points < 0 || points > 227){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new Mob Level, >= 0, <= 227y:",d);
	    break;
	}
	else{
	    GET_LEVEL(mob_edit) = points;
	    d->medit_mode = MTHACO_EDIT;
	    sprintf(bufme,"%sHitroll: %s%s%d%s\r\n\r\nEnter a new Mob Hitroll:\r\n",cy,nrm,rd,mob_edit->points.hitroll,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
    case MTHACO_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MHEIGHT_EDIT;
	    sprintf(bufme,"Hitroll Unchanged.\r\n\r\n%sHeight: %s%s%d%s\r\n\r\nEnter a new Mob Height:\r\n",cy,nrm,rd,mob_edit->player.height,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	points = atoi(str);	
	if (points < 0 || points > 20){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new Mob Hitroll, >= 0, <= 20:",d);
	    break;
	}
	else{
	    mob_edit->points.hitroll = points;
	    d->medit_mode = MHEIGHT_EDIT;
	    sprintf(bufme,"%sHeight: %s%s%d%s\r\n\r\nEnter a new Mob Height:\r\n",cy,nrm,rd,mob_edit->player.height,nrm);
	    SEND_TO_Q(bufme,d);
	    return;}
    case MHEIGHT_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MWEIGHT_EDIT;
	    sprintf(bufme,"Height Unchanged.\r\n\r\n%sWeight: %s%s%d%s\r\n\r\nEnter a new Mob Weight:\r\n",cy,nrm,rd,mob_edit->player.weight,nrm);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	points = atoi(str);		
	if (points < 0 || points > 1000){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new Mob height, >= 0, <= 1000:",d);
	    break;
	}
	else{
	    mob_edit->player.height = points;
	    d->medit_mode = MWEIGHT_EDIT;
	    sprintf(bufme,"%sWeight: %s%s%d%s\r\n\r\nEnter a new Mob Weight:\r\n",cy,nrm,rd,mob_edit->player.weight,nrm);
	    SEND_TO_Q(bufme,d);	    	    
	    return;}
    case MWEIGHT_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MMSTAT_EDIT;
	    sprintf(bufme,"Weight Unchanged.\r\n\r\n");
	    print_mobstats(d);
	    SEND_TO_Q(bufme,d);	    
	    return;}
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}	
	points = atoi(str);			
	if (points < 0 || points > 10000){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new Mob weight, >= 0, <= 10000:",d);
	    break;
	}
	else{
	    mob_edit->player.weight = atoi(str);
	    d->medit_mode = MMSTAT_EDIT;
	    print_mobstats(d);
	    return;}
    case MNHD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MSHD_EDIT;
	    sprintf(bufme,"Hit Dice unchanged.\r\n\r\n%sSize of Hit Dice: %s%d%s\r\n\r\nEnter a new Size of Hit Dice:\r\n",cy,rd,mob_edit->points.mana,nrm);
	    SEND_TO_Q(bufme,d);	    
	    return;}	    
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new number of hit dice, > 0, <= 100:",d);
	    break;
	}
	else if (atoi(str) <= 0 || atoi(str) > 100){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new number of hit dice, > 0, <= 100:",d);
	    break;
	}
	else{
	    mob_edit->points.hit = atoi(str);
	    d->medit_mode = MSHD_EDIT;
	    sprintf(bufme,"Size of Hit Dice: %s%s%d%s\r\n\r\nEnter a new Size of Hit Dice:\r\n",cy,rd,mob_edit->points.mana,nrm);
	    SEND_TO_Q(bufme,d);	    	    	    
	    return;}
    case MSHD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MHBON_EDIT;
	    sprintf(bufme,"Size of Hit Dice unchanged.\r\n\r\n%sHit Bonus: %s%d%s\r\n\r\nEnter a new Hit bonus:\r\n",cy,rd,mob_edit->points.move,nrm);
	    SEND_TO_Q(bufme,d);	    	    
	    return;}	    
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new size of hit dice, > 0, <= 100:",d);
	    break;
	}
	else if (atoi(str) <= 0 || atoi(str) > 100){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new size of hit dice, > 0, <= 100:",d);
	    break;
	}
	else{
	    mob_edit->points.mana = atoi(str);
	    d->medit_mode = MHBON_EDIT;
	    sprintf(bufme,"%sHit Bonus: %s%d%s\r\n\r\nEnter a new Hit Bonus:",cy,rd,mob_edit->points.move,nrm);
	    SEND_TO_Q(bufme,d);	    	    	    
	    sprintf(bufme,">= 0 <= %d:", 31999 - (mob_edit->points.hit*mob_edit->points.mana));
	    SEND_TO_Q(bufme,d);	    
	    return;}
    case MHBON_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MNDD_EDIT;
	    sprintf(bufme,"Size of Hit Bonus unchanged.\r\n\r\n%sDam Dice: %s%d%s\r\n\r\nEnter  new Dam Dice:\r\n",cy,rd,mob_edit->specials.damnodice,nrm);
	    SEND_TO_Q(bufme,d);	    	    
	    return;}	    
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	if (!is_number(str)){
	    sprintf(bufme,"\r\nIllegal Value\r\nEnter new hit bonus, > 0, <= %d:",31999 - (mob_edit->points.hit*mob_edit->points.mana));
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else if (atoi(str) <= 0 || atoi(str) > 31999 - (mob_edit->points.hit*mob_edit->points.mana)){
	    sprintf(bufme,"\r\nIllegal Value\r\nEnter new hit bonus, > 0, <= %d:",31999 - (mob_edit->points.hit*mob_edit->points.mana));
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    mob_edit->points.move = atoi(str);
	    d->medit_mode = MNDD_EDIT;
	    sprintf(bufme,"%sDam Dice: %s%d%s\r\n\r\nEnter new Dam Dice:\r\n",cy,rd,mob_edit->specials.damnodice,nrm);
	    SEND_TO_Q(bufme,d);	    	    	    
	    return;}
    case MNDD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MSDD_EDIT;
	    sprintf(bufme,"Number of Dam Dice Unchanged.\r\n\r\n%sDam Dice Size: %s%s%d%s\r\n\r\nEnter new Dam Dice Size:\r\n",cy,nrm,rd,mob_edit->specials.damsizedice,nrm);
	    SEND_TO_Q(bufme,d);	    	    	    	    
	    return;}	    
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new number of dam dice, > 0, <= 50:",d);
	    break;
	}
	else if (atoi(str) <= 0 || atoi(str) > 50){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new number of dam dice, > 0, <= 50:",d);
	    break;
	}
	else{
	    mob_edit->specials.damnodice = atoi(str);
	    d->medit_mode = MSDD_EDIT;
	    sprintf(bufme,"%sDam Dice Size: %s%s%d%s\r\n\r\nEnter new Dam Dice Size:\r\n",cy,nrm,rd,mob_edit->specials.damsizedice,nrm);
	    SEND_TO_Q(bufme,d);	    	    	    	    	    
	    return;}
    case MSDD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MDBON_EDIT;
	    sprintf(bufme,"Dam Dice Size Unchanged.\r\n\r\n%sDam Bonus: %s%d%s\r\n\r\nEnter new Dam Bonus:\r\n",cy,rd,mob_edit->points.damroll,nrm);
	    SEND_TO_Q(bufme,d);	    	    	    	    	    
	    return;}	    
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new size of dam dice, > 0, <= 50:",d);
	    break;
	}
	else if (atoi(str) < 0){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new size of dam dice, > 0, <= 50:",d);
	    break;
	}
	else{
	    mob_edit->specials.damsizedice = atoi(str);
	    d->medit_mode = MDBON_EDIT;
	    sprintf(bufme,"%sDam Bonus: %s%s%d%s\r\n\r\nEnter new Dam Bonus:\r\n\r\n",cy,nrm,rd,mob_edit->points.damroll,nrm);
	    SEND_TO_Q(bufme,d);	    	    	    	    	    	    
	    return;}
    case MDBON_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MATT_EDIT;
	    SEND_TO_Q("Dam bonus Unchanged.\r\nEnter a attack type:\r\n",d);
	    print_matt(d);
	    return;}	    
	if (*str == 'Q' || *str == 'q')
	    {
		d->medit_mode = MMSTAT_EDIT;
		print_mobstats(d);
		return;
		}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new dam bonus, > 0, <= 50:",d);
	    break;
	}
	else if (atoi(str) <= 0 || atoi(str) > 50){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new dam bonus, > 0, <= 50:",d);
	    break;
	}
	else{
	    mob_edit->points.damroll = atoi(str);
	    d->medit_mode = MATT_EDIT;
	    print_matt(d);
	    SEND_TO_Q("Enter a new attack type, Q to quit:",d);
	    return;}
    case MATT_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MATT_EDIT;
	    print_matt(d);
	    return;}
	if (is_number(str)){
	    d->n_att = atoi(str);
	    if (d->n_att >= TYPE_HIT && d->n_att <= TYPE_WHIP )
		{
		    mob_edit->specials.attack_type = d->n_att;
		}
	    print_matt(d);
	    break;
	}
	else if (*str == 'Q' || *str == 'q'){
	    d->medit_mode = MMSTAT_EDIT;
	    print_mobstats(d);
	    return;
	}
	else{
	    SEND_TO_Q("Illegal Attck.\r\n",d);
	    print_matt(d);
	    break;}
    case MPOS_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MPOS_EDIT;
	    d->ia_flag = print_mpos(d);
	    return;}
	if (is_number(str)){
	    d->n_att = atoi(str);
	    if (d->n_att >= 0 && d->n_att <= d->ia_flag )
		{
		    mob_edit->specials.position = d->n_att;
		}
	    d->ia_flag = print_mpos(d);
	    break;
	}
	else if (*str == 'Q' || *str == 'q'){
	    d->ia_flag = print_mdpos(d);
	    d->medit_mode = MDPOS_EDIT;
	    break;}
	else{
	    SEND_TO_Q("Illegal Position.\r\n",d);
	    print_mpos(d);
	    break;}
    case MDPOS_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MDPOS_EDIT;
	    d->ia_flag = print_mdpos(d);
	    return;}
	if (is_number(str)){
	    d->n_att = atoi(str);
	    if (d->n_att >= 0 && d->n_att <= d->ia_flag )
		{
		    mob_edit->specials.default_pos = d->n_att;
		}
	    d->ia_flag = print_mdpos(d);
	    break;
	}
	else if (*str == 'Q' || *str == 'q'){
	    print_msex(d);
	    d->medit_mode = MSEX_EDIT;
	    break;}
	else{
	    SEND_TO_Q("Illegal Position.\r\n",d);
	    ja_flag = print_mdpos(d);
	    break;}
	case MSEX_EDIT:
	    for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MSEX_EDIT;
	    print_msex(d);	    
	    break;}	    
	*str = LOWER(*str);
	if (*str != 'n' && *str != 'f' && *str != 'm' && *str != 'q'){
	    SEND_TO_Q("No such Sex.",d);
	    break;}
	switch (*str){
	case 'n':
	    mob_edit->player.sex = SEX_NEUTRAL;
	    d->medit_mode = MMSTAT_EDIT;
	    print_mobstats(d);	    
	    return;
	case 'm':
	    mob_edit->player.sex = SEX_MALE;
	    d->medit_mode = MMSTAT_EDIT;
	    print_mobstats(d);	    
	    return;
	case 'f':
	    mob_edit->player.sex = SEX_FEMALE;
	    d->medit_mode = MMSTAT_EDIT;
	    print_mobstats(d);	    
	    return;
	case 'q':
	    d->medit_mode = MMSTAT_EDIT;
	    print_mobstats(d);
	    return;	    
	}
    case MCLASS_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	print_races(d);
	break;
      }
      if (is_number(str)){
	ja_flag = atoi(str);
	if (rev_search_list(ja_flag, npc_races))
	  {
	    GET_RACE(d->mob_edit) = ja_flag;
	    print_races(d);
	    break;
	  }
	else {
	  SEND_TO_Q("Illegal class code.\r\n",d);
	  print_races(d);
	  break;
	}
      }
      else if (*str == 'Q' || *str == 'q'){
	d->medit_mode = MMSTAT_EDIT;
	print_mobstats(d);	    
	break;
      }
      else{
	SEND_TO_Q("Illegal class code.\r\n",d);
	print_races(d);
	break;
      }
    case MGOLD_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MFAME_EDIT;
	    sprintf(bufme,"Gold Unchanged.\r\n\r\n%sMob Fame:%s %s%d%s\r\n\r\n",cy,nrm,rd,GET_FAME(mob_edit),nrm);
	    SEND_TO_Q(bufme,d);
	    SEND_TO_Q("Enter Mob Fame:\r\n",d);
	    return;}
	if (*str == 'Q' || *str == 'q'){
	    d->medit_mode = MMSTAT_EDIT;
	    print_mobstats(d);
	    return;
	}
	if (!((*str == 'Y') || (*str == 'y')
	      || (*str == 'N') || (*str == 'n'))){
	  SEND_TO_Q("\r\nIllegal Value\r\nToggle Mob Gold:",d);
	  break;
	}
	else if (*str == 'Y' || *str == 'y')
	  GET_GOLD(mob_edit) = 1;
	else
	  GET_GOLD(mob_edit) = 0;
	d->medit_mode = MFAME_EDIT;
	sprintf(bufme,"%sMob Fame:%s %s%d%s\r\n\r\n",cy,nrm,rd,GET_FAME(mob_edit),nrm);
	SEND_TO_Q(bufme,d);
	SEND_TO_Q("Enter Mob Fame >= -1000 <= 1000:\r\n",d);		
	return;
    case MFAME_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	d->medit_mode = MAIN_MODE;
	SEND_TO_Q("Mob Fame Unchanged.\r\n",d);
	print_mob(d);
	return;
      }	    
      if (*str == 'Q' || *str == 'q'){
	d->medit_mode = MMSTAT_EDIT;
	print_mobstats(d);
	return;
      }
      points = atoi(str);
      if ((points > 1000) || (points < -1000)){
	SEND_TO_Q("\r\nIllegal Value\r\nEnter Mob Fame:",d);
	break;
      }
      else{
	GET_FAME(mob_edit) = points;
	d->medit_mode = MMSTAT_EDIT;
	print_mobstats(d);
	return;
      }
    case MSTRI_EDIT:
      for ( ; isspace(*str); str++)
	;
      if (!*str){
	d->medit_mode = MSTRI_EDIT;
	print_mstr(d);	    
	break;}	    
      *str = LOWER(*str);
      if (*str != 'n' && *str != 's' && *str != 'l' && *str != 'd' && *str != 'q'){
	SEND_TO_Q("No such string.",d);
	break;}
      switch(*str){
      case 'n':
	d->str = &(mob_edit->player.name);
	free(*d->str);
	*d->str = 0;
	d->max_str = 80;
	SEND_TO_Q("Enter a new name list, terminate with a @@.\r\n",d);
	return;
      case 's':
	d->str = &(mob_edit->player.short_descr);
	free(*d->str);
	*d->str = 0;
	d->max_str = 80;
	SEND_TO_Q("Enter a new short description, terminate with a @@.\r\n",d);
	return;
      case 'l':
	d->str = &(mob_edit->player.long_descr);
	free(*d->str);
	*d->str = 0;
	d->max_str = 80;
	SEND_TO_Q("Enter a new long decription. Remember the carriage return at the end\r\n, terminate with a @@.\r\n",d);
	return;
      case 'd':
	d->str = &(mob_edit->player.description);
	free(*d->str);
	*d->str = 0;
	d->max_str = 1440;
	SEND_TO_Q("Enter a new decription. Remember the carriage return at the end\r\n, terminate with a @@,\r\n",d);
	return;
      case 'q':
	d->medit_mode = MAIN_MODE;
	print_mob(d);
	return;	    
      }
    case MACS_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MACS_EDIT;
	    print_acsvs(d);	    
	    break;}	    
	*str = LOWER(*str);
	if (*str != 'a' && *str != 's' && *str != 't' && *str != 'q'){
	    SEND_TO_Q("No such option.",d);
	    break;}
	switch(*str){
	case 'a':
	    d->medit_mode = MAC_EDIT;
	    print_ac(d);
	    return;
	case 's':
	    d->medit_mode = MST_EDIT;
	    print_st(d);
	    return;
	case 't':
	    d->medit_mode = MSV_EDIT;
	    print_sv(d);
	    return;	    
	case 'q':
	    d->medit_mode = MAIN_MODE;
	    print_mob(d);
	    return;
	}
	break;
    case MSV_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MSV_EDIT;
	    print_sv(d);	    
	    break;}

	*str = LOWER(*str);
	if (*str != 'b' && *str != '0' && *str != 'p' && *str != 'r' && *str != 's' && *str != 't' && *str != 'q'){
	    SEND_TO_Q("No such option.",d);
	    break;}
	switch(*str){
	case '0':
	    d->medit_mode = MSAVEA_EDIT;
	    SEND_TO_Q("\r\nEnter new Saving Throws, > 0, <= 100",d);
	    return;	    
	case 'b':
	    d->medit_mode = MSAVE_EDIT;
	    SEND_TO_Q("\r\nEnter new Breath Saving Throw, > 0, <= 100",d);
	    d->isave = 3;	    
	    return;
	case 'p':
	    d->medit_mode = MSAVE_EDIT;
	    SEND_TO_Q("\r\nEnter new Paralysis Saving Throw, > 0, <= 100",d);
	    d->isave = 0;	    
	    return;
	case 'r':
	    d->medit_mode = MSAVE_EDIT;
	    SEND_TO_Q("\r\nEnter new Rods Saving Throw, > 0, <= 100",d);
	    d->isave = 1;	    
	    return;	    
	case 't':
	    d->medit_mode = MSAVE_EDIT;
	    SEND_TO_Q("\r\nEnter new Petrification Saving Throw, > 0, <= 100",d);
	    d->isave = 2;
	    return;
	case 's':
	    d->medit_mode = MSAVE_EDIT;
	    SEND_TO_Q("\r\nEnter new Spell Saving Throw, > 0, <= 100",d);
	    d->isave = 4;
	    return;
	case 'q':
	    d->medit_mode = MACS_EDIT;
	    print_acsvs(d);
	    return;
	}

    case MSAVEA_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSV_EDIT;
	    print_sv(d);
	    break;}
	points = atoi(str);
	if (points < 0 || points > 100){
	    sprintf(bufme,"\r\n%sIllegal Value\r\n%sEnter new Saving Throws, > 0, <= 100:%s",rd,cy,nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    mob_edit->specials2.apply_saving_throw[0] = points;
	    mob_edit->specials2.apply_saving_throw[1] = points;
	    mob_edit->specials2.apply_saving_throw[2] = points;
	    mob_edit->specials2.apply_saving_throw[3] = points;
	    mob_edit->specials2.apply_saving_throw[4] = points;	    
	    d->medit_mode = MSV_EDIT;
	    print_sv(d);
	    break;}
    case MSAVE_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSV_EDIT;
	    print_sv(d);
	    break;}
	points = atoi(str);
	if (points < 0 || points > 100){
	    sprintf(bufme,"\r\n%sIllegal Value\r\n%sEnter new %s Save, > 0, <= 100:%s",rd,cy,save[d->isave],nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    mob_edit->specials2.apply_saving_throw[d->isave] = points;
	    d->medit_mode = MSV_EDIT;
	    print_sv(d);
	    break;}
    case MST_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MST_EDIT;
	    print_st(d);	    
	    break;}	    
	*str = LOWER(*str);
	if (*str != 'b' && *str != 'h' && *str != 's' && *str != 'a' && *str != 'l' && *str != 'q'){
	    SEND_TO_Q("No such option.",d);
	    break;}
	switch(*str){
	case 's':
	    d->medit_mode = MSTOPA_EDIT;
	    SEND_TO_Q("\r\nEnter new stopping value, >= 0, <= 20",d);
	    return;	    
	case 'b':
	    d->medit_mode = MSTOP_EDIT;
	    SEND_TO_Q("\r\nEnter new Body stopping value, >= 0, <= 20",d);
	    d->iloc = 0;
	    return;
	case 'h':
	    d->medit_mode = MSTOP_EDIT;
	    SEND_TO_Q("\r\nEnter new Head stopping value, >= 0, <= 20",d);
	    d->iloc = 3;	    
	    return;
	case 'a':
	    d->medit_mode = MSTOP_EDIT;
	    SEND_TO_Q("\r\nEnter new Arms stopping value, >= 0, <= 20",d);
	    d->iloc = 2;	    
	    return;	    
	case 'l':
	    d->medit_mode = MSTOP_EDIT;
	    SEND_TO_Q("\r\nEnter new Legs stopping value, >= 0, <= 20",d);
	    d->iloc = 1;	    
	    return;
	case 'q':
	    d->medit_mode = MACS_EDIT;
	    print_acsvs(d);
	    return;
	}
	
    case MSTOPA_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MST_EDIT;
	    print_st(d);
	    break;}
	points = atoi(str);
        if (points < 0 || points > 20){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new stopping value, >= 0, <= 30:",d);
	    break;
	}
	else{
	    GET_LEGS_STOPPING(mob_edit) = points;
	    GET_ARMS_STOPPING(mob_edit) = points;
	    GET_HEAD_STOPPING(mob_edit) = points;
	    GET_BODY_STOPPING(mob_edit) = points;	    
	    d->medit_mode = MST_EDIT;
	    print_st(d);
	    break;}
    case MSTOP_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MST_EDIT;
	    print_st(d);
	    break;}
	points = atoi(str);
	if (points < 0 || points > 20){
	    sprintf(bufme,"\r\n%sIllegal Value\r\n%sEnter new %s stopping value, >= 0, <= 30:%s",rd,cy,locs[d->iloc],nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    mob_edit->points.stopping[d->iloc] = points;
	    d->medit_mode = MST_EDIT;
	    print_st(d);
	    break;}
    case MAC_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MAC_EDIT;
	    print_ac(d);	    
	    break;}	    
	*str = LOWER(*str);
	if (*str != 'b' && *str != 'h' && *str != 's' && *str != 'a' && *str != 'l' && *str != 'q'){
	    SEND_TO_Q("No such option.",d);
	    break;}
	switch(*str){
	case 's':
	    d->medit_mode = MARMORA_EDIT;
	    SEND_TO_Q("\r\nEnter new A/C values, >= -100, <= 100",d);
	    return;	    
	case 'b':
	    d->medit_mode = MARMOR_EDIT;
	    SEND_TO_Q("\r\nEnter new Body A/C value, >= -100, <= 100",d);
	    d->iloc = 0;
	    return;
	case 'h':
	    d->medit_mode = MARMOR_EDIT;
	    SEND_TO_Q("\r\nEnter new Head A/C value, >= -100, <= 100",d);
	    d->iloc = 3;	    
	    return;
	case 'a':
	    d->medit_mode = MARMOR_EDIT;
	    SEND_TO_Q("\r\nEnter new Arms A/C value, >= -100, <= 100",d);
	    d->iloc = 2;
	    return;	    
	case 'l':
	    d->medit_mode = MARMOR_EDIT;
	    SEND_TO_Q("\r\nEnter new Legs A/C value, >= -100, <= 100",d);
	    d->iloc = 1;
	    return;
	case 'q':
	    d->medit_mode = MACS_EDIT;
	    print_acsvs(d);
	    return;
	}
	
    case MARMORA_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MAC_EDIT;
	    print_ac(d);
	    break;}
	points = atoi(str);
	if (points <-100 || points > 100){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new A/C value, >= -100, <= 100:",d);
	    break;
	}
	else{
	    GET_LEGS_AC(mob_edit) = points;
	    GET_BODY_AC(mob_edit) = points;
	    GET_HEAD_AC(mob_edit) = points;
	    GET_ARMS_AC(mob_edit) = points;	    
	    d->medit_mode = MAC_EDIT;
	    print_ac(d);
	    break;}
    case MARMOR_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MAC_EDIT;
	    print_ac(d);
	    break;}
	points = atoi(str);
	if (points <-100 || points > 100){
	    sprintf(bufme,"\r\n%sIllegal Value\r\n%sEnter new %s a/c value, >= -100, <= 100:%s",rd,cy,locs[d->iloc],nrm);
	    SEND_TO_Q(bufme,d);
	    break;
	}
	else{
	    mob_edit->points.armor[d->iloc] = points;
	    d->medit_mode = MAC_EDIT;
	    print_ac(d);
	    break;}
    case MSTAT_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}	    
	*str = LOWER(*str);
	if (*str != 's' && *str != 'i' && *str != 'w' && *str != 'c' &&
	    *str != 'h' && *str != 'd' && *str != 'p' && *str != 'g' &&
	    *str != 'v' && *str != 'f' && *str != 'l' && *str != 'q'){
	    SEND_TO_Q("That's not a stat.\r\nEnter a Stat to change, Q to quit: ",d);
	    break;}
	switch(*str){
	case 's':
	    d->medit_mode = MSTR_EDIT;
	    SEND_TO_Q("\r\nEnter new strength value, >= 0, <=30:",d);
	    return;
	case 'i':
	    d->medit_mode = MINT_EDIT;
	    SEND_TO_Q("\r\nEnter new intelligence value, >= 0, <30:",d);
	    return;
	case 'w':
	    d->medit_mode = MWIS_EDIT;
	    SEND_TO_Q("\r\nEnter new wisdom value, >= 0, <=30:",d);
	    return;
	case 'd':
	    d->medit_mode = MDEX_EDIT;
	    SEND_TO_Q("\r\nEnter new dexterity value, >= 0, <= 30:",d);
	    return;
	case 'c':
	    d->medit_mode = MCON_EDIT;
	    SEND_TO_Q("\r\nEnter new constitution value, >= 0, <= 30:",d);
	    return;
	case 'h':
	    d->medit_mode = MCHR_EDIT;
	    SEND_TO_Q("\r\nEnter new charisma value, >= 0, <= 30:",d);
	    return;
	case 'p':
	    d->medit_mode = MPER_EDIT;
	    SEND_TO_Q("\r\nEnter new perception value, >= 0, <= 30:",d);
	    return;
	case 'g':
	    d->medit_mode = MGUI_EDIT;
	    SEND_TO_Q("\r\nEnter new guile value, >= 0, <= 30:",d);
	    return;
	case 'v':
	    d->medit_mode = MDEV_EDIT;
	    SEND_TO_Q("\r\nEnter new devotion value, >= 0, <= 30:",d);
	    return;
	case 'f':
	    d->medit_mode = MFOC_EDIT;
	    SEND_TO_Q("\r\nEnter new focus value, >= 0, <= 30:",d);
	    return;
	case 'l':
	    d->medit_mode = MLUC_EDIT;
	    SEND_TO_Q("\r\nEnter new luck value, >= 0, <= 30:",d);
	    return;
	case 'q':
	    d->medit_mode = MAIN_MODE;
	    print_mob(d);
	    SEND_TO_Q("\r\nEnter a letter a-w or  Q to Quit\r\n",d);
	    return;
	default:
	    print_mstats(d);
	    return;
	}
    case MSTR_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new strength value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new strength value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);
	    GET_STR(mob_edit) = points;
	    GET_RAW_STR(mob_edit) = points;
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MINT_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new intelligence value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new intelligence value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);
	    GET_RAW_INT(mob_edit) = points;
	    GET_INT(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MWIS_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new wisdom value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new wisdom value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    
	    GET_RAW_WIS(mob_edit) = points;
	    GET_WIS(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MDEX_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new dexterity value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new dexterity value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    
	    GET_RAW_DEX(mob_edit) = points;
	    GET_DEX(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MCON_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new constitution value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new constitution value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    
	    GET_RAW_CON(mob_edit) = points;
	    GET_CON(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MCHR_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new charisma value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new charisma value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    	    
	    GET_RAW_CHR(mob_edit) = points;
	    GET_CHR(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MPER_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new perception value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new perception value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    	    	    
	    GET_RAW_PER(mob_edit) = points;
	    GET_PER(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MGUI_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new guile value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new guile value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    	    	    	    
	    GET_RAW_GUI(mob_edit) = points;
	    GET_GUI(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MDEV_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new devotion value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new devotion value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    	    	    	    	    
	    GET_RAW_DEV(mob_edit) = points;
	    GET_DEV(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MFOC_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new focus value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new focus value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    
	    GET_RAW_FOC(mob_edit) = points;
	    GET_FOC(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
    case MLUC_EDIT:
	for ( ; isspace(*str); str++ )
	    ;
	if (!*str){
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	if (!is_number(str)){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new luck value, > 0, <= 30:",d);
	    break;
	}
	else if (atoi(str) <0 || atoi(str) > 30){
	    SEND_TO_Q("\r\nIllegal Value\r\nEnter new luck value, > 0, <= 30:",d);
	    break;
	}
	else{
	    points = atoi(str);	    	    	    
	    GET_RAW_LUC(mob_edit) = points;
	    GET_LUC(mob_edit) = points;	    
	    d->medit_mode = MSTAT_EDIT;
	    print_mstats(d);
	    break;}
	
    case MAFF_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MAFF_EDIT;
	    print_maff(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag )
		{
		    if (!IS_SET(mob_edit->specials.affected_by, 1<<ja_flag))
			SET_BIT(mob_edit->specials.affected_by, 1<<ja_flag);
		    else
			REMOVE_BIT(mob_edit->specials.affected_by, 1<<ja_flag);
		    d->ia_flag =  print_maff(d);
		    break;

		}
	    else{
		SEND_TO_Q("Illegal Flag.\r\n",d);
		d->ia_flag =  print_maff(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    print_mob(d);	
	    d->medit_mode = MAIN_MODE;
	    break;}
	else{
	    SEND_TO_Q("Illegal Flag.\r\n",d);
	    d->ia_flag =  print_maff(d);
	    break;}
    case MFLAG_EDIT:
	for ( ; isspace(*str); str++)
	    ;
	if (!*str){
	    d->medit_mode = MFLAG_EDIT;
	    d->ia_flag = print_mflags(d);
	    break;}
	if (is_number(str)){
	    ja_flag = atoi(str);
	    if (ja_flag >= 0 && ja_flag <= d->ia_flag
		&& ((1 << ja_flag) != MOB_ISNPC))
		{
		    if (!IS_SET(MOB_FLAGS(mob_edit), 1<<ja_flag))
			SET_BIT(MOB_FLAGS(mob_edit), 1<<ja_flag);
		    else
			REMOVE_BIT(MOB_FLAGS(mob_edit), 1<<ja_flag);
		    d->ia_flag =  print_mflags(d);
		    break;
		}
	    else{
		SEND_TO_Q("Illegal Flag.\r\n",d);
		d->ia_flag =  print_mflags(d);
		break;
	    }
	}
	else if (*str == 'Q' || *str == 'q'){
	    print_mob(d);	
	    d->medit_mode = MAIN_MODE;
	    break;}
	else{
	    SEND_TO_Q("Illegal Flag.\r\n",d);
	    d->ia_flag =  print_mflags(d);
	    break;}
    }
    

}
void print_matt(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    int i,attck;
    struct char_data *mob_edit;
    i = TYPE_HIT;

    mob_edit = d->mob_edit;
    
    if (mob_edit->specials.attack_type < TYPE_HIT)
	attck = TYPE_HIT;
    else
	attck = mob_edit->specials.attack_type;
    sprintf(bufme,"Attack Type: %s%s%s\r\n\r\n",rd,attack_hit_text[attck-TYPE_HIT].singular,nrm);
    SEND_TO_Q(bufme,d);
    while (i <= TYPE_WHIP)
	{
	    if (i+1 <= TYPE_KICK){
		sprintf(bufme,"[%3d]  %s%-20.20s%s      [%3d]  %s%-20.20s%s \r\n",i,rd,attack_hit_text[i-TYPE_HIT].singular,nrm,i+1,rd,attack_hit_text[i+1-TYPE_HIT].singular,nrm);
		i +=2;}
	    else{
		sprintf(bufme,"[%3d]  %s%-20.20s%s \r\n",i,rd,attack_hit_text[i-TYPE_HIT].singular,nrm);
		i +=1;}
	    SEND_TO_Q(bufme,d);        
	}
    
    sprintf(bufme ,"\r\n%sEnter attack # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufme,d);
 
}

int print_maff(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    int i;
    struct char_data *mob_edit;
    
    mob_edit = d->mob_edit;
    
    sprintbit(mob_edit->specials.affected_by, affected_bits, bufme);
    sprintf(bufm ,"\r\n%sAffected by Flags:%s %s%s%s\r\n\r\n",cy,nrm,gn,bufme,nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*affected_bits[i] != '\n')
	{
	    if (*affected_bits[i+2] != '\n'&& *affected_bits[i+1] != '\n'){
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
int print_mflags(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    int i;
    struct char_data *mob_edit;

    mob_edit = d->mob_edit;
    
    sprintbit(MOB_FLAGS(mob_edit), action_bits, bufme);
    sprintf(bufm ,"\r\n%sMob Flags: %s%s%s%s\r\n\r\n",cy,nrm,gn,bufme,nrm);
    SEND_TO_Q(bufm,d);
    i = 0;
    while (*action_bits[i] != '\n')
	{
	    if (*action_bits[i+1] != '\n'){
		sprintf(bufm,"[%2d]  %s%-20.20s%s      [%2d]  %s%-20.20s%s \r\n",i,gn,action_bits[i],nrm,i+1,gn,action_bits[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-20.20s%s \r\n",i,gn,action_bits[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter flag # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
void print_mstats(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    struct char_data *mob_edit;

    mob_edit = d->mob_edit;
    
    sprintf(bufme,"%sPlayer Stats:%s\r\n\r\n",cy,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"%s(S)tr: %2d, (I)nt: %2d, (W)is: %2d, (D)ex: %2d, (C)on: %2d, C(h)r %2d.\r\n",
	    yl,
	    GET_RAW_STR(mob_edit),
	    GET_RAW_INT(mob_edit),
	    GET_RAW_WIS(mob_edit),
	    GET_RAW_DEX(mob_edit),
	    GET_RAW_CON(mob_edit),
	    GET_RAW_CHR(mob_edit));
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"(P)er: %2d, (G)ui: %2d, De(v): %2d, (F)oc: %2d, (L)uc: %2d, (Q)uit.%s\r\n\r\n",
	    GET_RAW_PER(mob_edit),
	    GET_RAW_GUI(mob_edit),
	    GET_RAW_DEV(mob_edit),
	    GET_RAW_FOC(mob_edit),
	    GET_RAW_LUC(mob_edit),nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"%sEnter a Stat to change, Q to to Quit:%s ",cy,nrm);
    SEND_TO_Q(bufme,d);	
}
void print_acsvs(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    
    sprintf(bufme,"%s(A)rmour class, (S)topping and Saving (T)hrows:%s\r\n\r\n",cy,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme ,"%sEnter option to change or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufme,d);
}
void print_st(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    struct char_data *mob_edit;
    
    mob_edit = d->mob_edit;    
    sprintf(bufme,"%sStopping:%s\r\n\r\n",cy,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"%sStop:\r\n(%sS%s)et All, (%sB%s)ody: %s%3d%s (%sH%s)ead: %s%3d%s (%sA%s)rms: %s%3d%s (%sL%s)egs: %s%3d%s\r\n\r\n",
	    nrm,cy,nrm,cy,nrm,gn,
	    GET_BODY_STOPPING(mob_edit),
	    nrm,cy,nrm,gn,
	    GET_HEAD_STOPPING(mob_edit),
	    nrm,cy,nrm,gn,
	    GET_ARMS_STOPPING(mob_edit),
	    nrm,cy,nrm,gn,
	    GET_LEGS_STOPPING(mob_edit),nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme ,"%sEnter location to change or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufme,d);
}
void print_ac(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    struct char_data *mob_edit;

    mob_edit = d->mob_edit;    
    sprintf(bufme,"%sArmour class:%s\r\n\r\n",cy,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"%sA/C:\r\n(%sS%s)et All, (%sB%s)ody: %s%3d%s (%sH%s)ead: %s%3d%s (%sA%s)rms: %s%3d%s (%sL%s)egs: %s%3d%s\r\n\r\n",
	    nrm,cy,nrm,cy,nrm,gn,
	    GET_BODY_AC(mob_edit),
	    nrm,cy,nrm,gn,
	    GET_HEAD_AC(mob_edit),
	    nrm,cy,nrm,gn,
	    GET_ARMS_AC(mob_edit),
	    nrm,cy,nrm,gn,
	    GET_LEGS_AC(mob_edit),nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme ,"%sEnter location to change or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufme,d);
}
void print_sv(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    struct char_data *mob_edit;
    
    mob_edit = d->mob_edit;
    
    sprintf(bufme,"%sSaving Throws:%s\r\n\r\n",cy,nrm);
    SEND_TO_Q(bufme,d);
    sprintf(bufme,"%sSave:\r\n(%s0%s) Set all (%sP%s)ARA: %s%3d%s (%sR%s)OD: %s %3d%s PE(%sT%s)R:%s %3d%s (%sB%s)RTH: %s%3d%s (%sS%s)PELL: %s%3d%s\r\n\r\n",nrm,cy,nrm,cy,nrm,gn,
	    mob_edit->specials2.apply_saving_throw[0],
	    nrm,cy,nrm,gn,
	    mob_edit->specials2.apply_saving_throw[1],
	    nrm,cy,nrm,gn,
	    mob_edit->specials2.apply_saving_throw[2],
	    nrm,cy,nrm,gn,
	    mob_edit->specials2.apply_saving_throw[3],	
	    nrm,cy,nrm,gn,
	    mob_edit->specials2.apply_saving_throw[4],nrm);    
    SEND_TO_Q(bufme,d);
    sprintf(bufme ,"%sEnter location to change or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufme,d);
}

void print_mstr(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    struct char_data *mob_edit;
    
    mob_edit = d->mob_edit;
    
    sprintf(bufme,"%sText:%s\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(N)ame:%s %s%s%s,  ",cy,nrm,wh,mob_edit->player.name,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(S)hort Descr:%s %s%s%s\r\n",cy,nrm,wh,mob_edit->player.short_descr,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(L)ong Descr:%s %s%s%s",cy,nrm,wh,mob_edit->player.long_descr,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%s(D)escription:%s\r\n%s%s%s",cy,nrm,wh,mob_edit->player.description,nrm);
    send_to_char(bufme,d->character);
    SEND_TO_Q("\r\nChoose a string to change, Q to quit:",d);

}
int print_mpos(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    int i;
    struct char_data *mob_edit;
    mob_edit = d->mob_edit;

    sprinttype(mob_edit->specials.position, position_types,bufm);
    sprintf(bufme,"Position: %s%s%s\r\n\r\n",rd,bufm,nrm);
    SEND_TO_Q(bufme,d);	

    i = 0;
    while (*position_types[i] != '\n')
	{
	   if (*position_types[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,rd,position_types[i],nrm,i+1,rd,position_types[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,rd,position_types[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter position type # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}
int print_mdpos(struct descriptor_data *d)
{
    char bufme[MAX_STRING_LENGTH];
    char bufm[MAX_STRING_LENGTH];
    int i;
    struct char_data *mob_edit;
    mob_edit = d->mob_edit;
    
    sprinttype(mob_edit->specials.default_pos, position_types,bufm);
    sprintf(bufme,"Default Position: %s%s%s\r\n\r\n",rd,bufm,nrm);
    SEND_TO_Q(bufme,d);	

    i = 0;
    while (*position_types[i] != '\n')
	{
	   if (*position_types[i+1] != '\n') {
		sprintf(bufm,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,rd,position_types[i],nrm,i+1,rd,position_types[i+1],nrm);
		i +=2;}
	    else{
		sprintf(bufm,"[%2d]  %s%-15.15s%s\r\n",i,rd,position_types[i],nrm);
		i +=1;}
	    SEND_TO_Q(bufm,d);        
	}
    
    sprintf(bufm ,"\r\n%sEnter position type # to toggle or Q to quit.%s ",cy,nrm);
    SEND_TO_Q(bufm,d);
    return(i-1);

}

void print_msex(struct descriptor_data *d)
{
    char bufm[MAX_STRING_LENGTH];
    struct char_data *mob_edit;
    
    mob_edit = d->mob_edit;
    
    switch(mob_edit->player.sex)
	{
	case SEX_NEUTRAL:
	    send_to_char("Sex: NEUTER\r\n\r\n",d->character);
	    break;
	case SEX_MALE:
	    send_to_char("Sex: MALE\r\n\r\n",d->character);
	    break;	    
	case SEX_FEMALE:
	    send_to_char("Sex: FEMALE\r\n\r\n",d->character);
	    break;
	default:
	    send_to_char("Sex: ILLEGAL\r\n\r\n",d->character);
	}
    sprintf(bufm,"Sex: %s(N)euter, (F)emale, (M)ale%s\r\n\r\n",rd,nrm);
    SEND_TO_Q(bufm,d);

    SEND_TO_Q("Select a sex, Q to Quit.",d);
}

void print_mobstats(struct descriptor_data *d)
{
    int attck;
    char bufme[MAX_STRING_LENGTH], *tmp;
    char bufm[MAX_STRING_LENGTH];
    struct char_data *mob_edit;
    
    mob_edit = d->mob_edit;        
	
    sprintf(bufme,"(%sS)tats:%s\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%s1%s) Alignment:%s %d %sLevel:%s %d%s Hitroll:%s %d%s ",cy,nrm,gn,GET_ALIGNMENT(mob_edit),nrm,gn,GET_LEVEL(mob_edit),nrm,gn,mob_edit->points.hitroll,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Height:%s %d%s Weight:%s %d%s\r\n",gn,mob_edit->player.height,nrm,gn,mob_edit->player.weight,nrm);    
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%s2%s) Hit dice:%s %dd%d+%d%s ",cy,nrm,gn,mob_edit->points.hit,mob_edit->points.mana,mob_edit->points.move,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Dam dice: %s%dd%d+%d%s ",gn,mob_edit->specials.damnodice,mob_edit->specials.damsizedice,mob_edit->points.damroll,nrm);
    send_to_char(bufme,d->character);
    if (mob_edit->specials.attack_type >= 300)
	attck = mob_edit->specials.attack_type -300;
    else
	attck = mob_edit->specials.attack_type;
    sprintf(bufme,"Attack type: %s%s%s\r\n",gn,attack_hit_text[attck].singular,nrm);
    send_to_char(bufme,d->character);
    sprinttype(mob_edit->specials.position, position_types,bufm);
    sprintf(bufme,"(%s3%s) Position: %s%s%s ",cy,nrm,gn,bufm,nrm);
    send_to_char(bufme,d->character);	
    sprinttype(mob_edit->specials.default_pos, position_types,bufm);
    sprintf(bufme,"Def Position: %s%s%s ",gn,bufm,nrm);
    send_to_char(bufme,d->character);
    switch(mob_edit->player.sex)
	{
	case SEX_NEUTRAL:
	    sprintf(bufme,"%sNEUTRAL%s\r\n",gn,nrm);
	    break;
	case SEX_MALE:
	    sprintf(bufme,"%sMALE%s\r\n",gn,nrm);
	    break;	    
	case SEX_FEMALE:
	    sprintf(bufme,"%sFEMALE%s\r\n",gn,nrm);
	    break;
	default:
	    sprintf(bufme,"%sILLEGAL%s\r\n",gn,nrm);
	}
    sprintf(bufm,"Sex: %s",bufme);
    SEND_TO_Q(bufm,d);
    sprintf(bufme,"(%s4%s) Gold: %s%s%s Fame: %s%d%s.\r\n",cy,nrm,gn,(GET_GOLD(mob_edit) ? "yes" :"no"),nrm,gn,GET_FAME(mob_edit),nrm);
    send_to_char(bufme,d->character);
    tmp = rev_search_list(GET_RACE(mob_edit), npc_races);
    sprintf(bufme,"(%s5%s) Class: %s%s%s.\r\n",cy,nrm,gn,
	    ((tmp) ? tmp : "Unknown Class"),nrm);
    send_to_char(bufme,d->character);    
    sprintf(bufme,"(%sQ%s) uit.\r\n",cy,nrm);
    send_to_char(bufme,d->character);
	
}

void print_mob(struct descriptor_data *d)

{
    int attck;
    char bufme[MAX_STRING_LENGTH], *ctemp;
    char bufm[MAX_STRING_LENGTH];
    struct char_data *mob_edit;
    mob_edit = d->mob_edit;
    
    sprintf(cy,"%s",CCCYN(d->character,C_NRM));
    sprintf(wh,"%s",CCWHT(d->character,C_NRM));
    sprintf(ma,"%s",CCMAG(d->character,C_NRM));
    sprintf(nrm,"%s",CCNRM(d->character,C_NRM));
    sprintf(gn,"%s",CCGRN(d->character,C_NRM));
    sprintf(yl,"%s",CCBYEL(d->character,C_NRM));
    sprintf(rd,"%s",CCRED(d->character,C_NRM));                
    
    sprintf(bufme,"%sMob Number: %s%d%s Mob Creation Points: %s%d%s\r\n",
	    cy,gn,d->virtual,cy,yl,compute_mobproto_exp(mob_edit),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sT%s)ext:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Name: %s%s%s,  ",wh,mob_edit->player.name,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Short Descr: %s%s%s\r\n",wh,mob_edit->player.short_descr,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Long Descr: %s%s%s",wh,mob_edit->player.long_descr,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Description:\r\n%s%s%s",wh,mob_edit->player.description,nrm);
    send_to_char(bufme,d->character);        

    sprintbit(MOB_FLAGS(mob_edit), action_bits, bufme);
    sprintf(bufm ,"(%sM%s)ob Flags: %s%s%s\r\n",cy,nrm,gn, bufme,nrm);
    send_to_char(bufm,d->character);        

    sprintbit(mob_edit->specials.affected_by, affected_bits, bufme);
    sprintf(bufm ,"Affect (%sF%s)lags: %s%s%s\r\n",cy,nrm,gn, bufme,nrm);
    send_to_char(bufm,d->character);        
    sprintf(bufme,"(%sP%s)layer Stats:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%sStr:%s %2d,%s Int:%s %2d,%s Wis:%s %2d,%s Dex:%s %2d,%s Con:%s %2d,%s Chr:%s %2d\r\n"
	    ,nrm,gn,
	    GET_RAW_STR(mob_edit),
	    nrm,gn,
	    GET_RAW_INT(mob_edit),
	    nrm,gn,
	    GET_RAW_WIS(mob_edit),
	    nrm,gn,
	    GET_RAW_DEX(mob_edit),
	    nrm,gn,
	    GET_RAW_CON(mob_edit),
	    nrm,gn,
	    GET_RAW_CHR(mob_edit));
    send_to_char(bufme,d->character);
    sprintf(bufme,"%sPer:%s %2d,%s Gui:%s %2d,%s Dev:%s %2d,%s Foc:%s %2d,%s Luc:%s %2d%s\r\n",
	    nrm,gn,
	    GET_RAW_PER(mob_edit),
	    nrm,gn,
	    GET_RAW_GUI(mob_edit),
	    nrm,gn,
	    GET_RAW_DEV(mob_edit),
	    nrm,gn,
	    GET_RAW_FOC(mob_edit),
	    nrm,gn,
	    GET_RAW_LUC(mob_edit),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sA%s)rmour class, Stopping and Saving throws:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%sa/c:   Body:%s %3d%s Head:%s %3d%s Arms: %s%3d%s Legs:%s %3d%s\r\n",
	    nrm,gn,
	    GET_BODY_AC(mob_edit),
	    nrm,gn,
	    GET_HEAD_AC(mob_edit),
	    nrm,gn,
	    GET_ARMS_AC(mob_edit),
	    nrm,gn,
	    GET_LEGS_AC(mob_edit),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%sstop:  Body:%s %3d%s Head:%s %3d%s Arms:%s %3d%s Legs:%s %3d%s\r\n",
	    nrm,gn,
	    GET_BODY_STOPPING(mob_edit),
	    nrm,gn,
	    GET_HEAD_STOPPING(mob_edit),
	    nrm,gn,
	    GET_ARMS_STOPPING(mob_edit),
	    nrm,gn,
	    GET_LEGS_STOPPING(mob_edit),nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%sSave:  PARA:%s %3d%s ROD:%s  %3d%s PETR:%s %3d%s BRTH:%s %3d%s SPELL:%s %3d%s\r\n",
	    nrm,gn,
	    mob_edit->specials2.apply_saving_throw[0],
	    nrm,gn,
	    mob_edit->specials2.apply_saving_throw[1],
	    nrm,gn,
	    mob_edit->specials2.apply_saving_throw[2],
	    nrm,gn,
	    mob_edit->specials2.apply_saving_throw[3],	
	    nrm,gn,
	    mob_edit->specials2.apply_saving_throw[4],nrm);    
    send_to_char(bufme,d->character);
    sprintf(bufme,"(%sS%s)tats:\r\n",cy,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"%sAlignment:%s %d %sLevel:%s %d%s Hitroll:%s %d%s ",nrm,gn,GET_ALIGNMENT(mob_edit),nrm,gn,GET_LEVEL(mob_edit),nrm,gn,mob_edit->points.hitroll,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Height:%s %d%s Weight:%s %d%s\r\n",gn,mob_edit->player.height,nrm,gn,mob_edit->player.weight,nrm);    
    send_to_char(bufme,d->character);
    sprintf(bufme,"Hit dice:%s %dd%d+%d%s ",gn,mob_edit->points.hit,mob_edit->points.mana,mob_edit->points.move,nrm);
    send_to_char(bufme,d->character);
    sprintf(bufme,"Dam dice: %s%dd%d+%d%s ",gn,mob_edit->specials.damnodice,mob_edit->specials.damsizedice,mob_edit->points.damroll,nrm);
    send_to_char(bufme,d->character);
    if (mob_edit->specials.attack_type >= 300)
	attck = mob_edit->specials.attack_type -300;
    else
	attck = mob_edit->specials.attack_type;
    sprintf(bufme,"Attack type: %s%s%s\r\n",gn,attack_hit_text[attck].singular,nrm);
    send_to_char(bufme,d->character);
    sprinttype(mob_edit->specials.position, position_types,bufm);
    sprintf(bufme,"Position: %s%s%s ",gn,bufm,nrm);
    send_to_char(bufme,d->character);	
    sprinttype(mob_edit->specials.default_pos, position_types,bufm);
    sprintf(bufme,"Def Position: %s%s%s ",gn,bufm,nrm);
    send_to_char(bufme,d->character);
    switch(mob_edit->player.sex)
	{
	case SEX_NEUTRAL:
	    sprintf(bufme,"%sNEUTRAL%s\r\n",gn,nrm);
	    break;
	case SEX_MALE:
	    sprintf(bufme,"%sMALE%s\r\n",gn,nrm);
	    break;	    
	case SEX_FEMALE:
	    sprintf(bufme,"%sFEMALE%s\r\n",gn,nrm);
	    break;
	default:
	    sprintf(bufme,"%sILLEGAL%s\r\n",gn,nrm);
	}
    sprintf(bufm,"Sex: %s",bufme);
    SEND_TO_Q(bufm,d);
    sprintf(bufme,"Gold: %s%s%s Fame: %s%d%s.\r\n",gn,
	    (GET_GOLD(mob_edit) ? "yes":"no"),nrm,gn,GET_FAME(mob_edit),nrm);
    send_to_char(bufme,d->character);
    ctemp = rev_search_list(GET_RACE(mob_edit), npc_races);
    sprintf(bufme,"Class: %s%s%s.\r\n",gn,
	    (ctemp ? ctemp: "Unknown Class"),nrm);
       send_to_char(bufme,d->character);	    
}

void print_races(struct descriptor_data *d)
{
  int i;
  char *race, bufm[MAX_STRING_LENGTH];

  bzero(bufm,MAX_STRING_LENGTH);
    
  race = rev_search_list(GET_RACE(d->mob_edit),npc_races);
  sprintf(bufm, "%sClass:%s %s%s%s\r\n\r\n",cy,nrm,rd,
	  (race ? race : "Unknown Class"),
	  nrm);	    
  SEND_TO_Q(bufm,d);
  for (i=0;*npc_races[i].entry != '\n';i++)
    sprintf(bufm, "%s[%s%3d%s]   %s%s%s\r\n", bufm, cy, npc_races[i].index,nrm
	    ,gn,npc_races[i].entry,nrm);
  SEND_TO_Q(bufm,d);  
  sprintf(bufm,"\r\nEnter New class code for the mobile 'Q' to quit");
  SEND_TO_Q(bufm,d);  
  return;
}

int compute_mobproto_exp(struct char_data *mob)
{
  
  int points=0, ac=0, stop=0, abils=0, maxhit=0;
  
  maxhit = (mob->points.hit*mob->points.mana + mob->points.move);
  points += (mob->points.hit*mob->points.mana + mob->points.move);
  points += str_app[STRENGTH_APPLY_INDEX(mob)].tohit;
  points += GET_HITROLL(mob);
  points += ((mob->specials.damnodice*mob->specials.damsizedice
	     + mob->specials.damnodice)/2);
  points += mob->points.damroll;
  points += str_app[STRENGTH_APPLY_INDEX(mob)].todam;  
  ac += (100 - GET_BODY_AC(mob))*4;
  ac += (100 - GET_LEGS_AC(mob))*3;
  ac += (100 - GET_ARMS_AC(mob))*2;
  ac += (100 - GET_HEAD_AC(mob));
  ac /= 10;
  stop += GET_BODY_STOPPING(mob)*4;
  stop += GET_ARMS_STOPPING(mob)*2;  
  stop += GET_LEGS_STOPPING(mob)*3;
  stop += GET_HEAD_STOPPING(mob);
  stop /= 10;
  abils += GET_STR(mob)*2;
  abils += GET_INT(mob)*2;  
  abils += GET_DEX(mob)*2;
  abils += GET_WIS(mob);
  abils += GET_CON(mob);
  abils += GET_PER(mob);
  abils += GET_FOC(mob);
  abils += GET_CHR(mob);  
  abils += GET_GUI(mob);
  abils /= 12;
  points += abils;
  points += stop;
  points += ac;
  points *= MAX(8, MIN(100,maxhit/20));
  return(points);
}

int compute_mob_exp(struct char_data *mob)
{
  
  int points=0, ac=0, stop=0, abils=0, maxhit;
  
  maxhit = GET_MAX_HIT(mob);
  points += GET_MAX_HIT(mob);
  points += str_app[STRENGTH_APPLY_INDEX(mob)].tohit;
  points += 3*GET_HITROLL(mob);
  points += 10*((mob->specials.damnodice*mob->specials.damsizedice
	     + mob->specials.damnodice)/2);
  points += 10*mob->points.damroll;
  points += 10*str_app[STRENGTH_APPLY_INDEX(mob)].todam;  

  ac += (100 - GET_BODY_AC(mob))*4;
  ac += (100 - GET_LEGS_AC(mob))*3;
  ac += (100 - GET_ARMS_AC(mob))*2;
  ac += (100 - GET_HEAD_AC(mob));
  ac /= 10;
  stop += GET_BODY_STOPPING(mob)*4;
  stop += GET_ARMS_STOPPING(mob)*2;  
  stop += GET_LEGS_STOPPING(mob)*3;
  stop += GET_HEAD_STOPPING(mob);
  stop /= 10;
  abils += GET_STR(mob)*2;
  abils += GET_INT(mob)*2;  
  abils += GET_DEX(mob)*2;
  abils += GET_WIS(mob);
  abils += GET_CON(mob);
  abils += GET_PER(mob);
  abils += GET_FOC(mob);
  abils += GET_CHR(mob);  
  abils += GET_GUI(mob);
  abils /= 12;
  points += 4*abils;
  points += 4*stop;
  points += 4*ac;
  points *= MAX(3, MIN(45,maxhit/30));
  return(points);
}














