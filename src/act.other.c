/* ************************************************************************
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
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
#include <time.h>
#include <errno.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "limits.h"


/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern char	*class_abbrevs[];
extern  struct list_index_type *guild_list[];
 extern struct list_index_type CrIg[];
 extern struct list_index_type Water[];
 extern struct list_index_type CrCo[];
 extern struct list_index_type CrIm[];
 extern struct list_index_type ReCo[];
 extern struct list_index_type PeIm[];
 extern struct list_index_type InIm[];
 extern struct list_index_type InVm[];
 extern struct list_index_type MuCo[];
 extern struct list_index_type MuTe[];
 extern struct list_index_type ReTe[];
 extern struct list_index_type MuAu[];
 extern struct list_index_type Animal[];
 extern struct list_index_type PeCo[];
 extern struct list_index_type am_skills[];
 extern struct list_index_type Cleric_skills[];
 extern struct list_index_type Warrior_skills[];
 extern struct list_index_type Thief_skills[];
 extern struct list_index_type Ranger_skills[];
 extern struct list_index_type Horse_skills[];
/* extern procedures */
SPECIAL(shop_keeper);
void    drop_excess_gold(struct char_data *ch, int amount);
struct obj_data 	*die(struct char_data *ch);
int     save_zone(int zone);
int	save_mobiles(int zone);
int	save_objects(int zone);
int	save_rooms(int zone);
char    *report_cost(int gold);
int     steal_exp(int raw_exp, int value);
int     report_money_weight(int amount);
int     calc_difficulty(struct char_data *ch, int number);
int     calc_prof_difficulty(struct char_data *ch, int number);
int     calc_max_ability(struct char_data *ch, int number);
void    parse_text(struct char_data *c,struct char_data *vict, int mode,char *text);
int     report_highest_value_for_weight(int weight);
void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
int	get_alias_filename(char *orig_name, char *filename);
void bless_char(struct char_data *ch, struct char_data *subj, int total, int *level);
void curse_char(struct char_data *ch, struct char_data *subj, int total, int *level);
void bless_obj(struct char_data *ch, struct obj_data *obj, int total, int *level);
void curse_obj(struct char_data *ch, struct obj_data *obj, int total, int *level);
bool is_afflicted(struct char_data *ch);
ACMD(do_quit)
{
    struct obj_data *money;

    if (IS_NPC(ch) || !ch->desc)
	return;
    
    if (subcmd != SCMD_QUIT) {
	send_to_char("You have to type quit - no less, to quit!\r\n", ch);
	return;
    }
    
    if (ch->specials.fighting) {
	send_to_char("No way!  You're fighting for your life!\r\n", ch);
	return;
    }
    
    if (GET_POS(ch) < POSITION_STUNNED) {
	send_to_char("You die before your time...\r\n", ch);
	die(ch);
	return;
    }
    
    act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
    if (!GET_INVIS_LEV(ch))
	act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "%s has quit the game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LEVEL_GOD, GET_INVIS_LEV(ch)), TRUE);
    /* drop all your gold */
    if (GET_GOLD(ch) > 0){
	money = create_money(GET_GOLD(ch));
	change_gold(ch, -GET_GOLD(ch));
	obj_to_char(money,ch,1);
    }
    
    extract_char(ch, TRUE); /* Char is saved in extract char */
}

ACMD(do_untether) {
  struct char_data *victim;
  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("No one here by that name.\r\n", ch);
    return;
  }
  if (!IS_MOB(victim) ||
      (IS_MOB(victim) && !MOB_FLAGGED(victim, MOB_TETHERED))){
    act("$N is not tethered.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }
  REMOVE_BIT(MOB_FLAGS(victim), MOB_TETHERED);
  act("$n untethers $N.",FALSE,ch,0,victim,TO_NOTVICT);
  act("$n untethers you.",FALSE,ch,0,victim,TO_VICT);
  act("You untether $N.",FALSE,ch,0,victim,TO_CHAR);  
}


ACMD(do_tether)
{
  struct char_data *victim, *k;
  struct follow_type *f;
  bool found= FALSE;
  
  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("No one here by that name.\r\n", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("That would be a silly thing to do.\r\n", ch);
    return;
  }
  else {
    for (f = ch->followers; f; f = f->next) {
      if (f->follower == victim) {
	found = TRUE;
	break;
      }
    }
  }
  if (found) {
    if (!IS_MOB(victim)) {
      send_to_char("You can't tether people.\r\n",ch);
      return;
    }
    if (MOB_FLAGGED(victim, MOB_DOCILE) || IS_AFFECTED(victim, AFF_CHARM)) {
      act("$n tethers $N.",FALSE,ch,0,victim,TO_NOTVICT);
      act("$n tethers you.",FALSE,ch,0,victim,TO_VICT);
      act("You tether $N.",FALSE,ch,0,victim,TO_CHAR);
      if (!MOB_FLAGGED(victim, MOB_TETHERED))
	SET_BIT(MOB_FLAGS(victim), MOB_TETHERED);
    }
    else {
      act("$n tries to tether $N.",FALSE,ch,0,victim,TO_NOTVICT);
      act("$n tries to tether you.",FALSE,ch,0,victim,TO_VICT);
      act("You try to tether $N.",FALSE,ch,0,victim,TO_CHAR);
      if (!CAN_SPEAK(victim)) {
	act("$N shies away from $m.",FALSE,ch,0,victim,TO_NOTVICT);
	act("You shy away from $m.",FALSE,ch,0,victim,TO_VICT);
	act("$N shies away from you.",FALSE,ch,0,victim,TO_CHAR);
      }
      else {
	act("You tell $n, 'Get Lost!'",FALSE,ch,0,victim,TO_NOTVICT);
	act("$N tells you, 'Get Lost!'",FALSE,ch,0,victim,TO_CHAR);
      }
    }
  }
  else
    act("$N must follow you first.",FALSE, ch,0,victim,TO_CHAR);
  
}

ACMD(do_switchzones)
{
    long zne;
    int n;
    
    if (IS_SET(ch->specials2.builder_flag,BUILD_ZONE)
	&& ch->specials2.edit_zone > 0){
	n = save_zone(ch->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d zone resets for zone: %ld",
		GET_NAME(ch),n,ch->specials2.edit_zone);
	mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	REMOVE_BIT(ch->specials2.builder_flag,BUILD_ZONE);        	  
    }
    if (IS_SET(ch->specials2.builder_flag,BUILD_SHOPS)
	&& ch->specials2.edit_zone > 0){
      n = save_shops(ch->specials2.edit_zone);
      sprintf(buf,"(%s) Autosaving %d shops for zone: %ld"
	      ,GET_NAME(ch),n,ch->specials2.edit_zone);
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE); 
      REMOVE_BIT(ch->specials2.builder_flag,BUILD_SHOPS);        	  
    }
    if (IS_SET(ch->specials2.builder_flag,BUILD_MOBS)
	&& ch->specials2.edit_zone > 0){
	n = save_mobiles(ch->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d mobiles for zone: %ld",
		GET_NAME(ch),n,ch->specials2.edit_zone);
	mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	REMOVE_BIT(ch->specials2.builder_flag,BUILD_MOBS);        	  
    }
    if (IS_SET(ch->specials2.builder_flag,BUILD_ROOMS) &&
	ch->specials2.edit_zone > 0){
	n = save_rooms(ch->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d rooms for zone: %ld",
		GET_NAME(ch),n,ch->specials2.edit_zone);
	mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	REMOVE_BIT(ch->specials2.builder_flag,BUILD_ROOMS);        	  
    }         
    if (IS_SET(ch->specials2.builder_flag,BUILD_OBJS)
	&& ch->specials2.edit_zone > 0){
	n = save_objects(ch->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d objects for zone: %ld",
		GET_NAME(ch),n,ch->specials2.edit_zone);
	mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	REMOVE_BIT(ch->specials2.builder_flag,BUILD_OBJS);        	  
    }
    
    zne = ch->specials2.edit_zone;
    ch->specials2.edit_zone = ch->specials2.edit_zone2;
    ch->specials2.edit_zone2 = ch->specials2.edit_zone3;    
    ch->specials2.edit_zone3 = zne;
    if (ch->specials2.edit_zone <= 0){
	ch->specials2.edit_zone = -1;
	sprintf(buf,"Zone editing disabled.\r\n");}
    else
	sprintf(buf,"Ok, you now edit zone: %ld.\r\n",ch->specials2.edit_zone);
    send_to_char(buf,ch);
    return;
}

ACMD(do_save)
{
  int n;
  if (IS_NPC(ch) || !ch->desc)
    return;
  
  if (cmd != 0) {
    sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
  }
  if (IS_SET(ch->specials2.builder_flag,BUILD_SHOPS) && ch->specials2.edit_zone > 0 && !PLR_FLAGGED(ch,PLR_BUILDING)){
    n = save_shops(ch->specials2.edit_zone);
    sprintf(buf,"(%s) Autosaving %d shops for zone: %ld",GET_NAME(ch),n
	    ,ch->specials2.edit_zone);
    mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE); 
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_SHOPS);               
  }
  if (IS_SET(ch->specials2.builder_flag,BUILD_MOBS) && ch->specials2.edit_zone > 0 && !PLR_FLAGGED(ch,PLR_BUILDING)){
    n = save_mobiles(ch->specials2.edit_zone);
    sprintf(buf,"(%s) Autosaving %d mobiles for zone: %ld",GET_NAME(ch),n
	    ,ch->specials2.edit_zone);
    mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_MOBS);               
  }
  if (IS_SET(ch->specials2.builder_flag,BUILD_ROOMS) && ch->specials2.edit_zone > 0 && !PLR_FLAGGED(ch,PLR_BUILDING)){
    n = save_rooms(ch->specials2.edit_zone);
    sprintf(buf,"(%s) Autosaving %d rooms for zone: %ld",GET_NAME(ch),n
	    ,ch->specials2.edit_zone);
    mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_ROOMS);        
  }         
  if (IS_SET(ch->specials2.builder_flag,BUILD_ZONE) && ch->specials2.edit_zone > 0 && !PLR_FLAGGED(ch,PLR_BUILDING)){
    n = save_zone(ch->specials2.edit_zone);
    sprintf(buf,"(%s) Autosaving %d zone resets for zone: %ld",GET_NAME(ch),n
	    ,ch->specials2.edit_zone);
    mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_ZONE);               
  }
  if (IS_SET(ch->specials2.builder_flag,BUILD_OBJS) && ch->specials2.edit_zone > 0 && !PLR_FLAGGED(ch,PLR_BUILDING)){
    n = save_mobiles(ch->specials2.edit_zone);
    sprintf(buf,"(%s) Autosaving %d objects for zone: %ld",GET_NAME(ch),n
	    ,ch->specials2.edit_zone);
    mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    REMOVE_BIT(ch->specials2.builder_flag,BUILD_OBJS);               
  }
  save_char(ch, NOWHERE);
  Crash_crashsave(ch);
}


ACMD(do_not_here)
{
   send_to_char("Sorry, but you can't do that here!\r\n", ch);
}

