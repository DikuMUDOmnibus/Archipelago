#include "config.h"
/*#include "kiwi/kiwi.h"
#include "kiwi/string.h"
*/

extern int      sys_nerr;
extern char *   sys_errlist[];

char *strerror( int num )
{

    if( num > sys_nerr || num < 0 ) {
        return( "Unknown error code" );
    }
    return( sys_errlist[ num ] );
}
