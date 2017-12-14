/* ************************************************************************
 *   File: guilds.c                                   Part of Archipelago  *
 *  Usage: implementation of special procedures for guilds                 *
 *                                                                         *
 *  Archipelago is based on                                                *
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 *  Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 * 
 ************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"


/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct spell_info_type spell_info[];
extern struct list_index_type CrIg[];
extern struct list_index_type wizened[];
extern struct list_index_type archivist[];
extern struct list_index_type clio[];
extern struct list_index_type Water[];
extern struct list_index_type CrCo[];
extern struct list_index_type ReCo[];
extern struct list_index_type Mason_spells[];
extern struct list_index_type MuCo[];
extern struct list_index_type PeCo[];
extern struct list_index_type PeIm[];
extern struct list_index_type InIm[];
extern struct list_index_type Thin_spells[];
extern struct list_index_type Stout_spells[];
extern struct list_index_type Animal[];
extern struct list_index_type am_skills[];
extern struct list_index_type Cleric_skills[];
extern struct list_index_type Warrior_skills[];
extern struct list_index_type Thief_skills[];
extern struct list_index_type Hasad_skills[];
extern struct list_index_type Horse_skills[];
extern struct list_index_type Ranger_skills[];
extern struct list_index_type Weapons[];
int	add_follower(struct char_data *ch, struct char_data *leader);
int     calc_difficulty(struct char_data *ch, int number);
int     calc_prof_difficulty(struct char_data *ch, int number);
/*int     calc_max_ability(struct char_data *ch, int number); */
void check_autowiz(struct char_data *ch);
struct social_type {
  char	*cmd;
  int	next_line;
};


/* ********************************************************************
 *  Special procedures for rooms                                       *
 ******************************************************************** */
int     get_stat(struct char_data *ch, int stat_index)
{

  switch(stat_index){
  case 0:
    return 20;
  case 1:
    return GET_RAW_STR(ch);
  case 2:
    return GET_RAW_INT(ch);
  case 3:
    return GET_RAW_WIS(ch);
  case 4:
    return GET_RAW_CON(ch);
  case 5:
    return GET_RAW_DEX(ch);
  case 6:
    return GET_RAW_CHR(ch);    
  case 7:
    return GET_RAW_PER(ch);    
  case 8:
    return GET_RAW_GUI(ch);    
  case 9:
    return GET_RAW_LUC(ch);
  case 10:
    return GET_RAW_FOC(ch);
  case 11:
    return GET_RAW_DEV(ch);    	 
  default:
    return 20;
  }
}

int     calc_difficulty(struct char_data *ch, int number)
{
  int stat1,stat2,stat3,stat4;
  int tmp;


  stat1 = spell_info[number].key_stats;
  stat2 = spell_info[number].sec_stats;
  stat3 = spell_info[number].ter_stats;

  if(!(spell_info[number].spll_pointer)){
    if (stat1 !=0 && stat2 == 0 && stat3 ==0 )
      return (20*spell_info[number].difficulty)/get_stat(ch,stat1);
    if (stat1 !=0 && stat2 != 0 && stat3 ==0 ){
      stat4 = MAX(1,3*get_stat(ch,stat1)/4 + get_stat(ch,stat2)/4);
      return (20*spell_info[number].difficulty)/stat4;}
    if (stat1 !=0 && stat2 == 0 && stat3 !=0 ){
      stat4 = MAX(1,5*get_stat(ch,stat1)/6 + get_stat(ch,stat3)/6);
      return (20*spell_info[number].difficulty)/stat4;}
    if (stat1 !=0 && stat2 != 0 && stat3 !=0 ){
        stat4 = spell_info[number].difficulty;
        tmp = 8 - get_stat(ch,stat1)/3;
        tmp = MAX(0, tmp);
        stat4 += tmp;
        tmp = (8 - get_stat(ch,stat2)/3)/2;
        tmp = MAX(0, tmp);
        stat4 += tmp;
        tmp = (8 - get_stat(ch,stat3)/3)/4;
        tmp = MAX(0, tmp);
        stat4 += tmp;
        stat4 = MAX(spell_info[number].difficulty - 1, stat4);
      return( stat4);
}

    return spell_info[number].difficulty;
  }
  else{
    stat4 = GET_SKILL(ch,SKILL_MAGICTHEORY) + GET_RAW_INT(ch)
      + 3*stat1 + 3*stat4;
    if (stat4)
      return(MAX(1,(spell_info[number].difficulty*
		    spell_info[number].min_level)/stat4));
	 
    return spell_info[number].difficulty;
  }
}

