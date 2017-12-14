/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
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
#include <assert.h>
#include <math.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "screen.h"

/* Structures */

struct char_data *combat_list = 0;   /* head of l-list of fighting chars   */
struct char_data *combat_next_dude = 0; /* Next dude global trick         */


/* External structures */

extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int	pk_allowed;	/* see config.c */
extern int	auto_save;	/* see config.c */
extern int      pulse;
extern struct dual_list_type attack_hit_text[];
extern struct   index_data *mob_index;
extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
/* External procedures */
void poison(struct char_data *ch, struct char_data *victim, struct obj_data *obj);
char	*fread_string(FILE *fl, char *error);
void	stop_follower(struct char_data *ch);
ACMD(do_flee);
void	hit(struct char_data *ch, struct char_data *victim, int type);
void	forget(struct char_data *victim);
void	remember(struct char_data *ch, struct char_data *victim);
void	make_scraps(struct obj_data *obj, int r_num);
void    scrap_item(struct obj_data *obj);
void	loose_level(struct char_data *ch);
bool   is_two_handed(struct char_data *ch, struct obj_data *weapon);
int     spell_lev(struct char_data *caster, int spell);
void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
void botch_kick(struct char_data *ch, struct char_data *vict);
int stress_die(void);
int get_prof(struct char_data *ch, int *w_type, int type);
void damage_obj_corpse(struct obj_data *corpse, int spell_type, int dam);
void spell_damage_equipment(struct char_data *ch, struct char_data *vict, int spell_no, int dam);
int condition_bonus(struct char_data *ch);
int encumberance_level(struct char_data *ch);
int combat_bonus(struct char_data *ch);
int fatigue_bonus(struct char_data *ch);
int bodylevel_bonus(struct char_data *ch);
int compute_multi_attacks(struct char_data *ch, struct obj_data *weilded, int type);
struct obj_data 	*die(struct char_data *ch);
/* Weapon attack texts */

/* The Fight related routines */
int combat_bonus(struct char_data *ch)
{
  int level=0;
  
  level -= (3*encumberance_level(ch))/4;
  level += fatigue_bonus(ch);
  level += bodylevel_bonus(ch);
  level += condition_bonus(ch);
  
  return(level);
}
int encumberance_level(struct char_data *ch)
{
  int weight_worn=0,i, level=0, items_worn = 0;
  
  for (i=0; i < MAX_WEAR; i++)
    if (ch->equipment[i]){
      weight_worn += ch->equipment[i]->obj_flags.weight;
      items_worn++;
    }
  weight_worn /= 2;
  items_worn /= 3;
  if ((IS_CARRYING_W(ch) + weight_worn) > (4*CAN_CARRY_W(ch))/5)
    level = 8;
  else if ((IS_CARRYING_W(ch) + weight_worn) > (3*CAN_CARRY_W(ch))/5)
    level = 6;
  else if ((IS_CARRYING_W(ch) + weight_worn) > (2*CAN_CARRY_W(ch))/5)
    level = 4;
  else if ((IS_CARRYING_W(ch) + weight_worn) > (CAN_CARRY_W(ch))/5)
    level = 2;
  if ((IS_CARRYING_N(ch) + items_worn) > (4*CAN_CARRY_N(ch))/5)
    level += 4;
  else if ((IS_CARRYING_N(ch) + items_worn) > (3*CAN_CARRY_N(ch))/5)
    level += 3;
  else if ((IS_CARRYING_N(ch) + items_worn) > (2*CAN_CARRY_N(ch))/5)
    level += 2;  
  else if ((IS_CARRYING_N(ch) + items_worn) > (CAN_CARRY_N(ch))/5)
    level += 1;
  
  level /= 2;
  return(level);
}
int condition_bonus(struct char_data *ch)
{
  int level=0;
  
  if (GET_COND(ch,DRUNK) > 19)
    level = -10;
  else   if (GET_COND(ch,DRUNK) > 14)
    level = -7;
  else   if (GET_COND(ch,DRUNK) > 10)
    level = -4;
  else   if (GET_COND(ch,DRUNK) > 5)
    level = -2;
  else   if (GET_COND(ch,DRUNK) > 0)
    level = -1;

  if (GET_COND(ch,THIRST) == 0)
    level -= 1;
  if (GET_COND(ch,FULL) == 0)
    level -= 2;
  
  return(level);
}
int fatigue_bonus(struct char_data *ch)
{

  int max;

  max = GET_MAX_MOVE(ch);

  if (GET_MOVE(ch) > (3*max)/5)
    return(0);
  else if (GET_MOVE(ch) > (2*max)/5)
    return(-1);
  else if (GET_MOVE(ch) > (max)/5)
    return(-3);
  else
    return(-5);
  
}
int bodylevel_bonus(struct char_data *ch)
{

  int max;

  max = GET_MAX_HIT(ch);

  if (GET_HIT(ch) > (3*max)/5)
    return(0);
  else if (GET_HIT(ch) > (2*max)/5)
    return(-1);
  else if (GET_HIT(ch) > (max)/5)
    return(-3);
  else
    return(-5);
  
}
void scrap_item(struct obj_data *obj)
{
    struct obj_data *ot;

    act("$p falls to the ground in scraps!",FALSE,0,obj,0,TO_ROOM);

    if(obj->obj_flags.type_flag == ITEM_CONTAINER &&
       obj->contains){
      
	send_to_room("It's contents fall on the ground.\r\n",obj->in_room,FALSE);
	for(ot = obj->contains;ot;ot = obj->contains){
	  obj_from_obj(ot);
	  obj_to_room(ot,obj->in_room, FALSE);
	}
    }
    make_scraps(obj,obj->in_room);
    extract_obj(obj,0);
}

void	appear(struct char_data *ch)
{
   act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);

   if (affected_by_spell(ch, SPELL_INVIS3))
      affect_from_char(ch, SPELL_INVIS3);

   REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}



void	load_messages(void)
{
   FILE * f1;
   struct message_type *messages;
   int	i, type;
   char	chk[100];

   if (!(f1 = fopen(MESS_FILE, "r"))) {
      sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
      perror(buf2);
      exit(0);
   }

   for (i = 0; i < MAX_MESSAGES; i++) {
      fight_messages[i].a_type = 0;
      fight_messages[i].number_of_attacks = 0;
      fight_messages[i].msg = 0;
   }

   fscanf(f1, " %s \n", chk);

   while (*chk == 'M') {
       fscanf(f1, " %d\n", &type);
      for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) && 
          (fight_messages[i].a_type); i++)
	 ;
      if (i >= MAX_MESSAGES) {
	 logg("SYSERR: Too many combat messages.");
	 exit(0);
      }
       
       CREATE(messages, struct message_type, 1);
       fight_messages[i].number_of_attacks++;
       fight_messages[i].a_type = type;
       messages->next = fight_messages[i].msg;
       fight_messages[i].msg = messages;
       sprintf(buf2, "combat message #%d in file '%s'", i, MESS_FILE);
       
       messages->die_msg.attacker_msg      = fread_string(f1, buf2);
       messages->die_msg.victim_msg        = fread_string(f1, buf2);
       messages->die_msg.room_msg          = fread_string(f1, buf2);
       messages->miss_msg.attacker_msg     = fread_string(f1, buf2);
       messages->miss_msg.victim_msg       = fread_string(f1, buf2);
       messages->miss_msg.room_msg         = fread_string(f1, buf2);
       messages->hit_msg.attacker_msg      = fread_string(f1, buf2);
       messages->hit_msg.victim_msg        = fread_string(f1, buf2);
       messages->hit_msg.room_msg          = fread_string(f1, buf2);
       messages->god_msg.attacker_msg      = fread_string(f1, buf2);
       messages->god_msg.victim_msg        = fread_string(f1, buf2);
       messages->god_msg.room_msg          = fread_string(f1, buf2);
       fscanf(f1, " %s \n", chk);
       
   }

   fclose(f1);
}


