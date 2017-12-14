/* ************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                          *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* db.c split into two files db1.c and db2.c                              */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#define __DB_C__
#define ZCMD zone_table[zone].cmd[cmd_no]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "screen.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world = 0;         /* array of rooms			*/
int	top_of_world = 0;            /* ref to the top element of world	*/

struct char_data *character_list = 0; /* global linked list of chars	*/
struct index_data *mob_index;		/* index table for mobile file	*/
struct char_data *mob_proto;		/* prototypes for mobs		*/
int	top_of_mobt = 0;		/* top of mobile index table	*/

struct obj_data *object_list = 0;    /* the global linked list of objs	*/
struct index_data *obj_index;		/* index table for object file	*/
struct obj_data *obj_proto;		/* prototypes for objs		*/
int	top_of_objt = 0;		/* top of object index table	*/

struct zone_data *zone_table;	     /* zone table			*/
int	top_of_zone_table = 0;	     /* ref to top element of zone tab	*/
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages	*/

struct player_index_element *player_table = 0; /* index to player file	*/
FILE *player_fl = 0;			/* file desc of player file	*/
int	top_of_p_table = 0;		/* ref to top of table		*/
int	top_of_p_file = 0;		/* ref of size of p file	*/
long	top_idnum = 0;			/* highest idnum in use		*/

struct  obj_limit_list_type *obj_limit_table = 0;
FILE   *obj_limit_fl = 0;
int	top_of_ol_table = 0;		/* ref to top of ol table	*/
int	top_of_ol_file = 0;		/* ref of size of ol file	*/

int	no_mail = 0;			/* mail disabled?		*/
int	mini_mud = 0;			/* mini-mud mode?		*/
int	no_rent_check = 1;		/* skip rent check on boot?	*/
long	boot_time = 0;			/* time of mud boot		*/
int	restrict = 0;			/* level of game restriction	*/
short	r_mortal_start_room;		/* rnum of mortal start room	*/
short	r_immort_start_room;		/* rnum of immort start room	*/
short	r_frozen_start_room;		/* rnum of frozen start room	*/

char	*credits = 0;			/* game credits			*/
char	*news = 0;			/* mud news			*/
char	*motd = 0;			/* message of the day - mortals */
char	*imotd = 0;			/* message of the day - immorts */
char	*help = 0;			/* help screen			*/
char	*info = 0;			/* info page			*/
char	*wizlist = 0;			/* list of higher gods		*/
char	*immlist = 0;			/* list of peon gods		*/
char	*background = 0;		/* background story		*/
char	*handbook = 0;			/* handbook for new immortals	*/
char	*policies = 0;			/* policies page		*/
char    *building = 0;

int      slave_socket;                  /* socket for slave */
pid_t      slave_pid;
extern int	MAX_DESCRIPTORS_AVAILABLE;

FILE *help_fl = 0;			/* file for help text		*/
struct help_index_element *help_index = 0; /* the help table		*/
int	top_of_helpt;			/* top of help index table	*/
time_t  imotd_time;
time_t  motd_time;
struct stat motd_stat;
struct time_info_data time_info;	/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */
struct char_data *martha;
struct obj_data *don_box;

/* local functions */
void    save_all_object_limit_data(void);
void    load_limited(int obj_no);
void    extract_limited(int obj_no);
void    limited_to_char(int obj_no);
void    limited_from_char(int obj_no);
bool    is_maxed(int num, int mode);
void    save_an_object_limit_datum(int which);
void	index_boot(int mode);
void    boot_slave(void);
void	old_index_boot(int mode);
void    load_obj_limits_data(void);
void	load_zones(FILE *fl);
void	boot_the_shops(FILE *shop_f, char *filename);
void	assign_mobiles(void);
void	assign_objects(void);
void	assign_rooms(void);
void	assign_the_shopkeepers(void);
void	build_player_index(void);
void	char_to_store(struct char_data *ch, struct char_file_u *st);
void	store_to_char(struct char_file_u *st, struct char_data *ch);
int	is_empty(int zone_nr);
void	reset_zone(int zone);
int	file_to_string(char *name, char *buf);
int	file_to_string_alloc(char *name, char **buf);
void	check_start_rooms(void);
void	renum_world(void);
void	renum_objects(void);
void	renum_zone_table(void);
void	reset_time(void);
void	clear_char(struct char_data *ch);
void    obj_zone_table(void);
void	clear_room(struct room_data *room);
void    check_items_in_rent(long vnum);
void    check_obj_limits_data(void);
bool    twinkie_mob(struct char_data *mob);
int     total_cp(struct obj_data *obj, int *max, int *number);
void    extract_twinkie(struct char_data *mob);
void    remove_items(struct obj_data **obj);


/* external functions */
void    set_race_characteristics(struct char_data *ch);
extern struct descriptor_data *descriptor_list;
int     report_money_weight(int amount);
int     load_proto(int vnum, struct obj_data *obj);
int	add_follower(struct char_data *ch, struct char_data *leader);
bool    circle_follow(struct char_data *ch, struct char_data * victim);
void	load_messages(void);
void	weather_and_time(int mode);
void	assign_command_pointers(void);
void	assign_command_pointers_aa(void);
void	assign_command_pointers_ab(void);
void	assign_command_pointers_ac(void);
void	assign_command_pointers_ad(void);
void	assign_command_pointers_ae(void);
void	assign_command_pointers_af(void);
void	assign_command_pointers_ag(void);
void	assign_command_pointers_ah(void);
void	assign_command_pointers_ai(void);
void	assign_command_pointers_aj(void);
void	assign_command_pointers_ak(void);
void	assign_spell_pointers(void);
void	boot_social_messages(void);
void	update_obj_file(void); /* In objsave.c */
void	sort_commands(void);
void	load_banned(void);
void	Read_Invalid_List(void);
struct help_index_element *build_help_index(FILE *fl, int *num);
void	load_rooms(FILE *fl);
void	load_objects(FILE *obj_f);
void	load_mobiles(FILE *mob_f);
void    hitch(struct char_data *ch, struct obj_data *cart, struct char_data *vict, char quiet);
int     assess_item(struct obj_data *j);
int	shop_keeper(struct char_data *ch, void *me, int cmd, char *arg);
/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* this is necessary for the autowiz system */
void	reboot_wizlists(void) 
{
   file_to_string_alloc(WIZLIST_FILE, &wizlist);
   file_to_string_alloc(IMMLIST_FILE, &immlist);
}


ACMD(do_reboot)
{
   int	i, sv;

   one_argument(argument, arg);

   if (!str_cmp(arg, "all") || *arg == '*') {
      file_to_string_alloc(NEWS_FILE, &news);
      file_to_string_alloc(CREDITS_FILE, &credits);
      file_to_string_alloc(MOTD_FILE, &motd);
      file_to_string_alloc(IMOTD_FILE, &imotd);
      file_to_string_alloc(HELP_PAGE_FILE, &help);
      file_to_string_alloc(INFO_FILE, &info);
      file_to_string_alloc(WIZLIST_FILE, &wizlist);
      file_to_string_alloc(IMMLIST_FILE, &immlist);
      file_to_string_alloc(POLICIES_FILE, &policies);
      file_to_string_alloc(HANDBOOK_FILE, &handbook);
      file_to_string_alloc(BACKGROUND_FILE, &background);
      file_to_string_alloc(BUILDINGH_FILE, &building);
      if ((sv = stat(IMOTD_FILE,&motd_stat)) >= 0)
	  imotd_time = motd_stat.st_mtime;
      else
	  imotd_time = 0;
      if ((sv = stat(MOTD_FILE,&motd_stat)) >= 0)
	  motd_time = motd_stat.st_mtime;
      else
	  motd_time = 0;
   } else if (!str_cmp(arg, "wizlist"))
      file_to_string_alloc(WIZLIST_FILE, &wizlist);
   else if (!str_cmp(arg, "immlist"))
      file_to_string_alloc(IMMLIST_FILE, &immlist);
   else if (!str_cmp(arg, "news"))
      file_to_string_alloc(NEWS_FILE, &news);
   else if (!str_cmp(arg, "credits"))
      file_to_string_alloc(CREDITS_FILE, &credits);
   else if (!str_cmp(arg, "building_h"))
      file_to_string_alloc(BUILDINGH_FILE, &building);   
   else if (!str_cmp(arg, "motd")){
      file_to_string_alloc(MOTD_FILE, &motd);
            if ((sv = stat(MOTD_FILE,&motd_stat)) >= 0)
		motd_time = motd_stat.st_mtime;
      else
	  motd_time = 0;}
   else if (!str_cmp(arg, "imotd")){
      file_to_string_alloc(IMOTD_FILE, &imotd);
      if ((sv = stat(IMOTD_FILE,&motd_stat)) >= 0)
	  imotd_time = motd_stat.st_mtime;
   else
       imotd_time = 0;
   }
   else if (!str_cmp(arg, "help"))
      file_to_string_alloc(HELP_PAGE_FILE, &help);
   else if (!str_cmp(arg, "info"))
      file_to_string_alloc(INFO_FILE, &info);
   else if (!str_cmp(arg, "policy"))
      file_to_string_alloc(POLICIES_FILE, &policies);
   else if (!str_cmp(arg, "handbook"))
      file_to_string_alloc(HANDBOOK_FILE, &handbook);
   else if (!str_cmp(arg, "background"))
      file_to_string_alloc(BACKGROUND_FILE, &background);
   else if (!str_cmp(arg, "xhelp")) {
      if (help_fl)
	 fclose(help_fl);
      if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
	 return;
      else {
	 for (i = 0; i < top_of_helpt; i++)
	    free(help_index[i].keyword);
	 free(help_index);
	 help_index = build_help_index(help_fl, &top_of_helpt);
      }
   } else {
      send_to_char("Unknown reboot option.\r\n", ch);
      return;
   }

   send_to_char("Okay.\r\n", ch);
}


