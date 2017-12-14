/* ************************************************************************
*   File: comm.c                                        Part of CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "limits.h"
#include "screen.h"
#include "spells.h"

#define MAX_HOSTNAME	256
#define OPT_USEC	250000  /* time delay corresponding to 4 passes/sec */

/* externs */
extern int	restrict;
extern int	mini_mud;
extern int	no_rent_check;
extern FILE	*player_fl;
extern int	DFLT_PORT;
extern char	*DFLT_DIR;
extern int	MAX_PLAYERS;
extern int	MAX_DESCRIPTORS_AVAILABLE;
extern struct   index_data *mob_index;
extern char     *revdirs[];
extern char     *dirs[];
extern struct   room_data *world;		
extern int	top_of_world;   	   	
extern struct   time_info_data time_info;	
extern char	help[];
extern int      slave_socket;
extern pid_t    slave_pid; 

struct spell_info_type spell_info[MAX_SPL_LIST];


/* local globals */
struct descriptor_data *descriptor_list, *next_to_process;
struct txt_block *bufpool = 0;	/* pool of large output buffers */
int	buf_largecount;		/* # of large buffers which exist */
int	buf_overflows;		/* # of overflows of output */
int	buf_switches;		/* # of switches from small to large buf */
int	circle_shutdown = 0;	/* clean shutdown*/
int	circle_reboot = 0;	/* reboot the game after a shutdown */
int	no_specials = 0;	/* Suppress ass. of special routines */
int	no_limited_check = 0;	/* Suppress ass. of special routines */
int	last_desc = 0;		/* last unique num assigned to a desc. */
int	mother_desc = 0;	/* file desc of the mother connection */
int	maxdesc;		/* highest desc num used */
int	avail_descs;		/* max descriptors available */
int	tics = 0;		/* for extern checkpointing */
int     pulse =0;               /* for fine grained event management */
int	port;
extern int	nameserver_is_slow;	/* see config.c */
extern int	auto_save;		/* see config.c */
extern int	autosave_time;		/* see config.c */
struct event_type *events[301];

/* functions in this file */
void    parse_prompt(struct char_data *ch, char *pmt);
void    parse_text(struct char_data *c,struct char_data *vict, int mode, char *pmt);
int	get_from_q(struct txt_q *queue, char *dest);
void	run_the_game(int port);
void	game_loop(int s);
int	init_socket(int port);
int	new_connection(int s);
int	new_descriptor(int s);
int	process_output(struct descriptor_data *t);
int	process_input(struct descriptor_data *t);
void	close_sockets(int s);
void	close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval *a, struct timeval *b);
void	flush_queues(struct descriptor_data *d);
void	nonblock(int s);
int	perform_subst(struct descriptor_data *t, char *orig, char *subst);
void    cmpact(char *input);
void    extract_event(struct char_data *ch);
void    add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict);
void    process_events(int pulse);
static int get_slave_result(void);
/* extern fcnts */
char    *pluralise_string(char *arg);
void	boot_db(void);
void	zone_update(void);
void	affect_update(void); /* In spells.c */
void	point_update(void);  /* In limits.c */
void	mobile_activity(void);
void	room_activity(void);
void	string_add(struct descriptor_data *d, char *str);
void	perform_violence(void);
void	show_string(struct descriptor_data *d, char *input);
void	check_reboot(void);
int	isbanned(char *hostname);
void	weather_and_time(int mode);
void    shopedit(struct descriptor_data *d, char *str);
void    roomedit(struct descriptor_data *d, char *str);
void    mobedit(struct descriptor_data *d, char *str);
void    objedit(struct descriptor_data *d, char *str);
void    assign_levels();
char    *report_cost(int gold);
void    death_cry(struct char_data *ch);
int     spell_lev(struct char_data *caster, int spell);
char    *first_name(char *buf);
int     is_goditem(struct obj_data *j);
void    clear_queue(struct txt_q *queue);


/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */

int	main(int argc, char **argv)
{
    char	buf[512];
    int	pos = 1;
    char   *dir;

   port = DFLT_PORT;
   dir = DFLT_DIR;

   if (strstr(argv[0], "test")){
     no_limited_check = 1;
     logg("Suppressing check of limited equipment.");
   }
   while ((pos < argc) && (*(argv[pos]) == '-')) {
     switch (*(argv[pos] + 1)){
     case 'd':
       if (*(argv[pos] + 2))
	 dir = argv[pos] + 2;
       else if (++pos < argc)
	 dir = argv[pos];
       else {
	 logg("Directory arg expected after option -d.");
	 exit(0);
       }
       break;
     case 'm':
       mini_mud = 1;
       no_rent_check = 1;
       logg("Running in minimized mode & with no rent check.");
       break;
     case 'q':
       no_rent_check = 1;
       logg("Quick boot mode -- rent check suppressed.");
       break;
     case 'r':
       restrict = 1;
       logg("Restricting game -- no new players allowed.");
       break;
     case 's':
       no_specials = 1;
       logg("Suppressing assignment of special routines.");
       break;
     default:
       sprintf(buf, "SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
       logg(buf);
       break;
     }
     pos++;
   }
   
   if (pos < argc)
     if (!isdigit(*argv[pos])) {
       fprintf(stderr, "Usage: %s [-m] [-q] [-r] [-s] [-d pathname] [ port # ]\n", argv[0]);
       exit(0);
     }
   if (argv[pos])
     port = atoi(argv[pos]);

   assign_levels();
   sprintf(buf, "loading levels table");
   logg(buf);
   sprintf(buf, "Running game on port %d.", port);
   logg(buf);

   if (chdir(dir) < 0) {
      perror("Fatal error changing to data directory");
      exit(0);
   }

   sprintf(buf, "Using %s as data directory.", dir);
   logg(buf);

   srandom(time(0));

   run_the_game(port);
   return(0);
}





/* Init sockets, run game, and cleanup sockets */
void	run_the_game(int port)
{
   int	s;

   void	signal_setup(void);

   descriptor_list = NULL;

   logg("Signal trapping.");
   signal_setup();

   logg("Opening mother connection.");
   mother_desc = s = init_socket(port);

   boot_db();

   logg("Entering game loop.");

   game_loop(s);

   close_sockets(s);
   fclose(player_fl);

   if (circle_reboot) {
      logg("Rebooting.");
      exit(52);            /* what's so great about HHGTTG, anyhow? */
   }

   logg("Normal termination of game.");
}






/* Accept new connects, relay commands, and call 'heartbeat-functs' */
void	game_loop(int s)
{
   fd_set input_set, output_set, exc_set;
   struct timeval last_time, now, timespent, timeout, null_time;
   static struct timeval opt_time;
   char	comm[MAX_INPUT_LENGTH];
   char	prompt[MAX_INPUT_LENGTH];
   struct descriptor_data *point, *next_point;
   int mins_since_crashsave = 0, mask;
   int	i,sockets_connected, sockets_playing;
   char	buf[100];

   null_time.tv_sec = 0;
   null_time.tv_usec = 0;

   opt_time.tv_usec = OPT_USEC;  /* Init time values */
   opt_time.tv_sec = 0;
   gettimeofday(&last_time, (struct timezone *) 0);

   maxdesc = s;


#if defined (OPEN_MAX)
   avail_descs = OPEN_MAX - 8;
#elif defined (USE_TABLE_SIZE)
   {
      int retval;

      retval = setdtablesize(64);
      if (retval == -1)
        logg("SYSERR: unable to set table size");
      else {
         sprintf(buf, "%s %d\n", "dtablesize set to: ", retval);
         logg(buf);
      }
      avail_descs = getdtablesize() - 8;
   }
#else
   avail_descs = MAX_DESCRIPTORS_AVAILABLE;
#endif

   avail_descs = MIN(avail_descs, MAX_PLAYERS);

   mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |  
      sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) | 
       sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP) | 	
       sigmask(SIGSEGV) | sigmask(SIGBUS);

   for (i=0;i<=300;i++)
       events[i] = 0;
   /* Main loop */
   while (!circle_shutdown) {
      /* Check what's happening out there */
      FD_ZERO(&input_set);
      FD_ZERO(&output_set);
      FD_ZERO(&exc_set);
      FD_SET(s, &input_set);
      if (slave_socket != -1) 
	FD_SET(slave_socket, &input_set);
      for (point = descriptor_list; point; point = point->next) {
	 FD_SET(point->descriptor, &input_set);
	 FD_SET(point->descriptor, &exc_set);
	 FD_SET(point->descriptor, &output_set);
      }

      /* check out the time */
      gettimeofday(&now, (struct timezone *) 0);
      timespent = timediff(&now, &last_time);
      timeout = timediff(&opt_time, &timespent);
      last_time.tv_sec = now.tv_sec + timeout.tv_sec;
      last_time.tv_usec = now.tv_usec + timeout.tv_usec;
      if (last_time.tv_usec >= 1000000) {
	 last_time.tv_usec -= 1000000;
	 last_time.tv_sec++;
      }

      sigsetmask(mask);

      if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time)
           < 0) {
	 perror("Select poll");
	 return;
      }

      if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
	 perror("Select sleep");
	 exit(1);
      }