ACMD(do_bless)
{
  struct obj_data *obj=0;
  struct char_data *subj=0;
  int bless_total =0, level=0, casting_die, bits=0;

  if (IS_NPC(ch) && ch->specials.timer)
    return;
  
  one_argument(argument, arg);
  if (subcmd == SCMD_BLESS){
    if (!IS_GOOD(ch)){
      send_to_char("You feel out of tune with your deity.\r\n",ch);
      return;
    }
    if (!*arg){
      send_to_char("Bless what?\r\n", ch);
      return;
    }
    if (!(GET_SKILL(ch, SKILL_BLESS))){
      send_to_char("Nothing happens.\r\n",ch);
      return;
    }
    bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | 
			FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &subj, &obj);
    if (subj == ch){
      send_to_char("You cannot bless yourself.\r\n",ch);
      return;
    }
    if (IS_NPC(ch))
      ch->specials.timer = 2;
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    
  }
  else{
    subj = ch;
    if (IS_NPC(ch))
      ch->specials.timer = 2;
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
  }

  bless_total += GET_WIS(ch);
  bless_total += GET_FOC(ch);
  bless_total += GET_ALIGNMENT(ch)/20;
  bless_total +=(IS_NPC(ch) ? GET_LEVEL(ch)/2 : 3*GET_SKILL(ch,SKILL_BLESS));  
  casting_die = stress_die();
  
  
  bless_total += casting_die;
  bless_total -= spell_info[SKILL_BLESS].min_level ;

  while ((bless_total - level*SPELL_MULT) > 0)
    level++;
  
  if (subj)
    {
      if (subcmd == SCMD_BLESS){
	act("You bless $m.",FALSE,ch,0,subj,TO_CHAR);
	act("$n blesses $N.",FALSE,ch,0,subj,TO_NOTCHAR);
	if (!IS_GOOD(subj))
	  GET_ALIGNMENT(ch) -= bless_total;
	if (!affected_by_spell(subj, SKILL_BLESS))
	  bless_char(ch, subj, bless_total, &level);
      }
      else{
	act("You pray fervently.",FALSE,ch,0,subj,TO_CHAR);
	act("$n cries out to the gods for aid.",FALSE,ch,0,subj,TO_NOTCHAR);
	if (IS_GOOD(ch) && is_afflicted(ch)){
	  if (number(0,2)){
	    GET_ALIGNMENT(ch) -= bless_total;
	    bless_char(ch, subj, bless_total, &level);
	  }
	  else{
	    GET_ALIGNMENT(ch) -= bless_total/3;	    
	    send_to_char("The gods are deaf to your plea.\r\n",ch);
	  }
	}
	else{
	  if (is_afflicted(ch) && !number(0,5)){
	    act("The gods see that $n is reformed!",FALSE,ch,0,0,TO_ROOM);
	    act("The gods see that you are reformed!",FALSE,ch,0,0,TO_CHAR);
	    GET_ALIGNMENT(ch) += bless_total;
	    bless_char(ch, subj, bless_total, &level);	    
	  }
	  else if(number(0,2)){
	    send_to_room("The gods are displeased!\r\n",ch->in_room,0);
	    if (number(0,1000) > GET_ALIGNMENT(ch))
	      curse_char(ch, subj, bless_total, &level);
	  }
	  else
	    send_to_char("The gods are deaf to your plea.\r\n",ch);
	    
	}
	return;
      }	
    }
  else if (obj)
    {
      act("You bless $p.",FALSE,ch,obj,0,TO_CHAR);
      act("$n blesses $p.",FALSE,ch,obj,0,TO_NOTCHAR);
      bless_obj(ch, obj, bless_total, &level);
    }
  else
    send_to_char("Bless what?\r\n",ch);
  
}
bool is_afflicted(struct char_data *ch)
{
  if (IS_AFFECTED(ch, AFF_BLIND))
    return(TRUE);
  if (IS_AFFECTED(ch, AFF_POISON))
    return(TRUE);
  if (IS_AFFECTED(ch, AFF_SPELLBOTCH))
    return(TRUE);
  if (IS_AFFECTED(ch, AFF_CONFUSION))
    return(TRUE);
  if (GET_LEVEL(ch) > 20){
    if (GET_HIT(ch) <= GET_MAX_HIT(ch)/10)
      return(TRUE);
    if (GET_MOVE(ch) <= GET_MAX_MOVE(ch)/10)
      return(TRUE);
  }
  else {
    if (GET_HIT(ch) <= GET_MAX_HIT(ch)/3)
      return(TRUE);
    if (GET_MOVE(ch) <= GET_MAX_MOVE(ch)/3)
      return(TRUE);
    
  }

  return(FALSE);
}
void curse_char(struct char_data *ch, struct char_data *subj, int total, int *level)
{
  
  struct affected_type af;
  struct obj_data *obj, *j;
  int i;
  
  switch (number(0,10)){
  case 0:
    return;
    break;
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    if (!saves_spell(subj, SAVING_SPELL, GET_LEVEL(ch))
	&& !IS_AFFECTED(subj, AFF_SLIPPY)){
      af.type      = SPELL_FOOTSTEPS_OT_SLIPPERY;
      af.location  = APPLY_HITROLL;
      af.modifier  = 0 - *level;  /* Make hitroll worse */
      af.duration  = *level/2;
      af.level = total;
      af.bitvector = AFF_SLIPPY;	
      affect_to_char(subj, &af);
      af.location = APPLY_DEX;
      af.modifier = (0 - *level)/2; 
      affect_to_char(subj, &af);
    }
    if (!number(0,1))
      break;
  case 7:
  case 8:
    if (!saves_spell(subj, SAVING_SPELL, GET_LEVEL(ch)))
      {
	af.type      = SPELL_SPASMS;
	af.location  = APPLY_DEX;
	af.modifier  = -1*(*level/2);
	af.bitvector = 0;
	af.level = total;
	af.duration  = MAX(1,*level/2);
	affect_join(subj, &af, TRUE, TRUE);
	act("$n's hands start to tremble.",FALSE,subj,0,0,TO_ROOM);
	act("Your hands start trembling.",FALSE,subj,0,0,TO_CHAR);
	if (!saves_spell(subj, SAVING_SPELL, GET_LEVEL(ch)))
		{
		  if ((j = subj->equipment[WIELD]) && !number(0,1)){
		    obj_to_char(unequip_char(subj,WIELD),subj, 1);
		    sprintf(buf,"%s",j->name);
		    do_drop(subj,buf,0,0);
		  }
		  if ((j = subj->equipment[HOLD]) && !number(0,1)){
		    obj_to_char(unequip_char(subj,HOLD),subj, 1);
		    sprintf(buf,"%s",j->name);
		    do_drop(subj,buf,0,0);
		  }		    
		}
      }
    if (!number(0,1))
      break;
  case 9:
    if (!saves_spell(subj, SAVING_SPELL, GET_LEVEL(ch))){    
      af.type = SPELL_POISON;
      af.duration = *level;
      af.modifier = MIN(5, *level/2);
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join(subj, &af, FALSE, FALSE);
      poison_vict(subj);
    }
    break;
  case 10:
    if (!saves_spell(subj, SAVING_SPELL, GET_LEVEL(ch))){    
      act("$n is struck by a fiery thunderbolt!",FALSE,subj,0,0,TO_ROOM);
      act("You are struck by a fiery thunderbolt!",FALSE,subj,0,0,TO_CHAR);    
      damage(subj,subj,total, SKILL_BLESS,-1, 0);
      spell_damage_equipment(subj,subj,SKILL_IGNEM,total);
    }
    else {
      act("$n just dodges a fiery thunderbolt!",FALSE,subj,0,0,TO_ROOM);
      act("You narrowly avoid a fiery thunderbolt!",FALSE,subj,0,0,TO_CHAR);    
    }
    break;
  default:
    break;
  }
  act("$n is surrounded by a black aura.",FALSE,subj,0,0,TO_ROOM);
  act("You are surrounded by a black aura.",FALSE,subj,0,0,TO_CHAR);  
  for (i=0;i< MAX_WEAR; i++)
    curse_obj(ch, subj->equipment[i],total, level);
  for (obj= subj->inventory; obj; obj = obj->next_content)
    curse_obj(ch, obj,total, level);
    
  
}
void bless_char(struct char_data *ch, struct char_data *subj, int total, int *level)
{
  int i;
  struct char_data *tch, *next_char;
  struct affected_type *af, *next, *af2, *next2;
  struct obj_data *obj;
  struct affected_type aff;

  act("$n is surrounded by a pale glow.",FALSE,subj,0,0,TO_ROOM);
  act("You are surrounded by a pale glow.",FALSE,subj,0,0,TO_CHAR);

  aff.type      = SKILL_BLESS;
  aff.location  = APPLY_NONE;
  aff.bitvector = 0;
  aff.level = total;
  aff.duration  = 24;
  affect_to_char(subj, &aff);

  for (af = subj->affected;af;af = next){
    next = af->next;
    if (af->level > total)
      continue;
    if (af->type == SPELL_SPASMS){
      total -= af->level;
      affect_remove(subj,af);
      act("$n's hands stop shaking.",TRUE,subj,0,0,TO_ROOM);
      send_to_char("Your hands stop shaking.\r\n", subj);
    }
    if (af->type == SPELL_MILKY_EYES){
      if (af->location == APPLY_HITROLL){
	if (*level > af->duration)
	  affect_remove(subj,af);
	continue;
      }
      if (*level > af->duration){
	act("$n's eyes become clear again.",FALSE,subj,0,0,TO_ROOM);
	send_to_char("You can see once more.\r\n",subj);
	total -= af->level;
	affect_remove(subj,af);
      }
      else {
	act("$n's eyes become clear briefly.",FALSE,subj,0,0,TO_ROOM);
	act("$n's eyes cloud over.",FALSE,subj,0,0,TO_ROOM);
	send_to_char("You can see for a brief instant.\r\n",subj);
	send_to_char("Then darkness descends once more.\r\n",subj);
      }
    }
    else if (af->bitvector == AFF_BLIND){
      total -= af->level;      
      affect_remove(subj,af);
      act("$n's eyes become clear again.",FALSE,subj,0,0,TO_ROOM);
      send_to_char("You can see once more.\r\n",subj);      
    }
    else if (af->bitvector == AFF_SPELLBOTCH){
      total -= af->level;      
      affect_remove(subj,af);
      act("$n seems more hale.",TRUE,subj,0,0,TO_ROOM);
      send_to_char("You feel your health return.\r\n", subj);
    }
    else if (af->bitvector == AFF_CONFUSION){
      total -= af->level;      
      affect_remove(subj,af);
      act("$n seems less dazed.",TRUE,subj,0,0,TO_ROOM);
      send_to_char("You feel your faculties clear.\r\n", subj);
    }   
    else if (af->type == SPELL_POISON  && af->location == APPLY_NONE){
      if (*level < af->modifier){
	total -= af->level;	
	af->modifier -= *level;
	act("$n seems less sick.",TRUE,subj,0,0,TO_ROOM);
	send_to_char("The burning in your veins lessens.\r\n", subj);}
      else{
	total -= af->level;		
	affect_remove(subj,af);
	act("$n is restored to health.",TRUE,subj,0,0,TO_ROOM);
	send_to_char("You feel better!\r\n", subj);
      }
    }
  }

  if (ch->specials.fighting && (total > 100) &&
      (number(0,31) <
       (IS_NPC(ch) ? GET_LEVEL(ch)/6 : GET_SKILL(ch, SKILL_BLESS)))){
    act("$n calls down the wrath of the gods!",FALSE,ch,0,0,TO_ROOM);
    for (tch = world[ch->in_room].people; tch; tch = next_char){
      next_char = tch->next_in_room;
      if (tch->specials.fighting && (tch->specials.fighting == ch)){
	if (!saves_spell(tch, SAVING_SPELL, GET_LEVEL(ch))){    	
	  act("$n is struck by a fiery thunderbolt!",FALSE,tch,0,0,TO_ROOM);
	  act("You are struck by a fiery thunderbolt!",FALSE,tch,0,0,TO_CHAR);
	  damage(tch,tch,total, SKILL_BLESS,-1, 0);
	  spell_damage_equipment(tch,tch,SKILL_IGNEM,total);
	}
	else {
	  act("$n just dodges a fiery thunderbolt!",FALSE,tch,0,0,TO_ROOM);
	  act("You narrowly avoid a fiery thunderbolt!",FALSE,tch,0,0,TO_CHAR);
	}
      }
    }
  }
  
  GET_HIT(subj) = MIN(GET_MAX_HIT(subj), GET_HIT(subj) + total);
  GET_MOVE(subj) = MIN(GET_MAX_MOVE(subj), GET_MOVE(subj) + total);
  
  for (i=0;i< MAX_WEAR; i++)
    bless_obj(ch, subj->equipment[i],total, level);
  for (obj= subj->inventory; obj; obj = obj->next_content)
    bless_obj(ch, obj,total, level);
  
}