/* body of the booting system */
void	boot_db(void)
{
   int	i, sv;
   extern int	no_specials, no_limited_check;
   
   logg("Boot db -- BEGIN.");
   logg("Booting the Slave.");
   boot_slave();
   logg("Resetting the game time:");
   reset_time();

   logg("Reading news, credits, help, bground, info & motds.");
   file_to_string_alloc(NEWS_FILE, &news);
   file_to_string_alloc(CREDITS_FILE, &credits);
   file_to_string_alloc(MOTD_FILE, &motd);
   file_to_string_alloc(IMOTD_FILE, &imotd);
   file_to_string_alloc(HELP_PAGE_FILE, &help);
   file_to_string_alloc(INFO_FILE, &info);
   file_to_string_alloc(WIZLIST_FILE, &wizlist);
   file_to_string_alloc(IMMLIST_FILE, &immlist);
   file_to_string_alloc(POLICIES_FILE, &policies);
   file_to_string_alloc(BUILDINGH_FILE, &building);
   file_to_string_alloc(HANDBOOK_FILE, &handbook);
   file_to_string_alloc(BACKGROUND_FILE, &background);
   if ((sv = stat(IMOTD_FILE,&motd_stat)) >= 0)
       imotd_time = motd_stat.st_mtime;
   else
       imotd_time = 0;
   if ((sv = stat(MOTD_FILE,&motd_stat))>= 0)
       motd_time = motd_stat.st_mtime;
   else
       motd_time = 0;
   logg("Opening help file.");
   if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
      logg("   Could not open help file.");
   else
      help_index = build_help_index(help_fl, &top_of_helpt);

   logg("loading object limit data.");
   load_obj_limits_data();
   
   logg("Loading zone table.");
   index_boot(DB_BOOT_ZON);

   logg("Loading rooms.");
   index_boot(DB_BOOT_WLD);

/*   logg("Renumbering rooms.");
   renum_world(); */

   logg("Checking start rooms.");
   check_start_rooms();

   logg("Loading mobs and generating index.");
   index_boot(DB_BOOT_MOB);

   logg("Loading objs and generating index.");
   index_boot(DB_BOOT_OBJ);


   logg("Renumbering object (carts etc.).");
   renum_objects();

   logg("Renumbering zone table.");
   renum_zone_table();

   logg("Building zone object table.");
   obj_zone_table();   

   logg("Generating player index.");
   build_player_index();

   if (!no_limited_check){
     logg("Checking Obj limit data.");   
     check_obj_limits_data();
   }

   logg("Loading fight messages.");
   load_messages();

   logg("Loading social messages.");
   boot_social_messages();

   if (!no_specials) {
      logg("Loading shops.");
      index_boot(DB_BOOT_SHP);
   }

   logg("Assigning function pointers:");

   if (!no_specials) {
      logg("   Shopkeepers.");
      assign_the_shopkeepers();
      logg("   Mobiles.");
      assign_mobiles();
      logg("   Objects.");
      assign_objects();
      logg("   Rooms.");
      assign_rooms();
   }

   logg("   Commands.");
   assign_command_pointers();
   logg("   Spells.");
   assign_spell_pointers();
   logg("Sorting command list.");
   sort_commands();

   logg("Booting mail system.");
   if (!scan_file()) {
      logg("    Mail boot failed -- Mail system disabled");
      no_mail = 1;
   }

   logg("Reading banned site and invalid-name list.");
   load_banned();
   Read_Invalid_List();

   if (!no_rent_check) {
      logg("Deleting timed-out crash and rent files:");
      update_obj_file();
      logg("Done.");
   }

   for (i = 0; i <= top_of_zone_table; i++) {
      sprintf(buf2, "Resetting %s (rooms %d-%d).",
          zone_table[i].name,
          (i ? (zone_table[i - 1].top + 1) : 0),
          zone_table[i].top);
      logg(buf2);
      reset_zone(i);
   }

   reset_q.head = reset_q.tail = 0;

   boot_time = time(0);

   logg("Boot db -- DONE.");

}


/* reset the time in the game from file */
void	reset_time(void)
{
   long	beginning_of_time = 650336715;

   struct time_info_data mud_time_passed(time_t t2, time_t t1);

   time_info = mud_time_passed(time(0), beginning_of_time);


   switch (time_info.hours) {
   case 0 :
     weather_info.moonlight = MOON_LIGHT;      
   case 1 :
   case 2 :
   case 3 :
   case 4 :
      weather_info.sunlight = SUN_DARK;
      break;
   case 5 :
      weather_info.sunlight = SUN_RISE;
      break;
   case 6 :
   case 7 :
   case 8 :
     weather_info.moonlight = MOON_SET;
   case 9 :
   case 10 :
     weather_info.moonlight = MOON_DARK;
   case 11 :
   case 12 :
   case 13 :
   case 14 :
   case 15 :
   case 16 :
   case 17 :
   case 18 :
   case 19 :
   case 20 :
      weather_info.sunlight = SUN_LIGHT;
      break;
   case 21 :
     weather_info.sunlight = SUN_SET;
     break;
   case 22 :
   case 23 :
      weather_info.moonlight = MOON_RISE;
      break;
   default :
      weather_info.sunlight = SUN_DARK;
      break;
   }
   weather_info.moon_phase = number(0,8);
   
   sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.",
       time_info.hours, time_info.day,
       time_info.month, time_info.year);
   logg(buf);

   weather_info.pressure = 960;
   if ((time_info.month >= 7) && (time_info.month <= 12))
      weather_info.pressure += dice(1, 50);
   else
      weather_info.pressure += dice(1, 80);

   weather_info.change = 0;

   if (weather_info.pressure <= 980)
      weather_info.sky = SKY_LIGHTNING;
   else if (weather_info.pressure <= 1000)
      weather_info.sky = SKY_RAINING;
   else if (weather_info.pressure <= 1020)
      weather_info.sky = SKY_CLOUDY;
   else
      weather_info.sky = SKY_CLOUDLESS;
}




/* generate index table for the player file */
void	build_player_index(void)
{
   int	nr = -1, i;
   long	size, recs;
   struct char_file_u dummy;
   /* change to PLAYER_FILE2 to use converted playerfile */
   /* PLAYER_FILE to use normal file */
   if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
      perror("Error opening playerfile");
      exit(1);
   }

   fseek(player_fl, 0L, SEEK_END);
   size = ftell(player_fl);
   rewind(player_fl);
   if (size % sizeof(struct char_file_u))
      fprintf(stderr, "WARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
   recs = size / sizeof(struct char_file_u);
   if (recs) {
      sprintf(buf, "   %ld players in database.", recs);
      logg(buf);
      CREATE(player_table, struct player_index_element, recs);
   } else {
      player_table = 0;
      top_of_p_file = top_of_p_table = -1;
      return;
   }

   for (; !feof(player_fl); ) {
      fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
      if (!feof(player_fl))   /* new record */ {
	 nr++;
	 CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
	 for (i = 0;
	      (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++)
	    ;
	 player_table[nr].id_num = dummy.specials2.idnum;
	 player_table[nr].playing = FALSE;
	 top_idnum = MAX(top_idnum, dummy.specials2.idnum);
      }
   }

   top_of_p_file = top_of_p_table = nr;
}

/* function to count how many hash-mark delimited records exist in a file */
int	count_hash_records(FILE *fl)
{
   char	buf[120];
   int	count = 0;

   while (fgets(buf, 120, fl))
      if (*buf == '#')
	 count++;

   return (count - 1);
}

void index_boot(int mode)
{
   char	*index_filename, *prefix;
   FILE * index, *db_file;
   int	rec_count = 0;

   switch (mode) {
   case DB_BOOT_WLD	: prefix = WLD_PREFIX; break;
   case DB_BOOT_MOB	: prefix = MOB_PREFIX; break;
   case DB_BOOT_OBJ	: prefix = OBJ_PREFIX; break;
   case DB_BOOT_ZON	: prefix = ZON_PREFIX; break;
   case DB_BOOT_SHP	: prefix = SHP_PREFIX; break;
   default:
      logg("SYSERR: Unknown subcommand to index_boot!");
      exit(1);
      break;
   }

   if (mini_mud)
      index_filename = MINDEX_FILE;
   else
      index_filename = INDEX_FILE;

   sprintf(buf2, "%s/%s", prefix, index_filename);

   if (!(index = fopen(buf2, "r"))) {
      sprintf(buf1, "Error opening index file '%s'", buf2);
      perror(buf1);
      exit(1);
   }

   /* first, count the number of records in the file so we can malloc */
   if (mode != DB_BOOT_SHP) {
      fscanf(index, "%s\n", buf1);
      while (*buf1 != '$') {
	 sprintf(buf2, "%s/%s", prefix, buf1);
	 if (!(db_file = fopen(buf2, "r"))) {
	    perror(buf2);
	    exit(1);
	 } else {
	    if (mode == DB_BOOT_ZON)
	       rec_count++;
	    else
	       rec_count += count_hash_records(db_file);
	 }
	 fclose(db_file);
	 fscanf(index, "%s\n", buf1);
      }

      if (!rec_count) {
	 logg("SYSERR: boot error - 0 records counted");
	 exit(1);
      }

      rec_count++;

      switch (mode) {
      case DB_BOOT_WLD :
	 CREATE(world, struct room_data, rec_count);
	 break;
      case DB_BOOT_MOB :
	 CREATE(mob_proto, struct char_data, rec_count);
	 CREATE(mob_index, struct index_data, rec_count);
	 break;
      case DB_BOOT_OBJ :
	 CREATE(obj_proto, struct obj_data, rec_count);
	 CREATE(obj_index, struct index_data, rec_count);
	 break;
      case DB_BOOT_ZON :
	 CREATE(zone_table, struct zone_data, rec_count);
	 break;
      }
   }

   rewind(index);
   fscanf(index, "%s\n", buf1);
   while (*buf1 != '$') {
      sprintf(buf2, "%s/%s", prefix, buf1);
      if (!(db_file = fopen(buf2, "r"))) {
	 perror(buf2);
	 exit(1);
      }
      /*    printf("%s\n", buf1); */
      switch (mode) {
      case DB_BOOT_WLD	: load_rooms(db_file); break;
      case DB_BOOT_OBJ	: load_objects(db_file); break;
      case DB_BOOT_MOB	: load_mobiles(db_file); break;
      case DB_BOOT_ZON	: load_zones(db_file); break;
      case DB_BOOT_SHP	: boot_the_shops(db_file, buf2); break;
      }

      fclose(db_file);
      fscanf(index, "%s\n", buf1);
   }
}



void	check_start_rooms(void)
{
   extern short mortal_start_room;
   extern short immort_start_room;
   extern short frozen_start_room;

   if ((r_mortal_start_room = real_room(mortal_start_room)) < 0) {
      logg("SYSERR:  Mortal start room does not exist.  Change in config.c.");
      exit(1);
   }

   if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
      if (!mini_mud)
         logg("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
      r_immort_start_room = r_mortal_start_room;
   }

   if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
      if (!mini_mud)
         logg("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
      r_frozen_start_room = r_mortal_start_room;
   }
}



void	renum_world(void)
{
   register int	room, door;

   for (room = 0; room <= top_of_world; room++)
       {
	   for (door = 0; door <= 5; door++)
	       if (world[room].dir_option[door])
		   if (world[room].dir_option[door]->to_room != NOWHERE)
		       world[room].dir_option[door]->to_room = 
			   real_room(world[room].dir_option[door]->to_room);
	   if (world[room].tele_delay > -1)
	       world[room].tele_to_room = real_room(world[room].tele_to_room);
       }
}
void	renum_objects(void)
{
/*   register int obj;

   for (obj = 0; obj <= top_of_objt; obj++)
       {
	   if (GET_ITEM_TYPE(obj_proto + obj) == ITEM_CONTAINER &&
	       (obj_proto + obj)->obj_flags.value[3] > 0)
	       obj_proto[obj].obj_flags.value[3]
		   = real_room(obj_proto[obj].obj_flags.value[3]);
       } */
}


void	renum_zone_table(void)
{
   int	zone, comm, a, b;

   for (zone = 0; zone <= top_of_zone_table; zone++)
      for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++) {
	 a = b = 0;
	 zone_table[zone].cmd[comm].twinkie = FALSE;
	 switch (zone_table[zone].cmd[comm].command) {
	 case 'M':
	 case 'm':	   
	    a = zone_table[zone].cmd[comm].arg1 = 
	        real_mobile(zone_table[zone].cmd[comm].arg1);
	    b = zone_table[zone].cmd[comm].arg3 = 
	        real_room(zone_table[zone].cmd[comm].arg3);
	    break;
	 case 'R':
	 case 'r':	    	   
	 case 'F':
	 case 'f':
	 case 'C':	   
	 case 'c':	   
	    a = zone_table[zone].cmd[comm].arg1 = 
	        real_mobile(zone_table[zone].cmd[comm].arg1);
	    break;
	 case 'O':
	 case 'o':
	    a = zone_table[zone].cmd[comm].arg1 = 
	        real_object(zone_table[zone].cmd[comm].arg1);
	    if (zone_table[zone].cmd[comm].arg3 != NOWHERE)
	       b = zone_table[zone].cmd[comm].arg3 = 
	           real_room(zone_table[zone].cmd[comm].arg3);
	    break;
	 case 'G':
	 case 'g':
	    a = zone_table[zone].cmd[comm].arg1 = 
	        real_object(zone_table[zone].cmd[comm].arg1);
	    break;
	 case 'E':
	 case 'e':
	     a = zone_table[zone].cmd[comm].arg1 = 
	     real_object(zone_table[zone].cmd[comm].arg1);
	    break;
	 case 'H':
	 case 'h':
	     a = zone_table[zone].cmd[comm].arg1 =
	     real_mobile(zone_table[zone].cmd[comm].arg1);
	    break;
	 case 'P':
	 case 'p':
	    a = zone_table[zone].cmd[comm].arg1 = 
	        real_object(zone_table[zone].cmd[comm].arg1);
	    b = zone_table[zone].cmd[comm].arg3 = 
	        real_object(zone_table[zone].cmd[comm].arg3);
	    break;
	 case 'D':
	 case 'd':
	    a = zone_table[zone].cmd[comm].arg1 = 
	        real_room(zone_table[zone].cmd[comm].arg1);
	    break;
	 }
	 if (a < 0 || b < 0) {
	    if (!mini_mud)
	      fprintf(stderr,"Invalid vnum in zone reset cmd: zone #%d, cmd %d .. command disabled.\n",
		      zone_table[zone].number, comm + 1);
	    zone_table[zone].cmd[comm].command = '*'; 
	 }
      }
}
void obj_zone_table(void)
{
  int zone, min_onum, max_onum, i;
  struct zone_obj_list *zol, *tmp_zol, *tmp_zol1, *tmp_zol2;

  for (zone = 0; zone <= top_of_zone_table; zone++)
    {
      min_onum = zone_table[zone].number*100;
      max_onum = zone_table[zone].top;
      for (i=0;i <= top_of_objt;i++)
	{
	  if ((obj_index[i].virtual >= min_onum)
	      && (obj_index[i].virtual <= max_onum)) {
	    CREATE(zol, struct zone_obj_list, 1);
	    zol->vnum = obj_index[i].virtual;
	    zol->obj = &obj_proto[i];
	    if (!zone_table[zone].list) {
	      zone_table[zone].list = zol;
	      continue;
	    }
	    if (zol->vnum < zone_table[zone].list->vnum) {
	      tmp_zol = zone_table[zone].list;
	      zone_table[zone].list = zol;
	      zol->next = tmp_zol;
	      continue;
	    }
	    else 
	      for (tmp_zol = zone_table[zone].list
		     ;tmp_zol;tmp_zol = tmp_zol->next)
		{
		  if (tmp_zol->next) {
		    if (zol->vnum < tmp_zol->next->vnum)
		      {
			tmp_zol1 = tmp_zol->next;
			tmp_zol->next = zol;
			zol->next = tmp_zol1;
			break;
		      }
		  }
		  else {
		    tmp_zol->next = zol;
		    break;
		  }
		}
	  }
	}
    }
}
/* load the zone table and command tables */
void	load_zones(FILE *fl)
{
   static int	zon = 0;
   int	cmd_no = 0, expand, tmp;
   char	*check, buf[81], command;
   bool new=FALSE;

   for (; ; ) {
      fscanf(fl, " #%d\n", &tmp);
      sprintf(buf2, "beginning of zone #%d", tmp);
      check = fread_string(fl, buf2);

      if (*check == '$')
	 break;		/* end of file */

      zone_table[zon].number = tmp;
      zone_table[zon].name = check;
      fscanf(fl, " %d ", &zone_table[zon].top);
      fscanf(fl, " %d ", &zone_table[zon].lifespan);
      fscanf(fl, " %d ", &zone_table[zon].reset_mode);
      fscanf(fl, " %d ", (char *) &zone_table[zon].open);
      /* read the command table */

      cmd_no = 0;

      for (expand = 1; ; ) {
	 if (expand)
	    if (!cmd_no)
	       CREATE(zone_table[zon].cmd, struct reset_com, 1);
	    else if (!(zone_table[zon].cmd = 
	        (struct reset_com *) realloc(zone_table[zon].cmd, 
	        (cmd_no + 1) * sizeof(struct reset_com)))) {
	       perror("reset command load");
	       exit(0);
	    }

	 expand = 1;

	 fscanf(fl, " "); /* skip blanks */
	 fscanf(fl, "%c", &command);
	 zone_table[zon].cmd[cmd_no].command = command;

	 if (zone_table[zon].cmd[cmd_no].command == 'S')
	    break;

	 if (zone_table[zon].cmd[cmd_no].command == '*') {
	    expand = 0;
	    fgets(buf, 80, fl); /* skip command */
	    continue;
	 }

	 fscanf(fl, " %d %d %d", 
	     &tmp,
	     &zone_table[zon].cmd[cmd_no].arg1,
	     &zone_table[zon].cmd[cmd_no].arg2);
	 
	 if (command >= 'A' && command <= 'Z'){ /* old flags */
	   
	   zone_table[zon].cmd[cmd_no].if_flag = tmp;
	   zone_table[zon].cmd[cmd_no].num = 0;	   
	   if (command == 'M' ||  command == 'O' ||   command == 'E' ||
	       command == 'P' ||  command == 'D')
	     fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
	   if (command != 'D')
	     zone_table[zon].cmd[cmd_no].arg2 = 1;
	   zone_table[zon].cmd[cmd_no].percent = 100;
	   if (zone_table[zon].cmd[cmd_no].command == 'M')
	     zone_table[zon].cmd[cmd_no].command = 'm';
	   else if (zone_table[zon].cmd[cmd_no].command == 'O')
	     zone_table[zon].cmd[cmd_no].command = 'o';
	   else if (zone_table[zon].cmd[cmd_no].command == 'P')
	     zone_table[zon].cmd[cmd_no].command = 'p';
	   else if (zone_table[zon].cmd[cmd_no].command == 'E')
	     zone_table[zon].cmd[cmd_no].command = 'e';
	   else if (zone_table[zon].cmd[cmd_no].command == 'D')
	     zone_table[zon].cmd[cmd_no].command = 'd';
	   else if (zone_table[zon].cmd[cmd_no].command == 'G')
	     zone_table[zon].cmd[cmd_no].command = 'g';
	   else if (zone_table[zon].cmd[cmd_no].command == 'R')
	     zone_table[zon].cmd[cmd_no].command = 'r';
	   else if (zone_table[zon].cmd[cmd_no].command == 'F')
	     zone_table[zon].cmd[cmd_no].command = 'f';
	   else if (zone_table[zon].cmd[cmd_no].command == 'C')
	     zone_table[zon].cmd[cmd_no].command = 'c';
	   else if (zone_table[zon].cmd[cmd_no].command == 'H')
	     zone_table[zon].cmd[cmd_no].command = 'h';
	 }
	 else{
	   zone_table[zon].cmd[cmd_no].if_flag = tmp;
	   zone_table[zon].cmd[cmd_no].num = 0;	   
	   if (command == 'm' || command == 'o' || command == 'e' ||
	       command == 'p' || command == 'd')
	     fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
	   fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].percent);
	 }
	 fgets(buf, 80, fl);	/* read comment */
	 
	 cmd_no++;
      }
      zon++;
   }
   top_of_zone_table = zon - 1;
   free(check);
}



