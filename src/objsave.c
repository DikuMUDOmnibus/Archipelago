/* ************************************************************************
*   File: objsave.c                                     Part of CircleMUD *
*  Usage: loading/saving player objects for rent and crash-save           *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"
#include "screen.h"
/* these factors should be unique integers */
#define RENT_FACTOR 	2
#define CHEAP_FACTOR 	1
#define CRYO_FACTOR 	10

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct obj_data *obj_proto;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int	top_of_p_table;
extern int	min_rent_cost;

/* Extern functions */
ACMD(do_tell);
SPECIAL(receptionist);
SPECIAL(c_receptionist);
SPECIAL(cryogenicist);
void save_all_object_limit_data(void);
void process_equipment(struct char_data *ch);
void	silent_perform_wear(struct char_data *ch, struct obj_data *obj, int where);
int     find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);
int     report_money_weight(int amount);
char    *report_cost(int gold);
int     save_zone(int zone);
int	save_mobiles(int zone);
int	save_objects(int zone);
int	save_rooms(int zone);
int     is_goditem(struct obj_data *j);
bool    has_corpse(struct obj_data *obj);
void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
void   clear_timer(struct obj_data *object);
FILE *fd;

int	Crash_get_filename(char *orig_name, char *filename)
{
   char	*ptr, name[30];

   if (!*orig_name)
      return 0;

   strcpy(name, orig_name);
   for (ptr = name; *ptr; ptr++)
      *ptr = tolower(*ptr);
   
   switch (tolower(*name)) {
   case 'a': case 'b': case 'c': case 'd': case 'e':
      sprintf(filename, "plrobjs/A-E/%s.objs", name); break;
   case 'f': case 'g': case 'h': case 'i': case 'j':
      sprintf(filename, "plrobjs/F-J/%s.objs", name); break;
   case 'k': case 'l': case 'm': case 'n': case 'o':
      sprintf(filename, "plrobjs/K-O/%s.objs", name); break;
   case 'p': case 'q': case 'r': case 's': case 't':
      sprintf(filename, "plrobjs/P-T/%s.objs", name); break;
   case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
      sprintf(filename, "plrobjs/U-Z/%s.objs", name); break;
   default:
      sprintf(filename, "plrobjs/ZZZ/%s.objs", name); break;
   }
   
   return 1;
}

bool    has_corpse(struct obj_data *obj){
  bool found = FALSE;
  
  if (!obj)
    return(FALSE);
  if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER && obj->obj_flags.value[3] == -1)
    found = TRUE;
  if (!found)
    found = has_corpse(obj->contains);
  if (!found)
    found = has_corpse(obj->next_content);
  return(found);
}

int	Crash_delete_file(char *name)
{
   char	filename[50];
   FILE * fl;

   if (!Crash_get_filename(name, filename))
      return 0;
   if (!(fl = fopen(filename, "rb"))) {
      if (errno != ENOENT) { /* if it fails but NOT because of no file */
         sprintf(buf1, "SYSERR: deleting crash file %s (1)", filename);
         perror(buf1);
      }
      return 0;
   }
   fclose(fl);

   if (unlink(filename) < 0) {
      if (errno != ENOENT) { /* if it fails, NOT because of no file */
         sprintf(buf1, "SYSERR: deleting crash file %s (2)", filename);
         perror(buf1);
      }
   }

   return(1);
}


int	Crash_delete_crashfile(struct char_data *ch)
{
   char	fname[MAX_INPUT_LENGTH];
   struct rent_info rent;
   FILE * fl;

   if (!Crash_get_filename(GET_NAME(ch), fname))
      return 0;
   if (!(fl = fopen(fname, "rb"))) {
      if (errno != ENOENT) { /* if it fails, NOT because of no file */
         sprintf(buf1, "SYSERR: checking for crash file %s (3)", fname);
         perror(buf1);
      }
      return 0;
   }

   if (!feof(fl))
      fread(&rent, sizeof(struct rent_info), 1, fl);
   fclose(fl);

   if (rent.rentcode == RENT_CRASH)
      Crash_delete_file(GET_NAME(ch));

   return 1;
}


int	Crash_clean_file(char *name)
{
   char	fname[MAX_STRING_LENGTH], filetype[20];
   struct rent_info rent;
   extern int rent_file_timeout, crash_file_timeout;
   FILE * fl;

   if (!Crash_get_filename(name, fname))
      return 0;
   /* open for write so that permission problems will be flagged now,
      at boot time. */
   if (!(fl = fopen(fname, "r+b"))) {
      if (errno != ENOENT) { /* if it fails, NOT because of no file */
         sprintf(buf1, "SYSERR: OPENING OBJECT FILE %s (4)", fname);
         perror(buf1);
      }
      return 0;
   }

   if (!feof(fl))
      fread(&rent, sizeof(struct rent_info), 1, fl);
   fclose(fl);

   if ( (rent.rentcode == RENT_CRASH) || 
       (rent.rentcode == RENT_FORCED) || (rent.rentcode == RENT_TIMEDOUT) )  {
      if (rent.time < time(0) - (crash_file_timeout * SECS_PER_REAL_DAY)) {
	 Crash_delete_file(name);
	 switch (rent.rentcode) {
	 case RENT_CRASH   : strcpy(filetype, "crash"); break;
	 case RENT_FORCED  : strcpy(filetype, "forced rent"); break;
	 case RENT_TIMEDOUT: strcpy(filetype, "idlesave"); break;
	 default: strcpy(filetype, "UNKNOWN!"); break;
	 }
	 sprintf(buf, "    Deleting %s's %s file.", name, filetype);
	 logg(buf);
	 return 1;
      }

      /* Must retrieve rented items w/in 30 days */
   } else if (rent.rentcode == RENT_RENTED)
      if (rent.time < time(0) - (rent_file_timeout * SECS_PER_REAL_DAY)) {
	 Crash_delete_file(name);
	 sprintf(buf, "    Deleting %s's rent file.", name);
	 logg(buf);
	 return 1;
      }
   return(0);
}



void	update_obj_file(void) 
{
   int	i;

   for (i = 0; i <= top_of_p_table; i++)
      Crash_clean_file((player_table + i)->name);
   return;
}