void bless_obj(struct char_data *ch, struct obj_data *obj, int total, int *level)
{
  bool blessed=FALSE;
  
  if (!obj)
    return;
  
  if (*level <= 0)
    return;
  
  if ((obj->obj_flags.type_flag != ITEM_DRINKCON) &&
      (obj->obj_flags.type_flag != ITEM_FOOD) &&
      (obj->obj_flags.type_flag != ITEM_FOUNTAIN)){
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) && !number(0,10)){
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
      blessed=TRUE;
      *level -=1;
    }
  }
  else if (obj->obj_flags.value[3]){
    if ((total > obj->obj_flags.value[0]*obj->obj_flags.value[7]) &&
	*level > obj->obj_flags.value[7]){
      obj->obj_flags.value[7] = 0;
      obj->obj_flags.value[3] = 0;
      blessed = TRUE;
      *level =0;
    }
  }
  if (blessed){
    act("$p glows blue.",FALSE,ch,obj,0,TO_ROOM);
    act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);      
  }
}

void curse_obj(struct char_data *ch, struct obj_data *obj, int total, int *level)
{
  bool cursed = FALSE;
  if (!obj)
    return;

  if (*level <= 0)
    return;
  
  
  if ((obj->obj_flags.type_flag != ITEM_DRINKCON) &&
      (obj->obj_flags.type_flag != ITEM_FOOD) &&
      (obj->obj_flags.type_flag != ITEM_FOUNTAIN)){
    if (!IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) && !number(0,10)){
      SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
      *level -= 1;
      cursed=TRUE;
    }
  }
  else if (obj->obj_flags.value[3]){
    if ((total > obj->obj_flags.value[0]*obj->obj_flags.value[7]) &&
	*level > obj->obj_flags.value[7]){
      obj->obj_flags.value[7] = *level;
      obj->obj_flags.value[3] = 1;
      *level = 0;
      cursed=TRUE;
    }
  }
  if (cursed){
    act("$p glows red.",FALSE,ch,obj,0,TO_ROOM);
    act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);      
  }
}

ACMD(do_turn)
{
  ACMD(do_flee);
  struct obj_data *corpse;
  struct char_data *tmp, *next;
  int number=0,turn_level=0, casting_die, exp=0;

  if (IS_NPC(ch) && ch->specials.timer)
    return;
  
  if (ch->specials.fighting){
    send_to_char("You can't concentrate enough whilst fighting!\r\n",ch);
    return;
  }
  if (!IS_GOOD(ch)){
    send_to_char("You feel out of tune with your deity.\r\n",ch);
    return;
  }
  act("$n attempts to summon the aid of $s deity!",FALSE,ch,0,0,TO_ROOM);
  act("You attempt to summon the aid of your deity.",FALSE,ch,0,0,TO_CHAR);

  if (IS_NPC(ch))
    ch->specials.timer = 2;
  WAIT_STATE(ch, PULSE_VIOLENCE*2); 

  turn_level += 2*GET_WIS(ch);
  turn_level += 2*GET_FOC(ch);
  turn_level += GET_ALIGNMENT(ch)/20;
  turn_level += 3*GET_SKILL(ch,SKILL_TURN_UNDEAD);
  casting_die = stress_die();
  
  turn_level += casting_die;
  turn_level -= spell_info[SKILL_TURN_UNDEAD].min_level ;
  if (!(GET_SKILL(ch, SKILL_TURN_UNDEAD))){
    send_to_char("Nothing happens.\r\n",ch);
    return;
  }
  while ((turn_level - number*SPELL_MULT) > 0)
    number++;
  for (tmp = world[ch->in_room].people; tmp; tmp = next){
    next = tmp->next_in_room;
    if (IS_UNDEAD(tmp) && !IS_GOOD(tmp)){
      if (!saves_spell(tmp, SAVING_SPELL, GET_LEVEL(ch))
	  && number
	  && (turn_level > GET_LEVEL(tmp))){
	act("$N is repelled by $n's holy aura.",FALSE,ch,0,tmp,TO_ROOM);
	act("$N is repelled by your holy aura.",FALSE,ch,0,tmp,TO_CHAR);
	turn_level -= GET_LEVEL(tmp);
	GET_HIT(tmp) -= turn_level;
	gain_exp(ch, turn_level, 0);
	update_pos(tmp);
	if (GET_POS(tmp) == POSITION_DEAD){
	  act("$N crumbles to dust.\r\n",FALSE,tmp,0,tmp,TO_ROOM);
	  if (IS_AFFECTED(ch, AFF_GROUP))
	    group_gain(ch, tmp);
	  else{
	    exp = GET_EXP(tmp);
	    exp = MIN(exp,3000*GET_LEVEL(ch));
	    sprintf(buf2, "You receive %d experience points.\r\n", exp);
	    send_to_char(buf2, ch);
	    gain_exp(ch, exp,1);
	  }
	  gain_social_standing(ch, tmp, MODE_KILL);       	  
	  change_alignment(ch, tmp);
	  corpse = die(tmp);
	}
	else if (tmp)
	  do_flee(tmp,"",0,0);
	number--;
      }
      else{
	act("$N is unaffected.",FALSE,ch,0,tmp,TO_CHAR);
	if (!tmp->specials.fighting){
	  act("$n attacks $N.",FALSE,tmp,0,ch,TO_ROOM);
	  damage(ch,tmp,0,SKILL_TURN_UNDEAD, -1,1);
	}

      }
    }
  }
  WAIT_STATE(ch,2*PULSE_VIOLENCE);
}
ACMD(do_tame)
{
  struct char_data *victim;
  int num, mods=0;

 one_argument(argument, arg);  

 if (IS_SET(world[ch->in_room].room_flags, PEACEFULL)) {
   act("You feel too peaceful to contemplate such activities.",
       FALSE,ch,0,0,TO_CHAR);
   return;
 } 
 if (!*arg) {
   send_to_char("Tame what?\r\n",ch);
   return;
 }
 victim = get_char_room_vis(ch, arg);
 if (victim) {
   if (!IS_MOB(victim) || (IS_MOB(victim) && CAN_SPEAK(victim))) {
     send_to_char("Yeah right!  You can only tame dumb animals.\r\n",ch);
     return;
   }
   if (IS_SET(MOB_FLAGS(victim), MOB_AGGRESSIVE) ||
       IS_SET(MOB_FLAGS(victim), MOB_AGGRESSIVE_EVIL) ||
       IS_SET(MOB_FLAGS(victim), MOB_AGGRESSIVE_GOOD) ||
       IS_SET(MOB_FLAGS(victim), MOB_AGGRESSIVE_NEUTRAL)) {
     act("$N is too aggressive to tame.",FALSE,ch,0,victim, TO_CHAR);
     return;
   }
     
   if (IS_SET(MOB_FLAGS(victim), MOB_DOCILE)) {
     act ("$N is already tame.",FALSE,ch,0,victim, TO_CHAR);
     return;
   }
   mods += GET_CHR(ch);
   mods -= GET_INT(victim);
   mods -= (GET_LEVEL(victim) - GET_LEVEL(ch))/5;
   mods -= (abs(GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)))/50;
   mods += GET_SKILL(ch, SKILL_TAME);
   act("You attempt to tame $N.",FALSE,ch,0,victim, TO_CHAR);
   act("$n attempts to tame $N.",FALSE,ch,0,victim, TO_ROOM);   
   if ((num = number(0,31) < mods)) {
     SET_BIT(MOB_FLAGS(victim), MOB_DOCILE);
     act("$N licks your hand docilely.",FALSE,ch,0,victim, TO_CHAR);
     act("$N licks $n's hand docilely.",FALSE,ch,0,victim, TO_ROOM);        
     return;
   }
   if (number(0,5)) {
     act("$N shys away from you.",FALSE,ch,0,victim, TO_CHAR);
     act("$N shys away from $n.",FALSE,ch,0,victim, TO_ROOM);        
     return;
   }
   else {
     act("$N becomes enraged.",FALSE,ch,0,victim, TO_CHAR);
     act("$N becomes enraged.",FALSE,ch,0,victim, TO_ROOM);           
     hit(victim,ch, TYPE_UNDEFINED);
     return;
   }
 }
 send_to_char("You don't see that here.\r\n",ch);
}

ACMD(do_sneak)
{
   struct affected_type af;
   int percent, num, i, total=0;

   if (ch->specials.mount) {
     send_to_char("You had better dismount first.\r\n", ch);
     return;}
   if (ch->specials.cart) {
     act("Yeah right, sneak about dragging $p everywhere.",
	 FALSE,ch, ch->specials.cart, 0,TO_CHAR);
     return;}       

   for (i=0;i<MAX_WEAR;i++) /* the more you carry the harder it is to sneak */
     if (ch->equipment[i])
       total++;
   total += IS_CARRYING_N(ch);
   
   send_to_char("Ok, you'll try to move silently for a while.\r\n", ch);
   if (IS_AFFECTED(ch, AFF_SNEAK))
      affect_from_char(ch, SKILL_SNEAK);

   percent = 0;
   if (GET_WEIGHT(ch) > 1700)
     percent += (GET_WEIGHT(ch) - 1700)/6;
   if (GET_HEIGHT(ch) > 180)   
     percent += (GET_HEIGHT(ch) - 180);
   if (ch->inventory)
     percent += (total + encumberance_level(ch)*3);
   percent -= GET_DEX(ch)/2;
   percent -= GET_SKILL(ch, SKILL_SNEAK);
   
   if ((num = number(0,31)) < percent){
     if (num == 0 && !number(0,31) && GET_SKILL(ch, SKILL_SNEAK) &&
	 (GET_SKILL(ch, SKILL_SNEAK) < 30)){
       SET_SKILL(ch, SKILL_SNEAK, MIN(30,GET_SKILL(ch, SKILL_SNEAK) + number(1,2)));
       send_to_char("You realise a how to improve your sneaking.\r\n",ch);
     }
     return;
   }


   af.type = SKILL_SNEAK;
   af.duration = MAX(2,GET_LEVEL(ch)/10);
   af.modifier = 0;
   af.location = APPLY_NONE;
   af.bitvector = AFF_SNEAK;
   affect_to_char(ch, &af);
}



ACMD(do_hide)
{
   int percent, total=0, i, num;

   for (i=0;i<MAX_WEAR;i++) /* the more you carry the harder it is to sneak */
     if (ch->equipment[i])
       total++;
   total += IS_CARRYING_N(ch);
   
   send_to_char("You attempt to hide yourself.\r\n", ch);

   if (IS_AFFECTED(ch, AFF_HIDE))
      REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
   

   percent += (GET_HEIGHT(ch) - 180);
   if (ch->inventory)
     percent += (total/2 + encumberance_level(ch)*4);
   percent -= GET_SKILL(ch, SKILL_HIDE);

   percent = number(1, 31); /* 101% is a complete failure */

   if ((num = number(0,31)) < percent){
     if (num == 0 && !number(0,31) && GET_SKILL(ch, SKILL_HIDE) &&
	 (GET_SKILL(ch, SKILL_HIDE) < 30)){
       SET_SKILL(ch,SKILL_HIDE,MIN(30,GET_SKILL(ch,SKILL_HIDE)+ number(1,2)));
       send_to_char("You realise a how you might conceal yourself better.\r\n",ch);
     }
     return;
   }


   SET_BIT(ch->specials.affected_by, AFF_HIDE);
}




