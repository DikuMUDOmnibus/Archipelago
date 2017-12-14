/* ************************************************************************
*   File: act.obj1.c                                    Part of CircleMUD *
*  Usage: object handling routines -- get/drop and container handling     *
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

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

#define MARTHA_VNUM 2059
#define DONATE_RNUM 2024

/* extern variables */
extern struct str_app_type str_app[];
extern int top_of_world;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct char_data *martha;
extern struct obj_data *don_box;
extern struct dual_list_type nonreg_plurals[];
struct char_data *give_find_vict(struct char_data *ch, char *arg1);
void   clear_timer(struct obj_data *object);
char   *report_cost(int gold);
void   drop_excess_gold(struct char_data *ch, int amount);
int    report_money_weight(int amount);
int    report_pennies(int amount);
int    report_groats(int amount);
int    report_crowns(int amount);
bool   is_plural(char *arg);
char   *make_plural(char *arg);
void	perform_drop_gold(struct char_data *ch, int amount,
			  byte mode, short RDR);
int	perform_drop(struct char_data *ch, struct obj_data *obj,
		     byte mode, char *sname, short RDR);
void   hitch(struct char_data *ch, struct obj_data *cart, struct char_data *vict, char quiet);
void   unhitch(struct char_data *ch, struct obj_data *cart, struct char_data *vict, char quiet);
bool do_get_mob(struct char_data *ch, struct char_data *victim);
void do_drop_mob(struct char_data *ch);
void   perform_sheath(struct char_data *ch, struct obj_data *weapon,
		 struct obj_data *scabbard)
{
  if (GET_ITEM_TYPE(weapon) != ITEM_WEAPON) {
    send_to_char("You can only put weapons in sheaths.\r\n",ch);
    return;
  }
  if (IS_GODITEM(weapon) && !IS_GODITEM(scabbard)) {
    act("$p is too holy to go in $P.",
	FALSE, ch, weapon, scabbard, TO_CHAR);
    return;
  }
  if (scabbard->contains) {
    act("$P seems to have a weapon in it already.",
	FALSE, ch, weapon, scabbard, TO_CHAR);
    return;
  }
  if (scabbard->obj_flags.value[0] > 0) {
    if (scabbard->obj_flags.value[0] != obj_index[weapon->item_number].virtual) {
      act("$p does not belong in $P.",
	FALSE, ch, weapon, scabbard, TO_CHAR);
      return;
    }
  }
  else if (scabbard->obj_flags.value[3] != weapon->obj_flags.value[3]) {
    act("That sort of weapon does not belong in this kind of sheath.",
	FALSE, ch, weapon, scabbard, TO_CHAR);
    return;}

  else if (GET_OBJ_SIZE(scabbard) > GET_OBJ_SIZE(weapon)) {
    act("$p is too big to fit in $P.",
	FALSE, ch, weapon, scabbard, TO_CHAR);
    return;
  }
  
  /*  else if (GET_OBJ_SIZE(scabbard) < GET_OBJ_SIZE(weapon)) {
    act("$p is too small to fit in $P.",
	FALSE, ch, weapon, scabbard, TO_CHAR);
    return;
  }*/
  
  obj_from_char(weapon,0);
  clear_timer(weapon);
  obj_to_obj (weapon, scabbard);
  act("You slip $p into $P.", FALSE, ch, weapon, scabbard, TO_CHAR);
  act("$n slips $p into $P.", FALSE, ch, weapon, scabbard, TO_ROOM);  
}

void   perform_put(struct char_data *ch, struct obj_data *obj,
		 struct obj_data *container)
{
  if (GET_ITEM_TYPE(container) == ITEM_SCABBARD) {
    perform_sheath(ch,obj,container);
    return;
  }
  if (GET_OBJ_WEIGHT(container) + GET_OBJ_WEIGHT(obj) > container->obj_flags.value[0])
    act("$p won't fit in $P.", FALSE, ch, obj, container, TO_CHAR);
  else {
    obj_from_char(obj,0);
    clear_timer(obj);
    obj_to_obj (obj, container);
    act("You put $p in $P.", FALSE, ch, obj, container, TO_CHAR);
    act("$n puts $p in $P.", TRUE, ch, obj, container, TO_ROOM);
  }
}

   
/* The following put modes are supported by the code below:

	1) put <object> <container>
	2) put all.<object> <container>
	3) put all <container>

	<container> must be in inventory or on ground.
	all objects to be put into container must be in inventory.
*/

ACMD(do_put)
{
   char	arg1[MAX_INPUT_LENGTH];
   char	arg2[MAX_INPUT_LENGTH];
   struct obj_data *obj;
   struct obj_data *next_obj;
   struct obj_data *container;
   struct char_data *tmp_char;
   int	obj_dotmode, cont_dotmode;

   argument_interpreter(argument, arg1, arg2);
   obj_dotmode = find_all_dots(arg1);
   cont_dotmode = find_all_dots(arg2);

   if (cont_dotmode != FIND_INDIV)
      send_to_char("You can only put things into one container at a time.\r\n", ch);
   else if (!*arg1)
      send_to_char("Put what in what?\r\n", ch);
   else if (!*arg2) {
      sprintf(buf, "What do you want to put %s in?\r\n",
	      ((obj_dotmode != FIND_INDIV) ? "them" : "it"));
      send_to_char(buf, ch);
   } else {
      generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &container);
      if (!container) {
	 sprintf(buf, "You don't see a %s here.\r\n", arg2);
	 send_to_char(buf, ch);
      } else if ((GET_ITEM_TYPE(container) != ITEM_CONTAINER) && 
		 (GET_ITEM_TYPE(container) != ITEM_SCABBARD)) {
	 act("You cannot put things in $p.", FALSE, ch, container, 0, TO_CHAR);
      } else if (IS_SET(container->obj_flags.value[1], CONT_CLOSED)) {
	 send_to_char("You'd better open it first!\r\n", ch);
      } else {
	 if (obj_dotmode == FIND_ALL) {	    /* "put all <container>" case */
	    /* check and make sure the guy has something first */
	    if (container == ch->inventory && !ch->inventory->next_content)
	       send_to_char("You don't seem to have anything to put in it.\r\n", ch);
	    else for (obj = ch->inventory; obj; obj = next_obj) {
	       next_obj = obj->next_content;
	       if (obj != container)
		  perform_put(ch, obj, container);
	    }
	 } else if (obj_dotmode == FIND_ALLDOT) {  /* "put all.x <cont>" case */
	    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->inventory))) {
		if (is_plural(arg1))
		    sprintf(buf, "You don't seem to have any %s.\r\n", arg1);
		else
		    sprintf(buf, "You don't seem to have any %s.\r\n", make_plural(arg1)); 
	       send_to_char(buf, ch);
	    } else while (obj) {
	       next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
	       if (obj != container)
		  perform_put(ch, obj, container);
	       obj = next_obj;
	    }
	 } else {		    /* "put <thing> <container>" case */
	    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->inventory))) {
	       sprintf(buf, "You aren't carrying %s %s.\r\n", AN(arg1), arg1);
	       send_to_char(buf, ch);
	    } else if (obj == container)
	       send_to_char("You attempt to fold it into itself, but fail.\r\n", ch);
	    else
	       perform_put(ch, obj, container);
	 }
      }
   }
}



