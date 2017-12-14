/* ************************************************************************
*   File: modify.c                                      Part of CircleMUD *
*  Usage: Run-time modification of game variables                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "mail.h"
#include "objedit.h"
#include "roomedit.h"
#include "screen.h"

#define REBOOT_AT    10  /* 0-23, time of optional reboot if -e lib/reboot */


#define TP_MOB    0
#define TP_OBJ	  1
#define TP_ERROR  2

void print_mstr(struct descriptor_data *d);
void	show_string(struct descriptor_data *d, char *input);
void print_ostr(struct descriptor_data *d);
void print_exd(struct descriptor_data *d);
void print_rstr(struct descriptor_data *d);
void print_rexd(struct descriptor_data *d);
void print_tele(struct descriptor_data *d);
void print_exit(struct descriptor_data *d);
void print_messages(struct descriptor_data *d);
void print_zonedata(struct descriptor_data *d);

char	*string_fields[] = 
{
   "name",
   "short",
   "long",
   "description",
   "title",
   "delete-description",
   "\n"
};


/* maximum length for text field x+1 */
int	length[] = 
{
   15,
   60,
   256,
   240,
   60
};



char	*skill_fields[] = 
{
   "learned",
   "affected",
   "duration",
   "recognize",
   "\n"
};



int	max_value[] = 
{
   255,
   255,
   10000,
   1
};


/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void	string_add(struct descriptor_data *d, char *str)
{
   char	*scan;
   int	terminator = 0;
   extern char	*MENU;
   extern char	*ANSI_MENU;
   /* determine if this is the terminal string, and truncate if so */
   for (scan = str; *scan; scan++)
      if ((terminator = (*scan == '@' && *(scan -1) == '@'))) {
	 *(scan -1) = '\0';
	 break;
      }

   if (!(*d->str)) {
      if (strlen(str) > d->max_str) {
	 send_to_char("String too long - Truncated.\r\n",
	     d->character);
	 *(str + d->max_str) = '\0';
	 terminator = 1;
      }
      CREATE(*d->str, char, strlen(str) + 3);
      strcpy(*d->str, str);
   } else {
      if (strlen(str) + strlen(*d->str) > d->max_str) {
	 send_to_char("String too long. Last line skipped.\r\n",
	     d->character);
	 terminator = 1;
      } else	 {
	 if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) + 
	     strlen(str) + 3))) {
	    perror("string_add");
	    exit(1);
	 }
	 strcat(*d->str, str);
      }
   }

   if (terminator) {
      if (!d->connected && (PLR_FLAGGED(d->character, PLR_MAILING))) {
	 store_mail(d->name, d->character->player.name, *d->str);
	 free(*d->str);
	 free(d->str);
	 free(d->name);
	 d->name = 0;
	 SEND_TO_Q("Message sent!\r\n", d);
	 if (!IS_NPC(d->character)
	     && (PLR_FLAGGED(d->character,PLR_MAILING))){
	     act("$n finishes writing a message.",TRUE,d->character,0,0,TO_ROOM);	     
	    REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING);
	 }
      }
      d->str = 0;
      if (d->connected == CON_EXDSCR) {
	  if (d->color)
	      SEND_TO_Q(ANSI_MENU,d);
	  else
	      SEND_TO_Q(MENU, d);
	  d->connected = CON_SLCT;
      }
      if(d->mob_edit)
	  print_mstr(d);
      else if(d->obj_edit){
	  if (d->oedit_mode == OSTR_EDIT)
	      print_ostr(d);
	  else
	      print_exd(d);
      }
      else if (d->shop_edit) {
	print_messages(d);
      }
      else if (d->zone_edit) {
	print_zonedata(d);
      }      
      else if(d->room_edit){
	  if (d->redit_mode == RTEXT_EDIT)
	      print_rstr(d);
	  else if (d->redit_mode == REDD_EDIT)
	      print_rexd(d);
	  else if (d->redit_mode == RTELE_EDIT)
	      print_tele(d);	  
	  else if (d->redit_mode == REXIT_EDIT)
	      print_exit(d);
	  else if (d->redit_mode == WITH_MODE) {
	    SEND_TO_Q("With: ",d);
	    d->str = &(d->with);
	    free(*d->str);
	    *d->str = 0;
	    d->max_str = 29;
	    d->redit_mode = REPCON_MODE;
	    return;
	  }
	  else if (d->redit_mode == REPCON_MODE) {
	    sprintf(buf, "Confirm replace %s%s%s with %s%s%s (y/n): ",
		    CCGRN(d->character,C_NRM),d->replace
		    ,CCNRM(d->character,C_NRM),
		    CCGRN(d->character,C_NRM),d->with
		    ,CCNRM(d->character,C_NRM));
	    SEND_TO_Q(buf,d);
	  }
      }
	 if (!d->connected && d->character && !IS_NPC(d->character)
	     && (PLR_FLAGGED(d->character, PLR_WRITING))){
	     act("$n finishes writing a message on the board.",TRUE,d->character,0,0,TO_ROOM);
	  
	     REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
      }
   } else
      strcat(*d->str, "\r\n");
}