ACMD(do_steal)
{
   struct char_data *victim;
   struct obj_data *obj;
   char	victim_name[240];
   char	obj_name[240];
   int	percent=0, gold=0, eq_pos=0, pcsteal = 0,exp_gain=0, i;
   extern int	pt_allowed;
   bool ohoh = FALSE;
   struct affected_type *af;
   int steal_loc[MAX_WEAR];
   ACMD(do_gen_com);


   for (i=0;i < MAX_WEAR; i++){
     steal_loc[i] = 10;
   }
   /* set steal difficulties */
   steal_loc[WEAR_BODY] *=2;
   steal_loc[WEAR_LEGS] *=2;
   steal_loc[WEAR_ARMS] *=2;
   steal_loc[WEAR_WRIST_R] /=2;
   steal_loc[WEAR_WRIST_L] /=2;
   steal_loc[WEAR_HEAD] *=3;
   steal_loc[WEAR_NECK_1] *=2;
   steal_loc[WEAR_NECK_2] *=2;
   steal_loc[WEAR_EYES] *=5;
   steal_loc[WEAR_EARS_R] /=2;
   steal_loc[WEAR_EARS_L] /=2;
   steal_loc[WEAR_MOUTH] *=4;
   steal_loc[WEAR_HANDS] *=2;
   steal_loc[WEAR_FEET] /=2;
   steal_loc[WEAR_FINGER_R] /=2;
   steal_loc[WEAR_FINGER_L] /=2;
   steal_loc[WIELD] *=5;
   steal_loc[HOLD] *=5;
   
   argument = one_argument(argument, obj_name);
   one_argument(argument, victim_name);
   
   if (IS_SET(world[ch->in_room].room_flags, PEACEFULL)) 
       {
	 act("You feel too peaceful to contemplate such heinous activities.",
	     FALSE,ch,0,0,TO_CHAR);
	 return;
       } 
   if (ch->specials.mount) {
     send_to_char("You had better dismount first.\r\n", ch);
     return;
   }
   if (ch->specials.carried_by) {
     act("Better ask $N to drop you first.",
	 FALSE,ch,0,ch->specials.carried_by, TO_CHAR);
     return;
   }   

   if (!(victim = get_char_room_vis(ch, victim_name))) {
     send_to_char("Steal what from who?\r\n", ch);
     return;
   } else if (victim == ch) {
     send_to_char("Come on now, I bet you feel like a right pratt!\r\n", ch);
     return;
   }

   if (!IS_NPC(victim) && !pt_allowed){
     send_to_char("Tut tut tut! shame on you.\r\n",ch);
     return;
   }
   if (!pt_allowed) {
     if (!IS_NPC(victim) && !PLR_FLAGGED(victim, PLR_THIEF) && 
	 !PLR_FLAGGED(victim, PLR_KILLER) && !PLR_FLAGGED(ch, PLR_THIEF)) {
       pcsteal = 1;
     }
     
     if (PLR_FLAGGED(ch, PLR_THIEF))
       pcsteal = 1;
     
   }

   /* 31% is a complete failure */
   
   for (i=0;i<MAX_WEAR;i++) /* the more you carry the harder it is to steal */
     if (ch->equipment[i])
       percent++;
   percent /= 2;
   percent += IS_CARRYING_N(ch);
   
   percent += (GET_LEVEL(victim) - GET_LEVEL(ch))/10; /*level diff counts */
   percent -= GET_SKILL(ch, SKILL_STEAL);
   percent -= (GET_GUI(ch) + GET_DEX(ch))/2;
   percent += GET_PER(victim);   
   if (IS_AFFECTED(ch, AFF_SNEAK))
       percent -= 10;
   
   if (GET_POS(victim) < POSITION_SLEEPING)
      percent -= 150; /* ALWAYS SUCCESS */

   /* NO NO With Imp's and Shopkeepers! */
   if ((GET_LEVEL(victim) >= LEVEL_BUILDER) || pcsteal || 
       (IS_MOB(victim) && mob_index[victim->nr].func == shop_keeper))
      percent = 31; /* Failure */

   if (strcmp(obj_name, "coins") && strcmp(obj_name, "gold")) {
     if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->inventory))) {
       for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	 if (victim->equipment[eq_pos] && 
	     (isname(obj_name, victim->equipment[eq_pos]->name)) && 
	     CAN_SEE_OBJ(ch, victim->equipment[eq_pos])) {
	   obj = victim->equipment[eq_pos];
	   break;
	 }
       if (!obj) {
	 act("You failed.", FALSE, ch, 0, victim, TO_CHAR);
	 return;
       }
       else { /* It is equipment */
	 percent += GET_OBJ_WEIGHT(obj)/20; /* Make heavy harder */
	 percent += steal_loc[eq_pos];
	 percent -= GET_STR(ch)/8;
	 percent += (2*GET_SKILL(ch, SKILL_STEAL))/3;/*this is hardest to do */
	 if ((GET_POS(victim) > POSITION_STUNNED) && /* so it works on 1/3   */
	     (percent >= number(0,31))) {            /* your steal skill     */
	   send_to_char("Oops..\r\n", ch);
	   ohoh = TRUE;
	 }
	 else {
	   if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	     if (IS_GODITEM(obj) && GET_LEVEL(ch) < LEVEL_BUILDER){
	       sprintf(buf, "%s: You aren't holy enough to steal this item.\r\n", OBJS(obj, ch));
	       send_to_char(buf, ch);
	       return;}			    		     
	     if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	       act("You unequip $p and steal it.", FALSE, ch, obj , 0, TO_CHAR);
	       act("$n steals $p from $N.", FALSE,ch,obj,victim,TO_NOTVICT);
	       obj_to_char(unequip_char(victim, eq_pos), ch, 0);
	       exp_gain = steal_exp(GET_EXP(victim),obj->obj_flags.cost);
	       if (!AWAKE(victim))
		 exp_gain /= 100;
	       if (!(IS_AFFECTED(victim, AFF_CHARM))){
		 gain_exp(ch,exp_gain,1);
		 gain_social_standing(ch, victim, MODE_STEAL);
		 sprintf(buf,
			 "You gain %d experience points!\r\n"
			 ,exp_gain);
		 send_to_char(buf,ch);
	       }			  
	     }
	   }
	   else
	     send_to_char("You cannot carry that much.\r\n", ch);
	 }
       }
     }
     else {  /* obj found in inventory */
       percent += GET_OBJ_WEIGHT(obj)/20;      /* Make heavy harder       */
       percent -= GET_STR(ch)/8;               /* stealing from inventory */  
       percent += GET_SKILL(ch,SKILL_STEAL)/2; /* quite hard => 1/2 skill */
       
       if (AWAKE(victim) && (percent >= number(0,31))) {
	 ohoh = TRUE;
	 act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	 act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
	 act("$n tries to steal something from $N.",TRUE,ch,0,victim,TO_NOTVICT);
       }
       else { /* Steal the item */
	 if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	   if (IS_GODITEM(obj) && GET_LEVEL(ch) < LEVEL_BUILDER){
	     sprintf(buf, "%s: You aren't holy enough to steal this item.\r\n", OBJS(obj, ch));
	     send_to_char(buf, ch);
	     return;}
	   if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	     obj_from_char(obj,0);
	     obj_to_char(obj, ch,0);
	     send_to_char("Got it!\r\n", ch);
	     exp_gain = steal_exp(GET_EXP(victim),obj->obj_flags.cost);
	     if (!AWAKE(victim))
	       exp_gain /= 100;
	     if (!(IS_AFFECTED(victim, AFF_CHARM))){
	       gain_exp(ch,exp_gain/2,1);
	       gain_social_standing(ch, victim, MODE_STEAL);
	       sprintf(buf,
		       "You gain %d experience points!\r\n"
		       ,exp_gain/2);
	       send_to_char(buf,ch);
	     }			  
	   }
	 }
	 else
	   send_to_char("You cannot carry that much.\r\n", ch);
       }
     }
   }
   else { /* Steal some coins  easiest to do*/
     if (AWAKE(victim) && (percent >= number(0,31))) {
       ohoh = TRUE;
       act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
       act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
       act("$n tries to steal money from $N.",TRUE,ch,0,victim,TO_NOTVICT);
     }
     else {
       /* Steal some gold coins */
       gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 20);
       gold = MIN(5000, gold);
       if (gold < 10)
	 gold = 0;
       if (gold > 10 && report_money_weight(gold) < (CAN_CARRY_W(ch) - IS_CARRYING_W(ch))){
	 change_gold(victim, -gold);
	 change_gold(ch, gold);
	 sprintf(buf, "Bingo!  You got %s.\r\n", report_cost(gold));
	 send_to_char(buf, ch);
       }
       else 
	 send_to_char("You couldn't get any money...\r\n", ch);
     }
   }
   if (ohoh && GET_SKILL(ch, SKILL_STEAL) && (GET_SKILL(ch, SKILL_STEAL) < 30)
       && !number(0,31)){
     send_to_char("You feel your stealing skills improve!\r\n",ch);
     SET_SKILL(ch, SKILL_STEAL,
	       MIN(30,GET_SKILL(ch, SKILL_STEAL) + number(1,2)));
   }
   if (ohoh && IS_NPC(victim) && AWAKE(victim))
     if (IS_SET(MOB_FLAGS(victim), MOB_NICE_THIEF)) {
       sprintf(buf, "%s is a bloody thief!", GET_NAME(ch));
       do_gen_com(victim, buf, 0, SCMD_SHOUT);
       logg(buf);
       send_to_char("Don't you ever do that again!\r\n", ch);
     } 
     else{
       if (IS_NPC(victim) && IS_SET(victim->specials2.act, MOB_MEMORY) && 
	   !IS_NPC(ch) && (GET_LEVEL(ch) < LEVEL_BUILDER))
	 remember(victim, ch);
       if (!IS_SET(victim->specials2.act, MOB_SENTINEL))
	 victim->specials.hunting = ch;
       hit(victim, ch, TYPE_UNDEFINED);}
   if (ohoh && !AWAKE(victim))
     {
       if (IS_AFFECTED(victim,AFF_SLEEP)){
	 act("$n turns restlessly in $s sleep.",FALSE,victim,0,0,TO_ROOM);
	 for(af = victim->affected; af; af = af->next){
	   if (af->duration > 0 && af->type == SPELL_SLEEP)
	     af->duration -= 1;
	   if (af->duration == 0){
	     affect_remove(victim,af);
	     break;} 
	 }
       }
       else
	 GET_POS(victim) = POSITION_SITTING;
     }
}

int steal_exp(int raw_exp, int value)
{
    if (value <100)
	return(raw_exp/25);
    if (value <1000)
	return(raw_exp/10);
    if (value <10000)
	return(raw_exp/6);
    if (value <100000)
	return(raw_exp/3);
    if (value >100000)
	return(raw_exp/2);
    else
	return(0);
}