int	can_take_obj(struct char_data *ch, struct obj_data *obj)
{
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
	if (obj->description){
	sprintf(buf, "%s: You can't carry that many items.\r\n",OBJS(obj, ch));
	send_to_char(buf, ch);}
	return 0;}
    else if (!(CAN_WEAR(obj, ITEM_TAKE))) {
	if (obj->description){
	    sprintf(buf, "%s: You can't take that!\r\n", OBJS(obj, ch));
	    send_to_char(buf, ch);}
	return 0;}
    else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
	if (obj->description){
	    sprintf(buf,"%s: You can't carry that much weight.\r\n",OBJS(obj, ch));
	    send_to_char(buf, ch);}
	return 0;}
    else if (IS_GODITEM(obj) && GET_LEVEL(ch) < LEVEL_BUILDER){
	if (obj->description){
	    sprintf(buf, "%s: You aren't holy enough to use this item.\r\n", OBJS(obj, ch));
	    send_to_char(buf, ch);}
	return 0;			    		     
    }
    
    return 1;
}


void	get_check_money(struct char_data *ch, struct obj_data *obj)
{
    int amount;
    
    amount = obj->obj_flags.value[0];
    if ((GET_ITEM_TYPE(obj) == ITEM_MONEY) && (amount > 0)) {
	obj_from_char(obj,1);
	if (amount > 1 && amount != 1000 && amount != 100 && amount != 10) {
	    sprintf(buf, "There were %s.\r\n", report_cost(amount));
	    send_to_char(buf, ch);
	}
	else if (amount > 1){
	    sprintf(buf, "There was %s.\r\n", report_cost(amount));
	    send_to_char(buf, ch);
	}
	drop_excess_gold(ch, amount);
	extract_obj(obj,0);
    }
}
void	perform_unsheath(struct char_data *ch, struct obj_data *weapon,
				   struct obj_data *scabbard)
{
  int where;

  if (!(where = can_wield(ch, weapon)))
    return;
  obj_from_obj(weapon);
  clear_timer(weapon);
  if (ch->specials.carrying)
    do_drop_mob(ch);      
  act ("You slide $p from $P.",FALSE,ch,weapon, scabbard,TO_CHAR);
  act ("$n slides $p from $P.",FALSE,ch,weapon, scabbard,TO_ROOM);
  if (affected_by_spell(ch, SPELL_SPASMS) && !number(0,1)){
    send_to_char("Your hands are trembling too much.\r\n",ch);
    act("$n tries to wield $p, but $s hands shake.",FALSE,ch,weapon,0,TO_ROOM);
    obj_to_char(weapon, ch, 1);
    perform_drop(ch,weapon,SCMD_DROP,"drop",0);
    return;
  }
  
  equip_char(ch, weapon, where);
  wear_message(ch, weapon, where);
}

void	perform_get_from_container(struct char_data *ch, struct obj_data *obj,
				   struct obj_data *cont, int mode)
{
  if (GET_ITEM_TYPE(cont) == ITEM_SCABBARD) {
    perform_unsheath(ch,obj, cont);
    return;
  }
  if (mode == FIND_OBJ_INV || can_take_obj(ch, obj)) {
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      sprintf(buf, "%s: You can't hold any more items.\r\n", OBJS(obj, ch));
    else 
      if (can_take_obj(ch,obj))
	{
	  obj_from_obj(obj);
	  obj_to_char(obj, ch, 0);
	  act("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
	  act("$n gets $p from $P.", TRUE, ch, obj, cont, TO_ROOM);
	  get_check_money(ch, obj);
	}
    
  }
}

void	get_from_container(struct char_data *ch, struct obj_data *cont,
			   char *arg, int mode)
{
  struct obj_data *obj, *next_obj;
  int obj_dotmode, found = 0, to_room;

  obj_dotmode = find_all_dots(arg);
  to_room = real_room(cont->obj_flags.value[3]);
  if (IS_SET(cont->obj_flags.value[1], CONT_CLOSED)){
    act("The $p is closed.", FALSE, ch, cont, 0, TO_CHAR);
    return;
  }
  if (obj_dotmode == FIND_ALL) {
    if (HASROOM(cont))
      {
	for (obj = world[to_room].contents;obj;){
	  next_obj = obj->next_content;
	  if (CAN_SEE_OBJ(ch,obj)){
	    found = 1;
	    if (can_take_obj(ch,obj))
	      {
		GET_OBJ_WEIGHT(cont)
		  = MAX(10, GET_OBJ_WEIGHT(cont)-
			GET_OBJ_WEIGHT(obj));
		act("$N gets $p.",TRUE,0,obj,ch,TO_ROOM); 
		obj_from_room(obj);
		obj_to_char(obj, ch, 0);
		act("$n gets $p from $P.",
		    TRUE,ch,obj,cont,TO_ROOM);
		act("You get $p from $P.",
		    TRUE,ch,obj,cont,TO_CHAR); 
		get_check_money(ch,obj);
	      }
	  }
	  obj = next_obj;
		   
	}
      }
    else
      {
	for (obj = cont->contains; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (CAN_SEE_OBJ(ch, obj)) {
	    found = 1;
	    perform_get_from_container(ch, obj, cont, mode);
	  }
	}
      }
    if (!found)
      act("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
  } else if (obj_dotmode == FIND_ALLDOT) {
    if (!*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    if (HASROOM(cont))
      {
	obj = get_obj_in_list_vis(ch, arg, world[to_room].contents);
	while(obj) {
	  next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
	  if (CAN_SEE_OBJ(ch,obj)) {
	    found = 1;
	    if (can_take_obj(ch,obj))
	      {
		GET_OBJ_WEIGHT(cont)
		  = MAX(10,GET_OBJ_WEIGHT(cont)-
			GET_OBJ_WEIGHT(obj));
		act("$N gets $p.",TRUE,0,obj,ch,TO_ROOM);
		obj_from_room(obj);
		obj_to_char(obj, ch, 0);
		act("$n gets $p from $P.",
		    TRUE,ch,obj,cont,TO_ROOM);
		act("You get $p from $P.",
		    TRUE,ch,obj,cont,TO_CHAR);
		get_check_money(ch,obj);
	      }
	  }
	  obj = next_obj;
	}
      }
    else
      {
	obj = get_obj_in_list_vis(ch, arg, cont->contains);
	while (obj) {
	  next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
	  if (CAN_SEE_OBJ(ch, obj)) {
	    found = 1;
	    perform_get_from_container(ch, obj, cont, mode);
	  }
	  obj = next_obj;
	}
      }
    if (!found) {
      if (is_plural(arg))
	sprintf(buf, "You can't find any %s in $p.", arg);
      else
	sprintf(buf, "You can't find any %s in $p.", make_plural(arg));
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
    }
  }
  else
    {
      if (HASROOM(cont)){
	if (!(obj = get_obj_in_list_vis(ch, arg, world[to_room].contents)))
	  {
	    sprintf(buf, "There doesn't seem to be %s %s in $p.",
		    AN(arg), arg);
	    act(buf, FALSE, ch, cont, 0, TO_CHAR);
	  }
	else
	  if (can_take_obj(ch,obj))
	    {
	      GET_OBJ_WEIGHT(cont) = MAX(10,GET_OBJ_WEIGHT(cont)-
					 GET_OBJ_WEIGHT(obj));
	      act("$N gets $p.",TRUE,0,obj,ch,TO_ROOM);
	      obj_from_room(obj);
	      obj_to_char(obj, ch, 0);
	      act("$n gets $p from $P.",TRUE,ch,obj,cont,TO_ROOM);
	      act("You get $p from $P.",TRUE,ch,obj,cont,TO_CHAR);
	      get_check_money(ch,obj);
	    }		
      }
      else
	{
	  if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains))) {
	    sprintf(buf, "There doesn't seem to be %s %s in $p.",
		    AN(arg), arg);
	    act(buf, FALSE, ch, cont, 0, TO_CHAR);
	  }
	  else
	    perform_get_from_container(ch, obj, cont, mode);
	}
    }
}


