/* ************************************************************************
*   File: config.c                                      Part of CircleMUD *
*  Usage: Configuration of various aspects of CircleMUD operation         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include "structs.h"

#define TRUE	1
#define YES	1
#define FALSE	0
#define NO	0

/*
Below are several constants which you can change to alter certain aspects
of the way CircleMUD acts.  Since this is a .c file, all you have to do
to change one of the constants (assuming you keep your object files around)
is change the constant in this file and type 'make'.  Make will recompile
this file and relink; you don't have to wait for the whole thing to recompile
as you do if you change a header file.

I realize that it would be slightly more efficient to have lots of
#defines strewn about, so that, for example, the autowiz code isn't
compiled at all if you don't want to use autowiz.  However, the actual
code for the various options is quite small, as is the computational time
in checking the option you've selected at run-time, so I've decided the
convenience of having all your options in this one file outweighs the
efficency of doing it the other way.
*/

/****************************************************************************/
/****************************************************************************/


/* GAME PLAY OPTIONS */

/* is playerkilling allowed? */
int pk_allowed = NO;

/* is playerthieving allowed? */
int pt_allowed = NO;

/* minimum level a player must be to shout/holler/gossip/auction */
int level_can_shout = 2;

/* number of movement points it costs to holler */
int holler_move_cost = 20;

/* number of tics (usually 75 seconds) before PC/NPC corpses decompose */
int max_npc_corpse_time = 5;
int max_pc_corpse_time = 10;

/* should items in death traps automatically be junked? */
int dts_are_dumps = YES;

/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/* maximum number of items players are allowed to rent */
int max_obj_save = 70;

/* receptionist's surcharge on top of item costs */
int min_rent_cost = 0;

/* Should the game automatically save people?  (i.e., save player data
   every 4 kills (on average), and Crash-save as defined below. */
int auto_save = YES;

/* if auto_save (above) is yes, how often (in minutes) should the MUD
   Crash-save people's objects? */
int autosave_time = 4;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
int crash_file_timeout = 10;

/* Lifetime of normal rent files in days */
int rent_file_timeout = 90;


/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that immorts should enter at by default */
short immort_start_room = 1204;

/* virtual number of room that mortals should enter at */
short mortal_start_room = 2000;

/* virtual number of room that frozen players should enter at */
short frozen_start_room = 1202;

/* virtual numbers of donation rooms.  note: you must change code in
   do_drop of act.obj1.c if you change the number of non-NOWHERE
   donation rooms.
*/
short donation_room_1 = 2024;
short donation_room_2 = 7194;
short donation_room_3 = 11288;
short donation_room_4 = NOWHERE;


/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/* default port the game should run on if no port given on command-line */
int DFLT_PORT = 4000;

/* default directory to use as data directory */
char *DFLT_DIR = "lib";

/* maximum number of players allowed before game starts to turn people away */
int MAX_PLAYERS = 30;

/* maximum number of files your system allows a process to have open at
   once -- not used if you define #TABLE_SIZE in comm.c or if your system
   defines OPEN_MAX.
*/
int MAX_DESCRIPTORS_AVAILABLE = 256;

/* Some nameservers (such as the one here at JHU) are slow and cause the
   game to lag terribly every time someone logs in.  The lag is caused by
   the gethostbyaddr() function -- the function which resolves a numeric
   IP address (such as 128.220.13.30) into an alphabetic name (such as
   circle.cs.jhu.edu).

   The nameserver at JHU can get so bad at times that the incredible lag
   caused by gethostbyaddr() isn't worth the luxury of having names
   instead of numbers for players' sitenames.

   If your nameserver is fast, set the variable below to NO.  If your
   nameserver is slow, of it you would simply prefer to have numbers
   instead of names for some other reason, set the variable to YES.

   You can experiment with the setting of nameserver_is_slow on-line using
   the SLOWNS command from within the MUD.
*/

int nameserver_is_slow = NO;
   

char *MENU = 
"\r\n"
"          Archipelago MUD         \r\n"
"                                      CircleMUD 2.2 by: Jeremy Elson.\r\n"
"    [0]  Exit from Archipelago.       DikuMUD (Gamma 0.0) by:  \r\n"
"    [1]  Enter Archipelago.          Katja Nyboe, Michael Seifert,   \r\n"
"    [2]  Describe your Character.    Tom Madsen, Sebastian Hammer,\r\n"
"    [3]  Info about Game Play.       and Hans Henrik Staerfeldt.  \r\n"
"    [4]  Change your Password.                                       \r\n"
"    [5]  Delete this Character.       Archipelago by: Alastair Neil, \r\n"
"    [6]  Ban this Character.         Soraya Ghiasi and Shawn L. Baird.\r\n"
"\r\n"
"What is thy choice?\r\n";
char *ANSI_MENU =
"\r\n"
"          Archipelago MUD         \r\n"
"                                      CircleMUD 2.2 by: Jeremy Elson.\r\n"
"    [0]  Exit from Archipelago.       DikuMUD (Gamma 0.0) by:  \r\n"
"    [1]  Enter Archipelago.          Katja Nyboe, Michael Seifert,   \r\n"
"    [2]  Describe your Character.    Tom Madsen, Sebastian Hammer,\r\n"
"    [3]  Info about Game Play.       and Hans Henrik Staerfeldt.  \r\n"
"    [4]  Change your Password.                                       \r\n"
"    [5]  Delete this Character.       Archipelago by: Alastair Neil, \r\n"
"    [6]  Ban this Character.         Soraya Ghiasi and Shawn L. Baird.\r\n"
"\r\n"
"What is thy choice?\r\n";



char *GREETINGS =
"\r\n"
"                           Archipelago a MUD based on\r\n"
"                                CircleMUD 2.2\r\n"
"                             and DikuMUD (Gamma 0.0)\r\n"
"                                  created by\r\n"
"                      H. Staerfeldt, K. Nyboe, M. Seifert,\r\n"
"                           T. Madsen and S. Hammer.\r\n\r\n"
"\r\n\0";




char *GREET_ANSI =
"\r\n"
"                          [0;0m[31mArchipelago a MUD based on\r\n"
"                               [0;0m[31mCircleMUD 2.2\r\n"
"                            [0;0m[31mand DikuMUD (Gamma 0.0)\r\n"
"                                 [0;0m[31mcreated by\r\n"
"                     [0;0m[31mH. Staerfeldt, K. Nyboe, M. Seifert\r\n"
"                          [0;0m[31mT. Madsen and S. Hammer.\r\n"
"[0m\r\n\0";


char *WELC_MESSG =
"\r\n"
"Welcome to Archipelago!  Prepare to enter the realms of mystery.."
"\r\n\r\n";

char *START_MESSG =
"Welcome to Archipelago.\r\n\r\n";

/****************************************************************************/
/****************************************************************************/


/* AUTOWIZ OPTIONS */

/* Should the game automatically create a new wizlist/immlist every time
   someone immorts, or is promoted to a higher (or lower) god level? */
int use_autowiz = YES;

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead. */
int min_wizlist_lev = LEVEL_GOD;














