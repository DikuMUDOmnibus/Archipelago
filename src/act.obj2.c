/* ************************************************************************
*   File: act.obj2.c                                    Part of CircleMUD *
*  Usage: eating/drinking and wearing/removing equipment                  *
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

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct index_data *obj_index;
extern char	*drinks[];
extern int	drink_aff[][3];
void poison_vict(struct char_data *i);
bool   is_held_obj_twohanded(struct char_data *ch, struct obj_data *obj);
bool   is_two_handed(struct char_data *ch, struct obj_data *weapon);
void   clear_timer(struct obj_data *obj);
int	perform_drop(struct char_data *ch, struct obj_data *obj,
		     byte mode, char *sname, short RDR);
bool can_hold(struct char_data *ch, struct obj_data *obj);
int  can_wield(struct char_data *ch, struct obj_data *weapon);
void	weight_change_object(struct obj_data *obj, int weight)
{
   struct obj_data *tmp_obj;
   struct char_data *tmp_ch;

   if (obj->in_room != NOWHERE) {
      GET_OBJ_WEIGHT(obj) += weight;
   } else if ((tmp_ch = obj->carried_by)) {
      obj_from_char(obj,0);
      GET_OBJ_WEIGHT(obj) += weight;
      obj_to_char(obj, tmp_ch,0);
   } else if ((tmp_obj = obj->in_obj)) {
      obj_from_obj(obj);
      GET_OBJ_WEIGHT(obj) += weight;
      obj_to_obj(obj, tmp_obj);
   } else {
      logg("SYSERR: Unknown attempt to subtract weight from an object.");
   }
}



void	name_from_drinkcon(struct obj_data *obj)
{
   int	i;
   char	*new_name;
   extern struct obj_data *obj_proto;

   for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++)
      ;

   if (*((obj->name) + i) == ' ') {
      new_name = str_dup((obj->name) + i + 1);
      if (obj->item_number < 0 || obj->name != obj_proto[obj->item_number].name)
	 free(obj->name);
      obj->name = new_name;
   }
}



void	name_to_drinkcon(struct obj_data *obj, int type)
{
   char	*new_name;
   extern struct obj_data *obj_proto;
   extern char	*drinknames[];

   CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
   sprintf(new_name, "%s %s", drinknames[type], obj->name);
   if (obj->item_number < 0 || obj->name != obj_proto[obj->item_number].name)
      free(obj->name);
   obj->name = new_name;
}



ACMD(do_drink)
{
   struct obj_data *temp;
   struct affected_type af;
   int	amount, weight;
   int	on_ground = 0;

   one_argument(argument, arg);

   if (!*arg) {
      send_to_char("Drink from what?\r\n", ch);
      return;
   }

   if (ch->equipment[WEAR_MOUTH]){
       send_to_char("You can drink with something in your mouth?\r\n", ch);
       return;
   }   
   
   if (!(temp = get_obj_in_list_vis(ch, arg, ch->inventory))) {
      if (!(temp = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
	 act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      } else
	 on_ground = 1;
   }

   if ((GET_ITEM_TYPE(temp) != ITEM_DRINKCON) && 
       (GET_ITEM_TYPE(temp) != ITEM_FOUNTAIN)) {
      send_to_char("You can't drink from that!\r\n", ch);
      return;
   }

   if (on_ground && (GET_ITEM_TYPE(temp) == ITEM_DRINKCON)) {
      send_to_char("You have to be holding that to drink from it.\r\n", ch);
      return;
   }

   if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
      /* The pig is drunk */
      send_to_char("You can't seem to get close enough to your mouth.\r\n", ch);
      act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
      return;
   }

   if (!temp->obj_flags.value[1]) {
      send_to_char("It's empty.\r\n", ch);
      return;
   }

   if (subcmd == SCMD_DRINK) {
      if (temp->obj_flags.type_flag != ITEM_FOUNTAIN) {
         sprintf(buf, "$n drinks %s from $p.", drinks[temp->obj_flags.value[2]]);
         act(buf, TRUE, ch, temp, 0, TO_ROOM);
      }

      sprintf(buf, "You drink the %s.\r\n", drinks[temp->obj_flags.value[2]]);
      send_to_char(buf, ch);

      if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
         amount = (25 - GET_COND(ch, THIRST)) / drink_aff[temp->obj_flags.value[2]][DRUNK];
      else
         amount = number(3, 10);

   } else {
      act("$n sips from the $o.", TRUE, ch, temp, 0, TO_ROOM);
      sprintf(buf, "It tastes like %s.\r\n", drinks[temp->obj_flags.value[2]]);
      send_to_char(buf, ch);
      amount = 1;
   }

   amount = MIN(amount, temp->obj_flags.value[1]);

   /* You can't subtract more than the object weighs */
   weight = MIN(amount, temp->obj_flags.weight);

   weight_change_object(temp, -weight);  /* Subtract amount */

   gain_condition(ch, DRUNK,
      (int)((int)drink_aff[temp->obj_flags.value[2]][DRUNK]*amount) / 4);

   gain_condition(ch, FULL,
      (int)((int)drink_aff[temp->obj_flags.value[2]][FULL]*amount) / 4);

   gain_condition(ch, THIRST,
      (int)((int)drink_aff[temp->obj_flags.value[2]][THIRST]*amount) / 4);

   if (GET_COND(ch, DRUNK) > 10)
      send_to_char("You feel drunk.\r\n", ch);

   if (GET_COND(ch, THIRST) > 20)
      send_to_char("You don't feel thirsty any more.\r\n", ch);

   if (GET_COND(ch, FULL) > 20)
      send_to_char("You are full.\r\n", ch);

   if (temp->obj_flags.value[3]) { /* The shit was poisoned ! */
      send_to_char("Oops, it tasted rather strange!\r\n", ch);
      act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

      af.type = SPELL_POISON;
      af.duration = amount * 3;
      af.modifier = temp->obj_flags.value[7];
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join(ch, &af, FALSE, FALSE);
      poison_vict(ch);
   }

   /* empty the container, and no longer poison. */
   temp->obj_flags.value[1] -= amount;
   if (!temp->obj_flags.value[1]) {  /* The last bit */
      temp->obj_flags.value[2] = 0;
      temp->obj_flags.value[3] = 0;
      name_from_drinkcon(temp);
   }

   return;
}