int	perform_get_from_room(struct char_data *ch, struct obj_data *obj)
{
   if (can_take_obj(ch, obj)) {
      obj_from_room(obj);
      obj_to_char(obj, ch, 0);
      act("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
      get_check_money(ch, obj);
      return 1;
   }

   return 0;
}


void get_from_room(struct char_data *ch, char *arg)
{
   struct obj_data *obj, *next_obj;
   struct char_data *pobj;
   int	dotmode, found = 0;

   dotmode = find_all_dots(arg);

   if (dotmode == FIND_ALL) {
      for (obj = world[ch->in_room].contents; obj; obj = next_obj) {
	 next_obj = obj->next_content;
	 if (CAN_SEE_OBJ(ch, obj) && obj->description) {
	    found = 1;
	    perform_get_from_room(ch, obj);
	 }
      }
      if (!found)
	 send_to_char("There doesn't seem to be anything here.\r\n", ch);
   } else if (dotmode == FIND_ALLDOT) {
      if (!*arg) {
	 send_to_char("Get all of what?\r\n", ch);
	 return;
      }
      if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
	  if (is_plural(arg))
	      sprintf(buf, "You don't see any %s here.\r\n", arg);
	  else
	      sprintf(buf, "You don't see any %s here.\r\n", make_plural(arg));
	 send_to_char(buf, ch);
      } else while (obj) {
	 next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
	 perform_get_from_room(ch, obj);
	 obj = next_obj;
      }
   } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
	if (generic_find(arg, FIND_CHAR_ROOM, ch, &pobj, &obj))
	  if (CAN_SEE(ch, pobj)) {
	    do_get_mob(ch, pobj);
	    return;
	  }
	sprintf(buf, "You don't see %s %s here.\r\n", AN(arg), arg); 
	send_to_char(buf, ch);
      }
      else
	perform_get_from_room(ch, obj);
   }
}



ACMD(do_get)
{
   char	arg1[MAX_INPUT_LENGTH];
   char	arg2[MAX_INPUT_LENGTH];

   int	cont_dotmode, found = 0, mode;
   struct obj_data *cont, *next_cont;
   struct char_data *tmp_char;

   if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) 
      send_to_char("Your arms are already full!\r\n", ch);
   else {
      argument_interpreter(argument, arg1, arg2);
      if (!*arg1)
         send_to_char("Get what?\r\n", ch);
      else {
         if (!*arg2)
            get_from_room(ch, arg1);
         else {
	    cont_dotmode = find_all_dots(arg2);
	    if (cont_dotmode == FIND_ALL) { /* use all in inv. and on ground */
	       for(cont = ch->inventory; cont; cont = cont->next_content)
		  if ((GET_ITEM_TYPE(cont) == ITEM_CONTAINER) ||
		       (GET_ITEM_TYPE(cont) == ITEM_SCABBARD)) {
		     found = 1;
		     get_from_container(ch, cont, arg1, FIND_OBJ_INV);
		  }
	       for(cont = world[ch->in_room].contents; cont; cont = cont->next_content)
		  if (CAN_SEE_OBJ(ch, cont) && ((GET_ITEM_TYPE(cont) == ITEM_CONTAINER) || (GET_ITEM_TYPE(cont) == ITEM_SCABBARD))) {
		     found = 1;
		     get_from_container(ch, cont, arg1, FIND_OBJ_ROOM);
		  }
	       if (!found)
		  send_to_char("You can't seem to find any containers.\r\n", ch);
	    } else if (cont_dotmode == FIND_ALLDOT) {
	       if (!*arg2) {
		  send_to_char("Get from all of what?\r\n", ch);
		  return;
	       }
	       cont = get_obj_in_list_vis(ch, arg2, ch->inventory);
	       while (cont) {
		  found = 1;
		  next_cont = get_obj_in_list_vis(ch, arg2, cont->next_content);
		  if ((GET_ITEM_TYPE(cont) != ITEM_CONTAINER) &&
		      (GET_ITEM_TYPE(cont) != ITEM_SCABBARD))
		     act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
		  else
		     get_from_container(ch, cont, arg1, FIND_OBJ_INV);
		  cont = next_cont;
	       }
	       cont = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents);
	       while (cont) {
		  found = 1;
		  next_cont = get_obj_in_list_vis(ch, arg2, cont->next_content);
		  if ((GET_ITEM_TYPE(cont) != ITEM_CONTAINER) &&
		      (GET_ITEM_TYPE(cont) != ITEM_SCABBARD))
		     act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
		  else
		      get_from_container(ch, cont, arg1, FIND_OBJ_ROOM);
		  cont = next_cont;
	       }
	       if (!found) {
		   if (is_plural(arg2))
		       sprintf(buf, "You can't seem to find any %s here.\r\n", arg2);
		   else
		       sprintf(buf, "You can't seem to find any %s here.\r\n",make_plural(arg2));
		  send_to_char(buf, ch);
	       }
	    } else { /* get <items> <container> (no all or all.x) */
	       mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &cont);
	       if (!cont) {
		  sprintf(buf, "You don't have %s %s.\r\n", AN(arg2), arg2);
		  send_to_char(buf, ch);
	       }
	       else if ((GET_ITEM_TYPE(cont) != ITEM_CONTAINER) &&
			(GET_ITEM_TYPE(cont) != ITEM_SCABBARD))
		 act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	       else
		 get_from_container(ch, cont, arg1, mode);
	    }
	 }
      }
   }
}


