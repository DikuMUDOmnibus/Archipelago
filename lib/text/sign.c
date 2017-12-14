From: jelson@condor.cs.jhu.edu (Jeremy Elson)
Newsgroups: rec.games.mud.diku,rec.games.mud.admin
Subject: Yet another sign.c program (corrected)
Date: 16 Aug 1994 02:32:08 -0400
Organization: The Johns Hopkins University CS Department
NNTP-Posting-Host: condor.cs.jhu.edu


Forgive me if you've seen this already, but I had to cancel the first post
and resend it.

This is yet another sign program for presenting a text message on a TCP
port.  Like the perl program Gnort recently posted, this program forks
children in order to handle multiple connections simultaneously; unlike
his, though, this one is written in C for people like me who are perl-
impaired. :)

This sign program automatically daemonizes itself so that you don't have
to bother running it in the background explicitly.

You can give it either a filename argument for the text, or "-" to have it
read from standard input.  If you do tell it to read from stdin, it will
prompt you for input unless it senses that standard input is not the console,
i.e. if you type "cat filename | a.out 5000 -" or something similar.

If you find any bugs, please forward them to me in a non-inflammatory manner.

This program is in the public domain.

Jeremy

----------------------------------------------------------------------------

/*
 * sign.c: a program to present text on a TCP port
 * Author: Jeremy Elson (jelson@cs.jhu.edu)
 *  Usage: sign <port> <filename> or
 *         sign <port> -
 *
 * '-' indicates file should be read from stdin.
 *
 * This program is in the public domain.  It may be copied, redistributed,
 * reused, modified, etc., but a notice of my authorship must be maintained.
 *
 * This program comes with no warranty of any kind, expressed or implied.
 */

#define MAX_FILESIZE	8192
#define LINEBUF_SIZE	128

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>

/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
int init_socket(int port)
{
  int s, opt;
  struct sockaddr_in sa;

  /*
   * Should the first argument to socket() be AF_INET or PF_INET?  I don't
   * know, take your pick.  PF_INET seems to be more widely adopted, and
   * Comer (_Internetworking with TCP/IP_) even makes a point to say that
   * people erroneously use AF_INET with socket() when they should be using
   * PF_INET.  However, the man pages of some systems indicate that AF_INET
   * is correct; some such as ConvexOS even say that you can use either one.
   * All implementations I've seen define AF_INET and PF_INET to be the same
   * number anyway, so ths point is (hopefully) moot.
   */

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Create socket");
    exit(1);
  }

#if defined(SO_REUSEADDR)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEADDR");
    exit(1);
  }
#endif

#if defined(SO_REUSEPORT)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEPORT");
    exit(1);
  }
#endif

#if defined(SO_LINGER)
  {
    struct linger ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
      perror("setsockopt LINGER");
      exit(1);
    }
  }
#endif

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(s, (struct sockaddr *) & sa, sizeof(sa)) < 0) {
    perror("bind");
    close(s);
    exit(1);
  }
  listen(s, 5);
  return s;
}


char *get_text(char *fname)
{
  static char t[MAX_FILESIZE];
  char tmp[LINEBUF_SIZE + 2];
  FILE *fl = NULL;

  *t = '\0';

  if (!strcmp(fname, "-")) {
    fl = stdin;
    if (isatty(STDIN_FILENO))
      fprintf(stderr, "Enter sign text; terminate with Ctrl-D.\n");
  } else {
    if (!(fl = fopen(fname, "r"))) {
      perror(fname);
      exit(1);
    }
  }

  while (fgets(tmp, LINEBUF_SIZE, fl)) {
    if (strlen(tmp) + strlen(t) < MAX_FILESIZE-1)
      strcat(t, strcat(tmp, "\r"));
    else {
      fprintf(stderr, "String too long.  Truncated.\n");
      break;
    }
  }

  return t;
}


/* clean up our zombie kids to avoid defunct processes */
reap()
{
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  signal(SIGCHLD, reap);
}


int main(int argc, char *argv[])
{
  char *txt;
  int desc, remaining, bytes_written, len, s, port;

  if (argc != 3 || (port = atoi(argv[1])) < 1024) {
    fprintf(stderr, "usage: %s <portnum> <\"-\" | filename>\n", argv[0]);
    exit(1);
  }

  s = init_socket(port);
  len = strlen(txt = get_text(argv[2]));

  if (fork() > 0) {
    fprintf(stderr, "Sign started on port %d.\n", port);
    exit(0);
  }

  signal(SIGCHLD, reap);

  for (;;) {
    if ((desc = accept(s, (struct sockaddr *) NULL, 0)) < 0)
      continue;

    if (fork() == 0) {
      remaining = len;
      do {
	if ((bytes_written = write(desc, txt, remaining)) < 0)
	  exit(0);
	else {
	  txt += bytes_written;
	  remaining -= bytes_written;
	}
      } while (remaining > 0);
      exit(0);
    }
    close(desc);
  }
}
-- 
Jeremy Elson
Internet: jelson@cs.jhu.edu; Bitnet: jelson@jhunix