ACMD(do_study)
{
    SPECIAL(guild);

    int i,j,diff,eff_lev;
    /*    struct list_index_type *guild_list[12], *this_list; */
    struct list_index_type *this_list;
    const char *components[]
      = {"CrIg","CrAq","CrCo","ReCo","MuCo","PeCo","CrAn", "ReTe", "PeIm",
      "InIm", "InVm", "CrIm, MuTe", "MuAu"};
    extern struct spell_info_type spell_info[MAX_SPL_LIST];
    
    if (IS_NPC(ch)) {
      send_to_char("Mobs aren't allowed to learn spells.\r\n",ch);
      return;
    }
    one_argument(argument, arg);

/*
    guild_list[0] = CrIg;
    guild_list[1] = Water;
    guild_list[2] = CrCo;
    guild_list[3] = ReCo;
    guild_list[4] = MuCo;
    guild_list[5] = PeCo;    
    guild_list[6] = Animal;
    guild_list[7] = ReTe;
    guild_list[8] = PeIm;
    guild_list[9] = InIm;
    guild_list[10] = InVm;                
    guild_list[11] = CrIm;
    guild_list[12] = MuTe;
*/    
    if (*arg)
	send_to_char("You can only study spells in a University.\r\n", ch);
    else if (!*arg) {
       sprintf(buf,"You can use these spells:\r\n");
       sprintf(buf2,"%-49s %-5s %-10s %-10s\r\n","Spell Name","Level","Difficulty","How Well");
       strcat(buf,buf2);
       sprintf(buf2,"------------------------------------------------- ----- ---------- ----------\r\n");
       strcat(buf,buf2);
       for (j=0;j<=13;j++)
	 {
	   this_list = guild_list[j];
	   for (i = 0; *(this_list[i].entry) != '\n';i++){	   
	     if ((GET_SKILL(ch,this_list[i].index) > 0)  &&
		 spell_info[this_list[i].index].spll_pointer){
	       diff = calc_difficulty(ch,this_list[i].index)+1;	       
	       sprintf(buf2, "%-44s %s %-5d %-10d %-10d\r\n",
		       this_list[i].entry,this_list[i].components,
		       spell_info[this_list[i].index].min_level,
		       diff,
		       GET_SKILL(ch, this_list[i].index));
	       strcat(buf, buf2);
	     }
	   }
	 }
       page_string(ch->desc,buf,0);
    }
    else
      (void) guild(ch, ch, cmd, "");
}

ACMD(do_learn)
{
   SPECIAL(guild);

   int i,diff, eff_lev, pc_race, j;
   struct list_index_type *guild_list[10], *this_list;
   const char *guilds[] = {"Warrior","Thief","Horsemanship","Ranger","Arcane", "Clerical"};
   bool found=FALSE;

   guild_list[0] = Warrior_skills;
   guild_list[1] = Thief_skills;
   guild_list[2] = Horse_skills;
   guild_list[3] = Ranger_skills;   
   guild_list[4] = am_skills;
   guild_list[5] = Cleric_skills;
   
    if (IS_NPC(ch)) {
      send_to_char("Mobs aren't allowed to learn skills.\r\n",ch);
      return;
    }
   one_argument(argument, arg);
   pc_race = (1 << GET_RACE(ch));
   if (*arg)
      send_to_char("You can only learn skills in a Guild or University.\r\n", ch);
   else if (!*arg) {
       *buf ='\0';
       for (j=0;j<=5;j++)
	 {
	   this_list = guild_list[j];
	   found = FALSE;
	   for (i= 0; (*(this_list[i].entry) != '\n') && !found;i++)
	     if (GET_SKILL(ch,this_list[i].index) > 0)
	       found = TRUE;
	   if (!found)
	     continue;
	   sprintf(buf2,"You have learned these %s skills:\r\n"
		   ,guilds[j]);
	   strcat(buf,buf2);
	   sprintf(buf2,"%-35s %-10s %-10s\r\n"
		   ,"Skill Name"
		   ,"Difficulty"
		   ,"How Well");
	   strcat(buf,buf2);
	   sprintf(buf2,"---------------------------------   ---------- ----------\r\n");
	   strcat(buf,buf2);
	   for (i = 0; *(this_list[i].entry) != '\n';i++){
	     diff = calc_difficulty(ch,this_list[i].index)+1;
	     if (spell_info[this_list[i].index].race_flag &&
		 !IS_SET(spell_info[this_list[i].index].race_flag,pc_race))
	       diff *= 2;
	     if (!spell_info[this_list[i].index].use_bas_lev)
	       eff_lev = (ubyte) spell_info[this_list[i].index].min_level*diff;
	     else
	       eff_lev = (ubyte) spell_info[this_list[i].index].min_level;       
	     if (GET_SKILL(ch,this_list[i].index) > 0) {
	       sprintf(buf2, "%-35s %-10d %-10d\r\n",
		       this_list[i].entry,
		       diff,
		       GET_SKILL(ch, this_list[i].index));
	       strcat(buf, buf2);
	     }
	   }
	   strcat(buf,"\r\n");
	 }
       if (*buf == '\0')
	 strcat(buf,"You have not learned any skills yet.\r\n");
       page_string(ch->desc,buf,0);
   }
   else
     (void) guild(ch, ch, cmd, "");
}

ACMD(do_train)
{
   SPECIAL(guild);

   int i,diff,eff_lev;
   extern struct list_index_type Weapons[];
   extern struct spell_info_type spell_info[MAX_SPL_LIST];

    if (IS_NPC(ch)) {
      send_to_char("Mobs aren't allowed to train weapons.\r\n",ch);
      return;
    }

   one_argument(argument, arg);

   if (*arg)
      send_to_char("You can only train with weapons in the warrior guild.\r\n", ch);
   else if (!*arg) {
     sprintf(buf,"You are proficient in the following:\r\n");
     sprintf(buf2,"%-35s %-10s %-10s\r\n","Weapon Type","Difficulty","How Well");
     strcat(buf,buf2);
     sprintf(buf2,"--------------------------------- ---------- ----------\r\n");
     strcat(buf,buf2);
     for (i = 0; *(Weapons[i].entry) != '\n'; i++){
       diff = calc_difficulty(ch,Weapons[i].index)+1;	       
       if (!spell_info[Weapons[i].index].use_bas_lev)
	 eff_lev = (ubyte) spell_info[Weapons[i].index].min_level*diff;
       else
	 eff_lev = (ubyte) spell_info[Weapons[i].index].min_level;
       if ((GET_SKILL(ch,Weapons[i].index) >0)
	   && eff_lev <= GET_LEVEL(ch)){
	 sprintf(buf2, "%-37s %-10d %-10d\r\n",
		 Weapons[i].entry,
		 diff,
		 GET_SKILL(ch, Weapons[i].index));
	 strcat(buf, buf2);
       }
     }
     send_to_char(buf, ch);
   }
   else
     (void) guild(ch, ch, cmd, "");
}



ACMD(do_visible)
{
   void	appear(struct char_data *ch);

   if IS_AFFECTED(ch, AFF_INVISIBLE) {
      appear(ch);
      send_to_char("You break the spell of invisibility.\r\n", ch);
   } else
      send_to_char("You are already visible.\r\n", ch);
}



ACMD(do_title)
{
    char *point;
    int n=0;
    bool okay=FALSE;
    for (; isspace(*argument); argument++)
	;
    point = argument;
    if (*argument){
    /* need to enforce at least a name in the title */
	while (*point != '\0'){
	    if (*point == '%' && *(point +1) == 'n'){
		okay = TRUE;
		break;}
	    ++n;
	    ++point;}
	if (!okay)
	    {
		send_to_char("You need at least your name in your title.\r\n", ch);
		send_to_char("%n is used to represent you name in the title.\r\n", ch);
		return;
	    }
	if (okay && n > 55)
	    {
		send_to_char("You need the %n closer to the beginning of your title.\r\n", ch);
		return;
	    }
    }
    if (IS_NPC(ch))
	send_to_char("Your title is fine... go away.\r\n", ch);
    else if (PLR_FLAGGED(ch, PLR_NOTITLE))
	send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
    else if (strstr(argument, "(") || strstr(argument, ")"))
      send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
    else if (strlen(argument) > 80)
	send_to_char("Sorry, titles can't be longer than 80 characters.\r\n", ch);
    else {
	if (!*argument)
	    argument = "\0";
	if (GET_TITLE(ch))
	   RECREATE(GET_TITLE(ch), char, strlen(argument) + 1);
	else
	    CREATE(GET_TITLE(ch), char, strlen(argument) + 1);
	strcpy(GET_TITLE(ch), argument);
	parse_text(ch,ch,0,buf2);
	sprintf(buf, "OK, you're now: %s.\r\n", buf2);
	send_to_char(buf, ch);
    }
}


ACMD(do_group)
{
   struct char_data *victim, *k;
   struct follow_type *f;
   bool found;

   one_argument(argument, buf);

   if (!*buf) {
     if (!IS_AFFECTED(ch, AFF_GROUP)) {
       send_to_char("But you are not the member of a group!\r\n", ch);
     } else {
       send_to_char("Your group consists of:\r\n", ch);
       
       k = (ch->master ? ch->master : ch);
       
       if (IS_AFFECTED(k, AFF_GROUP)) {
	 sprintf(buf, "[%4dH %4dV] $N (Leader)",
		 GET_HIT(k), GET_MOVE(k));
	 act(buf, FALSE, ch, 0, k, TO_CHAR);
       }
       
       for (f = k->followers; f; f = f->next)
	 if (IS_AFFECTED(f->follower, AFF_GROUP)) {
	   sprintf(buf, "[%4dH %4dV] $N",
		   GET_HIT(f->follower), GET_MOVE(f->follower));
	   act(buf, FALSE, ch, 0, f->follower, TO_CHAR);
	 }
     }
     
     return;
   }
   
   if (ch->master) {
     act("You can not enroll group members without being head of a group.",
	 FALSE, ch, 0, 0, TO_CHAR);
     return;
   }
   
   if (!str_cmp(buf, "all")) {
     SET_BIT(ch->specials.affected_by, AFF_GROUP);
     for (f = ch->followers; f; f = f->next) {
       victim = f->follower;
       if (!IS_NPC(victim)){
	 if ( GET_LEVEL(victim) > GET_LEVEL(ch) && (GET_LEVEL(victim) - GET_LEVEL(ch) > 25 || GET_LEVEL(victim)/2 > GET_LEVEL(ch))){
	   act("You wouldn't be able to keep up with $N.",FALSE,ch,0,victim,TO_CHAR);
	   act("$n wouldn't be able to keep up with you.",FALSE,ch,0,victim,TO_VICT);
	   break;}
	 else if ( GET_LEVEL(ch) > GET_LEVEL(victim) && (GET_LEVEL(ch) - GET_LEVEL(victim) >25 || GET_LEVEL(ch)/2 > GET_LEVEL(victim))){
	   act("You wouldn't be able to keep up with $n.",FALSE,ch,0,victim,TO_VICT);
	   act("$N wouldn't be able to keep up with you.",FALSE,ch,0,victim,TO_CHAR);
	   break;}
	 if (!IS_AFFECTED(victim, AFF_GROUP)) {
	   act("$N is now a member of your group.", FALSE, ch, 0, victim, TO_CHAR);
	   act("You are now a member of $n's group.", FALSE, ch, 0, victim, TO_VICT);
	   act("$N is now a member of $n's group.",FALSE,ch,0,victim,TO_NOTVICT);
	   SET_BIT(victim->specials.affected_by, AFF_GROUP);
	 }
       }
     }
     return;
   }
   
   if (!(victim = get_char_room_vis(ch, buf))) {
     send_to_char("No one here by that name.\r\n", ch);
   }
   else {
     found = FALSE;
     
     if (victim == ch)
       found = TRUE;
     else {
       for (f = ch->followers; f; f = f->next) {
	 if (f->follower == victim) {
	   found = TRUE;
	   break;
	 }
       }
     }
     
     if (found) {
       if (IS_NPC(victim)){
	 send_to_char("You can only group players.\r\n",ch);
	 return;
       }
       else if (IS_AFFECTED(victim, AFF_GROUP)) {
	 act("$N is already a member of your group.", FALSE, ch, 0, victim, TO_CHAR);
	 return;
       } else {
	 act("$N is now a member of your group.", FALSE, ch, 0, victim, TO_CHAR);
	 act("You are now a member of $n's group.", FALSE, ch, 0, victim, TO_VICT);
	 act("$N is now a member of $n's group.", FALSE, ch, 0, victim, TO_NOTVICT);
	 SET_BIT(victim->specials.affected_by, AFF_GROUP);
       }
     }
     else
       act("$N must follow you to enter your group.", FALSE, ch, 0, victim, TO_CHAR);
   }
}


