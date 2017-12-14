/*
    This slave does iptoname conversions, and identquery lookups.

    The philosophy is to keep this program as simple/small as possible.
    It does normal fork()s, so the smaller it is, the faster it goes.
*/
#define RETSIGTYPE void 

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "slave.h"
#include <arpa/inet.h>

void log_write(FILE *fp, char *fmt, ...);
int log_stderr(char *fmt);

pid_t parent_pid;

#define MAX_STRING 4096

char *arg_for_errors;

RETSIGTYPE child_timeout_signal()
{
  /* this just causes log spam -djg  
  log_write( stderr, "'%s' 5 minute timeout", arg_for_errors );  */
  exit( 1 );
}


int query( char *orig_arg )
{
  char *comma;
  char *port_pair;
  struct hostent *hp;
  struct sockaddr_in sin;
  int s;
  FILE *f;
  char result[ MAX_STRING ];
  char buf[ MAX_STRING ];
  char arg[ MAX_STRING ];
  size_t len;
  char *p;
  
  arg_for_errors = orig_arg;
  strcpy( arg, orig_arg );
  comma =(char *) strrchr( arg, ',' );
  if( comma == NULL ) {
    log_write( stderr, "invalid parm '%s'", orig_arg );
    return( -1 );
  }
  *comma = 0;
  port_pair =(char *) strrchr( arg, ',' );
  if( port_pair == NULL ) {
    log_write( stderr, "invalid parm '%s'", orig_arg );
    return( -1 );
  }
  *port_pair++ = 0;
  *comma = ',';
  
  hp = gethostbyname(arg);
  if( hp == NULL ) {
    static struct hostent def;
    static struct in_addr defaddr;
    static char *alist[1];
    static char namebuf[128];
    
    defaddr.s_addr = inet_addr(arg);
    if (defaddr.s_addr == -1) {
      log_write( stderr, "'%s': unknown host", orig_arg ); 
      return( -1 );
    }
    strcpy(namebuf, arg);
    def.h_name = namebuf;
    def.h_addr_list = alist;
    def.h_addr = (char *)&defaddr;
    def.h_length = sizeof (struct in_addr);
    def.h_addrtype = AF_INET;
    def.h_aliases = 0;
    hp = &def;
  }
  sin.sin_family = hp->h_addrtype;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons( 113 );        /* ident port */
  s = socket(hp->h_addrtype, SOCK_STREAM, 0);
  if( s < 0 ) {
    log_write( stderr, "'%s': socket: %s", orig_arg, strerror( errno ) );
    return (-1);
  }
  if( connect(s, (struct sockaddr *)&sin, sizeof (sin)) < 0 ) {
    if( errno != ECONNREFUSED
	&& errno != ETIMEDOUT
	&& errno != ENETUNREACH
	&& errno != EHOSTUNREACH ) {
      log_write( stderr, "'%s': connect: %s", orig_arg, strerror( errno ) ); 
    }
    close(s);
    return( -1 );
  }

  len = strlen( port_pair );
  if( write(s, port_pair, len) != len ) {
    log_write( stderr, "'%s': write: %s", orig_arg, strerror( errno ) ); 
    close(s);
    return( -1 );
  }
  if( write(s, "\r\n", 2) != 2 ) {
    log_write( stderr, "'%s': write: %s", orig_arg, strerror( errno ) );
    close(s);
    return( -1 );
  }
  f = fdopen(s, "r");
  {
    int c;

    p = result;
    while( ( c = fgetc( f ) ) != EOF ) {
      if( c == '\n' ) break;
      if( isprint( c ) ) {
	*p++ = c;
	if( p - result == MAX_STRING - 1 ) break;
      }
    }
    *p = 0;
  }
  (void)fclose(f);
  len = p - result;
  buf[0] = SLAVE_IDENTQ;
  p = (char *)format_inet_addr( buf+1, ntohl(sin.sin_addr.s_addr) );
  *p++ = ' ';
  if( len + ( p - buf ) >= sizeof( buf ) - 2 ) {
    log_write( stderr, "'%s': result too long", orig_arg );
    return( -1 );
  }
  p = (char *) stpcpy( p, result );
  *p++ = '\n';
  write( 1, buf, p - buf );
  return( 0 );
}


