/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: command interpreter for 'cast' command (spells)                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* spell_parser.c split into two files spell_parser1.c and spell_parser2.c*/
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"


#define NSKILLO(nr, beat, pos, lev, guild, mx_lev, \
     ubaslev, diff, gld, keys, secs, ters, tar, func, \
     race) { \
     spell_info[nr].spll_pointer = (func);    \
     spell_info[nr].beats = (beat); \
     spell_info[nr].minimum_position = (pos);  \
     spell_info[nr].min_level = (lev); \
     spell_info[nr].which_guild = (guild);  \
     spell_info[nr].max_level = (mx_lev); \
     spell_info[nr].difficulty = (diff);\
     spell_info[nr].use_bas_lev = (ubaslev);\
     spell_info[nr].gold = (gld);\
     spell_info[nr].key_stats = (keys); \
     spell_info[nr].sec_stats = (secs); \
     spell_info[nr].ter_stats = (ters); \
     spell_info[nr].targets = (tar); \
     spell_info[nr].race_flag = (race); \
	 }

#define NSPELLO(nr, beat, pos, lev, guild, mx_lev, \
     ubaslev, diff, gld, keys, secs, ters, tar, dur, func,\
     race) { \
     spell_info[nr].spll_pointer = (func);    \
     spell_info[nr].beats = (beat); \
     spell_info[nr].minimum_position = (pos);  \
     spell_info[nr].min_level = (lev); \
     spell_info[nr].which_guild = (guild);  \
     spell_info[nr].max_level = (mx_lev);  \
     spell_info[nr].difficulty = (diff);\
     spell_info[nr].use_bas_lev = (ubaslev);\
     spell_info[nr].gold = (gld);\
     spell_info[nr].key_stats = (keys); \
     spell_info[nr].sec_stats = (secs); \
     spell_info[nr].ter_stats = (ters); \
     spell_info[nr].targets = (tar); \
     spell_info[nr].dam_duration = (dur); \
     spell_info[nr].race_flag = (race); \
	 }

void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
int     spell_lev(struct char_data *caster, int spell);
int     stress_die(void);
int     find_spell_num(char *arg);
char    *find_spell_name(int spl);
char    *find_skill_name(int spl);
char    *rev_search_list(int num, struct list_index_type *list);
char	*skip_spaces(char *string);
void	say_spell(struct char_data *ch, int si, int subcmd );
int	generic_find_target(char *arg, int bitvector,
			    struct char_data *ch,
			    struct char_data **tar_ch,
			    struct obj_data **tar_obj);

/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern char	*spell_wear_off_msg[];
struct spell_info_type spell_info[MAX_SPL_LIST];


struct list_index_type am_skills[] = {
  {"Creo", 112,"\0"},
  {"Intellego", 113,"\0"},
  {"Muto", 114,"\0"},
  {"Perdo", 115,"\0"},
  {"Rego", 116,"\0"},
  {"Animal", 117,"\0"},
  {"Aquam", 118,"\0"},
  {"Auram", 119,"\0"},
  {"Corpus", 120,"\0"},
  {"Herbam", 121,"\0"},
  {"Ignem", 122,"\0"},
  {"Imagonem", 123,"\0"},
  {"Mentem", 124,"\0"},
  {"Terram", 125,"\0"},
  {"Vim", 126,"\0"},
  {"Magic Theory",127,"\0"},
  {"\n", -1,"\0"}
};
struct list_index_type Cleric_skills[] ={
  {"Bless", 135, "\0"},
  {"Turn Undead", 136, "\0"},
  {"\n",-1,"\0"}
};
struct list_index_type Horse_skills[] ={
  {"Mount", 129,"\0"},
  {"Riding", 130,"\0"},  
  {"\n",-1,"\0"}
};
struct list_index_type Ranger_skills[] ={
  {"Track", 131,"\0"},
  {"Taming the wild beast", 132,"\0"},  
  {"\n",-1,"\0"}
};
struct list_index_type Hasad_skills[] ={
  {"Mount", 129,"\0"},
  {"Riding", 130,"\0"},  
  {"Track", 131,"\0"},
  {"Taming the wild beast", 132,"\0"},  
  {"\n",-1,"\0"}
};

struct list_index_type Warrior_skills[] ={
  {"Fisticuffs", 133,"\0"},
  {"Kick", 106,"\0"},
  {"Bash", 107,"\0"},
  {"Rescue", 108,"\0"},
  {"Behead", 111,"\0"},
  {"Dual Wield", 134,"\0"},  
  {"Cavalry Skills", 128,"\0"},
  {"\n",-1,"\0"}
};
struct list_index_type Weapons[] ={
  {"Sword", 201,"\0"},
  {"2H Sword", 202,"\0"},
  {"Dagger", 203,"\0"},
  {"Club", 204,"\0"},
  {"2H Club", 205,"\0"},
  {"Hammer", 206,"\0"},
  {"2H Hammer",207,"\0"},
  {"Axe", 208,"\0"},
  {"2H Axe", 209,"\0"},
  {"Spear", 210,"\0"},
  {"Whip", 211,"\0"},
  {"Claw", 212,"\0"},
  {"\n",-1,"\0"}
};
struct list_index_type Thief_skills[] ={
  {"Sneak", 101,"\0"},
  {"Hide", 102,"\0"},
  {"Steal", 103,"\0"},
  {"Backstab", 104,"\0"},    
  {"Pick Lock", 105,"\0"},
  {"Climb", 109,"\0"},
  {"\n",-1,"\0"}
};
struct list_index_type combat_other[] ={
  {"Invocation of the Milky Eyes",4,"PeCo"},
  {"Palm of Flame", 92,"CrIg"},
  {"Flash of the Scarlet Flames", 93,"CrIg"},
  {"Pilum of Fire", 94,"CrIg"},
  {"Pilum of Fire", 94,"CrIg"},
  {"Pilum of Fire", 94,"CrIg"},
  {"Pilum of Fire", 94,"CrIg"},
  {"Spasms of the Uncontrollable Hands", 341,"ReCo"},
  {"Footsteps of the Slippery Oil", 302,"CrAq"},
  {"Pilum of Fire", 94,"CrIg"},  
  {"Invocation of the Milky Eyes",4,"PeCo"},    
  {"Palm of Flame", 92,"CrIg"},
  {"Flash of the Scarlet Flames", 93,"CrIg"},
  {"Palm of Flame", 92,"CrIg"},
  {"Flash of the Scarlet Flames", 93,"CrIg"},
  {"Palm of Flame", 92,"CrIg"},
  {"Flash of the Scarlet Flames", 93,"CrIg"},
  {"Deluge of the Rushing and Gushing", 320,"CrAq"},
  {"Mighty Torrent of Water", 321,"CrAq"},
  {"Blade of Shivering Ice", 322,"CrAq"},
  {"Blade of Shivering Ice", 322,"CrAq"},
  {"Encase in Ice", 323,"CrAq"},
  {"Mighty Torrent of Water", 321,"CrAq"},
  {"Blade of Shivering Ice", 322,"CrAq"},
  {"Blade of Shivering Ice", 322,"CrAq"},
  {"Encase in Ice", 323,"CrAq"},
  {"Deluge of the Rushing and Gushing", 320,"CrAq"},
  {"Mighty Torrent of Water", 321,"CrAq"},
  {"Deluge of the Rushing and Gushing", 320,"CrAq"},
  {"Mighty Torrent of Water", 321,"CrAq"},
  {"Blade of Shivering Ice", 322,"CrAq"},
  {"Encase in Ice", 323,"CrAq"},  
  {"Lungs of Water and Death", 324,"CrAq"},
  {"Seven League Stride", 346,"ReCo"},  
  {"Seven League Stride", 346,"ReCo"},  
  {"Carbuncle of the Earth",5,"ReTe"},
  {"Carbuncle of the Earth",5,"ReTe"},  
  {"Carbuncle of the Earth",5,"ReTe"},
  {"Carbuncle of the Earth",5,"ReTe"},  
  {"Seven League Stride", 346,"ReCo"},  
  {"Flash of the Scarlet Flames", 93,"CrIg"},  
  {"Pilum of Fire", 94,"CrIg"},
  {"Lance of Solar Fury", 98,"CrIg"},
  {"Weavers Trap of Webs",14,"CrAn"},
  {"Lance of Solar Fury", 98,"CrIg"},
  {"Weavers Trap of Webs",14,"CrAn"},  
  {"Arc of Fiery Ribbons", 95,"CrIg"},
  {"Ball of Abysmal Flame", 96,"CrIg"},
  {"Earth Shock",23,"ReTe"},
  {"Lance of Solar Fury", 98,"CrIg"},
  {"Weavers Trap of Webs",14,"CrAn"},
  {"Lance of Solar Fury", 98,"CrIg"},
  {"Weavers Trap of Webs",14,"CrAn"},  
  {"Arc of Fiery Ribbons", 95,"CrIg"},
  {"Ball of Abysmal Flame", 96,"CrIg"},
  {"Earth Shock",23,"ReTe"},
  {"Lance of Solar Fury", 98,"CrIg"},
  {"Weavers Trap of Webs",14,"CrAn"},
  {"Lance of Solar Fury", 98,"CrIg"},
  {"Weavers Trap of Webs",14,"CrAn"},  
  {"Arc of Fiery Ribbons", 95,"CrIg"},
  {"Ball of Abysmal Flame", 96,"CrIg"},
  {"Earth Shock",23,"ReTe"},
  {"Breath of Vulcan", 97,"CrIg"},
  {"The Pool of Jenny Greenteeth", 325,"CrAq"},
  {"Call to the Watery Grave", 326,"CrAq"},
  {"\n",-1,"\0"}    
};

struct list_index_type combat_self[] ={
  {"Free the Accursed Body", 332,"CrCo"},
  {"Breath of Invigoration",2,"CrCo"},
  {"Breath of Invigoration",2,"CrCo"},
  {"Breath of Invigoration",2,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},
  {"Restoration of the Body Defiled", 334,"CrCo"},
  {"Restoration of the Body Defiled", 334,"CrCo"},
  {"Strength of the Blacksmith", 331,"CrCo"},
  {"Grip of the Lightning Swordsman", 3,"MuCo"},
  {"Fortitude of the Bear", 1,"MuCo"},
  {"Bind Wounds", 330,"CrCo"},
  {"Discern Image of Truth and Falsehood", 19,"InIm"},
  {"Lungs of the Fish",47,"MuAq"},  
  {"Lungs of the Fish",47,"MuAq"},  
  {"\n",-1,"\0"}  
};