struct obj_data	*Crash_obj2char(struct char_data *ch, void *objct, byte rent_ver) 
{
   struct obj_data *obj;
   int	j, ronum;
   if (rent_ver == 0){
       struct obj_file_elem_0 *object;
       object = (struct obj_file_elem_0 *) objct;
       
       if ((ronum = real_object(object->item_number)) > 0) {
	 obj = read_object(object->item_number, VIRTUAL, 1);
	 CREATE(obj->name, char, strlen(object->name) +1);
	 strcpy(obj->name,object->name);
	 CREATE(obj->description, char, strlen(object->description) +1);
	 strcpy(obj->description,object->description);
	 CREATE(obj->short_description, char, strlen(object->short_description) +1);
	 strcpy(obj->short_description,object->short_description);
	 CREATE(obj->action_description, char, strlen(object->action_description) +1);
	 strcpy(obj->action_description,object->action_description);
	 obj->obj_flags.value[0] = object->value[0];
	 obj->obj_flags.value[1] = object->value[1];
	 obj->obj_flags.value[2] = object->value[2];
	 obj->obj_flags.value[3] = object->value[3];
	 obj->obj_flags.value[4] = object->value[4];
	 obj->obj_flags.value[5] = object->value[5];      
	 obj->obj_flags.value[6] = object->value[6];
	 obj->obj_flags.value[7] = object->value[7];
	 if (IS_SET(obj->obj_flags.extra_flags, ITEM_TEST_ONLY))
	   SET_BIT(object->extra_flags, ITEM_TEST_ONLY);
	 obj->obj_flags.extra_flags = object->extra_flags;
	 obj->obj_flags.weight = object->weight;
	 obj->obj_flags.timer = object->timer;
	 obj->obj_flags2.bitvector_aff = 0;
	 for (j = 0; j < MAX_OBJ_AFFECT; j++)
	   obj->affected[j] = object->affected[j];
	 if (((obj->obj_flags.type_flag == ITEM_PHILTRE) ||
	      (obj->obj_flags.type_flag == ITEM_ROD) ||
	      (obj->obj_flags.type_flag == ITEM_CANTRIP)) &&
	     (obj->obj_flags.value[0] !=obj_proto[ronum].obj_flags.value[0]))
	   {
	     extract_obj(obj,0);
	     return(0);
	   }
	 if (!IS_GODITEM(obj) && is_goditem(obj))
	   SET_BIT(obj->obj_flags.extra_flags, ITEM_GODONLY); 
	 if (IS_GODITEM(obj) && (GET_LEVEL(ch) < LEVEL_BUILDER)){
	   extract_obj(obj,0);
	   return(0);}
	 obj_to_char(obj, ch, 1);
	 if (object->bag[0] >= 0)
	   silent_perform_wear(ch,obj,object->bag[0]);
	 return(obj);
       }
       else
	 return(0);
   }
   else if (rent_ver == 1){
     struct obj_file_elem_1 *object;
     object = (struct obj_file_elem_1 *) objct;
     if ((ronum = real_object(object->item_number)) > 0) {
       obj = read_object(object->item_number, VIRTUAL, 1);
       CREATE(obj->name, char, strlen(object->name) +1);
       strcpy(obj->name,object->name);
       CREATE(obj->description, char, strlen(object->description) +1);
       strcpy(obj->description,object->description);
       CREATE(obj->short_description, char, strlen(object->short_description) +1);
       strcpy(obj->short_description,object->short_description);
       CREATE(obj->action_description, char, strlen(object->action_description) +1);
       strcpy(obj->action_description,object->action_description);      
       obj->obj_flags.value[0] = object->value[0];
       obj->obj_flags.value[1] = object->value[1];
       obj->obj_flags.value[2] = object->value[2];
       obj->obj_flags.value[3] = object->value[3];
       obj->obj_flags.value[4] = object->value[4];
       obj->obj_flags.value[5] = object->value[5];      
       obj->obj_flags.value[6] = object->value[6];
       obj->obj_flags.value[7] = object->value[7];
       if (IS_SET(obj->obj_flags.extra_flags, ITEM_TEST_ONLY))
	 SET_BIT(object->extra_flags, ITEM_TEST_ONLY);	   
       obj->obj_flags.extra_flags = object->extra_flags;
       obj->obj_flags.weight = object->weight;
       obj->obj_flags.timer = object->timer;
       obj->obj_flags.bitvector = object->bitvector;
       
       for (j = 0; j < MAX_OBJ_AFFECT; j++)
	 obj->affected[j] = object->affected[j];
       obj->obj_flags2.light = object->light;
       obj->obj_flags2.no_use_timer = object->no_use_timer;
       obj->obj_flags2.aff_timer = object->aff_timer;
       if (object->aff_timer > 0)  
	 obj->obj_flags2.bitvector_aff =  obj->obj_flags.bitvector;
       add_event(1,EVENT_OBJTIMER,0,0,0,0,0,obj,0);
       if (((obj->obj_flags.type_flag == ITEM_PHILTRE) ||
	    (obj->obj_flags.type_flag == ITEM_ROD) ||
	    (obj->obj_flags.type_flag == ITEM_CANTRIP)) &&
	   (obj->obj_flags.value[0] !=obj_proto[ronum].obj_flags.value[0]))
	 {
	   extract_obj(obj,0);
	   return(0);
	 }
       if (!IS_GODITEM(obj) && is_goditem(obj))
	 SET_BIT(obj->obj_flags.extra_flags, ITEM_GODONLY); 
       if (IS_GODITEM(obj) && (GET_LEVEL(ch) < LEVEL_BUILDER)){
	 extract_obj(obj,0);
	 return(0);}
       obj_to_char(obj, ch, 1);
       if (object->bag[0] >= 0)
	 silent_perform_wear(ch,obj,object->bag[0]);
       return(obj);
     }
     else
       return(0);
   }
   else if (rent_ver == 2){
     struct obj_file_elem *object;
     object = (struct obj_file_elem *) objct;
     if ((ronum = real_object(object->item_number)) > 0) {
       obj = read_object(object->item_number, VIRTUAL, 1);
       CREATE(obj->name, char, strlen(object->name) +1);
       strcpy(obj->name,object->name);
       CREATE(obj->description, char, strlen(object->description) +1);
       strcpy(obj->description,object->description);
       CREATE(obj->short_description, char, strlen(object->short_description) +1);
       strcpy(obj->short_description,object->short_description);
       CREATE(obj->action_description, char, strlen(object->action_description) +1);
       strcpy(obj->action_description,object->action_description);      
       obj->obj_flags.value[0] = object->value[0];
       obj->obj_flags.value[1] = object->value[1];
       obj->obj_flags.value[2] = object->value[2];
       obj->obj_flags.value[3] = object->value[3];
       obj->obj_flags.value[4] = object->value[4];
       obj->obj_flags.value[5] = object->value[5];      
       obj->obj_flags.value[6] = object->value[6];
       obj->obj_flags.value[7] = object->value[7];
       if (IS_SET(obj->obj_flags.extra_flags, ITEM_TEST_ONLY))
	 SET_BIT(object->extra_flags, ITEM_TEST_ONLY);	   
       obj->obj_flags.extra_flags = object->extra_flags;
       obj->obj_flags.weight = object->weight;
       obj->obj_flags.timer = object->timer;
       obj->obj_flags.bitvector = object->bitvector;
       obj->obj_flags.bitvector = object->bitvector;
       obj->obj_flags.cost = object->cost;
       obj->obj_flags.cost_per_day = object->cost_per_day;	   
       
       for (j = 0; j < MAX_OBJ_AFFECT; j++)
	 obj->affected[j] = object->affected[j];
       obj->obj_flags2.light = object->light;
       obj->obj_flags2.no_use_timer = object->no_use_timer;
       obj->obj_flags2.aff_timer = object->aff_timer;
       if (object->aff_timer > 0)  
	 obj->obj_flags2.bitvector_aff =  obj->obj_flags.bitvector;
       add_event(1,EVENT_OBJTIMER,0,0,0,0,0,obj,0);
       if (((obj->obj_flags.type_flag == ITEM_PHILTRE) ||
	    (obj->obj_flags.type_flag == ITEM_ROD) ||
	    (obj->obj_flags.type_flag == ITEM_CANTRIP)) &&
	   (obj->obj_flags.value[0] !=obj_proto[ronum].obj_flags.value[0]))
	 {
	   extract_obj(obj,0);
	   return(0);
	 }
       if (!IS_GODITEM(obj) && is_goditem(obj))
	 SET_BIT(obj->obj_flags.extra_flags, ITEM_GODONLY); 
       if (IS_GODITEM(obj) && (GET_LEVEL(ch) < LEVEL_BUILDER)){
	 extract_obj(obj,0);
	 return(0);}
       obj_to_char(obj, ch, 1);
       if (object->bag[0] >= 0)
	 silent_perform_wear(ch,obj,object->bag[0]);
       return(obj);
     }
     else
       return(0);
   }
   