ACMD(do_eat)
{
  struct obj_data *food;
  struct affected_type af;
  int	amount;
  
  one_argument(argument, arg);
  
  if (!*arg) {
    send_to_char("Eat what?\r\n", ch);
    return;
  }
  if (ch->equipment[WEAR_MOUTH]){
    send_to_char("You can eat with something in your mouth?\r\n", ch);
    return;
  }   

  if (!(food = get_obj_in_list_vis(ch, arg, ch->inventory))) {
    send_to_char("You don't seem to have any.\r\n", ch);
    return;
  }

  if ((subcmd == SCMD_TASTE) &&
      ((GET_ITEM_TYPE(food) == ITEM_DRINKCON) ||
       (GET_ITEM_TYPE(food) == ITEM_FOUNTAIN))) {
    do_drink(ch, argument, 0, SCMD_SIP);
    return;
  }

  if ((GET_ITEM_TYPE(food) != ITEM_FOOD)){
    if (GET_LEVEL(ch) < LEVEL_MBUILDER) 
      { 
	send_to_char("You can't eat THAT!\r\n", ch);
	return;
      }
    else {
      act("You eat the $o.", FALSE, ch, food, 0, TO_CHAR);
      act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
      if (food->action_description && *food->action_description){
	strcpy(buf,food->action_description);
	send_to_char(buf,ch);}
      extract_obj(food,0);
      return;
    }
  }

  if (GET_COND(ch, FULL) > 20) { /* Stomach full */
    act("You are too full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  
  if (subcmd == SCMD_EAT) {
    act("You eat the $o.", FALSE, ch, food, 0, TO_CHAR);
    act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
  } else {
    act("You nibble a little bit of the $o.", FALSE, ch, food, 0, TO_CHAR);
    act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
  }
  
  if (food->action_description && *food->action_description){
    strcpy(buf,food->action_description);
    send_to_char(buf,ch);}
  
  amount = (subcmd == SCMD_EAT ? food->obj_flags.value[0] : 1);
  
  gain_condition(ch, FULL, amount);
  
  if (GET_COND(ch, FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);
  
  if (food->obj_flags.value[3] && (GET_LEVEL(ch) < LEVEL_BUILDER)) {
      /* The shit was poisoned ! */
      send_to_char("Oops, that tasted rather strange!\r\n", ch);
      act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

      af.type = SPELL_POISON;
      af.duration = amount * 2;
      af.modifier = food->obj_flags.value[7];
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join(ch, &af, FALSE, FALSE);
      poison_vict(ch);
   }
   
   if (subcmd == SCMD_EAT)
      extract_obj(food,0);
   else {
      if (!(--food->obj_flags.value[0])) {
	 send_to_char("There's nothing left now.\r\n", ch);
	 extract_obj(food,0);
      }
   }
}


ACMD(do_pour)
{
   char	arg1[MAX_INPUT_LENGTH];
   char	arg2[MAX_INPUT_LENGTH];
   struct obj_data *from_obj;
   struct obj_data *to_obj;
   int	amount;

   argument_interpreter(argument, arg1, arg2);

   if (subcmd == SCMD_POUR) {
      if (!*arg1) /* No arguments */ {
	 act("What do you want to pour from?", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->inventory))) {
	 act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if (from_obj->obj_flags.type_flag != ITEM_DRINKCON) {
	 act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }
   }

   if (subcmd == SCMD_FILL) {
      if (!*arg1) /* no arguments */ {
	 send_to_char("What do you want to fill?  And what are you filling it from?\r\n", ch);
	 return;
      }

      if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->inventory))) {
	 send_to_char("You can't find it!", ch);
	 return;
      }

      if (GET_ITEM_TYPE(to_obj) != ITEM_DRINKCON) {
	 act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
	 return;
      }

      if (!*arg2) /* no 2nd argument */ {
	 act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
	 return;
      }

      if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents))) {
	 sprintf(buf, "There doesn't seem to be any '%s' here.\r\n", arg2);
	 send_to_char(buf, ch);
	 return;
      }

      if (GET_ITEM_TYPE(from_obj) != ITEM_FOUNTAIN) {
	 act("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
	 return;
      }
   }

   if (from_obj->obj_flags.value[1] == 0) {
      act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
   }

   if (subcmd == SCMD_POUR) /* pour */ {
      if (!*arg2) {
	 act("Where do you want it?  Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if (!str_cmp(arg2, "out")) {
	 act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
	 act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

	 weight_change_object(from_obj, -from_obj->obj_flags.value[1]); /* Empty */

	 from_obj->obj_flags.value[1] = 0;
	 from_obj->obj_flags.value[2] = 0;
	 from_obj->obj_flags.value[3] = 0;
	 name_from_drinkcon(from_obj);

	 return;
      }

      if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->inventory))) {
	 act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }

      if ((to_obj->obj_flags.type_flag != ITEM_DRINKCON) && 
          (to_obj->obj_flags.type_flag != ITEM_FOUNTAIN)) {
	 act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
	 return;
      }
   }

   if (to_obj == from_obj) {
      act("A most unproductive effort.", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   if ((to_obj->obj_flags.value[1] != 0) && 
       (to_obj->obj_flags.value[2] != from_obj->obj_flags.value[2])) {
      act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   if (!(to_obj->obj_flags.value[1] < to_obj->obj_flags.value[0])) {
      act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
      return;
   }

   if (subcmd == SCMD_POUR) {
      sprintf(buf, "You pour the %s into the %s.",
          drinks[from_obj->obj_flags.value[2]], arg2);
      send_to_char(buf, ch);
   }

   if (subcmd == SCMD_FILL) {
      act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
      act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
   }

   /* New alias */
   if (to_obj->obj_flags.value[1] == 0)
      name_to_drinkcon(to_obj, from_obj->obj_flags.value[2]);

   /* First same type liq. */
   to_obj->obj_flags.value[2] = from_obj->obj_flags.value[2];

   /* Then how much to pour */
   from_obj->obj_flags.value[1] -= (amount = 
       (to_obj->obj_flags.value[0] - to_obj->obj_flags.value[1]));

   to_obj->obj_flags.value[1] = to_obj->obj_flags.value[0];

   if (from_obj->obj_flags.value[1] < 0) {  /* There was too little */
      to_obj->obj_flags.value[1] += from_obj->obj_flags.value[1];
      amount += from_obj->obj_flags.value[1];
      from_obj->obj_flags.value[1] = 0;
      from_obj->obj_flags.value[2] = 0;
      from_obj->obj_flags.value[3] = 0;
      name_from_drinkcon(from_obj);
   }

   /* Then the poison boogie */
   to_obj->obj_flags.value[3] =
      (to_obj->obj_flags.value[3] || from_obj->obj_flags.value[3]);
   to_obj->obj_flags.value[7] =
      (to_obj->obj_flags.value[7] || from_obj->obj_flags.value[7]);   

   /* And the weight boogie */
   weight_change_object(from_obj, -amount);
   weight_change_object(to_obj, amount);   /* Add weight */

   return;
}



void	wear_message(struct char_data *ch, struct obj_data *obj, int where)
{
   char *wear_messages[][2] = {
      { "$n straps $p around $s waist.",
	"You strap $p around your waist." },

      { "$n slides $p on to $s right ring finger.",
	"You slide $p on to your right ring finger." },

      { "$n slides $p on to $s left ring finger.",
	"You slide $p on to your left ring finger." },

      { "$n wears $p around $s neck.",
        "You wear $p around your neck." },

      { "$n wears $p around $s neck.",
        "You wear $p around your neck." },

      { "$n wears $p on $s body." ,
	"You wear $p on your body.", },

      { "$n wears $p on $s head.",
	"You wear $p on your head." },

      { "$n puts $p on $s legs.",
	"You put $p on your legs." },

      { "$n wears $p on $s feet.",
	"You wear $p on your feet." },

      { "$n puts $p on $s hands.",
	"You put $p on your hands." },

      { "$n wears $p on $s arms.",
	"You wear $p on your arms." },

      { "$n straps $p around $s arm as a shield.",
	"You start to use $p as a shield." },

      { "$n wears $p about $s body." ,
	"You wear $p around your body." },

      { "$n wears $p around $s waist.",
	"You wear $p around your waist." },

      { "$n puts $p around $s right wrist.",
	"You put $p around your right wrist." },

      { "$n puts $p around $s left wrist.",
	"You put $p around your left wrist." },

      { "$n wields $p.",
        "You wield $p." },

      { "$n grabs $p.",
	"You grab $p." },
      
      { "$n puts $p over $s eyes.",
	"You wear $p over your eyes." },

      { "$n hooks $p on $s right ear.",
	"You put $p on your right ear." },

      { "$n hooks $p on $s left ear.",
	"You put $p on your left ear." },

      { "$n puts $p in $s mouth.",
	"You put $p in your mouth." },

      { "$n slings $p over $s back.",
	"You sling $p over your back." },

      { "$n wears $p around $s waist.",
	"You wear $p around your waist." },
      
   };

   act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM);
   act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}



void	perform_wear(struct char_data *ch, struct obj_data *obj, int where)
{
   int wear_bitvectors[] = {
      ITEM_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
      ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, 
      ITEM_WEAR_LEGS, ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, 
      ITEM_WEAR_SHIELD, ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, 
      ITEM_WEAR_WRIST, ITEM_WIELD, ITEM_TAKE, ITEM_WEAR_EYES, ITEM_WEAR_EARS,
      ITEM_WEAR_EARS, ITEM_WEAR_MOUTH, ITEM_WEAR_BACK};

   char *already_wearing[] = {
      "You're already wearing a scabbard.\r\n",
      "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
      "You're already wearing something on both of your ring fingers.\r\n",
      "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
      "You can't wear anything else around your neck.\r\n",
      "You're already wearing something on your body.\r\n",
      "You're already wearing something on your head.\r\n",
      "You're already wearing something on your legs.\r\n",
      "You're already wearing something on your feet.\r\n",
      "You're already wearing something on your hands.\r\n",
      "You're already wearing something on your arms.\r\n",
      "You're already using a shield.\r\n",
      "You're already wearing something about your body.\r\n",
      "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
      "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
      "You're already wearing something around both of your wrists.\r\n",
      "You're already wielding a weapon.\r\n",
      "You're already holding something.\r\n",
      "You're already wearing something over your eyes.\r\n",
      "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
      "You're already wearing something on both your ears.\r\n",
      "You already have something in your mouth.\r\n",
      "You already have something on your back.\r\n",
      "You already have enough around your waist.\r\n"
   };

   /* first, make sure that the wear position is valid. */
   if (!CAN_WEAR(obj, wear_bitvectors[where])) {
      act("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
      return;
   }

   /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
   if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R) || (where == WEAR_EARS_R))
     if (ch->equipment[where])
       where++;
   if (where == WEAR_WAIST)
     if (ch->equipment[where])
       where = WEAR_WAIST_2;
   
   if ((where == WEAR_SHIELD)
       && (ch->equipment[WIELD]  && is_two_handed(ch,ch->equipment[WIELD]))) {
     send_to_char("You can't wear a shield and wield a twohanded weapon.\r\n",ch);
     return;
   }
   if ((where == WEAR_SHIELD) &&
       (ch->equipment[WIELD]
	&& (ch->equipment[HOLD] 
	    && (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON)))){
     send_to_char("You can't wear a shield and wield a two weapons.\r\n",ch);     
     return;
   }
   if ((where == WIELD) && !(where = can_wield(ch,obj)))
     return;
   if ((where == HOLD) && !can_hold(ch,obj))
     return;   
   if (ch->equipment[where]) {
     send_to_char(already_wearing[where], ch);
     return;
   }

   wear_message(ch, obj, where);
   obj_from_char(obj,1);
   clear_timer(obj);
   equip_char(ch, obj, where);
}