ACMD(do_ungroup)
{
   struct follow_type *f, *next_fol;
   struct char_data *tch;
   void	stop_follower(struct char_data *ch);

   one_argument(argument, buf);

   if (!*buf) {
      if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
	 send_to_char("But you lead no group!\r\n", ch);
	 return;
      }

      sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
      for (f = ch->followers; f; f = next_fol) {
	 next_fol = f->next;
	 if (IS_AFFECTED(f->follower, AFF_GROUP)) {
	    REMOVE_BIT(f->follower->specials.affected_by, AFF_GROUP);
	    send_to_char(buf2, f->follower);
	    stop_follower(f->follower);
	 }
      }

      send_to_char("You have disbanded the group.\r\n", ch);
      return;
   }

   if (!(tch = get_char_room_vis(ch, buf))) {
     send_to_char("There is no such person!\r\n", ch);
     return;
   }
   
   if (tch->master != ch) {
     send_to_char("That person is not following you!\r\n", ch);
     return;
   }
   
   if (IS_AFFECTED(tch, AFF_GROUP))
     REMOVE_BIT(tch->specials.affected_by, AFF_GROUP);
   
   act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
   act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
   act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
   stop_follower(tch);
}




ACMD(do_report)
{
   sprintf(buf, "%s reports: %4d/%4dH %4d/%4dV",
       GET_NAME(ch),GET_HIT(ch),GET_MAX_HIT(ch),GET_MOVE(ch),GET_MAX_MOVE(ch));
   act(buf,TRUE,ch,0,0,TO_ROOM);
   act(buf,TRUE,ch,0,0,TO_CHAR);
}



ACMD(do_split)
{
   int	amount, num, share;
   char bufr1[80],bufr2[80];
   struct char_data *k;
   struct follow_type *f;

   if (IS_NPC(ch))
      return;

   half_chop(argument,buf,buf2);

   if (is_number(buf)) {
      amount = atoi(buf);
      if (amount <= 0) {
	 send_to_char("He He He... funny.\r\n", ch);
	 return;
      }

      if (!*buf2)
	  amount *= 10;
      else if( !strcmp("penny",buf2) ||  !strcmp("pennies",buf2) ||  !strcmp("coin",buf2) ||  !strcmp("coins",buf2)) 
	  amount *= 10;
      else if( !strcmp("groat",buf2) ||  !strcmp("groats",buf2))
	  amount *= 100;
      else if( !strcmp("crown",buf2) ||  !strcmp("crowns",buf2))
	  amount *= 1000;      
	       
      if (amount > GET_GOLD(ch)) {
	 send_to_char("You don't seem to have that much money to split.\r\n", ch);
	 return;
      }

      k = (ch->master ? ch->master : ch);

      if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
	 num = 1;
      else
	 num = 0;

      for (f = k->followers; f; f = f->next)
	 if (IS_AFFECTED(f->follower, AFF_GROUP) && 
	     (!IS_NPC(f->follower)) && 
	     (f->follower->in_room == ch->in_room))
	    num++;

      if (num && IS_AFFECTED(ch, AFF_GROUP))
	  share = amount/num;
      else {
	 send_to_char("With whom do you wish to share your gold?\r\n", ch);
	 return;
      }
      sprintf(bufr1,"%s",report_cost(amount));
      sprintf(bufr2,"%s",report_cost(share));
      change_gold(ch, -1*(share * (num - 1)));
      if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)
	  && !(IS_NPC(k)) &&  k != ch) {
	  sprintf(buf, "%s splits %s;\r\nyou receive %s.\r\n", GET_NAME(ch),
	  bufr1, bufr2);
	  send_to_char(buf, k);
	  drop_excess_gold(k, share);

      }

      for (f = k->followers; f; f = f->next) {
	 if (IS_AFFECTED(f->follower, AFF_GROUP) && 
	     (!IS_NPC(f->follower)) && 
	     (f->follower->in_room == ch->in_room) && 
	     f->follower != ch) {
	    sprintf(buf, "%s splits %s;\r\nyou receive %s.\r\n", GET_NAME(ch),
	        bufr1, bufr2);
	    send_to_char(buf, f->follower);
	    drop_excess_gold(f->follower, share);
	 }
      }
      sprintf(buf, "You split %s among %d members;\r\n%s each.\r\n",
          bufr1, num, bufr2);
      send_to_char(buf, ch);
   } else {
      send_to_char("How many coins do you wish to split with your group?\r\n", ch);
      return;
   }
}



ACMD(do_quaff)
{
   struct obj_data *temp;
   int	i;
   bool equipped;

   equipped = FALSE;

   one_argument(argument, buf);

   if (ch->specials.fighting){
     send_to_char("Don't be crazy, you can't recite quaff while fighting!\r\n",ch);
     return;
   }

   if (!(temp = get_obj_in_list_vis(ch, buf, ch->inventory))) {
      temp = ch->equipment[HOLD];
      equipped = TRUE;
      if ((temp == 0) || !isname(buf, temp->name)) {
	 act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }
   }

   if (temp->obj_flags.type_flag != ITEM_PHILTRE) {
      act("You can only quaff potions and philtres.", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
   act("You quaff $p which dissolves.", FALSE, ch, temp, 0, TO_CHAR);
   if (temp->action_description)
     act(temp->action_description, FALSE,ch,temp,0,TO_ROOM);
   if (temp->obj_flags.type_flag == ITEM_PHILTRE && temp->obj_flags.value[0])
     {
       ((*spell_info[temp->obj_flags.value[1]].spll_pointer)
	(temp->obj_flags.value[1], 0, ch,"", SPELL_TYPE_PHILTRE, ch, 0));
     }
   else {    /* old potions */
     for (i = 1; i < 4; i++)
       if (temp->obj_flags.value[i] >= 1)
	 ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
	  ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, 0));
   }
   if (equipped)
     unequip_char(ch, HOLD);

   extract_obj(temp,0);
}


ACMD(do_recite)
{
   struct obj_data *scroll, *obj;
   struct char_data *victim;
   int	i, bits;
   bool equipped;

   equipped = FALSE;
   obj = 0;
   victim = 0;

   argument = one_argument(argument, buf);

   if (ch->specials.fighting){
     send_to_char("Don't be crazy, you can't recite anything while fighting!\r\n",ch);
     return;
   }

   if (!(scroll = get_obj_in_list_vis(ch, buf, ch->inventory))) {
      scroll = ch->equipment[HOLD];
      equipped = TRUE;

      if ((scroll == 0) || !isname(buf, scroll->name)) {
	 act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }
   }
   if (scroll->obj_flags.type_flag != ITEM_CANTRIP) {
      act("Recite is normally used for scrolls and cantrips.", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }
   if (ch->equipment[HOLD] || ch->equipment[WIELD]){
     send_to_char("You must have both hands free to recite cantrips.\r\n",ch);
     return;
   }
   if (world[ch->in_room].sector_type == SECT_UNDER_WATER && !IS_AFFECTED(ch,AFF_FREE_ACTION)){
       send_to_char("The spell is garbled by the water.\r\n",ch);
       act("$n starts to gurgle",TRUE,ch,0,0,TO_ROOM);
       return;}
   else if (IS_SET(world[ch->in_room].room_flags, NO_MAGIC)){
       send_to_char("The spell dies on your lips as you see the 'No Magic' sign.\r\n",ch);
       return;}
   if (*argument) {
      bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM | 
          FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
      if (bits == 0) {
	 send_to_char("No such thing around to recite the scroll on.\r\n", ch);
	 return;
      }
   } else
    {
      victim = ch;
   }

   act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
   act("You recite $p which crackles with magic and bursts into flames.", FALSE, ch, scroll, 0, TO_CHAR);
   if (scroll->action_description)
     act(scroll->action_description, FALSE,ch,scroll,0,TO_ROOM);
   
   if (equipped)
      unequip_char(ch, HOLD);
   if (scroll->obj_flags.type_flag == ITEM_CANTRIP && scroll->obj_flags.value[0]) {
     ((*spell_info[scroll->obj_flags.value[1]].spll_pointer)
      (scroll->obj_flags.value[1], 0, ch,(char *)scroll, SPELL_TYPE_CANTRIP,victim, obj));
   }
   else {                       /* old scrolls */
     for (i = 1; i < 4; i++)
       if (scroll->obj_flags.value[i] >= 1){
         if(spell_info[scroll->obj_flags.value[i]].spell_pointer)
	   ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
	    ((byte) scroll->obj_flags.value[0], ch, (char *)scroll, SPELL_TYPE_SCROLL, victim, obj));
       }
   }
   extract_obj(scroll,0);

}