   return(obj);
}
void	Crash_listrent(struct char_data *ch, char *name) 
{
   FILE * fl;
   char	fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
   char pref[40], pref2[40], *buffz, col[10];
   struct obj_data *obj;
   struct rent_info rent;
   int nobj=0;
   byte container=0, last_cont = 0;

   if (!Crash_get_filename(name, fname))
      return;
   if (!(fl = fopen(fname, "rb"))) {
      sprintf(buf, "%s has no rent file.\r\n", name);
      send_to_char(buf, ch);
      return;
   }
   sprintf(buf, "%s\r\n", fname);
   if (!feof(fl))
      fread(&rent, sizeof(struct rent_info), 1, fl);
   switch (rent.rentcode) {
   case RENT_RENTED  :  strcat(buf, "Rent\r\n"); break;
   case RENT_CRASH   :  strcat(buf, "Crash\r\n"); break;
   case RENT_CRYO    :  strcat(buf, "Cryo\r\n");  break;
   case RENT_TIMEDOUT:
   case RENT_FORCED  : strcat(buf, "TimedOut\r\n"); break;
   default	     : strcat(buf, "Undef\r\n"); break;
   }
   if (rent.version == 0){
     struct obj_file_elem_0 object;
       while (!feof(fl)) {
	   fread(&object, sizeof(struct obj_file_elem_0), 1, fl);
	   if (ferror(fl)) {
	       fclose(fl);
	       return;
	   }
	   if (!feof(fl))
	     if (real_object(object.item_number) > 0) {
	       obj = read_object(object.item_number, VIRTUAL,0);
	       if (obj->obj_flags.value[6] > 0)
		 sprintf(col,"%s", CCRED(ch, C_NRM));
	       else
		 sprintf(col,"%s", CCWHT(ch, C_NRM));	       
	       sprintf(buf,"%s[%d] [%s%5d%s] (%s%5dau%s) %s%-20s%s\r\n",
		       buf,nobj,CCGRN(ch,C_NRM),
		       object.item_number,
		       CCNRM(ch,C_NRM),CCYEL(ch,C_NRM),
		       obj->obj_flags.cost_per_day,CCNRM(ch,C_NRM),
		       col,obj->short_description, CCNRM(ch,C_NRM));
	       extract_obj(obj,0);
	     }
	   nobj++;
       }
   }
   else if (rent.version == 1){
     struct obj_file_elem_1 object;
     while (!feof(fl)) {
       fread(&container, sizeof(byte),1,fl);
       fread(&object, sizeof(struct obj_file_elem_1), 1, fl);
       if (ferror(fl)) {
	 fclose(fl);
	 return;
       }
       bzero(pref,40);	   
       for(last_cont = 1; last_cont <= container; last_cont++)
	 sprintf(pref,"%s ", pref);
       bzero(pref2,40);
       if (container > 0)
	 sprintf(pref2,"%s%s>%s",CCBBLU(ch,C_NRM),pref,CCNRM(ch,C_NRM));
       else if (object.bag[0] >=0)
	 sprintf(pref2,"%s*%s",CCBBLU(ch,C_NRM),CCNRM(ch,C_NRM));
       else
	 sprintf(pref2,"%s"," ");
       if (!feof(fl))
	 if (real_object(object.item_number) > 0) {
	   obj = read_object(object.item_number, VIRTUAL,0);
	   if (obj->obj_flags.value[6] > 0)
	     sprintf(col,"%s", CCRED(ch, C_NRM));
	   else
	     sprintf(col,"%s", CCWHT(ch, C_NRM));	       
	   sprintf(buf,"%s[%3d]%s [%s%5d%s] (%s%5dau%s) %s%-20s%s\r\n",
		   buf,nobj,pref2,
		   CCGRN(ch,C_NRM),object.item_number
		   ,CCNRM(ch,C_NRM),CCBYEL(ch,C_NRM),
		   obj->obj_flags.cost_per_day,CCNRM(ch,C_NRM), col,
		   obj->short_description, CCNRM(ch,C_NRM));
	   extract_obj(obj,0);
	 }
       nobj++;
     }
   }
   else if (rent.version == 2){
     struct obj_file_elem object;
     while (!feof(fl)) {
       fread(&container, sizeof(byte),1,fl);
       fread(&object, sizeof(struct obj_file_elem), 1, fl);
       if (ferror(fl)) {
	 fclose(fl);
	 return;
       }
       bzero(pref,40);	   
       for(last_cont = 1; last_cont <= container; last_cont++)
	 sprintf(pref,"%s ", pref);
       bzero(pref2,40);
       if (container > 0)
	 sprintf(pref2,"%s%s>%s",CCBBLU(ch,C_NRM),pref,CCNRM(ch,C_NRM));
       else if (object.bag[0] >=0)
	 sprintf(pref2,"%s*%s",CCBBLU(ch,C_NRM),CCNRM(ch,C_NRM));
       else
	 sprintf(pref2,"%s"," ");
       if (!feof(fl))
	 if (real_object(object.item_number) > 0) {
	   obj = read_object(object.item_number, VIRTUAL,0);
	   if (obj->obj_flags.value[6] > 0)
	     sprintf(col,"%s", CCRED(ch, C_NRM));
	   else
	     sprintf(col,"%s", CCWHT(ch, C_NRM));	       
	   sprintf(buf,"%s[%3d]%s [%s%5d%s] (%s%5dau%s) %s%-20s%s\r\n",
		   buf,nobj,pref2,
		   CCGRN(ch,C_NRM),object.item_number
		   ,CCNRM(ch,C_NRM),CCBYEL(ch,C_NRM),
		   obj->obj_flags.cost_per_day,CCNRM(ch,C_NRM), col,
		   obj->short_description, CCNRM(ch,C_NRM));
	   extract_obj(obj,0);
	 }
       nobj++;
     }
   }
   fclose(fl);
   CREATE(buffz, char, strlen(buf) +1);
   strcpy(buffz, buf);
   page_string(ch->desc, buffz,1);
   free(buffz);
}



int	Crash_write_rentcode(struct char_data *ch, FILE *fl, struct rent_info *rent)
{
   if (fwrite(rent, sizeof(struct rent_info), 1, fl) < 1) {
      perror("Writing rent code.");
      return 0;
   }
   return 1;
}