bool can_hold(struct char_data *ch, struct obj_data *obj)
{
  if (ch->specials.carrying) {
    act("You can't hold anything else whilst carrying $N.",FALSE,ch,0,ch->specials.carrying, TO_CHAR);
    return(FALSE);
  }
  if (is_held_obj_twohanded(ch,obj)) {
    if (ch->equipment[WEAR_SHIELD]){
      act("You can't hold $p and wear a shield.",FALSE,ch,obj,0,TO_CHAR);
      return(0);}
    else if (ch->equipment[WIELD]){
      act("You can't hold $p while using a weapon.",FALSE,ch,obj,0,TO_CHAR);
      return(0);
    }
  }
  else if  (ch->equipment[WIELD] && is_two_handed(ch,ch->equipment[WIELD])) {
    act("You can't hold $p while wielding a two-handed weapon."
	,FALSE,ch,obj,0,TO_CHAR);
    return(0);
  }
  return(1);
}
int can_wield (struct char_data *ch, struct obj_data *weapon)
{
  if (GET_OBJ_WEIGHT(weapon) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w){
    act("$p is too heavy for you to use.",FALSE, ch,weapon,0,TO_CHAR);
    return(0);
  }

  if (ch->specials.carrying) {
    act("You can't wield a weapon whilst carrying $N.",FALSE,ch,0,ch->specials.carrying, TO_CHAR);
    return(FALSE);
  }
  
  if (ch->equipment[WIELD] && ch->equipment[HOLD]) {
    send_to_char("You need to have at least one hand free to wield anything.\r\n",ch);
    return(0);
  }
  
  if (is_two_handed(ch,weapon)) {
    if (ch->equipment[WEAR_SHIELD]){
      act("You can't wield $p and wear a shield.",FALSE,ch,weapon,0,TO_CHAR);
      return(0);}
    else if (ch->equipment[HOLD]){
      act("You need both hands to wield $p.",FALSE,ch,weapon,0,TO_CHAR);
      return(0);
    }
  }
  if (ch->equipment[HOLD] && is_held_obj_twohanded(ch,ch->equipment[HOLD])) {
    act("You cannot wield $p, $P requires two hands to hold.",FALSE,ch,weapon,
	ch->equipment[HOLD],TO_CHAR);
    return(0);
  }
  if (ch->equipment[WIELD])
    if (is_two_handed(ch,ch->equipment[WIELD])) {
      act("You cannot wield $p, $P requires two hands to wield.",FALSE,ch,weapon,
	  ch->equipment[WIELD],TO_CHAR);
      return(0);
    }
    else if (ch->equipment[WEAR_SHIELD]){
      send_to_char("You cannot wear a shield and wield two weapons.\r\n",ch);
      return(0);
    }
    else
      return(HOLD);
  return(WIELD);
}