int     calc_prof_difficulty(struct char_data *ch, int number)
{
  int stat1,stat2,stat3,stat4;

  stat1 = spell_info[number].key_stats;
  stat2 = spell_info[number].sec_stats;
  stat3 = spell_info[number].ter_stats;

  if (stat1 !=0 && stat2 == 0 && stat3 ==0 )
    return (20*spell_info[number].difficulty)/get_stat(ch,stat1);
  if (stat1 !=0 && stat2 != 0 && stat3 ==0 ){
    stat4 = MAX(1,3*get_stat(ch,stat1)/4 + get_stat(ch,stat2)/4);
    return (20*spell_info[number].difficulty)/stat4;}
  if (stat1 !=0 && stat2 == 0 && stat3 !=0 ){
    stat4 = MAX(1,5*get_stat(ch,stat1)/6 + get_stat(ch,stat3)/6);
    return (20*spell_info[number].difficulty)/stat4;}
  if (stat1 !=0 && stat2 != 0 && stat3 !=0 ){
    stat4 = MAX(1,2*get_stat(ch,stat1)/3 + 2*get_stat(ch,stat2)/9 
      + get_stat(ch,stat3)/9);
    return (20*spell_info[number].difficulty)/stat4;}

  return spell_info[number].difficulty;
}