void	perform_drop_gold(struct char_data *ch, int amount,
			  byte mode, short RDR)
{
   struct obj_data *obj;

   if (amount <= 0)
      send_to_char("Heh heh heh.. we are jolly funny today, eh?\r\n", ch);
   else if (GET_GOLD(ch) < amount)
      send_to_char("You don't have that many coins!\r\n", ch);
   else {
      if (mode != SCMD_JUNK) {
	 obj = create_money(amount);
	 if (mode == SCMD_DONATE) {
	    send_to_char("You throw some coins into the air where it disappears in a puff of smoke!\r\n", ch);
	    act("$n throws some coins into the air where it disappears in a puff of smoke!", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_room("Some money suddenly appears in mid-air, then drops to the ground!\r\n", RDR, FALSE);
	    obj_to_obj(obj, don_box);
	 } else {
	    send_to_char("You drop some money.\r\n", ch);
	    act("$n drops some money.", FALSE, ch, 0, 0, TO_ROOM);
	    obj_to_room(obj, ch->in_room, FALSE);
	 }
      } else {
	 act("$n drops some money which disappears in a puff of smoke!", FALSE, ch, 0, 0, TO_ROOM);
	 send_to_char("You drop some money which disappears in a puff of smoke!\r\n", ch);
      }
      change_gold(ch, -amount);
   }
}


#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")

int	perform_drop(struct char_data *ch, struct obj_data *obj,
		     byte mode, char *sname, short RDR)
{
   extern short donation_room_1;


   int	value=0;

   if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      sprintf(buf, "You can't %s %s, it must be CURSED!\r\n", sname, OBJS(obj, ch));
      send_to_char(buf, ch);
      return 0;
   }

   if ((mode == SCMD_DONATE) && IS_SET(obj->obj_flags.extra_flags, ITEM_NODONATE)) {
      sprintf(buf, "You can't donate %s.\r\n", OBJS(obj, ch));
      send_to_char(buf, ch);
      return 0;
   }

   sprintf(buf, "You %s %s.%s\r\n", sname, OBJS(obj, ch), VANISH(mode));
   send_to_char(buf, ch);
   sprintf(buf, "$n %ss $p.%s", sname, VANISH(mode));
   act(buf, TRUE, ch, obj, 0, TO_ROOM);

   obj_from_char(obj,0);

   switch(mode) {
   case SCMD_DROP:
     if (ch->specials.carrying)
       do_drop_mob(ch);
     obj_to_room(obj, ch->in_room, FALSE);
     return 0;
     break;
   case SCMD_DONATE:
     if(RDR == real_room(donation_room_1)){
       obj_to_char(obj,martha, 0);
       send_to_room("Martha grabs something from thin air!\r\n", RDR, FALSE);
     }
     return 0;
     break;
   case SCMD_JUNK:
     if ((GET_ITEM_TYPE(obj) == ITEM_CONTAINER) &&
	 (obj->obj_flags.value[3] < 0))
       value = MAX(10, MIN(200,obj->obj_flags.cost/4));
     
     extract_obj(obj,0);
     return value;
     break;
   default:
     logg("SYSERR: Incorrect argument passed to perform_drop");
     break;
   }
   return 0;
}