/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */



int	vnum_mobile(char *searchname, struct char_data *ch)
{
   int	nr, found = 0;
   
   half_chop( searchname, buf, buf2);
   
   if (!*buf)
     return(0);

   for (nr = 0; nr <= top_of_mobt; nr++) {
     
     if (isname(buf, mob_proto[nr].player.name)) 
       if (!*buf2 || isname( buf2, mob_proto[nr].player.name)) {
	 sprintf(buf1, "%3d. [%5d] %-65.65s\r\n", ++found,
		 mob_index[nr].virtual,
		 mob_proto[nr].player.short_descr);
	 send_to_char(buf1, ch);
       }
   }
   return(found);
}



int	vnum_object(char *searchname, struct char_data *ch)
{
  int	nr, found = 0;
  half_chop( searchname, buf, buf2);
  
  if (!*buf)
    return(0);
  
  for (nr = 0; nr <= top_of_objt; nr++) {
    if (obj_proto[nr].name && isname(buf, obj_proto[nr].name))
      if (!*buf2 || isname( buf2, obj_proto[nr].name)) {
	sprintf(buf1, "%3d. [%5d] %-65.65s\r\n", ++found,
		obj_index[nr].virtual,
		obj_proto[nr].short_description);
	send_to_char(buf1, ch);
      }
  }
  return(found);
}

