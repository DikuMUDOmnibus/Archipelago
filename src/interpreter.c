/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
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
#include <unistd.h>

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "limits.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"

#define COMMANDO(number,min_pos,pointer,min_level,subcommand) {      \
	cmd_info[(number)].command_pointer = (pointer);   \
	cmd_info[(number)].minimum_position = (min_pos);  \
	cmd_info[(number)].minimum_level = (min_level); \
	cmd_info[(number)].subcmd = (subcommand); }


extern char	*motd;
extern char	*imotd;
extern char	*background;
extern char	*MENU;
extern char	*ANSI_MENU;
extern char	*WELC_MESSG;
extern char	*START_MESSG;
extern char *GREETINGS;
extern char *GREET_ANSI;
extern struct ban_list_element *ban_list;
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int	top_of_p_table;
extern int	restrict;
extern int	MAX_PLAYERS;
extern time_t imotd_time;
extern time_t motd_time;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
struct command_info cmd_info[MAX_CMD_LIST];
extern struct zone_data *zone_table;
extern struct pc_stat_parameters pc_str[];
extern struct pc_stat_parameters pc_int[];
extern struct pc_stat_parameters pc_wis[];
extern struct pc_stat_parameters pc_per[];
extern struct pc_stat_parameters pc_gui[];
extern struct pc_stat_parameters pc_foc[];
extern struct pc_stat_parameters pc_con[];
extern struct pc_stat_parameters pc_dex[];
extern struct pc_stat_parameters pc_chr[];
/* external functions */

void	echo_on(int sock);
void	echo_off(int sock);
void	set_title(struct char_data *ch);
void	do_start(struct char_data *ch);
void	init_char(struct char_data *ch);
int	create_entry(char *name);
int	special(struct char_data *ch, int cmd, char *arg);
int	isbanned(char *hostname);
void    write_ban_list(void);
long	is_selfbanned(struct descriptor_data *d);
int	Valid_Name(char *newname);
void    load_aliases(struct char_data *ch);
void    set_race_characteristics(struct char_data *ch);
/* prototypes for all do_x functions. */
ACMD(do_move);
ACMD(do_look);
ACMD(do_read);
ACMD(do_say);
ACMD(do_exit);
ACMD(do_snoop);
ACMD(do_insult);
ACMD(do_quit);
ACMD(do_help);
ACMD(do_who);
ACMD(do_alias);
ACMD(do_unalias);
ACMD(do_emote);
ACMD(do_echo);
ACMD(do_trans);
ACMD(do_kill);
ACMD(do_stand);
ACMD(do_sit);
ACMD(do_rest);
ACMD(do_sleep);
ACMD(do_wake);
ACMD(do_force);
ACMD(do_get);
ACMD(do_drop);
ACMD(do_score);
ACMD(do_inventory);
ACMD(do_equipment);
ACMD(do_not_here);
ACMD(do_tell);
ACMD(do_respond);
ACMD(do_wear);
ACMD(do_wield);
ACMD(do_grab);
ACMD(do_remove);
ACMD(do_put);
ACMD(do_shutdown);
ACMD(do_save);
ACMD(do_hit);
ACMD(do_string);
ACMD(do_give);
ACMD(do_stat);
ACMD(do_set);
ACMD(do_time);
ACMD(do_weather);
ACMD(do_load);
ACMD(do_vstat);
ACMD(do_purge);
ACMD(do_whisper);
ACMD(do_at);
ACMD(do_goto);
ACMD(do_ask);
ACMD(do_drink);
ACMD(do_eat);
ACMD(do_pour);
ACMD(do_order);
ACMD(do_follow);
ACMD(do_rent);
ACMD(do_offer);
ACMD(do_advance);
ACMD(do_close);
ACMD(do_open);
ACMD(do_lock);
ACMD(do_unlock);
ACMD(do_exits);
ACMD(do_enter);
ACMD(do_leave);
ACMD(do_write);
ACMD(do_flee);
ACMD(do_abort);
ACMD(do_sneak);
ACMD(do_hide);
ACMD(do_backstab);
ACMD(do_pick);
ACMD(do_steal);
ACMD(do_bash);
ACMD(do_rescue);
ACMD(do_kick);
ACMD(do_examine);
ACMD(do_info);
ACMD(do_users);
ACMD(do_where);
ACMD(do_levels);
ACMD(do_consider);
ACMD(do_group);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_switch);
ACMD(do_quaff);
ACMD(do_recite);
ACMD(do_use);
ACMD(do_credits);
ACMD(do_display);
ACMD(do_prompt);
ACMD(do_poofset);
ACMD(do_teleport);
ACMD(do_gecho);
ACMD(do_wiznet);
ACMD(do_invis);
ACMD(do_wimpy);
ACMD(do_wizlock);
ACMD(do_dc);
ACMD(do_gsay);
ACMD(do_title);
ACMD(do_visible);
ACMD(do_assist);
ACMD(do_split);
ACMD(do_toggle);
ACMD(do_send);
ACMD(do_vnum);
ACMD(do_action);
ACMD(do_learn);
ACMD(do_study);
ACMD(do_train);
ACMD(do_uptime);
ACMD(do_commands);
ACMD(do_ban);
ACMD(do_unban);
ACMD(do_date);
ACMD(do_zreset);
ACMD(do_gen_write);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_com);
ACMD(do_wizutil);
ACMD(do_color);
ACMD(do_syslog);
ACMD(do_show);
ACMD(do_ungroup);
ACMD(do_report);
ACMD(do_page);
ACMD(do_diagnose);
ACMD(do_qsay);
ACMD(do_reboot);
ACMD(do_last);
ACMD(do_track);
ACMD(do_scan);
ACMD(do_attributes);
ACMD(do_link_load);
ACMD(do_rentchar);
ACMD(do_savemobs);
ACMD(do_medit);
ACMD(do_mcreate);
ACMD(do_mclone);
ACMD(do_savezone);
ACMD(do_zsave);
ACMD(do_zpurge);
ACMD(do_oedit);
ACMD(do_saveobjs);
ACMD(do_ocreate);
ACMD(do_oclone);
ACMD(do_rcreate);
ACMD(do_screate);
ACMD(do_saverooms);
ACMD(do_redit);
ACMD(do_zset);
ACMD(do_rclone);
ACMD(do_motd);
ACMD(do_whohas);
ACMD(do_hitch);
ACMD(do_unhitch);
ACMD(do_switchzones);
ACMD(do_invoke);
ACMD(do_sedit);
ACMD(do_sheath);
ACMD(do_draw);
ACMD(do_mount);
ACMD(do_dismount);
ACMD(do_tame);
ACMD(do_tether);
ACMD(do_untether);
ACMD(do_user_check);
ACMD(do_bless);
ACMD(do_turn);
ACMD(do_zcreate);
ACMD(do_zedit);
char	*command[] = 
{
   "north", 		/* 1 */
   "east",
   "south",
   "west",
   "up",
   "down",
   "enter",
   "exits",
   "kiss",
   "get",
   "drink", 		/* 11 */
   "eat",
   "wear",
   "wield",
   "look",
   "score",
   "say",
   "shout",
   "tell",
   "inventory",
   "qui", 		/* 21 */
   "bounce",
   "smile",
   "dance",
   "kill",
   "cackle",
   "laugh",
   "giggle",
   "shake",
   "puke",
   "growl", 		/* 31 */
   "scream",
   "insult",
   "comfort",
   "nod",
   "sigh",
   "sulk",
   "help",
   "who",
   "emote",
   "echo", 		/* 41 */
   "stand",
   "sit",
   "rest",
   "sleep",
   "wake",
   "force",
   "transfer",
   "hug",
   "snuggle",
   "cuddle", 		/* 51 */
   "nuzzle",
   "cry",
   "news",
   "equipment",
   "buy",
   "sell",
   "value",
   "list",
   "drop",
   "goto", 		/* 61 */
   "weather",
   "read",
   "pour",
   "grab",
   "remove",
   "put",
   "shutdow",
   "save",
   "hit",
   "string", 		/* 71 */
   "give",
   "quit",
   "stat",
   "skillset",
   "time",
   "load",
   "purge",
   "shutdown",
   "idea",
   "typo", 		/* 81 */
   "bug",
   "whisper",
   "cast",
   "at",
   "ask",
   "order",
   "sip",
   "taste",
   "snoop",
   "follow", 		/* 91 */
   "rent",
   "offer",
   "poke",
   "advance",
   "accuse",
   "grin",
   "bow",
   "open",
   "close",
   "lock", 		/* 101 */
   "unlock",
   "leave",
   "applaud",
   "blush",
   "burp",
   "chuckle",
   "clap",
   "cough",
   "curtsey",
   "fart", 		/* 111 */
   "flip",
   "fondle",
   "frown",
   "gasp",
   "glare",
   "groan",
   "grope",
   "hiccup",
   "lick",
   "love", 		/* 121 */
   "moan",
   "nibble",
   "pout",
   "purr",
   "ruffle",
   "shiver",
   "shrug",
   "sing",
   "slap",
   "smirk", 		/* 131 */
   "snap",
   "sneeze",
   "snicker",
   "sniff",
   "snore",
   "spit",
   "squeeze",
   "stare",
   "strut",
   "thank", 		/* 141 */
   "twiddle",
   "wave",
   "whistle",
   "wiggle",
   "wink",
   "yawn",
   "snowball",
   "write",
   "hold",
   "flee", 		/* 151 */
   "sneak",
   "hide",
   "backstab",
   "pick",
   "steal",
   "bash",
   "rescue",
   "kick",
   "french",
   "comb", 		/* 161 */
   "massage",
   "tickle",
   "study",
   "pat",
   "examine",
   "take",
   "info",
   "'",
   "learn",
   "curse", 		/* 171 */
   "use",
   "where",
   "levels",
   "reroll",
   "pray",
   ":",
   "beg",
   "bleed",
   "cringe",
   "daydream", 		/* 181 */
   "fume",
   "grovel",
   "hop",
   "nudge",
   "peer",
   "point",
   "ponder",
   "punch",
   "snarl",
   "spank", 		/* 191 */
   "steam",
   "tackle",
   "taunt",
   "think",
   "whine",
   "worship",
   "yodel",
   "brief",
   "wizlist",
   "consider", 		/* 201 */
   "group",
   "restore",
   "return",
   "switch",
   "quaff",
   "recite",
   "users",
   "immlist",
   "noshout",
   "wizhelp", 		/* 211 */
   "credits",
   "compact",
   "display",
   "poofin",    
   "poofout",   
   "teleport",
   "gecho",
   "wiznet",
   "holylight",
   "invis", 		/* 221 */
   "wimpy",
   "set",
   "ungroup",
   "notell",
   "wizlock",
   "junk",
   "gsay",
   "pardon",
   "murder",
   "title", 		/* 231 */
   ";", /* WIZNET */
   "balance",
   "deposit",
   "withdraw",
   "visible",
   "question",
   "freeze",
   "dc",
   "nosummon",
   "assist", 		/* 241 */
   "split",
   "gtell",
   "brb",
   "norepeat",
   "toggle",
   "mail",
   "check",
   "receive",
   "send", /* not mail related :-) */
   "yell", 		/* 251 */
   "vnum",
   "fill",
   "uptime",
   "commands",
   "socials",
   "ban",
   "unban",
   "roll",
   "flirt",
   "tango", 		/* 261 */
   "embrace",
   "stroke",
   "greet",
   "drool", 
   "date",
   "zreset",
   "colour",
   "show",
   "prompt",
   "handbook", 		/* 271 */
   "policy",
   "nohassle",
   "nogossip",
   "noauction",
   "nograts",
   "roomflags",
   "mute",
   "nowiz",
   "notitle",
   "thaw", 		/* 281 */
   "unaffect",
   "cls",
   "clear",
   "version",
   "gossip",
   "auction",
   "grats",
   "donate",
   "page",
   "report", 		/* 291 */
   "diagnose",
   "qsay",
   "reboot",
   "syslog",
   "last",
   "slowns",
   "track",
   "whoami",
   "vstat",              /* 301 */
   "scan",
   "gain",
   "brag",
   "nobrag",
   "attributes",
   "chant",
   "link_load",
   "rentachar",
   "train",
   "savemobs",
   "medit",            /* 311 */
   "mcreate",
   "mclone",
   "savezone",
   "zsave",
   "zpurge",
   "oedit",
   "saveobjs",
   "ocreate",
   "oclone",
   "rcreate",              /* 321 */ 
   "saverooms",   
   "redit",
   "zset",
   "rclone",
   "press",
   "scratch",
   "arch",
   "tap",
   "agree",
   "tease",                 /* 331 */
   "puzzle",
   "blink",
   "hmmm",
   "shudder",
   "motd",
   "whohas",
   "hitch",
   "unhitch",
   "noconditions",
   "switchzones",    /* 341 */
   "jeer",
   "cheer",
   "invoke",
   "building",
   "screate",
   "sedit",
   "adieu",
   "afk",
   "apologize",
   "babble", /* 351 */
   "bark",
   "beam",
   "bearhug",
   "beckon",
   "belch",
   "blame",
   "boast",
   "boggle",
   "bonk",
   "booger",  /* 361 */
   "buzz",
   "cake",
   "caress",
   "cartwheel",
   "coy",
   "challenge",
   "charm",
   "charge",
   "chitter",
   "collapse", /* 371 */
   "cower",
   "creep",
   "critisize",
   "croon",
   "differ",
   "discombobulate",
   "doh",
   "duck",
   "evileye",
   "excuse", /* 381 */
   "eyebrow",
   "faint",
   "flare",
   "flash",
   "phlegm",
   "flex",
   "flinch",
   "flutter",
   "foam",
   "froth",   /* 391 */
   "furrow",
   "fuss",
   "goose",
   "grimace",
   "grumble",
   "highfive",
   "hotfoot",
   "howl",
   "hum",
   "hush", /* 401 */
   "hustle",
   "huzzah",
   "innocent",
   "insane",
   "offend",
   "kneel",
   "knight",
   "lag",
   "lust",
   "meow",  /* 411 */
   "moo",
   "mooch",
   "moon",
   "muahaha",
   "mutter",
   "noogie",
   "nyuk",
   "ogle",
   "oh",  
   "sheathe", /* 421 */
   "draw",
   "mount",
   "dismount",
   "tame",
   "tether",
   "untether",
   "checkuser",
   "abort",
   "alias", 
   "unalias", /* 431 */
   "respond",
   "bless",
   "turn",
   "zcreate",
   "zedit",
   "\n"
};


