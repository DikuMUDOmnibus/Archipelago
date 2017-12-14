#include "slave.h"

pid_t slave_pid;
static int slave_socket = -1;   /* handle for the slave task */

..... somewhere in your descriptor structure you should have these fields

    unsigned long addr;         /* numerical address of host            */
    unsigned long port;         /* remote port number                   */
    char host[MAX_HOSTNAME+1];  /* text hostname                        */
    char userid[MAX_USERID+1];  /* userid from rfc1413 lookup           */

..... here is how the beginning of my new_connection routine looks
int new_connection(int s)
{
    int desc, size;
    unsigned long addr;
    struct descriptor_data *newd;
    struct sockaddr_in sock;
    char buf[ MAX_STRING_LENGTH+1 ];
    char *p;

    size = sizeof(sock);
    desc = accept(s, (struct sockaddr *)&sock, &size);
    if( desc < 0 ) {
        syslogf( "new_connection: accept(): %s", strerror( errno ) );
        return(-1);
    }

    addr = ntohl(sock.sin_addr.s_addr);
    
    /* game full check omitted */

    /* mark non-blocking, and close-on-exec */
    if( fcntl( desc, F_SETFL, FNDELAY ) == -1 ) {
        syslogf( "new_connection: fcntl( F_SETFL, FNDELAY ): %s",
            strerror( errno ) );
        close( desc );
        return( 0 );
    }
    if( fcntl( desc, F_SETFD, 1 ) == -1 ) {
        syslogf( "new_connection: fcntl( F_SETFD ): %s", strerror( errno ) );
        /* this isn't really fatal */
    }

    buf[0] = SLAVE_IPTONAME;
    p = format_inet_addr( buf+1, addr );

    /* banned check omitted */

    CREATE(newd, struct descriptor_data, 1);
    newd->descriptor = desc;
    strcpy( newd->host, buf+1 );        /* default to numeric hostname */
    newd->addr = addr;
    newd->port = ntohs( sock.sin_port );

    if( slave_socket != -1 && !SET_is_member( mudmode, MUD_NO_IPTONAME ) ) {
        /* ask slave to do a hostname lookup for us */
        *p = '\n';
        if( write( slave_socket, buf, p - buf + 1 ) != p - buf + 1 ) {
            syslogf( "{SLAVE} losing slave on write: %s", strerror( errno ));
            close( slave_socket );
            slave_socket = -1;
        }
        /* ask slave to do an identquery for us */
        buf[0] = SLAVE_IDENTQ;
        p += sprintf( p, ",%d,%d\n", ntohs( sock.sin_port ), game_port );
        if( write( slave_socket, buf, p - buf ) != p - buf ) {
            syslogf( "{SLAVE} losing slave on write: %s", strerror( errno ));
            close( slave_socket );
            slave_socket = -1;
        }
    }

    ..... remainder of new_connection code
}


......