ACMD(do_drop)
{
   extern short donation_room_1;
   extern short donation_room_2;
   extern short donation_room_3;
   struct obj_data *obj, *next_obj;
   short RDR = 0;
   byte	mode = SCMD_DROP;
   int	dotmode, amount = 0;
   char	*sname;

   switch (subcmd) {
   case SCMD_JUNK:
      sname = "junk";
      mode = SCMD_JUNK;
      break;
   case SCMD_DONATE:
      sname = "donate";
      mode = SCMD_DONATE;
      switch (number(0, 5)) {
      case 0:
      case 1: RDR = real_room(donation_room_1); break;
      }
      if (RDR == NOWHERE) {
	 send_to_char("Sorry, you can't donate anything right now.\r\n", ch);
	 return;
      }
      break;
   default:
      sname = "drop";
      break;
   }

   argument = one_argument(argument, arg);

   if (!*arg) {
      sprintf(buf, "What do you want to %s?\r\n", sname);
      send_to_char(buf, ch);
      return;
   } else if (is_number(arg)) {
      amount = atoi(arg);
      argument = one_argument(argument, arg);
      if (!str_cmp("coins", arg) || !str_cmp("coin", arg) || !str_cmp("pennies", arg) || !str_cmp("penny",arg)){
	  if (amount > report_pennies(GET_GOLD(ch)))
	      send_to_char("You don't have that many pennies.\r\n",ch);
	  else
	      perform_drop_gold(ch, amount*10, mode, RDR);}
      else if (!str_cmp("groats", arg) || !str_cmp("groat", arg) ){
	  if (amount > report_groats(GET_GOLD(ch)))
	      send_to_char("You don't have that many groats.\r\n",ch);
	  else
	      perform_drop_gold(ch, amount*100, mode, RDR);}
      else if (!str_cmp("crowns", arg) || !str_cmp("crown", arg) ){
	  if (amount > report_crowns(GET_GOLD(ch)))
	      send_to_char("You don't have that many crowns.\r\n",ch);
	  else
	      perform_drop_gold(ch, amount*1000, mode, RDR);}
      else {
	  /* code to drop multiple items.  anyone want to write it? -je */
	  send_to_char("Sorry, you can't do that (yet)...\r\n", ch);
      }
      return;
   } else {
      dotmode = find_all_dots(arg);

      /* Can't junk or donate all */
      if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
         if (subcmd == SCMD_JUNK) 
	    send_to_char("Go to the dump if you want to junk EVERYTHING!\r\n", ch);
         else 
	    send_to_char("Go do the donation room if you want to donate EVERYTHING!\r\n", ch);
         return;
      }

      if (dotmode == FIND_ALL) {
	if (ch->specials.carrying)
	  do_drop_mob(ch);
	if (!ch->inventory)
	  send_to_char("You don't seem to be carrying anything.\r\n", ch);
	 else
	   for (obj = ch->inventory; obj; obj = next_obj) {
	     next_obj = obj->next_content;
	     amount += perform_drop(ch, obj, mode, sname, RDR);
	   }
      } else if (dotmode == FIND_ALLDOT) {
	 if (!*arg) {
	    sprintf(buf, "What do you want to %s all of?\r\n", sname);
	    send_to_char(buf, ch);
	    return;
	 }
	 
         if (!(obj = get_obj_in_list_vis(ch, arg, ch->inventory))) {
	     if (is_plural(arg))
		 sprintf(buf, "You don't seem to have any %s.\r\n", arg);
	     else
		 sprintf(buf, "You don't seem to have any %s.\r\n", make_plural(arg));
	    send_to_char(buf, ch);
	 }
	if (ch->specials.carrying)
	  do_drop_mob(ch);
	while (obj) {
	    next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
	    amount += perform_drop(ch, obj, mode, sname, RDR);
	    obj = next_obj;
	 }
      } else {
	if (ch->specials.carrying && isname(arg, ch->specials.carrying->player.name))
	  do_drop_mob(ch);
	else if (!(obj = get_obj_in_list_vis(ch, arg, ch->inventory))) {
	    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	    send_to_char(buf, ch);
	 } else
	    amount += perform_drop(ch, obj, mode, sname, RDR);
      }
   }

   if (amount && (subcmd == SCMD_JUNK) && (GET_LEVEL(ch)< 10) &&
       ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) < 1000)) {
     send_to_char("You have been rewarded by the gods!\r\n", ch);
     act("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
     change_gold(ch , amount*2);
     
   }
}


void	perform_give(struct char_data *ch, struct char_data *vict,
		     struct obj_data *obj)
{
    if (IS_GODITEM(obj) && GET_LEVEL(vict) < LEVEL_BUILDER){
      sprintf(buf, "%s: You aren't holy enough to use this item.\r\n", OBJS(obj, vict));
      send_to_char(buf, vict);
      sprintf(buf, "%s isn't holy enough to use this item.\r\n", GET_NAME(vict));
      send_to_char(buf, ch);      
      return;}	
    
   if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
      return;
   }

   if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
      act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
      return;
   }

   if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict) > CAN_CARRY_W(vict)) {
      act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
      return;
   }

   obj_from_char(obj,0);
   obj_to_char(obj, vict,0);
   act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
   act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
   act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);
}

/* utility function for give */
struct char_data *give_find_vict(struct char_data *ch, char *arg1)
{
   struct char_data *vict;
   char arg2[MAX_INPUT_LENGTH];

   strcpy(buf, arg1);
   argument_interpreter(buf, arg1, arg2);
   if (!*arg1) {
      send_to_char("Give what to who?\r\n", ch);
      return 0;
   } else if (!*arg2) {
      send_to_char("To who?\r\n", ch);
      return 0;
   } else if (!(vict = get_char_room_vis(ch, arg2))) {
      send_to_char("No-one by that name here.\r\n", ch);
      return 0;
   } else if (vict == ch) {
      send_to_char("What's the point of that?\r\n", ch);
      return 0;
   } else
      return vict;
}


void	perform_give_gold(struct char_data *ch, struct char_data *vict,
			  int amount)
{
    if (amount <= 0) {
	send_to_char("Heh heh heh ... we are jolly funny today, eh?\r\n", ch);
	return;
    }

    if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_GOD))) {
	send_to_char("You haven't got that many coins!\r\n", ch);
	return;
    }

    send_to_char("Ok.\r\n", ch);
    sprintf(buf, "%s gives you %s.\r\n", PERS(ch, vict), report_cost(amount));
    send_to_char(buf, vict);
    act("$n gives some money to $N.", TRUE, ch, 0, vict, TO_NOTVICT);

    drop_excess_gold(vict, amount);
    if (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_GOD))
      change_gold(ch, -amount);

    
}

ACMD(do_unhitch)
{
    ACMD(do_say);
    struct char_data *vict;
    struct obj_data *cart;
    
    one_argument(argument, arg);
    if (!*arg){
	send_to_char("Unhitch what?\r\n", ch);
	return;
    }
    if (!(vict = get_char_room_vis(ch, arg))){
	send_to_char("Unhitch who?\r\n", ch);       
	return;}
    
    else if (!(cart = IS_PULLING(vict)))
	{
	    act("$N isn't hitched to anything.\n",TRUE,ch,0,vict,TO_CHAR);
	    return;
	}
    if ((vict->master != ch && vict != ch)
	    && vict->in_room == ch->in_room
	    && (GET_LEVEL(ch) < LEVEL_BUILDER)){
	    if (IS_MOB(vict)){
		if (CAN_SPEAK(vict)){
		    act("$n slaps your wrist.",TRUE,vict,0,ch,TO_VICT);
		    act("$n slaps $N's wrist.",TRUE,vict,0,ch,TO_ROOM);
		    do_say(vict,"Hey get your own cart!",0,0);}
		else{
		    act("$n shies away from $N.",TRUE,vict,0,ch,TO_NOTVICT);
		    act("$n shies away from you.",TRUE,vict,0,ch,TO_VICT);
		}
	    }
	    else
		act("$n just tried to remove your cart.",TRUE,ch,0,vict,TO_VICT);
	    return;
	    
    }
    else
	unhitch(ch, cart, vict, 0);
    return;
    
}