/* interpret an argument for do_string */
void	quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
   char	buff[MAX_STRING_LENGTH];

   /* determine type */
   arg = one_argument(arg, buff);
   if (is_abbrev(buff, "char"))
      *type = TP_MOB;
   else if (is_abbrev(buff, "obj"))
      *type = TP_OBJ;
   else {
      *type = TP_ERROR;
      return;
   }

   /* find name */
   arg = one_argument(arg, name);

   /* field name and number */
   arg = one_argument(arg, buf);
   if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
      return;

   /* string */
   for (; isspace(*arg); arg++)
      ;
   for (; (*string = *arg); arg++, string++)
      ;

   return;
}

/* modification of malloc'ed strings in chars/objects */
ACMD(do_string)
{
   char	name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
   int	field, type;
   struct char_data *mob;
   struct obj_data *obj;
   struct extra_descr_data *ed=0, *tmp=0;

   if (IS_NPC(ch))
      return;

   quad_arg(argument, &type, name, &field, string);

   if (type == TP_ERROR) {
      send_to_char("Usage: string ('obj'|'char') <name> <field> [<string>]", ch);
      return;
   }

   if (!field) {
      send_to_char("No field by that name. Try 'help string'.\r\n", ch);
      return;
   }

   if (type == TP_MOB) {
      /* locate the beast */
      if (!(mob = get_char_vis(ch, name))) {
	 send_to_char("I don't know anyone by that name...\r\n", ch);
	 return;
      }

      /*      if (IS_NPC(mob)) {
	 send_to_char("Sorry, string is disabled for mobs.\r\n", ch);
	 return; 
      }*/

      switch (field) {
      case 1:
	 if (!IS_NPC(mob)) {
	    send_to_char("You can't change that field for players.", ch);
	    return;
	 }
	 ch->desc->str = &(mob->player.name);
	 if (!IS_NPC(mob))
	    send_to_char("WARNING: You have changed the name of a player.\r\n", ch);
	 break;
      case 2:
	 if (!IS_NPC(mob)) {
	    send_to_char("That field is for monsters only.\r\n", ch);
	    return;
	 }
	 ch->desc->str = &mob->player.short_descr;
	 break;
      case 3:
	 if (!IS_NPC(mob)) {
	    send_to_char("That field is for monsters only.\r\n", ch);
	    return;
	 }
	 ch->desc->str = &mob->player.long_descr;
	 break;
      case 4:
	 ch->desc->str = &mob->player.description;
	 break;
      case 5:
	 if (IS_NPC(mob)) {
	    send_to_char("Monsters have no titles.\r\n", ch);
	    return;
	 }
	 ch->desc->str = &mob->player.title;
	 break;
      default:
	 send_to_char("That field is undefined for monsters.\r\n", ch);
	 return;
	 break;
      }
   } else {/* type == TP_OBJ 	 {
      send_to_char("Sorry, string has been disabled for objects.\r\n", ch);
      return; */

      /* locate the object */
      if (!(obj = get_obj_vis(ch, name))) {
	 send_to_char("Can't find such a thing here..\r\n", ch);
	 return;
      }

      switch (field) {
      case 1:
	 ch->desc->str = &obj->name;
	 break;
      case 2:
	 ch->desc->str = &obj->short_description;
	 break;
      case 3:
	 ch->desc->str = &obj->description;
	 break;
      case 4:
	 if (!*string) {
	    send_to_char("You have to supply a keyword.\r\n", ch);
	    return;
	 }
	 /* try to locate extra description */
	 for (ed = obj->ex_description; ; ed = ed->next)
	    if (!ed) /* the field was not found. create a new one. */ {
	       CREATE(ed , struct extra_descr_data, 1);
	       ed->next = obj->ex_description;
	       obj->ex_description = ed;
	       CREATE(ed->keyword, char, strlen(string) + 1);
	       strcpy(ed->keyword, string);
	       ed->description = 0;
	       ch->desc->str = &ed->description;
	       send_to_char("New field.\r\n", ch);
	       break;
	    }
	    else if (!str_cmp(ed->keyword, string)) /* the field exists */ {
	       free(ed->description);
	       ed->description = 0;
	       ch->desc->str = &ed->description;
	       send_to_char("Modifying description.\r\n", ch);
	       break;
	    }
	 ch->desc->max_str = MAX_STRING_LENGTH;
	 return; /* the stndrd (see below) procedure does not apply here */
	 break;
      case 6:
	 if (!*string) {
	    send_to_char("You must supply a field name.\r\n", ch);
	    return;
	 }
	 /* try to locate field */
	 for (ed = obj->ex_description; ; ed = ed->next)
	    if (!ed) {
	       send_to_char("No field with that keyword.\r\n", ch);
	       return;
	    }
	    else if (!str_cmp(ed->keyword, string)) {
	       free(ed->keyword);
	       if (ed->description)
		  free(ed->description);

	       /* delete the entry in the desr list */
	       if (ed == obj->ex_description)
		  obj->ex_description = ed->next;
	       else {
		  for (tmp = obj->ex_description; tmp->next != ed; 
		      tmp = tmp->next)
		     ;
		  tmp->next = ed->next;
	       }
	       free(ed);

	       send_to_char("Field deleted.\r\n", ch);
	       return;
	    }
	 break;
      default:
	 send_to_char(
	     "That field is undefined for objects.\r\n", ch);
	 return;
	 break;
      }
   }

   if (*ch->desc->str) {
      free(*ch->desc->str);
   }

   if (*string)   /* there was a string in the argument array */ {
      if (strlen(string) > length[field - 1]) {
	 send_to_char("String too long - truncated.\r\n", ch);
	 *(string + length[field - 1]) = '\0';
      }
      CREATE(*ch->desc->str, char, strlen(string) + 1);
      strcpy(*ch->desc->str, string);
      ch->desc->str = 0;
      send_to_char("Ok.\r\n", ch);
   } else /* there was no string. enter string mode */	 {
      send_to_char("Enter string. terminate with '@@'.\r\n", ch);
      *ch->desc->str = 0;
      ch->desc->max_str = length[field - 1];
   }
}