int	vnum_zone(char *searchname, struct char_data *ch)
{
  int zone, cmd_no, found=0, last_cmd = -1;
  char bufa[40],bufb[255];
  
  half_chop( searchname, buf, buf2);
  *buf1 = '\0';  
  if (!*buf)
    return(0);
  for (zone = 0; zone_table[zone].number != ch->specials2.edit_zone  &&
	 zone <= top_of_zone_table; zone++)
    if (zone > top_of_zone_table)
      return(0);
  for (cmd_no=0; ZCMD.command != 'S';cmd_no++){
    *bufb = '\0';  
    switch(ZCMD.command){
    case 'm':
      sprintf(bufa,"%sM#%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, mob_proto[ZCMD.arg1].player.name))
	if (!*buf2 || isname(buf2, mob_proto[ZCMD.arg1].player.name))
	  sprintf(bufb,"Mob: %-25.25s in %d\r\n"
		  ,((mob_proto[ZCMD.arg1].player.short_descr) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		  ,world[ZCMD.arg3].number );
      found = 1;
      break;
    case 'f':
      for (last_cmd = cmd_no; last_cmd; last_cmd--)
	if (zone_table[zone].cmd[last_cmd].command == 'm')
	  break;
      sprintf(bufa,"%sM#%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, mob_proto[ZCMD.arg1].player.name))
	if (!*buf2 || isname(buf2, mob_proto[ZCMD.arg1].player.name))
	  sprintf(bufb,"Fol: %-25.25s in %d\r\n"
		  ,((mob_proto[ZCMD.arg1].player.short_descr) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		  ,world[zone_table[zone].cmd[last_cmd].arg3].number );
      found = 1;      
      break;
    case 'r':
      for (last_cmd = cmd_no; last_cmd; last_cmd--)
	if (zone_table[zone].cmd[last_cmd].command == 'm')
	  break;
      sprintf(bufa,"%sM#%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, mob_proto[ZCMD.arg1].player.name))
	if (!*buf2 || isname(buf2, mob_proto[ZCMD.arg1].player.name))
	  sprintf(bufb,"Rid: %-25.25s in %d\r\n"
		  ,((mob_proto[ZCMD.arg1].player.short_descr) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		  ,world[zone_table[zone].cmd[last_cmd].arg3].number );
      found = 1;            
      break;
    case 'c':
      for (last_cmd = cmd_no; last_cmd; last_cmd--)
	if (zone_table[zone].cmd[last_cmd].command == 'm')
	  break;
      sprintf(bufa,"%sM#%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, mob_proto[ZCMD.arg1].player.name))
	if (!*buf2 || isname(buf2, mob_proto[ZCMD.arg1].player.name))
	  sprintf(bufb,"Car: %-25.25s in %d\r\n"
		  ,((mob_proto[ZCMD.arg1].player.short_descr) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		  ,world[zone_table[zone].cmd[last_cmd].arg3].number );
      found = 1;            
      break;
    case 'h':
      for (last_cmd = cmd_no; last_cmd; last_cmd--)
	if (zone_table[zone].cmd[last_cmd].command == 'o')
	  break;
      sprintf(bufa,"%sM#%d %s",CCRED(ch,C_NRM),mob_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, mob_proto[ZCMD.arg1].player.name))
	if (!*buf2 || isname(buf2, mob_proto[ZCMD.arg1].player.name))
	  sprintf(bufb,"Crt: %-25.25s in %d\r\n"
		  ,((mob_proto[ZCMD.arg1].player.short_descr) ? mob_proto[ZCMD.arg1].player.short_descr : bufa)
		  ,world[zone_table[zone].cmd[last_cmd].arg3].number );
      found = 1;            
      break;      
    case 'o':
      sprintf(bufa,"%sO#%d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, obj_proto[ZCMD.arg1].name))
	if (!*buf2 || isname(buf2, obj_proto[ZCMD.arg1].name))
	  sprintf(bufb,"Obj: %-25.25s in %d\r\n"
		  ,((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		  ,world[ZCMD.arg3].number );
      last_cmd = cmd_no;      
      found = 1;            
      break;
    case 'e':
      for (last_cmd = cmd_no; last_cmd; last_cmd--)
	if (zone_table[zone].cmd[last_cmd].command == 'm')
	  break;
      sprintf(bufa,"%sO#%d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, obj_proto[ZCMD.arg1].name))
	if (!*buf2 || isname(buf2, obj_proto[ZCMD.arg1].name))
	  sprintf(bufb,"Equ: %-25.25s in %d\r\n"
		  ,((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		  ,world[zone_table[zone].cmd[last_cmd].arg3].number );
      break;
    case 'p':
      for (last_cmd = cmd_no; last_cmd; last_cmd--)
	if (zone_table[zone].cmd[last_cmd].command == 'o')
	  break;
      sprintf(bufa,"%sO#%d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, obj_proto[ZCMD.arg1].name))
	if (!*buf2 || isname(buf2, obj_proto[ZCMD.arg1].name))
	  sprintf(bufb,"Put: %-25.25s in %d\r\n"
		  ,((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		  ,world[zone_table[zone].cmd[last_cmd].arg3].number );
      found = 1;            
      break;
    case 'g':
      for (last_cmd = cmd_no; last_cmd; last_cmd--)
	if (zone_table[zone].cmd[last_cmd].command == 'm')
	  break;      
      sprintf(bufa,"%sO#%d %s",CCRED(ch,C_NRM),obj_index[ZCMD.arg1].virtual,
	      CCNRM(ch,C_NRM));      
      if (isname(buf, obj_proto[ZCMD.arg1].name))
	if (!*buf2 || isname(buf2, obj_proto[ZCMD.arg1].name))
	  sprintf(bufb,"Giv: %-25.25s in %d\r\n"
		  ,((obj_proto[ZCMD.arg1].short_description) ? obj_proto[ZCMD.arg1].short_description : bufa)
		  ,world[zone_table[zone].cmd[last_cmd].arg3].number );
      found = 1;            
      break;
    default:
      *bufb = '\0';
    }
    if (*bufb)
      if (strlen(bufb) + strlen(buf1) +1 < MAX_STRING_LENGTH)    
	strcat(buf1, bufb);
  }
      
  if (buf1)
    page_string(ch->desc,buf1,1);
  
  return(found);
}
/* create a new mobile from a prototype */
struct char_data *read_mobile(int nr, int type)
{
  int	i;
  struct char_data *mob;

   if (type == VIRTUAL) {
      if ((i = real_mobile(nr)) < 0) {
	 sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
	 return(0);
      }
   } else
      i = nr;

   CREATE(mob, struct char_data, 1);

   *mob = mob_proto[i];

   if (!mob->points.max_hit) {
      mob->points.max_hit = dice(mob->points.hit, mob->points.mana) + 
          mob->points.move;
   } else
      mob->points.max_hit = number(mob->points.hit, mob->points.mana);
   mob->points.max_move = 300 + GET_LEVEL(mob)*20;
   mob->points.hit = mob->points.max_hit;
   mob->points.mana = mob->points.max_mana;
   mob->points.move = mob->points.max_move;

   mob->player.time.birth = time(0);
   mob->player.time.played = 0;
   mob->player.time.logon  = time(0);
   mob->desc=0;
   mob->reset =0;
   GET_MOOD(mob) = number(-950,1000);
   if (IS_SET(mob->specials2.act , MOB_HAPPY))
       GET_MOOD(mob) = MIN(1000, GET_MOOD(mob) + 700);
   if (IS_SET(mob->specials2.act , MOB_SAD))
       GET_MOOD(mob) = MAX(-1000, GET_MOOD(mob) - 700);

   /* insert in list */
   mob->next = character_list;
   character_list = mob;

   mob_index[i].number++;
   mob->points.exp = compute_mob_exp(mob);
   if (GET_GOLD(mob)){
     GET_GOLD(mob) = number(MAX(1,compute_mob_exp(mob)/100)
			    ,MAX(1,compute_mob_exp(mob)/50));
     if (!number(0,10) && !IS_ANIMAL(mob) && (GET_RACE(mob) != CLASS_PLANT))
       GET_GOLD(mob) += number(0,MAX(1,compute_mob_exp(mob)/25));
     if (!number(0,20) && !IS_ANIMAL(mob) && (GET_RACE(mob) != CLASS_PLANT))
       GET_GOLD(mob) += number(0,MAX(1,compute_mob_exp(mob)/10));
     GET_GOLD(mob) = MAX(10,GET_GOLD(mob));
     if (mob_index[mob->nr].func == shop_keeper)
       GET_GOLD(mob) += number(1,20)*1000;
   }
   return mob;
}



/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type, int mode)
{
   struct obj_data *obj;
   int	i, zone;

   if (nr < 0) {
      logg("SYSERR: trying to create obj with negative num!");
      return 0;
   }

   if (type == VIRTUAL) {
      if ((i = real_object(nr)) < 0) {
	 sprintf(buf, "Object (V) %d does not exist in database.", nr);
	 return(0);
      }
   } else
      i = nr;

   CREATE(obj, struct obj_data, 1);
   *obj = obj_proto[i];
   obj->next = object_list;
   object_list = obj;
   obj->reset =0;
   obj_index[i].number++;
   if (mode == 0 && obj->obj_flags.value[6] >0)
	   load_limited(obj_index[i].virtual);
   if (HASROOM(obj)){
       if (!(world[real_room(obj->obj_flags.value[3])].obj))
	   world[real_room(obj->obj_flags.value[3])].obj = obj;
       else
	   obj->obj_flags.value[3] = 0;
   }
   /* temp Hack to make old LIGHT items work */
   if (obj->obj_flags.type_flag == ITEM_LIGHT){
       obj->obj_flags2.light = MAX(7,obj->obj_flags.value[2]/5);
   }
   for (zone = 0; zone <= top_of_zone_table;zone++)
     if ((obj_index[i].virtual >= zone_table[zone].number*100) &&
	 (obj_index[i].virtual <= zone_table[zone].top))
       break;
   if (!zone_table[zone].open)
     SET_BIT(obj->obj_flags.extra_flags, ITEM_TEST_ONLY);
   if (obj->obj_flags.type_flag != ITEM_WEAPON)
     GET_OBJ_WEIGHT(obj) = (GET_OBJ_WEIGHT(obj)*(9 - GET_OBJ_SIZE(obj)))/7;
   return (obj);
   
}
void    check_obj_limits_data(void)
{
  int i=0;

  for (i = 0;i <= top_of_ol_table; i++){
    obj_limit_table[i].no_stored = 0;
    obj_limit_table[i].modified = 1;
  }
  
  check_items_in_rent(obj_limit_table[i].obj_num);  
  save_all_object_limit_data();    
}
void check_items_in_rent(long vnum)
{
    int i=0,ii=0;
    struct rent_info rent;
    struct obj_file_elem tmp_p;
    struct obj_file_elem_1 tmp_1p;    
    char fname[MAX_INPUT_LENGTH];
    FILE *rf;

    for (i=0;i <= top_of_p_table;i++){
      if(!Crash_get_filename((player_table +i)->name,fname)){
	sprintf(buf,"Error getting Rentfile name for: %s",
		(player_table +i)->name);
	logg(buf);
	continue;
      }
      if (!(rf = fopen(fname,"r+b"))){
	if (errno != ENOENT) { /* if it fails, NOT because of no file */
	  sprintf(buf1, "SYSERR: READING OBJECT FILE %s", fname);
	  perror(buf1);
	  logg(buf1);
	}
	continue;
      }
      fread(&rent, sizeof(struct rent_info),1, rf);
      if (rent.version < 1) {
	logg("SYSERR: Old Rent file.");
	fclose(rf);
	continue;
      }
      else if (rent.version == 1){
	while (!feof(rf)) {
	  fseek( rf,1,1);
	  fread(&tmp_1p, sizeof(struct obj_file_elem_1),1,rf);
	  for (ii=0;ii<=top_of_ol_table;ii++)
	    if (tmp_1p.item_number == obj_limit_table[ii].obj_num){
	      obj_limit_table[ii].no_stored++;
	      continue;
	    }
	  tmp_1p.item_number = -1;
	}
	fclose(rf);      }
      else {
	while (!feof(rf)) {
	  fseek( rf,1,1);
	  fread(&tmp_p, sizeof(struct obj_file_elem),1,rf);
	  for (ii=0;ii<=top_of_ol_table;ii++)
	    if (tmp_p.item_number == obj_limit_table[ii].obj_num){
	      obj_limit_table[ii].no_stored++;
	      continue;
	    }
	  tmp_p.item_number = -1;
	}
	fclose(rf);
      }
    }
}

void	load_obj_limits_data(void)
{
   int	nr = -1;
   long	size, recs;
   struct obj_limit_type dummy;

   if (!(obj_limit_fl = fopen(OBJ_LIMITS_FILE, "r+b"))) {
      perror("Error opening object limits data file");
      exit(1);
   }

   fseek(obj_limit_fl, 0L, SEEK_END);
   size = ftell(obj_limit_fl);
   rewind(obj_limit_fl);
   if (size % sizeof(struct obj_limit_type))
      fprintf(stderr, "WARNING: OBJECT LIMIT DATAFILE IS PROBABLY CORRUPT!\n");
   recs = size / sizeof(struct obj_limit_type);
   if (recs) {
      sprintf(buf, "   %ld limited objects in database.", recs);
      logg(buf);
      CREATE(obj_limit_table, struct obj_limit_list_type, recs);
   } else {
      obj_limit_table = 0;
      top_of_ol_file = top_of_ol_table = -1;
      return;
   }

   for (; !feof(obj_limit_fl); ) {
      fread(&dummy, sizeof(struct obj_limit_type), 1, obj_limit_fl);
      if (!feof(obj_limit_fl))   /* new record */ {
	 nr++;
	 obj_limit_table[nr].obj_num = dummy.obj_num;
	 obj_limit_table[nr].no_stored = dummy.no_stored;
	 obj_limit_table[nr].no_loaded = 0;
      }
   }
   
   top_of_ol_file = top_of_ol_table = nr;
}





#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void	zone_update(void)
{
   int	i;
   struct reset_q_element *update_u, *temp;
   static int timer = 0;
   char buf[128];

   /* jelson 10/22/92 */
   if (((++timer * PULSE_ZONE) / 4) >= 60) { /* 4 comes from 4 passes/sec */
      /* one minute has passed */
      /* NOT accurate unless PULSE_ZONE is a multiple of 4 or a factor of 60 */

      timer = 0;

      /* since one minute has passed, increment zone ages */
      for (i = 0; i <= top_of_zone_table; i++) {
         if (zone_table[i].age < zone_table[i].lifespan &&
	     zone_table[i].reset_mode)
	       (zone_table[i].age)++;

         if (zone_table[i].age >= zone_table[i].lifespan &&
	     zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
                /* enqueue zone */
 
	        CREATE(update_u, struct reset_q_element, 1);

		update_u->zone_to_reset = i;
		update_u->next = 0;

		if (!reset_q.head)
		   reset_q.head = reset_q.tail = update_u;
		else {
		   reset_q.tail->next = update_u;
		   reset_q.tail = update_u;
		}

		zone_table[i].age = ZO_DEAD;
	 }
      }
   }

   /* dequeue zones (if possible) and reset */

   for (update_u = reset_q.head; update_u; update_u = update_u->next)
      if (zone_table[update_u->zone_to_reset].reset_mode == 2 || 
          is_empty(update_u->zone_to_reset)) {
	 reset_zone(update_u->zone_to_reset);
	 sprintf(buf, "Auto zone reset: (%3d) %s",
		 zone_table[update_u->zone_to_reset].number,
		 zone_table[update_u->zone_to_reset].name);
	 mudlog(buf, CMP, LEVEL_GOD, FALSE);
	 /* dequeue */
	 if (update_u == reset_q.head)
	    reset_q.head = reset_q.head->next;
	 else {
	    for (temp = reset_q.head; temp->next != update_u;
		 temp = temp->next) ;

	    if (!update_u->next)
	       reset_q.tail = temp;

	    temp->next = update_u->next;
	 }

	 free(update_u);
	 break;
      }
}

bool twinkie_mob(struct char_data *mob)
{
  int i,total=0, max = 0, number = 0;

  for (i=0; i< MAX_WEAR;i++)
    if (mob->equipment[i])
      total += total_cp(mob->equipment[i], &max, &number);
  total += total_cp(mob->inventory, &max, &number);
  if (number)
    total /= number;
  max = MAX(max, total);
  
  if (GET_EXP(mob) < 10000){
    if (max > 30)
      return(TRUE);
  }
  else if (GET_EXP(mob) < 25000){
      if (max > 40)
	return(TRUE);
  }
  else if (GET_EXP(mob) < 50000){
    if (max > 60)
      return(TRUE);
  }
  else {
    if (max*max*10 > GET_EXP(mob))
      return(TRUE);
  }
  return(FALSE);

}
void extract_twinkie(struct char_data *mob)
{
  int i;
  struct obj_data *tmp;
  
  for (i=0; i< MAX_WEAR;i++)
    if (mob->equipment[i])
      obj_to_char(unequip_char(mob,i), mob, 0);
  remove_items(&mob->inventory);
  extract_char(mob, 0);

}
void remove_items(struct obj_data **obj)
{
  if (*obj){
    remove_items(&(*obj)->next_content);
    obj_from_char(*obj, 0);
    extract_obj(*obj, 0);
  }
}
int total_cp(struct obj_data *obj, int *max, int *number)
{
  int total = 0, num = 0;
  
  if (obj){
    total += total_cp(obj->contains, max, number);
    total += total_cp(obj->next_content, max, number);
    num = assess_item(obj);
    if (num >= *max)
      *max = num;
    total += num;
    *number += 1;
  }
  return(total);

}


/* execute the reset command table of a given zone */
void	reset_zone(int zone)
{
  int	cmd_no, last_cmd = 1, last_mobile= -1;
  char	buf[256], command;
  struct char_data *mob=0, *mob2=0;
  struct obj_data *obj=0, *obj_to=0;

  for (cmd_no = 0; ; cmd_no++) {
    if (ZCMD.command == 'S')
      break;
    if (last_cmd || !ZCMD.if_flag)
      switch (ZCMD.command) {
      case '*': /* ignore command */
	break;
      case 'm': /* read a mobile */
	if (ZCMD.arg3 >= 0) {	
	  if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	    ZCMD.num++;
	    mob = read_mobile(ZCMD.arg1, REAL);
	    mob->reset = &ZCMD;
	    char_to_room(mob, ZCMD.arg3, FALSE);
	    if (ZCMD.arg1 == real_mobile(2059))
	      martha = mob;
	    last_cmd = 1;
	    last_mobile = cmd_no;
	  }
	  else
	    last_cmd = 0;
	}
	else
	  last_cmd = 0;
	break;
      case 'o': /* read an object */
	/* if (!get_obj_in_list_num(ZCMD.arg1, world[ZCMD.arg3].contents)) { */
	last_mobile = -1;
	if (ZCMD.arg3 >= 0) {
	  if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	    ZCMD.num++;
	    obj = read_object(ZCMD.arg1, REAL,0);
	    obj->reset = &ZCMD;
	    if (ZCMD.arg1 == real_object(2001))
	      don_box = obj;
	    if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)) {
	      extract_obj(obj,0);
	      last_cmd = 0;
	      break;
	    }
	    else {
	      obj_to_room(obj, ZCMD.arg3, FALSE);
	      last_cmd = 1;
	    }
	  }
	  else
	    last_cmd = 0;
	}
	else {
	  last_cmd = 0; /* no valid - room - don't load the obj */
	}
	break;
      case 'p': /* object to object */
	if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	  ZCMD.num++;
	  obj = read_object(ZCMD.arg1, REAL,0);
	  obj->reset = &ZCMD;
	  if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)){
	    extract_obj(obj,0);
	    last_cmd = 0;
	    break;
	  }
	  if (!(obj_to = get_obj_num(ZCMD.arg3))) {
	    
	    logg("SYSERR: error in zone file: target obj not found.");
	    sprintf(buf, "SYSERR:   Offending cmd: \"P %d %d %d\" in zone #%d (cmd %d)",
		    obj_index[ZCMD.arg1].virtual, ZCMD.arg2,
		    obj_index[ZCMD.arg3].virtual,
		    zone_table[zone].number, cmd_no);
	    mudlog(buf, NRM, LEVEL_BUILDER, TRUE);
	    extract_obj(obj,0);
	    last_cmd = 0;
	    break;
	  }
	  obj_to_obj(obj, obj_to);
	  last_cmd = 1;
	}
	else
	  last_cmd = 0;
	break;
      case 'g': /* obj_to_char */
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt to give obj to non-existant mob.");
	  sprintf(buf, "SYSERR:   Offending cmd: \"G %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	  ZCMD.num++;
	  obj = read_object(ZCMD.arg1, REAL,0);
	  obj->reset = &ZCMD;
	  if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)) {
	    extract_obj(obj,0);
	    last_cmd = 0;
	  }
	  else {
	    obj_to_char(obj, mob, 1);
	    last_cmd = 1;
	  }
	} else
	  last_cmd = 0;
	break;
      case 'f': /*  follow */
	while (mob->master){
	  if (IS_MOB(mob->master))
	    mob = mob->master;
	  else
	    break;}
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt add follower to non-existant mob.");
	  sprintf(buf, "SYSERR:   Offending cmd: \"H %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);
	  last_cmd = 0;
	  break;
	}
	if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	  ZCMD.num++;
	  mob2 = read_mobile(ZCMD.arg1, REAL);
	  mob2->reset = &ZCMD;
	  char_to_room(mob2, mob->in_room, FALSE);
	  if (!circle_follow(mob2,mob))
	    add_follower(mob2,mob);
	  last_cmd = 1;
	  last_mobile = cmd_no;		  
	  mob = mob2;
	} else
	  last_cmd = 0;
	break;
      case 'r': /*  follow  and make mount*/
	while (mob->master){
	  if (IS_MOB(mob->master))
	    mob = mob->master;
	  else
	    break;}
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt add follower to non-existant mob.");
	  sprintf(buf,"SYSERR:Offending cmd: \"R %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	  ZCMD.num++;
	  mob2 = read_mobile(ZCMD.arg1, REAL);
	  mob2->reset = &ZCMD;
	  char_to_room(mob2, mob->in_room, FALSE);
	  if (!circle_follow(mob2,mob))
	    add_follower(mob2,mob);
	  mob->specials.mount = mob2;
	  mob2->specials.rider = mob;
	  last_cmd = 1;
	  mob = mob2;
	} else
	  last_cmd = 0;
	break;
      case 'C': /*  follow  and make carried*/
	while (mob->master){
	  if (IS_MOB(mob->master))
	    mob = mob->master;
	  else
	    break;}
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt add follower to non-existant mob.");
	  sprintf(buf,"SYSERR:Offending cmd: \"C %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	  ZCMD.num++;
	  mob2 = read_mobile(ZCMD.arg1, REAL);
	  mob2->reset = &ZCMD;
	  char_to_room(mob2, mob->in_room, FALSE);
	  if (!circle_follow(mob2,mob))
	    add_follower(mob2,mob);
	  if (mob->equipment[WIELD]){
	    obj = unequip_char(mob,WIELD);
	    extract_obj(obj,0);
	  }
	  if (mob->equipment[HOLD]){
	    obj = unequip_char(mob,HOLD);
	    extract_obj(obj,0);
	  }
	  mob->specials.carrying = mob2;
	  mob2->specials.carried_by = mob;
	  last_cmd = 1;
	  last_mobile = cmd_no;	
	  mob = mob2;
	} else
	  last_cmd = 0;
	break;
      case 'h': /*  hitch char_to_obj */
	if (!obj) {
	  logg("SYSERR: error in zone file: attempt to hitch mob to non-existant obj.");
	  sprintf(buf, "SYSERR:   Offending cmd: \"H %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	while(obj->in_obj) /* skip contents of cart */
	  obj = obj->in_obj;
	if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	  ZCMD.num++;
	  mob = read_mobile(ZCMD.arg1, REAL);
	  mob->reset = &ZCMD;
	  char_to_room(mob, obj->in_room, FALSE);
	  if (last_cmd)
	    hitch(mob,obj,mob,1);
	  last_cmd = 1;
	  last_mobile = cmd_no;
	} else
	  last_cmd = 0;
	break;
      case 'e': /* object to equipment list */
	if (!mob) {
	  logg("SYSERR: error in zone file: trying to equip non-existant mob");
	  sprintf(buf, "SYSERR:   Offending cmd: \"E %d %d %d\" in zone #%d (cmd %d)",
		  obj_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  ZCMD.arg3, zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if ((ZCMD.num < ZCMD.arg2) && (number(1,100) < ZCMD.percent)) {
	  if (ZCMD.arg3 < 0 || ZCMD.arg3 >= MAX_WEAR) {
	    logg("SYSERR: error in zone file: invalid equipment pos number");
	    sprintf(buf, "SYSERR:   Offending cmd: \"E %d %d %d\" in zone #%d (cmd %d)",
		    obj_index[ZCMD.arg1].virtual, ZCMD.arg2,
		    ZCMD.arg3, zone_table[zone].number, cmd_no);
	    mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	    
	    last_cmd = 0;
	  } else {
	    ZCMD.num++;
	    obj = read_object(ZCMD.arg1, REAL,0);
	    obj->reset = &ZCMD;
	    if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)) {
	      extract_obj(obj,0);
	      last_cmd = 0;
	    }
	    else {
	      equip_char(mob, obj, ZCMD.arg3);
	      last_cmd = 1;
	    }
	  }
	} else{
	  last_cmd = 0;}
	break;
      case 'd': /* set state of door */
	last_mobile = -1;
	if (number(1,100) > ZCMD.percent)
	  break;
	if (!world[ZCMD.arg1].dir_option[ZCMD.arg2]){
	  logg("SYSERR: Error in zone file: Bogus exit on door reset.");
	  sprintf(buf,"SYSERR:   Offending Cmd: \"D %d %d %d %d\" in zone #%d (cmd %d)"
		  ,ZCMD.if_flag,world[ZCMD.arg1].number, ZCMD.arg2
		  , ZCMD.arg3,zone_table[zone].number,cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  break;}
		 
	switch (ZCMD.arg3) {
	case 0:
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_CLOSED);
	  break;
	case 1:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  break;
	case 2:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_LOCKED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  break;
	case 3:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_SECRET);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  break;
	case 4:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_SECRET);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_LOCKED);
	  break;
	}
	last_cmd = 1;
	break;
      case 'M': /* read a mobile */
	if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	  mob = read_mobile(ZCMD.arg1, REAL);
	  char_to_room(mob, ZCMD.arg3, FALSE);
	  if (ZCMD.arg1 == real_mobile(2059))
	    martha = mob;
	  last_cmd = 1;
	  last_mobile = cmd_no;
	} else
	  last_cmd = 0;
	break;	
      case 'O': /* read an object */
	/* if (!get_obj_in_list_num(ZCMD.arg1, world[ZCMD.arg3].contents)) { */
	last_mobile = -1;
	if (ZCMD.arg3 >= 0) {
	  if (obj_index[ZCMD.arg1].number < ZCMD.arg2){
	    obj = read_object(ZCMD.arg1, REAL,0);
	    if (ZCMD.arg1 == real_object(2001))
	      don_box = obj;
	    if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)) {
	      extract_obj(obj,0);
	      last_cmd = 0;
	    }
	    else {
	      obj_to_room(obj, ZCMD.arg3, FALSE);
	      last_cmd = 1;
	    }
	  }
	  else
	    last_cmd = 0;
	}
	else {
	  last_cmd = 0; /* no valid - room - don't load the obj */
	}
	break;
      case 'P': /* object to object */
	if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	  obj = read_object(ZCMD.arg1, REAL,0);
	  if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)){
	    extract_obj(obj,0);
	    last_cmd = 0;
	    break;
	  }
	  if (!(obj_to = get_obj_num(ZCMD.arg3))) {
	    
	    logg("SYSERR: error in zone file: target obj not found.");
	    sprintf(buf, "SYSERR:   Offending cmd: \"P %d %d %d\" in zone #%d (cmd %d)",
		    obj_index[ZCMD.arg1].virtual, ZCMD.arg2,
		    obj_index[ZCMD.arg3].virtual,
		    zone_table[zone].number, cmd_no);
	    mudlog(buf, NRM, LEVEL_BUILDER, TRUE);
	    extract_obj(obj,0);
	    last_cmd = 0;
	    break;
	  }
	  obj_to_obj(obj, obj_to);
	  last_cmd = 1;
	}
	else
	  last_cmd = 0;
	break;
      case 'G': /* obj_to_char */
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt to give obj to non-existant mob.");
	  sprintf(buf, "SYSERR:   Offending cmd: \"G %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	  obj = read_object(ZCMD.arg1, REAL,0);
	  if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)) {
	    extract_obj(obj,0);
	    last_cmd = 0;
	  }
	  else {
	    obj_to_char(obj, mob, 1);
	    last_cmd = 1;
	  }
	} else
	  last_cmd = 0;
	break;

      case 'F': /*  follow */
	while (mob->master){
	  if (IS_MOB(mob->master))
	    mob = mob->master;
	  else
	    break;}
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt add follower to non-existant mob.");
	  sprintf(buf, "SYSERR:   Offending cmd: \"H %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);
	  last_cmd = 0;
	  break;
	}
	if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	  mob2 = read_mobile(ZCMD.arg1, REAL);
	  char_to_room(mob2, mob->in_room, FALSE);
	  if (!circle_follow(mob2,mob))
	    add_follower(mob2,mob);
	  last_cmd = 1;
	  last_mobile = cmd_no;		  
	  mob = mob2;
	} else
	  last_cmd = 0;
	break;
      case 'R': /*  follow  and make mount*/
	while (mob->master){
	  if (IS_MOB(mob->master))
	    mob = mob->master;
	  else
	    break;}
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt add follower to non-existant mob.");
	  sprintf(buf,"SYSERR:Offending cmd: \"R %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	  mob2 = read_mobile(ZCMD.arg1, REAL);
	  char_to_room(mob2, mob->in_room, FALSE);
	  if (!circle_follow(mob2,mob))
	    add_follower(mob2,mob);
	  mob->specials.mount = mob2;
	  mob2->specials.rider = mob;
	  last_cmd = 1;
	  mob = mob2;
	} else
	  last_cmd = 0;
	break;
      case 'c': /*  follow  and make carried*/
	while (mob->master){
	  if (IS_MOB(mob->master))
	    mob = mob->master;
	  else
	    break;}
	if (!mob) {
	  logg("SYSERR: error in zone file: attempt add follower to non-existant mob.");
	  sprintf(buf,"SYSERR:Offending cmd: \"C %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	  mob2 = read_mobile(ZCMD.arg1, REAL);
	  char_to_room(mob2, mob->in_room, FALSE);
	  if (!circle_follow(mob2,mob))
	    add_follower(mob2,mob);
	  if (mob->equipment[WIELD]){
	    obj = unequip_char(mob,WIELD);
	    extract_obj(obj,0);
	  }
	  if (mob->equipment[HOLD]){
	    obj = unequip_char(mob,HOLD);
	    extract_obj(obj,0);
	  }
	  mob->specials.carrying = mob2;
	  mob2->specials.carried_by = mob;
	  last_cmd = 1;
	  last_mobile = cmd_no;	
	  mob = mob2;
	} else
	  last_cmd = 0;
	break;
      case 'H': /*  hitch char_to_obj */
	if (!obj) {
	  logg("SYSERR: error in zone file: attempt to hitch mob to non-existant obj.");
	  sprintf(buf, "SYSERR:   Offending cmd: \"H %d %d\" in zone #%d (cmd %d)",
		  mob_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	while(obj->in_obj) /* skip contents of cart */
	  obj = obj->in_obj;
	if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	  mob = read_mobile(ZCMD.arg1, REAL);
	  char_to_room(mob, obj->in_room, FALSE);
	  if (last_cmd)
	    hitch(mob,obj,mob,1);
	  last_cmd = 1;
	  last_mobile = cmd_no;
	} else
	  last_cmd = 0;
	break;
      case 'E': /* object to equipment list */
	if (!mob) {
	  logg("SYSERR: error in zone file: trying to equip non-existant mob");
	  sprintf(buf, "SYSERR:   Offending cmd: \"E %d %d %d\" in zone #%d (cmd %d)",
		  obj_index[ZCMD.arg1].virtual, ZCMD.arg2,
		  ZCMD.arg3, zone_table[zone].number, cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  last_cmd = 0;
	  break;
	}
	if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	  if (ZCMD.arg3 < 0 || ZCMD.arg3 >= MAX_WEAR) {
	    logg("SYSERR: error in zone file: invalid equipment pos number");
	    sprintf(buf, "SYSERR:   Offending cmd: \"E %d %d %d\" in zone #%d (cmd %d)",
		    obj_index[ZCMD.arg1].virtual, ZCMD.arg2,
		    ZCMD.arg3, zone_table[zone].number, cmd_no);
	    mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	    
	    last_cmd = 0;
	  } else {
	    obj = read_object(ZCMD.arg1, REAL,0);
	    if (obj->obj_flags.value[6] > 0 && is_maxed(ZCMD.arg1, REAL)) {
	      extract_obj(obj,0);
	      last_cmd = 0;
	    }
	    else {
	      equip_char(mob, obj, ZCMD.arg3);
	      last_cmd = 1;
	    }
	  }
	} else{
	  last_cmd = 0;}
	break;

      case 'D': /* set state of door */
	last_mobile = -1;	
	if (!world[ZCMD.arg1].dir_option[ZCMD.arg2]){
	  logg("SYSERR: Error in zone file: Bogus exit on door reset.");
	  sprintf(buf,"SYSERR:   Offending Cmd: \"D %d %d %d %d\" in zone #%d (cmd %d)"
		  ,ZCMD.if_flag,world[ZCMD.arg1].number, ZCMD.arg2
		  , ZCMD.arg3,zone_table[zone].number,cmd_no);
	  mudlog(buf, NRM, LEVEL_BUILDER, TRUE);	  
	  break;}
		 
	switch (ZCMD.arg3) {
	case 0:
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_CLOSED);
	  break;
	case 1:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  break;
	case 2:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_LOCKED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  break;
	case 3:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_SECRET);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  break;
	case 4:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_SECRET);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_LOCKED);
	  break;
	}
	last_cmd = 1;
	break;

      default:
	sprintf(buf, "SYSERR:  Unknown cmd in reset table; zone %d cmd %d.\r\n",
	        zone_table[zone].number, cmd_no);
	mudlog(buf, NRM, LEVEL_GRGOD, TRUE);
	break;
      }
    else
      last_cmd = 0;

  }

  zone_table[zone].age = 0;
}