SPECIAL(guild)
{
  ACMD(do_say);
  int number, npracs, mpracs,i, percent,diff;
  long guild_no = 0 ;
  int eff_lev, pc_race, race_mult=15, max_lev;
  struct char_data *guildmaster;
  char str0[100],str1[100],str2[100],str3[100];
  struct list_index_type *this_list;
  
  bool no_pointer = FALSE, found = FALSE;
  /*
    164 = study
    170 = learn
    309 = train
    */
  /* find a guildmaster */
  for (guildmaster = world[ch->in_room].people;
       guildmaster;
       guildmaster= guildmaster->next_in_room)
    if (mob_index[guildmaster->nr].func == guild)
      break;
  if(!guildmaster)
    return(FALSE);
     
     
  if (IS_NPC(ch) || ((cmd != 164) && (cmd != 170) && (cmd != 302) && (cmd != 309)))
    return(FALSE);
     
  pc_race = (1 << GET_RACE(ch));     
  if (cmd == 164 || cmd == 170){
    if (cmd == 164){
      strcpy(str0,"You have got %d study sessions left.\r\nYou can study any of these spells:\r\n");
      sprintf(str1,"%-36s      %-5s %-10s %-10s %-10s\r\n","Spell Name","Level","Difficulty","How Well","Max Guild.");
      strcpy(str2,"----------------------------------------- ----- ---------- ---------- ----------\r\n");
      strcpy(str3,"You do not know of this spell...\r\n");
      if (mob_index[guildmaster->nr].virtual == 1219){
	SET_BIT(guild_no, (1 << CRIG_GUILD));
	this_list = CrIg;
      }
      else if (mob_index[guildmaster->nr].virtual == 1221){
	SET_BIT(guild_no, (1 << CRAQ_GUILD));
	SET_BIT(guild_no, (1 << MUAQ_GUILD));		
	this_list = Water;
      }
      else if (mob_index[guildmaster->nr].virtual == 2011){
	SET_BIT(guild_no, (1 << CRCO_GUILD));
	SET_BIT(guild_no, (1 << CRIG_GUILD));	
	SET_BIT(guild_no, (1 << MUAQ_GUILD));		
	this_list = wizened;
      }
      else if (mob_index[guildmaster->nr].virtual == 1206){
	SET_BIT(guild_no, (1 << CRCO_GUILD));
	SET_BIT(guild_no, (1 << MUCO_GUILD));
	SET_BIT(guild_no, (1 << RECO_GUILD));			
	this_list = clio;
      }
      else if (mob_index[guildmaster->nr].virtual == 1222){
	SET_BIT(guild_no, (1 << CRCO_GUILD));	
	this_list = CrCo;
      }
      else if (mob_index[guildmaster->nr].virtual == 1223){
	SET_BIT(guild_no, (1 << RECO_GUILD));
	SET_BIT(guild_no, (1 << MUCO_GUILD));
	SET_BIT(guild_no, (1 << PECO_GUILD));				
	this_list = Stout_spells;
      }
      else if (mob_index[guildmaster->nr].virtual == 2067){
	SET_BIT(guild_no, (1 << MUCO_GUILD));
	SET_BIT(guild_no, (1 << PEIM_GUILD));
	SET_BIT(guild_no, (1 << INIM_GUILD));
	SET_BIT(guild_no, (1 << INVM_GUILD)); 		
	SET_BIT(guild_no, (1 << CRIM_GUILD));
	SET_BIT(guild_no, (1 << MUAU_GUILD)); 			
	this_list = Thin_spells;
      }
      else if (mob_index[guildmaster->nr].virtual == 2083){
	SET_BIT(guild_no, (1 << INVM_GUILD));
	SET_BIT(guild_no, (1 << INIM_GUILD)); 			
	SET_BIT(guild_no, (1 << CRIM_GUILD)); 		
	this_list = archivist;
      }
      else if (mob_index[guildmaster->nr].virtual == 2080){
	SET_BIT(guild_no, (1 << RETE_GUILD));
	SET_BIT(guild_no, (1 << MUTE_GUILD));				
	this_list = Mason_spells;
      }
      else
	return(FALSE);
    }
    else if (cmd == 170){
      strcpy(str0,"You have got %d study sessions left.\r\nYou can practice any of these skills:\r\n");
      sprintf(str1,"%-36s %-10s %-10s %-10s\r\n","Skill Name","Difficulty","How Well","Max Guild.");
      strcpy(str2,"----------------------------------   ---------- ---------- ------------\r\n");
      strcpy(str3,"You do not know of this skill...\r\n");
      if ((mob_index[guildmaster->nr].virtual == 2014)){
	SET_BIT(guild_no, (1 << WARRIOR_GUILD));		
	this_list = Warrior_skills;
      }
      else if (mob_index[guildmaster->nr].virtual == 2070){
	SET_BIT(guild_no, (1 << RIDING_GUILD));			
	this_list = Horse_skills;
      }
      else if ((mob_index[guildmaster->nr].virtual == 2012)){
	SET_BIT(guild_no, (1 << CLERIC_GUILD));	
	this_list = Cleric_skills;
      } 
      else if (mob_index[guildmaster->nr].virtual == 2013){
	SET_BIT(guild_no, (1 << THIEF_GUILD));	 	
	this_list = Thief_skills;
      }
      else{
	this_list = am_skills;
	SET_BIT(guild_no, (1 << ARSMAGICA_GUILD));	 		
	no_pointer = TRUE;
      }
    }
  }
  else if ((cmd == 309) &&((mob_index[guildmaster->nr].virtual== 2014))){
    strcpy(str0,"You have got %d training sessions left.\r\nYou can train any of the following weapons:\r\n");
    sprintf(str1,"%-36s %-10s %-10s %-10s\r\n","Weapon Name","Difficulty","How Well","Max Guild.");
    strcpy(str2,"----------------------------------   ---------- ---------- ------------\r\n");
    strcpy(str3,"You do not know of this weapon...\r\n");
    SET_BIT(guild_no, (1 << WARRIOR_GUILD));		    
    this_list = Weapons;
  }
  else
    return(FALSE);
  for (; *arg == ' '; arg++)
    ;
  if (!*arg) {
    found = FALSE;
    for (i = 0; *(this_list[i].entry) != '\n';i++){
      diff = calc_difficulty(ch,this_list[i].index ) + 1;
      if (!spell_info[this_list[i].index].use_bas_lev)
	eff_lev = (ubyte) spell_info[this_list[i].index].min_level*diff;
      else
	eff_lev = (ubyte) spell_info[this_list[i].index].min_level;
      if (cmd == 164){
	if ((!spell_info[this_list[i].index].race_flag ||
	     IS_SET(spell_info[this_list[i].index].race_flag, pc_race)) &&
	    (spell_info[this_list[i].index].min_level
	     <= (GET_SKILL(ch, SKILL_MAGICTHEORY)+GET_RAW_INT(ch)
		 + 3*GET_SKILL(ch, spell_info[this_list[i].index].key_stats) +
		 3*GET_SKILL(ch, spell_info[this_list[i].index].sec_stats))) &&
	    IS_SET(guild_no,(1<< spell_info[this_list[i].index].which_guild)))
	  {
	    found = TRUE;
	    break;}		     
      }
      else
	{
	  if (eff_lev <= GET_LEVEL(ch) &&
	      IS_SET(guild_no,(1<< spell_info[this_list[i].index].which_guild))) {
	    found = TRUE;
	    break;
	  }
	}
    }
    if (!found){
      do_say(guildmaster,"I cannot instruct you in this.",0,0);
      do_say(guildmaster,"You are not ready for my lore.",0,0); 
      return(TRUE);
    }
    sprintf(buf,str0 ,SPELLS_TO_LEARN(ch));
    sprintf(buf2,str1);
    strcat(buf,buf2);
    sprintf(buf2,str2);
    strcat(buf,buf2);
    if (cmd == 164) {
      for (i = 0; *(this_list[i].entry) != '\n'; i++){
	diff = calc_difficulty(ch,this_list[i].index ) + 1;	
	eff_lev = (GET_SKILL(ch, SKILL_MAGICTHEORY) +
		   GET_RAW_INT(ch) + 
		   3*GET_SKILL(ch, spell_info[this_list[i].index].key_stats) +
		   3*GET_SKILL(ch, spell_info[this_list[i].index].sec_stats));
	if (spell_info[this_list[i].index].race_flag &&
	    IS_SET(spell_info[this_list[i].index].race_flag, pc_race))
	  race_mult = 15;
	else
	  race_mult = 10;
	max_lev = (race_mult*spell_info[this_list[i].index].max_level)/10
	  + (GET_RAW_CHR(ch) -15)/3;
	max_lev = MIN(30,max_lev);
	if ((!spell_info[this_list[i].index].race_flag ||
	     IS_SET(spell_info[this_list[i].index].race_flag, pc_race)) &&
	    (eff_lev >= spell_info[this_list[i].index].min_level) &&
	    IS_SET(guild_no,(1<< spell_info[this_list[i].index].which_guild)))
	  {
	  sprintf(buf2, "%-36s %4s %-5d %-10d %-10d %-10d\r\n",
		  this_list[i].entry,
		  this_list[i].components,
		  spell_info[this_list[i].index].min_level,		  
		  diff,
		  GET_SKILL(ch, this_list[i].index),
		  ((spell_info[this_list[i].index].max_level)
		   ? max_lev : 30));
	  strcat(buf, buf2);
	}
      }
    }
    else {
      for (i = 0; *(this_list[i].entry) != '\n'; i++){
	diff = calc_difficulty(ch,this_list[i].index) + 1;
	if (spell_info[this_list[i].index].race_flag &&
	    !IS_SET(spell_info[this_list[i].index].race_flag,pc_race)) {
	  diff *= 2;
	  race_mult = 10;
	}
	else
	  race_mult = 15;
	max_lev = (race_mult*spell_info[this_list[i].index].max_level)/10
	  + (GET_RAW_CHR(ch) -15)/3;
	max_lev = MIN(30,max_lev);	
	if (!spell_info[this_list[i].index].use_bas_lev)
	  eff_lev = (ubyte) spell_info[this_list[i].index].min_level*diff;
	else
	  eff_lev =(ubyte) spell_info[this_list[i].index].min_level;
	if ((eff_lev <= GET_LEVEL(ch)) && 
	    IS_SET(guild_no,(1<< spell_info[this_list[i].index].which_guild)))
	  {
	    sprintf(buf2, "%-37s %-10d %-10d %-10d\r\n", this_list[i].entry,
		    diff,
		    GET_SKILL(ch, this_list[i].index),
		    ((spell_info[this_list[i].index].max_level)
		     ? max_lev :30));  
	    strcat(buf, buf2);
	  }
      }
    }
    send_to_char(buf, ch);
    return(TRUE); 
  }
  half_chop(arg,buf1,buf2);
  if (!is_number(buf1)) {
    npracs = 1;
    strcpy(buf2, arg);
  }
  else
    npracs = atoi(buf1);
  if (!*buf2){
    send_to_char("What was that?\r\n",ch);
    return(TRUE);}
  number = search_list(buf2, this_list, FALSE);
  if (number == -1) {
    send_to_char(str3, ch);
    return(TRUE);
  }
  if (number >= 0 &&
      !(IS_SET(guild_no,(1<< spell_info[number].which_guild))))
    {
      send_to_char("Can you be a bit more specific?\r\n",ch);
      return(TRUE);
    }
  diff = calc_difficulty(ch,number) +1;
  if (cmd == 164){
    eff_lev = (GET_SKILL(ch, SKILL_MAGICTHEORY) + GET_RAW_INT(ch) +
	       3*GET_SKILL(ch, spell_info[number].key_stats) +
	       3*GET_SKILL(ch, spell_info[number].sec_stats));
    if ( (!spell_info[number].race_flag ||
	  IS_SET(spell_info[number].race_flag, pc_race)) && 
	 (eff_lev < spell_info[number].min_level) &&
	 (IS_SET(guild_no,(1<< spell_info[number].which_guild))))
      {
	send_to_char(str3, ch);
	return(TRUE);
      }
  }
  else{
    if (spell_info[number].race_flag &&
	!IS_SET(spell_info[number].race_flag, pc_race))
      diff *= 2;
    if (!spell_info[number].use_bas_lev)
      eff_lev = (ubyte) spell_info[number].min_level*diff;
    else
      eff_lev = (ubyte) spell_info[number].min_level;
    if ((GET_LEVEL(ch) < eff_lev) &&
	(IS_SET(guild_no,(1<< spell_info[number].which_guild))))
      {
	send_to_char(str3, ch);
	return(TRUE);
      }
  }
  if (SPELLS_TO_LEARN(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    return(TRUE);
  }
  if (spell_info[number].race_flag &&
      !IS_SET(spell_info[number].race_flag,pc_race)) 
    race_mult = 10;
  else
    race_mult = 15;
  max_lev =(race_mult*spell_info[number].max_level)/10+(GET_RAW_CHR(ch) -15)/3;
  max_lev = MIN(30,max_lev);  
  if (spell_info[number].max_level &&
      (GET_SKILL(ch, number) >= max_lev))
    {
      send_to_char("I'm sorry I cannot instruct you further.\r\n", ch);
      return(TRUE);
    }	     
  if (GET_SKILL(ch, number) >= 30) {
    send_to_char("You are already learned in this area.\r\n", ch);
    return(TRUE);
  }

  while (npracs && GET_SKILL(ch, number) < 30) {
    if (SPELLS_TO_LEARN(ch) <  (diff*GET_SKILL(ch,number) +1 )) {
      send_to_char("Not enough practices to get to the next level.\r\n",ch);
      return(TRUE);}      
    if (spell_info[number].max_level &&
	(GET_SKILL(ch, number) >= max_lev))
      break;
    percent = GET_SKILL(ch,number) + 1;
    SET_SKILL(ch,number, percent);
    SPELLS_TO_LEARN(ch) -= diff*GET_SKILL(ch,number);
    npracs--;
  }
	 
  send_to_char("You Practice for a while...\r\n", ch);	 

  if (GET_SKILL(ch, number) >= 30) {
    send_to_char("You are now learned in this area.\r\n", ch);
    return(TRUE);
  }
  return(TRUE);
}