void	silent_perform_wear(struct char_data *ch, struct obj_data *obj, int where)
{
   int wear_bitvectors[] = {
      ITEM_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
      ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, 
      ITEM_WEAR_LEGS, ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, 
      ITEM_WEAR_SHIELD, ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, 
      ITEM_WEAR_WRIST, ITEM_WIELD, ITEM_TAKE, ITEM_WEAR_EYES, ITEM_WEAR_EARS,
      ITEM_WEAR_EARS,ITEM_WEAR_MOUTH,ITEM_WEAR_BACK};



   /* first, make sure that the wear position is valid. */
   if (!CAN_WEAR(obj, wear_bitvectors[where])) {
      return;
   }

   /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
   if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R) || (where == WEAR_EARS_R))
      if (ch->equipment[where])
         where++;
   if (where == WEAR_WAIST)
     if (ch->equipment[where])
       where = WEAR_WAIST_2;

   if ((where == WEAR_SHIELD) &&
       (ch->equipment[WIELD]
	&& is_two_handed(ch,ch->equipment[WIELD])))
     return;
   if ((where == WEAR_SHIELD) &&
       (ch->equipment[WIELD]
	&& (ch->equipment[HOLD] 
	    && (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON))))
     return;
				 
   if ((where == WIELD) && !(where = can_wield(ch,obj)))
     return;
   if ((where == HOLD) && !can_hold(ch,obj))
     return;   
   
   if (ch->equipment[where]) {
     return;
   }

   obj_from_char(obj,1);
   clear_timer(obj);
   equip_char(ch, obj, where);
}