/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int	is_empty(int zone_nr)
{
   struct descriptor_data *i;

   for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
	 if (world[i->character->in_room].zone == zone_nr)
	    return(0);

   return(1);
}





/*************************************************************************
*  stuff related to the save/load player system				 *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int	load_char(char *name, struct char_file_u *char_element)
{
   int	player_i;

   int	find_name(char *name);

   if ((player_i = find_name(name)) >= 0) {
      fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
      fread(char_element, sizeof(struct char_file_u), 1, player_fl);
      return(player_i);
   } else
      return(-1);
}
int	load_char_bynum(int player_i, struct char_file_u *char_element)
{

   if (player_i >= 0) {
       fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
       fread(char_element, sizeof(struct char_file_u), 1, player_fl);
       return(player_i);
   } else
       return(-1);
}




/* copy data from the file structure to a char struct */
void	store_to_char(struct char_file_u *st, struct char_data *ch)
{
    long r_time;
    int	i;

   GET_SEX(ch) = st->sex;
/* probably replace with a race ajn- skills-system */
   GET_RACE(ch) = st->race;
   GET_LEVEL(ch) = st->level;

   ch->player.short_descr = 0;
   ch->player.long_descr = 0;

   if (*st->title) {
      CREATE(ch->player.title, char, strlen(st->title) + 1);
      strcpy(ch->player.title, st->title);
   } else
      GET_TITLE(ch) = 0;
   if (*st->poofin) {
      CREATE(ch->specials.poofIn, char, strlen(st->poofin) + 1);
      strcpy(ch->specials.poofIn, st->poofin);
   } else
      ch->specials.poofIn = 0;
   if (*st->poofout) {
      CREATE(ch->specials.poofOut, char, strlen(st->poofout) + 1);
      strcpy(ch->specials.poofOut, st->poofout);
   } else
      ch->specials.poofOut = 0;      
   if (*st->prmpt) {
       CREATE(ch->player.prmpt, char,
	      strlen(st->prmpt) + 1);
       strcpy(ch->player.prmpt, st->prmpt);
   }   else
       ch->player.prmpt = 0;
   if (*st->description) {
      CREATE(ch->player.description, char, 
          strlen(st->description) + 1);
      strcpy(ch->player.description, st->description);
   } else
      ch->player.description = 0;

   ch->player.hometown = st->hometown;

   ch->player.time.birth = st->birth;
   ch->player.time.played = st->played;
   ch->player.time.logon  = time(0);

   for (i = 0; i < MAX_TOUNGE; i++)
      ch->player.talks[i] = st->talks[i];

   ch->player.weight = st->weight;
   ch->player.height = st->height;

   ch->abilities = st->abilities;
   ch->tmpabilities = st->abilities;
   ch->points = st->points;
   ch->specials2 = st->specials2;
   ch->specials2.last_tell_id = NOBODY;
   /* New dynamic skill system: only PCs have a skill array allocated. */
   CREATE(ch->skills, byte, MAX_SKILLS);
   for (i = 0; i < MAX_SKILLS; i++)
      SET_SKILL(ch, i, st->skills[i]);


   ch->specials.carry_weight = 0;
   ch->specials.carry_items  = 0;
   ch->points.hitroll        = 0;
   ch->points.damroll        = 0;
   ch->specials.hitroll        = 0;
   ch->specials.damroll        = 0;   

   CREATE(ch->player.name, char, strlen(st->name) + 1);
   strcpy(ch->player.name, st->name);

   /* Add all spell effects */
   set_race_characteristics(ch);
   for (i = 0; i < MAX_AFFECT; i++) {
      if (st->affected[i].type)
	 affect_to_char(ch, &st->affected[i]);
   }

   ch->in_room = GET_LOADROOM(ch);
   
   affect_total(ch);


   /* If you're not poisioned and you've been away for more than
      half an hour, we'll set your HMV back to full
      hmm a regen mode
      */
   time(&r_time);
   r_time -=  st->last_logon;
   r_time /= SECS_PER_REAL_MIN;

   if (!IS_AFFECTED(ch, AFF_POISON) )
     r_time /= 2;

   if (r_time < 10 )
     r_time = 0;
   else if (r_time > 60)
     r_time = 60;
   
   GET_HIT(ch) += (r_time*(GET_MAX_HIT(ch) - GET_HIT(ch)))/60;
   GET_MOVE(ch) += (r_time*(GET_MAX_MOVE(ch) - GET_MOVE(ch)))/60;
} /* store_to_char */