struct list_index_type mob_beneficial[] ={
  {"Bind Wounds", 330,"CrCo"},
  {"Bind Wounds", 330,"CrCo"},
  {"Bind Wounds", 330,"CrCo"},
  {"Bind Wounds", 330,"CrCo"},
  {"Bind Wounds", 330,"CrCo"},  
  {"Free the Accursed Body", 332,"CrCo"},
  {"Breath of Invigoration",2,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},  
  {"Restoration of the Body Defiled", 334,"CrCo"},
  {"\n",-1,"\0"}    
};

struct list_index_type Water[] = {
  {"Quench the Thirst of Five Men", 301,"CrAq"},
  {"Lungs of the Fish",47,"MuAq"},
  {"Footsteps of the Slippery Oil", 302,"CrAq"},
  {"Deluge of the Rushing and Gushing", 320,"CrAq"},
  {"Grant Lungs of the Fish",48,"MuAq"},
  {"Mighty Torrent of Water", 321,"CrAq"},
  {"Blade of Shivering Ice", 322,"CrAq"},
  {"Encase in Ice", 323,"CrAq"},
  {"Lungs of Water and Death", 324,"CrAq"},
  {"The Pool of Jenny Greenteeth", 325,"CrAq"},
  {"Call to the Watery Grave", 326,"CrAq"},
  {"\n", -1,"\0"}
};

struct list_index_type InVm[] ={
  {"Sense the Nature of Vis", 20, "InVm"},
  {"Scales of the Magical Balance", 21, "InVm"},  
  {"Perceive Magical Scent", 22, "InVm"},  
  {"\n", -1,"\0"}
};

struct list_index_type ReCo[] ={
  {"Spasms of the Uncontrollable Hands", 341,"ReCo"},
  {"Endurance of the Beserker", 344,"ReCo"},
  {"Gift of Vigour", 343,"ReCo"},
  {"The Walking Corpse", 345,"ReCo"},
  {"Leap of Homecoming", 342,"ReCo"},
  {"Seven League Stride", 346,"ReCo"},
  {"\n", -1,"\0"}
};

struct list_index_type ReTe[] ={
  {"Carbuncle of the Earth",5,"ReTe"},
  {"Earth Shock",23,"ReTe"},
  {"\n", -1,"\0"}
};

struct list_index_type MuTe[] ={
  {"Edge of the Razor",30,"MuTe"},
  {"\n", -1,"\0"}
};

struct list_index_type Mason_spells[] ={
  {"Edge of the Razor",30,"MuTe"},
  {"Carbuncle of the Earth",5,"ReTe"},
  {"Earth Shock",23,"ReTe"},
};

struct list_index_type PeIm[] ={
  {"Invisibility of the Standing Wizard", 27,"PeIm"},
  {"Chamber of Invisibility", 28,"PeIm"},
  {"Veil of Invisibility", 29,"PeIm"},    
  {"\n", -1,"\0"}
};

struct list_index_type CrIm[]= {
  {"Reveal the Lost Image", 24,"CrIm"},    
  {"\n", -1,"\0"}
};

struct list_index_type InIm[]={
  {"Discern Image of Truth and Falsehood", 19,"InIm"},    
  {"\n", -1,"\0"}
};
struct list_index_type archivist[] ={
  {"Discern Image of Truth and Falsehood", 19,"InIm"},
  {"Reveal the Lost Image", 24,"CrIm"},      
  {"Sense the Nature of Vis", 20, "InVm"},
  {"Scales of the Magical Balance", 21, "InVm"},
  {"Perceive Magical Scent", 22, "InVm"},
  {"\n", -1,"\0"}
};
struct list_index_type wizened[] ={
  {"Lamp Without Flame", 91,"CrIg"},
  {"Bind Wounds", 330,"CrCo"},  
  {"Palm of Flame", 92,"CrIg"},
  {"Lungs of the Fish",47,"MuAq"},
  {"\n", -1,"\0"}
};
struct list_index_type clio[] ={
  {"Spasms of the Uncontrollable Hands", 341,"ReCo"},
  {"Grip of the Lightning Swordsman", 3,"MuCo"},  
  {"Fortitude of the Bear", 1,"MuCo"},
  {"Strength of the Blacksmith", 331,"CrCo"},
  {"\n", -1,"\0"}
};
struct list_index_type Stout_spells[] ={
  {"Spasms of the Uncontrollable Hands", 341,"ReCo"},
  {"Grip of the Lightning Swordsman", 3,"MuCo"},
  {"Fortitude of the Bear", 1,"MuCo"},
  {"Endurance of the Beserker", 344,"ReCo"},
  {"Gift of Vigour", 343,"ReCo"},
  {"Invocation of the Milky Eyes",4,"PeCo"},
  {"The Walking Corpse", 345,"ReCo"},
  {"Leap of Homecoming", 342,"ReCo"},
  {"Seven League Stride", 346,"ReCo"},
  {"\n", -1,"\0"}
};

struct list_index_type MuCo[] = {
  {"Grip of the Lightning Swordsman", 3,"MuCo"},
  {"Fortitude of the Bear", 1,"MuCo"},
  {"\n", -1,"\0"}
};

struct list_index_type Thin_spells[] = {
  {"Reveal the Lost Image", 24,"CrIm"},      
  {"Discern Image of Truth and Falsehood", 19,"InIm"},      
  {"Sense the Nature of Vis", 20, "InVm"},  
  {"Grip of the Lightning Swordsman", 3,"MuCo"},
  {"Fortitude of the Bear", 1,"MuCo"},
  {"Scales of the Magical Balance", 21, "InVm"},  
  {"Perceive Magical Scent", 22, "InVm"},  
  {"Rain of Stones",32,"MuAu"},  
  {"Invisibility of the Standing Wizard", 27,"PeIm"},
  {"Chamber of Invisibility", 28,"PeIm"},
  {"Veil of Invisibility", 29,"PeIm"},
  {"\n", -1,"\0"}
};
  
struct list_index_type CrCo[] = {
  {"Bind Wounds", 330,"CrCo"},
  {"Strength of the Blacksmith", 331,"CrCo"},
  {"Free the Accursed Body", 332,"CrCo"},
  {"Breath of Invigoration",2,"CrCo"},
  {"Healing Touch of the Chirugeon", 333,"CrCo"},
  {"Restoration of the Body Defiled", 334,"CrCo"},
  {"Beauty of the Youthful", 335,"CrCo"},
  {"Feet of the Nimble Cat", 336,"CrCo"},
  {"Health of the Hearty Lad", 337,"CrCo"},
  {"Conjure the Homunculous", 338,"CrCo"},
  {"Incantation of the Body Made Whole", 339,"CrCo"},
  {"Conjure the Golem", 340,"CrCo"},
  {"\n", -1,"\0"}
};

struct list_index_type PeCo[] = {
  {"Invocation of the Milky Eyes",4,"PeCo"},
  {"\n", -1,"\0"}
};
  
struct list_index_type CrIg[] = {
  {"Lamp Without Flame", 91,"CrIg"},
  {"Palm of Flame", 92,"CrIg"},
  {"Flash of the Scarlet Flames", 93,"CrIg"},
  {"Pilum of Fire", 94,"CrIg"},
  {"Arc of Fiery Ribbons", 95,"CrIg"},
  {"Ball of Abysmal Flame", 96,"CrIg"},
  {"Breath of Vulcan", 97,"CrIg"},
  {"Lance of Solar Fury", 98,"CrIg"},
  {"Last Flight of the Phoenix", 99,"CrIg"},
  {"\n", -1,"\0"}
};

struct list_index_type Animal[] = {
  {"Feast for Five Men",12,"CrAn"},
  {"Soothe the Pains of the Beast",13,"CrAn"},
  {"Weavers Trap of Webs",14,"CrAn"},
  {"\n", -1,"\0"}
};
struct list_index_type MuAu[] ={
  {"Rain of Stones",32,"MuAu"},
  {"\n", -1,"\0"}
};
struct list_index_type Techs[] = {
  {"Creo", SKILL_CREO,"Cr"},
  {"Intellego", SKILL_INTELLEGO,"In"},
  {"Muto", SKILL_MUTO,"Mu"},
  {"Perdo", SKILL_PERDO,"Pr"},
  {"Rego", SKILL_REGO,"Re"},  
  {"\n", -1,"\0"}
};

struct list_index_type Forms[] = {
  {"Animal", SKILL_ANIMAL,"An"},
  {"Aquam", SKILL_AQUAM,"Aq"},
  {"Auram", SKILL_AURAM,"Au"},
  {"Corpus", SKILL_CORPUS,"Co"},
  {"Herbam", SKILL_HERBAM,"He"},
  {"Ignem", SKILL_IGNEM,"Ig"},
  {"Imagonem", SKILL_IMAGONEM,"Im"},
  {"Mentem", SKILL_MENTEM,"Me"},
  {"Terram", SKILL_TERRAM,"Te"},
  {"Vim", SKILL_VIM,"Vm"},      
  {"\n", -1,"\0"}
};  

struct list_index_type TechForms[] = {
  {"Creo Ignem", CRIG_GUILD,"CrIg"},
  {"Creo Aquam", CRAQ_GUILD,"CrAq"},
  {"Creo Corpus", CRCO_GUILD,"CrCo"},
  {"Rego Corpus", RECO_GUILD,"ReCo"},
  {"Perdo Corpus", PECO_GUILD,"PeCo"},  
  {"Muto Corpus", MUCO_GUILD,"MuCo"},
  {"Creo Animal", CRAN_GUILD,"CrAn"},
  {"Rego Terram", RETE_GUILD,"ReTe"},
  {"Perdo Imagonem", PEIM_GUILD,"PeIm"},
  {"Intellego Imagonem", INIM_GUILD,"InIm"},
  {"Intellego Vim", INVM_GUILD,"InVm"},
  {"Creo Imagonem", CRIM_GUILD,"CrIm"},      
  {"Muto Terram", MUTE_GUILD,"MuTe"},
  {"Muto Auram", MUAU_GUILD,"MuAu"},        
  {"\n", -1,"\0"}
};  
struct list_index_type *guild_list[] ={
  CrIg,
  Water,
  CrCo,
  ReCo,
  MuCo,
  PeCo,
  Animal,
  ReTe,
  PeIm,
  InIm,
  InVm,
  CrIm,
  MuTe,
  MuAu
};