int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg)
{
   int where = -1;

   static char	*keywords[] = {
      "scabbard",
      "finger",
      "!RESERVED!",
      "neck",
      "!RESERVED!",
      "body",
      "head",
      "legs",
      "feet",
      "hands",
      "arms",
      "shield",
      "about",
      "waist",
      "wrist",
      "!RESERVED!",
      "!RESERVED!",
      "!RESERVED!",
      "eyes",
      "ears",
      "!RESERVED!",
      "mouth",
      "back",
      "\n"
   };

   if (!arg || !*arg) {
       if (CAN_WEAR(obj, ITEM_WIELD))           where = WIELD;
       if (CAN_WEAR(obj, ITEM_WEAR_FINGER))	where = WEAR_FINGER_R;
       if (CAN_WEAR(obj, ITEM_WEAR_NECK))	where = WEAR_NECK_1;
       if (CAN_WEAR(obj, ITEM_WEAR_BODY))	where = WEAR_BODY;
       if (CAN_WEAR(obj, ITEM_WEAR_HEAD))	where = WEAR_HEAD;
       if (CAN_WEAR(obj, ITEM_WEAR_LEGS))	where = WEAR_LEGS;
       if (CAN_WEAR(obj, ITEM_WEAR_FEET))	where = WEAR_FEET;
       if (CAN_WEAR(obj, ITEM_WEAR_HANDS))	where = WEAR_HANDS;
       if (CAN_WEAR(obj, ITEM_WEAR_ARMS))	where = WEAR_ARMS;
       if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))	where = WEAR_SHIELD;
       if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))	where = WEAR_ABOUT;
       if (CAN_WEAR(obj, ITEM_WEAR_WAIST))	where = WEAR_WAIST;
       if (CAN_WEAR(obj, ITEM_WEAR_WRIST))	where = WEAR_WRIST_R;
       if (CAN_WEAR(obj, ITEM_HOLD))            where = HOLD;
       if (CAN_WEAR(obj, ITEM_WEAR_EYES))       where = WEAR_EYES;
       if (CAN_WEAR(obj, ITEM_WEAR_EARS))       where = WEAR_EARS_R;
       if (CAN_WEAR(obj, ITEM_WEAR_MOUTH))      where = WEAR_MOUTH;
       if (CAN_WEAR(obj, ITEM_WEAR_BACK))       where = WEAR_BACK;
       if (CAN_WEAR(obj, ITEM_WEAR_SCABBARD))   where = WEAR_SCABBARD;
       
   } else {
      if ((where = search_block(arg, keywords, FALSE)) < 0) {
	 sprintf(buf, "'%s'?  What part of your body is THAT?\r\n", arg);
	 send_to_char(buf, ch);
      }
   }

   return where;
}