/* copy vital data from a players char-structure to the file structure */
void	char_to_store(struct char_data *ch, struct char_file_u *st)
{
   int	i;
   struct affected_type *af;
   struct obj_data *char_eq[MAX_WEAR];

   /* Unaffect everything a character can be affected by */

   for (i = 0; i < MAX_WEAR; i++) {
      if (ch->equipment[i])
	 char_eq[i] = unequip_char(ch, i);
      else
	 char_eq[i] = 0;
   }

   for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
      if (af) {
	 st->affected[i] = *af;
	 st->affected[i].next = 0;
	 af = af->next;
      } else {
	 st->affected[i].type = 0;  /* Zero signifies not used */
	 st->affected[i].duration = 0;
	 st->affected[i].modifier = 0;
	 st->affected[i].location = 0;
	 st->affected[i].bitvector = 0;
	 st->affected[i].next = 0;
      }
   }


   /* remove the affections so that the raw values are stored;
      otherwise the effects are doubled when the char logs back in. */

   while (ch->affected)
      affect_remove(ch, ch->affected);

   if ((i >= MAX_AFFECT) && af && af->next)
      logg("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

   ch->tmpabilities = ch->abilities;

   st->birth      = ch->player.time.birth;
   st->played     = ch->player.time.played;
   st->played    += (long) (time(0) - ch->player.time.logon);
   st->last_logon = time(0);

   ch->player.time.played = st->played;
   ch->player.time.logon = time(0);

   st->hometown = ch->player.hometown;
   st->weight   = GET_WEIGHT(ch);
   st->height   = GET_HEIGHT(ch);
   st->sex      = GET_SEX(ch);
   st->race    = GET_RACE(ch);
   st->level    = GET_LEVEL(ch);
   st->abilities = ch->abilities;
   st->points    = ch->points;
   st->specials2 = ch->specials2;

   st->points.armor[0]   = 100;
   st->points.armor[0]   = 100;
   st->points.armor[0]   = 100;
   st->points.armor[0]   = 100;
   st->points.hitroll =  0;
   st->points.damroll =  0;

   if (GET_TITLE(ch))
      strcpy(st->title, GET_TITLE(ch));
   else
      *st->title = '\0';
   if (ch->player.prmpt)
       strcpy(st->prmpt,ch->player.prmpt);
   else
       *st->prmpt = '\0';
   if (ch->specials.poofIn)
       strcpy(st->poofin,ch->specials.poofIn);
   else
       *st->poofin = '\0';
   if (ch->specials.poofOut)
       strcpy(st->poofout,ch->specials.poofOut);
   else
       *st->poofout = '\0';
   if (ch->player.description){
       strcpy(st->description, ch->player.description);}
   else
      *st->description = '\0';


   for (i = 0; i < MAX_TOUNGE; i++)
      st->talks[i] = ch->player.talks[i];

   for (i = 0; i < MAX_SKILLS; i++)
      st->skills[i] = GET_SKILL(ch, i);

   strcpy(st->name, GET_NAME(ch));

   /* add spell and eq affections back in now */
   for (i = 0; i < MAX_AFFECT; i++) {
      if (st->affected[i].type)
	 affect_to_char(ch, &st->affected[i]);
   }

   for (i = 0; i < MAX_WEAR; i++) {
      if (char_eq[i])
	 equip_char(ch, char_eq[i], i);
   }

   affect_total(ch);
} /* Char to store */