/* CEND: search for me when you're looking for the end of the cmd list! :) */

char	*fill[] = 
{
   "in",
   "from",
   "with",
   "the",
   "on",
   "at",
   "to",
   "\n"
};

int	search_block(char *arg, char **list, bool exact)
{
   register int	i, l;

   /* Make into lower case, and get length of string */
   for (l = 0; *(arg + l); l++)
      *(arg + l) = LOWER(*(arg + l));

   if (exact) {
      for (i = 0; **(list + i) != '\n'; i++)
	 if (!strcmp(arg, *(list + i)))
	    return(i);
   } else {
      if (!l)
	 l = 1; /* Avoid "" to match the first available string */
      for (i = 0; **(list + i) != '\n'; i++)
	 if (!strncmp(arg, *(list + i), l))
	    return(i);
   }

   return(-1);
}
int     search_list(char *arg, struct list_index_type *list, bool exact)
{
    register int i, l, ll;
    *buf1 = '\0';
    
    for (l = 0; *(arg + l); l++)
      *(arg + l) = LOWER(*(arg + l));

    if (exact) {
	for (i=0; *(list[i].entry) != '\n'; i++){
	    strcpy(buf1, list[i].entry);
	    for (ll = 0; *(buf1 + ll); ll++)
		*(buf1 + ll) = LOWER(*(buf1 + ll));
	    if (!strcmp(arg, buf1))
		return(list[i].index);
	}
   } else {
      if (!l)
	 l = 1; /* Avoid "" to match the first available string */
      for (i = 0; *(list[i].entry) != '\n'; i++){
	  strcpy(buf1, list[i].entry);
	  for (ll = 0; *(buf1 + ll); ll++)
	      *(buf1 + ll) = LOWER(*(buf1 + ll));
	  if (!strncmp(arg, buf1, l))
	      return(list[i].index);
      }
   }

   return(-1);
}
char    *rev_search_list(int num, struct list_index_type *list)
{
    register int i;

    for (i=0; list[i].index > 0; i++)
	if (num == list[i].index)
		return(list[i].entry);
   return(0);
}

int	old_search_block(char *argument, int begin, int length, char **list, int mode)
{
   int	guess, found, search;

   /* If the word contain 0 letters, then a match is already found */
   found = (length < 1);

   guess = 0;

   /* Search for a match */

   if (mode)
      while ( NOT found AND * (list[guess]) != '\n' ) {
	 found = (length == strlen(list[guess]));
	 for (search = 0; (search < length AND found); search++)
	    found = (*(argument + begin + search) == *(list[guess] + search));
	 guess++;
      }
   else {
      while ( NOT found AND * (list[guess]) != '\n' ) {
	 found = 1;
	 for (search = 0; (search < length AND found); search++)
	    found = (*(argument + begin + search) == *(list[guess] + search));
	 guess++;
      }
   }

   return ( found ? guess : -1 );
}


void	command_interpreter(struct char_data *ch, char *argument)
{
   int	look_at, cmd, begin, i, j=0, found, length;
   extern int	no_specials;
   char *temp;

   if (affected_by_spell(ch, SPELL_INVIS1))
     affect_from_char(ch, SPELL_INVIS1);
   
   REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
   REMOVE_BIT(PLR_FLAGS(ch), PLR_NEEDCRLF);
   /* Find first non blank */
   for (begin = 0 ; (*(argument + begin ) == ' ' ) ; begin++)  
     ;
   
   /* Find length of first word */
   for(look_at = 0; isalpha(*(argument + begin + look_at )) ; look_at++){
     /* Make all letters lower case AND find length */
     *(argument + begin + look_at) = LOWER(*(argument + begin + look_at));
   }
   if(*(argument + begin + look_at) && !look_at){
     look_at++;
   }
   
   cmd = old_search_block(argument, begin, look_at, command, 0);

   if (cmd < 0){
     /* didn't find the command so check aliases */
     for (i=0;i< MAX_ALIASES;i++){
       found = FALSE;
       if (ch->specials.aliases[i].alias){
	 length = strlen(ch->specials.aliases[i].alias);
	 length = MIN(look_at, length);
	 if (!strncmp(argument+begin, ch->specials.aliases[i].alias, length))
	   found = TRUE;
	 if (found && ch->specials.aliases[i].text){
	   sprintf(buf,"%s %s", ch->specials.aliases[i].text,
		   (argument + begin + look_at));
	   
	   command_interpreter(ch, buf);
	   return;
	 }
       }
     }
     send_to_char("Huh?!?\r\n",ch);
     return;
   }
   if (!cmd)
     return;
   if (IS_AFFECTED(ch, AFF_PARALYSIS) && (GET_LEVEL(ch) < LEVEL_BUILDER)){
     if (!((ch->desc &&  ch->desc->original) && (cmd == 204))){
       send_to_char("You try, but you are unable to do anything!\r\n", ch);
       return;
     }

   }
   if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LEVEL_IMPL) {
      send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
      return;
   }

   if (cmd > 0 && GET_LEVEL(ch) < cmd_info[cmd].minimum_level) {
      send_to_char("Huh?!?\r\n", ch);
      return;
   }

   if (cmd > 0 && (cmd_info[cmd].command_pointer != 0)) {
      if (cmd_info[cmd].minimum_position == POSITION_STANDING && ch->specials.fighting)
	  send_to_char("No way!  You're fighting for your life!\r\n", ch);
      else if (GET_POS(ch) < cmd_info[cmd].minimum_position
	       && GET_POS(ch) < POSITION_STANDING)
	 switch (GET_POS(ch)) {
	 case POSITION_DEAD:
	    send_to_char("Lie still; you are DEAD!!! :-( \r\n", ch);
	    break;
	 case POSITION_INCAP:
	 case POSITION_MORTALLYW:
	    send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
	    break;
	 case POSITION_STUNNED:
	    send_to_char("All you can do right now is think about the stars!\r\n", ch);
	    break;
	 case POSITION_SLEEPING:
	    send_to_char("In your dreams, or what?\r\n", ch);
	    break;
	 case POSITION_RESTING:
	    send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
	    break;
	 case POSITION_SITTING:
	    send_to_char("Maybe you should get on your feet first?\r\n", ch);
	    break;
	 }
      else {
	 if (!no_specials && special(ch, cmd, argument + begin + look_at))
	    return;

	 ((*cmd_info[cmd].command_pointer)
	     (ch, argument + begin + look_at, cmd, cmd_info[cmd].subcmd));
      }

      return;
   }

   if (cmd > 0 && (cmd_info[cmd].command_pointer == 0))
      send_to_char("Sorry, but that command has yet to be implemented...\r\n", ch);
   else if (*(argument + begin) == '\'') 
     {
       temp = argument+begin+1;
       do_say(ch,temp,0,0);}
   else 
     send_to_char("Huh?!?\r\n", ch);
}


void	argument_interpreter(char *argument, char *first_arg, char *second_arg )
{
   int	look_at, found, begin;

   found = begin = 0;

   do {
      /* Find first non blank */
      for ( ; *(argument + begin ) == ' ' ; begin++) 
	 ;

      /* Find length of first word */
      for ( look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)

	 /* Make all letters lower case, AND copy them to first_arg */
	 *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

      *(first_arg + look_at) = '\0';
      begin += look_at;
   } while (fill_word(first_arg));

   do {
      /* Find first non blank */
      for ( ; *(argument + begin ) == ' ' ; begin++)
	 ;

      /* Find length of first word */
      for ( look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)

	 /* Make all letters lower case, AND copy them to second_arg */
	 *(second_arg + look_at) = LOWER(*(argument + begin + look_at));

      *(second_arg + look_at) = '\0';
      begin += look_at;
   } while (fill_word(second_arg));
}


int	is_number(char *str)
{
   int	look_at;

   if (*str == '\0')
      return(0);
   for (look_at = 0; *(str + look_at) != '\0'; look_at++)
      if ((*(str + look_at) < '0') || (*(str + look_at) > '9'))
	 return(0);
   return(1);
}

/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg */
char	*one_argument(char *argument, char *first_arg)
{
   int	found, begin, look_at;

   found = begin = 0;

   do {
      /* Find first non blank */
      for ( ; isspace(*(argument + begin)); begin++)
	 ;

      /* Find length of first word */
      for (look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)

	 /* Make all letters lower case, AND copy them to first_arg */
	 *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

      *(first_arg + look_at) = '\0';
      begin += look_at;
   } while (fill_word(first_arg));

   return(argument + begin);
}


int	fill_word(char *argument)
{
   return (search_block(argument, fill, TRUE) >= 0);
}


/* determine if a given string is an abbreviation of another */
int	is_abbrev(char *arg1, char *arg2)
{
   if (!*arg1)
      return(0);

   for (; *arg1; arg1++, arg2++)
      if (LOWER(*arg1) != LOWER(*arg2))
	 return(0);

   return(1);
}



/* return first 'word' plus trailing substring of input string */
void	half_chop(char *string, char *arg1, char *arg2)
{
   for (; isspace(*string); string++)
      ;

   for (; !isspace(*arg1 = *string) && *string; string++, arg1++)
      ;

   *arg1 = '\0';

   for (; isspace(*string); string++)
      ;

   for (; (*arg2 = *string); string++, arg2++)
      ;
}



int	special(struct char_data *ch, int cmd, char *arg)
{
   register struct obj_data *i;
   register struct char_data *k;
   int	j;

   /* special in room? */
   if (world[ch->in_room].funct)
      if ((*world[ch->in_room].funct)(ch, &world[ch->in_room], cmd, arg))
	 return(1);

   /* special in equipment list? */
   for (j = 0; j <= (MAX_WEAR - 1); j++)
      if (ch->equipment[j] && ch->equipment[j]->item_number >= 0)
	 if (obj_index[ch->equipment[j]->item_number].func)
	    if ((*obj_index[ch->equipment[j]->item_number].func)
	        (ch,ch->equipment[j],cmd, arg))
	       return(1);

   /* special in inventory? */
   for (i = ch->inventory; i; i = i->next_content)
      if (i->item_number >= 0)
	 if (obj_index[i->item_number].func)
	    if ((*obj_index[i->item_number].func)(ch, i,cmd, arg))
	       return(1);

   /* special in mobile present? */
   for (k = world[ch->in_room].people; k; k = k->next_in_room)
      if (IS_MOB(k))
	 if (mob_index[k->nr].func)
	    if ((*mob_index[k->nr].func)(ch, k, cmd, arg))
	       return(1);

   /* special in object present? */
   for (i = world[ch->in_room].contents; i; i = i->next_content)
      if (i->item_number >= 0)
	 if (obj_index[i->item_number].func)
	    if ((*obj_index[i->item_number].func)(ch, i, cmd, arg))
	       return(1);

   return(0);
}