void unhitch(struct char_data *ch, struct obj_data *cart, struct char_data *vict, char quiet)
{
    struct follow_type *j,*k;
    if (!quiet){
	act("You unhitch $N.",FALSE,ch,0,vict,TO_CHAR);
	act("$n unhitches $N from $p.",FALSE,ch,cart,vict,TO_ROOM);
	act("$n unhitches $p from you.",FALSE,ch,cart,vict,TO_VICT);
    }
    if (cart->pulled_by->follower == vict){
	k = cart->pulled_by;
	cart->pulled_by = k->next;
	free(k);}
    else {
	for (k = cart->pulled_by; k->next->follower != vict;k = k->next)
	    ;
	j = k->next;
	k->next = j->next;
	free(j);
    }
    vict->specials.cart = 0;
	
}

void hitch(struct char_data *ch, struct obj_data *cart, struct char_data *vict, char quiet)
{
    struct follow_type *k;
    
    CREATE(k, struct follow_type, 1);

    k->follower = vict;
    k->next = cart->pulled_by;
    cart->pulled_by = k;
    vict->specials.cart = cart;
    if (!quiet){
	act("You hitch $p to $N",TRUE,ch,cart,vict,TO_CHAR);
	act("$n hitches $p to $N",TRUE,ch,cart,vict,TO_ROOM);
	act("$n hitches $p to you",TRUE,ch,cart,vict,TO_VICT);	       
    }
}
ACMD(do_mount)
{
  ACMD(do_say);
  struct char_data *victim;
  int num, result;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("Mount what?\r\n",ch);
    return;
  }
  if (ch->specials.mount) {
    send_to_char("You should dismount before you try mounting another mount.\r\n",ch);
    return;
  }     
  victim = get_char_room_vis(ch, arg);
  if (victim) {
    
    if (victim == ch) {
      send_to_char("You attempt to clamber on your own back - to no avail.\r\n"
		   ,ch);
      return;
    }
    if (victim->specials.fighting) {
      send_to_char("You can't mount beasts in combat.\r\n",ch);
      return;
    }
    if (IS_PULLING(victim)) {
      send_to_char("You can mount beasts pulling carts.\r\n",ch);
      return;
    }
    if (victim->specials.carried_by) {
      act("$N is being carried you cannot mount $M."
	  ,FALSE,ch,0,victim,TO_CHAR);
      return;
    }
    if (ch->specials.carrying) {
      act("You had better drop $N first.",
	  FALSE,ch,0,ch->specials.carrying,TO_CHAR);
      return;
    }
    if (victim->specials.rider) {
      act("$N already has a rider.",FALSE,ch,0,victim,TO_CHAR);
      return;
    }
    if ((GET_SIZE(ch) > GET_SIZE(victim)) &&
	((GET_SIZE(ch) - GET_SIZE(victim)) > 1)) {
      act("You can't ride $N, $E's too big.",FALSE,ch,0,victim,TO_CHAR);
      return;
    }
     else if ((GET_SIZE(ch) < GET_SIZE(victim)) &&
	((GET_SIZE(victim) - GET_SIZE(ch)) > 1)) {
      act("You can't ride $N, $E's too small.",FALSE,ch,0,victim,TO_CHAR);
      return;
    }
    if ((GET_WEIGHT(ch) + IS_CARRYING_W(ch)) > CAN_CARRY_W(victim)) {
      act("You can't ride $N. You are too heavy.",FALSE,ch,0,victim,TO_CHAR); 
      return;
    }      
    if (GET_SKILL(ch, SKILL_MOUNT) < (num = number(0,31))) {
      act("$n trys to mount up on $N and falls on $s backside.",
	  FALSE,ch,0,victim,TO_ROOM);
      act("$n trys to mount up on you and falls on $s backside.",
	  FALSE,ch,0,victim,TO_VICT);
      act("You try to mount up on $N and fall on your backside.",
	  FALSE,ch,0,victim,TO_CHAR);
      GET_POS(ch) = POSITION_SITTING;
      if (num == 0 && (number(0,31) == 0)
	  && (number(0,31) > GET_SKILL(ch, SKILL_MOUNT))
	  && (GET_SKILL(ch, SKILL_MOUNT) < 30)) {
	send_to_char("You realise how you can improve your mounting technique\r\n.",ch);
	SET_SKILL(ch, SKILL_MOUNT,(GET_SKILL(ch,SKILL_MOUNT) +1));
	rmdamage(ch,GET_HEIGHT(victim)/number(5,20));
      }
      return;
    }
    if (IS_MOB(victim) && MOB_FLAGGED(victim, MOB_DOCILE) 
	&& !IS_AFFECTED(victim, AFF_CHARM))
	{
	  if (victim->master && victim->master != ch) 
	    stop_follower_quiet(victim);
	  if (!victim->master)
	    if (!circle_follow(victim, ch))
	      if ((result = add_follower(victim,ch)) == 0){
		act("$N tried to follow you but you already have too many followers."
		    ,FALSE,ch,0,victim,TO_CHAR);
		act("$n already has too many followers."
		    ,FALSE,ch,0,victim,TO_VICT);
		return;
	      }
	}
    if (victim->master != ch) {
      send_to_char("Your mount must follow you first.\r\n",ch);
      if (IS_MOB(victim)) {
	act("$n attempts to mount $N.",FALSE,ch,0,victim,TO_ROOM);
	if (!CAN_SPEAK(victim))
	  act("$N shies away from $n.",FALSE,ch,0,victim,TO_ROOM);
	else {
	  act("$N tells $n to get lost.",FALSE,ch,0,victim,TO_ROOM);
	  act("$N tells you to get lost.",FALSE,ch,0,victim,TO_CHAR);
	}
      }
      else
	act("$N just tried to mount up on your back.",
	    FALSE,victim,0,ch,TO_CHAR);
      return;
    }

    act("$n mounts up on $N's back.",FALSE,ch,0,victim,TO_NOTVICT);
    act("$n mounts up on your back.",FALSE,ch,0,victim,TO_VICT);
    act("You mount up on $N's back.",FALSE,ch,0,victim,TO_CHAR);

    if (IS_AFFECTED(ch, AFF_SNEAK))
      affect_from_char(ch, SKILL_SNEAK);
    if (IS_SET(MOB_FLAGS(victim), MOB_MEMORY))
      REMOVE_BIT(MOB_FLAGS(victim), MOB_MEMORY);
    ch->specials.mount = victim;
    victim->specials.rider = ch;
    IS_CARRYING_W(victim) += (GET_WEIGHT(ch) + IS_CARRYING_W(ch));
    return;
  }
  sprintf(buf,"You don't see %s here.\r\n",arg);
  send_to_char(buf,ch);
  return;
}