/* create a new entry in the in-memory index table for the obj  limit table */
int	create_ol_entry(int obj_no)
{

   if (top_of_ol_table == -1) {
      CREATE(obj_limit_table, struct obj_limit_list_type, 1);
      top_of_ol_table = 0;
   } else if (!(obj_limit_table = (struct obj_limit_list_type *)
       realloc(obj_limit_table, sizeof(struct obj_limit_list_type) * 
       (++top_of_ol_table + 1)))) {
      perror("create ol entry");
      exit(1);
   }

   obj_limit_table[top_of_ol_table].obj_num = obj_no;
   obj_limit_table[top_of_ol_table].no_stored = 0;
   obj_limit_table[top_of_ol_table].no_loaded = 0;
   obj_limit_table[top_of_ol_table].modified = 1;
   return (top_of_ol_table);
}


int obj_is_in_limit_table(int obj_no)
{
    int i;

    for (i=0;i<=top_of_ol_table;i++)
	    if (obj_limit_table[i].obj_num == obj_no)
		return(i);
    return(-1);
}
bool    is_maxed(int num, int mode)
{
    int ii, vnum,rnum;
    
    if (mode == VIRTUAL)
	{
	    vnum = num;
	    rnum = real_object(vnum);
	}
    else{
	vnum = obj_index[num].virtual;
	rnum = num;
    }
    
    ii =  obj_is_in_limit_table(vnum);
    if (ii <0)
	ii = create_ol_entry(vnum);
    
    if (obj_proto[rnum].obj_flags.value[6] <
	obj_limit_table[ii].no_stored + obj_limit_table[ii].no_loaded)
	return(TRUE);
    else
	return(FALSE);
    
}
void limited_to_char(int obj_no)
{
    int ii=0;

    ii = obj_is_in_limit_table(obj_no);
    if (ii <0)
	ii = create_ol_entry(obj_no);
    obj_limit_table[ii].no_stored++;
    obj_limit_table[ii].no_loaded--;
    obj_limit_table[ii].modified++;

}

void load_limited(int obj_no)
{
    int ii=0;

    ii = obj_is_in_limit_table(obj_no);
    if (ii <0)
	ii = create_ol_entry(obj_no);

    obj_limit_table[ii].no_loaded++;
    if (obj_limit_table[ii].no_loaded < 0)
	obj_limit_table[ii].no_loaded =0;
}
void extract_limited(int obj_no)
{
    int ii=0;


    ii = obj_is_in_limit_table(obj_no);
    if (ii <0)
	ii = create_ol_entry(obj_no);
    else
	obj_limit_table[ii].no_loaded--;
    
    if (obj_limit_table[ii].no_loaded < 0)
	obj_limit_table[ii].no_loaded =0;

}

void limited_from_char(int obj_no)
{
    int ii;
    
    ii = obj_is_in_limit_table(obj_no);
    if (ii >= 0)
	{
	    obj_limit_table[ii].no_stored--;
	    if (obj_limit_table[ii].no_stored <0)
		obj_limit_table[ii].no_stored = 0;
	    obj_limit_table[ii].no_loaded++;
	}
    else{
	ii = create_ol_entry(obj_no);
	obj_limit_table[ii].no_loaded++;}
    
    obj_limit_table[ii].modified++;
    
}

void save_an_object_limit_datum(int obj_no)
{
    int which;
    struct obj_limit_type dummy;
    
    for (which = 0; which <= top_of_ol_table ; which++)
	if (obj_no == obj_limit_table[which].obj_num)
	    break;
    if (which >=0)
	if (obj_limit_table[which].modified > 0)
	    {
		dummy.no_stored = obj_limit_table[which].no_stored;
		dummy.obj_num = obj_limit_table[which].obj_num;
		obj_limit_table[which].modified = 0;
		fseek(obj_limit_fl, which*sizeof(struct obj_limit_type), SEEK_SET);
		fwrite(&dummy, sizeof(struct obj_limit_type),1, obj_limit_fl);
	    }
}

void save_all_object_limit_data(void)
{
    int which;
    struct obj_limit_type dummy;
    
    for (which = 0; which <= top_of_ol_table ; which++)
      if (obj_limit_table[which].modified >0)
	{
	  dummy.no_stored = obj_limit_table[which].no_stored;
	  dummy.obj_num = obj_limit_table[which].obj_num;
	  obj_limit_table[which].modified = 0;
	  fseek(obj_limit_fl, which*sizeof(struct obj_limit_type), SEEK_SET);
	  fwrite(&dummy, sizeof(struct obj_limit_type),1, obj_limit_fl);
	}
}

/* create a new entry in the in-memory index table for the player file */
int	create_entry(char *name)
{
   int	i;

   if (top_of_p_table == -1) {
      CREATE(player_table, struct player_index_element, 1);
      top_of_p_table = 0;
   } else if (!(player_table = (struct player_index_element *)
       realloc(player_table, sizeof(struct player_index_element) * 
       (++top_of_p_table + 1)))) {
      perror("create entry");
      exit(1);
   }

   CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);

   /* copy lowercase equivalent of name to table field */
   for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
	i++)
      ;
   
   return (top_of_p_table);
}




/* write the vital data of a player to the player file */
void	save_char(struct char_data *ch, short load_room)
{
   struct descriptor_data *d;
   struct char_file_u st;

   if (IS_NPC(ch) || !ch->desc)
      return;
   
   d = ch->desc;
   if (!d->connected && d->original)
       ch = d->original;
   
   char_to_store(ch, &st);

   strncpy(st.host, d->host, HOST_LEN);
   st.host[HOST_LEN] = '\0';

   if (!PLR_FLAGGED(ch, PLR_LOADROOM)) 
       st.specials2.load_room = load_room;


   strcpy(st.pwd, d->pwd);

   fseek(player_fl, d->pos * sizeof(struct char_file_u), SEEK_SET);
   fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
}


/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char	*fread_string(FILE *fl, char *error)
{
   char	buf[MAX_STRING_LENGTH], tmp[500];
   char	*rslt;
   register char	*point;
   int	flag;

   bzero(buf, MAX_STRING_LENGTH);

   do {
      if (!fgets(tmp, MAX_STRING_LENGTH, fl)) {
	 fprintf(stderr, "fread_string: format error at or near %s\n", error);
	 exit(0);
      }

      if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH) {
	 logg("SYSERR: fread_string: string too large (db.c)");
	 exit(0);
      } else
	 strcat(buf, tmp);

      for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point); 
          point--)
	 ;
      if ((flag = (*point == '~')))
	 if (*(buf + strlen(buf) - 3) == '\n') {
	    *(buf + strlen(buf) - 3) = '\r';
	    *(buf + strlen(buf) - 2) = '\r';	    
	    *(buf + strlen(buf) - 1) = '\0';
	 } 
	 else
	    *(buf + strlen(buf) - 2) = '\0';
      else {
	 *(buf + strlen(buf) + 1) = '\0';
	 *(buf + strlen(buf)) = '\r';
      }
   } while (!flag);

   /* do the allocate boogie  */

   if (strlen(buf) > 0) {
      CREATE(rslt, char, strlen(buf) + 1);
      strcpy(rslt, buf);
   } else
       rslt = 0;
   return(rslt);
}