void	update_pos( struct char_data *victim )
{

   if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED))
      return;
   else if (GET_HIT(victim) <= -11)
      GET_POS(victim) = POSITION_DEAD;
   else if (GET_HIT(victim) <= -6)
      GET_POS(victim) = POSITION_MORTALLYW;
   else if (GET_HIT(victim) <= -3)
      GET_POS(victim) = POSITION_INCAP;
   else if (GET_HIT(victim) <= 0)
      GET_POS(victim) = POSITION_STUNNED;
   else
      GET_POS(victim) = POSITION_RESTING;   

}


void	check_killer(struct char_data *ch, struct char_data *vict)
{
   if (!PLR_FLAGGED(vict, PLR_KILLER) && !PLR_FLAGGED(vict, PLR_THIEF)
        && !PLR_FLAGGED(ch, PLR_KILLER) && !IS_NPC(ch) && !IS_NPC(vict) && 
       (ch != vict)) {

/* don't auto set killer flag
      SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
      sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.", 
      GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name); 
      mudlog(buf, BRF, LEVEL_BUILDER, TRUE);
      send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n", ch);  */
   }
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void	set_fighting(struct char_data *ch, struct char_data *vict)
{
   if (ch == vict)
      return;

   assert(!ch->specials.fighting);

   ch->next_fighting = combat_list;
   combat_list = ch;

   ch->specials.fighting = vict;

   if (!pk_allowed)
      check_killer(ch, vict);
}



/* remove a char from the list of fighting chars */
void	stop_fighting(struct char_data *ch)
{
   struct char_data *tmp;

   assert(ch->specials.fighting);

   if (ch == combat_next_dude)
      combat_next_dude = ch->next_fighting;

   if (combat_list == ch)
      combat_list = ch->next_fighting;
   else {
      for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
          tmp = tmp->next_fighting)
	 ;
      if (!tmp) {
	 logg("SYSERR: Char fighting not found Error (fight.c, stop_fighting)");
	 abort();
      }
      tmp->next_fighting = ch->next_fighting;
   }

   ch->next_fighting = 0;
   ch->specials.fighting = 0;
   update_pos(ch);
}



struct obj_data 	*make_corpse(struct char_data *ch)
{
   struct obj_data *corpse, *o;
   struct obj_data *money;
   int	i;
   bool undead = FALSE;
   extern int max_npc_corpse_time, max_pc_corpse_time;


   struct obj_data *create_money(int amount);
   
   
   corpse = read_object(1274, VIRTUAL, 1);
   corpse->obj_flags.value[5] = GET_SIZE(ch);
   corpse->obj_flags.value[4] = GET_LEVEL(ch);   
   if (IS_UNDEAD(ch))
       undead = TRUE;
   corpse->obj_flags.value[2] = GET_IDNUM(ch);
   if (IS_NPC(ch))
     corpse->obj_flags.value[7] = mob_index[ch->nr].virtual;
   corpse->in_room = NOWHERE;
   if (undead)
       sprintf(buf2,"dust pile");
   else
       sprintf(buf2,"corpse %s",GET_NAME(ch));
   corpse->name = str_dup(buf2);
   if (undead)
       sprintf(buf2, "A pile of dust is lying here.");
   else
       sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
   corpse->description = str_dup(buf2);
   if (undead)
       sprintf(buf2, "a pile of dust");
   else
       sprintf(buf2, "the corpse of %s", GET_NAME(ch));
   corpse->short_description = str_dup(buf2);
   /*   if (!undead){
     sprintf(buf2, "The stench rising from the corpse makes your gorge rise.");
     corpse->action_description = str_dup(buf2);
   }*/
   corpse->contains = ch->inventory;
   if (GET_GOLD(ch) > 0) {
      /* following 'if' clause added to fix gold duplication loophole */
      if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
	 money = create_money(GET_GOLD(ch));
	 obj_to_obj(money, corpse);
      }
      GET_GOLD(ch) = 0;
   }

   corpse->obj_flags.wear_flags = ITEM_TAKE;
   corpse->obj_flags.extra_flags = ITEM_NODONATE;
   corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
   if (undead)
       corpse->obj_flags.value[3] = -2; /* undead corpse identifyer */
   else
       corpse->obj_flags.value[3] = -1; /* corpse identifyer */
   corpse->obj_flags.type_flag = ITEM_CONTAINER;
   corpse->obj_flags.weight = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
   corpse->obj_flags.cost_per_day = 10;
   if (IS_NPC(ch)){
       if (undead)
	   corpse->obj_flags.timer = max_npc_corpse_time/2;
       else
	   corpse->obj_flags.timer = max_npc_corpse_time;
   }
   else
      corpse->obj_flags.timer = max_pc_corpse_time;

   for (i = 0; i < MAX_WEAR; i++)
      if (ch->equipment[i])
	 obj_to_obj(unequip_char(ch, i), corpse);

   ch->inventory = 0;
   IS_CARRYING_N(ch) = 0;
   IS_CARRYING_W(ch) = 0;

   for (o = corpse->contains; o; o->in_obj = corpse, o = o->next_content)
      ;
   object_list_new_owner(corpse, 0);

   obj_to_room(corpse, ch->in_room, FALSE);
   return(corpse);
}

void	make_scraps(struct obj_data *obj, int r_num)
{
   struct obj_data *scraps;


   CREATE(scraps, struct obj_data, 1);
   clear_object(scraps);

   scraps->item_number = NOWHERE;
   scraps->in_room = NOWHERE;
   scraps->name = str_dup("scraps");

   sprintf(buf2, "Scraps from %s are lying here.", obj->short_description);
   scraps->description = str_dup(buf2);

   sprintf(buf2, "scraps from %s", obj->short_description);
   scraps->short_description = str_dup(buf2);


   scraps->obj_flags.type_flag = ITEM_TRASH;
   scraps->obj_flags.wear_flags = ITEM_TAKE;
   scraps->obj_flags.extra_flags = ITEM_NODONATE;
   scraps->obj_flags.value[0] = 0;
   scraps->obj_flags.value[3] = 0;
   scraps->obj_flags.weight = 1;
   scraps->obj_flags.cost_per_day = 100000;
   scraps->obj_flags.timer = 3;

     
   obj_to_room(scraps,r_num, FALSE);
}


/* When ch kills victim */
void	change_alignment(struct char_data *ch, struct char_data *victim)
{
#define GT_LEVEL(ch) (((ch)->player.level > 0) ? (ch)->player.level : 1)
  
  int	align, lev_dif;

   align = GET_ALIGNMENT(ch) - GET_ALIGNMENT(victim);
   lev_dif = GT_LEVEL(ch) - GT_LEVEL(victim);   

   if (GET_ALIGNMENT(victim) < 0) {
     if (align > 650) {
       if (lev_dif > 0)
	 GET_ALIGNMENT(ch) =
	   MIN(1000, GET_ALIGNMENT(ch) + GT_LEVEL(victim)*
				 (((align - 650) / 8) +
				  ((1000 - GET_ALIGNMENT(victim))/ 5))
				 / GT_LEVEL(ch));
       else
	 GET_ALIGNMENT(ch) = MIN(1000, GET_ALIGNMENT(ch) + GT_LEVEL(ch)*
				 (((align - 650) / 8) +
				  ((1000 - GET_ALIGNMENT(victim))/ 5))
				 / GT_LEVEL(victim));
       
     }
   }
   else {
     if (lev_dif > 0)
       GET_ALIGNMENT(ch) = MIN(1000, GET_ALIGNMENT(ch) - GT_LEVEL(victim)*
			       (GET_ALIGNMENT(victim) / 8)
				 /GT_LEVEL(ch));
     else
       GET_ALIGNMENT(ch) = MIN(1000, GET_ALIGNMENT(ch) - GT_LEVEL(ch)*
			       (GET_ALIGNMENT(victim) / 8)
			       /GT_LEVEL(victim));
       
   }
   if (align < -650) {
     if (lev_dif > 0)
       GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch)
			       + GT_LEVEL(victim)*
			       (((align + 650) / 8)
				+ ((-1000 + GET_ALIGNMENT(victim))
				   / 5))/GT_LEVEL(ch));
     else
       GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch)
			       + GT_LEVEL(ch)*
			       (((align + 650) / 8)
				+ ((-1000 + GET_ALIGNMENT(victim))
				   / 5))/GT_LEVEL(victim));
     
   }
   
   else {
     if (lev_dif > 0)
       GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) -
			       GT_LEVEL(victim)*
			       (GET_ALIGNMENT(victim) / 8)
			       / GT_LEVEL(ch));
     else
       GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) -
			       GT_LEVEL(ch)*
			       (GET_ALIGNMENT(victim) / 8)
			       / GT_LEVEL(victim));
   }
   GET_ALIGNMENT(ch) = MIN(1000, GET_ALIGNMENT(ch));
   GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch));   
}
   