/* get a result from the slave */
static int get_slave_result( void )
{
    char buf[ MAX_STRING_LENGTH +1 ];
    char token[ MAX_STRING_LENGTH ];
    char os[ MAX_STRING_LENGTH ];
    char userid[ MAX_STRING_LENGTH ];
    int octet[4];
    int local_port, remote_port;
    char *p;
    struct descriptor_data *d;
    long addr;
    int len;

    len = read( slave_socket, buf, MAX_STRING_LENGTH );
    if( len < 0 ) {
        if( errno == EAGAIN || errno == EWOULDBLOCK ) return( -1 );
        syslogf( "{SLAVE} losing slave on read: %s", strerror( errno ) );
        close( slave_socket );
        slave_socket = -1;
        return( -1 );
    } else if( len == 0 ) {
        return( -1 );
    }
    buf[len] = 0;

    switch( buf[0] ) {
    case SLAVE_IDENTQ:
        if( sscanf( buf+1, "%d.%d.%d.%d %d , %d : %[^: ] : %[^:] : %s",
            &octet[0], &octet[1], &octet[2], &octet[3],
            &remote_port, &local_port,
            token, os, userid ) != 9 ) {
            /* don't bother logging "ERROR" result codes */
            if( sscanf( buf+1, "%d.%d.%d.%d %d , %d : %[^:] : %s",
                &octet[0], &octet[1], &octet[2], &octet[3],
                &remote_port, &local_port,
                token, userid ) != 8 || strnicmp( token, "ERROR", 5 ) ) {
                p = strchr( buf, '\n' );
                *p = 0;
                syslogf( "{SLAVE} invalid: '%s'", buf );
            }
            return( 0 );
        }
        if( local_port != game_port ) {
            syslogf( "{SLAVE} invalid local port: '%s'", buf );
            return( 0 );
        }
        addr = ( octet[0] << 24 )
            + ( octet[1] << 16 )
            + ( octet[2] << 8 )
            + ( octet[3] );
        for( d = descriptor_list; d; d = d->next ) {
            if( d->port != remote_port ) continue;
            if( d->addr != addr ) continue;
            strncpy( d->userid, userid, MAX_USERID );
            d->userid[MAX_USERID] = 0;
            return( 0 );
        }
        break;
    
    case SLAVE_IPTONAME:
        if( sscanf( buf+1, "%d.%d.%d.%d %s",
                &octet[0], &octet[1], &octet[2], &octet[3], token ) != 5 ) {
            syslogf( "{SLAVE} invalid: %s", buf );
            return( 0 );
        }
        addr = ( octet[0] << 24 )
            + ( octet[1] << 16 )
            + ( octet[2] << 8 )
            + ( octet[3] );
        for( d = descriptor_list; d; d = d->next ) {
            if( d->addr != addr ) continue;
            strncpy( d->host, token, MAX_HOSTNAME );
            d->host[MAX_HOSTNAME] = 0;
        }
        break;

    default:
        syslogf( "{SLAVE} invalid: %s", buf );
        break;
    }
    return( 0 );
}


....


void game_loop(int s)
{

    ..... in the main game loop before the select() call
            /* Check what's happening out there */
            ARGV0( ARGV0_select );
            FD_ZERO(&input_set);
            FD_ZERO(&output_set);
            FD_ZERO(&exc_set);
            FD_SET(s, &input_set);
            if( slave_socket != -1 ) {
                FD_SET( slave_socket, &input_set );
            }

    .... somewhere after the select() call
                /* slave result? */
                if( slave_socket != -1
                    && FD_ISSET( slave_socket, &input_set ) ) {
                    while( get_slave_result() == 0 )
                        ;
                }

    .... somewhere while shutting down

    /* deal with slave */
    if( slave_socket != -1 ) {
        kill( slave_pid, SIGKILL );
    }
}


void boot_slave( void )
{
    int sv[2];
    int i;

    if( slave_socket != -1 ) {
        close( slave_socket );
        slave_socket = -1;
    }

    syslogf( "{BOOT} booting slave" );
    syslogf( "{BOOT} slave: ouch" );

    if( socketpair( AF_UNIX, SOCK_DGRAM, 0, sv ) < 0 ) {
        syslogf( "boot_slave: socketpair: %s", strerror( errno ) );
        return;
    }
    /* set to nonblocking */
    if( fcntl( sv[0], F_SETFL, FNDELAY ) == -1 ) {
        syslogf( "boot_slave: fcntl( F_SETFL, FNDELAY ): %s",
            strerror( errno ) );
        close(sv[0]);
        close(sv[1]);
        return;
    }
    slave_pid = vfork();
    switch( slave_pid ) {
    case -1:
        syslogf( "boot_slave: vfork: %s", strerror( errno ) );
        close( sv[0] );
        close( sv[1] );
        return;

    case 0: /* child */
        close( sv[0] );
        close( 0 );
        close( 1 );
        if( dup2( sv[1], 0 ) == -1 ) {
            syslogf( "boot_slave: child: unable to dup stdin: %s", strerror( errno ) );
            _exit( 1 );
        }
        if( dup2( sv[1], 1 ) == -1 ) {
            syslogf( "boot_slave: child: unable to dup stdout: %s", strerror( errno ) );
            _exit( 1 );
        }
        for( i = 3; i < system_max_handle; ++i ) {
            close( i );
        }
        execlp( "slave", "slave", NULL );
        syslogf( "boot_slave: child: unable to exec: %s", strerror( errno ) );
        _exit( 1 );
    }
    close( sv[1] );

    if( fcntl(sv[0], F_SETFL, FNDELAY ) == -1 ) {
        syslogf( "boot_slave: fcntl: %s", strerror( errno ) );
        close( sv[0] );
        return;
    }
    slave_socket = sv[0];
}
