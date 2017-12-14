
char *format_inet_addr( char *dest, long addr )
{
    dest += sprintf( dest, "%ld.%ld.%ld.%ld", 
                (addr & 0xFF000000) >> 24,
                (addr & 0x00FF0000) >> 16,
                (addr & 0x0000FF00) >> 8,
                (addr & 0x000000FF) );
    return( dest );
}