/*      sigsetmask(0);  changed AJN 28th Sept. 94 */
       sigsetmask(sigmask(SIGPIPE));

      /* Respond to whatver might be happening */
       if (slave_socket != -1 && FD_ISSET( slave_socket, &input_set)) {
	 while (get_slave_result() == 0)
	   ;
       }  

       /* New connection? */
       if (FD_ISSET(s, &input_set))
	 if (new_descriptor(s) < 0)
	   perror("New connection");
       
       /* kick out the freaky folks */
       for (point = descriptor_list; point; point = next_point) {
	 next_point = point->next;
	 if (FD_ISSET(point->descriptor, &exc_set)) {
	   FD_CLR(point->descriptor, &input_set);
	   FD_CLR(point->descriptor, &output_set);
	   close_socket(point);
	 }
       }

       for (point = descriptor_list; point; point = next_point) {
	 next_point = point->next;
	 if (FD_ISSET(point->descriptor, &input_set))
	   if (process_input(point) < 0)
	     close_socket(point);
       }

      /* process_commands; */
       for (point = descriptor_list; point; point = next_to_process) {
	 next_to_process = point->next;
	 
	 if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
	   if (point->character && point->connected == CON_PLYNG && 
	        point->character->specials.was_in_room != NOWHERE) {
	      if (point->character->in_room != NOWHERE)
		 char_from_room(point->character);
	       char_to_room(point->character, 
			    point->character->specials.was_in_room, FALSE);
	       point->character->specials.was_in_room = NOWHERE;
	       act("$n has returned.", 	TRUE, point->character, 0, 0, TO_ROOM);
	    }

	    point->wait = 1;
	    if (point->character)
	      point->character->specials.timer = 0;
	    point->prompt_mode = 1;
	    
	    if (point->str)
	      string_add(point, comm);
	    else if (point->mob_edit)
	      mobedit(point, comm);
	    else if (point->obj_edit)
	      objedit(point, comm);
	    else if (point->room_edit)
	      roomedit(point, comm);
	    else if (point->shop_edit)
	      shopedit(point, comm);	    	    	    
	    else if (point->zone_edit)
	      zoneedit(point, comm);	    	    	    
	    else if (!point->connected)
	      if (point->showstr_point)
		show_string(point, comm);
	      else
		  command_interpreter(point->character, comm);
	    else
	      nanny(point, comm);
	 }
      }


      for (point = descriptor_list; point; point = next_point) {
	 next_point = point->next;
	 if (FD_ISSET(point->descriptor, &output_set) && *(point->output))
	    if (process_output(point) < 0)
	       close_socket(point);
	    else
	       point->prompt_mode = 1;
      }

      /* kick out the Phreaky Pholks II  -JE */
      for (point = descriptor_list; point; point = next_to_process) {
	 next_to_process = point->next;
	 if (STATE(point) == CON_CLOSE) 
	    close_socket(point);
      }


      /* give the people some prompts */
      for (point = descriptor_list; point; point = point->next){
	if (point->prompt_mode) {
	  if (point->str) {
	    write_to_descriptor(point->descriptor, "] ");
	  }
	  else if (point->mob_edit
		   || point->shop_edit
		   || point->obj_edit
		   || point->room_edit)
	    strcpy(prompt, "**Enter Q to quit**");
	  else if (!point->connected) {
	    if (point->showstr_point) {
	      strcpy(prompt, "[*** Press return to continue, q to quit ***]");
	    }
	    else {
	      parse_prompt(point->character, prompt);
	    }
	    write_to_descriptor(point->descriptor,  prompt);
	    SET_BIT(PLR_FLAGS(point->character), PLR_NEEDCRLF);
	  }
	  point->prompt_mode = 0;
	}
      }
      
      
      /* handle heartbeat stuff */
      /* Note: pulse now changes every 1/4 sec  */
      process_events(pulse); 
      pulse++;

      if (!(pulse % PULSE_ZONE))	zone_update();
      if (!(pulse % (2*PULSE_VIOLENCE))) mobile_activity();
      if (!(pulse % PULSE_ROOM))	room_activity();
      if (!(pulse % PULSE_ROOM))	object_activity(); 
      if (!(pulse % PULSE_VIOLENCE))	perform_violence();

      if (!(pulse % (SECS_PER_MUD_HOUR * 4))) {
	 weather_and_time(1);
	 affect_update();
	 point_update();
	 fflush(player_fl);
      }


      if (auto_save)
         if (!(pulse % (60 * 4))) /* one minute */
	    if (++mins_since_crashsave >= autosave_time) {
	       mins_since_crashsave = 0;
	       Crash_save_all();
	    }

      if (time_info.hours && !((time_info.hours*SECS_PER_MUD_HOUR*4 + pulse)  % 1200)) {
	 sockets_connected = sockets_playing = 0;

	 for (point = descriptor_list; point; point = next_point) {
	    next_point = point->next;
	    sockets_connected++;
	    if (!point->connected)
	       sockets_playing++;
	 }

	 sprintf(buf, "nusage: %-3d sockets connected, %-3d sockets playing",
	     sockets_connected,
	     sockets_playing);
	 logg(buf);

#ifdef RUSAGE
	  {
	    struct rusage rusagedata;

	    getrusage(0, &rusagedata);
	    sprintf(buf, "rusage: %d %d %d %d %d %d %d",
	        rusagedata.ru_utime.tv_sec,
	        rusagedata.ru_stime.tv_sec,
	        rusagedata.ru_maxrss,
	        rusagedata.ru_ixrss,
	        rusagedata.ru_ismrss,
	        rusagedata.ru_idrss,
	        rusagedata.ru_isrss);
	    logg(buf);
	 }
#endif 

      }

      if (pulse >= 300) {
	 pulse = 0;
	 check_reboot();
      }

      tics++;        /* tics since last checkpoint signal */
   }
}


/* ******************************************************************
*  general utility stuff (for local use)			    *
****************************************************************** */