int	Crash_load(struct char_data *ch)
/* return values:
	0 - successful load, keep char in rent room.
	1 - load failure or load of crash items -- put char in temple.
	2 - rented equipment lost (no $)
*/
{
   void	Crash_crashsave(struct char_data *ch);

   struct container_type {
       struct obj_data *bag;
       struct container_type *next;
   };
   
   FILE * fl;
   struct obj_data *tmp;
   char	fname[MAX_STRING_LENGTH];
   struct rent_info rent;
   int	cost,j, orig_rent_code,i, amount;
   float	num_of_days;
   struct container_type *cont=0, *tmp_cont, *tmpp_cont;
   byte container;
   byte last_contain;
   /* add money weight */

   add_weight_to_char(ch,report_money_weight(GET_GOLD(ch)));
   if (!Crash_get_filename(GET_NAME(ch), fname))
      return 1;
   if (!(fl = fopen(fname, "r+b"))) {
      if (errno != ENOENT) { /* if it fails, NOT because of no file */
         sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", fname);
         perror(buf1);
	 send_to_char("\r\n********************* NOTICE *********************\r\n"
		      "There was a problem loading your objects from disk.\r\n"
                      "Contact a God for assistance.\r\n", ch);
      }
      sprintf(buf, "%s entering game with no equipment.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
      return 1;
   }

   if (!feof(fl))
      fread(&rent, sizeof(struct rent_info), 1, fl);
   if (rent.rentcode == RENT_RENTED) {
      num_of_days = (float)(time(0) - rent.time) / SECS_PER_REAL_DAY;
      cost = (int)(rent.net_cost_per_diem * num_of_days);
      if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
	 sprintf(buf, "%s entering game, rented equipment lost (no $).",
	     GET_NAME(ch));
         mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
	 if (rent.version == 0){
	   struct obj_file_elem_0 object;   
	     while (!feof(fl)) {
		 fread(&object, sizeof(struct obj_file_elem_0),1,fl);
		 if (ferror(fl)) {
		     perror("Reading crash file: Crash_load.");
		     fclose(fl);
		     return 1;
		 }
		 if (!feof(fl))
		     tmp = Crash_obj2char(ch, &object, rent.version);
	     }
	 }
	 else if (rent.version == 1){
	       struct obj_file_elem_1 object;
		 while (!feof(fl)) {
		     fread(&container, 1,1,fl);
		     fread(&object, sizeof(struct obj_file_elem_1), 1, fl);
		     if (ferror(fl)) {
		       perror("Reading crash file: Crash_load.");
		       fclose(fl);
		       return 1;
		     }
		     if (!feof(fl))
		       tmp = Crash_obj2char(ch, &object, rent.version);
		 }
	 }
	 else if (rent.version == 2){
	       struct obj_file_elem object;
		 while (!feof(fl)) {
		     fread(&container, 1,1,fl);
		     fread(&object, sizeof(struct obj_file_elem), 1, fl);
		     if (ferror(fl)) {
		       perror("Reading crash file: Crash_load.");
		       fclose(fl);
		       return 1;
		     }
		     if (!feof(fl))
		       tmp = Crash_obj2char(ch, &object, rent.version);
		 }
	 }
	 for (j=0;j< MAX_WEAR;j++)
	     if (ch->equipment[j])
		 obj_to_char(unequip_char(ch,j), ch,1);
	 
	 /* ok now remove all the stuff and extract it */
	 for (tmp = ch->inventory; tmp;tmp = tmp->next_content){
	     obj_from_char(tmp,0);
	     extract_obj(tmp,0);}
	 
	 Crash_crashsave(ch);
	 return 2;
      }
      else {
	if (cost > GET_GOLD(ch)){
	  cost -= GET_GOLD(ch);
	  amount = GET_GOLD(ch);
	  change_gold(ch, -amount);
	  GET_BANK_GOLD(ch) -= cost;
	}
	else
	  change_gold(ch, -cost);	  
	 save_char(ch, NOWHERE);
      }
   }
   
   switch (orig_rent_code = rent.rentcode) {
   case RENT_RENTED:
      sprintf(buf, "%s un-renting and entering game.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
      break;
   case RENT_CRASH:
      sprintf(buf, "%s retrieving crash-saved items and entering game. Rent Version %d", GET_NAME(ch), rent.version);
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
      break;
   case RENT_CRYO:
      sprintf(buf, "%s un-cryo'ing and entering game.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
      break;
   case RENT_FORCED:
   case RENT_TIMEDOUT:
      sprintf(buf, "%s retrieving force-saved items and entering game.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
      break;
   default:
      sprintf(buf, "WARNING: %s entering game with undefined rent code.", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
      break;
   }
   if (rent.version == 0){
     struct obj_file_elem_0 object;
       while (!feof(fl)) {
	   fread(&object, sizeof(struct obj_file_elem_0),1,fl);
	   if (ferror(fl)) {
	       perror("Reading crash file: Crash_load.");
	       fclose(fl);
	       return 1;
	   }
	   if (!feof(fl))
	       tmp = Crash_obj2char(ch, &object, rent.version);
       }
   }
   else if (rent.version == 1){
     struct obj_file_elem_1 object;
     while (!feof(fl)) {
       fread(&container, 1,1,fl);
       fread(&object, sizeof(struct obj_file_elem_1), 1, fl);
       if (ferror(fl)) {
	 perror("Reading crash file: Crash_load.");
	 fclose(fl);
	 return 1;
       }
       if ((last_contain == container +1) && cont) {
	 for (tmp_cont = cont;
	      (tmp_cont->next && tmp_cont->next->next);
	      tmp_cont = tmp_cont->next)
	   ;
	   {
	     tmpp_cont = tmp_cont->next;
	     free(tmpp_cont);
	     tmp_cont->next = 0;
	   }
       }
       last_contain = container;
       if ((container == 0) && cont){ 
	 for (tmp_cont = cont; tmp_cont;)
	   {
	     tmpp_cont = tmp_cont->next;
	     free(tmp_cont);
	     tmp_cont = tmpp_cont;
	   }
	 cont =0;
       }
       tmp = 0;
       if (!feof(fl))
	 tmp = Crash_obj2char(ch, &object, rent.version);
       if (tmp){
	 if ((GET_ITEM_TYPE(tmp) == ITEM_CONTAINER) ||
	     (GET_ITEM_TYPE(tmp) == ITEM_SCABBARD))
	   {
	     CREATE(tmp_cont, struct container_type,1);
	     tmp_cont->bag = tmp;
	     if (cont){
	       for (tmpp_cont = cont;
		    tmpp_cont->next;
		    tmpp_cont = tmpp_cont->next)
		 ;
	       tmpp_cont->next = tmp_cont;
	     }
	     else
	       cont = tmp_cont;
	   }
	 if (container > 0 && cont){
	   i = 1;
	   tmp_cont = cont;
	   while(i < container && tmp_cont)
	     {
	       tmp_cont = tmp_cont->next;
	       i++;
	     }
	   if (tmp_cont && tmp_cont->bag && tmp != tmp_cont->bag)
	     {
	       obj_from_char(tmp, 1);
	       clear_timer(tmp);
	       obj_to_obj(tmp, tmp_cont->bag); 
	     }
	 }
	 
       }
     }
   }
   else if (rent.version == 2){
     struct obj_file_elem object;
     while (!feof(fl)) {
       fread(&container, 1,1,fl);
       fread(&object, sizeof(struct obj_file_elem), 1, fl);
       if (ferror(fl)) {
	 perror("Reading crash file: Crash_load.");
	 fclose(fl);
	 return 1;
       }
       if (container == 0 && cont){ 
	 for (tmp_cont = cont; tmp_cont;)
	   {
	     tmpp_cont = tmp_cont->next;
	     free(tmp_cont);
	     tmp_cont = tmpp_cont;
	   }
	 cont =0;
       }
       tmp = 0;
       if (!feof(fl))
	 tmp = Crash_obj2char(ch, &object, rent.version);
       if (tmp){
	 if ((GET_ITEM_TYPE(tmp) == ITEM_CONTAINER) ||
	     (GET_ITEM_TYPE(tmp) == ITEM_SCABBARD))
	   {
	     CREATE(tmp_cont, struct container_type,1);
	     tmp_cont->bag = tmp;
	     if (cont){
	       for (tmpp_cont = cont;
		    tmpp_cont->next;
		    tmpp_cont = tmpp_cont->next)
		 ;
	       tmpp_cont->next = tmp_cont;
	     }
	     else
	       cont = tmp_cont;
	   }
	 if (container > 0 && cont){
	   i = 1;
	   tmp_cont = cont;
	   while(i < container && tmp_cont)
	     {
	       tmp_cont = tmp_cont->next;
	       i++;
	     }
	   if (tmp_cont && tmp_cont->bag && tmp != tmp_cont->bag)
	     {
	       obj_from_char(tmp, 1);
	       clear_timer(tmp);
	       obj_to_obj(tmp, tmp_cont->bag); 
	     }
	 }
	 
       }
     }
   }
   
   if (cont) {/* free the container list */
     for (tmp_cont = cont; tmp_cont;)
       {
	 tmpp_cont = tmp_cont->next;
	 free(tmp_cont);
	 tmp_cont = tmpp_cont;
       }
     cont =0;
   }
   
/*   process_equipment(ch); */

   /* turn this into a crash file by re-writing the control block */
   rent.rentcode = RENT_CRASH;
   rent.version = 2;
   rent.time = time(0);
   rewind(fl);
   Crash_write_rentcode(ch, fl, &rent);

   fclose(fl);

   if ((orig_rent_code == RENT_RENTED) || (orig_rent_code == RENT_CRYO))
      return 0;
   else
      return 1;
}




int	Crash_obj2store(struct obj_data *obj, int where, int container, struct char_data *ch, FILE *fl)
{
   int	j;
   struct obj_file_elem object;
   bzero(object.action_description,1920);
   strcpy(object.name,obj->name);
   strcpy(object.description,obj->description);
   strcpy(object.short_description,obj->short_description);
   if (obj->action_description && *(obj->action_description))
       strcpy(object.action_description,obj->action_description);
   object.item_number = obj_index[obj->item_number].virtual;
   object.light    = obj->obj_flags2.light;
   object.value[0] = obj->obj_flags.value[0];
   object.value[1] = obj->obj_flags.value[1];
   object.value[2] = obj->obj_flags.value[2];
   object.value[3] = obj->obj_flags.value[3];
   object.value[4] = obj->obj_flags.value[4];
   object.value[5] = obj->obj_flags.value[5];
   object.value[6] = obj->obj_flags.value[6];
   object.value[7] = obj->obj_flags.value[7];
   object.extra_flags = obj->obj_flags.extra_flags;
   object.weight = obj->obj_flags.weight;
   object.timer = obj->obj_flags.timer;
   object.aff_timer = obj->obj_flags2.aff_timer;
   object.no_use_timer = obj->obj_flags2.no_use_timer;   
   object.bitvector = obj->obj_flags.bitvector;
   object.cost = obj->obj_flags.cost;
   object.cost_per_day = obj->obj_flags.cost_per_day;   
   object.bag[0] = where;
   for (j = 0; j < MAX_OBJ_AFFECT; j++)
      object.affected[j] = obj->affected[j];
   
   if (fwrite(&container, 1, 1, fl) < 1) {
      perror("Writing crash data Crash_obj2store");
      return 0;
   }
      if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
      perror("Writing crash data Crash_obj2store");
      return 0;
   }

   return 1;
}


int	Crash_save(struct obj_data *obj,
		   int where,
		   struct char_data *ch,
		   FILE *fp)
{
  struct obj_data *tmp;
  int	result;
  byte container=0;
  
  save_all_object_limit_data();
  if (obj) {
    container = 0;
    if (!(IS_SET(obj->obj_flags.extra_flags, ITEM_NORENT) ||
	  IS_SET(obj->obj_flags.extra_flags, ITEM_TEST_ONLY))){
      for (tmp = obj->contains; tmp; tmp = tmp->next_content)
	 GET_OBJ_WEIGHT(obj) -= GET_OBJ_WEIGHT(tmp);
      result = Crash_obj2store(obj, where, container, ch, fp);
      for (tmp = obj->contains; tmp; tmp = tmp->next_content)
	GET_OBJ_WEIGHT(obj) += GET_OBJ_WEIGHT(tmp);
      if (!result) 
	return 0;
    }
    if (obj->contains){
      container++;
      Crash_save_container(obj->contains, ch, container, fp);
    }
    Crash_save(obj->next_content, -1, ch, fp);
  }
  return TRUE;
}

int	Crash_save_container(struct obj_data *obj, struct char_data *ch, byte container, FILE *fp)
{
    int	result;
    struct obj_data *tmp;
    bool saved=FALSE;
    
    if (obj) {
      if (!(IS_SET(obj->obj_flags.extra_flags, ITEM_NORENT) ||
	    IS_SET(obj->obj_flags.extra_flags, ITEM_TEST_ONLY))){
	for (tmp = obj->contains; tmp; tmp = tmp->next_content)
	  GET_OBJ_WEIGHT(obj) -= GET_OBJ_WEIGHT(tmp);
	result = Crash_obj2store(obj, -1, container, ch, fp);
	saved=TRUE;
	for (tmp = obj->contains; tmp; tmp = tmp->next_content)
	  GET_OBJ_WEIGHT(obj) += GET_OBJ_WEIGHT(tmp);
	if (!result) 
	  return 0;
      }
      if (obj->contains){
	if (saved){
	  container++;
	  Crash_save_container(obj->contains, ch, container,fp);
	  container--;
	}
	else
	  Crash_save_container(obj->contains, ch, container,fp);	  
      }
      if (obj->next_content)
	Crash_save_container(obj->next_content, ch, container,fp);
    }
    return TRUE;
}

void	Crash_extract_objs(struct obj_data *obj)
{
  if (obj) {
    Crash_extract_objs(obj->contains);
    Crash_extract_objs(obj->next_content);
    extract_obj(obj,1);
  }
}


int	Crash_is_unrentable(struct obj_data *obj)
{
   if (!obj)
      return 0;
   if (IS_SET(obj->obj_flags.extra_flags, ITEM_NORENT) ||
       IS_SET(obj->obj_flags.extra_flags, ITEM_TEST_ONLY) ||
       obj->obj_flags.cost_per_day < 0 || 
       obj->item_number <= -1 ||
       GET_ITEM_TYPE(obj) == ITEM_TRASH ||
       GET_ITEM_TYPE(obj) == ITEM_KEY )
      return 1;
   return 0;
}


void	Crash_extract_norents(struct char_data *ch, struct obj_data **obj)
{
  struct obj_data *tmp=0;
  
  if (*obj) {
    Crash_extract_norents(ch,&(*obj)->contains);
    Crash_extract_norents(ch,&(*obj)->next_content);
    if (Crash_is_unrentable(*obj)){
      if(ch)
	act("$p disintegrates.",TRUE,ch,*obj,0,TO_ROOM);
      extract_obj(*obj,0);
    }
  }
}

struct obj_data 	*find_expensive_cont(struct obj_data *obj, int mcost)
{
  struct obj_data *max=0, *tmpo, *tmax=0;
  
  for (tmpo = obj;tmpo; tmpo = tmpo->next_content){	
    if (tmpo->contains){
      if((tmax  = find_expensive_cont(tmpo->contains, mcost))){
	max = tmax;
	mcost = max->obj_flags.cost_per_day;}
    }
    else if (tmpo->obj_flags.cost_per_day > mcost){
      max = tmpo;
      mcost = max->obj_flags.cost_per_day;
    }
  }
  return(max);
}

struct obj_data 	*find_expensive(struct char_data *ch)
{
  struct obj_data *max=0, *tmpo, *tmax=0;
  int j, mcost = 0;
  
    for (tmpo=ch->inventory;tmpo; tmpo = tmpo->next_content){
      if (tmpo->contains){
	if ((tmax = find_expensive_cont(tmpo->contains, mcost))){
	  max = tmax;
	  mcost = max->obj_flags.cost_per_day;}
      }
      else if (tmpo->obj_flags.cost_per_day > mcost){
	max = tmpo;
	mcost = max->obj_flags.cost_per_day;
      }
    }
    for (j= 0;j < MAX_WEAR; j++){
      if (ch->equipment[j]){
	if (ch->equipment[j]->contains)
	  if ((tmax=find_expensive_cont(ch->equipment[j]->contains, mcost))){
	    max = tmax;
	    mcost = max->obj_flags.cost_per_day;}
	if (ch->equipment[j]->obj_flags.cost_per_day > mcost){
	  max = ch->equipment[j];
	  mcost = max->obj_flags.cost_per_day;
	}
      }
    }
    return(max);
}
void	extract_expensive_ch(struct char_data *ch)
{
  struct obj_data *max;
  int j;
  
  if ((max = find_expensive(ch))){
    if (max && max->worn_by)
      for (j=0;j<MAX_WEAR;j++)
	if (max == ch->equipment[j]){
	  max = unequip_char(ch,j);
	  extract_obj(max,0);
	  return;
	}
    if (max && max->in_obj){
      obj_from_obj(max);
      extract_obj(max,0);
      return;
    }
    obj_from_char(max,0);
    extract_obj(max,0);
  }
}



void	Crash_calculate_idlerent(struct obj_data *obj, int *cost)
{
  
  if (obj) {
    if (IS_LIMITED(obj))
      *cost += MAX(0, obj->obj_flags.cost_per_day/4);
    else
      *cost += MAX(0, obj->obj_flags.cost_per_day/20);
    Crash_calculate_idlerent(obj->contains, cost);
    Crash_calculate_idlerent(obj->next_content, cost);
  }
}

void	Crash_idlesave(struct char_data *ch)
{
  char	buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  struct obj_data *obj;
  int	j;
  bool has_eq= FALSE;
  int	cost;
  FILE * fp;
  
  if (IS_NPC(ch)) 
    return;
  
  if (!Crash_get_filename(GET_NAME(ch), buf))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;
  cost = 0;
  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j]){
      obj = unequip_char(ch, j);
      Crash_extract_norents(ch, &obj);
      if (obj){
	Crash_calculate_idlerent(obj, &cost);
	obj_to_char(obj, ch, 1);	      
	silent_perform_wear(ch, obj, j);
      }
       }
  Crash_extract_norents(ch, &ch->inventory);   
  Crash_calculate_idlerent(ch->inventory, &cost);
  if (cost < 10)
    cost = 0;
  while (cost && cost > (GET_GOLD(ch) + GET_BANK_GOLD(ch))){
    cost = 0;
    extract_expensive_ch(ch);
       for (j = 0; j < MAX_WEAR; j++)
	 if (ch->equipment[j]){
	   obj = unequip_char(ch, j);
	   Crash_calculate_idlerent(obj, &cost);
	   obj_to_char(obj, ch, 1);	      
	   silent_perform_wear(ch, obj, j);
	 }
       Crash_calculate_idlerent(ch->inventory, &cost);
       if (cost < 10)
	 cost = 0;
  }
   
  if (!ch->inventory) {
    for (j= 0;j<MAX_WEAR;j++)
      if (ch->equipment[j]){
	has_eq = TRUE;
	break;
      }
    if (!has_eq){
      fclose(fp);
      Crash_delete_file(GET_NAME(ch));
      return;
    }
   }
  
  rent.net_cost_per_diem = cost;
  rent.version = 2;
  rent.rentcode = RENT_TIMEDOUT;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
      return;
  }
  
  if (!Crash_save(ch->inventory,-1, ch, fp)) {
    fclose(fp);
    return;
  }
  
  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j]) {
      if (!Crash_save(ch->equipment[j],j, ch, fp)) {
	fclose(fp);
	return;
      }
      obj = unequip_char(ch,j);
      obj_to_char(obj, ch, 1);
      
    }
  
  
   Crash_extract_objs(ch->inventory);
   fclose(fp);
}