bool do_get_mob(struct char_data *ch, struct char_data *victim)
{
  ACMD(do_say);
  int num, result;

  if (ch->specials.carrying) {
    act("You are already carrying $N.",
	FALSE,ch,0,ch->specials.carrying, TO_CHAR);
    return(FALSE);
  }     
  if (victim) {
    if (victim == ch) {
      send_to_char("Don't be silly.\r\n",ch);
      return(FALSE);
    }
    if (victim->specials.fighting) {
      send_to_char("You pick people up in combat.\r\n",ch);
      return(FALSE);
    }
    if (IS_PULLING(victim)) {
      send_to_char("Try unhitching them first.\r\n",ch);
      return(FALSE);
    }
    if (victim->specials.rider) {
      act("Get $N to dismount first.",
	  FALSE,ch,0,victim->specials.rider,TO_CHAR);
      return(FALSE);
    }
    if (victim->specials.mount) {
      act("Get $N to dismount first.",FALSE,ch,0,victim,TO_CHAR);
      return(FALSE);
    }
    if (victim->specials.carried_by) {
      act("$N is already being carried.",FALSE,ch,0,victim,TO_CHAR);
      return(FALSE);
    }
    if (ch->equipment[WIELD] || ch->equipment[HOLD]) {
      send_to_char("Your hands seem to be full.\r\n",ch);
      return;
    }
    if ((GET_SIZE(ch) > GET_SIZE(victim)) &&
	     ((GET_SIZE(ch) - GET_SIZE(victim)) > 1)) {
      act("You can't carry $N, $E's too big.",FALSE,ch,0,victim,TO_CHAR);
      return(FALSE);
    }
    if ((GET_WEIGHT(victim) + IS_CARRYING_W(victim)) > CAN_CARRY_W(ch)) {
      act("You can't carry $N, $E's too heavy.",FALSE,ch,0,victim,TO_CHAR); 
      return(FALSE);
    }      
    if (IS_MOB(victim) && MOB_FLAGGED(victim, MOB_DOCILE) 
	&& !IS_AFFECTED(victim, AFF_CHARM))
	{
	  if (victim->master && victim->master != ch) 
	    stop_follower_quiet(victim);
	  if (!victim->master)
	    if (!circle_follow(victim, ch))
	      if ((result = add_follower(victim,ch)) == 0){
		act("$N tried to follow you but you already have too many followers."
		    ,FALSE,ch,0,victim,TO_CHAR);
		act("$n already has too many followers."
		    ,FALSE,ch,0,victim,TO_VICT);
		return;
	      }
	}
    if (victim->master != ch) {
      act("$N must follow you first.",FALSE,ch,0,victim, TO_CHAR);
      if (IS_MOB(victim)) {
	act("$n attempts to pick up $N.",FALSE,ch,0,victim,TO_ROOM);
	if (!CAN_SPEAK(victim))
	  act("$N shies away from $n.",FALSE,ch,0,victim,TO_ROOM);
	else {
	  act("$N tells $n to get lost.",FALSE,ch,0,victim,TO_ROOM);
	  act("$N tells you to get lost.",FALSE,ch,0,victim,TO_CHAR);
	}
      }
      else
	act("$N just tried to pick you up.",
	    FALSE,victim,0,ch,TO_CHAR);
      return(FALSE);
    }

    act("$n picks up $N.",FALSE,ch,0,victim,TO_NOTVICT);
    act("$n picks you up.",FALSE,ch,0,victim,TO_VICT);
    act("You pick $N up.",FALSE,ch,0,victim,TO_CHAR);

    if (IS_AFFECTED(ch, AFF_SNEAK))
      affect_from_char(ch, SKILL_SNEAK);
    if (IS_SET(MOB_FLAGS(victim), MOB_MEMORY))
      REMOVE_BIT(MOB_FLAGS(victim), MOB_MEMORY);
    ch->specials.carrying = victim;
    victim->specials.carried_by = ch;
    IS_CARRYING_W(ch) += (GET_WEIGHT(victim) + IS_CARRYING_W(victim));
    return(TRUE);
  }
  send_to_char("Get who?\r\n",ch);
  return(FALSE);
}
void do_drop_mob(struct char_data *ch)
{
  struct char_data *victim;

  if (victim = ch->specials.carrying) {
    victim->specials.carried_by = 0;
    ch->specials.carrying = 0;
    IS_CARRYING_W(ch) -= (GET_WEIGHT(victim)  + IS_CARRYING_W(victim));
    act("You drop $N.",FALSE,ch,0,victim, TO_CHAR);
    act("$n drops You.",FALSE,ch,0,victim, TO_VICT);    
    act("$n drops $N.",FALSE,ch,0,victim, TO_NOTVICT);    
  }
}
void unmount(struct char_data *ch) {
  struct char_data *victim;
  
  ch->specials.mount->specials.rider = 0;
  victim = ch->specials.mount;
  ch->specials.mount = 0;
  IS_CARRYING_W(victim) -= (GET_WEIGHT(ch) + IS_CARRYING_W(ch));
  
}

ACMD(do_dismount)
{
  int num;

  if (!ch->specials.mount) {
    send_to_char("Dismount?  You are not even riding anything.\r\n",ch);
      return;
    }
    if (GET_SKILL(ch, SKILL_MOUNT) < (num = number(0,31))) {
      act("$n trys to dismount $N and falls on $s backside.",
	  FALSE,ch,0,ch->specials.mount,TO_ROOM);
      act("$n trys to dismount you and falls on $s backside.",
	  FALSE,ch,0,ch->specials.mount,TO_VICT);
      act("You try to dismount $N and fall on your backside.",
	  FALSE,ch,0,ch->specials.mount,TO_CHAR);
      unmount(ch);
      GET_POS(ch) = POSITION_SITTING;
      if (num == 0 && (number(0,31) == 0)
	  && (number(0,31) > GET_SKILL(ch, SKILL_MOUNT))
	  && (GET_SKILL(ch, SKILL_MOUNT) < 30)) {
	send_to_char("You realise how you can improve your dismounting technique\r\n.",ch);
	SET_SKILL(ch, SKILL_MOUNT,(GET_SKILL(ch,SKILL_MOUNT) + 1));
	rmdamage(ch,GET_HEIGHT(ch->specials.mount)/number(5,20));
      }
    }
    else {
      act("$n climbs off $N's back.",FALSE,ch,0,ch->specials.mount,TO_NOTVICT);
      act("$n climbs off your back.",FALSE,ch,0,ch->specials.mount,TO_VICT);
      act("You climb off $N's back.",FALSE,ch,0,ch->specials.mount,TO_CHAR);
      unmount(ch);
    }

}