/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char	*one_word(char *argument, char *first_arg )
{
   int	found, begin, look_at;

   found = begin = 0;

   do {
      for ( ; isspace(*(argument + begin)); begin++)
	 ;

      if (*(argument + begin) == '\"') {  /* is it a quote */

	 begin++;

	 for (look_at = 0; (*(argument + begin + look_at) >= ' ') && 
	     (*(argument + begin + look_at) != '\"') ; look_at++)
	    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

	 if (*(argument + begin + look_at) == '\"')
	    begin++;

      } else {

	 for (look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)
	    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

      }

      *(first_arg + look_at) = '\0';
      begin += look_at;
   } while (fill_word(first_arg));

   return(argument + begin);
}


struct help_index_element *build_help_index(FILE *fl, int *num)
{
   int	nr = -1, issorted, i;
   struct help_index_element *list = 0, mem;
   char	buf[81], tmp[81], *scan;
   long	pos;

   for (; ; ) {
      pos = ftell(fl);
      fgets(buf, 81, fl);
      *(buf + strlen(buf) - 1) = '\0';
      scan = buf;
      for (; ; ) {
	 /* extract the keywords */
	 scan = one_word(scan, tmp);

	 if (!*tmp)
	    break;

	 if (!list) {
	    CREATE(list, struct help_index_element, 1);
	    nr = 0;
	 } else
	    RECREATE(list, struct help_index_element, ++nr + 1);

	 list[nr].pos = pos;
	 CREATE(list[nr].keyword, char, strlen(tmp) + 1);
	 strcpy(list[nr].keyword, tmp);
      }
      /* skip the text */
      do
	 fgets(buf, 81, fl);
      while (*buf != '#');
      if (*(buf + 1) == '~')
	 break;
   }
   /* we might as well sort the stuff */
   do {
      issorted = 1;
      for (i = 0; i < nr; i++)
	 if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0) {
	    mem = list[i];
	    list[i] = list[i + 1];
	    list[i + 1] = mem;
	    issorted = 0;
	 }
   } while (!issorted);

   *num = nr;
   return(list);
}