void	Crash_crashsave(struct char_data *ch)
{
  char	buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int	j;
  FILE * fp;
  
  if (IS_NPC(ch)) 
    return;
  if (!Crash_get_filename(GET_NAME(ch), buf)){
    sprintf(buf,"Crash_get_filename failed for char: %s",GET_NAME(ch));
    mudlog(buf, CMP, MAX(LEVEL_IMPL, GET_LEVEL(ch)+1), TRUE);
    return;
  }
  if (!(fp = fopen(buf, "wb")))
    return;
  
  rent.rentcode = RENT_CRASH;
  rent.version = 2;
  rent.time = time(0);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->inventory, -1, ch, fp)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j]) {
      if (!Crash_save(ch->equipment[j],j, ch, fp)) {
	fclose(fp);
	return;
      }
    }
  
  fclose(fp);
  REMOVE_BIT(PLR_FLAGS(ch), PLR_CRASH);
}




void	Crash_rentsave(struct char_data *ch, int cost)
{
  char	buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  struct obj_data *obj;
  int	j;
  FILE * fp;
  
  if (IS_NPC(ch)) 
    return;
  
  if (!Crash_get_filename(GET_NAME(ch), buf))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;
  
  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j]){
      obj = unequip_char(ch, j);
      Crash_extract_norents(ch, &obj);
      if (obj){
	obj_to_char(obj, ch, 1);		      
	silent_perform_wear(ch, obj, j);
      }
    }
  Crash_extract_norents(ch, &ch->inventory);
  rent.version = 2;
  rent.net_cost_per_diem = cost;
  rent.rentcode = RENT_RENTED;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->inventory, -1, ch, fp)) {
    fclose(fp);
    return;
  }
  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j]) {
      if (!Crash_save(ch->equipment[j], j, ch, fp)) {
	fclose(fp);
	return;
      }
      obj = unequip_char(ch, j);
      obj_to_char(obj, ch, 1);
    }
  fclose(fp);
  Crash_extract_objs(ch->inventory);
}