ACMD(do_use)
{
   struct char_data *tmp_char=0;
   struct obj_data *tmp_object=0, *stick=0;
   bool found = FALSE;
   short targets;
   int	bits,j;

   argument = one_argument(argument, buf);

   for (j = 0;j < MAX_WEAR; j++){
     if (ch->equipment[j])
       if (isname(buf,ch->equipment[j]->name)){
	 found = TRUE;
	 break;}
   }
   if (!found)
     {
       act("You do not seem to be wearing that item.", FALSE, ch, 0, 0, TO_CHAR);
       return;
     }
   stick = ch->equipment[j];
   
   if (IS_SET(world[ch->in_room].room_flags, NO_MAGIC)){
     act("$p shivers briefly.",FALSE,ch,stick,0,TO_CHAR);
     return;}
   /*   if (stick->obj_flags2.no_use_timer > 0 &&
       stick->obj_flags.type_flag == ITEM_STAFF)
       {
	   act("$n gestures with $p.", TRUE, ch, stick, 0, TO_ROOM);
	   act("You try to invoke the power of $p.",FALSE,ch,stick,0,TO_CHAR);
	   act("$p whines briefly.",FALSE,ch,stick,0,TO_CHAR);
	   act("$p whines briefly.",FALSE,ch,stick,0,TO_ROOM);
	   return;
       }
   if (stick->obj_flags2.no_use_timer > 0 &&
       stick->obj_flags.type_flag == ITEM_WAND)
       {
	   act("$n gestures with $p.", TRUE, ch, stick, 0, TO_ROOM);
	   act("You try to use $p.",FALSE,ch,stick,0,TO_CHAR);
	   act("$p coughs and dies.",FALSE,ch,stick,0,TO_CHAR);
	   act("$p coughs and dies.",FALSE,ch,stick,0,TO_ROOM);
	   return;
       } */
   if (stick->obj_flags2.no_use_timer > 0 &&
       stick->obj_flags.type_flag == ITEM_ROD)
     {
       act("$n extends $p.", TRUE, ch, stick, 0, TO_ROOM);
       act("You try to use $p.",FALSE,ch,stick,0,TO_CHAR);
       act("$p shudders.",FALSE,ch,stick,0,TO_CHAR);
       act("$p shudders.",FALSE,ch,stick,0,TO_ROOM);
	   return;
     }
   if ((stick->obj_flags.type_flag == ITEM_ROD) && (stick->obj_flags.value[0]))
     {
       one_argument(argument, buf);
       targets = spell_info[stick->obj_flags.value[1]].targets;
       bits = generic_find_target(buf, targets, ch, &tmp_char, &tmp_object);
       if (bits) {
	 if ((bits == TAR_CHAR_ROOM )|| (bits == TAR_FIGHT_VICT)) {
	   act("$n points $p at you.",TRUE, ch, stick, tmp_char, TO_VICT);    
	   act("$n points $p at $N.", TRUE, ch, stick, tmp_char, TO_NOTVICT);
	   act("You point $p at $N.", FALSE, ch, stick, tmp_char, TO_CHAR);
	 } else if (bits == TAR_OBJ_ROOM) {
	   act("$n points $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
	   act("You point $p at $P.", FALSE, ch, stick, tmp_object, TO_CHAR);
	 }
	 else {
	   act("What should $p be pointed at?",FALSE,ch,stick,0,TO_CHAR);
	   return;
	 }
	 stick->obj_flags2.no_use_timer
	   = stick->obj_flags2.no_use_dur;
	 stick->obj_flags2.aff_timer = 0;
	 add_event(2,EVENT_OBJTIMER,0,0,0,0,0,stick,0);
	 if (stick->obj_flags.value[3] > 0) { /* Is there any charges left? */
	   stick->obj_flags.value[3]--;
	   ((*spell_info[stick->obj_flags.value[1]].spll_pointer)
	    (stick->obj_flags.value[1], 0, ch, (char *)stick, SPELL_TYPE_ROD, tmp_char, tmp_object));
	 } else
	   act("$p seems powerless.",FALSE,ch,stick,0,TO_CHAR);
       }
       else
	 act("What should $p be pointed at?",FALSE,ch,stick,0,TO_CHAR);
     }
   /*    else if (stick->obj_flags.type_flag == ITEM_STAFF) {
      act("$n gestures with $p.", TRUE, ch, stick, 0, TO_ROOM);
      act("You invoke the power of $p.", FALSE, ch, stick, 0, TO_CHAR);
      stick->obj_flags2.no_use_timer
	  = stick->obj_flags2.no_use_dur;
      stick->obj_flags2.aff_timer = 0;
      add_event(2,EVENT_OBJTIMER,0,0,0,0,0,stick,0);
      if (stick->obj_flags.value[2] > 0) {  
	  stick->obj_flags.value[2]--;
	  ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	   ((byte) stick->obj_flags.value[0], ch, (char *)stick, SPELL_TYPE_STAFF, 0, 0));
	 
      }
      else {
	
	act("$p seems powerless.",FALSE,ch,stick,0,TO_CHAR);
      }
    }
   else if (stick->obj_flags.type_flag == ITEM_WAND) {
  
      bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM | 
          FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
      if (bits) {
	 if (bits == FIND_CHAR_ROOM) {
	    act("$n points $p at you.",TRUE, ch, stick, tmp_char, TO_VICT);    
	    act("$n points $p at $N.", TRUE, ch, stick, tmp_char, TO_NOTVICT);
	    act("You point $p at $N.", FALSE, ch, stick, tmp_char, TO_CHAR);
	 } else {
	    act("$n points $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
	    act("You point $p at $P.", FALSE, ch, stick, tmp_object, TO_CHAR);
	 }
	 stick->obj_flags2.no_use_timer
	     = stick->obj_flags2.no_use_dur;
	 stick->obj_flags2.aff_timer = 0;
	 add_event(2,EVENT_OBJTIMER,0,0,0,0,0,stick,0);
	 if (stick->obj_flags.value[2] > 0) { 
	    stick->obj_flags.value[2]--;
	    ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	        ((byte) stick->obj_flags.value[0], ch, (char *)stick, SPELL_TYPE_WAND, tmp_char, tmp_object));
	 } else
	     act("$p seems powerless.",FALSE,ch,stick,0,TO_CHAR);
      } else
	  act("What should $p be pointed at?",FALSE,ch,stick,0,TO_CHAR);
   } */
   else if (stick->obj_flags.bitvector)
       {
	   if (stick->obj_flags2.no_use_timer > 0){
	       act("Nothing seems to happen.",FALSE,ch,stick,0,TO_CHAR);
	       return;}
	   else {
	       stick->obj_flags2.no_use_timer
		   = stick->obj_flags2.no_use_dur;
	       stick->obj_flags2.aff_timer
		   = stick->obj_flags2.aff_dur;
	       if ((tmp_char = stick->worn_by)){
		   affect_modify(tmp_char,APPLY_NONE,0,
				 stick->obj_flags2.bitvector_aff,FALSE);
		   stick->obj_flags2.bitvector_aff=stick->obj_flags.bitvector;
		   affect_modify(tmp_char,APPLY_NONE,0,
				 stick->obj_flags2.bitvector_aff,TRUE);
	       }
	       else{
		   stick->obj_flags2.bitvector_aff=stick->obj_flags.bitvector;
	       }
	       act("$p grows slightly warm.", TRUE,ch,stick,0,TO_CHAR);
	       add_event(-1,EVENT_OBJTIMER,0,0,0,0,0,stick,0);
	   }
       }
   else
       send_to_char("You cannot use this object in this way.\r\n",ch);
}
   

ACMD(do_wimpy)
{
   int	wimp_lev;

   one_argument(argument, arg);

   if (!*arg) {
      if (WIMP_LEVEL(ch)) {
	 sprintf(buf, "Your current wimp level is %d hit points.\r\n",
	     ch->specials2.wimp_level);
	 send_to_char(buf, ch);
	 return;
      } else {
	 send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
	 return;
      }
   }

   if (isdigit(*arg)) {
      if ((wimp_lev = atoi(arg))) {
	 if (wimp_lev < 0) {
	    send_to_char("Heh, heh, heh.. we are jolly funny today, eh?\r\n", ch);
	    return;
	 }
	 if (wimp_lev > GET_MAX_HIT(ch)/5) {
	    send_to_char("Sorry max wimp value is 0.2 of your Max hit points\r\n", ch);
	    return;
	 }
	 sprintf(buf, "OK, you'll wimp out if you drop below %d hit points.\r\n",
	     wimp_lev);
	 send_to_char(buf, ch);
	 WIMP_LEVEL(ch) = wimp_lev;
      } else {
	 send_to_char("OK, you'll now tough out fights to the bitter end.\r\n", ch);
	 WIMP_LEVEL(ch) = 0;
      }
   } else
      send_to_char("Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n", ch);

   return;

}
ACMD(do_alias)
{
  int i=0, nalias=0,ii=0;
  bool found= FALSE;

  if (IS_MOB(ch)){
    send_to_char("Mobiles don't get aliases.\r\n",ch);
    return;
  }
  for (; *argument == ' '; argument++)
      ;
  
  if (!*argument){
    send_to_char("You currently have these aliases set:\r\n", ch);
    for (i=0;i<MAX_ALIASES; i++)
      if (ch->specials.aliases[i].alias){
	nalias++;
	sprintf(buf,"[%d] '%s' aliased to '%s'\r\n",
		nalias,
		ch->specials.aliases[i].alias,
		(ch->specials.aliases[i].text ?
		 ch->specials.aliases[i].text: "Error"));
	send_to_char(buf,ch);
      }
    if (!nalias)
      send_to_char("None set.\r\n", ch);
    return;
  }
  half_chop(argument, buf1, buf2);
  if (!*buf2){
    send_to_char("Usage: alias aliasname text.\r\n",ch);
    return;
  }
  else {
    for (i=0; i< MAX_ALIASES; i++)
      if (ch->specials.aliases[i].alias &&
	  !strcmp(ch->specials.aliases[i].alias, buf1)){
	sprintf(buf,"%s is already an alias - unalias it first.\r\n", buf1);
	send_to_char(buf,ch);
	return;
      }
    for (i=0; i< MAX_ALIASES; i++)
      if (!ch->specials.aliases[i].alias){
	found = TRUE;
	break;
      }
    if (!found){
      send_to_char("No more free alias slots, better unalias something.\r\n",ch);
      return;
    }
    else{
      if (!strncmp(buf2, buf1, strlen(buf1))){
	send_to_char("Recursive aliases not allowed.\r\n",ch);
	return;
      }
      for (ii=0;ii< MAX_ALIASES; ii++)
	if (ch->specials.aliases[ii].alias &&
	    !strncmp(buf2, ch->specials.aliases[ii].alias,
		     strlen(ch->specials.aliases[ii].alias))){
	  send_to_char("Possible alias loop detected alias not allowed.\r\n",ch);
	  return;
	}
      CREATE(ch->specials.aliases[i].alias, char, strlen(buf1) +1);
      strcpy(ch->specials.aliases[i].alias, buf1);
      CREATE(ch->specials.aliases[i].text, char, strlen(buf2) +1);
      strcpy(ch->specials.aliases[i].text, buf2);
      sprintf(buf,"Ok, %s aliased to %s\r\n",buf1, buf2);
      send_to_char(buf,ch);
      if (!save_aliases(ch)){
	sprintf(buf, "Error saving aliases for %s",GET_NAME(ch));
	mudlog(buf, CMP, LEVEL_IMPL, TRUE);
      }
    }
    
  }
}
ACMD(do_unalias)
{
  int i=0, nalias=0, num=0;
  bool found= FALSE;

  if (IS_MOB(ch)){
    send_to_char("Mobiles don't get aliases.\r\n",ch);
    return;
  }
  for (; *argument == ' '; argument++)
      ;
  
  if (!*argument){
    send_to_char("Usage: unalias aliasname.\r\n",ch);
    return;
  }
  for (i=0;i<MAX_ALIASES; i++)
    if (ch->specials.aliases[i].alias){
      nalias++;
      if (!strcmp(ch->specials.aliases[i].alias, argument)){
	free(ch->specials.aliases[i].alias);
	ch->specials.aliases[i].alias = 0;	
	free(ch->specials.aliases[i].text);
	ch->specials.aliases[i].text =0;	
	sprintf(buf, "OK, %s is no longer an alias.\r\n",argument);
	found = TRUE;
	if (!save_aliases(ch)){
	  sprintf(buf, "Error saving aliases for %s",GET_NAME(ch));
	  mudlog(buf, CMP, LEVEL_IMPL, TRUE);
	}
	break;
      }
    }
  if (!found)
    sprintf(buf, "%s, is not an alias.\r\n", argument);
  if (!nalias)
    sprintf(buf, "No aliases set.\r\n");
  send_to_char(buf,ch);
}

int save_aliases(struct char_data *ch){
  char filename[50];
  int ierr, i, iwritten=0;
  FILE *fl;
  
  if (!get_alias_filename(GET_NAME(ch), filename))
    return(0);
  ierr = remove(filename);
  if (!(fl = fopen(filename,"w")))
    if (errno != ENOENT){
      sprintf(buf, "SYSERR: Error writing aliases for %s",GET_NAME(ch));
      mudlog(buf, CMP, LEVEL_IMPL, TRUE);
      return(0);
    }
  for(i=0;i< MAX_ALIASES;i++)
    if (ch->specials.aliases[i].alias){
      fprintf(fl,"#\n");
      fwrite_string(fl, ch->specials.aliases[i].alias);
      fwrite_string(fl, ch->specials.aliases[i].text);
      iwritten++;
    }
  if (iwritten == 0)
    ierr = remove(filename);
  fprintf(fl,"$\n");
  fflush(fl);
  fclose(fl);
  return(1);
}
void load_aliases(struct char_data *ch)
{
  char filename[50], chk[10];
  int ierr, i;
  FILE *fl;
  
  if (!get_alias_filename(GET_NAME(ch), filename))
    return;

  if (!(fl = fopen(filename,"r"))){
    if (errno != ENOENT){
      sprintf(buf, "SYSERR: Error reading aliases for %s",GET_NAME(ch));
      logg(buf);
      return;
    }
    sprintf(buf, "%s entering the game with no alias file.",GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_IMPL, FALSE);
    return;
  }
  if (!fscanf(fl, "%s\n", chk)){
    sprintf(buf, "SYSERR: Error reading aliases for %s",GET_NAME(ch));
    mudlog(buf, CMP, LEVEL_IMPL, TRUE);
    return;
  }
  for(i=0;i<MAX_ALIASES ;){
    if (*chk == '$')
      break;
    else if (*chk == '#') {
      ch->specials.aliases[i].alias = fread_string(fl, buf2);
      ch->specials.aliases[i].text = fread_string(fl, buf2);
      i++;
    }
    if (!fscanf(fl, "%s\n", chk)){
      sprintf(buf2, "SYSERR: Format error in alias file of %s", GET_NAME(ch));
      logg(buf2);
      return;
    }
  }
  
  fclose(fl);
	       
}
int	get_alias_filename(char *orig_name, char *filename)
{
   char	*ptr, name[30];

   if (!*orig_name)
      return 0;

   strcpy(name, orig_name);
   for (ptr = name; *ptr; ptr++)
      *ptr = tolower(*ptr);
   
   switch (tolower(*name)) {
   case 'a': case 'b': case 'c': case 'd': case 'e':
      sprintf(filename, "plrobjs/A-E/%s.aliases", name); break;
   case 'f': case 'g': case 'h': case 'i': case 'j':
      sprintf(filename, "plrobjs/F-J/%s.aliases", name); break;
   case 'k': case 'l': case 'm': case 'n': case 'o':
      sprintf(filename, "plrobjs/K-O/%s.aliases", name); break;
   case 'p': case 'q': case 'r': case 's': case 't':
      sprintf(filename, "plrobjs/P-T/%s.aliases", name); break;
   case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
      sprintf(filename, "plrobjs/U-Z/%s.aliases", name); break;
   default:
      sprintf(filename, "plrobjs/ZZZ/%s.aliases", name); break;
   }
   
   return 1;
}


ACMD(do_prompt)
{
   char bf[MAX_STRING_LENGTH];
   int i;

   for (i = 0; *(argument + i) == ' '; i++)
      ;
   
   if (!*argument)
       {
	   if(ch->player.prmpt){
	       send_to_char("Old Prompt:\r\n",ch);
	       send_to_char(ch->player.prmpt, ch);
	       send_to_char("\r\n", ch);}
	   free(ch->player.prmpt);
	   ch->player.prmpt = 0;
	   send_to_char("Enter Prompt. terminate with '@@'.\r\n", ch);
	   ch->desc->max_str = 239;
	   ch->desc->str = &ch->player.prmpt;
	   return;
   }


   else
       {
	   if ( strlen(argument) > 240 )
	       argument[240] = '\0';
	   for(i=0; *(argument + i) == ' '; i++)
	       ;
	   strcpy( bf, (argument + i) );
       }
   

   if(ch->player.prmpt)
       RECREATE(ch->player.prmpt, char, strlen(bf) +1);
   else
       CREATE(ch->player.prmpt, char, strlen(bf) +1);
   strcpy(ch->player.prmpt,bf);
   send_to_char( "Ok.\r\n", ch );
   return;
} 

ACMD(do_display)
{ 
   char bf[MAX_STRING_LENGTH];
   int	i;

   for (i = 0; argument[i] == ' '; i++) 
      ;

   if (!*(argument + i)) {
      send_to_char("FORMAT: display {default | 1 | 2 | 3 | 4 | 5 }\r\n", ch);
      return;
   }

   else if( !strcmp( argument, "default" ) )
       strcpy(bf,"Hp: %1%h%0/%H Mv: %5%v%0/%V Coins: %g, %G%n%4%l%0: %L%nEx: %X [%i%w%f] %o: %O%n%r>");
   else if (atoi(argument) <0 || atoi(argument) > 5){
       send_to_char("Usage: display {default | 1 | 2 | 3| 4 | 5 }\r\n", ch);
       return;}
   else
       switch (atoi(argument))
	   {
	   case 1:
	       strcpy(bf,"%hH %vV %L %O %X>");
	       break;
	   case 2:
	       strcpy(bf,"H: %1%h%0/%H V: %5%v%0/%V Ex: %X%n>");
	       break;
	   case 3:
	       strcpy(bf,"Hp: %1%h%0/%H Mv: %5%v%0/%V%nEx: %X [%i%w%f]%n%o: %O%n>");
	       break;
	   case 4:
	       strcpy(bf,"Hp: %1%h%0/%H Mv: %5%v%0/%V Coins: %g, %G%nEx: %X [%i%w%f]%n%o: %O%n%r>");
	       break;
	   case 5:
	       strcpy(bf,"H:%b%1%h%0/%H  V:%b%5%v%0/%V  Exp:%X%n[%i] %o: %O >");
	       break;
	   default:
	       strcpy(bf,"H:%1%h%0/%H V:%5%v%0/%V%n>");
	       break;
	   }
   if(ch->player.prmpt)
       RECREATE(ch->player.prmpt, char, strlen(bf) +1);
   else
       CREATE(ch->player.prmpt, char, strlen(bf) +1);
   strcpy(ch->player.prmpt,bf);
   send_to_char("Ok.\r\n", ch);
}



ACMD(do_gen_write)
{
   FILE * fl;
   char	*tmp, *filename;
   long	ct;
   char	str[MAX_STRING_LENGTH];

   switch (subcmd) {
   case SCMD_BUG:	filename = BUG_FILE; break;
   case SCMD_TYPO:	filename = TYPO_FILE; break;
   case SCMD_IDEA:	filename = IDEA_FILE; break;
   default: return;
   }

   ct  = time(0);
   tmp = asctime(localtime(&ct));

   if (IS_NPC(ch)) {
      send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
      return;
   }

   for (; isspace(*argument); argument++)
      ;
   if (!*argument) {
      send_to_char("That must be a mistake...\r\n", ch);
      return;
   }

   if (!(fl = fopen(filename, "a"))) {
      perror ("do_gen_write");
      send_to_char("Could not open the file.  Sorry.\r\n", ch);
      return;
   }
   sprintf(str, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
       world[ch->in_room].number, argument);
   fputs(str, fl);
   fclose(fl);
   send_to_char("Ok.  Thanks.  :)\r\n", ch);
}


static char	*ctypes[] = {
   "off", "sparse", "normal", "complete", "\n" };

ACMD(do_color)
{
   int	tp;

   if (IS_NPC(ch))
      return;

   one_argument (argument, arg);

   if (!*arg) {
      sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
      send_to_char(buf, ch);
      return;
   }

   if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
      send_to_char ("Usage: color { Off | Sparse | Normal | Complete }\r\n", ch);
      return;
   }

   REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
   SET_BIT(PRF_FLAGS(ch), (PRF_COLOR_1 * (tp & 1)) | (PRF_COLOR_2 * (tp & 2) >> 1));

   sprintf (buf, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR),
       CCNRM(ch, C_OFF), ctypes[tp]);
   send_to_char(buf, ch);
}


