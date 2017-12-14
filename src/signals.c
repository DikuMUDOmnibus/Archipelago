/* ************************************************************************
*   File: signals.c                                     Part of CircleMUD *
*  Usage: Signal trapping and signal handlers                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "structs.h"
#include "utils.h"

extern struct descriptor_data *descriptor_list;
extern int	mother_desc;

void	checkpointing();
void	logsig();
void	hupsig();
void	badcrash();
void	unrestrict_game();
void	reread_wizlists();

int	graceful_tried = 0;

void	signal_setup(void)
{
   struct itimerval itime;
   struct timeval interval;

   signal(SIGUSR1, reread_wizlists);
   signal(SIGUSR2, unrestrict_game);

   /* just to be on the safe side: */
   signal(SIGHUP, hupsig);
   signal(SIGPIPE, SIG_IGN);
   signal(SIGINT, hupsig);
   signal(SIGALRM, logsig);
   signal(SIGTERM, hupsig);
   signal(SIGBUS, badcrash);
#ifndef ESSENCE
   signal(SIGSEGV, badcrash);

   /* set up the deadlock-protection */
   interval.tv_sec = 900;    /* 15 minutes */
   interval.tv_usec = 0;
   itime.it_interval = interval;
   itime.it_value = interval;
   setitimer(ITIMER_VIRTUAL, &itime, 0);
   signal(SIGVTALRM, checkpointing);
#endif
}



void	checkpointing(void)
{
   extern int	tics;

   if (!tics) {
      logg("CHECKPOINT shutdown: tics not updated");
      abort();
   } else
      tics = 0;
}


void	reread_wizlists()
{
   void	reboot_wizlists(void);

   signal(SIGUSR1, reread_wizlists);
   mudlog("Rereading wizlists.", CMP, LEVEL_BUILDER, FALSE);
   reboot_wizlists();
}


void	unrestrict_game()
{
   extern int	restrict;
   extern struct ban_list_element *ban_list;
   extern int	num_invalid;

   signal(SIGUSR2, unrestrict_game);
   mudlog("Received SIGUSR2 - unrestricting game (emergent)",
	  BRF, LEVEL_BUILDER, TRUE);
   ban_list = 0;
   restrict = 0;
}





/* kick out players etc */
void	hupsig(void)
{
   logg("Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
   exit(0);   /* something more elegant should perhaps be substituted */
}


void	badcrash(void)
{
   void	close_socket(struct descriptor_data *d);
   struct rlimit          limit;
   int                    old_core_limit;
   struct descriptor_data *desc;

   logg("SIGSEGV or SIGBUS received.  Trying to shut down gracefully.");
   if (!graceful_tried) {
      graceful_tried = 1;
      /* prevent us from generating a second core for debugging reasons */
      getrlimit(RLIMIT_CORE, &limit);
      old_core_limit = limit.rlim_cur;
      limit.rlim_cur = 0;
      setrlimit(RLIMIT_CORE, &limit);
      close(mother_desc);
      logg("Trying to close all sockets.");
      for (desc = descriptor_list; desc; desc = desc->next)
	 close(desc->descriptor);
      limit.rlim_cur = old_core_limit;
      setrlimit(RLIMIT_CORE, &limit);
   }
   abort();
}


void	logsig(void)
{
   logg("Signal received.  Ignoring.");
}