void	page_string(struct descriptor_data *d, char *str, int keep_internal)
{
   if (!d)
      return;

   if (keep_internal) {
      CREATE(d->showstr_head, char, strlen(str) + 1);
      strcpy(d->showstr_head, str);
      d->showstr_point = d->showstr_head;
   } else
      d->showstr_point = str;

   show_string(d, "");
}



void	show_string(struct descriptor_data *d, char *input)
{
   char	buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
   register char	*scan, *chk;
   int	lines = 0, toggle = 1;

   one_argument(input, buf);

   if (*buf) {
      if (d->showstr_head) {
	 free(d->showstr_head);
	 d->showstr_head = 0;
      }
      d->showstr_point = 0;
      return;
   }

   /* show a chunk */
   for (scan = buffer; ; scan++, d->showstr_point++)
      if ((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) && 
          ((toggle = -toggle) < 0))
	 lines++;
      else if (!*scan || (lines >= 21)) {
	 *scan = '\0';
	 SEND_TO_Q(buffer, d);

	 /* see if this is the end (or near the end) of the string */
	 for (chk = d->showstr_point; isspace(*chk); chk++)
	    ;
	 if (!*chk) {
	    if (d->showstr_head) {
	       free(d->showstr_head);
	       d->showstr_head = 0;
	    }
	    d->showstr_point = 0;
	 }
	 SEND_TO_Q("\r\n",d); /* add an extra crlf for giggles */
	 return;
      }
}



void	check_reboot(void)
{
   long	tc;
   struct tm *t_info;
   char	dummy;
   FILE * boot;

   extern int	circle_shutdown, circle_reboot;

   tc = time(0);
   t_info = localtime(&tc);

   if ((t_info->tm_hour + 1) == REBOOT_AT && t_info->tm_min > 30)
      if ((boot = fopen("./reboot", "r"))) {
	 if (t_info->tm_min > 50) {
	    logg("Reboot exists.");
	    fread(&dummy, sizeof(dummy), 1, boot);
	    if (!feof(boot))   /* the file is nonepty */ {
	       logg("Reboot is nonempty.");
	       if (system("./reboot")) {
		  logg("Reboot script terminated abnormally");
		  send_to_all("The reboot was cancelled.\r\n");
		  system("mv ./reboot reboot.FAILED");
		  fclose(boot);
		  return;
	       } else
		  system("mv ./reboot reboot.SUCCEEDED");
	    }

	    send_to_all("Automatic reboot.  Come back in a little while.\r\n");
	    circle_shutdown = circle_reboot = 1;
	 } else if (t_info->tm_min > 40)
	    send_to_all("ATTENTION: CircleMUD will reboot in 10 minutes.\r\n");
	 else if (t_info->tm_min > 30)
	    send_to_all(
	        "Warning: The game will close and reboot in 20 minutes.\r\n");

	 fclose(boot);
      }
}

