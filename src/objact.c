/* ************************************************************************
*   File: objact.c                                     Part of Archipelago*
*  Usage: Functions for generating interesting room bhaviour              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*  AJN Feb 96                                                             *
*  Archipelago is based on CircleMud                                      *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*  Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"

/* external structs */
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
ACMD(do_move);
ACMD(do_look);
ACMD(do_get);
ACMD(do_wear);
ACMD(do_flee);
ACMD(do_stand);
void	update_pos( struct char_data *victim );
void	die(struct char_data *ch);

void object_activity(void)
{
  register struct char_data *ch;
  register struct obj_data *obj;  

  for (ch = character_list; ch; ch = ch->next)
    if (!IS_MOB(ch)){
      for (obj = world[ch->in_room].contents; obj; obj = obj->next_content){
	if ((obj->obj_flags.type_flag == ITEM_FOUNTAIN)
	    && obj->action_description && !number(0,5))
	  act(obj->action_description,FALSE,ch,obj,obj,TO_CHAR);
	else if ((obj->obj_flags.type_flag == ITEM_CONTAINER)
		 && obj->action_description && (obj->obj_flags.value[3] < 0)
		 && !number(0,5))
	  act(obj->action_description,FALSE,ch,obj,obj,TO_CHAR);	
      }
    }
}