bool botch_level_0(struct char_data *ch, int spell_no,
	      int botches, int delay, char *arg
	      , struct char_data *tar_char
	      , struct obj_data *tar_obj)
{
  struct char_data *tmp;
  int temp;
  int value;
  bool state = TRUE;
  
  value = number(0,20);
  /*  sprintf(buf,"botch_level_0: value1 = %d",value);
  logg(buf); */
  switch (value)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      act("$n messed up $s spell.",TRUE,ch,0,0,TO_ROOM);	       
      act("Loses $s balance and falls over.",TRUE,ch,0,0,TO_ROOM);
      act("You lost your balance.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= 25;
      if (ch->specials.mount) {
	act("$n falls from $N's back.",FALSE,ch,0,ch->specials.mount,TO_ROOM);
	act("You fall from $N's back.",FALSE,ch,0,ch->specials.mount,TO_CHAR);
	rmdamage(ch,GET_HEIGHT(ch->specials.mount)/number(5,20));
	tmp = ch->specials.mount;
	unmount(ch);
	if (!number(0,10) && IS_MOB(tmp) && (GET_INT(tmp) < 10))
	  do_flee(tmp,"",0,0);
      }
      GET_POS(ch) = POSITION_SITTING;
      state = FALSE;
      break;
    case 5:
    case 6:		
    case 7:		
    case 8:
    case 9:
    case 10:		
      act("$n messed up $s spell.",TRUE,ch,0,0,TO_ROOM);
      act("You botched the spell. You are fatigued.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= 50;
      state = FALSE;
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      act("$n messed up $s spell.",TRUE,ch,0,0,TO_ROOM);
      act("You botched the spell. You are fatigued.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= 65;
      state = FALSE;
      break;
    case 16:
    case 17:
    case 18:		
    case 19:
    case 20:
      act("$n really messes up $s spell, $e seem exhausted!",TRUE,ch,0,0,TO_ROOM);
      act("You botched the spell badly. You are fatigued.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= GET_MAX_MOVE(ch)/2;
      break;
    }

    value = number(0,20);
    /*    sprintf(buf,"botch_level_0: value2 = %d",value);
    logg(buf); */
    if (value == 0)
        {
        act("You botched the spell badly. You gain new insight.",TRUE,ch,0,0,TO_CHAR);
        if (!IS_NPC(ch)){
          	switch (number(0,2)) {
        	case 0:
        	  temp = GET_SKILL(ch,spell_info[spell_no].key_stats) +1;
        	  SET_SKILL(ch,spell_info[spell_no].key_stats, MIN(30,temp));
                  break;
	        case 1:
        	  temp = GET_SKILL(ch,spell_info[spell_no].sec_stats) +1;
        	  SET_SKILL(ch,spell_info[spell_no].sec_stats, MIN(30,temp));
                  break;
	        case 2:
        	  temp = GET_SKILL(ch,SKILL_MAGICTHEORY) +1;
        	  SET_SKILL(ch,SKILL_MAGICTHEORY, MIN(30,temp));
                  break;
	        }
              }
        }
  return(state);    
}
bool botch_level_1(struct char_data *ch, int spell_no,
	      int botches, int delay, char *arg
	      , struct char_data *tar_char
	      , struct obj_data *tar_obj)
{
  int temp;
  struct affected_type af;
  int value;
  bool state = TRUE;

  value = number(0,3);
  /*  sprintf(buf, "botch_level_1: value1 = %d", value);
  logg(buf);*/
  switch(value)
    {
    case 0:
      act("$n messed up $s spell.",TRUE,ch,0,0,TO_ROOM);
      act("You botched the spell. You are fatigued.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= 65;
      state = FALSE;
      break;
    case 1:
    case 2: 
      act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
      act("$n seems dazed!",TRUE,ch,0,0,TO_ROOM);	
      act("You botched the spell badly. You are disoriented.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= GET_MOVE(ch) + number(1,30);
      af.type      = spell_no;
      af.location  = APPLY_INT;
      af.modifier  = -10;  
      af.duration  = 1;
      af.level     = GET_LEVEL(ch);
      af.bitvector = AFF_CONFUSION;
      affect_join(ch, &af, FALSE, FALSE);
      af.location  = APPLY_FOCUS;
      af.modifier  = -10;  
      af.duration  = 1;
      affect_join(ch, &af, FALSE, FALSE);
      break;
    default:
      break;
  }
  value = number(0,10);
  /*  sprintf(buf, "botch_level_1: value2 = %d", value);
  logg(buf);*/
  if (value == 0){
    act("You botched the spell badly. You gain new insight.",TRUE,ch,0,0,TO_CHAR);
    GET_MOVE(ch) -= 25;
    if (!IS_NPC(ch)){
      switch (number(0,3)) {
      case 0:
	temp = GET_SKILL(ch,spell_info[spell_no].sec_stats) +1;
	SET_SKILL(ch,spell_info[spell_no].sec_stats, MIN(30,temp));
        break;
      case 1:
	temp = GET_SKILL(ch,spell_info[spell_no].key_stats) +1;
	SET_SKILL(ch,spell_info[spell_no].key_stats, MIN(30,temp));
        break;
      case 2:
      case 3:
        temp = GET_SKILL(ch,spell_no) + 1;
        SET_SKILL(ch,spell_no,MIN(30,temp));
        break;
      }
    }
  }
  return(state);	
}
bool botch_level_2(struct char_data *ch, int spell_no,
		   int botches, int delay, char *arg
		   , struct char_data *tar_char
		   , struct obj_data *tar_obj)
{
  int temp;
  struct affected_type af;
  struct char_data *vict;
  struct obj_data *vict_obj;
  int value;
  bool state = TRUE;
  
  value = number(0,12);
  /*  sprintf(buf, "botch_level_2: value1 = %d", value);
  logg(buf); */
  switch(value){
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:	    
  case 5:
  case 6:
    if (IS_SET(spell_info[spell_no].targets, TAR_CHAR_ROOM)){
      act("$n messes up $s spell targeting!",TRUE,ch,0,0,TO_ROOM);
      act("Uhoh! something is badly wrong!",TRUE,ch,0,0,TO_CHAR);
      for (vict = world[ch->in_room].people;vict;vict = vict->next_in_room)
	if (IS_NPC(vict) && vict != tar_char&& !number(0,1))
	  break;
      GET_MOVE(ch) -= GET_MAX_MOVE(ch)/2;
      if (vict)
	add_event(delay,EVENT_SPELL,0,-30,0,spell_no,arg,ch,vict);
      else
	add_event(delay,EVENT_SPELL,0,-30,0,spell_no,arg,ch,ch);
    }
    else if (IS_SET(spell_info[spell_no].targets, TAR_OBJ_INV) ||
	     IS_SET(spell_info[spell_no].targets, TAR_OBJ_EQUIP) ||
	     IS_SET(spell_info[spell_no].targets, TAR_OBJ_WORLD) ||
	     IS_SET(spell_info[spell_no].targets, TAR_OBJ_ROOM)){
      act("$n messes up $s spell targeting!",TRUE,ch,0,0,TO_ROOM);
      for (vict_obj = ch->inventory;
	   vict_obj; vict_obj = vict_obj->next_content)
	if (vict_obj != tar_obj && !number(0,1))
	  break;
      if (vict_obj)
	add_event(delay,EVENT_SPELL,0,-30,1,spell_no,arg,ch,vict_obj);
    }
    else {
      act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
      act("$n seems dazed!",TRUE,ch,0,0,TO_ROOM);	
      act("You botched the spell badly. You are disoriented.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= GET_MOVE(ch) + number(1,30);
      af.type      = spell_no;
      af.location  = APPLY_INT;
      af.modifier  = -15;  
      af.duration  = 1;
      af.level     = spell_info[spell_no].min_level + 30;
      af.bitvector = AFF_CONFUSION;
      affect_join(ch, &af, FALSE, FALSE);
      af.location  = APPLY_FOCUS;
      af.modifier  = -15;  
      af.duration  = 1;
      affect_join(ch, &af, FALSE, FALSE);		
    }
    break;
  case 8:
  case 9:
    act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
    act("$n seems dazed!",TRUE,ch,0,0,TO_ROOM);	
    act("You botched the spell badly. You are disoriented.",TRUE,ch,0,0,TO_CHAR);
    GET_MOVE(ch) -= GET_MOVE(ch) + number(15,30);
    af.type      = spell_no;
    af.location  = APPLY_INT;
    af.modifier  = -20;  
    af.duration  = 1;
    af.level     = spell_info[spell_no].min_level + 30;
    af.bitvector = AFF_CONFUSION;
    affect_to_char(ch, &af);
    af.location  = APPLY_FOCUS;
    af.modifier  = -20;  
    af.duration  = 1;
    affect_to_char(ch, &af);		
  case 10:
  case 11:
    act("$n messed up $s spell.",TRUE,ch,0,0,TO_ROOM);
    act("You botched the spell. You are fatigued.",TRUE,ch,0,0,TO_CHAR);
    GET_MOVE(ch) -= 65;
    state = FALSE;
    break;
  case 12:
    act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
    act("$n seems dazed!",TRUE,ch,0,0,TO_ROOM);	
    act("You botched the spell badly. You are disoriented.",TRUE,ch,0,0,TO_CHAR);
    GET_MOVE(ch) -= GET_MOVE(ch) + number(1,30);
    af.type      = spell_no;
    af.location  = APPLY_INT;
    af.modifier  = -10;  
    af.duration  = 1;
    af.level     = spell_info[spell_no].min_level + 30;
    af.bitvector = AFF_CONFUSION;
    affect_join(ch, &af, FALSE, FALSE);
    af.location  = APPLY_FOCUS;
    af.modifier  = -10;  
    af.duration  = 1;
    affect_join(ch, &af, FALSE, FALSE);
    break;
  default:
    break;
  }


  value = number(0,3);
  if (!value){
    /*    sprintf(buf, "botch_level_2: value2 = %d", value);
    logg(buf);*/
    act("Something seems to go wrong with $n's spell.",TRUE,ch,0,0,TO_ROOM);
    act("You botched the spell badly. You gain new insight.",TRUE,ch,0,0,TO_CHAR);
    switch(value){
    case 0:
      temp = GET_SKILL(ch,spell_info[spell_no].key_stats) +1;
      SET_SKILL(ch,spell_info[spell_no].key_stats, MIN(30,temp));
      break;
    case 1:
      temp = GET_SKILL(ch,spell_info[spell_no].sec_stats) + 1;
      SET_SKILL(ch,spell_info[spell_no].sec_stats,MIN(30,temp));
      break;
    case 2:
    case 3:
      temp = GET_SKILL(ch,spell_no) + 1;
      SET_SKILL(ch,spell_no,MIN(30,temp));
      break;
    default:
      break;
    }
  }
  return(state);	
}
bool botch_level_3(struct char_data *ch, int spell_no,
		   int botches, int delay, char *arg
		   , struct char_data *tar_char
		   , struct obj_data *tar_obj)
{
  int temp;
  struct affected_type af;
  struct char_data *vict;
  struct obj_data *vict_obj;
  int value;
  bool state = TRUE;
  
  value = number(1,10);
  /*  sprintf(buf,"botch_level_3: value1 = %d", value);
  logg(buf);*/
  switch (value)
    {
    case 1:
	act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
	act("You botched the spell badly. You damage your health.",TRUE,ch,0,0,TO_CHAR);
	af.type      = spell_no;
	af.location  = APPLY_CON;
	af.modifier  = -number(1,3);  
	af.duration  = -1;
	af.level     = spell_info[spell_no].min_level + 30;
	af.bitvector = AFF_SPELLBOTCH;
	affect_join(ch, &af, TRUE, FALSE );
	GET_MAX_MOVE(ch) -= number(GET_LEVEL(ch)/10 + 1,GET_LEVEL(ch)/5 + 5);
	break;
    case 2:
	act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
	act("$n passes out!",TRUE,ch,0,0,TO_ROOM);	
	act("You botched the spell badly. You faint!.",TRUE,ch,0,0,TO_CHAR);
	GET_MOVE(ch) -= GET_MOVE(ch) + number(1,30);
	GET_POS(ch) = POSITION_SLEEPING;
	break;
    case 3:
    case 4:
      act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
      act("$n seems dazed!",TRUE,ch,0,0,TO_ROOM);	
      act("You botched the spell badly. You are disoriented.",TRUE,ch,0,0,TO_CHAR);
      GET_MOVE(ch) -= GET_MOVE(ch) + number(15,30);
      af.type      = spell_no;
      af.location  = APPLY_INT;
      af.modifier  = -20;  
      af.level     = spell_info[spell_no].min_level + 30;
      af.duration  = 1;
      af.bitvector = AFF_CONFUSION;
      affect_to_char(ch, &af);
      af.location  = APPLY_FOCUS;
      af.modifier  = -20;  
      af.duration  = 1;
      affect_to_char(ch, &af);		
      break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      if (IS_SET(spell_info[spell_no].targets, TAR_CHAR_ROOM)){
	act("$n messes up $s spell targeting!",TRUE,ch,0,0,TO_ROOM);
	act("Uhoh! something is badly wrong!",TRUE,ch,0,0,TO_CHAR);
	for (vict = world[ch->in_room].people;vict;vict = vict->next_in_room)
	  if (IS_NPC(vict) && vict != tar_char&& !number(0,1))
	    break;
	GET_MOVE(ch) -= GET_MAX_MOVE(ch)/2;
	if (vict)
	  add_event(delay,EVENT_SPELL,0,-30,0,spell_no,arg,ch,vict);
	else
	  add_event(delay,EVENT_SPELL,0,-30,0,spell_no,arg,ch,ch);
	
      }
      else if (IS_SET(spell_info[spell_no].targets, TAR_OBJ_INV) ||
	       IS_SET(spell_info[spell_no].targets, TAR_OBJ_EQUIP) ||
	       IS_SET(spell_info[spell_no].targets, TAR_OBJ_WORLD) ||
	       IS_SET(spell_info[spell_no].targets, TAR_OBJ_ROOM)){
	act("$n messes up $s spell targeting!",TRUE,ch,0,0,TO_ROOM);
	for (vict_obj = ch->inventory;
	     vict_obj; vict_obj = vict_obj->next_content)
	  if (vict_obj != tar_obj && !number(0,1))
	    break;
	if (vict_obj)
	  add_event(delay,EVENT_SPELL,0,-30,1,spell_no,arg,ch,vict_obj);
      }
      else {
	act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
	act("$n seems dazed!",TRUE,ch,0,0,TO_ROOM);	
	act("You botched the spell badly. You are disoriented.",TRUE,ch,0,0,TO_CHAR);
	GET_MOVE(ch) -= GET_MOVE(ch) + number(15,30);
	af.type      = spell_no;
	af.location  = APPLY_INT;
	af.modifier  = -20;  
	af.level     = spell_info[spell_no].min_level + 30;
	af.duration  = 1;
	af.bitvector = AFF_CONFUSION;
	affect_to_char(ch, &af);
	af.location  = APPLY_FOCUS;
	af.modifier  = -20;  
	af.duration  = 1;
	affect_to_char(ch, &af);		
      }
      break;
    }
  value = number(1,12);
  /*  sprintf(buf,"botch_level_3: value2 = %d", value);
  logg(buf);*/
  switch (value)
    {
    case 1:
           act("You botched the spell badly. You gain new insight.",TRUE,ch,0,0,TO_CHAR);
        if (!IS_NPC(ch)){
  	  switch (number(0,2)) {
  	    case 0:
  	      temp = GET_SKILL(ch,spell_info[spell_no].sec_stats) +1;
  	      SET_SKILL(ch,spell_info[spell_no].sec_stats, MIN(30,temp));
              break;
  	    case 1:
  	      temp = GET_SKILL(ch,spell_info[spell_no].key_stats) +1;
  	      SET_SKILL(ch,spell_info[spell_no].key_stats, MIN(30,temp));
              break;
  	    case 2:
  	      temp = GET_SKILL(ch,SKILL_MAGICTHEORY) +1;
  	      SET_SKILL(ch,SKILL_MAGICTHEORY, MIN(30,temp));
              break;
    	    }
          }
        break;
    case 2:
    case 3:
    case 4:
    case 5:
       act("You gain deep new insight.",TRUE,ch,0,0,TO_CHAR);
       if (!IS_NPC(ch)){
         temp = GET_SKILL(ch,spell_no) + 2;
         SET_SKILL(ch,spell_no,MIN(30,temp));
       }
      break;
    default:
      break;
    }
  return(state);
}
bool botch_level_4(struct char_data *ch, int spell_no,
		   int botches, int delay, char *arg
		   , struct char_data *tar_char
		   , struct obj_data *tar_obj)
{
  int temp;
  struct affected_type af;
  int value;
  bool state = TRUE;
  
  value = number(0,9);
  /*  sprintf(buf,"botch_level_4: value1 = %d", value);
  logg(buf);*/
  switch (value)
      {
      case 0:
      case 1:
      case 2:
	act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
	act("$n seems dazed!",TRUE,ch,0,0,TO_ROOM);	
	act("You botched the spell badly. You are disoriented.",TRUE,ch,0,0,TO_CHAR);
	GET_MOVE(ch) -= GET_MOVE(ch) + number(15,30);
	af.type      = spell_no;
	af.location  = APPLY_INT;
	af.modifier  = -20;  
	af.level     = spell_info[spell_no].min_level + 30;
	af.duration  = 1;
	af.bitvector = AFF_CONFUSION;
	affect_to_char(ch, &af);
	af.location  = APPLY_FOCUS;
	af.modifier  = -20;  
	af.duration  = 1;
	affect_to_char(ch, &af);		
      case 3:
      case 4:
      case 5:
	act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
	act("$n passes out!",TRUE,ch,0,0,TO_ROOM);	
	act("You botched the spell badly. You faint!.",TRUE,ch,0,0,TO_CHAR);
	GET_MOVE(ch) -= GET_MOVE(ch) + number(1,30);
	GET_POS(ch) = POSITION_SLEEPING;
	break;
      case 6:
      case 7:
      case 8:	    
      case 9:
	act("$n really messes up $s spell!",TRUE,ch,0,0,TO_ROOM);
	act("You botched the spell badly. You damage your health.",TRUE,ch,0,0,TO_CHAR);
	af.type      = spell_no;
	af.location  = APPLY_CON;
	af.modifier  = -number(1,3);  
	af.duration  = -1;
	af.level     = spell_info[spell_no].min_level + 30;
	af.bitvector = AFF_SPELLBOTCH;
	affect_join(ch, &af, TRUE, FALSE );
	GET_MAX_MOVE(ch) -= number(GET_LEVEL(ch)/10 + 1,GET_LEVEL(ch)/5 + 5);
	break;
      }
  value = number(0,9);
  /*  sprintf(buf,"botch_level_4: value1 = %d", value);
  logg(buf);*/
  switch (value)
      {
      case 0:
      case 1:
        act("You botched the spell badly. You gain new insight.",TRUE,ch,0,0,TO_CHAR);
        if (!IS_NPC(ch)){
  	  switch (number(0,2)) {
  	    case 0:
  	      temp = GET_SKILL(ch,spell_info[spell_no].sec_stats) +1;
  	      SET_SKILL(ch,spell_info[spell_no].sec_stats, MIN(30,temp));
              break;
  	    case 1:
  	      temp = GET_SKILL(ch,spell_info[spell_no].key_stats) +1;
  	      SET_SKILL(ch,spell_info[spell_no].key_stats, MIN(30,temp));
              break;
  	    case 2:
  	      temp = GET_SKILL(ch,SKILL_MAGICTHEORY) +1;
  	      SET_SKILL(ch,SKILL_MAGICTHEORY, MIN(30,temp));
              break;
    	    }
          }
        break;
      case 2:
      case 3:
      case 4:
	act("You gain deep new insight.",TRUE,ch,0,0,TO_CHAR);
        GET_MOVE(ch) -= 25;
	if (!IS_NPC(ch)){
	  temp = GET_SKILL(ch,spell_no) + 3;
	  SET_SKILL(ch,spell_no,MIN(30,temp));
	}
	break;
      default:
        break;
      }
  return(state);
}
bool do_botch(struct char_data *ch, int spell_no,
	      int botches, int delay, char *arg
	      , struct char_data *tar_char
	      , struct obj_data *tar_obj)
{
    int spell_lev, prof, total=0;

    if (!IS_NPC(ch))
	prof = GET_SKILL(ch,spell_no);
    else
	prof = MAX(30, GET_LEVEL(ch)/3);
    spell_lev = spell_info[spell_no].min_level;
    if (spell_lev > GET_LEVEL(ch) + 10)
	while (spell_lev > GET_LEVEL(ch) + 10){
	    spell_lev -= 10;
	    botches++;}
    else if (spell_lev < GET_LEVEL(ch) - 10)
	while (spell_lev < GET_LEVEL(ch) - 10){
	    spell_lev += 10;
	    botches--;}
    if (prof < 5){
	    prof -= 5;
	    botches -= prof/2;}
    if (prof > 5)
	while (prof > 0){
	    prof -= 5;
	    botches--;}
    botches = MAX(1,botches);
    botches = MIN(10,botches);
    total = 0;
    while (botches > 0){
	if (!stress_die())
	    total++;
	sprintf(buf,"Botch total: %d\r\n",total);
	if (PRF_FLAGGED(ch,PRF_DEBUG))
	    send_to_char(buf,ch);
	botches--;
    }
    if (!total)
	return(FALSE);
    total--;
    total = MIN(4,total);
    switch(total)
	{
	case 0:
	    return(botch_level_0(ch,spell_no,botches,delay,arg,tar_char,tar_obj));
	    break;
	case 1:
	    return(botch_level_1(ch,spell_no,botches,delay,arg,tar_char,tar_obj));
	    break;
	case 2:
	    return(botch_level_2(ch,spell_no,botches,delay,arg,tar_char,tar_obj));
	    break;
	case 3:
	    return(botch_level_3(ch,spell_no,botches,delay,arg,tar_char,tar_obj));
	    break;
	case 4:
	    return(botch_level_4(ch,spell_no,botches,delay,arg,tar_char,tar_obj));
	    break;
	}
    return(TRUE);
}

ACMD(do_invoke)
{
  ACMD(do_action);
  struct obj_data *tar_obj, *vis;
  struct char_data *tar_char;
  char name[MAX_STRING_LENGTH], *wrd;
  int	inv, qend, spll, result, i,delay, casting_total, botches = 1, cast_die;
  bool no_voice = FALSE, no_gesture = FALSE, spontaneous = FALSE;
  bool botched=FALSE, use_vis=FALSE;
  int target_ok, bonus, tech, form, pawns=0;
  argument = skip_spaces(argument);
  bonus = combat_bonus(ch);
  if (ch->equipment[WEAR_MOUTH])
    no_voice = TRUE;
  if (ch->equipment[WIELD] && ch->equipment[HOLD])
    no_gesture = TRUE;
  if (ch->equipment[HOLD] && is_held_obj_twohanded(ch, ch->equipment[HOLD]))
    no_gesture = TRUE;
  if (ch->equipment[WIELD] && is_two_handed(ch, ch->equipment[WIELD]))
    no_gesture = TRUE;
  if (IS_NPC(ch) && ch->specials.timer)
    return;
  /* things that add into botches */

  if (ch->specials.fighting)
    botches++;
  if (ch->specials.fighting && ch->specials.fighting->specials.fighting == ch)
    botches++;
  if (IS_AFFECTED(ch, AFF_CHARM))
    botches += 3;
  if (IS_AFFECTED(ch,AFF_CONFUSION))
    botches += 3;    
  if (!affected_by_spell(ch, SPELL_ENDURANCE))
    botches += bonus;
  
  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast what which where?\r\n", ch);
    return;}
  while (*argument == '-')
    {
      half_chop(argument,buf, buf2);
      if (strstr("-novoice", buf))
	no_voice = TRUE;
      else if (strstr("-nogesture", buf))
	no_gesture = TRUE;
      else if (strstr("-vis", buf))
	use_vis = TRUE;
      strcpy(argument, buf2);
    }
  if (*argument != '\'') {
    send_to_char("Magic must always be enclosed by the holy magic symbols: '\r\n", ch);
    return;
  }
  if (world[ch->in_room].sector_type == SECT_UNDER_WATER && !IS_AFFECTED(ch,AFF_FREE_ACTION))
    no_gesture = TRUE;
  if (world[ch->in_room].sector_type == SECT_UNDER_WATER && !IS_AFFECTED(ch,AFF_WATER_BREATH))
    no_voice = TRUE;    

  /* Locate the last quote && lowercase the magic words (if any) */
  *buf = 0;
  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\'') ; qend++)
    *(buf+qend-1) = LOWER(*(argument + qend));
    
  if (*(argument + qend) != '\'') {
    send_to_char("Magic must always be enclosed by the holy magic symbols: '\r\n", ch);
    return;
  }
    
  *(buf + qend -1) = '\0';
    
  spll = find_spell_num(buf);
    
  if (spll < 0) {
    send_to_char("Your lips do not move, no magic appears.\r\n", ch);
    return;
  }
  if (spll == SPELL_LANCE_OF_SOLAR_FURY)
    if (IS_DARK(ch->in_room) || (world[ch->in_room].sector_type
				 == SECT_INSIDE || IS_SET(world[ch->in_room].room_flags, INDOORS)))
      {
	act("You are unable to see the sun.\r\nYou cannot cast this spell here!",FALSE,ch,0,0,TO_CHAR);
	return;
      }
    else if (weather_info.sunlight == SUN_DARK){
      act("It's nighttime.\r\nYou cannot cast this spell now!",FALSE,ch,0,0,TO_CHAR);
      return;	    
    }

  if (spll >= 0 && spell_info[spll].spll_pointer) {
	
    if (!GET_SKILL(ch,spll) || (IS_NPC(ch) && !MOB_FLAGGED(ch, MOB_SPELL_CASTER))){
      wrd = find_spell_name(spll);
      sprintf(buf,"You attempt to spontaneously cast '%s'.\r\n",((wrd) ?
								 wrd: "this is a bug report me"));
      send_to_char(buf,ch);
      spontaneous = TRUE;
    }
    if (spell_info[spll].minimum_position == POSITION_STANDING
	&& ch->specials.fighting){
      send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
      return;
    }
    else if (GET_POS(ch) < spell_info[spll].minimum_position
	     && GET_POS(ch) < POSITION_STANDING) {
      switch (GET_POS(ch))
	{
	case POSITION_SLEEPING :
	  send_to_char("You dream of great magical powers.\r\n", ch);
	  break;
	case POSITION_RESTING :
	  send_to_char("You can't concentrate enough while resting.\r\n", ch);
	  break;
	case POSITION_SITTING :
	  send_to_char("You can't do this sitting!\r\n", ch);
	  break;
	} /* Switch */
      return;
    }
	
    argument += qend + 1;	/* Point to the last ' */
    for (; *argument == ' '; argument++)
      ;
	    
    /* **************** Locate targets **************** */
	
    target_ok = FALSE;
    tar_char = 0;
    tar_obj = 0;
    target_ok = generic_find_target(argument, spell_info[spll].targets,
				    ch, &tar_char, &tar_obj);
	    
 
    if (!target_ok) {
      if (*name) {
	if (IS_SET(spell_info[spll].targets, TAR_CHAR_ROOM))
	  send_to_char("Nobody here by that name.\r\n", ch);
	else if (IS_SET(spell_info[spll].targets, TAR_CHAR_WORLD))
	  send_to_char("Nobody playing by that name.\r\n", ch);
	else if (IS_SET(spell_info[spll].targets, TAR_OBJ_INV))
	  send_to_char("You are not carrying anything like that.\r\n", ch);
	else if (IS_SET(spell_info[spll].targets, TAR_OBJ_ROOM))
	  send_to_char("Nothing here by that name.\r\n", ch);
	else if (IS_SET(spell_info[spll].targets, TAR_OBJ_WORLD))
	  send_to_char("Nothing at all by that name.\r\n", ch);
	else if (IS_SET(spell_info[spll].targets, TAR_OBJ_EQUIP))
	  send_to_char("You are not wearing anything like that.\r\n", ch);
	else if (IS_SET(spell_info[spll].targets, TAR_OBJ_WORLD))
	  send_to_char("Nothing at all by that name.\r\n", ch);
	return;
      }
      else { /* Nothing was given as argument */
	if (spell_info[spll].targets < TAR_OBJ_INV){
	  send_to_char("Who should the spell be cast upon?\r\n", ch);
	  return;
	}
	else{
	  send_to_char("What should the spell be cast upon?\r\n",ch);
	  return;
	}
      }
    }
    /* TARGET IS OK */
    else {
      if ((tar_char == ch)
	  && IS_SET(spell_info[spll].targets, TAR_SELF_NONO)) {
	send_to_char("You cannot cast this spell upon yourself.\r\n", ch);
	return;}
      else if ((tar_char != ch) && IS_SET(spell_info[spll].targets,
					  TAR_SELF_ONLY)) {
	send_to_char("You can only cast this spell upon yourself.\r\n", ch);
	return;
      }
      else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
	send_to_char("You are afraid that it could harm your master.\r\n", ch);
	return;
      }
    }
    if (GET_MOVE(ch) <= 0){
      act("You are too exhausted to concentrate.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
    delay = spell_info[spll].beats*10;;
    if (!no_gesture) {
      if (tar_char){
	if (tar_char != ch){
	  act("$n starts making cryptic gestures at $N."
	      ,TRUE,ch,tar_obj, tar_char, TO_ROOM);
	  act("You start making mystic gestures."
	      ,TRUE,ch,tar_obj,tar_char, TO_CHAR);		
	}
	else{
	  act("$n starts gesturing mystically.",TRUE,ch,0,0,TO_ROOM);
	  act("You start tracing the mystic runes.",TRUE,ch,0,0,TO_CHAR);
	    }
      }
      else if (tar_obj){
	sprintf(buf,"$n starts making cryptic gestures at $p.");
	act(buf,TRUE,ch,tar_obj,tar_char,TO_ROOM);
	sprintf(buf,"You start making cryptic gestures at $p.");
	act(buf,TRUE,ch,tar_obj,tar_char,TO_CHAR);
      }
      else{
	act("$n starts gesturing mystically.",TRUE,ch,0,0,TO_ROOM);
	act("You start tracing the mystic runes.",TRUE,ch,0,0,TO_CHAR);
      }
    }
    else
      delay += 20;
    if (!no_voice) {
      act("$n starts muttering under $s breath.",TRUE,ch,0,0,TO_ROOM);
      act("You start to intone the words of power.",TRUE,ch,0,0,TO_CHAR);
      say_spell(ch, spll, -1);	    
    }
    else
    delay += 20 ;
    
    if (!IS_NPC(ch))
      delay /= (1 + ((float)GET_SKILL(ch,spll)/30.));
    else
      delay /= (1 + GET_LEVEL(ch)/3);
	
    if ((spell_info[spll].spll_pointer == 0) && spll > 0){
      send_to_char("Sorry, this magic does not exist!\r\n", ch);
      return;
    }
    /* casting total additions*/
    if (use_vis && !GET_SKILL(ch, SKILL_VIM)){
      send_to_char("You are not knowlegable enough to use raw Vis!.\r\n",ch);
      use_vis = FALSE;
    }
    tech = spell_info[spll].key_stats;
    form = spell_info[spll].sec_stats;
    
    casting_total = GET_INT(ch);
    /* find vis objects */
    if (use_vis)
      for (i = 0; i < MAX_WEAR; i++)
	if (vis = ch->equipment[i])
	  if (GET_ITEM_TYPE(vis) == ITEM_VIS)
	    if (((vis->obj_flags.value[1] == tech) &&
		 (!vis->obj_flags.value[2] 
		  || vis->obj_flags.value[2] == form)) ||
		((vis->obj_flags.value[2] == form) &&
		 (!vis->obj_flags.value[1] 
		  || vis->obj_flags.value[1] == tech))){
	      pawns += vis->obj_flags.value[3];
	    }
    botches += pawns;
    pawns = MIN(pawns, GET_SKILL(ch, SKILL_VIM));
    if (!IS_NPC(ch)){
      casting_total += 3*GET_SKILL(ch,tech);
      casting_total += 3*GET_SKILL(ch,form);
    }
    else
      casting_total += GET_LEVEL(ch)*3;
    casting_total += 5*pawns;
    casting_total -= 3*bonus;
    cast_die = stress_die();
    sprintf(buf,"No of Botch dies: %d\r\n",botches);
    if (PRF_FLAGGED(ch,PRF_DEBUG))	    
      send_to_char(buf,ch);
    if (!cast_die){
      if ((botched
	   = do_botch(ch,spll,botches,delay,argument,tar_char,tar_obj)))
	return;
    }
    if (IS_AFFECTED(ch,AFF_CONFUSION))
      {
	act("You are feeling a bit muddled.",FALSE,ch,0,0,TO_CHAR);
	casting_total /= 2;
      }
    casting_total +=  cast_die;

    if (IS_AFFECTED(ch, AFF_CHARM))
      casting_total -= 200;
    if (IS_SET(world[ch->in_room].room_flags, NO_MAGIC)){
      send_to_char("Something about this place makes magic very difficult.\r\n",ch);
      casting_total -= 200;
    }
    if (no_voice)
      casting_total -= 30;
    if (no_gesture)
      casting_total -= 15;
    if (!spontaneous)
      result = spell_info[spll].min_level - casting_total;
    else
      result = 2*spell_info[spll].min_level - casting_total;
    sprintf(buf,"Total: %d, Level: %d, Result: %d\r\n",
	    casting_total, spell_info[spll].min_level, result);
    if (PRF_FLAGGED(ch,PRF_DEBUG))
      send_to_char(buf,ch);
    if (result >= 30){
      send_to_char("You fail to cast the spell...",ch);
      send_to_char("You fatigue yourself.\r\n",ch);
      act("$n loses $s concentration and is fatigued.",TRUE,ch,0,0,TO_ROOM);
      if (IS_NPC(ch))
	ch->specials.timer = delay/2;
      WAIT_STATE(ch,delay);
      GET_MOVE(ch) -= 45;
      return;}
    else {
      send_to_char("You cast the spell.",ch);
      if (result > 0){
	if (result >= 15)
	  GET_MOVE(ch) -= 30;
	else 
	  GET_MOVE(ch) -= 15;
	send_to_char("..You fatigue yourself.\r\n",ch);
      }
      else
	send_to_char("\r\n",ch);
      if (tar_char && !tar_obj)
	add_event(delay/2,EVENT_SPELL,subcmd,result,0,spll,argument,ch,tar_char);
      else if (!tar_char && tar_obj)
	add_event(delay/2,EVENT_SPELL,subcmd,result,1,spll,argument,ch,tar_obj);
      else
	add_event(delay/2,EVENT_SPELL,subcmd,result,1,spll,argument,ch,0);
      if (spll == SPELL_LAST_FLIGHT_OT_POENIX){
	act("$N's body starts to glow in an alarming fashion.", FALSE,ch,0,ch,
	    TO_NOTVICT);
	act("You feel your body starting to fill with energy.", FALSE,ch,0,ch,
	    TO_CHAR);	  
      }
      if (IS_NPC(ch))
	ch->specials.timer = delay;
      WAIT_STATE(ch,delay);
      /* scrap and clear the vis list */
      if (use_vis)
	for (i=0;i < MAX_WEAR;i++)
	  if (vis = ch->equipment[i])
	    if (GET_ITEM_TYPE(vis) == ITEM_VIS)
	      if (((vis->obj_flags.value[1] == tech) &&
		   (!vis->obj_flags.value[2] 
		    || vis->obj_flags.value[2] == form)) ||
		  ((vis->obj_flags.value[2] == form) &&
		   (!vis->obj_flags.value[1] 
		    || vis->obj_flags.value[1] == tech))){
		vis = unequip_char(ch, i);
		obj_to_room(vis,ch->in_room, FALSE);
		scrap_item(vis);
	      }
      
      return;
    }
  }
  
  switch (number(1, 5)) {
  case 1:
    send_to_char("Bylle Grylle Grop Gryf???\r\n", ch);
    break;
  case 2:
    send_to_char("Olle Bolle Snop Snyf?\r\n", ch);
    break;
  case 3:
    send_to_char("Olle Grylle Bolle Bylle?!?\r\n", ch);
    break;
  case 4:
    send_to_char("Gryffe Olle Gnyffe Snop???\r\n", ch);
    break;
  default:
    send_to_char("Bolle Snylle Gryf Bylle?!!?\r\ny", ch);
    break;
  }

}
int	generic_find_target(char *arg, int bitvector,
			    struct char_data *ch,
			    struct char_data **tar_ch,
			    struct obj_data **tar_obj)
{
  char name[100];
  int inv;
  
   if (*arg){
      arg = one_argument(arg, name);
      if (IS_SET(bitvector, TAR_CHAR_ROOM))
	if ((*tar_ch = get_char_room_vis(ch, name)))
	  return(TAR_CHAR_ROOM);
		    
      if (IS_SET(bitvector,TAR_CHAR_WORLD))
	if ((*tar_ch = get_char_vis(ch, name)))
	  return(TAR_CHAR_WORLD);
	    
      if (IS_SET(bitvector, TAR_OBJ_INV))
	if ((*tar_obj = get_obj_in_list_vis(ch, name,ch->inventory)))
	  return(TAR_OBJ_INV);
	    
      if (IS_SET(bitvector,TAR_OBJ_ROOM))
	if ((*tar_obj=get_obj_in_list_vis(ch,name,world[ch->in_room].contents)))
	  return(TAR_OBJ_ROOM);
      
      if (IS_SET(bitvector, TAR_OBJ_WORLD))
	if ((*tar_obj = get_obj_vis(ch, name)))
	  return(TAR_OBJ_WORLD);
		    
      if (IS_SET(bitvector, TAR_OBJ_EQUIP)) 
	if((*tar_obj = get_object_in_equip_vis(ch, name,ch->equipment,&inv)))
	  return(TAR_OBJ_EQUIP);
      
      if (IS_SET(bitvector, TAR_SELF_ONLY))
	if ((str_cmp(GET_NAME(ch), name) == 0) ||
	    (str_cmp("self", name) == 0) ||
	    (str_cmp("me", name) == 0)){
	  *tar_ch = ch;
	  return(TAR_SELF_ONLY);
	}
	    
      if (bitvector == TAR_IGNORE)
	{
	  *tar_ch = 0;
	  *tar_obj = 0;
	  return(TAR_IGNORE);
	}	    
    }
    else
      { /* No argument was typed */
	    
	if (IS_SET(bitvector, TAR_FIGHT_SELF))
	  if (ch->specials.fighting) {
	    *tar_ch = ch;
	    return(TAR_FIGHT_SELF);
	  }
		    
	if (IS_SET(bitvector, TAR_FIGHT_VICT))
	  if (ch->specials.fighting) {
	    /* WARNING, MAKE INTO POINTER */
	    *tar_ch = ch->specials.fighting;
	    return(TAR_FIGHT_VICT);
	  }

	if (IS_SET(bitvector, TAR_SELF_ONLY))
	  {
	    *tar_ch = ch;
	    return(TAR_SELF_ONLY);
	  }
	if (IS_SET(bitvector, TAR_IGNORE))
	  {
	    *tar_ch = 0;
	    *tar_obj = 0;
	    return(TAR_IGNORE);
	  }
	    
      }

}
int find_spell_num(char *arg)
{
    int spl = -1;
    
    spl = search_list(arg, CrIg, FALSE);
    if (spl < 0)
	spl = search_list(arg, Water, FALSE);
    if (spl < 0)
	spl = search_list(arg, CrCo, FALSE);
    if (spl < 0)
	spl = search_list(arg, ReCo, FALSE);
    if (spl < 0)
	spl = search_list(arg, ReTe, FALSE);
    if (spl < 0)
	spl = search_list(arg, MuTe, FALSE);
    if (spl < 0)
	spl = search_list(arg, MuCo, FALSE);
    if (spl < 0)
	spl = search_list(arg, PeCo, FALSE);        
    if (spl < 0)
	spl = search_list(arg, PeIm, FALSE);        
    if (spl < 0)
	spl = search_list(arg, InIm, FALSE);        
    if (spl < 0)
	spl = search_list(arg, InVm, FALSE);        
    if (spl < 0)
	spl = search_list(arg, CrIm, FALSE);        
    if (spl < 0)
	spl = search_list(arg, Animal, FALSE);        
    if (spl < 0)
	spl = search_list(arg, MuAu, FALSE);        
    return(spl);
}
char *find_spell_name(int spl)
{
    char *name = 0;
    name = rev_search_list(spl, CrIg);
    if (!name)
	name = rev_search_list(spl, Water);
    if (!name)
	name = rev_search_list(spl, CrCo);
    if (!name)
	name = rev_search_list(spl, ReCo);
    if (!name)
	name = rev_search_list(spl, ReTe);
    if (!name)
	name = rev_search_list(spl, MuTe);
    if (!name)
	name = rev_search_list(spl, MuCo);
    if (!name)
	name = rev_search_list(spl, PeCo);
    if (!name)
	name = rev_search_list(spl, PeIm);
    if (!name)
	name = rev_search_list(spl, InIm);
    if (!name)
	name = rev_search_list(spl, InVm);
    if (!name)
	name = rev_search_list(spl, CrIm);
    if (!name)
	name = rev_search_list(spl, Animal);    
    if (!name)
	name = rev_search_list(spl, MuAu);    
    return(name);
}
char *find_skill_name(int spl)
{
    char *name = 0;
    name = rev_search_list(spl, Warrior_skills);
    if (!name)
	name = rev_search_list(spl, Thief_skills);
    if (!name)
	name = rev_search_list(spl, Ranger_skills);
    if (!name)
	name = rev_search_list(spl, Horse_skills);
    if (!name)
	name = rev_search_list(spl, am_skills);    
    return(name);
}
void	assign_spell_pointers(void)
{
   int	i;

   for (i = 0; i < MAX_SPL_LIST; i++)
      spell_info[i].spll_pointer = 0;


   NSPELLO(1, 4, POSITION_STANDING, 20, MUCO_GUILD, 20,TRUE,1,10, 
	   SKILL_MUTO, SKILL_CORPUS, 0,
	   TAR_CHAR_ROOM, 0, cast_fortitude_ot_bear,0)
     
     NSPELLO(3, 4, POSITION_STANDING, 20, MUCO_GUILD, 20,TRUE,1,10, 
	     SKILL_MUTO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0, cast_lightning_swordsman,0)
     
     NSPELLO(4, 4, POSITION_FIGHTING, 80, PECO_GUILD, 20,TRUE,3,10, 
	     SKILL_PERDO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0, cast_invok_ot_milky_eyes,0)
     
     NSPELLO(20, 4, POSITION_STANDING, 10, INVM_GUILD, 20,TRUE,1,10, 
	     SKILL_INTELLEGO, SKILL_VIM, 0,
	     TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_OBJ_INV, 0,cast_detect_magic1
	     ,0)
     NSPELLO(21, 4, POSITION_STANDING, 30, INVM_GUILD, 20,TRUE,1,10, 
	     SKILL_INTELLEGO, SKILL_VIM, 0,
	     TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_OBJ_INV, 0,cast_detect_magic2
	     ,0)
     NSPELLO(22, 4, POSITION_STANDING, 40, INVM_GUILD, 20,TRUE,1,10, 
	     SKILL_INTELLEGO, SKILL_VIM, 0,
	     TAR_SELF_ONLY, 0,cast_detect_magic3 ,0)
     
     NSPELLO(91, 4, POSITION_STANDING, 20, CRIG_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_IGNORE, 0,
	     cast_lamp_wo_flame,0)
       
     NSPELLO(92, 4, POSITION_FIGHTING, 40, CRIG_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
     
     NSPELLO(93, 4, POSITION_FIGHTING, 60, CRIG_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
       
     NSPELLO(94, 6, POSITION_FIGHTING, 80, CRIG_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
     
     NSPELLO(95, 6, POSITION_FIGHTING, 100, CRIG_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_IGNORE, 0,cast_gen_dam,0)
       
   NSPELLO(96, 6, POSITION_FIGHTING, 120, CRIG_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
       
   NSPELLO(97, 8, POSITION_STANDING, 140, CRIG_GUILD, 20,TRUE,4,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_IGNORE, 3,cast_gen_dam,0)
   NSPELLO(32, 6, POSITION_STANDING, 60, MUAU_GUILD, 20,TRUE,4,10, 
	     SKILL_MUTO, SKILL_AURAM, 0,
	     TAR_IGNORE, 3,cast_gen_dam,0)
       
   NSPELLO(98, 6, POSITION_FIGHTING, 160, CRIG_GUILD, 20,TRUE,5,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
       
   NSPELLO(99, 10, POSITION_FIGHTING, 180, CRIG_GUILD, 20,TRUE,6,10, 
	     SKILL_CREO, SKILL_IGNEM, 0,
	     TAR_IGNORE, 0,cast_gen_dam,0)
   NSPELLO(23, 3, POSITION_FIGHTING, 120, RETE_GUILD, 20,TRUE,6,10, 
	     SKILL_REGO, SKILL_TERRAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
   NSPELLO(5, 2, POSITION_FIGHTING, 60, RETE_GUILD, 20,TRUE,6,10, 
	     SKILL_REGO, SKILL_TERRAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
     
   NSPELLO(30, 8, POSITION_STANDING, 20, MUTE_GUILD, 20,TRUE,1,10, 
	     SKILL_MUTO, SKILL_TERRAM, 0,
	     TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_OBJ_INV, 0,cast_enchant1,0)
     
   NSPELLO(301, 4, POSITION_STANDING, 40, CRAQ_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_OBJ_INV, 0,cast_quench_thirst,0) 
   NSPELLO(12, 4, POSITION_STANDING, 40, CRAN_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_ANIMAL, 0,
	     TAR_IGNORE, 0,cast_feast_for_five,0)
   NSPELLO(13, 4, POSITION_FIGHTING, 60, CRAN_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_ANIMAL, 0,
	     TAR_CHAR_ROOM, 0,cast_soothe_pains_ot_beast, 0)
   NSPELLO(14, 4, POSITION_FIGHTING, 80, CRAN_GUILD, 20,TRUE,4,10, 
	     SKILL_CREO, SKILL_ANIMAL, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_web,0)
   NSPELLO(47, 4, POSITION_STANDING, 40, MUAQ_GUILD, 20,TRUE,1,10, 
	     SKILL_MUTO, SKILL_AQUAM, 0,
	     TAR_SELF_ONLY , 0,cast_lungs_ot_fish,0) 
   NSPELLO(19, 4, POSITION_FIGHTING, 0, INIM_GUILD, 20,TRUE,1,10, 
	     SKILL_INTELLEGO, SKILL_IMAGONEM, 0,
	     TAR_SELF_ONLY , 0,cast_detect_invis,0)
   NSPELLO(24, 2, POSITION_FIGHTING, 0, CRIM_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_IMAGONEM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT , 0,cast_dispel_invis,0)
   NSPELLO(27, 4, POSITION_STANDING, 60, PEIM_GUILD, 20,TRUE,1,10, 
	     SKILL_PERDO, SKILL_IMAGONEM, 0,
	     TAR_SELF_ONLY , 0,cast_invis1,0)
   NSPELLO(28, 4, POSITION_STANDING, 100, PEIM_GUILD, 20,TRUE,1,10, 
	     SKILL_PERDO, SKILL_IMAGONEM, 0,
	     TAR_SELF_ONLY , 0,cast_invis2,0)
   NSPELLO(29, 4, POSITION_STANDING, 120, PEIM_GUILD, 20,TRUE,1,10, 
	     SKILL_PERDO, SKILL_IMAGONEM, 0,
	     TAR_SELF_ONLY , 0,cast_invis3,0)      
   NSPELLO(48, 4, POSITION_STANDING, 80, MUAQ_GUILD, 20,TRUE,3,10, 
	     SKILL_MUTO, SKILL_AQUAM, 0,
	     TAR_CHAR_ROOM , 0,cast_lungs_ot_fish, TEACH_MERMAN) 
   NSPELLO(302, 4, POSITION_FIGHTING, 60, CRAQ_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_footstep_slippery,0)
   NSPELLO(320, 4, POSITION_FIGHTING, 80, CRAQ_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
   NSPELLO(321, 4, POSITION_FIGHTING, 100, CRAQ_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
   NSPELLO(322, 4, POSITION_FIGHTING, 120, CRAQ_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
   NSPELLO(323, 4, POSITION_FIGHTING, 140, CRAQ_GUILD, 20,TRUE,4,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_encase_in_ice,0)
   NSPELLO(324, 4, POSITION_FIGHTING, 160, CRAQ_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0,cast_gen_dam,0)
   NSPELLO(325, 4, POSITION_FIGHTING, 180, CRAQ_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_IGNORE, 0,cast_gen_dam,0)
   NSPELLO(326, 4, POSITION_FIGHTING, 200, CRAQ_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_AQUAM, 0,
	     TAR_IGNORE, 0,cast_gen_dam,0)
   NSPELLO(330, 4, POSITION_FIGHTING, 20, CRCO_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_bind_wounds,0)
   NSPELLO(331, 4, POSITION_FIGHTING, 40, CRCO_GUILD, 20,TRUE,1,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_blacksmith_might,0)
   NSPELLO(2, 4, POSITION_STANDING, 60, CRCO_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM | TAR_SELF_NONO, 0,cast_breath_of_vigor, TEACH_ELF
	   | TEACH_HELF | TEACH_PIXIE | TEACH_GNOME)
   NSPELLO(333, 4, POSITION_FIGHTING, 80, CRCO_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_healing_touch, 0)
   NSPELLO(334, 4, POSITION_STANDING, 100, CRCO_GUILD, 20,TRUE,3,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_restoration,TEACH_GNOME | TEACH_HUMAN
	   | TEACH_OGIER)
   NSPELLO(335, 4, POSITION_FIGHTING, 120, CRCO_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_youthful_beauty,0) 
   NSPELLO(336, 4, POSITION_FIGHTING, 130, CRCO_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM , 0,cast_nimble_cat,0)
   NSPELLO(337, 4, POSITION_FIGHTING, 140, CRCO_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_SELF_ONLY, 0,cast_hearty_health,0)
   NSPELLO(339, 4, POSITION_FIGHTING, 180, CRCO_GUILD, 20,TRUE,2,10, 
	     SKILL_CREO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_body_made_whole, TEACH_GNOME | TEACH_HUMAN)
   NSPELLO(341, 1, POSITION_FIGHTING, 20, RECO_GUILD, 20,TRUE,2,10, 
	     SKILL_REGO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_spasm,TEACH_HUMAN)

   NSPELLO(342, 8, POSITION_STANDING, 140, RECO_GUILD, 20,TRUE,2,10, 
	     SKILL_REGO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_leap,0)
       
   NSPELLO(343, 4, POSITION_FIGHTING, 80, RECO_GUILD, 20,TRUE,2,10, 
	     SKILL_REGO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM | TAR_SELF_NONO, 0,cast_gift_of_vigour,
	   TEACH_GNOME | TEACH_HUMAN)
       
     NSPELLO(344, 4, POSITION_FIGHTING, 60, RECO_GUILD, 20,TRUE,2,10, 
	     SKILL_REGO, SKILL_CORPUS, 0,
	     TAR_CHAR_ROOM, 0,cast_endurance,0)

   NSPELLO(345, 4, POSITION_FIGHTING, 100, RECO_GUILD, 20,TRUE,2,10, 
	     SKILL_REGO, SKILL_CORPUS, 0,
	     TAR_OBJ_ROOM, 0,cast_walking_corpse,TEACH_GNOME | TEACH_HUMAN)
       
   NSPELLO(346, 5, POSITION_FIGHTING, 180, RECO_GUILD, 20,TRUE,3,10, 
	     SKILL_REGO, SKILL_CORPUS, 0,
	     TAR_SELF_ONLY, 0,cast_seven_league,0)
   
   NSKILLO(101, 0, POSITION_STANDING, 1, THIEF_GUILD,15,TRUE,1,0, STAT_DEX,
	  STAT_GUI,0, TAR_IGNORE, 0,
	   TEACH_ALL ^ TEACH_GIANT ^ TEACH_OGIER);   
   
   NSKILLO(102, 0, POSITION_STANDING, 1, THIEF_GUILD, 15,TRUE,1,0,
	  STAT_DEX,STAT_GUI,0,TAR_IGNORE, 0,
   	   TEACH_ALL ^ TEACH_GIANT ^ TEACH_OGIER);   
   NSKILLO(103, 0, POSITION_STANDING, 1, THIEF_GUILD, 10,TRUE,2,0,
	  STAT_DEX,STAT_GUI,0,TAR_IGNORE, 0,
      	   TEACH_ALL ^ TEACH_GIANT ^ TEACH_OGIER ^ TEACH_GNOME);   
   NSKILLO(104, 0, POSITION_STANDING, 2, THIEF_GUILD, 5,TRUE,3,40,
	  STAT_DEX,STAT_STR,STAT_GUI,TAR_IGNORE, 0,
      	   TEACH_ALL ^ TEACH_GIANT ^ TEACH_OGIER ^ TEACH_GNOME);   
   NSKILLO(105, 0, POSITION_STANDING, 5, THIEF_GUILD, 10,TRUE,3,3, 
	  STAT_DEX,0,0,TAR_IGNORE, 0,
      	   TEACH_ALL ^ TEACH_GIANT ^ TEACH_OGIER);      
   NSKILLO(106, 0, POSITION_STANDING,1, WARRIOR_GUILD, 10,TRUE,1,0,
	  STAT_DEX,STAT_STR,0,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_GNOME ^ TEACH_HALF);   
   NSKILLO(107, 0, POSITION_STANDING, 2, WARRIOR_GUILD, 10, TRUE,1,0, 
	  STAT_STR,0,STAT_DEX,TAR_IGNORE, 0,
   	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_GNOME ^ TEACH_HALF);   
   NSKILLO(108, 0, POSITION_STANDING, 5, WARRIOR_GUILD, 10,TRUE,2,3, 
	  STAT_DEX,STAT_STR,0,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_GNOME ^ TEACH_HALF);   
   NSKILLO(109, 0, POSITION_STANDING, 5, THIEF_GUILD, 10,TRUE,2,3, 
	  STAT_DEX,STAT_STR,STAT_FOC,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_GIANT ^ TEACH_OGIER);   
   NSKILLO(135, 0, POSITION_STANDING, 10, CLERIC_GUILD, 30,TRUE,6,3, 
	  STAT_WIS,STAT_FOC,0,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_ELF ^ TEACH_HELF);
   NSKILLO(136, 0, POSITION_STANDING, 25, CLERIC_GUILD, 30,TRUE,6,3, 
	  STAT_WIS,STAT_FOC,0,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_ELF ^ TEACH_HELF);      
   NSKILLO(111, 0, POSITION_STANDING, 10, WARRIOR_GUILD, 10,TRUE,2,3, 
	  STAT_STR,STAT_DEX,STAT_STR,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_GNOME ^ TEACH_GIANT);
   /* Ars magica skills */
   NSKILLO(112, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_WIS,TAR_IGNORE, 0, 0);  
   NSKILLO(113, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_WIS,TAR_IGNORE, 0, 0);   
   NSKILLO(114, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_WIS,TAR_IGNORE, 0, 0);   
   NSKILLO(115, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_WIS,TAR_IGNORE, 0, 0);   
   NSKILLO(116, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_WIS,STAT_FOC,STAT_INT,TAR_IGNORE, 0, 0);   
   NSKILLO(117, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_WIS,STAT_FOC,0,TAR_IGNORE, 0,
	   TEACH_ALL ^ TEACH_DWARF  ^ TEACH_GNOME );   
   NSKILLO(118, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,0,TAR_IGNORE,0, TEACH_MERMAN | TEACH_HUMAN);   
   NSKILLO(119, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,0,TAR_IGNORE,0, TEACH_PIXIE | TEACH_HELF
	   | TEACH_ELF | TEACH_HUMAN );   
   NSKILLO(120, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_WIS,STAT_FOC,0,TAR_IGNORE, 0, TEACH_GNOME | TEACH_HUMAN);   
   NSKILLO(121, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_WIS,STAT_FOC,0,TAR_IGNORE, 0,
	   TEACH_GNOME | TEACH_ELF | TEACH_HUMAN | TEACH_HELF | TEACH_OGIER);   
   NSKILLO(122, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,0,TAR_IGNORE, 0,
	   TEACH_HUMAN | TEACH_DWARF);   
   NSKILLO(123, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_WIS,TAR_IGNORE, 0,
	   TEACH_HUMAN | TEACH_OGIER | TEACH_PIXIE);   
   NSKILLO(124, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_INT,TAR_IGNORE, 0,
	   TEACH_GNOME | TEACH_HUMAN | TEACH_OGIER);   
   NSKILLO(125, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_STR,TAR_IGNORE, 0,
	   TEACH_DWARF | TEACH_HUMAN);   
   NSKILLO(126, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,3,15, 
	  STAT_INT,STAT_FOC,STAT_FOC,TAR_IGNORE, 0,
	   TEACH_HUMAN | TEACH_PIXIE | TEACH_ELF | TEACH_HELF);   
   NSKILLO(127, 0, POSITION_STANDING, 1, ARSMAGICA_GUILD, 15,TRUE,5,10, 
	  STAT_INT,STAT_FOC,STAT_INT,TAR_IGNORE, 0,TEACH_ALL ^ TEACH_GIANT);
   NSKILLO(128, 0, POSITION_STANDING, 20, WARRIOR_GUILD, 10,TRUE,2,3, 
	  STAT_DEX,STAT_DEX,STAT_STR,TAR_IGNORE, 0,TEACH_ALL ^ TEACH_PIXIE ^
	   TEACH_GNOME ^ TEACH_DWARF ^ TEACH_HALF ^ TEACH_GIANT);
   NSKILLO(129, 0, POSITION_STANDING, 1, RIDING_GUILD, 10,TRUE,2,3, 
	  STAT_DEX,STAT_DEX,0,TAR_IGNORE, 0, TEACH_ALL )
   NSKILLO(130, 0, POSITION_STANDING, 1, RIDING_GUILD, 10,TRUE,2,3, 
	  STAT_DEX,STAT_DEX,0,TAR_IGNORE, 0, TEACH_ALL )     
   NSKILLO(131, 0, POSITION_STANDING, 40, RANGER_GUILD, 10,TRUE,2,3, 
	  STAT_PER,STAT_WIS,0,TAR_IGNORE, 0, TEACH_ALL )
   NSKILLO(132, 0, POSITION_STANDING, 60, RANGER_GUILD, 10,TRUE,2,3, 
	  STAT_CHR,STAT_PER,0,TAR_IGNORE, 0, TEACH_ALL )        
   NSKILLO(133, 0, POSITION_STANDING,1, WARRIOR_GUILD, 10,TRUE,2,0,
	  STAT_DEX,STAT_STR,0,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_GNOME ^ TEACH_HALF);
   NSKILLO(134, 0, POSITION_STANDING,20, WARRIOR_GUILD, 10,TRUE,5,0,
	  STAT_DEX,STAT_DEX,STAT_STR,TAR_IGNORE, 0,
	  TEACH_ALL ^ TEACH_PIXIE ^ TEACH_GNOME ^ TEACH_HALF);      
   /* weapon profs */
   NSKILLO(201, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,1,3, 
	  STAT_STR,0,0,TAR_IGNORE, 0,0);
   NSKILLO(202, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,2,3, 
	  STAT_STR,STAT_DEX,0,TAR_IGNORE, 0,0)
   NSKILLO(203,0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,1,3, 
	  STAT_DEX,0,STAT_FOC,TAR_IGNORE, 0,
	   TEACH_ALL ^ TEACH_GIANT ^ TEACH_OGIER);   
   NSKILLO(204, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,1,3, 
	  STAT_STR,0,STAT_DEV,TAR_IGNORE, 0,0);            
   NSKILLO(205, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,2,3, 
	  STAT_STR,STAT_DEV,STAT_DEX,TAR_IGNORE, 0,0);
   NSKILLO(206, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,1,3, 
	  STAT_STR,0,STAT_DEV,TAR_IGNORE, 0,0);         
   NSKILLO(207, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,2,3, 
	  STAT_STR,STAT_DEV,STAT_DEX,TAR_IGNORE, 0,0);
   NSKILLO(208, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,1,3, 
	  STAT_STR,STAT_DEX,0,TAR_IGNORE, 0,0);
   NSKILLO(209, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0, TRUE,2,3, 
	  STAT_STR,STAT_DEX,STAT_STR,TAR_IGNORE, 0,0);
   NSKILLO(210, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,2,3, 
	  STAT_STR,STAT_DEX,0,TAR_IGNORE, 0,0);   
   NSKILLO(211, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,1,3, 
	  STAT_STR,STAT_DEX,0,TAR_IGNORE, 0,0);
   NSKILLO(212, 0, POSITION_STANDING, 1, WARRIOR_GUILD, 0,TRUE,1,3, 
	  STAT_STR,STAT_DEX,0,TAR_IGNORE, 0,0);      
}