ACMD(do_hitch)
{
    ACMD(do_say);
    struct char_data *vict;
    struct obj_data *cart;
    
    argument_interpreter(argument, arg, buf2);
    if (!*arg){
	send_to_char("Hitch what?\r\n", ch);
	return;
    }
    else if (!(cart = get_obj_in_list_vis(ch,arg,world[ch->in_room].contents)))
	{
		   if (is_plural(arg))
		       sprintf(buf, "You can't seem to find any %s here.\r\n", arg);
		   else
		       sprintf(buf, "You can't seem to find any %s here.\r\n", make_plural(arg));

	    send_to_char(buf, ch);
	    return;}
    if (!(vict = get_char_room_vis(ch, buf2))){
	send_to_char("Hitch to what?\r\n", ch);       
	return;}
    if (vict == ch){
	send_to_char("Now why would you want to do that?\r\n", ch);       
	return;}
    if (vict->specials.rider) {
      send_to_char("You can hitch carts to mounts.\r\n",ch);
      return;
    }
    if(!IS_CART(cart)){
	send_to_char("You can't hitch that!\r\n",ch);
	return;}
    if (cart && vict){
	if (IS_PULLING(vict))
	    {
		do_unhitch(ch,GET_NAME(vict),0,0);
		if (!IS_PULLING(vict)) 
		    return;
	    }
	if (vict->master != ch && vict != ch
	    && (GET_LEVEL(ch) < LEVEL_BUILDER)){	       
	    if (IS_MOB(vict)){
		if (CAN_SPEAK(vict)){
		    act("$n slaps your wrist.",TRUE,vict,0,ch,TO_VICT);
		    act("$n slaps $N's wrist.",TRUE,vict,0,ch,TO_NOTVICT);
		    do_say(vict,"Hey watch it!",0,0);}
		else {
		    act("$n shies away from $N.",TRUE,vict,0,ch,TO_NOTVICT);
		       act("$n shies away from you.",TRUE,vict,0,ch,TO_VICT);
		}
	    }
	    else
		   act("$n just tried to hitch a cart to you.",TRUE,ch,0,vict,TO_VICT);
	    return;
	}
	if(IS_NPC(vict) && !IS_SET(vict->specials2.act, MOB_SENTINEL))
	    SET_BIT(vict->specials2.act, MOB_SENTINEL);
	hitch(ch,cart,vict,0);
    }
    return;
}
ACMD(do_give)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int	amount, dotmode;
    struct char_data *vict;
    struct obj_data *obj, *next_obj;

    half_chop(argument, arg1, arg2);
    
    *arg3 = LOWER(*arg2);
    if (!*arg1)
	send_to_char("Give what to whom?\r\n", ch);
    else if (is_number(arg1)) {
	amount = atoi(arg1);
	if (!(vict = give_find_vict(ch, arg2)))
	    return;
	if (!str_cmp("coins", arg2) || !str_cmp("coin", arg2) || !str_cmp("pennies", arg2) || !str_cmp("penny",arg2))
	    perform_give_gold(ch,vict, amount*10);
	else if (!str_cmp("groats", arg2) || !str_cmp("groat", arg2) )
	    perform_give_gold(ch,vict, amount*100);
	else if (!str_cmp("crowns", arg2) || !str_cmp("crown", arg2) )
	    perform_give_gold(ch,vict, amount*1000);
	else {
	    /* code to give multiple items.  anyone want to write it? -je */
	    send_to_char("Sorry, you can't do that (yet)...\r\n", ch);
	    return;
	}
   }
    else {
       if (!(vict = give_find_vict(ch, argument)))
	   return;
       if (IS_NPC(vict) && GET_LEVEL(ch) <= LEVEL_BUILDER){
	   act("$N refuses your kind offer.",FALSE,ch,0,vict,TO_CHAR);
	   return;}
      dotmode = find_all_dots(argument);
       if (dotmode == FIND_ALL) {
	   if (!ch->inventory)
	       send_to_char("You don't seem to be holding anything.\r\n", ch);
	   else for (obj = ch->inventory; obj; obj = next_obj) {
	       next_obj = obj->next_content;
	       perform_give(ch, vict, obj);
	   }
       } else if (dotmode == FIND_ALLDOT) {
	   if (!*argument) {
	       send_to_char("All of what?\r\n", ch);
	       return;
	   }
	 if (!(obj = get_obj_in_list_vis(ch, argument, ch->inventory))) {
	     if(is_plural(argument))
		 sprintf(buf, "You don't seem to have any %s.\r\n", argument);
	     else
		 sprintf(buf, "You don't seem to have any %s.\r\n", make_plural(argument));
	     send_to_char(buf, ch);
	 } else while (obj) {
	     next_obj = get_obj_in_list_vis(ch, argument, obj->next_content);
	     perform_give(ch, vict, obj);
	    obj = next_obj;
	 }
       } else {
	   if (!(obj = get_obj_in_list_vis(ch, argument, ch->inventory))) {
	       sprintf(buf, "You don't seem to have %s %s.\r\n", AN(argument), argument);
	       send_to_char(buf, ch);
	   } else
	       perform_give(ch, vict, obj);
       }
   }
}

bool   is_plural(char *arg)
{
    char *loc;
    int i;
/* this is a really simple minded algorithm -
   checks to see if last char is an 's'
   AJN 29th Nov 94 */
    loc = arg;
    if (!*arg)
	return(FALSE);
/* check exceptions */
   for(i=0;strcmp(nonreg_plurals[i].singular,"\n");i++)
	{
	    if (!strcmp(nonreg_plurals[i].plural,arg)) /*already plural*/
		return(TRUE);
	    else if (!strcmp(nonreg_plurals[i].singular,arg))
		return(FALSE);
	}
    
    while (*loc != '\0')
	loc++;
    loc--;
    if (*loc == 's')
	return(TRUE);
    
    return(FALSE);	
}
