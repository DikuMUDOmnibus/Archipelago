/* copy a string, returning pointer to the null terminator of dest */
char *stpcpy(char *dest, const char *src)
{
    while( (*dest = *src) ) {
        ++dest;
        ++src;
    }
    return (dest);
}