void	Crash_cryosave(struct char_data *ch, int cost)
{
  char	buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  struct obj_data *obj;
  int	j, amount;
  FILE * fp;
  
  if (IS_NPC(ch)) 
    return;
  
  if (!Crash_get_filename(GET_NAME(ch), buf))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;
  
  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j]){
      obj = unequip_char(ch, j);
      Crash_extract_norents(ch, &obj);
      if (obj){
	obj_to_char(obj, ch, 1);		      
	silent_perform_wear(ch, obj, j);
      }
    }
  Crash_extract_norents(ch, &ch->inventory);
   if (cost > GET_GOLD(ch)){
     cost -= GET_GOLD(ch);
     amount = GET_GOLD(ch);
     change_gold(ch, -amount);
     GET_BANK_GOLD(ch) -= cost;
   }
   else
     change_gold(ch, -1*cost);
   
   
   rent.version = 2;
   rent.rentcode = RENT_CRYO;
   rent.time = time(0);
   rent.gold = GET_GOLD(ch);
   rent.account = GET_BANK_GOLD(ch);
   rent.net_cost_per_diem = 0;
   if (!Crash_write_rentcode(ch, fp, &rent)) {
      fclose(fp);
      return;
   }
   if (!Crash_save(ch->inventory, -1,ch, fp)) {
     fclose(fp);
     return;
   }
   for (j = 0; j < MAX_WEAR; j++)
     if (ch->equipment[j]) {
       if (!Crash_save(ch->equipment[j],j, ch, fp)) {
	    fclose(fp);
	    return;
       }
       obj = unequip_char(ch, j);
       obj_to_char(obj, ch, 1);
     }
   fclose(fp);
   
   Crash_extract_objs(ch->inventory);
   SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
}


