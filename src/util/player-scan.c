/* ************************************************************************
*  file: purgeplay.c                                    Part of CircleMUD * 
*  Usage: purge useless chars from playerfile                             *
*  All Rights Reserved                                                    *
*  Copyright (C) 1992, 1993 The Trustees of The Johns Hopkins University  *
************************************************************************* */

#include <stdio.h>
#include <ctype.h>
#include "../structs.h"
#include "../utils.h"

void print_details(struct char_file_u *player)
{
    printf("%s %s\n", player->name, player->title);
}

void	purge(char *filename)
{
   FILE * fl;
   FILE * outfile;
   struct char_file_u player;
   int	okay, num = 0, j;
   long	timeout;
   char	*ptr, reason[80], *name[500];

   if (!(fl = fopen(filename, "r+"))) {
      printf("Can't open %s.", filename);
      exit();
   }
   num = 0;
   for (; ; ) {
       fread(&player, sizeof(struct char_file_u ), 1, fl);
       if (feof(fl)) 
	   fclose(fl);
       else{
	   CREATE(name[num], char ,strlen(player.name) + 1);
	   strcpy(name[num], player.name);
	   for (j = num-1; j > 0; j--){
	       if (strcmp(name[j], name[num]) == 0)
		   {
		       printf("A Match %d %d\n", j,num);
		       rewind(fl);
		       fseek(fl,(long) (j*sizeof( struct char_file_u)), SEEK_SET);
		       fread(&player, sizeof(struct char_file_u ), 1, fl);
		       print_details(&player);
		       rewind(fl);
		       fseek(fl, (long) (num*sizeof( struct char_file_u)), SEEK_SET);
		       fread(&player, sizeof(struct char_file_u ), 1, fl);
		       print_details(&player);
		   }
	       
	   }
	   num++;
       }
   }
   close(fl);
   return;
}


main(int argc, char *argv[])
{
   if (argc != 2)
      printf("Usage: %s playerfile-name\n", argv[0]);
   else
      purge(argv[1]);
}