static char	*logtypes[] = {
   "off", "brief", "normal", "complete", "\n" };

ACMD(do_syslog)
{
   int	tp;

   if (IS_NPC(ch))
      return;

   one_argument (argument, arg);

   if (!*arg) {
      tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
	    (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
      sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
      send_to_char(buf, ch);
      return;
   }

   if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
      send_to_char ("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
      return;
   }

   REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
   SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

   sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
   send_to_char(buf, ch);
}


#define TOG_OFF 0
#define TOG_ON  1

#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))

ACMD(do_gen_tog)
{
   long	result;
   extern int	nameserver_is_slow;

   char	*tog_messages[][2] = {
      { "You are now safe from summoning by other players.\r\n",
      "You may now be summoned by other players.\r\n" },
      { "Nohassle disabled.\r\n", 
      "Nohassle enabled.\r\n" },
      { "Brief mode off.\r\n", 
      "Brief mode on.\r\n" },
      { "Compact mode off.\r\n", 
      "Compact mode on.\r\n" },
      { "You can now hear tells.\r\n", 
      "You are now deaf to tells.\r\n" },
      { "You can now hear auctions.\r\n", 
      "You are now deaf to auctions.\r\n" },
      { "You can now hear shouts.\r\n", 
      "You are now deaf to shouts.\r\n" },
      { "You can now hear gossip.\r\n", 
      "You are now deaf to gossip.\r\n" },
      { "You can now hear the congratulation messages.\r\n", 
      "You are now deaf to the congratulation messages.\r\n" },
      { "You can now hear the Wiz-channel.\r\n", 
      "You are now deaf to the Wiz-channel.\r\n" },
      { "You are no longer part of the Quest.\r\n",
      "Okay, you are part of the Quest!\r\n" },
      { "You will no longer see the room flags.\r\n", 
	  "You will now see the room flags.\r\n" },
      { "You will now have your communication repeated.\r\n",
	  "You will no longer have your communication repeated.\r\n" },
      { "HolyLight mode off.\r\n",
	  "HolyLight mode on.\r\n" },
      { "Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
	  "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n" },
      {"You will now hear brags.\r\n",
	 "You are now deaf to brags.\r\n"},
      { "No conditions mode off.\r\n", 
	    "No conditions mode on.\r\n" }
   };
   

   if (IS_NPC(ch)) 
      return;

   switch (subcmd) {
   case SCMD_NOSUMMON	: result = PRF_TOG_CHK(ch, PRF_SUMMONABLE); break;
   case SCMD_NOHASSLE	: result = PRF_TOG_CHK(ch, PRF_NOHASSLE); break;
   case SCMD_BRIEF	: result = PRF_TOG_CHK(ch, PRF_BRIEF); break;
   case SCMD_COMPACT	: result = PRF_TOG_CHK(ch, PRF_COMPACT); break;
   case SCMD_NOTELL	: result = PRF_TOG_CHK(ch, PRF_NOTELL); break;
   case SCMD_NOAUCTION	: result = PRF_TOG_CHK(ch, PRF_NOAUCT); break;
   case SCMD_DEAF	: result = PRF_TOG_CHK(ch, PRF_DEAF); break;
   case SCMD_NOGOSSIP	: result = PRF_TOG_CHK(ch, PRF_NOGOSS); break;
   case SCMD_NOGRATZ 	: result = PRF_TOG_CHK(ch, PRF_NOGRATZ); break;
   case SCMD_NOWIZ	: result = PRF_TOG_CHK(ch, PRF_NOWIZ); break;
   case SCMD_QUEST	: result = PRF_TOG_CHK(ch, PRF_QUEST); break;
   case SCMD_ROOMFLAGS	: result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS); break;
   case SCMD_NOREPEAT	: result = PRF_TOG_CHK(ch, PRF_NOREPEAT); break;
   case SCMD_HOLYLIGHT  : result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT); break;
   case SCMD_SLOWNS	: 
     result = (nameserver_is_slow = !nameserver_is_slow);
     break;
   case SCMD_NOBRAG 	: result = PRF_TOG_CHK(ch, PRF_NOBRAG); break;
   case SCMD_NOCOND	: result = PRF_TOG_CHK(ch, PRF_NOCOND); break;       
   default :
      logg("SYSERR: Unknown subcmd in do_gen_toggle");
      return;
      break;
   }

   if (result)
      send_to_char(tog_messages[subcmd-SCMD_TOG_BASE][TOG_ON], ch);
   else
      send_to_char(tog_messages[subcmd-SCMD_TOG_BASE][TOG_OFF], ch);

   return;
}

void drop_excess_gold(struct char_data *ch, int amount)
{
    int get_amount=0, drop_amount=0;
    struct obj_data *tmp_obj;

    if (report_money_weight(amount) + IS_CARRYING_W(ch) <= CAN_CARRY_W(ch))
	  change_gold(ch, amount);
    else
	{
	    get_amount =
		report_highest_value_for_weight(CAN_CARRY_W(ch)
						- IS_CARRYING_W(ch));
	    drop_amount = amount - get_amount;
	    change_gold(ch, get_amount);
	    if (ch->in_room != NOWHERE){
		send_to_char("You can't carry all the coins.\r\n",ch);
		act("$n drops some coins.",TRUE,ch,0,0,TO_ROOM);
		act("You drop some of the money.",TRUE,ch,0,0,TO_CHAR);
		tmp_obj = create_money(drop_amount);
		obj_to_room(tmp_obj,ch->in_room, FALSE);
	    }
	    return;
	}
    
}