int iptoname( char *arg )
{
  unsigned long addr;
  struct hostent *hp;
  char buf[ MAX_STRING ];
  char *p;
  
  addr = inet_addr( arg );
  if( addr == -1 ) {
    log_write( stderr, "%s is not a valid decimal ip address", arg );
    return( -1 );
    }
  
  hp = gethostbyaddr( (char *)&addr, sizeof(addr), AF_INET );
  if( hp ) {
    buf[0] = SLAVE_IPTONAME;
    p = (char *)stpcpy( buf+1, arg );
    *p++ = ' ';
    p = (char *) stpcpy( p, hp->h_name );
    *p++ = '\n';
    write( 1, buf, p - buf );
  }
  return( 0 );
}


RETSIGTYPE child_signal()
{
  /* collect any children */
  while( waitpid( 0, NULL, WNOHANG ) > 0 )
    ;
    signal( SIGCHLD, child_signal );
}


RETSIGTYPE alarm_signal()
{
  struct itimerval itime;
  struct timeval interval;
  
  if( getppid() != parent_pid ) {
    log_write( stderr, "shutdown since parent is 1" );
    exit( 1 );
  }
  signal( SIGALRM, alarm_signal );
  interval.tv_sec = 120;      /* 2 minutes */
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_REAL, &itime, 0);
}


void main( int argc, char **argv )
{
  char arg[ MAX_STRING +1 ];
  char *p;
  int len;
  
  if( log_stderr( "slave_log" ) < 0 ) {
    exit( 1 );
  }
  log_write( stderr, "slave booted" ); 
  parent_pid = getppid();
  if( parent_pid == 1 ) {
    log_write( stderr, "parent is pid 1, shutting down" ); 
    exit( 1 );
  }
  alarm_signal();
  signal( SIGCHLD, child_signal );
  signal( SIGPIPE, SIG_DFL );
  
  for(;;) {
    len = read( 0, arg, MAX_STRING );
    if( len == 0 ) break;
    if( len < 0 ) {
      if( errno == EINTR ) {
	errno = 0;
	continue;
      }
      log_write( stderr, "read: %s", strerror( errno ) );
      break;
    }
    arg[len] = 0;
    p = (char *) strchr( arg, '\n' );
    if( p ) *p = 0;
    log_write( stderr, "received: '%s'", arg );
    switch( fork() ) {
    case -1:
      log_write( stderr, "'%s': fork: %s", arg, strerror( errno ) );
      exit( 1 );
        
    case 0: /* child */
      {
	/* we don't want to try this for more than 5 minutes */
	struct itimerval itime;
	struct timeval interval;
	
	interval.tv_sec = 300;      /* 5 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	signal(SIGALRM, child_timeout_signal);
	setitimer(ITIMER_REAL, &itime, 0);
      }
      switch( arg[0] ) {
      case SLAVE_IDENTQ:
	exit( query( arg+1 ) != 0 );
      case SLAVE_IPTONAME:
	exit( iptoname( arg+1 ) != 0 );
      default:
	log_write( stderr, "invalid arg: %s", arg );
      }
    }
    /* collect any children */
        while( waitpid( 0, NULL, WNOHANG ) > 0 )
	  ;
  }
  log_write( stderr, "exiting" );
  exit( 0 );
}
void log_write(FILE *fp, char *fmt, ...)
{
  va_list args;
  
  va_start(args, fmt);
  fprintf(fp, "RFC 1413 Slave: ");
  vfprintf(fp, fmt, args);
  fprintf(fp, "\n");
  va_end(args);
  return;

}

int log_stderr(char *fmt)
{
  return(fprintf(stderr, "%s\n", fmt));

}