/* ************************************************************************
* Routines used for the "Offer"                                           *
************************************************************************* */

int	Crash_report_unrentables(struct char_data *ch, struct char_data *recep,
struct obj_data *obj)
{
    int has_norents;
    
    if (obj) {
	if (Crash_is_unrentable(obj)) {
	    has_norents += 1;
	    sprintf(buf,"$n tells you, 'You cannot store %s.'", OBJS(obj, ch));
	    act(buf, FALSE, recep, 0, ch, TO_VICT); 
	}
	has_norents += Crash_report_unrentables(ch, recep, obj->contains);
	has_norents += Crash_report_unrentables(ch, recep, obj->next_content);
    }
    return(has_norents);
}



void	Crash_report_rent(struct char_data *ch, struct char_data *recep, 
struct obj_data *obj, long *cost, long *nitems, int display, int factor)
{
   static char	buf[256];

   if (obj) {
     if (!Crash_is_unrentable(obj)){
       (*nitems)++;
       if (!IS_LIMITED(obj)){
	 *cost += MAX(0, (obj->obj_flags.cost_per_day*factor)/40);
	 if (display) {
	   sprintf(buf, "$n tells you, '%s  for %s..'", 
		   report_cost((obj->obj_flags.cost_per_day*factor)/40), OBJS(obj, ch));
	   act(buf, FALSE, recep, 0, ch, TO_VICT);
	 }
       }
       else{
	 *cost += MAX(0, (obj->obj_flags.cost_per_day*factor)/8);
	 if (display) {
	   sprintf(buf, "$n tells you, '%s  for %s..'", 
		   report_cost((obj->obj_flags.cost_per_day*factor)/8), OBJS(obj, ch));
	   act(buf, FALSE, recep, 0, ch, TO_VICT);
	 }
       }
       
     }
     Crash_report_rent(ch, recep, obj->contains, cost, nitems, display, factor);
     Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor);
   }
}



int	Crash_offer_rent(struct char_data *ch, struct char_data *receptionist,
int display, int factor)
{
   extern int	max_obj_save;	/* change in config.c */
   char	buf[MAX_INPUT_LENGTH];
   int	i;
   long	totalcost = 0, numitems = 0, norent = 0, rent_deadline;

   norent = Crash_report_unrentables(ch, receptionist, ch->inventory);
   for (i = 0; i < MAX_WEAR; i++)
      norent += Crash_report_unrentables(ch, receptionist, ch->equipment[i]);

   totalcost = min_rent_cost * factor;

   Crash_report_rent(ch, receptionist, ch->inventory, &totalcost, &numitems, display, factor);

   for (i = 0; i < MAX_WEAR; i++)
      Crash_report_rent(ch, receptionist, ch->equipment[i], &totalcost, &numitems, display, factor);

   if (numitems > max_obj_save) {
      sprintf(buf, "$n tells you, 'Sorry, but I cannot store more than %d items.'",
          max_obj_save);
      act(buf, FALSE, receptionist, 0, ch, TO_VICT);
      return(0);
   }

   if (display) {
       if (min_rent_cost >0){
      sprintf(buf, "$n tells you, 'Plus, my %s fee..'",
          report_cost(min_rent_cost * factor));
      if (totalcost < 10)
	  totalcost = 0;
      act(buf, FALSE, receptionist, 0, ch, TO_VICT);}
      sprintf(buf, "$n tells you, 'For a total of, hmm, call it %s %s'",
          report_cost(totalcost), ((factor == RENT_FACTOR || factor == CHEAP_FACTOR) ? " per day" : ""));
      act(buf, FALSE, receptionist, 0, ch, TO_VICT);
       if (totalcost < 10)
	   totalcost = 0;
      if (totalcost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
	 act("$n tells you, '...which I see you can't afford.'",
	     FALSE, receptionist, 0, ch, TO_VICT);
	 return(0);
      } else if (factor == RENT_FACTOR || factor == CHEAP_FACTOR) {
	 if (totalcost) {
	    rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / (totalcost));
	    sprintf(buf, 
	        "$n tells you, 'You can rent for %ld day%s with the money"
	        " you have\r\non hand and in the bank.'\r\n",
	        rent_deadline, (rent_deadline > 1) ? "s" : "");
	 }
	 act(buf, FALSE, receptionist, 0, ch, TO_VICT);
      }
   }

   return(totalcost);
}