/* release memory allocated for a char struct */
void	free_char(struct char_data *ch)
{
  int	i;
  
  if (ch->specials.poofIn) 
    free(ch->specials.poofIn);
  if (ch->specials.poofOut) 
    free(ch->specials.poofOut);
  if (ch->player.prmpt)
    free(ch->player.prmpt);
  if (ch->player.title)
    free(ch->player.title);
  for (i=0;i<MAX_ALIASES; i++)
    {
      if (ch->specials.aliases[i].alias)
	free(ch->specials.aliases[i].alias);
      if (ch->specials.aliases[i].text)
	free(ch->specials.aliases[i].text);
    }
  
  if (!IS_NPC(ch) || (IS_NPC(ch) && ch->nr == -1)) {
    if (GET_NAME(ch)) 
      free(GET_NAME(ch));
    if (ch->player.short_descr) 
      free(ch->player.short_descr);
    if (ch->player.long_descr) 
      free(ch->player.long_descr);
    if (ch->player.description) 
      free(ch->player.description);
  }
  else if ((i = ch->nr) > -1) {
    if (ch->player.name && ch->player.name != mob_proto[i].player.name)
      free(ch->player.name);
    if (ch->player.title && ch->player.title != mob_proto[i].player.title)
      free(ch->player.title);
    if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description && ch->player.description != mob_proto[i].player.description)
      free(ch->player.description);
  }
  
  if (ch->skills) {
    free(ch->skills);
    if (IS_NPC(ch))
      logg("SYSERR: Mob had skills array allocated!");
  }
  
  while (ch->affected)
    affect_remove(ch, ch->affected);
  
  free(ch);
}




/* release memory allocated for an obj struct */
void	free_obj(struct obj_data *obj)
{
   struct extra_descr_data *this=0, *next_one=0;

   if (obj->item_number == -1) {
     if (obj->name) 
       free(obj->name);
     if (obj->description) 
       free(obj->description);
     if (obj->short_description) 
       free(obj->short_description);
     if (obj->action_description) 
       free(obj->action_description);
     if (obj->ex_description)
       for (this = obj->ex_description; this; this = next_one) {
	 next_one = this->next;
	 if (this->keyword) 
	   free(this->keyword);
	 if (this->description) 
	   free(this->description);
	 free(this);
       }
   }
   else {
     if (obj->name &&
	 (obj->name != obj_proto[obj->item_number].name))
       free(obj->name);
     if (obj->description &&
	 (obj->description != obj_proto[obj->item_number].description))
       free(obj->description);
     if (obj->short_description &&
	 (obj->short_description != obj_proto[obj->item_number].short_description))
       free(obj->short_description);
     if (obj->action_description &&
	 (obj->action_description != obj_proto[obj->item_number].action_description))
       free(obj->action_description);
     if (obj->ex_description &&
	 (obj->ex_description != obj_proto[obj->item_number].ex_description))
       for (this = obj->ex_description; this; this = next_one) {
	 next_one = this->next;
	 if (this->keyword) 
	   free(this->keyword);
	 if (this->description) 
	   free(this->description);
	 free(this);
       }
   }
   free(obj);
}





/* read contets of a text file, alloc space, point buf to it */
int	file_to_string_alloc(char *name, char **buf)
{
   char	temp[MAX_STRING_LENGTH];

   if (file_to_string(name, temp) < 0)
      return -1;

   if (*buf)
      free(*buf);

   *buf = str_dup(temp);

   return 0;
}




/* read contents of a text file, and place in buf */
int	file_to_string(char *name, char *buf)
{
   FILE * fl;
   char	tmp[100];

   *buf = '\0';

   if (!(fl = fopen(name, "r"))) {
      sprintf(tmp, "Error reading %s", name);
      perror(tmp);
      *buf = '\0';
      return(-1);
   }

   do {
      fgets(tmp, 99, fl);

      if (!feof(fl)) {
	 if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
	    logg("SYSERR: fl->strng: string too big (db.c, file_to_string)");
	    *buf = '\0';
	    return(-1);
	 }

	 strcat(buf, tmp);
	 *(buf + strlen(buf) + 1) = '\0';
	 *(buf + strlen(buf)) = '\r';
      }
   } while (!feof(fl));

   fclose(fl);

   return(0);
}




/* clear some of the the working variables of a char */
void	reset_char(struct char_data *ch)
{
   int	i;

   for (i = 0; i < MAX_WEAR; i++) /* Initialisering */
      ch->equipment[i] = 0;

   ch->followers = 0;
   ch->master = 0;
   /*	ch->in_room = NOWHERE; Used for start in room */
   ch->inventory = 0;
   ch->next = 0;
   ch->next_fighting = 0;
   ch->next_in_room = 0;
   ch->specials.fighting = 0;
   ch->specials.position = POSITION_STANDING;
   ch->specials.default_pos = POSITION_STANDING;
   ch->specials.carry_weight = 0;
   ch->specials.carry_items = 0;

   if (GET_HIT(ch) <= 0)
      GET_HIT(ch) = 1;
   if (GET_MOVE(ch) <= 0)
      GET_MOVE(ch) = 1;
}



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void	clear_char(struct char_data *ch)
{
   memset((char *)ch, (char)'\0', (int)sizeof(struct char_data));

   ch->in_room = NOWHERE;
   ch->specials.was_in_room = NOWHERE;
   ch->specials.position = POSITION_STANDING;
   ch->specials.default_pos = POSITION_STANDING;

   set_race_characteristics(ch);   

}
void	clear_room(struct room_data *room)
{
   memset((char *)room, (char)'\0', (int)sizeof(struct room_data));
}


void	clear_object(struct obj_data *obj)
{
   memset((char *)obj, (char)'\0', (int)sizeof(struct obj_data));

   obj->item_number = -1;
   obj->in_room	  = NOWHERE;
   obj->obj_flags.timer = -1;
}




/* initialize a new character only if class is set */
void	init_char(struct char_data *ch)
{
   int	i;

   /* *** if this is our first player --- he be God *** */

   if (top_of_p_table < 0) {
      GET_EXP(ch) = 9000000;
      GET_LEVEL(ch) = LEVEL_IMPL;
      GET_SUB_LEVEL(ch) = 0;
      ch->points.max_hit = 5000;
      ch->points.max_move = 1000;
   }

   set_title(ch);

   ch->player.short_descr = 0;
   ch->player.long_descr = 0;
   ch->player.description = 0;

   ch->player.hometown = number(1, 4);

   ch->player.time.birth = time(0);
   ch->player.time.played = 0;
   ch->player.time.logon = time(0);

   for (i = 0; i < MAX_TOUNGE; i++)
      ch->player.talks[i] = 0;


   /* make favors for sex */
   if (ch->player.sex == SEX_MALE) {
     ch->player.weight += number(1500, 2500);
     ch->player.height += number(160, 200);
   } else {
     ch->player.weight += number(1000, 2100);
     ch->player.height += number(150, 180);
   }

   ch->player.weight = MAX(10, ch->player.weight );
   ch->player.height = MAX(10, ch->player.height );
   
   ch->points.sub_level = 0; 
   ch->points.hit = GET_MAX_HIT(ch);
   ch->points.max_move = 100;
   ch->points.move = GET_MAX_MOVE(ch);
   set_race_characteristics(ch);

   
   ch->specials2.idnum = ++top_idnum;
   ch->specials2.last_tell_id = NOBODY;   
   if (!ch->skills)
      CREATE(ch->skills, byte, MAX_SKILLS);

   for (i = 0; i < MAX_SKILLS; i++) {
      if (GET_LEVEL(ch) < LEVEL_IMPL)
	 SET_SKILL(ch, i, 0)
      else
	 SET_SKILL(ch, i, 30);
   }

   ch->specials.affected_by = 0;

   for (i = 0; i < 5; i++)
      ch->specials2.apply_saving_throw[i] = 0;

   for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (GET_LEVEL(ch) == LEVEL_IMPL ? -1 : 24);
}



/* returns the real number of the room with given virtual number */
int	real_room(int virtual)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_world;

   if (virtual < 0)
       return(-1);
   
   /* perform binary search on world-table */
   for (; ; ) {
      mid = (bot + top) / 2;

      if ((world + mid)->number == virtual)
	 return(mid);
      if (bot >= top) {
	 if (!mini_mud)
	    fprintf(stderr, "Room %d does not exist in database\n", virtual);
	 return(-1);
      }
      if ((world + mid)->number > virtual)
	 top = mid - 1;
      else
	 bot = mid + 1;
   }
}






/* returns the real number of the monster with given virtual number */
int	real_mobile(int virtual)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_mobt;

   /* perform binary search on mob-table */
   for (; ; ) {
      mid = (bot + top) / 2;

      if ((mob_index + mid)->virtual == virtual)
	 return(mid);
      if (bot >= top)
	 return(-1);
      if ((mob_index + mid)->virtual > virtual)
	 top = mid - 1;
      else
	 bot = mid + 1;
   }
}






/* returns the real number of the object with given virtual number */
int	real_object(int virtual)
{
   int	bot, top, mid;

   bot = 0;
   top = top_of_objt;

   /* perform binary search on obj-table */
/*   for (; ; ) {
      mid = (bot + top) / 2;

      if ((obj_index + mid)->virtual == virtual)
	 return(mid);
      if (bot >= top)
	 return(-1);
      if ((obj_index + mid)->virtual > virtual)
	 top = mid - 1;
      else
	 bot = mid + 1;
   } */

   for(mid = 0; mid <= top; mid++){
      if(obj_index[mid].virtual == virtual){
         return(mid);
      }
   }

  return(-1);
}

void boot_slave( void )
{
    int sv[2];
    int i;

    if( slave_socket != -1 ) {
        close( slave_socket );
        slave_socket = -1;
    }

    if( socketpair( AF_UNIX, SOCK_DGRAM, 0, sv ) < 0 ) {
      sprintf(buf, "boot_slave: socketpair: %s", strerror( errno ) );
      logg( buf);
        return;
    }
    /* set to nonblocking */
    if( fcntl( sv[0], F_SETFL, FNDELAY ) == -1 ) {
      sprintf(buf, "boot_slave: fcntl( F_SETFL, FNDELAY ): %s",
	       strerror( errno ) );
      logg(buf);
      close(sv[0]);
      close(sv[1]);
      return;
    }
    slave_pid = vfork();
    switch( slave_pid ) {
    case -1:
        sprintf( buf,"boot_slave: vfork: %s", strerror( errno ) );
	logg(buf);
        close( sv[0] );
        close( sv[1] );
        return;

    case 0: /* child */
        close( sv[0] );
        close( 0 );
        close( 1 );
        if( dup2( sv[1], 0 ) == -1 ) {
            sprintf(buf, "boot_slave: child: unable to dup stdin: %s", strerror( errno ) );
	    logg(buf);
            _exit( 1 );
        }
        if( dup2( sv[1], 1 ) == -1 ) {
            sprintf(buf, "boot_slave: child: unable to dup stdout: %s", strerror( errno ) );
	    logg(buf);
            _exit( 1 );
        }
        for( i = 3; i < MAX_DESCRIPTORS_AVAILABLE; ++i ) {
            close( i );
        }
        execlp( "slave", "slave", NULL );
        sprintf(buf, "boot_slave: child: unable to exec: %s", strerror( errno ) );
	logg(buf);
        _exit( 1 );
    }
    close( sv[1] );

    if( fcntl(sv[0], F_SETFL, FNDELAY ) == -1 ) {
        sprintf(buf, "boot_slave: fcntl: %s", strerror( errno ) );
        close( sv[0] );
        return;
    }
    slave_socket = sv[0];
}