void	assign_command_pointers (void)
{
   int	position;

   for (position = 0 ; position < MAX_CMD_LIST; position++)
      cmd_info[position].command_pointer = 0;

   COMMANDO(1  , POSITION_STANDING, do_move     , 0, 0)
   COMMANDO(2  , POSITION_STANDING, do_move     , 0, 0)
   COMMANDO(3  , POSITION_STANDING, do_move     , 0, 0)
   COMMANDO(4  , POSITION_STANDING, do_move     , 0, 0)
   COMMANDO(5  , POSITION_STANDING, do_move     , 0, 0)
   COMMANDO(6  , POSITION_STANDING, do_move     , 0, 0)
   COMMANDO(7  , POSITION_STANDING, do_enter    , 0, 0)
   COMMANDO(8  , POSITION_RESTING , do_exits    , 0, 0)
   COMMANDO(9  , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(10 , POSITION_RESTING , do_get      , 0, 0)
   COMMANDO(11 , POSITION_RESTING , do_drink    , 0, SCMD_DRINK)
   COMMANDO(12 , POSITION_RESTING , do_eat      , 0, SCMD_EAT)
   COMMANDO(13 , POSITION_RESTING , do_wear     , 0, 0)
   COMMANDO(14 , POSITION_RESTING , do_wield    , 0, 0)
   COMMANDO(15 , POSITION_RESTING , do_look     , 0, 0)
   COMMANDO(16 , POSITION_DEAD    , do_score    , 0, 0)
   COMMANDO(17 , POSITION_RESTING , do_say      , 0, 0)
   COMMANDO(18 , POSITION_RESTING , do_gen_com  , 0, SCMD_SHOUT)
   COMMANDO(19 , POSITION_RESTING , do_tell     , 0, 0)
   COMMANDO(20 , POSITION_DEAD    , do_inventory, 0, 0)
   COMMANDO(21 , POSITION_DEAD    , do_quit     , 0, 0)
   COMMANDO(22 , POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(23 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(24 , POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(25 , POSITION_FIGHTING, do_kill     , 0, 0)
   COMMANDO(26 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(27 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(28 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(29 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(30 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(31 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(32 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(33 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(34 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(35 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(36 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(37 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(38 , POSITION_DEAD    , do_help     , 0, 0)
   COMMANDO(39 , POSITION_DEAD    , do_who      , 0, 0)
   COMMANDO(40 , POSITION_SLEEPING, do_emote    , 1, 0)
   COMMANDO(41 , POSITION_SLEEPING, do_echo     , LEVEL_BUILDER, 0)
   COMMANDO(42 , POSITION_RESTING , do_stand    , 0, 0)
   COMMANDO(43 , POSITION_RESTING , do_sit      , 0, 0)
   COMMANDO(44 , POSITION_RESTING , do_rest     , 0, 0)
   COMMANDO(45 , POSITION_SLEEPING, do_sleep    , 0, 0)
   COMMANDO(46 , POSITION_SLEEPING, do_wake     , 0, 0)
   COMMANDO(47 , POSITION_SLEEPING, do_force    , LEVEL_BUILDER, 0)
   COMMANDO(48 , POSITION_SLEEPING, do_trans    , LEVEL_BUILDER, 0)
   COMMANDO(49 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(50 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(51 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(52 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(53 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(54 , POSITION_SLEEPING, do_gen_ps   , 0, SCMD_NEWS)
   COMMANDO(55 , POSITION_SLEEPING, do_equipment, 0, 0)
   COMMANDO(56 , POSITION_STANDING, do_not_here , 0, 0)
   COMMANDO(57 , POSITION_STANDING, do_not_here , 0, 0)
   COMMANDO(58 , POSITION_STANDING, do_not_here , 0, 0)
   COMMANDO(59 , POSITION_STANDING, do_not_here , 0, 0)
   COMMANDO(60 , POSITION_RESTING , do_drop     , 0, SCMD_DROP)
   COMMANDO(61 , POSITION_SLEEPING, do_goto     , 200, 0)
   COMMANDO(62 , POSITION_RESTING , do_weather  , 0, 0)
   COMMANDO(63 , POSITION_RESTING , do_read     , 0, 0)
   COMMANDO(64 , POSITION_STANDING, do_pour     , 0, SCMD_POUR)
   COMMANDO(65 , POSITION_RESTING , do_grab     , 0, 0)
   COMMANDO(66 , POSITION_RESTING , do_remove   , 0, 0)
   COMMANDO(67 , POSITION_RESTING , do_put      , 0, 0)
   COMMANDO(68 , POSITION_DEAD    , do_shutdown , LEVEL_IMPL, 0)
   COMMANDO(69 , POSITION_SLEEPING, do_save     , 0, 0)
   COMMANDO(70 , POSITION_FIGHTING, do_hit      , 0, SCMD_HIT)
   COMMANDO(71 , POSITION_SLEEPING, do_string   , LEVEL_IMPL, 0)
   COMMANDO(72 , POSITION_RESTING , do_give     , 0, 0)
   COMMANDO(73 , POSITION_DEAD    , do_quit     , 0, SCMD_QUIT)
   COMMANDO(74 , POSITION_DEAD    , do_stat     , LEVEL_BUILDER, 0)
   COMMANDO(75 , POSITION_SLEEPING, do_not_here , LEVEL_GRGOD, 0)
   COMMANDO(76 , POSITION_DEAD    , do_time     , 0, 0)
   COMMANDO(77 , POSITION_DEAD    , do_load     , LEVEL_BUILDER, 0)
   COMMANDO(78 , POSITION_DEAD    , do_purge    , LEVEL_BUILDER, 0)
   COMMANDO(79 , POSITION_DEAD    , do_shutdown , LEVEL_IMPL, SCMD_SHUTDOWN)
   COMMANDO(80 , POSITION_DEAD    , do_gen_write, 0, SCMD_IDEA)
   COMMANDO(81 , POSITION_DEAD    , do_gen_write, 0, SCMD_TYPO)
   COMMANDO(82 , POSITION_DEAD    , do_gen_write, 0, SCMD_BUG)
   COMMANDO(83 , POSITION_RESTING , do_whisper  , 0, 0)
   COMMANDO(84 , POSITION_SITTING , do_action   , 0, 0)
   COMMANDO(85 , POSITION_DEAD    , do_at       , LEVEL_BUILDER, 0)
   COMMANDO(86 , POSITION_RESTING , do_ask      , 0, 0)
   COMMANDO(87 , POSITION_RESTING , do_order    , 1, 0)
   COMMANDO(88 , POSITION_RESTING , do_drink    , 0, SCMD_SIP)
   COMMANDO(89 , POSITION_RESTING , do_eat      , 0, SCMD_TASTE)
   COMMANDO(90 , POSITION_DEAD    , do_snoop    , LEVEL_GRGOD, 0)
   COMMANDO(91 , POSITION_RESTING , do_follow   , 0, 0)
   COMMANDO(92 , POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(93 , POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(94 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(95 , POSITION_DEAD    , do_advance  , LEVEL_IMPL, 0)
   COMMANDO(96 , POSITION_SITTING , do_action   , 0, 0)
   COMMANDO(97 , POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(98 , POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(99 , POSITION_RESTING , do_open     , 0, 0)
   COMMANDO(100, POSITION_SITTING , do_close    , 0, 0)
   COMMANDO(101, POSITION_SITTING , do_lock     , 0, 0)
   COMMANDO(102, POSITION_SITTING , do_unlock   , 0, 0)
   COMMANDO(103, POSITION_STANDING, do_leave    , 0, 0)
   COMMANDO(104, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(105, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(106, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(107, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(108, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(109, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(110, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(111, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(112, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(113, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(114, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(115, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(116, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(117, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(118, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(119, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(120, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(121, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(122, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(123, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(124, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(125, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(126, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(127, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(128, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(129, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(130, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(131, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(132, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(133, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(134, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(135, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(136, POSITION_SLEEPING, do_action   , 0, 0)
   COMMANDO(137, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(138, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(139, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(140, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(141, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(142, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(143, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(144, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(145, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(146, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(147, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(148, POSITION_STANDING, do_action   , 200, 0)
   COMMANDO(149, POSITION_RESTING,  do_write    , 1, 0)
   COMMANDO(150, POSITION_RESTING , do_grab     , 1, 0)
   COMMANDO(151, POSITION_FIGHTING, do_flee     , 1, 0)
   COMMANDO(152, POSITION_STANDING, do_sneak    , 1, 0)
   COMMANDO(153, POSITION_RESTING , do_hide     , 1, 0)
   COMMANDO(154, POSITION_STANDING, do_backstab , 1, 0)
   COMMANDO(155, POSITION_STANDING, do_pick     , 1, 0)
   COMMANDO(156, POSITION_STANDING, do_steal    , 1, 0)
   COMMANDO(157, POSITION_FIGHTING, do_bash     , 1, 0)
   COMMANDO(158, POSITION_FIGHTING, do_rescue   , 1, 0)
   COMMANDO(159, POSITION_FIGHTING, do_kick     , 1, 0)
   COMMANDO(160, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(161, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(162, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(163, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(164, POSITION_SLEEPING , do_study    , 1, 0)
   COMMANDO(165, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(166, POSITION_SITTING , do_examine  , 0, 0)
   COMMANDO(167, POSITION_RESTING , do_get      , 0, 0)
   COMMANDO(168, POSITION_SLEEPING, do_gen_ps   , 0, SCMD_INFO)
   COMMANDO(169, POSITION_RESTING , do_say      , 0, 0)
   COMMANDO(170, POSITION_SLEEPING , do_learn    , 1, 0)
   COMMANDO(171, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(172, POSITION_SITTING , do_use      , 1, 0)
   COMMANDO(173, POSITION_DEAD    , do_where    , 1, 0)
   COMMANDO(174, POSITION_DEAD    , do_levels   , LEVEL_BUILDER, 0)
   COMMANDO(175, POSITION_DEAD    , do_wizutil  , LEVEL_GRGOD, SCMD_REROLL)
   COMMANDO(176, POSITION_SITTING , do_bless     , 0, SCMD_PRAY)
   COMMANDO(177, POSITION_SLEEPING, do_emote    , 1, 0)
   COMMANDO(178, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(179, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(180, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(181, POSITION_SLEEPING, do_action   , 0, 0)
   COMMANDO(182, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(183, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(184, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(185, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(186, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(187, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(188, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(189, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(190, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(191, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(192, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(193, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(194, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(195, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(196, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(197, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(198, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(199, POSITION_DEAD    , do_gen_tog  , 0, SCMD_BRIEF)
   COMMANDO(200, POSITION_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST)
   COMMANDO(201, POSITION_RESTING , do_consider , 0, 0)
   COMMANDO(202, POSITION_SLEEPING , do_group    , 1, 0)
   COMMANDO(203, POSITION_DEAD    , do_restore  , LEVEL_GOD, 0)
   COMMANDO(204, POSITION_DEAD    , do_return   , 0, 0)
   COMMANDO(205, POSITION_DEAD    , do_switch   , LEVEL_GRGOD, 0)
   COMMANDO(206, POSITION_RESTING , do_quaff    , 0, 0)
   COMMANDO(207, POSITION_RESTING , do_recite   , 0, 0)
   COMMANDO(208, POSITION_DEAD    , do_users    , LEVEL_BUILDER, 0)
   COMMANDO(209, POSITION_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST)
   COMMANDO(210, POSITION_SLEEPING, do_gen_tog  , 1, SCMD_DEAF)
   COMMANDO(211, POSITION_SLEEPING, do_commands , 200,SCMD_WIZHELP)
   COMMANDO(212, POSITION_DEAD    , do_gen_ps   , 0, SCMD_CREDITS)
   COMMANDO(213, POSITION_DEAD    , do_gen_tog  , 0, SCMD_COMPACT)
   COMMANDO(214, POSITION_DEAD    , do_display   , 0, 0)
   COMMANDO(215, POSITION_DEAD    , do_poofset  ,200, SCMD_POOFIN)
   COMMANDO(216, POSITION_DEAD    , do_poofset  ,200, SCMD_POOFOUT)
   COMMANDO(217, POSITION_DEAD    , do_teleport , LEVEL_GOD, 0)
   COMMANDO(218, POSITION_DEAD    , do_gecho    , LEVEL_GOD, 0)
   COMMANDO(219, POSITION_DEAD    , do_wiznet   , LEVEL_BUILDER, 0)
   COMMANDO(220, POSITION_DEAD    , do_gen_tog  , LEVEL_BUILDER, SCMD_HOLYLIGHT)
   COMMANDO(221, POSITION_DEAD    , do_invis    , LEVEL_BUILDER, 0)
   COMMANDO(222, POSITION_DEAD    , do_wimpy    , 0, 0)
   COMMANDO(223, POSITION_DEAD    , do_set      , LEVEL_GOD, 0)
   COMMANDO(224, POSITION_DEAD    , do_ungroup  , 0, 0)
   COMMANDO(225, POSITION_DEAD    , do_gen_tog  , 1, SCMD_NOTELL)
   COMMANDO(226, POSITION_DEAD    , do_wizlock  , LEVEL_IMPL, 0)
   COMMANDO(227, POSITION_RESTING , do_drop     , 0, SCMD_JUNK)
   COMMANDO(228, POSITION_SLEEPING , do_gsay     , 0, 0)
   COMMANDO(229, POSITION_DEAD    , do_wizutil  , LEVEL_GOD, SCMD_PARDON)
   COMMANDO(230, POSITION_FIGHTING, do_hit      , 0, SCMD_MURDER)
   COMMANDO(231, POSITION_DEAD    , do_title    , 0, 0)
   COMMANDO(232, POSITION_DEAD    , do_wiznet   , LEVEL_BUILDER, 0)
   COMMANDO(233, POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(234, POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(235, POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(236, POSITION_RESTING , do_visible  , 200, 0)
   COMMANDO(237, POSITION_DEAD    , do_not_here  , 0, 0)
   COMMANDO(238, POSITION_DEAD    , do_wizutil  , LEVEL_FREEZE, SCMD_FREEZE)
   COMMANDO(239, POSITION_DEAD    , do_dc       , LEVEL_MBUILDER, 0)
   COMMANDO(240, POSITION_DEAD    , do_gen_tog  , 1, SCMD_NOSUMMON)
   COMMANDO(241, POSITION_FIGHTING, do_assist   , 1, 0)
   COMMANDO(242, POSITION_SITTING , do_split    , 1, 0)
   COMMANDO(243, POSITION_SLEEPING, do_gsay     , 0, 0)
   COMMANDO(244, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(245, POSITION_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT)
   COMMANDO(246, POSITION_DEAD    , do_toggle   , 0, 0)
   COMMANDO(247, POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(248, POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(249, POSITION_STANDING, do_not_here , 1, 0)
   COMMANDO(250, POSITION_SLEEPING, do_send     , LEVEL_GOD, 0)
   COMMANDO(251, POSITION_RESTING , do_gen_com  , 1, SCMD_HOLLER)
   COMMANDO(252, POSITION_DEAD    , do_vnum     , LEVEL_BUILDER, 0)
   COMMANDO(253, POSITION_STANDING, do_pour     , 0, SCMD_FILL)
   COMMANDO(254, POSITION_DEAD    , do_uptime   , 200, 0)
   COMMANDO(255, POSITION_DEAD    , do_commands , 0, SCMD_COMMANDS)
   COMMANDO(256, POSITION_DEAD    , do_commands , 0, SCMD_SOCIALS)
   COMMANDO(257, POSITION_DEAD    , do_ban      , LEVEL_GRGOD, 0)
   COMMANDO(258, POSITION_DEAD    , do_unban    , LEVEL_GRGOD, 0)
   COMMANDO(259, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(260, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(261, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(262, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(263, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(264, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(265, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(266, POSITION_DEAD    , do_date     , 200, 0)
   COMMANDO(267, POSITION_DEAD    , do_zreset   , LEVEL_GRGOD, 0)
   COMMANDO(268, POSITION_DEAD    , do_color    , 0, 0)
   COMMANDO(269, POSITION_DEAD    , do_show     , 0, 0)
   COMMANDO(270, POSITION_DEAD    , do_prompt   , 0, 0)
   COMMANDO(271, POSITION_DEAD    , do_gen_ps   , LEVEL_BUILDER, SCMD_HANDBOOK)
   COMMANDO(272, POSITION_DEAD    , do_gen_ps   , 0, SCMD_POLICIES)
   COMMANDO(273, POSITION_DEAD    , do_gen_tog  , LEVEL_BUILDER, SCMD_NOHASSLE)
   COMMANDO(274, POSITION_DEAD    , do_gen_tog  , 0, SCMD_NOGOSSIP)
   COMMANDO(275, POSITION_DEAD    , do_gen_tog  , 0, SCMD_NOAUCTION)
   COMMANDO(276, POSITION_DEAD    , do_gen_tog  , 0, SCMD_NOGRATZ)
   COMMANDO(277, POSITION_DEAD    , do_gen_tog  , LEVEL_BUILDER, SCMD_ROOMFLAGS)
   COMMANDO(278, POSITION_DEAD    , do_wizutil  , LEVEL_GOD, SCMD_SQUELCH)
   COMMANDO(279, POSITION_DEAD    , do_gen_tog  , LEVEL_BUILDER, SCMD_NOWIZ)
   COMMANDO(280, POSITION_DEAD    , do_wizutil  , LEVEL_GOD, SCMD_NOTITLE)
   COMMANDO(281, POSITION_DEAD    , do_wizutil  , LEVEL_FREEZE, SCMD_THAW)
   COMMANDO(282, POSITION_DEAD    , do_wizutil  , LEVEL_GOD, SCMD_UNAFFECT)
   COMMANDO(283, POSITION_DEAD    , do_gen_ps   , 0, SCMD_CLEAR)
   COMMANDO(284, POSITION_DEAD    , do_gen_ps   , 0, SCMD_CLEAR)
   COMMANDO(285, POSITION_DEAD    , do_gen_ps   , 0, SCMD_VERSION)
   COMMANDO(286, POSITION_SLEEPING, do_gen_com  , 0, SCMD_GOSSIP)
   COMMANDO(287, POSITION_RESTING, do_gen_com  , 0, SCMD_AUCTION)
   COMMANDO(288, POSITION_RESTING, do_gen_com  , 0, SCMD_GRATZ)
   COMMANDO(289, POSITION_RESTING , do_drop     , 0, SCMD_DONATE)
   COMMANDO(290, POSITION_DEAD    , do_page     , LEVEL_GOD, 0)
   COMMANDO(291, POSITION_RESTING , do_report   , 0, 0)
   COMMANDO(292, POSITION_RESTING , do_diagnose , 0, 0)
   COMMANDO(293, POSITION_RESTING , do_qsay     , 0, 0)
   COMMANDO(294, POSITION_DEAD    , do_reboot   , LEVEL_IMPL, 0)
   COMMANDO(295, POSITION_DEAD    , do_syslog   , LEVEL_BUILDER, 0)
   COMMANDO(296, POSITION_DEAD    , do_last     , LEVEL_GOD, 0)
   COMMANDO(297, POSITION_DEAD    , do_gen_tog  , LEVEL_IMPL, SCMD_SLOWNS)
   COMMANDO(298, POSITION_STANDING, do_track    , 0, 0)
   COMMANDO(299, POSITION_DEAD    , do_gen_ps   , 0, SCMD_WHOAMI)
   COMMANDO(300, POSITION_DEAD    , do_vstat    , LEVEL_BUILDER, 0)
   COMMANDO(301, POSITION_STANDING, do_scan     , 0, 0) 
   COMMANDO(302, POSITION_STANDING, do_not_here , 0, 0)
   COMMANDO(303, POSITION_STANDING, do_gen_com  , 0, SCMD_BRAG)
   COMMANDO(304, POSITION_STANDING, do_gen_tog  , 0, SCMD_NOBRAG)
   COMMANDO(305, POSITION_SLEEPING, do_attributes, 0, 0)
   COMMANDO(306, POSITION_FIGHTING, do_action     , 0, 0)
   COMMANDO(307, POSITION_DEAD    , do_link_load , LEVEL_ASS_IMPL, 0)
   COMMANDO(308, POSITION_DEAD    , do_rentchar , LEVEL_ASS_IMPL, 0)
   COMMANDO(309, POSITION_SLEEPING , do_train , 1, 0)
   COMMANDO(310, POSITION_DEAD    , do_savemobs , LEVEL_BUILDER, 0)       
   COMMANDO(311, POSITION_DEAD    , do_medit , LEVEL_BUILDER, 0)
   COMMANDO(312, POSITION_DEAD    , do_mcreate , LEVEL_BUILDER, 0)
   COMMANDO(313, POSITION_DEAD    , do_mclone , LEVEL_BUILDER, 0)
   COMMANDO(314, POSITION_DEAD    , do_savezone, LEVEL_BUILDER, 0)
   COMMANDO(315, POSITION_DEAD    , do_zsave, LEVEL_BUILDER, 0)
   COMMANDO(316, POSITION_DEAD    , do_zpurge, LEVEL_BUILDER, 0)
   COMMANDO(317, POSITION_DEAD    , do_oedit, LEVEL_BUILDER, 0)
   COMMANDO(318, POSITION_DEAD    , do_saveobjs, LEVEL_BUILDER, 0)
   COMMANDO(319, POSITION_DEAD    , do_ocreate , LEVEL_BUILDER, 0)       
   COMMANDO(320, POSITION_DEAD    , do_oclone , LEVEL_BUILDER, 0)
   COMMANDO(321, POSITION_DEAD    , do_rcreate , LEVEL_BUILDER, 0)
   COMMANDO(322, POSITION_DEAD    , do_saverooms , LEVEL_BUILDER, 0)
   COMMANDO(323, POSITION_DEAD    , do_redit , LEVEL_BUILDER, 0)
   COMMANDO(324, POSITION_DEAD    , do_zset , LEVEL_BUILDER, 0)
   COMMANDO(325, POSITION_DEAD    , do_rclone , LEVEL_BUILDER, 0)
   COMMANDO(326, POSITION_DEAD    , do_not_here , 1, 0)
   COMMANDO(327, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(328, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(329, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(330, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(331, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(332, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(333, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(334, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(335, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(336, POSITION_SLEEPING, do_motd     , 0, 0)
   COMMANDO(337, POSITION_DEAD    , do_whohas   , LEVEL_ASS_IMPL, 0)
   COMMANDO(338, POSITION_STANDING    , do_hitch    , 0, 0)
   COMMANDO(339, POSITION_STANDING    , do_unhitch    , 0, 0)
   COMMANDO(340, POSITION_STANDING, do_gen_tog  , 0, SCMD_NOCOND)
   COMMANDO(341, POSITION_DEAD    , do_switchzones, LEVEL_MBUILDER, 0)       
   COMMANDO(342, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(343, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(344, POSITION_RESTING , do_invoke   , 0, 0)
   COMMANDO(345, POSITION_RESTING , do_gen_ps   , LEVEL_BUILDER, SCMD_BUILDH)
   COMMANDO(346, POSITION_DEAD    , do_screate  , LEVEL_ASS_IMPL, 0)
   COMMANDO(347, POSITION_DEAD    , do_sedit    , LEVEL_ASS_IMPL, 0)
   COMMANDO(348, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(349, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(350, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(351, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(352, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(353, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(354, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(355, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(356, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(357, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(358, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(359, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(360, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(361, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(362, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(363, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(364, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(365, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(366, POSITION_RESTING , do_action   , 0, 84)
   COMMANDO(367, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(368, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(369, POSITION_STANDING , do_action   , 0, 0)
   COMMANDO(370, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(371, POSITION_STANDING , do_action   , 0, 0)
   COMMANDO(372, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(373, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(374, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(375, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(376, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(377, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(378, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(379, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(380, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(381, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(382, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(383, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(384, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(385, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(386, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(387, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(388, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(389, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(390, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(391, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(392, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(393, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(394, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(395, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(396, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(397, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(398, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(399, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(400, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(401, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(402, POSITION_STANDING, do_action   , 0, 0)
   COMMANDO(403, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(404, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(405, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(406, POSITION_RESTING , do_action   , 0, 33)
   COMMANDO(407, POSITION_STANDING , do_action   , 0, 0)
   COMMANDO(408, POSITION_STANDING , do_action   , LEVEL_GOD, 0)
   COMMANDO(409, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(410, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(411, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(412, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(413, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(414, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(415, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(416, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(417, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(418, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(419, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(420, POSITION_RESTING , do_action   , 0, 0)
   COMMANDO(421, POSITION_STANDING , do_sheath   , 0, 0)
   COMMANDO(422, POSITION_FIGHTING , do_draw     , 0, 0)
   COMMANDO(423, POSITION_STANDING , do_mount     , 0, 0)
   COMMANDO(424, POSITION_STANDING , do_dismount     , 0, 0)
   COMMANDO(425, POSITION_STANDING , do_tame     , 0, 0)
   COMMANDO(426, POSITION_STANDING , do_tether     , 0, 0)
   COMMANDO(427, POSITION_STANDING , do_untether     , 0, 0)
   COMMANDO(428, POSITION_STANDING , do_user_check   , LEVEL_IMPL, 0)
   COMMANDO(429, POSITION_DEAD ,     do_abort     , 0, 0)
   COMMANDO(430, POSITION_DEAD ,     do_alias     , 0, 0)
   COMMANDO(431, POSITION_DEAD ,     do_unalias   , 0, 0)
   COMMANDO(432, POSITION_RESTING ,     do_respond   , 0, 0)
   COMMANDO(433, POSITION_RESTING ,     do_bless   , 0, 0)
   COMMANDO(434, POSITION_RESTING ,     do_turn   , 0, 0)
   COMMANDO(435, POSITION_RESTING ,     do_zcreate   , LEVEL_IMPL, 0)
   COMMANDO(436, POSITION_RESTING ,     do_zedit   , LEVEL_GOD, 0)
     }
/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int	find_name(char *name)
{
   int	i;

   for (i = 0; i <= top_of_p_table; i++) {
      if (!str_cmp((player_table + i)->name, name))
	 return(i);
   }

   return(-1);
}


int	_parse_name(char *arg, char *name)
{
   int	i;

   /* skip whitespaces */
   for (; isspace(*arg); arg++)
      ;

   for (i = 0; (*name = *arg); arg++, i++, name++)
      if (!isalpha(*arg) || i > 15)
	 return(1);

   if (!i)
      return(1);

   return(0);
}

void display_stats(struct descriptor_data *d)
{
    sprintf(buf,"[S]tr: %-2i", GET_RAW_STR(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,", [I]nt: %-2i", GET_RAW_INT(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,", [W]is: %-2i", GET_RAW_WIS(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,", [D]ex: %-2i", GET_RAW_DEX(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,", [F]oc: %-2i\r\n", GET_RAW_FOC(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,"[C]on: %-2i", GET_RAW_CON(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,", C[h]r: %-2i", GET_RAW_CHR(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,", [P]er: %-2i", GET_RAW_PER(d->character));
    SEND_TO_Q(buf,d);
    sprintf(buf,", [G]ui: %-2i", GET_RAW_GUI(d->character));
    SEND_TO_Q(buf,d);
    SEND_TO_Q(", [Q]uit.\r\n",d);
    sprintf(buf,"You have %i points to add.\r\n",d->stat_points);
    SEND_TO_Q(buf,d);
    SEND_TO_Q("Stat to modify: ",d);
}

void    set_race_characteristics(struct char_data *ch)
{
  GET_BODY_AC(ch) = 100;
  GET_ARMS_AC(ch) = 100;
  GET_HEAD_AC(ch) = 100;
  GET_LEGS_AC(ch) = 100;	  
  GET_BODY_STOPPING(ch) = 0;
  GET_ARMS_STOPPING(ch) = 0;
  GET_HEAD_STOPPING(ch) = 0;
  GET_LEGS_STOPPING(ch) = 0;	  
  if (!IS_NPC(ch)){
    switch (GET_RACE(ch)) {
    case 1: 
    case 2: 
      break;
    case 3:
      GET_BODY_AC(ch) = 50;
      GET_ARMS_AC(ch) = 50;
      GET_HEAD_AC(ch) = 50;
      GET_LEGS_AC(ch) = 50;	  
      break;
    case 4:
      GET_BODY_STOPPING(ch) = 2;
      GET_ARMS_STOPPING(ch) = 2;
      GET_HEAD_STOPPING(ch) = 1;
      GET_LEGS_STOPPING(ch) = 2;	  
      break;
    case 5:
      GET_BODY_AC(ch) = 70;
      GET_ARMS_AC(ch) = 70;
      GET_HEAD_AC(ch) = 70;
      GET_LEGS_AC(ch) = 70;	  
      break;
    case 6:
      break;
    case 7: 
      GET_BODY_STOPPING(ch) = 1;
      GET_ARMS_STOPPING(ch) = 1;
      GET_LEGS_STOPPING(ch) = 1;	  
      break;
    case 8: 
      GET_BODY_AC(ch) = 60;
      GET_ARMS_AC(ch) = 60;
      GET_HEAD_AC(ch) = 60;
      GET_LEGS_AC(ch) = 60;	  	  
      GET_BODY_STOPPING(ch) = 1;
      GET_ARMS_STOPPING(ch) = 1;
      GET_LEGS_STOPPING(ch) = 1;	  
      break;
    case 9:
      break;
    case 10: 
      GET_BODY_AC(ch) = 25;
      GET_ARMS_AC(ch) = 25;
      GET_HEAD_AC(ch) = 25;
      GET_LEGS_AC(ch) = 25;	  	  
      break;
    case 11: 
	break;	  
    case 12:
      GET_BODY_STOPPING(ch) = 1;
      GET_ARMS_STOPPING(ch) = 1;
      GET_HEAD_STOPPING(ch) = 1;
      GET_LEGS_STOPPING(ch) = 1;	  	
      break;
    }
  }
}
/* deal with newcomers and other non-playing sockets */
void	nanny(struct descriptor_data *d, char *arg)
{
  char	buf[100], *nextchar;
  struct ban_list_element *ban_node;   
  int player_i,i_to_add,points_to_min,points_to_max;
  int nplaying,load_result,points,ierr;
  char	tmp_name[20];
  struct char_file_u tmp_store;
  struct char_data *tmp_ch, *tch;
  struct descriptor_data *k, *next;
  extern struct descriptor_data *descriptor_list;
  extern short r_mortal_start_room;
  extern short r_immort_start_room;
  extern short r_frozen_start_room;
  short load_room;
  
  ACMD(do_look);
  ACMD(do_color);
  int	load_char(char *name, struct char_file_u *char_element);
  void display_stats(struct descriptor_data *d);
  
  switch (STATE(d)) {
    
  case CON_QCOLOR:
    if (!d->character) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      d->character->desc = d;
    }
    for (; isspace(*arg); arg++)
      ;
    
    switch (*arg) {
    case 'x':
      d->color = FALSE;
      break;
    case 'X':
      d->color = TRUE;
      break;
    case 'n':
    case 'N':
      d->color = FALSE;
      SEND_TO_Q(GREETINGS, d);
      break;
    default:
      d->color = TRUE;
      SEND_TO_Q(GREET_ANSI, d);
      break;}
    
    STATE(d) = CON_NME;
    
    SEND_TO_Q("What is thy name traveller? ", d);
    return;
    
  case CON_NME:		/* wait for input of name	*/

    for (; isspace(*arg); arg++)
      ;
    
    if (!*arg)
      close_socket(d);
    else {
      if ((_parse_name(arg, tmp_name)) || fill_word(strcpy(buf,tmp_name)) ||
	  strlen(tmp_name) > MAX_NAME_LENGTH || strlen(tmp_name) < 2 ||
	  !Valid_Name(tmp_name)) {
	SEND_TO_Q("Invalid name, please try another.\r\n", d);
	SEND_TO_Q("Name: ", d);
	return;
      }
      if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
	d->pos = player_i;
	store_to_char(&tmp_store, d->character);
	d->last_logon = tmp_store.last_logon;
	if (PLR_FLAGGED(d->character, PLR_DELETED)) {
	  nplaying = 0;
	  for (tch = character_list; tch; tch = tch->next)
	    if (!IS_NPC(tch) && (GET_LEVEL(tch) < LEVEL_BUILDER)
		&& tch->desc)
	      nplaying++;
	  if (nplaying >= MAX_PLAYERS){
	    SEND_TO_Q("Archipelago is full right now, try again later.\r\n", d);
	    STATE(d) = CON_CLOSE;
	    return;	      
	  }
	  free_char(d->character);
	  CREATE(d->character, struct char_data, 1);
	  clear_char(d->character);
	  d->character->desc = d;
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));
	  sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NMECNF;
	}
	else {
	  strcpy(d->pwd, tmp_store.pwd);
	  /* undo it just in case they are set */
	  nplaying = 0;
	  if (GET_LEVEL(d->character) < LEVEL_BUILDER){
	    for (tch = character_list; tch; tch = tch->next)
	      if (!IS_NPC(tch) && (GET_LEVEL(tch) < LEVEL_BUILDER)
		  && tch->desc)
		nplaying++;
	    if (nplaying >= MAX_PLAYERS){
	      SEND_TO_Q("Archipelago is full right now, try again later.\r\n", d);
	      STATE(d) = CON_CLOSE;
	      return;	      
	    }
	  }
	  REMOVE_BIT(PLR_FLAGS(d->character),
		     PLR_WRITING | PLR_BUILDING
		     | PLR_MAILING | PLR_CRYO
		     | PLR_NEEDCRLF | PLR_AFK);
	  SEND_TO_Q("Password: ", d);
	  echo_off(d->descriptor);
	  STATE(d) = CON_PWDNRM;
	}
      }
      else {
	/* player unknown gotta make a new */
	nplaying = 0;
	for (tch = character_list; tch; tch = tch->next)
	  if (!IS_NPC(tch) && (GET_LEVEL(tch) < LEVEL_BUILDER)
	      && tch->desc)
	    nplaying++;
	if (nplaying >= MAX_PLAYERS){
	  SEND_TO_Q("Archipelago is full right now, try again later.\r\n", d);
	  STATE(d) = CON_CLOSE;
	  return;	      
	}
	
	if (!Valid_Name(tmp_name)) {
	  SEND_TO_Q("Invalid name, please try another.\r\n", d);
	  SEND_TO_Q("Name: ", d);
	  return;
	}
	
	CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	strcpy(d->character->player.name, CAP(tmp_name));
	
	sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	SEND_TO_Q(buf, d);
	STATE(d) = CON_NMECNF;
      }
    }
    break;
  case CON_NMECNF:	/* wait for conf. of new name	*/
    /* skip whitespaces */
    for (; isspace(*arg); arg++)
      ;
    
    if (*arg == 'y' || *arg == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
	sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
	        GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_GOD, TRUE);
	SEND_TO_Q("Sorry, new characters not allowed from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (restrict) {
	SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
	sprintf(buf, "Request for new char %s denied from %s (wizlock)",
	        GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_GOD, TRUE);
	STATE(d) = CON_CLOSE;
	return;
      }
      SEND_TO_Q("New character.\r\n", d);
      if(!Crash_get_filename(GET_NAME(d->character), buf)){
	SEND_TO_Q("Wierd character name error.\r\n",d);
	sprintf(buf, "Wierd char name %s denied from %s",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_GOD, TRUE);
	STATE(d) = CON_CLOSE;
	return;
	
      }
      ierr = remove(buf);
      sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      echo_off(d->descriptor);
      STATE(d) = CON_PWDGET;
    } else if (*arg == 'n' || *arg == 'N') {
      SEND_TO_Q("Ok, what IS it, then? ", d);
      free(GET_NAME(d->character));
      d->character->player.name = 0;
      STATE(d) = CON_NME;
    } else { /* Please do Y or N */
      SEND_TO_Q("Please type Yes or No: ", d);
    }
    break;
  case CON_PWDNRM:	/* get pwd for known player	*/
    
    /* turn echo back on */
    echo_on(d->descriptor);
    
    /* skip whitespaces */
    for (; isspace(*arg); arg++) 
      ;
    if (!*arg)
      close_socket(d);
    else {
      if (strncmp(CRYPT(arg, d->pwd), d->pwd, MAX_PWD_LENGTH)) {
	sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	mudlog(buf, BRF, LEVEL_GOD, TRUE);
	d->character->specials2.bad_pws++;
	save_char(d->character, NOWHERE);
	if (++(d->bad_pws) >= 3) { /* 3 strikes and you're out. */
	  SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
	  STATE(d) = CON_CLOSE;
	} else {
	  SEND_TO_Q("Wrong password.\r\nPassword: ", d);
	  echo_off(d->descriptor);
	}
	return;
      }
      
      save_char(d->character, NOWHERE);
      
      if (isbanned(d->host) == BAN_SELECT && 
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Connection attempt for %s denied from %s",
	        GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_GOD, TRUE);
	return;
      }
      if (ierr = is_selfbanned(d)) {
	sprintf(buf, "You have temporarily banned this character.\r\nTry again in %d hours.\r\n", ierr);
	SEND_TO_Q(buf, d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Connection attempt for %s denied from %s",
	        GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_GOD, TRUE);
	return;
      }

      if (GET_LEVEL(d->character) < restrict) {
	SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
	        GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LEVEL_GOD, TRUE);
	return;
      }
      
      /* first, check for switched characters */
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
	if (IS_NPC(tmp_ch) && tmp_ch->desc && tmp_ch->desc->original &&
	    GET_IDNUM(tmp_ch->desc->original) == GET_IDNUM(d->character)) {
	  SEND_TO_Q("Disconnecting.", tmp_ch->desc);
	  free_char(d->character);
	  d->character = tmp_ch->desc->original;
	  d->character->desc = d;
	  tmp_ch->desc->character = 0;
	  tmp_ch->desc->original = 0;
	  STATE(tmp_ch->desc) = CON_CLOSE;
	  d->character->specials.timer = 0;
	  SEND_TO_Q("Reconnecting to unswitched char.", d);
	  REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_BUILDING | PLR_WRITING);
	  STATE(d) = CON_PLYNG;
	  sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
	  mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
	  if (d->character->in_room != NOWHERE)
	    do_look(d->character,"",0,0);
	  return;
	}
      
      /* now check for linkless and usurpable */
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
	if (!IS_NPC(tmp_ch) && 
	    GET_IDNUM(d->character) == GET_IDNUM(tmp_ch)) {
	  if (!tmp_ch->desc) {
	    SEND_TO_Q("Reconnecting.\r\n", d);
	    act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
	    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
	    mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
	  } else {
	    sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
		    GET_NAME(tmp_ch));
	    mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(tmp_ch)), TRUE);
	    SEND_TO_Q("This body has been usurped!\r\n", tmp_ch->desc);
	    STATE(tmp_ch->desc) = CON_CLOSE;
	    tmp_ch->desc->character = 0;
	    tmp_ch->desc = 0;
	    SEND_TO_Q("You take over your own body, already in use!\r\n", d);
	    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
		"$n's body has been taken over by a new spirit!",
		TRUE, tmp_ch, 0, 0, TO_ROOM);
	  }
	  
	  free_char(d->character);
	  tmp_ch->desc = d;
	  d->character = tmp_ch;
	  tmp_ch->specials.timer = 0;
	  REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_BUILDING | PLR_WRITING | PLR_NEEDCRLF);
	  STATE(d) = CON_PLYNG;
	  if (d->color)
	    do_color(d->character,"complete",0,0);
	  else
	    do_color(d->character,"off",0,0);
	  if (d->character->in_room != NOWHERE)
	    do_look(d->character,"",0,0);
	  return;
	}
      
      sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
      mudlog(buf, BRF, MAX(LEVEL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
      if (GET_LEVEL(d->character) >= LEVEL_BUILDER ){
	if (imotd_time > d->last_logon){
	  page_string(d,imotd,1);
	  if(d->showstr_point){
	    SEND_TO_Q("\r\n\r\n*** PRESS RETURN: ", d);
	    STATE(d) = CON_MOTD;
	    break;}
	}
	else{
	  STATE(d) = CON_SLCT;
	  if (d->color)
	    SEND_TO_Q(ANSI_MENU,d);
	  else
	    SEND_TO_Q(MENU,d);
	  return;}
      }
      else {
	if(motd_time > d->last_logon){
	  page_string(d,motd,1);
	  if(d->showstr_point){
	    SEND_TO_Q("\r\n\r\n*** PRESS RETURN: ", d);
	    STATE(d) = CON_MOTD;
	    break;}
	}
	else{
	  STATE(d) = CON_SLCT;
	  if (d->color)
	    SEND_TO_Q(ANSI_MENU,d);
	  else
	    SEND_TO_Q(MENU,d);
	  return;}
      }
      
      load_result = d->character->specials2.bad_pws;
      d->character->specials2.bad_pws = 0;
      
      if (load_result) {
	sprintf(buf, "\r\n\r\n\007\007\007"
	        "%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
	        CCRED(d->character, C_SPR), load_result,
	        (load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
	SEND_TO_Q(buf, d);
      }
      
      SEND_TO_Q("\r\n\r\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
    }
    break;
    
  case CON_PWDGET:	/* get pwd for new player	*/
    
    /* skip whitespaces */
    for (; isspace(*arg); arg++)
      ;
    
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) <= 3 || 
	!str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Passwords must be more than 3 chars long.\r\n", d);
      SEND_TO_Q("Passwords cannot be more than 10 chars long.\r\n", d);
      SEND_TO_Q("Passwords cannot be your character name.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }
    
    strncpy(d->pwd, CRYPT(arg, d->character->player.name), MAX_PWD_LENGTH);
    *(d->pwd + MAX_PWD_LENGTH) = '\0';
    
    SEND_TO_Q("\r\nPlease retype password: ", d);
    STATE(d) = CON_PWDCNF;
    break;
      
  case CON_PWDCNF:	/* get confirmation of new pwd	*/
    
    /* skip whitespaces */
    for (; isspace(*arg); arg++)
      ;
    
    if (strncmp(CRYPT(arg, d->pwd), d->pwd, MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      STATE(d) = CON_PWDGET;
      return;
    }
    
    /* turn echo back on */
    echo_on(d->descriptor);
    
    SEND_TO_Q("\r\nWhat is your sex (M/F)? ", d);
    STATE(d) = CON_QSEX;
    break;

  case CON_QSEX:	/* query sex of new user	*/
    
    /* skip whitespaces */
    for (; isspace(*arg); arg++)
      ;
    switch (*arg) {
    case 'm':
    case 'M':
	/* sex MALE */
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      /* sex FEMALE */
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      SEND_TO_Q("That's not a sex..\r\n", d);
      SEND_TO_Q("What IS your sex? :", d);
      return;
      break;
    }
    
    SEND_TO_Q("\r\nChoose and alignment [Good, Neutral, Evil]?: ", d);
    STATE(d) = CON_QALIGN;
    break;
    
  case CON_QALIGN:	/* query alignment of new user	*/
    
    /* skip whitespaces */
     for (; isspace(*arg); arg++)
       ;
     switch (*arg) {
     case 'g':
     case 'G':
       /* Good*/
       d->character->specials2.alignment = 1000;
       break;
     case 'e':
     case 'E':
       /* Evil */
       d->character->specials2.alignment = -1000;
       break;
     case 'n':
     case 'N':
       /* Neutral */
       d->character->specials2.alignment = 0;
       break;	 
     default:
       SEND_TO_Q("That's not an alignment..\r\n", d);
       SEND_TO_Q("What IS your alignment? :", d);
       return;
       break;
     }
     
      
/*      for(i_to_add = 17;i_to_add;i_to_add--){ */
/*        switch(number(0,9)) */
/* 	 { */
/* 	 case 0: */
/* 	   if (GET_RAW_STR(d->character) >= pc_str[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_STR(d->character) += 1; */
/* 	   break; */
/* 	 case 1: */
/* 	   if (GET_RAW_INT(d->character) >= pc_int[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_INT(d->character) += 1; */
/* 	   break; */
/* 	 case 2: */
/* 	   if (GET_RAW_WIS(d->character) >= pc_wis[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_WIS(d->character) += 1; */
/* 	   break; */
/* 	 case 3: */
/* 	   if (GET_RAW_DEX(d->character) >= pc_dex[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_DEX(d->character) += 1; */
/* 	   break;	       */
/* 	 case 4: */
/* 	   if (GET_RAW_CON(d->character) >= pc_con[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_CON(d->character) += 1; */
/* 	   break; */
/* 	 case 5: */
/* 	   if (GET_RAW_CHR(d->character) >= pc_chr[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_CHR(d->character) += 1; */
/* 	   break; */
/* 	 case 6: */
/* 	   if (GET_RAW_PER(d->character) >= pc_per[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_PER(d->character) += 1; */
/* 	   break; */
/* 	 case 7: */
/* 	   if (GET_RAW_GUI(d->character) >= pc_gui[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_GUI(d->character) += 1; */
/* 	   break; */
/* 	 case 8: */
/* 	   if (GET_RAW_FOC(d->character) >= pc_foc[GET_RACE(d->character)].max) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_FOC(d->character) += 1; */
/* 	   break; */
/* 	 case 9: */
/* 	   if (GET_RAW_LUC(d->character) > 29) */
/* 	     i_to_add++; */
/* 	   else */
/* 	     GET_RAW_LUC(d->character) += 1; */
/* 	   break; */
/* 	 } */
/*      } */

     SEND_TO_Q("\r\n\r\nSelect a race:\r\n  [E]lven ... InfraVision Dex+ Int+ Wis+ Str- Con-\r\n  H[a]lfling ... Gui+ Chr+ Str- Int- Wis-\r\n  [G]iant ... Str+ Con+ Int- Wis-\r\n  [H]uman\r\n  Ha[l]f Elf... InfraVision Str- Con- Per+ Gui+ Chr+ Dex+\r\n  Sel[k]ie ... WaterBreath Gui- Dex-\r\n  [D]warf ... InfraVision Str+ Con+ Dex+ Wis+ Int-\r\n  [P]ixie ... Fly Str- Con- Wis+ Int+ Foc+\r\n  G[n]ome ... Wis+ Str- Con- Gui-\r\n  [O]gier ... Int+ Wis+ Foc+ Str+ Con- Dex- Gui- Chr-\r\n  Amar[y]a ... Infravision Dex+ Str+ Gui+ Int- Wis- Foc- ", d);
     SEND_TO_Q("\r\n  [T]roll ... Fast Healing Wis- Chr- Per- Gui- Int- Dex- Str+ Con+", d);
     SEND_TO_Q("\r\nRace: ", d);
     STATE(d) = CON_QRACE;
     
     break;


   case CON_QSTAT:
       {
	   for (; isspace(*arg); arg++)
	       ;
	   *arg = LOWER(*arg);
	   if (*arg != 's' && *arg != 'i' && *arg != 'w' && *arg != 'c' &&
	       *arg != 'd' && *arg != 'p' && *arg != 'h' && *arg != 'g' &&
	       *arg != 'q' && *arg != 'f' && *arg != '?') {
	       SEND_TO_Q("\r\nThat's not a stat. \r\nStat: ",d);
	       return;
	   }
	   SEND_TO_Q("How many points to add: ",d);
	   switch(*arg) 
	       {
	       case 's':{
		   STATE(d) = CON_QSTATS;
		   break;}
	       case 'i':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATI;
		   break;}
	       case 'w':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATW;
		   break;}
	       case 'd':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATD;
		   break;}
	       case 'c':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATC;
		   break;}
	       case 'h':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATH;
		   break;}
	       case 'p':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATP;
		   break;}
	       case 'g':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATG;
		   break;}
	       case 'f':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATF;
		   break;}
	       case 'v':{
		   SEND_TO_Q("How many points to add: ",d);
		   STATE(d) = CON_QSTATDE;
		   break;}		   
               case '?':{
                   SEND_TO_Q("\r\n[S]tr - Strength",d);
                   SEND_TO_Q("\r\n[I]nt - Intelligence",d);
                   SEND_TO_Q("\r\n[W]is - Wisdom",d);
                   SEND_TO_Q("\r\n[D]ex - Dexterity",d);
                   SEND_TO_Q("\r\n[F]oc - Focus (concentration)",d);
                   SEND_TO_Q("\r\n[C]on - Constituition",d);
                   SEND_TO_Q("\r\nC[h]r - Charisma",d);
                   SEND_TO_Q("\r\n[P]er - Perception",d);
                   SEND_TO_Q("\r\n[G]ui - Guile",d);
                   display_stats(d);
                   break;}
	       case 'q':{
		 init_char(d->character);
		 if (d->pos < 0)
		   d->pos = create_entry(GET_NAME(d->character));
		 save_char(d->character, NOWHERE);
		 page_string(d,motd,1);
		 sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
		 mudlog(buf, NRM, LEVEL_BUILDER, TRUE);
		 if(d->showstr_point){
		   SEND_TO_Q("\r\n\r\n*** PRESS RETURN: ", d);
		   STATE(d) = CON_MOTD;
		   break;
		 }
		 STATE(d) = CON_RMOTD;
		 break;}
	       }
	   break;
	   
       }
   case CON_QSTATS:
     for (; isspace(*arg); arg++)
       ;
     if (atoi(arg) > d->stat_points){
       SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
       return;}
      points = atoi(arg);
      if ((GET_RAW_STR(d->character)) >= pc_str[GET_RACE(d->character)].max && (points > 0)){
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if ((GET_RAW_STR(d->character)) <= pc_str[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if (points < 0){
	  points_to_min = GET_RAW_STR(d->character) -pc_str[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_STR(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_STR(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
	  points_to_max = pc_str[GET_RACE(d->character)].max - GET_RAW_STR(d->character);
	  if (points_to_max < points) {
	      GET_RAW_STR(d->character) += points_to_max;
	      d->stat_points -= points_to_max;
	  }
	  else {
	      GET_RAW_STR(d->character) += points;
	      d->stat_points -= points;
	  }
      }
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;

   case CON_QSTATI:
       for (; isspace(*arg); arg++)
	       ;
      if (atoi(arg) > d->stat_points){
	SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
	  return;}
      points = atoi(arg);
      if ((GET_RAW_INT(d->character) >= pc_int[GET_RACE(d->character)].max) && (points > 0)){
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if ((GET_RAW_INT(d->character)) <= pc_int[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if (points < 0){
	  points_to_min = GET_RAW_INT(d->character) - pc_int[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_INT(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_INT(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
	  points_to_max = pc_int[GET_RACE(d->character)].max - GET_RAW_INT(d->character);
	  if (points_to_max < points) {
	      GET_RAW_INT(d->character) += points_to_max;
	      d->stat_points -= points_to_max;
      }
	  else {
	      GET_RAW_INT(d->character) += points;
	  d->stat_points -= points;
	  }
      }
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;

   case CON_QSTATW:
       for (; isspace(*arg); arg++)
	       ;
      if (atoi(arg) > d->stat_points){
	  SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
	  return;}
      points = atoi(arg);
      if ((GET_RAW_WIS(d->character) >= pc_wis[GET_RACE(d->character)].max) && (points > 0)) {
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
      	  break;}
      if ((GET_RAW_WIS(d->character)) <= pc_wis[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      
      if (points < 0){
	  points_to_min = GET_RAW_WIS(d->character) - pc_wis[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_WIS(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_WIS(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
      points_to_max = pc_wis[GET_RACE(d->character)].max - GET_RAW_WIS(d->character);
      if (points_to_max < points) {
	  GET_RAW_WIS(d->character) += points_to_max;
	  d->stat_points -= points_to_max;
      }
      else {
	  GET_RAW_WIS(d->character) += points;
	  d->stat_points -= points;
      }
      }
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;
   case CON_QSTATD:
       for (; isspace(*arg); arg++)
	       ;
      if (atoi(arg) > d->stat_points){
	SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
	return;}
      points = atoi(arg);
      if ((GET_RAW_DEX(d->character) >= pc_dex[GET_RACE(d->character)].max) && (points > 0)){
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if ((GET_RAW_DEX(d->character)) <= pc_dex[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      
      if (points < 0){
	  points_to_min = GET_RAW_DEX(d->character) -pc_dex[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_DEX(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_DEX(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
	  points_to_max = pc_dex[GET_RACE(d->character)].max - GET_RAW_DEX(d->character);
	  if (points_to_max < points) {
	      GET_RAW_DEX(d->character) += points_to_max;
	      d->stat_points -= points_to_max;
	  }
	  else {
	      GET_RAW_DEX(d->character) += points;
	      d->stat_points -= points;
	  }
      }
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;
   case CON_QSTATC:
     for (; isspace(*arg); arg++)
       ;
     if (atoi(arg) > d->stat_points){
       SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
       return;}
     points = atoi(arg);
     if ((GET_RAW_CON(d->character) >= pc_con[GET_RACE(d->character)].max) && (points > 0)){
       SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
       STATE(d) = CON_QSTAT;
       display_stats(d);
       break;}
      if ((GET_RAW_CON(d->character)) <= pc_con[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
     
     if (points < 0){
       points_to_min = GET_RAW_CON(d->character) - pc_con[GET_RACE(d->character)].min;
       if (points_to_min < abs(points)) {
	 GET_RAW_CON(d->character) -= points_to_min;
	 d->stat_points += points_to_min;
       }
       else {
	 GET_RAW_CON(d->character) += points;
	 d->stat_points -= points;
       }
       
     }
     else{
       points_to_max = pc_con[GET_RACE(d->character)].max - GET_RAW_CON(d->character);
       if (points_to_max < points) {
	 GET_RAW_CON(d->character) += points_to_max;
	 d->stat_points -= points_to_max;
       }
       else {
	 GET_RAW_CON(d->character) += points;
	 d->stat_points -= points;
       }
     }
     STATE(d) = CON_QSTAT;
     display_stats(d);
     break;
     
   case CON_QSTATH:
     for (; isspace(*arg); arg++)
       ;
     if (atoi(arg) > d->stat_points){
       SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
       return;}
      points = atoi(arg);
      if ((GET_RAW_CHR(d->character) >= pc_chr[GET_RACE(d->character)].max) && (points >0))
	  {
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if ((GET_RAW_CHR(d->character)) <= pc_chr[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      
      if (points < 0){
	  points_to_min = GET_RAW_CHR(d->character) - pc_chr[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_CHR(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_CHR(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
      points_to_max = pc_chr[GET_RACE(d->character)].max - GET_RAW_CHR(d->character);
      if (points_to_max < points) {
	  GET_RAW_CHR(d->character) += points_to_max;
	  d->stat_points -= points_to_max;
      }
      else {
	  GET_RAW_CHR(d->character) += points;
	  d->stat_points -= points;
      }}
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;
   case CON_QSTATP:
       for (; isspace(*arg); arg++)
	       ;
      if (atoi(arg) > d->stat_points){
	  SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
	  return;}
      points = atoi(arg);
      if ((GET_RAW_PER(d->character) >= pc_per[GET_RACE(d->character)].max) && (points > 0))
	  {
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if ((GET_RAW_PER(d->character)) <= pc_per[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      
      if (points < 0){
	  points_to_min = GET_RAW_PER(d->character) - pc_per[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_PER(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_PER(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
      points_to_max = pc_per[GET_RACE(d->character)].max - GET_RAW_PER(d->character);
      if (points_to_max < points) {
	  GET_RAW_PER(d->character) += points_to_max;
	  d->stat_points -= points_to_max;
      }
      else {
	  GET_RAW_PER(d->character) += points;
	  d->stat_points -= points;
      }}
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;
   case CON_QSTATG:
       for (; isspace(*arg); arg++)
	       ;
      if (atoi(arg) > d->stat_points){
	  SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
	  return;}
      points = atoi(arg);
      if ((GET_RAW_GUI(d->character) >= pc_gui[GET_RACE(d->character)].max) && (points > 0)){
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if ((GET_RAW_GUI(d->character)) <= pc_gui[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      
      if (points < 0){
	  points_to_min = GET_RAW_GUI(d->character) -pc_gui[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_GUI(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_GUI(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
      points_to_max = pc_gui[GET_RACE(d->character)].max - GET_RAW_GUI(d->character);
      if (points_to_max < points) {
	  GET_RAW_GUI(d->character) += points_to_max;
	  d->stat_points -= points_to_max;
      }
      else {
	  GET_RAW_GUI(d->character) += points;
	  d->stat_points -= points;
      }}
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;

   case CON_QSTATF:
       for (; isspace(*arg); arg++)
	       ;
      if (atoi(arg) > d->stat_points){
	  SEND_TO_Q("\r\nThats a bit too much. \r\nHow many points to add: ",d);
	  return;}
      points = atoi(arg);
      if ((GET_RAW_FOC(d->character) >= pc_foc[GET_RACE(d->character)].max) && (points > 0)){
	  SEND_TO_Q("\r\nSorry, that stat is now maxed.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      if ((GET_RAW_FOC(d->character)) <= pc_foc[GET_RACE(d->character)].min && (points < 0)){
	  SEND_TO_Q("\r\nSorry, that stat is at minimum.\r\n",d);
	  STATE(d) = CON_QSTAT;
	  display_stats(d);
	  break;}
      
      if (points < 0){
	  points_to_min = GET_RAW_FOC(d->character) - pc_foc[GET_RACE(d->character)].min;
	  if (points_to_min < abs(points)) {
	      GET_RAW_FOC(d->character) -= points_to_min;
	      d->stat_points += points_to_min;
	  }
	  else {
	      GET_RAW_FOC(d->character) += points;
	      d->stat_points -= points;
	  }

      }
      else{
      points_to_max = pc_foc[GET_RACE(d->character)].max - GET_RAW_FOC(d->character);
      if (points_to_max < points) {
	  GET_RAW_FOC(d->character) += points_to_max;
	  d->stat_points -= points_to_max;
      }
      else {
	  GET_RAW_FOC(d->character) += points;
	  d->stat_points -= points;
      }}
      STATE(d) = CON_QSTAT;
      display_stats(d);
      break;
      
   case CON_QRACE:
      /* skip whitespaces */
      for (; isspace(*arg); arg++)
	 ;
      *arg = LOWER(*arg);
      if (*arg != 'a' && *arg != 'e' && *arg != 'g' && *arg != 'h' && *arg != 'l' && *arg != 'o' && *arg != 'd' && *arg != 'p' && *arg != 'y' && *arg != 'n' && *arg != 'k' && *arg != 't') {
	 SEND_TO_Q("\r\nThat's not a race.\r\nRace: ", d);
	 return;
      }

      switch (*arg) {
      case 'h':
	d->stat_points = 65;
	GET_RACE(d->character) = RACE_HUMAN;
	break;
      case 'e': 
	d->stat_points = 60;
	GET_RACE(d->character) = RACE_ELVEN; 
	GET_HEIGHT(d->character) -= 10;
	GET_WEIGHT(d->character) -= 250;
	break;
      case 'g':
	d->stat_points = 50;	
	GET_RACE(d->character) = RACE_GIANT; 
	GET_HEIGHT(d->character) += 100;
	GET_WEIGHT(d->character) += 2000;
	break;
      case 'a':
	d->stat_points = 60;	
	GET_RACE(d->character) = RACE_HALFLING; 
	GET_HEIGHT(d->character) -= 70;
	GET_WEIGHT(d->character) -= 600;
	break;
      case 'l':
	d->stat_points = 62;	
	GET_RACE(d->character) = RACE_HALF_ELF; 
	GET_HEIGHT(d->character) -= 10;
	GET_WEIGHT(d->character) -= 150;
	break;
      case 'o':
	d->stat_points = 60;	
	GET_RACE(d->character) = RACE_OGIER; 
	GET_HEIGHT(d->character) += 50;
	GET_WEIGHT(d->character) += 500;
	break;
      case 'n':
	d->stat_points = 60;	
	GET_RACE(d->character) = RACE_GNOME;
	GET_WEIGHT(d->character) -= 200;
	GET_HEIGHT(d->character) -= 60;
	break;
      case 'd': 
	d->stat_points = 50;	
	GET_RACE(d->character) = RACE_DWARF;
	GET_WEIGHT(d->character) += 200;
	GET_HEIGHT(d->character) -= 50;
	break;
      case 'k': 
	d->stat_points = 60;	
	GET_RACE(d->character) = RACE_MERMAN;
	GET_WEIGHT(d->character) -= 100;
	GET_HEIGHT(d->character) -= 10;
	break;
      case 'y': 
	d->stat_points = 55;	
	GET_RACE(d->character) = RACE_AMARYA;
	GET_WEIGHT(d->character) -= 300;
	GET_HEIGHT(d->character) -= 20;
	break;	  
      case 't':
	d->stat_points = 50;	
	GET_RACE(d->character) = RACE_TROLL;
	GET_WEIGHT(d->character) += 1000;
	GET_HEIGHT(d->character) += 50;
	break;
      case 'p': 
	d->stat_points = 55;	
	GET_RACE(d->character) = RACE_PIXIE;
	GET_WEIGHT(d->character) -= 1400;
	GET_HEIGHT(d->character) -= 160;
	break;
      }
      GET_RAW_STR(d->character)= pc_str[GET_RACE(d->character)].min;
      GET_RAW_INT(d->character)= pc_int[GET_RACE(d->character)].min;
      GET_RAW_WIS(d->character)= pc_wis[GET_RACE(d->character)].min;
      GET_RAW_DEX(d->character)= pc_dex[GET_RACE(d->character)].min;
      GET_RAW_CON(d->character)= pc_con[GET_RACE(d->character)].min;
      GET_RAW_CHR(d->character)= pc_chr[GET_RACE(d->character)].min;
      GET_RAW_PER(d->character)= pc_per[GET_RACE(d->character)].min;
      GET_RAW_GUI(d->character)= pc_gui[GET_RACE(d->character)].min;
      GET_RAW_FOC(d->character)= pc_foc[GET_RACE(d->character)].min;
      GET_RAW_LUC(d->character)=number(15,22);
      
      set_race_characteristics(d->character);
      SEND_TO_Q("\r\n[S]tr - Strength",d);
      SEND_TO_Q("\r\n[I]nt - Intelligence",d);
      SEND_TO_Q("\r\n[W]is - Wisdom",d);
      SEND_TO_Q("\r\n[D]ex - Dexterity",d);
      SEND_TO_Q("\r\n[F]oc - Focus (concentration)",d);
      SEND_TO_Q("\r\n[C]on - Constituition",d);
      SEND_TO_Q("\r\nC[h]r - Charisma",d);
      SEND_TO_Q("\r\n[P]er - Perception",d);
      SEND_TO_Q("\r\n[G]ui - Guile",d);
      SEND_TO_Q("\r\nType ? when it asks which stat to change to see this list again",d);
      SEND_TO_Q("\r\n\r\nThese are your base stats, all stats are on a 1-30 scale.\r\n",d);

      STATE(d) = CON_QSTAT;
      
      display_stats(d);
      
      break;

   case CON_RMOTD:		/* read CR after printing motd	*/
       if (d->color)
	   SEND_TO_Q(ANSI_MENU,d);
       else
	   SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
      break;

   case CON_MOTD:
      if (d->showstr_point){
       page_string(d,d->showstr_point,0);
       SEND_TO_Q("\r\n\r\n*** PRESS RETURN: ", d);
       break;}
      else
	  {
	      load_result = d->character->specials2.bad_pws;
	      d->character->specials2.bad_pws = 0;
	      if (load_result)
		  {
		      sprintf(buf, "\r\n\r\n\007\007\007"
			      "%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
			      CCRED(d->character, C_SPR), load_result,
			      (load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
		      SEND_TO_Q(buf, d);
		      SEND_TO_Q("\r\n\r\n*** PRESS RETURN: ", d);}
	      if (d->color)
		  SEND_TO_Q(ANSI_MENU,d);
	      else
		  SEND_TO_Q(MENU,d);
	      STATE(d) = CON_SLCT;
	      break;
	  }

   case CON_SLCT:		/* get selection from main menu	*/

      /* skip whitespaces */
      for (; isspace(*arg); arg++)
	 ;
      switch (*arg) {
      case '0':
	 close_socket(d);
	 break;

      case '1':
         /* this code is to prevent people from multiply logging in */
         for (k = descriptor_list; k; k=next) {
            next = k->next;
            if (k->character && GET_NAME(k->character) &&(k != d) &&
		!str_cmp(GET_NAME(k->character), GET_NAME(d->character))){
                  SEND_TO_Q("Already playing.\r\n",d);
                  STATE(d) = CON_CLOSE;
                  return;
	       }
	 }
	 reset_char(d->character);
	 if ((load_result = Crash_load(d->character)))
	    d->character->in_room = NOWHERE;
	 save_char(d->character, NOWHERE);
	 send_to_char(WELC_MESSG, d->character);
	 d->character->next = character_list;
	 character_list = d->character;

	 if (GET_LEVEL(d->character) >= LEVEL_BUILDER) {
	    if (PLR_FLAGGED(d->character, PLR_LOADROOM)) {
		if ((load_room = real_room(GET_LOADROOM(d->character))) < 0)
		   load_room = r_immort_start_room;
	    } else
		load_room = r_immort_start_room;
	    if (PLR_FLAGGED(d->character, PLR_INVSTART))
	       GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
	 } else {
	    if (PLR_FLAGGED(d->character, PLR_FROZEN))
	       load_room = r_frozen_start_room;
	    else {
	       if (d->character->in_room == NOWHERE)
		  load_room = r_mortal_start_room;
	       else if ((load_room = real_room(d->character->in_room)) < 0)
		     load_room = r_mortal_start_room;
	    }
	 }
	 /* load aliases */
	 load_aliases(d->character);
	 char_to_room(d->character, load_room, FALSE);
	 d->character->specials2.rent_zone =
	   zone_table[world[d->character->in_room].zone].number;
	 
	 act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

	 STATE(d) = CON_PLYNG;
	 if (!GET_LEVEL(d->character)) {
	    do_start(d->character);
	    send_to_char(START_MESSG, d->character);
	 }
	 if (d->color)
	     do_color(d->character,"complete",0,0);
	 else
	     do_color(d->character,"off",0,0);
	 do_look(d->character, "", 0, 0);
	 if (has_mail(GET_NAME(d->character)))
	    send_to_char("You have mail waiting.\r\n", d->character);
	 if (load_result == 2) { /* rented items lost */
	    send_to_char("\r\n\007You could not afford your rent!\r\n"
	        "Your possesions have been donated to the Salvation Army!\r\n", d->character);
	 }
	 d->prompt_mode = 1;
	 break;

      case '2':
	 SEND_TO_Q("Enter the text you'd like others to see when they look at you.\r\n", d);
	 SEND_TO_Q("Terminate with a '@@'.\r\n", d);
	 if (d->character->player.description) {
	    SEND_TO_Q("Old description :\r\n", d);
	    SEND_TO_Q(d->character->player.description, d);
	    free(d->character->player.description);
	    d->character->player.description = 0;
	 }
	 d->str = &d->character->player.description;
	 d->max_str = 240;
	 STATE(d) = CON_EXDSCR;
	 break;

      case '3':
	 SEND_TO_Q(background, d);
	 SEND_TO_Q("\r\n\r\n*** PRESS RETURN:", d);
	 STATE(d) = CON_RMOTD;
	 break;

      case '4':
	 SEND_TO_Q("Enter your old password: ", d);
	 echo_off(d->descriptor);
	 STATE(d) = CON_PWDNQO;
	 break;

      case '5':
	SEND_TO_Q("\r\nEnter your password for verification: ", d);
	echo_off(d->descriptor);
	STATE(d) = CON_DELCNF1;
        break;
	
      case '6':
	SEND_TO_Q("\r\nEnter your password for verification: ", d);
	echo_off(d->descriptor);
	STATE(d) = CON_QBANCON1;
        break;

      default:
	 SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
	 if (d->color)
	     SEND_TO_Q(ANSI_MENU,d);
	 else
	     SEND_TO_Q(MENU, d);
	 break;
      }

      break;

   case CON_PWDNQO:
      /* skip whitespaces */
      for (; isspace(*arg); arg++) ;
      if (strncmp(CRYPT(arg,d->pwd), d->pwd, MAX_PWD_LENGTH)) {
	 SEND_TO_Q("\r\nIncorrect password.\r\n", d);
	 if (d->color)
	     SEND_TO_Q(ANSI_MENU,d);
	 else
	     SEND_TO_Q(MENU, d);
	 STATE(d) = CON_SLCT;
	 echo_on(d->descriptor);
	 return;
      } else {
         SEND_TO_Q("\r\nEnter a new password: ", d);
         STATE(d) = CON_PWDNEW;
	 return;
      }
      break;
   case CON_PWDNEW:

      /* skip whitespaces */
      for (; isspace(*arg); arg++)
	 ;

      if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 || 
          !str_cmp(arg, GET_NAME(d->character))) {
	 SEND_TO_Q("\r\nIllegal password.\r\n", d);
	 SEND_TO_Q("Password: ", d);
	 return;
      }

      strncpy(d->pwd, CRYPT(arg, d->character->player.name), MAX_PWD_LENGTH);
      *(d->pwd + MAX_PWD_LENGTH) = '\0';

      SEND_TO_Q("\r\nPlease retype password: ", d);
      STATE(d) = CON_PWDNCNF;
      break;

   case CON_PWDNCNF:
      /* skip whitespaces */
      for (; isspace(*arg); arg++)
	 ;

      if (strncmp(CRYPT(arg, d->pwd), d->pwd, MAX_PWD_LENGTH)) {
	 SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
	 SEND_TO_Q("Password: ", d);
	 STATE(d) = CON_PWDNEW;
	 return;
      }

      SEND_TO_Q("\r\nDone.  You must enter the game to make the change final.\r\n", d);
      if (d->color)
	  SEND_TO_Q(ANSI_MENU,d);
      else      
	  SEND_TO_Q(MENU, d);
      echo_on(d->descriptor);
      STATE(d) = CON_SLCT;
      break;

   case CON_DELCNF1:
      echo_on(d->descriptor);
      for (; isspace(*arg); arg++);
      if (strncmp(CRYPT(arg,d->pwd), d->pwd, MAX_PWD_LENGTH)) {
	 SEND_TO_Q("\r\nIncorrect password.\r\n", d);
	 if (d->color)
	     SEND_TO_Q(ANSI_MENU,d);
	 else
	     SEND_TO_Q(MENU, d);
	 STATE(d) = CON_SLCT;
      } else {
	 SEND_TO_Q("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
		   "ARE YOU ABSOLUTELY SURE?\r\n\r\n"
		   "Please type \"yes\" to confirm: ", d);
	 STATE(d) = CON_DELCNF2;
      }
      break;

   case CON_DELCNF2:
      if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
         if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
            SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n",d);
            SEND_TO_Q("Character not deleted.\r\n\r\n",d);
            STATE(d) = CON_CLOSE;
            return;
	 }

         if (GET_LEVEL(d->character) < LEVEL_GRGOD)
            SET_BIT(PLR_FLAGS(d->character), PLR_DELETED);
         save_char(d->character, NOWHERE);
         Crash_delete_file(GET_NAME(d->character));
         sprintf(buf, "Character '%s' deleted!\r\nGoodbye.\r\n", GET_NAME(d->character));
         SEND_TO_Q(buf, d);
         sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character), GET_LEVEL(d->character));
         mudlog(buf, NRM, LEVEL_GOD, TRUE);
         STATE(d) = CON_CLOSE;
         return;
      } else {
	 SEND_TO_Q("Character not deleted.\r\n\r\n", d);
	 if (d->color)
	     SEND_TO_Q(ANSI_MENU,d);
	 else
	     SEND_TO_Q(MENU, d);
	 STATE(d) = CON_SLCT;
      }
      break;
   case CON_QBANCON1:
      echo_on(d->descriptor);
      for (; isspace(*arg); arg++);
      if (strncmp(CRYPT(arg,d->pwd), d->pwd, MAX_PWD_LENGTH)) {
	 SEND_TO_Q("\r\nIncorrect password.\r\n", d);
	 if (d->color)
	     SEND_TO_Q(ANSI_MENU,d);
	 else
	     SEND_TO_Q(MENU, d);
	 STATE(d) = CON_SLCT;
      } else {
	 SEND_TO_Q("\r\nYOU ARE ABOUT TO BAN YOURSELF FROM PLAYING THIS CHARACTER FOR 24 HOURS.\r\n"
		   "ARE YOU ABSOLUTELY SURE?\r\n\r\n"
		   "Please type \"yes\" to confirm: ", d);
	 STATE(d) = CON_QBANCON2;
      }
      break;

   case CON_QBANCON2:
      if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
	CREATE(ban_node, struct ban_list_element, 1);
	strncpy(ban_node->site, GET_NAME(d->character), BANNED_SITE_LENGTH);
	for (nextchar = ban_node->site; *nextchar; nextchar++)
	  *nextchar = tolower(*nextchar);
	ban_node->site[BANNED_SITE_LENGTH] = '\0';
	strncpy(ban_node->name, GET_NAME(d->character), MAX_NAME_LENGTH);
	ban_node->name[MAX_NAME_LENGTH] = '\0';
	ban_node->date = time(0);
	
	ban_node->type = BAN_SELF;
	ban_node->next = ban_list;
	ban_list = ban_node;
	
	sprintf(buf, "%s has self-banned for 24 hours.", GET_NAME(d->character));
	mudlog(buf, NRM, MAX(LEVEL_GOD, GET_INVIS_LEV(d->character)), TRUE);
	write_ban_list();
	STATE(d) = CON_CLOSE;
	return;
      } else {
	 SEND_TO_Q("Character not self-banned.\r\n\r\n", d);
	 if (d->color)
	     SEND_TO_Q(ANSI_MENU,d);
	 else
	     SEND_TO_Q(MENU, d);
	 STATE(d) = CON_SLCT;
      }
      break;

   case CON_CLOSE :
      close_socket(d);
      break;

   default:
      logg("SYSERR: Nanny: illegal state of con'ness");
      abort();
      break;
   }
}