void parse_prompt(struct char_data *ch, char *pmt)
{
    static const char *default_prompt = "H: %1%h%0/%H V: %5%v%0/%V Ex: %X%n>";
    char bff[512];
    char bff2[512];
    const char *string;
    const char *i;
    char *point;
    int percent, slev;

    if( ch->player.prmpt == NULL || ch->player.prmpt[0] == '\0' ) {
        string = default_prompt;
    }
    else{
        string = ch->player.prmpt;
    }
    bzero(bff,512);
    bzero(bff2,512);
    point = bff;
    while( *string != '\0' )
	{
	    if( *string != '%' )
		{
		    *point++ = *string++;
		    continue;
		}
	    ++string;
	    switch( *string )
		{
		default:
		    i = " "; break;
		case '0' :
		    sprintf(bff2,CCNRM(ch, C_NRM));
		    i = bff2; break;
		case '1' :
		    sprintf(bff2,CCRED(ch, C_NRM));
		    i = bff2; break;
		case '2' :
		    sprintf(bff2,CCGRN(ch, C_NRM));
		    i = bff2; break;
		case '3' :
		    sprintf(bff2,CCYEL(ch, C_NRM));
		    i = bff2; break;
		case '4' :
		    sprintf(bff2,CCBLU(ch, C_NRM));
		    i = bff2; break;
		case '5' :
		    sprintf(bff2,CCMAG(ch, C_NRM));
		    i = bff2; break;
		case '6' :
		    sprintf(bff2,CCCYN(ch, C_NRM));
		    i = bff2; break;
		case '7' :
		    sprintf(bff2,CCWHT(ch, C_NRM));
		    i = bff2; break;
		case '8' :
		    sprintf(bff2,CCBLK(ch, C_NRM));
		    i = bff2; break;		    
		case 'b' :
		    sprintf(bff2,CCBLD(ch, C_NRM));
		    i = bff2; break;
		case 'u' :
		    sprintf(bff2,CCUND(ch, C_NRM));
		    i = bff2; break;
		case 'n':
		    sprintf(bff2,"\r\n");
		    i = bff2; break;
		case 'i':
		    if (IS_AFFECTED(ch,AFF_INVISIBLE)){
			sprintf(bff2,"*invis*");
			i=bff2;break;}
		    else if (GET_INVIS_LEV(ch)){
			sprintf(bff2,"<i%d>",GET_INVIS_LEV(ch));
			i = bff2; break;}
                    *bff2 = '\0';
                    i = bff2;
		    break;
		case 'f':
		    if (IS_AFFECTED(ch,AFF_FLY)){
			sprintf(bff2,"*fly*");
			i = bff2; break;}
		    break;
		case 'w':
		    if (IS_AFFECTED(ch,AFF_WATER_BREATH)){
			sprintf(bff2,"*water*");
			i = bff2; break;}
		    break;
		case 'h' :
		    sprintf( bff2, "%d", ch->points.hit );
		    i = bff2; break;
		case 'H' :
		    sprintf( bff2, "%d", ch->points.max_hit );
		    i = bff2; break;
		case 'm' :
		    break;
		case 'M' :
		    break;
		case 'p' :
		    break;
		case 'P' :
		    break;
		case 'v' :
		    sprintf( bff2, "%d", ch->points.move );
		    i = bff2; break;
		case 'V' :
		    sprintf( bff2, "%d", ch->points.max_move );
		    i = bff2; break;
		case 'x' :
		    sprintf( bff2, "%d", ch->points.exp );
		    i = bff2; break;
		case 'X' :
		  slev = (levels_table[GET_LEVEL(ch)+1]
			   - levels_table[GET_LEVEL(ch)])/10;
		  sprintf( bff2, "%d",levels_table[GET_LEVEL(ch)] +
			   slev*(GET_SUB_LEVEL(ch) +1)
			    - GET_EXP(ch) );
		    i = bff2; break;
		case 'l':
		    if(ch->master && !IS_NPC(ch))
			sprintf( bff2, "%s",GET_NAME(ch->master));
		    else if (ch->master && !IS_NPC(ch))
			sprintf( bff2, "%s",PERS(ch->master, ch));
		    else
			sprintf( bff2,"%s", "nobody");
		    i = bff2; break;
		case 'L':
		    if (ch->master)
			sprintf( bff2, "%d",100*GET_HIT(ch->master)
				 /GET_MAX_HIT(ch->master));
		    else
			sprintf( bff2, " ");
		    i = bff2; break;
		case 'o' :
		    if (ch->specials.fighting)
			sprintf( bff2,"%s",PERS(ch->specials.fighting, ch));
		    else
			sprintf( bff2,"%s","nobody");
		    i = bff2; break;
		case 'O':
		    if (ch->specials.fighting){
			if (GET_MAX_HIT(ch->specials.fighting) >0)
			    percent = (100 * GET_HIT(ch->specials.fighting)/
				       GET_MAX_HIT(ch->specials.fighting));
			else
			    percent = -1;
			if (percent >= 100)
			    sprintf(bff2,"%s","unhurt");
			else if (percent >= 90)
			    sprintf(bff2,"%s","a few scratches");
			else if (percent >= 75)
			    sprintf(bff2,"%s","small wounds");
			else if (percent >= 50)
			    sprintf(bff2,"%s","quite a few wounds");
			else if (percent >= 30)
			    sprintf(bff2,"%s","big nasty wounds");
			else if (percent >= 15)
			    sprintf(bff2,"%s","pretty hurt");
			else if (percent >= 0)
			    sprintf(bff2,"%s","awful");
			else
			    sprintf(bff2,"%s","unconscious"); }
		    else
			sprintf(bff2,"%s"," ");
		    i = bff2; break;
		case 'g' :
		    sprintf( bff2, "%s",report_cost(ch->points.gold) );
		    i = bff2; break;
		case 'G' :
		    sprintf( bff2, "%s",report_cost(GET_BANK_GOLD(ch))); 
		    i = bff2; break;
		case 'a' :
		    if( ch->player.level < 5 )
			sprintf( bff2, "%d", ch->specials2.alignment );
		    else
			sprintf( bff2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? 
				 "evil" : "neutral" );
		    i = bff2; break;
		case 'r' :
		    if( world[ch->in_room].name != NULL ){
			if (IS_DARK(ch->in_room) &&
			    !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
			    sprintf( bff2, "Pitch Black");
			else
			    sprintf( bff2, "%s", world[ch->in_room].name );
		    }
		    else
			sprintf( bff2, " " );
		    i = bff2; break;		    
		case 'R' :
		    if( GET_LEVEL( ch ) >= LEVEL_BUILDER && world[ch->in_room].name != NULL )
			sprintf( bff2, "%d", world[ch->in_room].number );
		    else
			sprintf( bff2, " " );
		    i = bff2; break;
		case '%' :
		    sprintf( bff2, "%%" );
		    i = bff2; break;
		}
	    if (strlen(bff) + strlen(bff2) >= 510){
		    strcpy(pmt,bff);
		    return;}
	    ++string;
	    while( (*point = *i) != '\0')
		++point, ++i;      
	}
    point = bff;
    for ( ; *point != '\0'; point++ )
	{
	    if ( *point == '~' )
		*point = '-';
	}
    
    strcpy(pmt,bff);
    return; 
}
void parse_text(struct char_data *ch, struct char_data *vict, int mode,char *text)
{
  char bff[1024];
  char bff2[1024];
  char bff4[1024];
  char def[512];
  char defi[512];    
  const char *string;
  const char *i,*j;
  char *point,*pmt;
  bool start = TRUE, include = TRUE;
  int len=0;


  switch(mode){
  case 0:
    strcpy(defi,"%n the %R %l");
    strcpy(def,"%n the %l %c");
    if( ch->player.title == NULL || ch->player.title[0] == '\0')
      if ( GET_LEVEL(ch) < LEVEL_BUILDER)
	{
	  CREATE(GET_TITLE(ch), char, strlen(def) + 1);
	  strcpy(GET_TITLE(ch), def);
		    
	}
      else
	{
	  CREATE(GET_TITLE(ch), char, strlen(defi) + 1);
	  strcpy(GET_TITLE(ch), defi);
	}
    string = ch->player.title;
    break;
  case 1:
    strcpy(defi,"%n appears with an ear-splitting %bbang!%0");
    if( ch->specials.poofIn == NULL || ch->specials.poofIn[0] == '\0')
      {
	CREATE(ch->specials.poofIn, char, strlen(defi) + 1);
	strcpy(ch->specials.poofIn, defi);
      }
    string = ch->specials.poofIn;
    break;
  case 2:
    strcpy(defi,"%n disappears in a puff of logic.");
    if( ch->specials.poofOut == NULL || ch->specials.poofOut[0] == '\0')
      {
	CREATE(ch->specials.poofOut, char, strlen(defi) + 1);
	strcpy(ch->specials.poofOut, defi);
      }
    string = ch->specials.poofOut;
    break;
  }

  bzero(bff,1024);
  bzero(bff2,1024);
  point = bff;
  pmt = bff4;

  while( *string != '\0' )
    {
      if( *string != '%' )
	{
	  *point++ = *string++;
	  len++;
	  continue;
	}
      while(*(++string) == '%')
	;
      include  = TRUE;
      switch( *string )
	{
	default:
	  i = " "; break;
	case '0' :
	  sprintf(bff2,CCNRM(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '1' :
	  sprintf(bff2,CCRED(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '2' :
	  sprintf(bff2,CCGRN(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '3' :
	  sprintf(bff2,CCYEL(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '4' :
	  sprintf(bff2,CCBLU(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '5' :
	  sprintf(bff2,CCMAG(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '6' :
	  sprintf(bff2,CCCYN(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '7' :
	  sprintf(bff2,CCWHT(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case '8' :
	  sprintf(bff2,CCBLK(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;		    
	case 'b' :
	  sprintf(bff2,CCBLD(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case 'u' :
	  sprintf(bff2,CCUND(vict, C_NRM));
	  include = FALSE;
	  i = bff2; break;
	case 'n':
	  if (GET_LEVEL(ch) < LEVEL_BUILDER)
	    sprintf(bff2,"%s",GET_NAME(ch));
	  else if (GET_INVIS_LEV(ch) > GET_LEVEL(vict))
	    {
	      for (j = bff;*j != '\0';j++) 
		if (*j != ' ')
		  start = FALSE; 
	      sprintf(bff2,(start ? "Someone": "someone"));
	    }
	  else
	    sprintf(bff2,"%s",GET_NAME(ch));
	  i = bff2; break;
	case 'o' :
	  if (ch->specials.fighting)
	    sprintf( bff2,"%s",PERS(ch->specials.fighting, ch));
	  else
	    sprintf( bff2,"%s","nobody");
	  i = bff2; break;
	case 'r' :
	  if( world[ch->in_room].name != NULL ){
	    if (IS_DARK(ch->in_room) &&
		!PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	      sprintf( bff2, "Pitch Black");
	    else
	      sprintf( bff2, "%s", world[ch->in_room].name );
	  }
	  else
	    sprintf( bff2, " " );
	  i = bff2; break;
	case 'l':
	  if (GET_LEVEL(ch) < 4)
	    sprintf( bff2, "newbie");
	  else if (GET_LEVEL(ch) < 20)
	    sprintf( bff2, "wily");
	  else if (GET_LEVEL(ch) < 50)
	    sprintf( bff2, "tough");
	  else if (GET_LEVEL(ch) < 100)
	    sprintf( bff2, "experienced");
	  else if (GET_LEVEL(ch) < 150)
	    sprintf( bff2, "hardened");
	  else if (GET_LEVEL(ch) < LEVEL_IMMORT)
	    sprintf( bff2, "veteran");	  
	  else if (GET_LEVEL(ch) >= LEVEL_IMPL)
	    sprintf( bff2, "%s","Implementor");
	  else if (GET_LEVEL(ch) >= LEVEL_ASS_IMPL)
	    sprintf( bff2, "%s","Overlord");
	  else if (GET_LEVEL(ch) >= LEVEL_GRGOD)
	    sprintf( bff2, "%s","Djinn");
	  else if (GET_LEVEL(ch) >= LEVEL_GOD)
	    sprintf( bff2, "%s","Daemon");
	  else if (GET_LEVEL(ch) >= LEVEL_MBUILDER)
	    sprintf( bff2, "%s","Bodhisattva");
	  else if (GET_LEVEL(ch) >= LEVEL_BUILDER)
	    sprintf( bff2, "%s","Builder");
	  else if (GET_LEVEL(ch) >= LEVEL_IMMORT)
	    sprintf( bff2, "%s","Avatar");
	  else
	    sprintf( bff2, "%s","bug");
	  i = bff2; break;
	case 'c' :
	  switch(GET_RACE(ch)){
	  case 0:
	    sprintf( bff2, "%s", "error");
	    break;
	  case 1:
	    sprintf( bff2, "%s", "Human");
	    break;
	  case 2:
	    sprintf( bff2, "%s", "Elf");
	    break;
	  case 3:
	    sprintf( bff2, "%s", "Halfling");
	    break;
	  case 4:
	    sprintf( bff2, "%s", "Giant");
	    break;
	  case 5:
	    sprintf( bff2, "%s", "Gnome");
	    break;
	  case 6:
	    sprintf( bff2, "%s", "Half-elf");
	    break;
	  case 7:
	    sprintf( bff2, "%s", "Ogier");
	    break;
	  case 8:
	    sprintf( bff2, "%s", "Dwarf");
	    break;
	  case 9:
	    sprintf( bff2, "%s", "Selkie");
	    break;
	  case 10:
	    sprintf( bff2, "%s", "Pixie");
	    break;
	  case 11:
	    sprintf( bff2, "%s", "Amarya");
	    break;			
	  case 12:
	    sprintf( bff2, "%s", "Troll");
	    break;			
	  }
	  i = bff2; break;
	case 'R' :
	  switch(GET_RACE(ch)){
	  case 0:
	    sprintf( bff2, "%s", "error");
	    break;
	  case 1:
	    sprintf( bff2, "%s", "Human");
	    break;
	  case 2:
	    sprintf( bff2, "%s", "Elven");
	    break;
	  case 3:
	    sprintf( bff2, "%s", "Halfling");
	    break;
	  case 4:
	    sprintf( bff2, "%s", "Gargantuan");
	    break;
	  case 5:
	    sprintf( bff2, "%s", "Gnomish");
	    break;
	  case 6:
	    sprintf( bff2, "%s", "Half-elven");
	    break;
	  case 7:
	    sprintf( bff2, "%s", "Ogier");
	    break;
	  case 8:
	    sprintf( bff2, "%s", "Dwarven");
	    break;
	  case 9:
	    sprintf( bff2, "%s", "Selkan");
	    break;
	  case 10:
	    sprintf( bff2, "%s", "Pixie");
	    break;
	  case 11:
	    sprintf( bff2, "%s", "Amaryan");
	    break;
	  case 12:
	    sprintf( bff2, "%s", "Trollan");
	    break;			
	    
	  }
	  i = bff2; break;
	case '%' :
	  sprintf( bff2, "%%" );
	  i = bff2; break;
	}
      if (len + (include ? strlen(bff2) : 0) > 79){
	sprintf(text,"%s%s",bff,CCNRM(vict, C_NRM));
	return;
      }
      ++string;
      while( (*point = *i) != '\0'){
	++point, ++i;
	if (include)
	  ++len;
      }
    }
  point = bff;
  for ( ; *point != '\0'; point++ )
    {
      if ( *point == '~' )
	*point = '-';
    }
  sprintf(text,"%s%s",bff,CCNRM(vict, C_NRM));
  
}

int	get_from_q(struct txt_q *queue, char *dest)
{
   struct txt_block *tmp;

   /* Q empty? */
   if (!queue->head)
      return(0);

   tmp = queue->head;
   strcpy(dest, queue->head->text);
   queue->head = queue->head->next;
   
   free(tmp->text);
   free(tmp);
   
   return(1);
}
void clear_queue(struct txt_q *queue)
{
  struct txt_block *tmp, *tmp2;
  for (tmp = queue->head; tmp; tmp = tmp2){
    tmp2 = tmp->next;
    free(tmp->text);
    free(tmp);
  }
  
}
void cmpact(char *input)
{
    int count=0, size;
    char j[24576],priv_buf[4096],*pp, *end;

    if ((size = strlen(input)) < 80)
	return; /* don't compact one liners */
    *j = '\0';
    end = input + strlen(input);

    bzero(priv_buf,4096);
    if (!(pp = (char *)  strtok(input,"\n"))) 
	return;
    strcpy(priv_buf,pp);
    while ((pp = strtok(0,"\n"))){
	if (strcmp(priv_buf,pp) != 0)
	  { 
	    if ((count > 0)){
	      if (!strcmp(priv_buf,"\r"))
		while (1 + count--)
		  strcat(j,"\r\n"); 
	      else
		sprintf(j,"%s(%i*)%s\n",j,++count,priv_buf); 
	      strcpy(priv_buf,pp);
	      count=0; 
	    } 
	    else { 
	      sprintf(j,"%s%s\n",j,priv_buf);
	      strcpy(priv_buf,pp);
	      count = 0;
	    }
	  }
	else {
	  strcpy(priv_buf,pp);
	  count++;
	}
    }
    sprintf(input,"%s", j); 	
    if (count) {
      if (!strcmp(priv_buf,"\r")) 
	  while (1 + count--)
	    strcat(input,"\r\n");
      else 
	sprintf(input,"%s(%i*)%s\n",input,++count,priv_buf);
    }
    else {
      if (!strcmp(priv_buf,"\r"))
	  strcat(input,"\r\n"); 	
      else {
	size = strlen(priv_buf);
	if (*(priv_buf + size - 1 ) == '\r')
	  sprintf(input,"%s%s\n",input,priv_buf);
	else
	  sprintf(input,"%s%s",input,priv_buf);
      }
    }
}



void	write_to_output(char *txt, struct descriptor_data *t)
{
   int size = 0;
   if (!txt)
       return;
   size = strlen(txt);
   
   if (t->character && PLR_FLAGGED(t->character, PLR_NEEDCRLF))
       size += 2;    


   /* if we're in the overflow state already, ignore this */
   if (t->bufptr < 0)
      return;

   /* if we have enough space, just write to buffer and that's it! */
   if (t->bufspace >= size) {
       if (t->character && PLR_FLAGGED(t->character, PLR_NEEDCRLF)){
	   sprintf(t->output+t->bufptr,"\r\n%s", txt);
       }
       else
	   strcpy(t->output+t->bufptr, txt);
      t->bufspace -= size;
      t->bufptr += size;
   }   else {      /* otherwise, try to switch to a large buffer */
      if (t->large_outbuf || ((size + strlen(t->output)) > LARGE_BUFSIZE)) {
	 /* we're already using large buffer, or even the large buffer
	    in't big enough -- switch to overflow state */
	 t->bufptr = -1;
	 buf_overflows++;
	 return;
      }

      buf_switches++;
      /* if the pool has a buffer in it, grab it */
      if (bufpool) {
	 t->large_outbuf = bufpool;
	 bufpool = bufpool->next;
      } else { /* else create one */
	 CREATE(t->large_outbuf, struct txt_block, 1);
	 CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
	 buf_largecount++;
      }
      strcpy(t->large_outbuf->text, t->output);
      t->output = t->large_outbuf->text;
      strcat(t->output, txt);
      t->bufspace = LARGE_BUFSIZE-1 - strlen(t->output);
      t->bufptr = strlen(t->output);
   }
   if (t->character && PLR_FLAGGED(t->character, PLR_NEEDCRLF))
       REMOVE_BIT(PLR_FLAGS(t->character), PLR_NEEDCRLF);
}



void	write_to_q(char *txt, struct txt_q *queue)
{
   struct txt_block *new;
   if (!txt)
       return;
   CREATE(new, struct txt_block, 1);
   CREATE(new->text, char, strlen(txt) + 1);

   strcpy(new->text, txt);
   /* Q empty? */
   if (!queue->head) {
      new->next = NULL;
      queue->head = queue->tail = new;
   } else {
      queue->tail->next = new;
      queue->tail = new;
      new->next = NULL;
   }
}




struct timeval timediff(struct timeval *a, struct timeval *b)
{
   struct timeval rslt, tmp;

   tmp = *a;

   if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0) {
      rslt.tv_usec += 1000000;
      --(tmp.tv_sec);
   }
   if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0) {
      rslt.tv_usec = 0;
      rslt.tv_sec = 0;
   }
   return(rslt);
}





/* Empty the queues before closing connection */
void	flush_queues(struct descriptor_data *d)
{
   if (d->large_outbuf) {
      d->large_outbuf->next = bufpool;
      bufpool = d->large_outbuf;
   }

   while (get_from_q(&d->input, buf2)) 
      ;
}





/* ******************************************************************
*  socket handling						    *
****************************************************************** */



int	init_socket(int port)
{
   int	s;
   int  opt = 1; 
   char	hostname[MAX_HOSTNAME+1];
   struct sockaddr_in sa;
   struct hostent *hp;

   bzero(&sa, sizeof(struct sockaddr_in ));
   gethostname(hostname, MAX_HOSTNAME);

   hp = gethostbyname(hostname);
   if (hp == NULL) {
      perror("gethostbyname");
      exit(1);
   }
   sa.sin_family = hp->h_addrtype;
   sa.sin_port	 = htons(port);

   if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("Init-socket");
      exit(1);
   }

   if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof (opt)) < 0) {
      perror ("setsockopt REUSEADDR");
      exit (1);
   }

#ifdef USE_LINGER 
   {
      struct linger ld;

      ld.l_onoff = 0;
      ld.l_linger = 1000;
      if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
         perror("setsockopt LINGER");
         exit(1);
      }
   }
#endif 

   if (bind(s, (struct sockaddr *) & sa, sizeof(sa)) < 0) {
      perror("bind");
      close(s);
      exit(1);
   }
   listen(s, 5);
   return(s);
}





int	new_connection(int s)
{
   struct sockaddr_in isa;
   size_t	i, t;

   i = sizeof(isa);

   if ((t = accept(s, (struct sockaddr *)(&isa), &i)) < 0) {
      perror("Accept");
      return(-1);
   }
   nonblock(t);
   return(t);
}



char *format_inet_addr( char *dest, long addr )
{
    dest += sprintf( dest, "%ld.%ld.%ld.%ld", 
                (addr & 0xFF000000) >> 24,
                (addr & 0x00FF0000) >> 16,
                (addr & 0x0000FF00) >> 8,
                (addr & 0x000000FF) );
    return( dest );
}


int	new_descriptor(int s)
{
   int	desc;
   size_t size;
   char *pbuf;
   struct descriptor_data *newd, *point, *next_point;
   int	sockets_connected, sockets_playing, i;
   struct sockaddr_in sock;
   struct hostent *from, *p;
   char buf[4097];

   if ((desc = new_connection(s)) < 0)
      return (-1);

   sockets_connected = sockets_playing = 0;

   for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      sockets_connected++;
      if (!point->connected)
	 sockets_playing++;
   }

   /* if ((maxdesc + 1) >= avail_descs) 
      if (sockets_connected >= avail_descs) {
      write_to_descriptor(desc, "Sorry, Archipelago is full right now... try again later!\r\n");
      close(desc);
      return(0);
      } else */
   if (desc > maxdesc)
     maxdesc = desc;
   
   CREATE(newd, struct descriptor_data, 1);
   
   /* find info */
   size = sizeof(sock);
   if (nameserver_is_slow ||!(from = gethostbyaddr((char *)&sock.sin_addr,
      sizeof(sock.sin_addr), PF_INET))) {
         if (!nameserver_is_slow)
	    perror("gethostbyaddr");
         i = sock.sin_addr.s_addr;
	 sprintf(newd->host, "%d.%d.%d.%d", (i & 0x000000FF),
		 (i & 0x0000FF00) >> 8, (i & 0x00FF0000) >> 16,
		 (i & 0xFF000000) >> 24 );
   }
   else {
      strncpy(newd->host, from->h_name, 49);
      *(newd->host + 49) = '\0';
   }
   if (slave_socket != -1) {
     buf[0] = 'i';
     pbuf = format_inet_addr(buf+1, ntohl(sock.sin_addr.s_addr));
     pbuf += sprintf(pbuf, ",%d,%d\n", ntohs(sock.sin_port), port);
     if (write(slave_socket, buf, pbuf - buf + 1) != (pbuf - buf + 1)) {
       logg("[SLAVE] loosing slave on write:");
       close(slave_socket);
       slave_socket = -1;
     }
   }
   
   if (isbanned(newd->host) == BAN_ALL) {
      close(desc);
      /*      sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
      mudlog(buf2, CMP, LEVEL_GOD, TRUE);*/
      free(newd);
      return(0);
   }

/*  Uncomment this if you want new connections logged.  It's usually not
    necessary, and just adds a lot of unnecessary bulk to the logs.
    */
   newd->port = ntohs(sock.sin_port);
   sprintf(buf2, "New connection from [%s], port: %d", newd->host,
	   newd->port);
   mudlog(buf2, CMP, LEVEL_IMPL, TRUE);
   /*   logg(buf2);*/

   /* init desc data */
   newd->addr = ntohl(sock.sin_addr.s_addr);
   newd->user_id[0] = '\0';
   newd->descriptor = desc;
   newd->connected = CON_QCOLOR;
   newd->bad_pws = 0;
   newd->pos = -1;
   newd->wait = 1;
   newd->color = TRUE;
   newd->prompt_mode = 0;
   *newd->buf = '\0';
   newd->str = 0;
   newd->replace = 0;
   newd->with = 0;   
   newd->showstr_head = 0;
   newd->showstr_point = 0;
   *newd->last_input = '\0';
   newd->output = newd->small_outbuf;
   *(newd->output) = '\0';
   newd->bufspace = SMALL_BUFSIZE-1;
   newd->large_outbuf = NULL;
   newd->input.head = NULL;
   newd->next = descriptor_list;
   newd->character = 0;
   newd->original = 0;
   newd->snoop.snooping = 0;
   newd->snoop.snoop_by = 0;
   newd->login_time = time(0);

   if (++last_desc == 1000)
      last_desc = 1;
   newd->desc_num = last_desc;
   newd->mob_edit = 0;
   newd->medit_mode = 0;
   newd->obj_edit = 0;
   newd->oedit_mode = 0;
   newd->room_edit = 0;
   newd->redit_mode = 0;      
   newd->zone_edit = 0;
   newd->zedit_mode = 0;
   newd->shop_edit = 0;
   /* newd->sedit_mode = 0; --- This isn't defined!  */
   
   /* prepend to list */

   descriptor_list = newd;

   SEND_TO_Q("***Colour [Y/n]? (default is yes)\r\n*Note colour is not recommended if your connection is slow/laggy*: ", newd); 
   return(0);
}

static int get_slave_result(void)
{
  char *p, buf[4097], token[4096], os[4096], userid[4096];
  int octet[4], local_port, remote_port, len;
  struct descriptor_data *d;
  long addr;
  
  len = read(slave_socket, buf, 4096);
  if (len < 0) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
      return(-1);
    logg("[SLAVE] loosing slave on read");
    close(slave_socket);
    slave_socket = -1;
    return(-1);
  }
  else if (len==0)
    return(-1);
  buf[len] = 0;
  if (sscanf(buf + 1, "%d.%d.%d.%d %d , %d : %[^: ] : %[^: ] : %s",
	     &octet[0], &octet[1], &octet[2], &octet[3],
	     &remote_port, &local_port, token, os, userid) != 9) {
    if((sscanf(buf + 1, "%d.%d.%d.%d %d , %d : %[^: ] : %s",
	     &octet[0], &octet[1], &octet[2], &octet[3],
	     &remote_port, &local_port, token, userid) != 8) ||
       strncmp(token, "ERROR", 5)) {
      logg("[SLAVE] invalid");
    }
    return(0);
  }
  if (local_port != port) {
    logg ("[SLAVE] local port != game port");
    return(0);
  }
  addr = (octet[0] << 24) + (octet[1] << 16) + (octet[2] << 8) + octet[3];

  for (d = descriptor_list; d; d = d->next) {
    if (d->port != remote_port ) continue;
    if (d->addr != addr) continue;
    strncpy(d->user_id, userid, (MAX_USER_ID - 1));
    d->user_id[MAX_USER_ID-1] = '\0';
    return(0);
  }
    
}


int	process_output(struct descriptor_data *t)
{
   static char	i[LARGE_BUFSIZE + 20];


   /* start writing at the 2nd space so we can prepend "% " for snoop */
   if (t->character &&
       !t->connected &&
       !(!IS_NPC(t->character) && 
	 (PRF_FLAGGED(t->character, PRF_COMPACT)))){
      strcpy(i+2, "\r\n"); 
      strcpy(i+2, t->output);}
   else
      strcpy(i+2, t->output);
   
   if (t->bufptr < 0)
      strcat(i+2, "**OVERFLOW**");
   if (t->character &&
       !t->connected &&
       !(!IS_NPC(t->character) && 
	 (PRF_FLAGGED(t->character, PRF_COMPACT))))
     strcat(i+2, "\r\n");
   if (write_to_descriptor(t->descriptor, i+2) < 0) 
      return -1;

   if (t->snoop.snoop_by) {
      i[0] = '%';
      i[1] = ' ';
      SEND_TO_Q(i, t->snoop.snoop_by->desc);
   }

   /* if we were using a large buffer, put the large buffer on the buffer
      pool and switch back to the small one */
   if (t->large_outbuf) {
      t->large_outbuf->next = bufpool;
      bufpool = t->large_outbuf;
      t->large_outbuf = NULL;
      t->output = t->small_outbuf;
   }

   /* reset total bufspace back to that of a small buffer */
   t->bufspace = SMALL_BUFSIZE-1;
   t->bufptr = 0;
   *(t->output) = '\0';

   return 1;
}



int	write_to_descriptor(int desc, char *txt)
{
   int	sofar, thisround, total;

   if (!txt)
       return(0);
   
   cmpact(txt);
   
   total = strlen(txt);
   sofar = 0;

   do {
       thisround = write(desc, txt + sofar, total - sofar);
       if (thisround < 0) {
	   perror("Write to socket");
	   return(-1);
       }
       sofar += thisround;
   } while (sofar < total);
   return(0);
}





int	process_input(struct descriptor_data *t)
{
   int	sofar, thisround, begin, squelch, i, k, flag, failed_subst = 0;
   char	tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];

   sofar = 0;
   flag = 0;
   begin = strlen(t->buf);

   /* Read in some stuff */
   do {
      if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
          MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0)
	 sofar += thisround;
      else if (thisround < 0)
	 if (errno != EWOULDBLOCK) {
	    perror("Read1 - ERROR");
	    return(-1);
	 } 
	 else
	    break;
      else {
	 logg("EOF encountered on socket read.");
	 return(-1);
      }
   } while (!ISNEWL(*(t->buf + begin + sofar - 1)));

   *(t->buf + begin + sofar) = 0;

   /* if no newline is contained in input, return without proc'ing */
   for (i = begin; !ISNEWL(*(t->buf + i)); i++)
      if (!*(t->buf + i))
	 return(0);

   /* input contains 1 or more newlines; process the stuff */
   for (i = 0, k = 0; *(t->buf + i); ) {
      if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
	 if (*(t->buf + i) == '\b')	 /* backspace */
	    if (k) {  /* more than one char ? */
	       if (*(tmp + --k) == '$')
		  k--;
	       i++;
	    } 
	    else
	       i++;  /* no or just one char.. Skip backsp */
	 else if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
	    /* trans char, double for '$' (printf)	*/
	    if ((*(tmp + k) = *(t->buf + i)) == '$' && !t->str)
	       *(tmp + ++k) = '$';
	    k++;
	    i++;
	 } 
	 else
	    i++;
      else {
	 *(tmp + k) = 0;
	 if (*tmp == '!')
	    strcpy(tmp, t->last_input);
	 else if (*tmp == '^') {
	    if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
	       strcpy(t->last_input, tmp);
	 } else
	    strcpy(t->last_input, tmp);

	 if (!failed_subst)
	   {
	     if (!strncmp(tmp, "abort",5) || !strncmp(tmp, "flee",4))
	       while(get_from_q(&t->input, buf2))
		 ;
	     write_to_q(tmp, &t->input);
	   }
	 
	 if (t->snoop.snoop_by) {
	    SEND_TO_Q("% ", t->snoop.snoop_by->desc);
	    SEND_TO_Q(tmp, t->snoop.snoop_by->desc);
	    SEND_TO_Q("\r\n", t->snoop.snoop_by->desc);
	 }

	 if (flag) {
	    sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
	    if (write_to_descriptor(t->descriptor, buffer) < 0)
	       return(-1);

	    /* skip the rest of the line */
	    for (; !ISNEWL(*(t->buf + i)); i++)
	       ;
	 }

	 /* find end of entry */
	 for (; ISNEWL(*(t->buf + i)); i++)
	    ;

	 /* squelch the entry from the buffer */
	 for (squelch = 0; ; squelch++)
	    if ((*(t->buf + squelch) = *(t->buf + i + squelch)) == '\0')
	       break;
	 k = 0;
	 i = 0;
      }
   }
   return(1);
}



int	perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
   char new[MAX_INPUT_LENGTH+5];

   char *first, *second, *strpos;

   first = subst+1;
   if (!(second = strchr(first, '^'))) {
      SEND_TO_Q("Invalid substitution.\r\n", t);
      return 1;
   }

   *(second++) = '\0';

   if (!(strpos = strstr(orig, first))) {
      SEND_TO_Q("Invalid substitution.\r\n", t);
      return 1;
   }

   strncpy(new, orig, (strpos-orig));
   new[(strpos-orig)] = '\0';
   strcat(new, second);
   if (((strpos-orig) + strlen(first)) < strlen(orig))
      strcat(new, strpos+strlen(first));
   strcpy(subst, new);

   return 0;
}



void	close_sockets(int s)
{
   logg("Killing Slave");
   if (slave_socket != -1)
     kill( slave_pid, SIGKILL);
   
   logg("Closing all sockets.");   
   while (descriptor_list)
      close_socket(descriptor_list);

   close(s);
}




void	close_socket(struct descriptor_data *d)
{
  struct descriptor_data *tmp;
  char	buf[100];
  
  close(d->descriptor);
  flush_queues(d);
  if (d->descriptor == maxdesc)
    --maxdesc;
  
  /* Forget snooping */
  if (d->snoop.snooping)
    d->snoop.snooping->desc->snoop.snoop_by = 0;

  if (d->snoop.snoop_by) {
    send_to_char("Your victim is no longer among us.\r\n", d->snoop.snoop_by);
    d->snoop.snoop_by->desc->snoop.snooping = 0;
  }
  
  if (d->character)
    if (d->connected == CON_PLYNG) {
      save_char(d->character, NOWHERE);
      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
      mudlog(buf, NRM, MAX(LEVEL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
      d->character->desc = 0;
    }
    else {
      sprintf(buf, "Losing player: %s.", GET_NAME(d->character));
      mudlog(buf, CMP, MAX(LEVEL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
      free_char(d->character);
    }
  else
    mudlog("Losing descriptor without char.", CMP, LEVEL_BUILDER, TRUE);
  
  if (next_to_process == d)  /* to avoid crashing the process loop */
    next_to_process = next_to_process->next;
  
  if (d == descriptor_list) /* this is the head of the list */
    descriptor_list = descriptor_list->next;
  else {  /* This is somewhere inside the list */
    /* Locate the previous element */
    for (tmp = descriptor_list; (tmp->next != d) && tmp; tmp = tmp->next)
      ;
    tmp->next = d->next;
  }
  
  if (d->showstr_head)
    free(d->showstr_head);
  if (d->name)
    free(d->name);
  if (d->replace)
    free(d->replace);
  if (d->with)
     free(d->with);   
  free(d);
}


#if defined(SVR4) || defined(LINUX)

void	nonblock(int s)
{
   int	flags;
   flags = fcntl(s, F_GETFL);
   flags |= O_NONBLOCK;
   if (fcntl(s, F_SETFL, flags) < 0) {
      perror("Fatal error executing nonblock (comm.c)");
      exit(1);
   }
}

#else

void	nonblock(int s)
{
   if (fcntl(s, F_SETFL, FNDELAY) == -1) {
      perror("Fatal error executing nonblock (comm.c)");
      exit(1);
   }
}

#endif



/* ****************************************************************
*	Public routines for system-to-player-communication	  *
*******************************************************************/



void	send_to_char(char *messg, struct char_data *ch)
{
    if (ch->desc && messg)
	SEND_TO_Q(messg, ch->desc);
}




void	send_to_all(char *messg)
{
    struct descriptor_data *i;
    if (messg)
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected)
		SEND_TO_Q(messg, i);
}


void	send_to_outdoor(char *messg)
{
   struct descriptor_data *i;

   if (messg)
      for (i = descriptor_list; i; i = i->next)
	 if (!i->connected)
	    if (OUTSIDE(i->character) &&
		(world[i->character->in_room].sector_type != SECT_UNDER_WATER)
		&& !PLR_FLAGGED(i->character, PLR_WRITING) &&
		!PLR_FLAGGED(i->character, PLR_BUILDING) && AWAKE(i->character))
	       SEND_TO_Q(messg, i);
}


void	send_to_except(char *messg, struct char_data *ch)
{
   struct descriptor_data *i;

   if (messg)
      for (i = descriptor_list; i; i = i->next)
	 if (ch->desc != i && !i->connected
	     && !PLR_FLAGGED(i->character, PLR_WRITING) &&
		!PLR_FLAGGED(i->character, PLR_BUILDING))
	    SEND_TO_Q(messg, i);
}



void	send_to_room(char *messg, int room, bool virtual)
{
   struct char_data *i;

   if (virtual)
       room = real_room(room);
   
   if (messg)
      for (i = world[room].people; i; i = i->next_in_room)
	  if (i->desc && !PLR_FLAGGED(i, PLR_WRITING) &&
		!PLR_FLAGGED(i, PLR_BUILDING))
	    SEND_TO_Q(messg, i->desc);
}




void	send_to_room_except(char *messg, int room, struct char_data *ch, bool virtual)
{
   struct char_data *i;

   if (virtual)
       room = real_room(room);   
   
   if (messg)
      for (i = world[room].people; i; i = i->next_in_room)
	 if (i != ch && i->desc && !PLR_FLAGGED(i, PLR_WRITING) &&
		!PLR_FLAGGED(i, PLR_BUILDING))
	    SEND_TO_Q(messg, i->desc);
}


void	send_to_room_except_two
(char *messg, int room, struct char_data *ch1, struct char_data *ch2, bool virtual)
{
   struct char_data *i;
   
   if (virtual)
       room = real_room(room);
   if (messg)
      for (i = world[room].people; i; i = i->next_in_room)
	 if (i != ch1 && i != ch2 && i->desc
	     && !PLR_FLAGGED(i, PLR_WRITING) &&
		!PLR_FLAGGED(i, PLR_BUILDING))
	    SEND_TO_Q(messg, i->desc);
}



/* higher-level communication */

void	act(char *str, int hide_invisible, struct char_data *ch,
struct obj_data *obj, void *vict_obj, int type)
{
   register char	*strp, *point, *i;
   struct char_data *to, *ignore=0;
   static char	buf[MAX_STRING_LENGTH];

   if (!str || !*str)
      return;
   if (!ch && !obj && !vict_obj)
       return;
   
   if (type == TO_VICT)
       to = (struct char_data *) vict_obj;
   else if (type == TO_CHAR)
       to = ch;
   else if (type == TO_ROOM){
     if (ch){
       if (ch->in_room >= 0)
	 to = world[ch->in_room].people;
       else
	 return;
     }
     else if (obj){
       if (obj->in_room >=0)
	 to = world[obj->in_room].people;
       else if (obj->worn_by)
	 to = world[obj->worn_by->in_room].people;
       else if (obj->carried_by)
	 to = world[obj->carried_by->in_room].people;
       else
	 return;
     }
   }
   if (vict_obj && type == TO_NOTVICT){
     ignore = (struct char_data *) vict_obj;
     if (ignore->in_room >= 0)
       to = world[ignore->in_room].people;
     else
       return;
   }
   if (ch && type == TO_NOTCHAR){
       to = world[ch->in_room].people;
       ignore = ch;
   }
   
   if (!to)
       return;
   for (; to; to = to->next_in_room) {
      if (to && to->desc && ((to != ch) || (type == TO_CHAR)) &&  
          ((ch && CAN_SEE(to, ch)) || !hide_invisible || (type == TO_VICT)) &&
	  AWAKE(to) && !PLR_FLAGGED(to, PLR_WRITING) &&
	  !PLR_FLAGGED(to, PLR_BUILDING) &&  !(to == ignore))
	  {
	      for (strp = str, point = buf; ; )
		  if (*strp == '$') {
		      switch (*(++strp)) {
		      case 'n':
			  i = PERS(ch, to);
			  break;
		      case 'N':
			  i = PERS((struct char_data *) vict_obj, to);
			  break;
		      case 'm':
			  i = HMHR(ch);
			  break;
		      case 'M':
			  i = HMHR((struct char_data *) vict_obj);
			  break;
		      case 's':
			  i = HSHR(ch);
			  break;
		      case 'S':
			  i = HSHR((struct char_data *) vict_obj);
			  break;
		      case 'e':
			  i = HSSH(ch);
			  break;
		      case 'E':
			  i = HSSH((struct char_data *) vict_obj);
			  break;
		      case 'o':
			  i = OBJN(obj, to);
			  break;
		      case 'O':
			  i = OBJN((struct obj_data *) vict_obj, to);
			  break;
		      case 'p':
			  i = OBJS(obj, to);
			  break;
		      case 'P':
			  i = OBJS((struct obj_data *) vict_obj, to);
			  break;
		      case 'a':
			  i = SANA(obj);
			  break;
		      case 'A':
			  i = SANA((struct obj_data *) vict_obj);
			  break;
		      case 'T':
			  i = (char *) vict_obj;
			  break;
		      case 'F':
			  i = fname((char *) vict_obj);
			  if (!(strcmp(i,"")))
			      strcpy(i,"hidden passage");
			  break;
		      case '$':
			  i = "$";
			  break;
		      default:
			  logg("SYSERR: Illegal $-code to act():");
			  strcpy(buf1, "SYSERR: ");
			  strcat(buf1, str);
			  logg(buf1);
			  break;
		      }
		      while ((*point = *(i++)))
			  ++point;
		      ++strp;
		  }
		  else if (!(*(point++) = *(strp++)))
		      break;
	      
	      *(--point) = '\n';
	      *(++point) = '\r';
	      *(++point) = '\0';
	      SEND_TO_Q(CAP(buf), to->desc);
	  }
      if ((type == TO_VICT) || (type == TO_CHAR))
	  return;
   }
}


void process_events(int pulse)
{
  ACMD(do_look);
  ACMD(do_wear);
  ACMD(do_get);
  struct event_type *tmp,*tmp2;
  struct char_data *subj, *vict, *next_vict;
  struct obj_data *obj_vict, *obj_subj;
  struct room_data *rm_subj;
  int spl, to_room, n = 0;
  struct follow_type *k;
  char buf_loc[256];

  tmp = events[pulse];
  while (tmp)
    {
      switch(tmp->event){
      case EVENT_IGNORE:
	break;
      case EVENT_REBOOT:
	subj = (struct char_data *) tmp->subject;
	tmp->info1--;
	if (tmp->info1 < 1) {
	  if (tmp->info3 > 0) {
	    circle_shutdown = 1;
	    break;
	  }
	  else {
	    do_force(subj, "all save",0,0);
	    sprintf(buf,"\007\007(GC) Shutdown by %s, now.\r\n",
		    GET_NAME(subj),tmp->info1-1);
	    send_to_all(buf);
	    logg(buf);
	    if (tmp->info2 > 0)
	      send_to_all("Rebooting.. come back in a minute or two.\r\n");
	    add_event(1, EVENT_REBOOT, tmp->info1,tmp->info2,1,0,"",subj,0);
	  }
	}
	else {
	  sprintf(buf,"\007\007(GC) %s by %s, in %d %s.\r\n",
		  (tmp->info2 ? "Reboot" : "Shutdown"),
		  GET_NAME(subj),tmp->info1,
		  ((tmp->info1 == 1) ? "minute" : "minutes"));
	  send_to_all(buf);
	  logg(buf);
	  add_event(250, EVENT_REBOOT, tmp->info1,tmp->info2,0,0,"",subj,0);
	}
	break;
      case EVENT_ATTACK:
	subj = (struct char_data *) tmp->subject;
	vict = (struct char_data *) tmp->target;
	if (CAN_SPEAK(subj) && (GET_INT(subj) < 15))
	  act("$n screams, '$N! I'm gonna kill you'",FALSE,subj,0,vict,TO_ROOM);
	else if (CAN_SPEAK(subj) && (GET_INT(subj) >= 15))
	  act("$n says, '$N! Time to die!'",FALSE,subj,0,vict,TO_ROOM);
	else if (!CAN_SPEAK(subj))
	  act("$n roars! and attacks $N!",FALSE,subj,0,vict, TO_NOTVICT);
	if (vict->in_room == subj->in_room)
	  hit(subj, vict,0);
	break;
      case EVENT_OBJTIMER:
	obj_subj = (struct obj_data *) tmp->subject;
	subj = obj_subj->worn_by;
	if (is_goditem(obj_subj))
	  break;
	if (obj_subj->obj_flags2.aff_timer == 1){
	  if (subj){
	    affect_modify(subj, APPLY_NONE,0,
			  obj_subj->obj_flags2.bitvector_aff,
			  FALSE);
	    obj_subj->obj_flags2.bitvector_aff = 0;
	    SET_BIT(PLR_FLAGS(subj), PLR_CRASH);
			    
	  }
	  else
	    obj_subj->obj_flags2.bitvector_aff = 0;
	  obj_subj->obj_flags2.aff_timer = 0;
	  if (obj_subj->obj_flags2.no_use_timer > 0)
	    obj_subj->obj_flags2.no_use_timer--;	
	}
	else{
	  if (obj_subj->obj_flags2.aff_timer > 0)
	    obj_subj->obj_flags2.aff_timer--;
	  switch (GET_ITEM_TYPE(obj_subj))
	    {
	    case ITEM_WAND:
	      obj_subj->obj_flags2.no_use_timer -= 3;
	      break;
	    case ITEM_STAFF:
	      obj_subj->obj_flags2.no_use_timer -= 2;
	      break;
	    default:
	      obj_subj->obj_flags2.no_use_timer--;
	      break;
	    }
	  obj_subj->obj_flags2.no_use_timer
	    =MAX(0,obj_subj->obj_flags2.no_use_timer);
	}
	if (obj_subj->obj_flags2.no_use_timer > 0)
	  if (GET_ITEM_TYPE(obj_subj) != ITEM_WAND &&
	      GET_ITEM_TYPE(obj_subj) != ITEM_STAFF )
	    add_event(-1,EVENT_OBJTIMER,0,0,0,0,0,obj_subj,0);
	  else 
	    add_event(2,EVENT_OBJTIMER,0,0,0,0,0,obj_subj,0);
	break;
      case EVENT_LOOT:
	subj = (struct char_data *) tmp->subject;
	obj_vict = (struct obj_data *) tmp->target;
	if (obj_vict && (GET_ITEM_TYPE(obj_vict) == ITEM_CONTAINER)
	    && obj_vict->obj_flags.value[3] < 0
	    && obj_vict->contains){
	  do_get(subj,"all corpse", 0,0);
	  if (GET_INT(subj) > 10)
	    do_wear(subj,"all",0,0);
	}
	break;
      case EVENT_LEAVE:
	if (!tmp->subject)
	  break;
	subj = (struct char_data *) tmp->subject;

	if (subj->specials.fighting)
	  break;
		    
	if (!IS_AFFECTED(subj, AFF_SNEAK) && !(subj->specials.mount)
	    && !(subj->specials.rider) && !(subj->specials.carried_by)){
	  if (IS_CLIMB(subj,tmp->info1)
	      && !IS_AFFECTED(subj,AFF_FLY))
	    {
	      sprintf(buf2, "$n starts to climb %s.", dirs[tmp->info1]);
	      act(buf2, TRUE, subj, 0, 0, TO_ROOM);
	      sprintf(buf2, "You start to climb.");
	      act(buf2, TRUE, subj, 0, 0, TO_CHAR);
	    }
	  else if (world[subj->in_room].sector_type
		   == SECT_UNDER_WATER
		   || world[subj->in_room].sector_type
		   == SECT_WATER_SWIM
		   || (world[subj->in_room].sector_type
		       == SECT_WATER_NOSWIM && !tmp->info2)) {
	    sprintf(buf2, "$n swims %s.",dirs[tmp->info1]);
	    act(buf2, TRUE, subj,0,0,TO_ROOM);
	    sprintf(buf2, "You swim %s.",dirs[tmp->info1]);
	    act(buf2, TRUE, subj,0,0,TO_CHAR);
	  }
	  else if (IS_AFFECTED(subj,AFF_FLY)) {
	    sprintf(buf2, "$n flies %s.",dirs[tmp->info1]);
	    act(buf2, TRUE, subj,0,0,TO_ROOM);
	    sprintf(buf2, "You fly %s.",dirs[tmp->info1]);
	    act(buf2, TRUE, subj,0,0,TO_CHAR);
	  }
	  else {
	    sprintf(buf2, "$n sets off %s.",dirs[tmp->info1]);
	    act(buf2, TRUE, subj,0,0,TO_ROOM);
	    sprintf(buf2, "You set off %s.",dirs[tmp->info1]);
	    act(buf2, TRUE, subj,0,0,TO_CHAR);
	  }
	}
	else if (subj->specials.mount)
	  {
	    if (world[subj->in_room].sector_type
		== SECT_UNDER_WATER
		|| world[subj->in_room].sector_type
		== SECT_WATER_SWIM
		|| (world[subj->in_room].sector_type
		    == SECT_WATER_NOSWIM && !tmp->info2)) {
	      sprintf(buf2, "$n swims %s on $N.",dirs[tmp->info1]);
	      act(buf2, TRUE, subj,0,subj->specials.mount,TO_ROOM);
	      sprintf(buf2, "You swim %s on $N.",dirs[tmp->info1]);
	      act(buf2, TRUE, subj,0,subj->specials.mount,TO_CHAR);
	    }
	    else if (IS_AFFECTED(subj->specials.mount,AFF_FLY)) {
	      sprintf(buf2, "$n flies %s on $N.",dirs[tmp->info1]);
	      act(buf2, TRUE, subj,0,subj->specials.mount,TO_ROOM);
	      sprintf(buf2, "You fly %s on $N.",dirs[tmp->info1]);
	      act(buf2, TRUE, subj,0,subj->specials.mount,TO_CHAR);
	    }
	    else {
	      sprintf(buf2, "$n rides off %s on $N.",dirs[tmp->info1]);
	      act(buf2, TRUE, subj,0,subj->specials.mount,TO_ROOM);
	      sprintf(buf2, "You ride off %s on $N.",dirs[tmp->info1]);
	      act(buf2, TRUE, subj,0,subj->specials.mount,TO_CHAR);
	    }
	  }
	break;
      case EVENT_CART_LEAVE:
	if (!tmp->subject)
	  break;
	obj_subj = (struct obj_data *) tmp->subject;
	bzero(buf_loc,256);
	for (k = obj_subj->pulled_by;k;k = k->next){
	  strcat(buf_loc,
		 first_name(k->follower->player.name ? k->follower->player.name :
			    k->follower->player.short_descr));
	  strcat(buf_loc, " ");
	  n++;}
	if (n > 1)
	  sprintf(buf2,"The %s pull $p %s.",pluralise_string(buf_loc),dirs[tmp->info1]);
	else{	
	  half_chop(buf_loc,buf,buf1);
	  sprintf(buf2,"The %s pulls $p %s.",buf,dirs[tmp->info1]);}
	act(buf2,TRUE,world[obj_subj->in_room].people,obj_subj,0,TO_ROOM);
	act(buf2,TRUE,world[obj_subj->in_room].people,obj_subj,0,TO_CHAR);
	to_room = -1;
	if (GET_ITEM_TYPE(obj_subj) == ITEM_CONTAINER)
	  to_room = real_room(obj_subj->obj_flags.value[3]);
	if (world[to_room].people)
	  if (!IS_SET(obj_subj->obj_flags.value[1],CONT_CLOSED)){
	    if (to_room > 0 && to_room <= top_of_world){
	      sprintf(buf1,"The %s moves off %s.",
		      first_name(obj_subj->name)
		      ,dirs[tmp->info1]);
	      act(buf1,TRUE,world[to_room].people
		  ,obj_subj,0
		  ,TO_CHAR);
	      act(buf1,TRUE,world[to_room].people
		  ,obj_subj,0
		  ,TO_ROOM);
	    }
	  }
	  else if (to_room > 0 && to_room <= top_of_world){
	    sprintf(buf1,"The %s rattles and bumps about."
		    ,first_name(obj_subj->name));
	    act(buf1,TRUE,world[to_room].people
		,obj_subj,0
		,TO_CHAR);
	    act(buf1,TRUE,world[to_room].people
		,obj_subj,0
		,TO_ROOM);				
	  }
	break;
      case EVENT_CART_ARRIVE:
	if (!tmp->subject)
	  break;
	obj_subj = (struct obj_data *) tmp->subject;
	if (!obj_subj->pulled_by)
	  break;
	obj_from_room(obj_subj);
	obj_to_room(obj_subj, tmp->info2, TRUE);
	sprintf(buf2, "$p arrives from %s.",revdirs[tmp->info1]);
	act(buf2,FALSE, 0,obj_subj,0,TO_ROOM);
	break;
      case EVENT_ARRIVE:
	if (!tmp->subject)
	  break;
	subj = (struct char_data *) tmp->subject;
	if (subj->specials.fighting)
	  break;
	if (subj->desc)
	  subj->desc->prompt_mode =1;
	if (subj->specials.cart
	    && (subj->specials.cart->in_room == subj->in_room)){
	  obj_from_room(subj->specials.cart);
	  obj_to_room(subj->specials.cart, tmp->info2, TRUE);
	}
	char_from_room(subj);
	char_to_room(subj,tmp->info2, TRUE);
	if (subj->specials.carrying){
	  char_from_room(subj->specials.carrying);
	  char_to_room(subj->specials.carrying,tmp->info2, TRUE);
	}
	if (!IS_AFFECTED(subj, AFF_SNEAK) && !subj->specials.mount
	    && !subj->specials.rider && !subj->specials.carried_by) {
	  sprintf(buf2, "$n arrives from %s.",revdirs[tmp->info1]);
	  act(buf2, TRUE, subj,0,0,TO_ROOM);}
	else if (subj->specials.mount) {
	  sprintf(buf2, "$n rides in from %s on $N.",revdirs[tmp->info1]);
	  act(buf2, TRUE, subj,0,subj->specials.mount,TO_ROOM);}
	do_look(subj,"",0,0);
	if (IS_SET(world[subj->in_room].room_flags, DEATH) && 
	    GET_LEVEL(subj) < LEVEL_BUILDER) {
	  log_death_trap(subj);
	  death_cry(subj);
	  extract_char(subj, TRUE);
	  break;
	}
	for (vict = world[subj->in_room].people;
	     vict;vict = vict->next_in_room){
	  if (IS_MOB(vict) && mob_index[vict->nr].func)
	    (*mob_index[vict->nr].func)(subj,vict,SPEC_ARRIVE,"");
	}
	break;
      case EVENT_TELEPORT:
	rm_subj = (struct room_data *) tmp->subject;
	vict    = (struct char_data *) tmp->target;
	if (!(rm_subj) || !(vict)){
	  tmp->event = EVENT_IGNORE;
	  break;}
	if (rm_subj->number != world[vict->in_room].number){
	  tmp->event = EVENT_IGNORE;
	  break;}
	if (rm_subj->tele_mesg1)
	  act(rm_subj->tele_mesg1,FALSE,vict,0,0,TO_CHAR);
	if (real_room(rm_subj->tele_to_room) != vict->in_room){
	  if (rm_subj->tele_mesg2)
	    act(rm_subj->tele_mesg2,FALSE,vict,0,0,TO_ROOM);
	  char_from_room(vict);
	  char_to_room(vict,rm_subj->tele_to_room, TRUE);
	  do_look(vict,"",0,0);
	  if (rm_subj->tele_mesg3)   	    
	    act(rm_subj->tele_mesg3,FALSE,vict,0,0,TO_ROOM);}
	tmp->event = EVENT_IGNORE;
	break;
      case EVENT_COMBAT:
	if (tmp->subject)
	  subj = (struct char_data *) tmp->subject;
	else
	  break;
	if (GET_POS(subj) <=  POSITION_SLEEPING)
	  break;
	vict = (struct char_data *) tmp->target;
	if (vict->in_room != subj->in_room)
	  break;
	if (vict)
	  if (affected_by_spell(vict, SPELL_ENCASE_IN_ICE)){
	    if (!number(0,3) && (tmp->info1 > 5)){
	      act("$n's blow shatters the ice encasing $N!",
		  TRUE,subj,0,vict,TO_NOTVICT);
	      act("$n's blow shatters the ice encasing you!",
		  TRUE,subj,0,vict,TO_VICT);
	      act("Your blow shatters the ice encasing $N.",
		  TRUE,subj,0,vict,TO_CHAR);
	      damage(subj,vict,tmp->info1/4,tmp->info2,tmp->info3,0);
	      affect_from_char(vict, SPELL_ENCASE_IN_ICE);
	    }
	    else {
	      act("$n's blow glances off the ice encasing $N!",
		  TRUE,subj,0,vict,TO_NOTVICT);
	      act("$n's blow glances off the ice encasing you!",
		  TRUE,subj,0,vict,TO_VICT);
	      act("Your blow glances off the ice encasing $N.",
		  TRUE,subj,0,vict,TO_CHAR);
	    }
	  }
	  else
	    damage(subj, vict, tmp->info1, tmp->info2, tmp->info3,0);

	tmp->event = EVENT_IGNORE;
	break;
      case EVENT_ROOM_DAMAGE:
	{
	  int dam, damtype;

	  if (tmp->info3 == SPELL_BREATH_OF_VULCAN)
	    damtype = SKILL_IGNEM;
	  else
	    damtype = tmp->info3;
	  if (real_room(tmp->info2)){
	    rm_subj = world + real_room(tmp->info2);
	    if ((tmp->info3 < SPELL_FROST_BREATH) && 
		IS_SET(rm_subj->room_flags, PEACEFULL)
		|| IS_SET(rm_subj->room_flags, NO_MAGIC)){
	      tmp->event = EVENT_IGNORE;
	      break;
	    }
	  }
	  else{
	    tmp->event = EVENT_IGNORE;
	    break;
	  }
	  subj = (struct char_data *) tmp->subject;
	  vict = rm_subj->people;
	  next_vict = ((vict) ? vict->next_in_room : 0);
	  for (; next_vict; vict= next_vict){
	    next_vict = vict->next_in_room;
	    act(tmp->arg,FALSE,subj,0,vict,TO_ROOM);
	    dam = tmp->info1;
	    if (!tmp->info4 && vict == subj)
	      dam = 0;
	    if (((tmp->info3 == SPELL_BREATH_OF_VULCAN) ||
		 (tmp->info3 == SPELL_FIRE_BREATH)) &&
		(affected_by_spell(vict, SPELL_ENCASE_IN_ICE))){
	      reduce_ice(vict, dam);
	      dam = 0;
	    }
	    if (tmp->info3 == SPELL_CALL_OF_THE_WATERY_GRAVE
		&& IS_AFFECTED(vict, AFF_WATER_BREATH))
	      dam = 0;
	    else if (((tmp->info3 == SPELL_BREATH_OF_VULCAN) ||
		      (tmp->info3 == SPELL_FIRE_BREATH)) &&
		      IS_AFFECTED(vict, AFF_RESIST_HEAT))
	      dam /= 2;
	    else if (((tmp->info3 == SPELL_BREATH_OF_VULCAN) ||
		      (tmp->info3 == SPELL_FIRE_BREATH)) &&
		      IS_AFFECTED(vict, AFF_RESIST_COLD))
	      dam *= 2;
	    if (saves_spell(vict, SAVING_SPELL, GET_LEVEL(subj))) {
	      dam /= 3;
	      damage(subj,vict,dam,tmp->info3,-1,1);
	      if ((IS_MOB(subj) &&
		   !IS_MOB(vict))
		  || (!IS_MOB(subj)  &&  IS_MOB(vict)))
		spell_damage_equipment(subj, vict, damtype, 3*dam);	      
	    }
	    else{
	      damage(subj,vict,dam,tmp->info3,-1,0);
	      if ((IS_MOB(subj) &&
		   !IS_MOB(vict))
		  || (!IS_MOB(subj)  &&  IS_MOB(vict)))
		spell_damage_equipment(subj, vict, damtype, 2*dam);
	    }
	  }
	  tmp->event = EVENT_IGNORE;
	}
	break;
      case EVENT_SPELL:
	spl = tmp->info4;
	if ((!(spell_info[spl].spell_pointer) && !tmp->info2)
	    || (!(spell_info[spl].spll_pointer) && tmp->info2))
	  break;
	if (tmp->subject)
	  subj = (struct char_data *) tmp->subject;
	else
	  break;
	if (tmp->target)
	  switch(tmp->info3){
	  case 0:
	    vict = (struct char_data *) tmp->target;
	    obj_vict =0;
	    break;
	  case 1:
	    vict = 0;
	    obj_vict = (struct obj_data *) tmp->target;
	    break;
	  }
	else{
	  vict =0;
	  obj_vict = 0;
	  if (!IS_SET(spell_info[spl].targets,TAR_IGNORE)){
	    send_to_char("Your target seems to have wandered off.\r\n",subj);
			    
	    break;
	  }
	}
	if (vict)
	  if (vict->in_room != subj->in_room
	      && spell_info[spl].targets != TAR_CHAR_WORLD ){
	    send_to_char("Your target seems to have wandered off.\r\n",subj);
	    tmp->event = EVENT_IGNORE;
	    break;
	  }
	if (tmp->info2){
	  if (spell_info[spl].spll_pointer)
	    ((*spell_info[spl].spll_pointer) (spl, tmp->info2, subj, tmp->arg, SPELL_TYPE_SPELL, vict, obj_vict));
	  break;
	}
	break;
      default:
	break;
      }
      tmp = tmp->next;
    }
  tmp = events[pulse];
  while (tmp){
    tmp2 = tmp->next;
    free(tmp);
    tmp = tmp2;}
    
  events[pulse] = 0; 

    
  return;
}

void add_event(int plse, int event, int inf1, int inf2, int inf3
	       , int inf4, char *arg, void *subj, void *vict)
{
    struct event_type *evnt,*tmp;
    
    CREATE(evnt, struct event_type, 1);
    evnt->event = event;
    evnt->info1 = inf1;
    evnt->info2 = inf2;    
    evnt->info3 = inf3;	
    evnt->info4 = inf4;
    evnt->arg   = arg;
    evnt->subject = subj;
    evnt->target = vict;
    evnt->next = 0;

    plse += pulse;
    
    plse = plse % 300;
    /* now locate end of event stack
     and add the event on the end */
    if (events[plse]){
	tmp = events[plse];
	while(tmp->next)
	    tmp = tmp->next;

	tmp->next = evnt;}
    else
	events[plse] = evnt;
    return;

}

void extract_event(struct char_data *ch)
{
    int i;
    struct event_type *tmp;
    
    for (i=0;i< 300; i++)
	if ((tmp = events[i]) != NULL)
	    while(tmp)
		{
		    if(tmp->event != EVENT_IGNORE)
			if ((ch == (struct char_data *) tmp->subject)
			    || (ch == (struct char_data *) tmp->target))
			    tmp->event = EVENT_IGNORE;
		    tmp = tmp->next;
		}
}