ACMD(do_wear)
{
   char	arg1[MAX_INPUT_LENGTH];
   char	arg2[MAX_INPUT_LENGTH];
   struct obj_data *obj, *next_obj;
   int	where, dotmode, items_worn = 0;
 
   argument_interpreter(argument, arg1, arg2);

   if (!*arg1) {
      send_to_char("Wear what?\r\n", ch);
      return;
   }

   dotmode = find_all_dots(arg1);

   if (*arg2 && (dotmode != FIND_INDIV)) {
      send_to_char("You can't specify the same body location for more than one item!\r\n", ch);
      return;
   }

   if (dotmode == FIND_ALL) {
      for (obj = ch->inventory; obj; obj = next_obj) {
	 next_obj = obj->next_content;
	 if ((where = find_eq_pos(ch, obj, 0)) >= 0) {
	     if (CAN_WEAR(obj, ITEM_WIELD)){
	       send_to_char("You might want to try wielding weapons.\r\n", ch);
	       continue;}
	     else if (abs(GET_OBJ_SIZE(obj) - GET_SIZE(ch)) > 1
		      && GET_OBJ_SIZE(obj) > GET_SIZE(ch) )
	       act("$p doesn't fit, it's too small",FALSE,ch,obj,0,TO_CHAR);
	     else if (abs(GET_OBJ_SIZE(obj) - GET_SIZE(ch)) > 1
		      && GET_OBJ_SIZE(obj) < GET_SIZE(ch) )
	       act("$p doesn't fit, it's too big",FALSE,ch,obj,0,TO_CHAR);
	     else{
	       perform_wear(ch, obj, where);
	       items_worn++;}
	     
	 }
      }
      if (!items_worn)
	send_to_char("You don't seem to have anything wearable.\r\n", ch);
   } else if (dotmode == FIND_ALLDOT) {
     if (!*arg1) {
       send_to_char("Wear all of what?\r\n", ch);
       return;
      }
     if (!(obj = get_obj_in_list_vis(ch, arg1, ch->inventory))) {
       sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
       send_to_char(buf, ch);
     } else while (obj) {
       next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
       if ((where = find_eq_pos(ch, obj, 0)) >= 0){
	 if (CAN_WEAR(obj, ITEM_WIELD)
	     && GET_OBJ_WEIGHT(obj) >
	     str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
	   {
	     send_to_char("It's too heavy for you to use.\r\n", ch);
	   }
	 else if (abs(GET_OBJ_SIZE(obj) - GET_SIZE(ch)) > 1
		  && GET_OBJ_SIZE(obj) > GET_SIZE(ch) )
	   act("$p doesn't fit, it's too small",FALSE,ch,obj,0,TO_CHAR);
	 else if (abs(GET_OBJ_SIZE(obj) - GET_SIZE(ch)) > 1
		  && GET_OBJ_SIZE(obj) < GET_SIZE(ch) )
	   act("$p doesn't fit, it's too big",FALSE,ch,obj,0,TO_CHAR);
	 else
	   perform_wear(ch, obj, where);
       }
       else
	 act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
       obj = next_obj;
     }
   } else {
     if (!(obj = get_obj_in_list_vis(ch, arg1, ch->inventory))) {
       sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
       send_to_char(buf, ch);
     }
     else {
       if ((where = find_eq_pos(ch, obj, arg2)) >= 0){
	 if (CAN_WEAR(obj, ITEM_WIELD)
	     && GET_OBJ_WEIGHT(obj) >
	     str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
	   {
	     send_to_char("It's too heavy for you to use.\r\n", ch);
	   }
	 else if (abs(GET_OBJ_SIZE(obj) - GET_SIZE(ch)) > 1
		  && GET_OBJ_SIZE(obj) > GET_SIZE(ch) )
	   act("$p doesn't fit, it's too small",FALSE,ch,obj,0,TO_CHAR);
	 else if (abs(GET_OBJ_SIZE(obj) - GET_SIZE(ch)) > 1
		  && GET_OBJ_SIZE(obj) < GET_SIZE(ch) )
	   act("$p doesn't fit, it's too big",FALSE,ch,obj,0,TO_CHAR);
	 else
	   perform_wear(ch, obj, where);}
       else if (!*arg2)
	 act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
     }
   }
}
ACMD(do_sheath)
{
  struct obj_data *obj=0, *tmp_obj=0, *weapon=0;
  int i;
  bool found=0;
  one_argument(argument, arg);
  if (!*arg) {
    if (ch->equipment[WIELD])
      weapon = ch->equipment[WIELD];
    else if ((ch->equipment[HOLD] &&
	      ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON))
      weapon = ch->equipment[HOLD];
    if (!weapon) {
      send_to_char("You can't do that, you aren't wielding anything!\r\n",ch);
      return;
    }
  }
  else {
    if (ch->equipment[WIELD] && isname(arg, ch->equipment[WIELD]->name))
      weapon = ch->equipment[WIELD];
    else if ((ch->equipment[HOLD] &&
	      ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON) &&
	     isname(arg, ch->equipment[HOLD]->name))
      weapon = ch->equipment[HOLD];
    if (!weapon) {
      sprintf(buf,"You don't seem to be wielding a %s!\r\n",arg);
      send_to_char(buf,ch);
      return;
    }
  }
  for (i=0;i< MAX_WEAR;i++)
    if (ch->equipment[i] && CAN_SEE_OBJ(ch,ch->equipment[i]) &&
	(ch->equipment[i]->obj_flags.type_flag == ITEM_SCABBARD))
      if (ch->equipment[i]->obj_flags.value[0] ==
	  obj_index[weapon->item_number].virtual &&
	  !(ch->equipment[i]->contains)) {
	found = TRUE;
	obj = ch->equipment[i];
      }
  if (!found)
    for (tmp_obj = ch->inventory; tmp_obj; tmp_obj = tmp_obj->next_content) 
      if (CAN_SEE_OBJ(ch, tmp_obj) &&
	  (tmp_obj->obj_flags.type_flag == ITEM_SCABBARD))
	if (tmp_obj->obj_flags.value[0] ==
	    obj_index[weapon->item_number].virtual && !tmp_obj->contains) {
	  found = TRUE;
	  obj= tmp_obj;
	  break;
	}
  if (!found)
    for (i=0;i< MAX_WEAR;i++)
      if (ch->equipment[i] && CAN_SEE_OBJ(ch,ch->equipment[i]) &&
	  (ch->equipment[i]->obj_flags.type_flag == ITEM_SCABBARD))
	if (ch->equipment[i]->obj_flags.value[3] ==
	    weapon->obj_flags.value[3] && !(ch->equipment[i]->contains)) {
	  found = TRUE;
	  obj = ch->equipment[i];
      }
  if (!found)
    for (tmp_obj = ch->inventory; tmp_obj; tmp_obj = tmp_obj->next_content) 
      if (CAN_SEE_OBJ(ch, tmp_obj) &&
	  (tmp_obj->obj_flags.type_flag == ITEM_SCABBARD))
	if (tmp_obj->obj_flags.value[0] ==
	    weapon->obj_flags.value[3] && !tmp_obj->contains) {
	  found = TRUE;
	  obj= tmp_obj;
	  break;
	}
  if (!found) {
    act("You don't seem to have a sheath for $p",FALSE,
	ch,weapon,0,TO_CHAR);
    return;
  }
    
  if (weapon->action_description)
    act(weapon->action_description,FALSE,
	ch,weapon,obj,TO_ROOM);
  
  if (weapon == ch->equipment[WIELD])
    tmp_obj = unequip_char(ch, WIELD);
  else if (weapon == ch->equipment[HOLD])
    tmp_obj = unequip_char(ch, HOLD);
  else
    return;
  obj_to_char(tmp_obj, ch, 1);
  perform_sheath(ch,tmp_obj, obj);
}


ACMD(do_wield)
{
   struct obj_data *obj;

   one_argument(argument, arg);

   if (!*arg)
      send_to_char("Wield what?\r\n", ch);
   else if (!(obj = get_obj_in_list(arg, ch->inventory))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
   } else {
       if (affected_by_spell(ch, SPELL_SPASMS) && !number(0,1)){
	   send_to_char("Your hands are trembling too much.\r\n",ch);
	   act("$n tries to wield $p, but $s hands shake.",FALSE,ch,obj,0,TO_ROOM);
	   perform_drop(ch,obj,SCMD_DROP,"drop",0);
	   return;
       }
       if (!CAN_WEAR(obj, ITEM_WIELD)) 
	   send_to_char("You can't wield that.\r\n", ch);
       else if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
	   send_to_char("It's too heavy for you to use.\r\n", ch);
       else{
	 if (ch->specials.carrying)
	   do_drop_mob(ch);      
	 perform_wear(ch, obj, WIELD);
       }
   }
}

ACMD(do_draw)
{
  struct obj_data *obj=0, *tmp_obj=0;
  int i;
  bool found=0;
  one_argument(argument, arg);
  
  if (!*arg) {
    for (i=0;i< MAX_WEAR;i++)
      if (ch->equipment[i] && CAN_SEE_OBJ(ch,ch->equipment[i]) &&
	  (ch->equipment[i]->obj_flags.type_flag == ITEM_SCABBARD))
	if (ch->equipment[i]->contains) {
	  found = TRUE;
	  obj = ch->equipment[i];
	}
    /*    if (!found)
      for (tmp_obj = ch->inventory; tmp_obj; tmp_obj = tmp_obj->next_content) 
	if (CAN_SEE_OBJ(ch, tmp_obj) &&
	    (tmp_obj->obj_flags.type_flag == ITEM_SCABBARD))
	  if (tmp_obj->contains) {
	    found = TRUE;
	    obj= tmp_obj;
	    break;
	  }*/
    if (!found) {
      send_to_char("You don't seem to be wearing a scabbard with a weapon.\r\n",ch);
      return;
    }
  }
  else {
    for (i=0;i< MAX_WEAR;i++)
      if (ch->equipment[i] && CAN_SEE_OBJ(ch,ch->equipment[i]) &&
	  (ch->equipment[i]->obj_flags.type_flag == ITEM_SCABBARD))
	if (ch->equipment[i]->contains
	    && isname(arg,ch->equipment[i]->contains->name)) {
	  found = TRUE;
	  obj = ch->equipment[i];
	}
    if (!found)
      for (tmp_obj = ch->inventory; tmp_obj; tmp_obj = tmp_obj->next_content) 
	if (CAN_SEE_OBJ(ch, tmp_obj) &&
	    (tmp_obj->obj_flags.type_flag == ITEM_SCABBARD))
	  if (tmp_obj->contains && isname(arg,tmp_obj->contains->name)) {
	    found = TRUE;
	    obj= tmp_obj;
	    break;
	  } 
    if (!found) {
      do_wield(ch,arg,0,0);
      return;
    }
  }

  if (!found) {
    send_to_char("You don't have a scabbard to draw from.\r\n",ch);
    return;
  }
  if (obj->action_description)
    act(obj->action_description,FALSE,ch,obj,obj->contains,TO_ROOM);
  perform_unsheath(ch,obj->contains,obj);
  return;
}


ACMD(do_grab)
{
   struct obj_data *obj;

   one_argument(argument, arg);

   if (!*arg)
      send_to_char("Hold what?\r\n", ch);
   else if (!(obj = get_obj_in_list(arg, ch->inventory))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
   } else {       
       if (affected_by_spell(ch, SPELL_SPASMS) && !number(0,1)){
	   send_to_char("Your hands are trembling too much.\r\n",ch);
	   act("$n tries to grab $p, but $s hands shake.",FALSE,ch,obj,0,TO_ROOM);
	   perform_drop(ch,obj,SCMD_DROP,"drop",0);
	   if (ch->specials.carrying)
	    do_drop_mob(ch);
	  return;
       }
       else if (CAN_WEAR(obj,ITEM_HOLD)){
	 if (ch->specials.carrying)
	   do_drop_mob(ch);
	 perform_wear(ch, obj, HOLD);
       }
       else
	 send_to_char("That item is not holdable.\r\n",ch);
   }
}



void	perform_remove(struct char_data *ch, int pos)
{
   struct obj_data *obj;

   if (!(obj = ch->equipment[pos])) {
      logg("Error in perform_remove: bad pos passed.");
      return;
   }
   
   if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
     act("You can't remove $p, it must be CURSED!",FALSE,ch,obj,0,TO_CHAR);
     return;
   }
   if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
     act("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
   else {
     obj_to_char(unequip_char(ch, pos), ch, 1);
     
     act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
     act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);
   }
}



ACMD(do_remove)
{
   struct obj_data *obj;
   int	i, dotmode, found, nfound;

   one_argument(argument, arg);

   if (!*arg) {
      send_to_char("Remove what?\r\n", ch);
      return;
   }
   nfound = 0;
   if (atoi(arg) > 0 && atoi(arg) < MAX_WEAR)
       for (i = 0; i < MAX_WEAR; i++)
         if (ch->equipment[i]) {
	     nfound++;
	     if (nfound == atoi(arg)){
		 perform_remove(ch, i);
		 return;
	     }
	 }
   dotmode = find_all_dots(arg);

   if (dotmode == FIND_ALL) {
      found = 0;
      for (i = 0; i < MAX_WEAR; i++)
         if (ch->equipment[i]) {
            perform_remove(ch, i);
	    found = 1;
	 }
      if (!found)
	 send_to_char("You're not using anything.\r\n", ch);
   } else if (dotmode == FIND_ALLDOT) {
      if (!*arg)
         send_to_char("Remove all of what?\r\n", ch);
      else {
         found = 0;
         for (i = 0; i < MAX_WEAR; i++)
            if (ch->equipment[i] && CAN_SEE_OBJ(ch, ch->equipment[i]) &&
	     isname(arg, ch->equipment[i]->name)) {
	       perform_remove(ch, i);
	       found = 1;
	    }
	 if (!found) {
	    sprintf(buf, "You don't seem to be using any %ss.\r\n", arg);
	    send_to_char(buf, ch);
	 }
      }
   } else {
      if (!(obj = get_object_in_equip_vis(ch, arg, ch->equipment, &i))) {
	 sprintf(buf, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
	 send_to_char(buf, ch);
      } else
	 perform_remove(ch, i);
   }
}
bool is_held_obj_twohanded(struct char_data *ch, struct obj_data *obj)
{

   if (IS_SET(obj->obj_flags.extra_flags,ITEM_2HANDED))
	return TRUE;
    if (( GET_SIZE(ch) > GET_OBJ_SIZE(obj) + 1)
	&& !IS_SET(obj->obj_flags.extra_flags,ITEM_2HANDED))
	return TRUE;
    else  if (( GET_SIZE(ch) > GET_OBJ_SIZE(obj) + 2)
	      && !IS_SET(obj->obj_flags.extra_flags,ITEM_2HANDED))
	return TRUE;
    return FALSE;
}

bool is_two_handed(struct char_data *ch, struct obj_data *weapon)
{
    if (IS_SET(weapon->obj_flags.extra_flags,ITEM_2HANDED))
	return TRUE;
    if (( GET_SIZE(ch) > GET_OBJ_SIZE(weapon) + 1)
	&& !IS_SET(weapon->obj_flags.extra_flags,ITEM_2HANDED)
	&& weapon->obj_flags.value[3] != TYPE_PIERCE )
	return TRUE;
    else  if (( GET_SIZE(ch) > GET_OBJ_SIZE(weapon) + 2)
	      && !IS_SET(weapon->obj_flags.extra_flags,ITEM_2HANDED)
	      && weapon->obj_flags.value[3] == TYPE_PIERCE )
	return TRUE;
    return FALSE;
}