int	gen_receptionist(struct char_data *ch, int cmd, char *arg, int mode)
{
   int	cost = 0;
   struct char_data *recep = 0;
   struct char_data *tch;
   short save_room;
   short action_tabel[9] = { 23, 24, 36, 105, 106, 109, 111, 142, 147 };
   long	rent_deadline;

   ACMD(do_action);
   int	number(int from, int to);

   if (cmd <0)
       return(0);
   
   if ((!ch->desc) || IS_NPC(ch))
      return(FALSE);

   for (tch = world[ch->in_room].people; (tch) && (!recep); tch = tch->next_in_room)
      if (IS_MOB(tch) && ((mob_index[tch->nr].func == receptionist ||
			   mob_index[tch->nr].func == c_receptionist ||
			   mob_index[tch->nr].func == cryogenicist)))
	 recep = tch;
   if (!recep) {
      logg("SYSERR: Fubar'd receptionist.");
      exit(1);
   }

   if ((cmd != 92) && (cmd != 93)) {
      if (!number(0, 30))
	 do_action(recep, "", action_tabel[number(0,8)], 0);
      return(FALSE);
   }
   if (!AWAKE(recep)) {
      act("$N is unable to talk to you...",FALSE, ch,0,recep,TO_CHAR);
      return(TRUE);
   }
   if (!CAN_SEE(recep, ch) && GET_LEVEL(ch) < LEVEL_BUILDER) {
      act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
      return(TRUE);
   }

   if (cmd == 92) { /* Rent  */
     if (has_corpse(ch->inventory)){
       act("$n tells you, 'No-one keeps dead bodies in my inn.'", FALSE, recep, 0, ch, TO_VICT);
       return(TRUE);
     }
     cost = Crash_offer_rent(ch, recep, FALSE,  mode);
     if (cost == 0)
       cost++;
     if (mode == RENT_FACTOR || mode == CHEAP_FACTOR)
       sprintf(buf, "$n tells you, 'Rent will cost you %s per day.'", report_cost(cost));
     else if (mode == CRYO_FACTOR)
       sprintf(buf, "$n tells you, 'It will cost you %s to enter the monastery.'", report_cost(cost));
     act(buf, FALSE, recep, 0, ch, TO_VICT);
     if (cost < 10)
       cost = 0;
     if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) {
       act("$n tells you, '...which I see you can't afford.'",
	   FALSE, recep, 0, ch, TO_VICT);
       return(1);
     }
     
     if (cost && (mode == RENT_FACTOR)) {
       rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / (cost));
       sprintf(buf, 
	       "$n tells you, 'You can rent for %ld day%s with the money you have\r\n"
	       "on hand and in the bank.'\r\n",
	       rent_deadline, (rent_deadline > 1) ? "s" : "");
       act(buf, FALSE, recep, 0, ch, TO_VICT);
     }
     
     if (mode == RENT_FACTOR || mode == CHEAP_FACTOR) {
       act("$n stores your belongings and helps you into your private chamber.",
	   FALSE, recep, 0, ch, TO_VICT);
       Crash_rentsave(ch, cost);
       sprintf(buf, "%s has rented (%d/day, %d tot.)", GET_NAME(ch),
	       cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
     } else { /* cryo */
       act("$n stores your belongings and shows you to your cell.\r\n"
	   "$n hands you a habit and a small oil lamp...\r\n",
	   FALSE, recep, 0, ch, TO_VICT);
       act("$n tells you, 'Vespers are at seven of the clock.'",
	   FALSE, recep, 0, ch, TO_VICT);
       Crash_cryosave(ch, cost);
       sprintf(buf, "%s has cryo-rented.", GET_NAME(ch));
       SET_BIT(PLR_FLAGS(ch), PLR_CRYO);
     }
     
     mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
     act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);
     save_room = ch->in_room;
     extract_char(ch, FALSE);
     ch->in_room = world[save_room].number;
     save_char(ch, ch->in_room);
   } else {  /* Offer */
     if (has_corpse(ch->inventory)){
       act("$n tells you, 'No-one keeps dead bodies in my inn.'", FALSE, recep, 0, ch, TO_VICT);
       return(TRUE);
     }
     Crash_offer_rent(ch, recep, TRUE, mode);
     act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
   }
   return(TRUE);
}


SPECIAL(receptionist)
{
   return(gen_receptionist(ch, cmd, arg, RENT_FACTOR));
}

SPECIAL(c_receptionist)
{
   return(gen_receptionist(ch, cmd, arg, CHEAP_FACTOR));
}


SPECIAL(cryogenicist)
{
   return(gen_receptionist(ch, cmd, arg, CRYO_FACTOR));
}


void	Crash_save_all(void)
{
  int n;
  struct descriptor_data *d;
  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLYNG) && !IS_NPC(d->character)) {
      if (PLR_FLAGGED(d->character, PLR_CRASH)) {
	Crash_crashsave(d->character);
	save_char(d->character, NOWHERE);
	act("Auto-saving $n.",FALSE,d->character,0,0,TO_CHAR);
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_CRASH);
      }
      if (IS_SET(d->character->specials2.builder_flag,BUILD_ZONE)
	  && d->character->specials2.edit_zone > 0
	  && !PLR_FLAGGED(d->character, PLR_BUILDING)){
	n = save_zone(d->character->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d zone resets for zone: %ld"
		,GET_NAME(d->character),n,d->character->specials2.edit_zone);
	mudlog(buf,NRM,MAX(LEVEL_BUILDER,GET_INVIS_LEV(d->character)),TRUE);
	REMOVE_BIT(d->character->specials2.builder_flag,BUILD_ZONE);
	 }
      if (IS_SET(d->character->specials2.builder_flag,BUILD_SHOPS)
	  && d->character->specials2.edit_zone > 0
	  && !PLR_FLAGGED(d->character, PLR_BUILDING)){
	n = save_objects(d->character->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d shops for zone: %ld"
		,GET_NAME(d->character),n,d->character->specials2.edit_zone);
	mudlog(buf, NRM,MAX(LEVEL_BUILDER,GET_INVIS_LEV(d->character)), TRUE); 
	REMOVE_BIT(d->character->specials2.builder_flag,BUILD_SHOPS);
      }
      if (IS_SET(d->character->specials2.builder_flag,BUILD_MOBS)
	  && d->character->specials2.edit_zone > 0
	  && !PLR_FLAGGED(d->character, PLR_BUILDING)){
	n = save_mobiles(d->character->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d mobiles for zone: %ld"
		,GET_NAME(d->character),n,d->character->specials2.edit_zone);
	mudlog(buf,NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
	     REMOVE_BIT(d->character->specials2.builder_flag,BUILD_MOBS);	 }
      if (IS_SET(d->character->specials2.builder_flag,BUILD_ROOMS)
	  && d->character->specials2.edit_zone > 0
	  && !PLR_FLAGGED(d->character, PLR_BUILDING)){
	n = save_rooms(d->character->specials2.edit_zone);
	sprintf(buf,"(%s) Autosaving %d rooms for zone: %ld"
		,GET_NAME(d->character),n,d->character->specials2.edit_zone);
	mudlog(buf, NRM, MAX(LEVEL_BUILDER,GET_INVIS_LEV(d->character)), TRUE);
	REMOVE_BIT(d->character->specials2.builder_flag,BUILD_ROOMS);
      }
    }
  }
}