void	death_cry(struct char_data *ch)
{
   int	door, was_in;

   act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
   was_in = ch->in_room;

   for (door = 0; door < NUM_OF_DIRS; door++) {
      if (CAN_GO(ch, door))	 {
	 ch->in_room = real_room(world[was_in].dir_option[door]->to_room);
	 act("Your blood freezes as you hear someone's death cry.", FALSE, ch, 0, 0, TO_ROOM);
	 ch->in_room = was_in;
      }
   }
}



struct obj_data 	*raw_kill(struct char_data *ch)
{
    struct obj_data *corpse;
    
    if (ch->specials.fighting)
      stop_fighting(ch);
    
    while (ch->affected)
       affect_remove(ch, ch->affected);
    
    death_cry(ch);
    
    corpse = make_corpse(ch);
    extract_char(ch, TRUE);
    return(corpse);
}



struct obj_data 	*die(struct char_data *ch)
{
    struct obj_data *corpse;
    
    gain_exp(ch, -(GET_EXP(ch) / 5),0);
    gain_social_standing(ch, ch->specials.fighting, MODE_DIE);
    GET_COND(ch, FULL) = 24;
    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, DRUNK) = 0;        
    if (!IS_NPC(ch))
	REMOVE_BIT(PLR_FLAGS(ch), PLR_KILLER | PLR_THIEF);
    if (!IS_NPC(ch))
      forget(ch);
    corpse = raw_kill(ch);
    return(corpse);
}



void	group_gain(struct char_data *ch, struct char_data *victim)

{
   int	no_members, share, tot_exp, my_share;
   struct char_data *k;
   struct follow_type *f;

   if (!(k = ch->master))
       k = ch;        /* k is the leader */


   if (IS_AFFECTED(k, AFF_GROUP) && 
       (k->in_room == victim->in_room)){
      no_members = 1;
      tot_exp = GET_LEVEL(k);}
   else{
      no_members = 0;
      tot_exp = 0;}

   for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) && 
          (f->follower->in_room == victim->in_room)){
	 no_members++;
	 tot_exp += GET_LEVEL(f->follower);
      }
   share = GET_EXP(victim);
   if (IS_AFFECTED(k, AFF_GROUP) && 
       (k->in_room == victim->in_room)) {
       my_share = share*GET_LEVEL(k)/tot_exp;
       my_share = MIN(6000*GET_LEVEL(ch),my_share);
       sprintf(buf2, "You receive your share of experience -- %d points.",
	       my_share);
       act(buf2, FALSE, k, 0, 0, TO_CHAR);
       gain_exp(k, my_share,1);
       gain_social_standing(ch, victim, MODE_GROUP_KILL);       
       change_alignment(k, victim);
       if (auto_save) {
	   /* save 25% of the time to avoid lag in big groups */
	   if (!(number(0, 3)))
	       save_char(k, NOWHERE);
       }
   }

   for (f = k->followers; f; f = f->next) {
      if (IS_AFFECTED(f->follower, AFF_GROUP) && 
          (f->follower->in_room == victim->in_room)) {
	  my_share = share*GET_LEVEL(f->follower)/tot_exp;
	  share = MIN(3000*GET_LEVEL(f->follower),share);
	  sprintf(buf2, "You receive your share of experience -- %d points.", my_share);
	 act(buf2, FALSE, f->follower, 0, 0, TO_CHAR);
	 gain_exp(f->follower, my_share,1);
	 gain_social_standing(ch, victim, MODE_GROUP_KILL);       	 
	 change_alignment(f->follower, victim);
	 if (auto_save) {
	    /* save only 25% of the time to avoid lag in big groups */
	    if (!(number(0, 3)))
	       save_char(f->follower, NOWHERE);
	 }
      }
   }
}


char	*replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
   static char	buf[256];
   char	*cp;

   cp = buf;

   for (; *str; str++) {
      if (*str == '#') {
	 switch (*(++str)) {
	 case 'W' :
	    for (; *weapon_plural; *(cp++) = *(weapon_plural++)) ;
	    break;
	 case 'w' :
	    for (; *weapon_singular; *(cp++) = *(weapon_singular++)) ;
	    break;
	 default :
	    *(cp++) = '#';
	    break;
	 }
      } else
	 *(cp++) = *str;

      *cp = 0;
   } /* For */

   return(buf);
}



void	dam_message(int dam, struct char_data *ch, struct char_data *victim,
int w_type,int hit_loc)
{
   struct obj_data *wield;
   char	*buf;
   int	msgnum;
   static char *hit_locs1[] = {
       "to the body.",
       "to the arms.",
       "to the legs.",
       "to the head.",
   };

   static char *hit_locs2[] = {
       "in the body.",
       "in the arms.",
       "in the legs.",
       "in the head.",
   };

   static struct dam_weapon_type {
       char	*to_room;
       char	*to_char;
       char	*to_victim;
   } dam_weapons[] = {

   /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

      { "$n misses $N with $s #w ",			/* 0: 0     */
      "You miss $N with your #w ",
      "$n misses you with $s #w " },

      { "$n bruises $N with $s #w ",			/* 1: 1..2  */
      "You bruise $N as you #w $M ",
      "$n bruises you as $e #W you " },

      { "$n barely #W $N ",				/* 2: 3..4  */
      "You barely #w $N ",
      "$n barely #W you " },

      { "$n #W $N ",					/* 3: 5..6  */
      "You #w $N ",
      "$n #W you " },

      { "$n #W $N hard ",				/* 4: 7..10  */
      "You #w $N hard ",
      "$n #W you hard " },

      { "$n #W $N very hard ",				/* 5: 11..14  */
      "You #w $N very hard ",
      "$n #W you very hard " },

      { "$n #W $N extremely hard ", 			/* 6: 15..18  */
      "You #w $N extremely hard ",
      "$n #W you extremely hard " },

      { "$n massacres $N with $s #w ",                  /* 7: 19..25	 */
      "You massacre $N with your #w ",
      "$n massacres you with $s #w " },

      { "$n pulverises $N with $s #w ",                  /* 8: 25..32 */
      "You pulverise $N with your #w ",
      "$n pulverises you with $s #w " },

      { "$n obliterates $N with $s #w ",	       /* 9:  32..45   */
      "You obliterate $N with your #w ",
      "$n obliterates you with $s #w " },

      { "$n annihilates $N with $s #w ",	      /* 10: >45   */
      "You annihilate $N with your #w ",
      "$n annihilates you with $s #w " }
   };   



   w_type -= TYPE_HIT;   /* Change to base of table with text */

   wield = ch->equipment[WIELD];

   if (dam == 0)	msgnum = 0;
   else if (dam <= 2)	msgnum = 1;
   else if (dam <= 4)	msgnum = 2;
   else if (dam <= 6)	msgnum = 3;
   else if (dam <= 10)	msgnum = 4;
   else if (dam <= 14)	msgnum = 5;
   else if (dam <= 19)	msgnum = 6;
   else if (dam <= 25)	msgnum = 7;
   else	if (dam <= 32)	msgnum = 8;
   else	if (dam <= 45)	msgnum = 9;   
   else                 msgnum = 10;


   /* damage message to onlookers */
   buf = replace_string(dam_weapons[msgnum].to_room,
			attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
   if (hit_loc >= 0 && hit_loc <= 3)
     if (msgnum != 0)
       strcat(buf,hit_locs2[hit_loc]);
     else
       strcat(buf,hit_locs1[hit_loc]);
   act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
   
   /* damage message to damager */
   
   buf = replace_string(
			dam_weapons[msgnum].to_char,
			attack_hit_text[w_type].singular,
			attack_hit_text[w_type].plural);
   if (hit_loc >= 0 && hit_loc <= 3)
     if (msgnum != 0)
       strcat(buf,hit_locs2[hit_loc]);
     else
       strcat(buf,hit_locs1[hit_loc]);
   if (dam > 0 || !PRF_FLAGGED(ch, PRF_COMPACT))   
     act(buf, FALSE, ch, wield, victim, TO_CHAR);
   

   /* damage message to damagee */
   
   buf = replace_string(
			dam_weapons[msgnum].to_victim,
			attack_hit_text[w_type].singular,
			attack_hit_text[w_type].plural);
   if (hit_loc >= 0 && hit_loc <= 3)
     if (msgnum != 0)
       strcat(buf,hit_locs2[hit_loc]);
     else
       strcat(buf,hit_locs1[hit_loc]);
   if (dam > 0 || !PRF_FLAGGED(victim, PRF_COMPACT))
     act(buf, FALSE, ch, wield,victim, TO_VICT);
   
}


void	damage(struct char_data *ch, struct char_data *victim, int dam,
	       int attacktype,int hit_loc, bool saved)
{ 
  struct message_type *messages;
  struct msg_type *diemessage;
  struct obj_data *obj_tmp;
  int	i, j, nr, exp,where,atcktype,bhead=0,w_type;
  char bufer[100];
  char bufer2[100];
  char *buf;
  ACMD(do_gen_com);
  ACMD(do_wake);
  static struct behead_message_type {
    char	*to_room;
    char	*to_char;
    char	*to_victim;
  } behead_messages = 
    { "$n severs $N's head with $s mighty #w.",
      "You sever $N's head with a mighty #w.",
      "Everything suddenly goes black."};
   
   
    if (GET_POS(victim) <= POSITION_DEAD) {
      logg("SYSERR: Attempt to damage a corpse.");
      return;   /* -je, 7/7/92 */
    }

    if (!IS_NPC(ch) && !pk_allowed && !IS_NPC(victim) && victim != ch){
      act("You don't think attacking $N such a good idea.",
	  FALSE,ch,0,victim,TO_CHAR);
      return;
    }
    assert(GET_POS(victim) > POSITION_DEAD);

    /* You can't damage an immortal! */
    if ((GET_LEVEL(victim) >= LEVEL_BUILDER) && !IS_NPC(victim))
      dam = 0;
    if (GET_POS(victim) == POSITION_SLEEPING){
      if (IS_AFFECTED(victim, AFF_SLEEP))
	affect_from_char(victim, SPELL_SLEEP);
      do_wake(victim,"",0,0);
    }
    if (IS_AFFECTED(ch, AFF_SNEAK)
	&& (GET_SKILL(ch, SKILL_SNEAK) < number(0,31)))
      affect_from_char(ch, SKILL_SNEAK);
   
    if (ch->specials.mount && (GET_SKILL(ch, SKILL_CAVALRY) < number(0,31)))
      if (throw_rider(ch))
	return;
   
    if (victim != ch) {
      if (GET_POS(ch) > POSITION_STUNNED) {
	if (!(ch->specials.fighting))
	  set_fighting(ch, victim);

	if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	    ((GET_INT(ch) > 10 && !number(0,10))
	     || (GET_INT(ch) <= 10 && GET_INT(ch) > 7 && !number(0,70))
	     || (GET_INT(ch) <= 7 && GET_INT(ch) > 3 && !number(0,120)) )
	    && CAN_SEE(ch, victim->master)
	    && IS_AFFECTED(victim, AFF_CHARM) && 
	    (victim->master->in_room == ch->in_room)) {
	  if (ch->specials.fighting)
	    stop_fighting(ch);
	  act("$n realises who's the boss.",FALSE,ch,0,0,TO_ROOM);
	  hit(ch, victim->master, TYPE_UNDEFINED);
	  return;
	}
      }

      if (GET_POS(victim) > POSITION_STUNNED) {
	if (!(victim->specials.fighting))
	  set_fighting(victim, ch);
	if (IS_NPC(victim) && IS_SET(victim->specials2.act, MOB_MEMORY) && 
	    !IS_NPC(ch) && (GET_LEVEL(ch) < LEVEL_BUILDER))
	  remember(victim, ch);
      }
    }
   

    if (victim->master == ch)
      stop_follower(victim);

    /*    if (IS_AFFECTED(ch, AFF_INVISIBLE))
      appear(ch);*/

    if (IS_AFFECTED(victim, AFF_SANCTUARY) && attacktype > MAX_SPELL_NO)
      dam = dam / 2;  /* 1/2 physical damage when sanctuary */

    if (!pk_allowed) {
      check_killer(ch, victim);

      if (PLR_FLAGGED(ch, PLR_KILLER))
	dam = 0; 
    }

    dam = MAX(dam, 0);

    w_type = attacktype -TYPE_HIT;
    if (!IS_NPC(ch) && victim != ch  && hit_loc == 3
	&& (attacktype == TYPE_SLASH || attacktype == TYPE_CLEAVE))
      bhead = GET_SKILL(ch,SKILL_BEHEAD)*GET_LEVEL(ch)/MAX(2,GET_LEVEL(victim));
    if (bhead && (number(GET_LEVEL(victim),600) - GET_LEVEL(ch)/2) < bhead/10)
      {
	buf = replace_string(behead_messages.to_char,
			     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
	act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	buf = replace_string(behead_messages.to_room,
			     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
	act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);	       
	buf = replace_string(behead_messages.to_victim,
			     attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
	act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	GET_HIT(victim) = -100;
	update_pos(victim);
      }
    
    else if (GET_POS(victim) > POSITION_DEAD){
      GET_HIT(victim) -= dam;
      
      if (ch != victim && !IS_NPC(ch))
	gain_exp(ch, dam,0);
      
      update_pos(victim);
      
      if (!ch->equipment[WIELD] && !ch->specials.attack_type)
	atcktype = TYPE_HIT;
      else
	atcktype = attacktype;
      for (i = 0; i < MAX_MESSAGES; i++)
	{
	  if (fight_messages[i].a_type == atcktype){
	    diemessage = &fight_messages[i].msg->die_msg;
	    break;}
	}
       
      if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_WHIP)) {
	if (GET_POS(victim) == POSITION_DEAD
	    && *diemessage->attacker_msg)
	  {
	    act(diemessage->attacker_msg,
		FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	    act(diemessage->victim_msg,
		FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	    act(diemessage->room_msg,
		FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	  }
	else {
	  dam_message(dam, ch, victim, attacktype, hit_loc);
	  if (IS_MOB(ch) && MOB_FLAGGED(ch, MOB_POISONOUS)
	      && (attacktype >= TYPE_CLAW )){
	    if (dam && (number(0,10) < MIN(1,GET_LEVEL(ch)/10)))
	      poison(ch, victim, 0);
	  }
	}
	
      }
      else {
	for (i = 0; i < MAX_MESSAGES; i++) {
	  if (fight_messages[i].a_type == attacktype) {
	    nr = dice(1, fight_messages[i].number_of_attacks);
	    for (j = 1, messages = fight_messages[i].msg; (j < nr) && (messages); j++)
	      messages = messages->next;
		   
	    if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LEVEL_BUILDER)) {
	      act(messages->god_msg.attacker_msg,
		  FALSE,ch, ch->equipment[WIELD], victim, TO_CHAR);
	      act(messages->god_msg.victim_msg,
		  FALSE,ch, ch->equipment[WIELD], victim, TO_VICT);
	      act(messages->god_msg.room_msg,
		  FALSE,ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	    }
	    else if (!saved) {
	      if (GET_POS(victim) == POSITION_DEAD) {
		act(messages->die_msg.attacker_msg,
		    FALSE,ch,ch->equipment[WIELD],victim,TO_CHAR);
		act(messages->die_msg.victim_msg,
		    FALSE,ch,ch->equipment[WIELD],victim,TO_VICT);
		act(messages->die_msg.room_msg,
		    FALSE,ch,ch->equipment[WIELD],victim,TO_NOTVICT);
	      } else {
		act(messages->hit_msg.attacker_msg,
		    FALSE,ch,ch->equipment[WIELD],victim,TO_CHAR);
		act(messages->hit_msg.victim_msg,
		    FALSE,ch,ch->equipment[WIELD],victim,TO_VICT);
		act(messages->hit_msg.room_msg,
		    FALSE,ch,ch->equipment[WIELD],victim,TO_NOTVICT);
	      }
	    }
	    else { /* tagert saved */
	      act(messages->miss_msg.attacker_msg,
		  FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
	      act(messages->miss_msg.victim_msg,
		  FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
	      act(messages->miss_msg.room_msg,
		  FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
	    }
	  }
	}
      }
   
      /* chance of damaging equipment if blow > 25 points damage) */
      if ((IS_NPC(ch) && dam >= 30) || (!IS_NPC(ch) && dam >= 35))
	if (dam < number(1,250) && ((attacktype >= TYPE_HIT)
				    && (attacktype <= TYPE_WHIP)))
	  {
	    where = number(0,17);
	    if (victim->equipment[where])
	      {
		obj_tmp = unequip_char(victim,where);
		if (IS_SET(obj_tmp->obj_flags.extra_flags,ITEM_FRAGILE))
		  obj_tmp->obj_flags.value[4] += number(1,11);
		else
		  obj_tmp->obj_flags.value[4] += number(0,4);
		if (obj_tmp->obj_flags.value[4] >10)
		  {
		    obj_to_room(obj_tmp,victim->in_room, FALSE);
		    scrap_item(obj_tmp);
		  }
		else if (where != WIELD) {
		  act("$p is crushed.",FALSE,
		      victim,obj_tmp,0,TO_ROOM);
		  act("$p is crushed.",FALSE,
		      victim,obj_tmp,0,TO_CHAR);
		  equip_char(victim,obj_tmp,where);
			   
		}
		else
		  equip_char(victim,obj_tmp,where);
	      }
	  }
    }
    
    /* Use send_to_char -- act() doesn't send message if you are DEAD. */
    switch (GET_POS(victim)) {
    case POSITION_MORTALLYW:
      act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
      break;
    case POSITION_INCAP:
      act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", victim);
      break;
    case POSITION_STUNNED:
      act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
      break;
    case POSITION_DEAD:
      act("$n is dead!  R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You are dead!  Sorry...\r\n", victim);
      break;

    default:  /* >= POSITION SLEEPING */
      if (dam > (GET_MAX_HIT(victim) / 5))
	act("That Really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

      if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 5)) {
	act("You wish that your wounds would stop BLEEDING so much!", FALSE, victim, 0, 0, TO_CHAR);
	if (IS_NPC(victim)) {
	  if (IS_SET(victim->specials2.act, MOB_WIMPY))
	    do_flee(victim, "", 0, 0);
	}
      }

      if (!IS_NPC(victim) && WIMP_LEVEL(victim) && victim != ch && 
          GET_HIT(victim) < WIMP_LEVEL(victim)) {
	send_to_char("You panic, and attempt to flee!\r\n", victim);
	if (GET_POS(victim) <= POSITION_STANDING)
	  do_stand(victim, "", 0, 0);	 
	do_flee(victim, "", 0, 0);
      }

      break;
    }
    if (!IS_NPC(victim) && !(victim->desc)) 
      do_flee(victim, "", 0, 0);
    if (!AWAKE(victim))
      if (victim->specials.fighting)
	stop_fighting(victim);

    if (GET_POS(victim) == POSITION_DEAD) {
      if (IS_NPC(victim) || (victim->desc && victim != ch))
	if (IS_AFFECTED(ch, AFF_GROUP)) {
	  group_gain(ch, victim);
	} 
	else {
	  exp = GET_EXP(victim);
	  exp = MIN(exp,3000*GET_LEVEL(ch));
	  sprintf(buf2, "You receive %d experience points.\r\n", exp);
	  send_to_char(buf2, ch);
	  gain_exp(ch, exp,1);
	  gain_social_standing(ch, victim, MODE_KILL);       	  
	  change_alignment(ch, victim);
	  if (auto_save) {
	    /* individuals saved 50% of the time to avoid lag */
	    if (!(number(0, 1)))
	      save_char(ch, NOWHERE);
	  }
	}
      if (!IS_NPC(victim)) {
	if (IS_NPC(ch)){
	  sprintf(bufer,"Ha! %s died too easily!",GET_NAME(victim));
	  sprintf(bufer2,"I need a tougher challenge!");
	  sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
		  world[victim->in_room].name);
	  mudlog(buf2, BRF, LEVEL_BUILDER, TRUE);
	}
      }
      spell_damage_equipment(ch, victim, attacktype, dam);


      if (!IS_NPC(victim))
	if (IS_NPC(ch) &&
	    (GET_LEVEL(ch) > 10 || IS_SET(ch->specials2.act,MOB_CAN_SPEAK))){
	  do_gen_com(ch,bufer,0,5);
	  do_gen_com(ch,bufer2,0,5);
	}
      
      obj_tmp = die(victim);      
      
    }
}

int get_prof(struct char_data *ch, int *w_type, int type)
{
    int prof;
    struct obj_data *wielded = 0;
    if (type == SKILL_DUAL_WIELD) {
      if (ch->equipment[HOLD] &&
	  (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON))
	wielded = ch->equipment[HOLD];
    }
    else {
      if (ch->equipment[WIELD] && 
	  (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) 
	wielded = ch->equipment[WIELD];
    }
    if (wielded) {
      switch (wielded->obj_flags.value[3]) {
      case 0:
      case 1:
	*w_type = TYPE_BLUDGEON;
	if (!IS_NPC(ch)){
	  prof = PROF_CLUB;
	  if (is_two_handed(ch,wielded))
	    prof++;}
	break;
      case 2:
	*w_type = TYPE_PIERCE;
	if (!IS_NPC(ch)){
	  prof = PROF_DAGGER;
	  if (is_two_handed(ch,wielded))
	    prof = PROF_SWORD +1;
	  else if (GET_SIZE(ch) > GET_OBJ_SIZE(wielded) +1)
	    prof = PROF_SWORD;}
	break;
      case 3:
	*w_type = TYPE_SLASH;
	if (!IS_NPC(ch)){      
	  prof = PROF_SWORD;
	  if (is_two_handed(ch,wielded))
	    prof++;}
	break;
      case 4:
	*w_type = TYPE_SMITE;
	if (!IS_NPC(ch)){      
	  prof = PROF_HAMMER;
	  if (is_two_handed(ch,wielded))
	    prof++;	  }
	break;
      case 5:
	*w_type = TYPE_CLEAVE;
	if (!IS_NPC(ch)){      
	  prof = PROF_AXE;
	  if (is_two_handed(ch,wielded))
	    prof++;	  }
	break;
      case 6:
	*w_type = TYPE_NO_BS_PIERCE;
	if (!IS_NPC(ch)){      
	  prof = PROF_SPEAR;
	  if (!is_two_handed(ch,wielded))
	    prof = PROF_DAGGER;}
	break;
      case 7:
	*w_type = TYPE_CLAW;
	if (!IS_NPC(ch))      
	  prof = PROF_CLAW;
	break;
      case 8:
	*w_type = TYPE_BITE;
	break;
      case 9:
	*w_type = TYPE_STING;
	break;
      case 10:
	*w_type = TYPE_CRUSH;
	break;
      case 11:
	*w_type = TYPE_PECK;
	break;
      case 12:
	*w_type = TYPE_BUTT;
	break;
      case 13:
	*w_type = TYPE_KICK;
	break;
      case 14:
	*w_type = TYPE_WHIP;
	break;
      default:
	*w_type = TYPE_HIT;
	break;
      }
    }
    else {
	if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
	    *w_type = ch->specials.attack_type;
	else
	    *w_type = TYPE_HIT;
    }
    return prof;
}
int compute_multi_attacks(struct char_data *ch, struct obj_data *wielded, int type)
{
  int raw_multi, nattacks=0, weap_multi=0, weap_weight=0, inatt=0;

  if (type == SKILL_BACKSTAB)
    return(1);
  
  raw_multi = GET_LEVEL(ch)/2; /* raw multiple attach percent chance */
  raw_multi = (raw_multi*GET_DEX(ch)*GET_DEX(ch))/255; /* dexterity bonus */
  
  if (wielded) {       
    weap_weight = GET_OBJ_WEIGHT(wielded)/10;
    if (is_two_handed(ch,wielded))
      weap_weight /= 2;
    if (weap_weight >= 3*GET_STR(ch)/4)
      weap_multi = (30 - (40*weap_weight)/GET_STR(ch)) + 30;
    else
      weap_multi = ((40*weap_weight)/GET_STR(ch) - 30) + 30;
    weap_multi = MAX(1,weap_multi);
    weap_multi = MIN(9,weap_multi);       
    raw_multi *= weap_multi;
    raw_multi /= 10.;
  }
  else {
    if (IS_NPC(ch))
      raw_multi += (3*GET_LEVEL(ch))/20;
    else
      raw_multi += GET_SKILL(ch, SKILL_FISTICUFFS);
  }    
  /* ok now determine how many attacks we can have max 6 */
  raw_multi = MIN(149,raw_multi);
  nattacks = 1;
  for (inatt=0;inatt<9;inatt++){
    if (number(0,150) < raw_multi) /*probability of more attacks goes down*/
      nattacks++;  
    raw_multi *= 2;    
    raw_multi /= 3;
  }
  if (type == SKILL_DUAL_WIELD) 
    if (IS_NPC(ch)) 
      nattacks = (nattacks*(GET_LEVEL(ch)/6))/30;
    else
      nattacks = (2*nattacks*GET_SKILL(ch, SKILL_DUAL_WIELD))/60;
  
  
  nattacks = MIN(10,nattacks);
  return(nattacks);
}

void	hit(struct char_data *ch, struct char_data *victim, int type)
{
  struct obj_data *wielded = 0;
  struct obj_data *held = 0;
  int	w_type,hit_loc,hit_location,die,num;
  int	victim_ac, thaco, weap_weight,weap_multi;
  int	dam,raw_multi,nattacks,inatt,icnt,damg,prof=0, prof_factor=0;
  int   diceroll=0, bonus=0, damroll=0, str_dam_bon=0, pos_dam_bon=0;
  int   raw_dam=0, dam_stop=0;
  
  
  if (IS_SET(world[ch->in_room].room_flags, PEACEFULL)) 
    {
      act("You feel too peaceful to contemplate violence.",
	  FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  if (ch->in_room != victim->in_room) {
    logg("SYSERR: NOT SAME ROOM WHEN FIGHTING!");
    return;
  }
  if (GET_POS(ch) < POSITION_SLEEPING) /* can't attack if stunned */
    return;
  
  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
  
  if (ch->equipment[HOLD])
    held = ch->equipment[HOLD];
  
  prof = get_prof(ch, &w_type, type);
  if (ch->equipment[WIELD] && 
      (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) 
    wielded = ch->equipment[WIELD];
  if (type == SKILL_DUAL_WIELD)
    wielded = ch->equipment[HOLD];
  /* Calculate the raw armor including magic armor */
  /* The lower AC, the better                      */
  
  thaco = 20;

  bonus = combat_bonus(ch);

  thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;

  if (!affected_by_spell(ch, SPELL_ENDURANCE))   
    thaco -= bonus;

  if (type == SKILL_DUAL_WIELD)
    thaco -= GET_HITROLL2(ch);
  else
    thaco -= GET_HITROLL(ch);
  
  thaco -= (2*(GET_DEX(ch) - 13))/3;   /* So does dexterity */
  
  if (!IS_NPC(ch) && (GET_LEVEL(ch) < 5))  /* helpt the newbies out */
    thaco -= (GET_LEVEL(ch)*GET_LEVEL(ch))/3;
  
  thaco -= GET_LEVEL(ch)/10;

  thaco = MIN(20, thaco);
  thaco = MAX(-20, thaco);

  
  /* multiple attacks */
  nattacks = compute_multi_attacks(ch, wielded, type);
  
  if (!IS_NPC(ch)){
    if ((type == SKILL_DUAL_WIELD)
	&& (num = number(0,31) > GET_SKILL(ch, SKILL_DUAL_WIELD))){
      if (num == 0)   /* botch */
	if (!number(0,31) && (GET_SKILL(ch, SKILL_DUAL_WIELD) <= 29)) {
	  SET_SKILL(ch, SKILL_DUAL_WIELD, GET_SKILL(ch, SKILL_DUAL_WIELD) +1);
	  send_to_char("You feel more skillful.\r\n",ch);
	  return;
	}
      return;
    }
  } 

  
  /* ok where are we going to hit ?*/
  
  if (ch->in_room != victim->in_room) /* victim may have fled or  */
    return;                         /* otherwise left combat    */
  if (!affected_by_spell(ch, SPELL_ENDURANCE))
    GET_MOVE(ch) -= 1*(encumberance_level(ch) +1);/*lose movement points per blow */
  else
    GET_MOVE(ch) -= 1;
  
  hit_location = number(1,60); 
  if (hit_location <= 60)
    hit_loc = 3;
  if (hit_location <= 56)
    hit_loc = 2;
  if (hit_location <= 40)
    hit_loc = 1;
  if (hit_location <= 30)
    hit_loc = 0;
  
  diceroll = number(0, 50);
  
  if (prof >= PROF_SWORD && prof <= PROF_CLAW)
    num = GET_SKILL(ch,prof);
  else if (!IS_MOB(ch) && !wielded)
    num = GET_SKILL(ch,SKILL_FISTICUFFS);
  else
    num =0;
  
  if(!IS_NPC(ch) && (prof >= PROF_SWORD) && (prof <= PROF_CLAW) && (num < 30)){
    if (!diceroll  && !number(0,3*num*num + 1))
      { 
	send_to_char("You feel more proficient.\r\n",ch);
	SET_SKILL(ch,prof,num + 1);
      }
  }
  else if (!IS_NPC(ch) && !wielded) {
    if (!diceroll  && !number(0, 2*num*num + 1))
      { 
	send_to_char("You feel more proficient.\r\n",ch);
	  SET_SKILL(ch,SKILL_FISTICUFFS,num + 1);
      }
  }
  
  switch (hit_loc){
  case 0:
    victim_ac  = GET_BODY_AC(victim);
    break;    
  case 1:
    victim_ac  = GET_LEGS_AC(victim);
    break;
  case 2:
    victim_ac  = GET_ARMS_AC(victim);
    break;
  case 3:
    victim_ac  = GET_HEAD_AC(victim);
    break;
  default:
    victim_ac  = GET_BODY_AC(victim);
  }
  if (AWAKE(victim))
    victim_ac += dex_app[GET_DEX(victim)].defensive;

  if (!can_see_char(ch,victim))
    victim_ac -= 15;
  
  victim_ac = MAX(-100, victim_ac);
  victim_ac = MIN(100,  victim_ac);  
  
  if (!IS_NPC(ch) && num)
    prof_factor = 50 -  (99*num)/30;
  else if (IS_NPC(ch))
    prof_factor = 0;
  else 
    prof_factor = 100;
  
  prof_factor /= 4;
  
  if ((GET_POS(victim) < POSITION_SLEEPING)
      || IS_AFFECTED(victim, AFF_PARALYSIS))
      diceroll = 50;
  
  
  if ((diceroll < 50) && AWAKE(victim) && (type != SKILL_BACKSTAB) &&
	 (!diceroll ||
	  (diceroll < (thaco + prof_factor + (100 - victim_ac)/4))))
      damage(ch, victim, 0, w_type,hit_loc, 0); /* MISSED */
  else
    {
      str_dam_bon  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
      dam = str_dam_bon;
      
      if (!affected_by_spell(ch, SPELL_ENDURANCE))
	dam += bonus;
      
      if (type == SKILL_DUAL_WIELD)
	damroll = GET_DAMROLL2(ch);
      else
	damroll = GET_DAMROLL(ch);
      
      dam += damroll;
      
      if (!wielded) {
	if (IS_NPC(ch) && ch->specials.damsizedice){
	  raw_dam = dice(ch->specials.damnodice, ch->specials.damsizedice);
	}
	else 
	    raw_dam = number(0,MIN(1,GET_SKILL(ch, SKILL_FISTICUFFS)/2));
      }
      else{
	damg = MAX(0,wielded->obj_flags.value[4]);
	if (wielded->obj_flags.value[2])
	  raw_dam = (dice(wielded->obj_flags.value[1],
			  wielded->obj_flags.value[2]))*(10-damg)/10;
	raw_dam *= (num + 120);
	raw_dam /= 150;
      }
      dam += raw_dam;
	
      if (GET_POS(victim) < POSITION_STANDING)
	pos_dam_bon = (dam*(POSITION_STANDING - GET_POS(victim)))/3.;
      dam += pos_dam_bon;
      
      switch (hit_loc){
      case 0:
	dam_stop = GET_BODY_STOPPING(ch);
	break;
      case 1:
	dam_stop = GET_LEGS_STOPPING(ch);
	break;
      case 2:
	dam_stop = GET_ARMS_STOPPING(ch);
	break;
      case 3:
	dam_stop = GET_HEAD_STOPPING(ch);
	break;
      default:
	dam_stop = GET_BODY_STOPPING(ch);
      }
      
      if (type == SKILL_BACKSTAB) {
	dam *= MIN(GET_LEVEL(ch) - spell_lev(ch,SKILL_BACKSTAB)
		   + GET_SKILL(ch,SKILL_BACKSTAB)/2,75);
	dam /= 5;
	dam *= GET_DEX(ch);
	dam /= 15;
	die = stress_die();
	dam += die;
	sprintf(buf,"Backstab: die = %d, level = %d dam = %d\r\n",
		die,spell_lev(ch, SKILL_BACKSTAB),dam);
	if (PRF_FLAGGED(ch, PRF_DEBUG))
	  send_to_char(buf,ch);
	if (affected_by_spell(victim, SPELL_ENCASE_IN_ICE)){
	  if (!number(0,3) && (dam > 5)){
	    act("$n's backstab shatters the ice encasing $N!",
		TRUE,ch,0,victim,TO_NOTVICT);
	    act("$n's backstab shatters the ice encasing you!",
		TRUE,ch,0,victim,TO_VICT);
	    act("Your backstab shatters the ice encasing $N.",
		TRUE,ch,0,victim,TO_CHAR);
	    damage(ch,victim,dam/4,SKILL_BACKSTAB,-1,0);
	    affect_from_char(victim, SPELL_ENCASE_IN_ICE);
	  }
	  else {
	    act("$n's backstab glances off the ice encasing $N!",
		TRUE,ch,0,victim,TO_NOTVICT);
	    act("$n's backstab glances off the ice encasing you!",
		TRUE,ch,0,victim,TO_VICT);
	    act("Your backstab glances off the ice encasing $N.",
		TRUE,ch,0,victim,TO_CHAR);
	    if (!(ch->specials.fighting) && (dam > 5))
	      set_fighting(ch, victim);	     
	  }
	}
	else
	  damage(ch,victim,dam,SKILL_BACKSTAB,-1,0);
      }
      else{
	dam -= dam_stop;
	
	dam = MAX(1, dam);  /* Not less than 0 damage */
	
	if (affected_by_spell(victim, SPELL_ENCASE_IN_ICE)){
	  if (!number(0,3) && (dam > 5)){
	    act("$n's blow shatters the ice encasing $N!",
		TRUE,ch,0,victim,TO_NOTVICT);
	    act("$n's blow shatters the ice encasing you!",
		TRUE,ch,0,victim,TO_VICT);
	    act("Your blow shatters the ice encasing $N.",
		TRUE,ch,0,victim,TO_CHAR);
	    dam /= 4;	     
	    damage(ch,victim,dam,w_type,hit_loc,0);
	    affect_from_char(victim, SPELL_ENCASE_IN_ICE);
	  }
	  else {
	    act("$n's blow glances off the ice encasing $N!",
		TRUE,ch,0,victim,TO_NOTVICT);
	    act("$n's blow glances off the ice encasing you!",
		TRUE,ch,0,victim,TO_VICT);
	    act("Your blow glances off the ice encasing $N.",
		TRUE,ch,0,victim,TO_CHAR);
	    if (!(ch->specials.fighting) && (dam > 5))
	      set_fighting(ch, victim);	     
	  }
	}
	else
	  damage(ch,victim,dam,w_type,hit_loc,0);
      }
    }
  nattacks--;
  
  if (nattacks > 0)
    for (icnt=0;icnt<nattacks;icnt++){
      if (type == SKILL_BACKSTAB)
	break;
      if (!victim)
	return;                    
      if (ch->in_room != victim->in_room)/* victim may have fled or */
	return;                    /* otherwise left combat */
      
      GET_MOVE(ch) -= 1;        /* loose a movement point per blow */
      
      hit_location = number(1,60); 
      if (hit_location <= 60)
	hit_loc = 3;
      if (hit_location <= 56)
	hit_loc = 2;
      if (hit_location <= 40)
	hit_loc = 1;
      if (hit_location <= 30)
	hit_loc = 0;
      
      diceroll = number(1, 50);

      if(!IS_NPC(ch) && (prof >= PROF_SWORD) && (prof <= PROF_CLAW) && (num < 30)){
	if (!diceroll  && !number(0,3*num*num + 1))
	  { 
	    send_to_char("You feel more proficient.\r\n",ch);
	    SET_SKILL(ch,prof,num + 1);
	  }
      }
      else if (!IS_NPC(ch) && !wielded) {
	if (!diceroll  && !number(0, 2*num*num + 1))
	  { 
	    send_to_char("You feel more proficient.\r\n",ch);
	    SET_SKILL(ch,SKILL_FISTICUFFS,num + 1);
	  }
      }
      
      switch (hit_loc){
      case 0:
	victim_ac  = GET_BODY_AC(victim);
	break;
      case 1:
	victim_ac  = GET_LEGS_AC(victim);
	break;
      case 2:
	victim_ac  = GET_ARMS_AC(victim);
	break;
      case 3:
	victim_ac  = GET_HEAD_AC(victim);
	break;
      default:
	victim_ac  = GET_BODY_AC(victim);	
      }
      if (AWAKE(victim))
	victim_ac -= dex_app[GET_DEX(victim)].defensive;
      
      victim_ac = MAX(-100, victim_ac);  /* -100 is lowest */
      if (!IS_NPC(ch) && num)
	prof_factor = 50 -  (99*num)/30;
      else if (IS_NPC(ch))
	prof_factor = 0;
      else 
	prof_factor = 100;

      prof_factor /= 4;
      
      if ((((diceroll < 50) && AWAKE(victim)) && 
	   (!diceroll
	    || (diceroll 
		<  (thaco + prof_factor + (100 - victim_ac)/4))))) {
	damage(ch, victim, 0, w_type,hit_loc,0);
      } else {
	
	str_dam_bon  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
	dam = str_dam_bon;
	
	if (type == SKILL_DUAL_WIELD)
	  damroll = GET_DAMROLL2(ch);
	else
	  damroll = GET_DAMROLL(ch);
	dam += damroll;
	
	if (!wielded) {
	  if (IS_NPC(ch) &&  ch->specials.damsizedice) {
	    raw_dam = dice(ch->specials.damnodice, ch->specials.damsizedice);
	  } else
	    raw_dam = number(0, MIN(1,GET_SKILL(ch, SKILL_FISTICUFFS)/2));
	}
	else{
	  damg = MAX(0,wielded->obj_flags.value[4]);
	  if (wielded->obj_flags.value[2])
	    raw_dam = (dice(wielded->obj_flags.value[1],
			    wielded->obj_flags.value[2]))*(10-damg)/10;
	  raw_dam *= (num + 120);
	  raw_dam /= 150;
	}
	dam += raw_dam;
	if (GET_POS(victim) < POSITION_STANDING)
	  pos_dam_bon = (dam*(POSITION_STANDING - GET_POS(victim)))/3.;
	dam += pos_dam_bon;
	/* Position  sitting  x 1.33 */
	/* Position  resting  x 1.66 */
	/* Position  sleeping x 2.00 */
	/* Position  stunned  x 2.33 */
	/* Position  incap    x 2.66 */
	/* Position  mortally x 3.00 */
	  
	switch (hit_loc){
	case 0:
	  dam_stop = GET_BODY_STOPPING(ch);
	  break;
	case 1:
	  dam_stop = GET_LEGS_STOPPING(ch);
	  break;
	case 2:
	  dam_stop = GET_ARMS_STOPPING(ch);
	  break;
	case 3:
	  dam_stop = GET_HEAD_STOPPING(ch);
	  break;
	default:
	  dam_stop = GET_BODY_STOPPING(ch);	  
	}
	dam -= dam_stop;
	dam = MAX(1, dam);  /* Not less than 0 damage */
	add_event(icnt/2, EVENT_COMBAT, dam, w_type, hit_loc,0,0,ch,victim); 
      }
    }
}


/* control the fights going on */
void	perform_violence(void)
{
    struct char_data *ch;
    int i;
    bool found = FALSE;
    struct obj_data *sheath;
   
   for (ch = combat_list; ch; ch = combat_next_dude) {
      combat_next_dude = ch->next_fighting;
      assert(ch->specials.fighting);
      if (IS_NPC(ch) && ch->specials.timer)
	ch->specials.timer--;
      if (AWAKE(ch) && (ch->in_room == ch->specials.fighting->in_room)) {
	if (!ch->equipment[WIELD])
	  for (i=0; i<= MAX_WEAR; i++)
	    if (sheath = ch->equipment[i])
	      if ((GET_ITEM_TYPE(sheath) == ITEM_SCABBARD) &&
		  (sheath->contains))
		found=TRUE;
	if (found)
	  do_draw(ch,"",0,0);
	if (GET_POS(ch) < POSITION_STANDING && IS_NPC(ch))
	  do_stand(ch,"",0,0);
	if (!IS_AFFECTED(ch, AFF_PARALYSIS)
	    || (GET_LEVEL(ch) >= LEVEL_BUILDER)) {
	  hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	  if (ch->equipment[HOLD]
	      && (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON)
	      && ch->specials.fighting)
	    hit(ch, ch->specials.fighting, SKILL_DUAL_WIELD);
	}
      } else { /* Not in same room */
	stop_fighting(ch);
      }
   }
}


void botch_kick(struct char_data *ch, struct char_data *vict)
{
  ACMD(do_say);
  int die;
  die = stress_die();
  
  if(die < 6){
    act("$n kicks out in a very energetic fashion.",FALSE, ch,0,vict,TO_ROOM);
    if (ch->specials.fighting && ch->specials.fighting == vict){
      if (GET_BODY_STOPPING(vict) > 0){
	act("Your kick bounces harmlessly off $N's armour."
	    ,FALSE, ch,0,vict,TO_CHAR);
	act("$n's kick bounces harmlessly off $N's armour."
	    ,FALSE, ch,0,vict,TO_NOTVICT);
	act("$n's kick bounces harmlessly off your armour."
	    ,FALSE, ch,0,vict,TO_VICT);}
      else if ((GET_POS(vict) > POSITION_SLEEPING) &&
	       !IS_AFFECTED(vict, AFF_PARALYSIS)){
	act("$N easily dodges your kick.",FALSE, ch,0,vict,TO_CHAR);
	act("$N easily dodges $n's kick.",FALSE, ch,0,vict,TO_NOTVICT);
	act("You easily dodge $n's kick.",FALSE, ch,0,vict,TO_VICT);
      }
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);       
      return;
    }
    else{
      act("You try to kick $N, but miss by miles.",FALSE, ch,0,vict,TO_CHAR);
      if (IS_NPC(vict) && CAN_SPEAK(vict))
	do_say(vict,"You'd better watch out. You could hurt yourself doing that.",0,0);
      return;
    }   
  }
  else {
    act("$n kicks out in a very energetic fashion.",FALSE, ch,0,vict,TO_ROOM);
    if (ch->specials.fighting && ch->specials.fighting == vict){
      if (GET_BODY_STOPPING(vict) > 0){
	act("Your kick bounces harmlessly off $N's armour."
	    ,FALSE, ch,0,vict,TO_CHAR);
	act("You fall over.",FALSE, ch,0,vict,TO_CHAR);
	act("$n falls flat in $s face.",FALSE, ch,0,vict,TO_ROOM);
	act("$n's kick bounces harmlessly off $N's armour."
	    ,FALSE, ch,0,vict,TO_NOTVICT);
	act("$n's kick bounces harmlessly off your armour."
	    ,FALSE, ch,0,vict,TO_VICT);}
      else{
	if ((GET_POS(vict) > POSITION_SLEEPING) &&
	    !IS_AFFECTED(vict, AFF_PARALYSIS)){
	  act("You easily dodge $n's kick.",FALSE, ch,0,vict,TO_VICT);
	  act("$N easily dodges your kick.",FALSE, ch,0,vict,TO_CHAR);
	  act("$N easily dodges $n's kick.",FALSE, ch,0,vict,TO_NOTVICT);
	}
	act("You fall over.",FALSE, ch,0,vict,TO_CHAR);
	act("$n falls flat in $s face.",FALSE, ch,0,vict,TO_ROOM);  
	
      }
      GET_POS(ch) = POSITION_SITTING;
      WAIT_STATE(ch, PULSE_VIOLENCE * 4);       
      return;
    }
    else{
      act("You try to kick $N, but miss by miles.",FALSE,
	  ch,0,vict,TO_CHAR);
      act("You fall over.",FALSE, ch,0,vict,TO_CHAR);
      act("$n falls flat in $s face.",FALSE, ch,0,vict,TO_ROOM); 	
      if (IS_NPC(vict) && CAN_SPEAK(vict))
	do_say(vict,"Hey!",0,0);
      act("$n is enraged.",TRUE,vict,0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      hit(vict,ch,TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE * 4);
      return;
    }   
  }
  return;
}

int stress_die(void)
{
    int die,bon;
    bon=1;
    while((die = number(0,9)) == 1 && (bon < (1<< 11)))
	bon <<= 1;
    die *= bon;
    return (MAX(0,die));

}
