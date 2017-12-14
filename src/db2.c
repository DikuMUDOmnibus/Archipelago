/* ************************************************************************
*   File: db2.c                                         Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* db.c split into two files db1.c and db2.c                              */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#define __DB_C__
#define ZCMD zone_table[zn].cmd[cmd_no]
#define PROTO_REVISION 3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "shop.h"
#include "boards.h"

extern short r_mortal_start_room;
extern short r_immort_start_room;
extern short r_frozen_start_room;
extern struct index_data *mob_index;
extern struct index_data *obj_index;	       
extern struct char_data *mob_proto;
extern struct obj_data *object_list;
extern struct obj_data *obj_proto;		
extern int	top_of_mobt;
extern int	top_of_objt;
extern int	top_of_zone_table;
extern int      top_of_world;
extern struct shop_data *shop_index;
extern int	number_of_shops;
extern struct char_data *character_list;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct board_info_type board_info[];
/* local functions */
int     create_zone(int zone);
void    write_db_index(int which);
void    copy_stringinfo(char **to, char **from);
void	load_rooms(FILE *fl);
void	load_objects(FILE *obj_f);
void	load_mobiles(FILE *mob_f);
int     create_zreset(int zn);
int     destroy_zreset(int zn, int no);
int     create_mob(int zone);
int     create_obj(int zone);
int     create_room(int zone);
int     create_shop(int zone);
void    make_dummy_mob(int i);
void    make_dummy_obj(int i);
void    make_dummy_room(int i);
void    clone_mob(int from, int to);
void    fwrite_string(FILE *wfile,char *wstr);
void    clone_room(int from, int to);
void	setup_dir(FILE *fl, int room, int dir, int mode);
void    copy_room(struct room_data *from, struct room_data *to);
void    copy_shop(struct shop_data *from, struct shop_data *to);
void    free_room(struct room_data *room);
void    free_shop(struct shop_data *shop);
void    copy_mob(struct char_data *from, struct char_data *to);
void    free_mob(struct char_data *mob);
void    copy_obj(struct obj_data *from, struct obj_data *to, bool copy_extra);
void    free_objstruct(struct obj_data *obj);
void    copy_extras(struct extra_descr_data **target, struct extra_descr_data *exfrm );
/* external functions */
void	load_messages(void);
void	weather_and_time(int mode);
void	assign_command_pointers(void);
void	assign_spell_pointers(void);
void	boot_social_messages(void);
void	update_obj_file(void); /* In objsave.c */
void	sort_commands(void);
void	load_banned(void);
void	Read_Invalid_List(void);
struct help_index_element *build_help_index(FILE *fl, int *num);
void	clear_room(struct room_data *room);

#define MARK(room) (SET_BIT(world[room].room_flags, BFS_MARK))
#define UNMARK(room) (REMOVE_BIT(world[room].room_flags, BFS_MARK))
#define IS_MARKED(room) (IS_SET(world[room].room_flags, BFS_MARK))
#define TOROOM(x, y) (real_room(world[(x)].dir_option[(y)]->to_room))
#define IS_CLOSED(x, y) (IS_SET(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED))

void	load_mobiles(FILE *mob_f)
{
   static int	i = 0;
   int	nr, j,ii;
   long	tmp, tmp2, tmp3, tmp4, tmp5, tmp1;
   char	chk[10], *tmpptr;
   char	letter;


   if (!fscanf(mob_f, "%s\n", chk)) {
      perror("load_mobiles");
      exit(1);
   }

   for (; ; ) {
      if (*chk == '#') {
	 sscanf(chk, "#%d\n", &nr);
	 if (nr >= 99999) 
	    break;

	 mob_index[i].virtual = nr;
	 mob_index[i].number  = 0;
	 mob_index[i].func    = 0;

	 clear_char(mob_proto + i);

	 sprintf(buf2, "mob vnum %d", nr);

	 /***** String data *** */
	 mob_proto[i].player.name = fread_string(mob_f, buf2);
	 tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
	 if (tmpptr && *tmpptr)
	    if (!str_cmp(fname(tmpptr), "a") || 
	        !str_cmp(fname(tmpptr), "an") || 
	        !str_cmp(fname(tmpptr), "the"))
	       *tmpptr = tolower(*tmpptr);
	 mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
	 mob_proto[i].player.description = fread_string(mob_f, buf2);
	 mob_proto[i].player.title = 0;
	 mob_proto[i].player.prmpt = 0;	 

	 /* *** Numeric data *** */
	 fscanf(mob_f, "%ld ", &tmp);
	 MOB_FLAGS(mob_proto + i) = tmp;
	 SET_BIT(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
	 fscanf(mob_f, " %ld %ld %c\n", &tmp, &tmp2, &letter);
	 mob_proto[i].specials.affected_by = tmp;
	 GET_ALIGNMENT(mob_proto + i) = tmp2;

	 if (letter == 'S') {
	    /* The new easy monsters */
	    mob_proto[i].abilities.str   = 14;
	    mob_proto[i].abilities.intel = 14;
	    mob_proto[i].abilities.wis   = 14;
	    mob_proto[i].abilities.dex   = 14;
	    mob_proto[i].abilities.con   = 14;
	    mob_proto[i].abilities.chr   = 14;
	    mob_proto[i].abilities.per   = 14;
	    mob_proto[i].abilities.gui   = 14;
	    mob_proto[i].abilities.luc   = 14;
	    mob_proto[i].abilities.foc   = 14;
	    mob_proto[i].abilities.dev   = 14;
	    
	    fscanf(mob_f, " %ld %ld %ld", &tmp, &tmp2, &tmp3);
	    GET_LEVEL(mob_proto + i) = tmp;
	    mob_proto[i].points.hitroll = 20 - tmp2;
	    mob_proto[i].points.armor[0] = 10 * tmp3;
	    mob_proto[i].points.armor[1] = 10 * tmp3;
	    mob_proto[i].points.armor[2] = 10 * tmp3;
	    mob_proto[i].points.armor[3] = 10 * tmp3;
	    
	    fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
	    mob_proto[i].points.max_hit = 0;
	    mob_proto[i].points.hit = tmp;
	    mob_proto[i].points.mana = tmp2;
	    mob_proto[i].points.move = tmp3;

	    mob_proto[i].points.max_mana = 100;
	    mob_proto[i].points.max_move = 500;

	    fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
	    mob_proto[i].specials.damnodice = tmp;
	    mob_proto[i].specials.damsizedice = tmp2;
	    mob_proto[i].points.damroll = tmp3;

	    fscanf(mob_f, " %ld %ld \n", &tmp, &tmp2);
	    GET_GOLD(mob_proto + i) = tmp;
	    /*	    GET_FAME(mob_proto + i) = tmp2;*/
	    GET_FAME(mob_proto + i) = 0;	    

	    fscanf(mob_f, " %ld %ld %ld \n", &tmp, &tmp2, &tmp3);
	    /* hack to fix up mob positions AJN 12th Oct 94*/
	    if (tmp2 > POSITION_STANDING)
		tmp2 = POSITION_STANDING;
	    if (tmp > POSITION_STANDING)
		tmp = POSITION_STANDING;	     	     
	    mob_proto[i].specials.position = tmp;
	    mob_proto[i].specials.default_pos = tmp2;
	    mob_proto[i].player.sex = tmp3;

	    mob_proto[i].player.race = 0;

	    mob_proto[i].player.weight = 2000;
	    mob_proto[i].player.height = 198;

	    for (j = 0; j < 3; j++)
	       GET_COND(mob_proto + i, j) = -1;

	    for (j = 0; j < 5; j++)
	       mob_proto[i].specials2.apply_saving_throw[j] = 
	           MAX(100 - GET_LEVEL(mob_proto + i), 2);
	 } else {  /* The New format monsters are down below here */
	     fscanf(mob_f, "%ld %ld %ld %ld %ld\n ", &tmp, &tmp2, &tmp3, &tmp4, &tmp5);
	     mob_proto[i].abilities.str = tmp;
	     mob_proto[i].abilities.intel = tmp2;
	     mob_proto[i].abilities.wis = tmp3;
	     mob_proto[i].abilities.dex = tmp4;
	     mob_proto[i].abilities.con = tmp5;
	    
	     fscanf(mob_f, "%ld %ld %ld %ld %ld\n ", &tmp, &tmp2, &tmp3, &tmp4, &tmp5);
	     mob_proto[i].abilities.chr = tmp;
	     mob_proto[i].abilities.per = tmp2;
	     mob_proto[i].abilities.gui = tmp3;
	     mob_proto[i].abilities.dev = tmp4;
	     mob_proto[i].abilities.foc = tmp5;
	     
	     fscanf(mob_f, "%ld %ld %ld\n", &tmp, &tmp2, &tmp3);

	     mob_proto[i].points.max_move = tmp3;
	     
	     fscanf(mob_f, "%ld %ld %ld %ld\n", &tmp, &tmp2, &tmp3, &tmp4);

	     mob_proto[i].player.race = tmp;
	     GET_RAW_LUC(mob_proto + i) = tmp2;
	     GET_LEVEL(mob_proto + i) = tmp3;
	     mob_proto[i].points.hitroll = (20 - tmp4);

	     fscanf(mob_f,"%ld %ld %ld\n",&tmp,&tmp2,&tmp3);
	     if (tmp < 300)
		 tmp += 300;
	     mob_proto[i].specials.attack_type =  tmp;
	     mob_proto[i].player.height = tmp2;
	     mob_proto[i].player.weight = tmp3;

	     fscanf(mob_f, "%ldd%ld+%ld", &tmp,&tmp2,&tmp3);

	     mob_proto[i].points.hit = tmp;
	     mob_proto[i].points.mana = tmp2;
	     mob_proto[i].points.move = tmp3;	     

	     fscanf(mob_f, " %ldd%ld+%ld\n", &tmp,&tmp2,&tmp3);
	     
	     mob_proto[i].specials.damnodice = tmp;
	     mob_proto[i].specials.damsizedice = tmp2;
	     mob_proto[i].points.damroll = tmp3;
	     for (ii=0;ii<=3;ii++){
	       fscanf(mob_f,"%ld ",&tmp);
	       mob_proto[i].points.armor[ii] = tmp;}
	     fscanf(mob_f,"\n");
	     for (ii=0;ii<=3;ii++){
	       fscanf(mob_f,"%ld ",&tmp);
	       mob_proto[i].points.stopping[ii] = tmp;}
	     fscanf(mob_f,"\n");
	     for (ii=0;ii<=4;ii++){
	       fscanf(mob_f,"%ld ",&tmp);
	       mob_proto[i].specials2.apply_saving_throw[ii] = tmp;}
	     fscanf(mob_f,"\n");
	     
	     
	    fscanf(mob_f, "%ld %ld\n ", &tmp, &tmp2);
	    GET_GOLD(mob_proto +i) = tmp;
	    GET_EXP(mob_proto + i) = tmp2;

	    fscanf(mob_f, "%ld %ld %ld\n", &tmp, &tmp2, &tmp3);
	    /* hack to fix up mob positions AJN 12th Oct 94*/
	    if (tmp2 > POSITION_STANDING)
		tmp2 = POSITION_STANDING;
	    if (tmp > POSITION_STANDING)
		tmp = POSITION_STANDING;	     	     
	    mob_proto[i].specials.position = tmp;
	    mob_proto[i].specials.default_pos = tmp2;
	    mob_proto[i].player.sex = tmp3;

	    for (j = 0; j < 3; j++) {
	       fscanf(mob_f, "%ld ", &tmp);
	       GET_COND(mob_proto + i, j) = tmp;
	    }

	    fscanf(mob_f, "\n");

	    /* Calculate THAC0 as a formula of Level */
	     mob_proto[i].points.hitroll = MIN(20,mob_proto[i].points.hitroll);
	     mob_proto[i].points.hitroll = MAX(0,mob_proto[i].points.hitroll);
	       /*	= MAX(0, 20 - 5*GET_LEVEL(mob_proto[i]/25); */
	 }

	 mob_proto[i].tmpabilities = mob_proto[i].abilities;

	 for (j = 0; j < MAX_WEAR; j++) /* Initialising Ok */
	    mob_proto[i].equipment[j] = 0;

	 mob_proto[i].nr = i;
	 mob_proto[i].desc = 0;

	 if (!fscanf(mob_f, "%s\n", chk)) {
	   sprintf(buf2, "SYSERR: EOF, Format error in mob file near mob #%d", nr);
	   logg(buf2);
	   exit(1);
	 }

	 if (*chk == 'E')
	     {
		 fscanf(mob_f, "%ld %ld %ld\n", &tmp, &tmp1, &tmp2);
		 mob_proto[i].specials.attack_type = tmp + 300;
		 mob_proto[i].player.height = tmp1;
		 mob_proto[i].player.weight = tmp2;
		 fscanf(mob_f, " %s \n", chk);
	     } 
	 i++;
      }
      else if (*chk == '$') /* EOF */
	break;
      else {
         sprintf(buf2, "SYSERR: Missing $, %s, Format error in mob file near mob #%d",chk, nr);
	 logg(buf2);
	 exit(1);
      }
   }
   top_of_mobt = i - 1;

}

int create_zreset(int zn)
{
    int cmd_no;

    for (cmd_no = 0; ;cmd_no++)
	if (ZCMD.command == 'S')
	    break;

    if (!(zone_table[zn].cmd = (struct reset_com *) realloc( zone_table[zn].cmd, (cmd_no +2)*sizeof(struct reset_com)))){
	printf("Error in realloc of zone table resets (increase).\n");
    }
    ZCMD.command = '*';
    cmd_no++;
    ZCMD.command = 'S';
    cmd_no--;
    return(cmd_no);
    
}

int destroy_zreset(int zn, int num)
{
    int cmd_no,no;

    cmd_no = num;
    if (zone_table[zn].cmd[num].command == 'S'){
	printf("Attempt to delete end of zreset table marker in zone %d\n",zn);
	return(-1);}
    zone_table[zn].cmd[num].command = '*';
    /* above new zone purge    
    for (no = 0;;no++)
	if (zone_table[zn].cmd[no].command == 'S')
	    break;
    memmove(&zone_table[zn].cmd[cmd_no],&zone_table[zn].cmd[cmd_no+1],sizeof(struct reset_com)*(no - cmd_no));
    */
    return(cmd_no);
}

int create_obj(int zone)
{
  bool write_index=FALSE;
  int i,zn,next_zone, max;
  struct zone_obj_list *zol, *tmp_zol;
  
  for (zn = 0;zone_table[zn].number != zone && zn <= top_of_zone_table;zn++)
    ;
  next_zone = (zone_table[zn].top +1)/100;
  
  /* if we didn't find an obj then zone must be no object in the zone */
  if (!zone_table[zn].list) {
    write_index=TRUE;
    max = zone_table[zn].number*100;
    CREATE(zol,struct zone_obj_list,1);
    zol->vnum = max;
    zol->next = 0;
    zone_table[zn].list = zol;
  }
  else {
    for (tmp_zol = zone_table[zn].list;tmp_zol;tmp_zol = tmp_zol->next)
      {
	if (tmp_zol->next) {
	  if (!tmp_zol->obj) {
	    max = tmp_zol->vnum;
	    zol = tmp_zol;
	    break;
	  }
	  if ((tmp_zol->next->vnum - tmp_zol->vnum) > 1)
	    {
	      max = tmp_zol->vnum +1;
	      CREATE(zol,struct zone_obj_list,1);
	      zol->vnum = max;
	      zol->next = tmp_zol->next;
	      tmp_zol->next = zol;
	      break;
	    }
	  else
	    continue;
	}
	else{
	  if ((tmp_zol->vnum + 1) <= zone_table[zn].top) {
	    max = tmp_zol->vnum +1;
	    CREATE(zol,struct zone_obj_list,1);
	    zol->vnum = max;
	    zol->next = tmp_zol->next;
	    tmp_zol->next = zol;
	    break;
	  }
	  else
	    return(0);
	}
      }
  }
  if (!(obj_index = (struct index_data *) realloc (obj_index, sizeof(struct index_data)*(top_of_objt + 2)))) {
    printf("Realloc error - obj index\n");
    return(0);
  }
  if (!(obj_proto = (struct obj_data *) realloc (obj_proto, sizeof(struct obj_data)*(top_of_objt + 2)))) {
    printf("Realloc error - obj proto\n");
    return(0);
  }
  
  top_of_objt++;
  i = top_of_objt;
  clear_object(obj_proto + i); 
  obj_index[i].virtual = max;    
  obj_index[i].number = 0;
  obj_index[i].func = 0;
  obj_proto[i].item_number = i;    
  zol->obj = &obj_proto[i];
  make_dummy_obj(i);
  if (write_index){
    save_objects(zone);
    write_db_index(DB_BOOT_OBJ);
  }
  return(i);
  
}
void write_db_index(int which)
{
  FILE *index;
  int zn=0, ierr;
  char *prefix, *suffix;

   switch (which) {
   case DB_BOOT_WLD:
     prefix = WLD_PREFIX;
     suffix = ".wld";
     break;
   case DB_BOOT_MOB:
     prefix = MOB_PREFIX;
     suffix = ".mob";
     break;
   case DB_BOOT_OBJ:
     prefix = OBJ_PREFIX;
     suffix = ".obj";
     break;
   case DB_BOOT_ZON:
     prefix = ZON_PREFIX;
     suffix = ".zon";
     break;
   case DB_BOOT_SHP:
     prefix = SHP_PREFIX;
     suffix = ".shp";
     break;
   default:
      logg("SYSERR: Unknown subcommand to write_db_index!");
      exit(1);
      break;
   }
  
  sprintf(buf2,"%s/index",prefix);
  sprintf(buf1,"%s/index.bk",prefix);
  ierr = remove(buf1);
  ierr = rename(buf2, buf1);
  
  if (!(index = fopen(buf2,"w"))){
    sprintf(buf, "Error opening index file '%s' for writing", buf2);
    perror(buf);
    return;
  }
  for (zn =0;zn <= top_of_zone_table;zn++)
    fprintf(index,"%d%s\n",zone_table[zn].number, suffix);
  fprintf(index,"$\n");
  fflush(index);
  fclose(index);
  return;

}
int create_zone(int zone){
  int i,j,next_zn,zn;

  /* it is the job of do_zcreate to make sure the zone won't overlap
     an existing zone */

  if (!(zone_table = (struct zone_data *) realloc(zone_table, sizeof(struct zone_data)*(top_of_zone_table +2)))){
    printf("Realloc error enlarging zone table\n");
    return(0);
  }
  
  for (zn = 0; zone_table[zn].number < zone && zn <= top_of_zone_table; zn++)
    ;
  if(zn <= top_of_zone_table){
    memmove((zone_table + zn+1), (zone_table + zn), sizeof(struct zone_data)*
	    (top_of_zone_table - zn +1));
    zone_table[zn].top = zone_table[zn + 1].number*100 -1;
  }
  else
    zone_table[zn].top = (zone + 10)*100 -1;
  zone_table[zn].name = "New Zone";
  zone_table[zn].number = zone;
  zone_table[zn].lifespan = 60;
  zone_table[zn].reset_mode = 0;
  zone_table[zn].open = 0;
  CREATE(zone_table[zn].cmd,struct reset_com, 1) ;
  zone_table[zn].cmd[0].command = 'S';
  zone_table[zn].list = 0;
  top_of_zone_table++;
  for (i=0;i<=top_of_world;i++)
    if (world[i].zone >= zn)
      world[i].zone++;
  save_zone(zone);
  write_db_index(DB_BOOT_ZON);
  return(zn);
}
int create_mob(int zone)
{
  bool write_index=FALSE;
  int j,i,cmd_no,zn,next_zone;
  struct char_data *ch;
  
  for (zn = 0; zone_table[zn].number != zone && zn <= top_of_zone_table; zn++)
    ;
  if (zn +1 <= top_of_zone_table)
    next_zone = zone_table[zn+1].number;
  else
    next_zone = zone +10;
  for (i=0 ; mob_index[i].virtual < next_zone*100 && i <= top_of_mobt; i++)
    ;
  /* if we didn't find a mob then zone must be new higest zone */
  if (i == top_of_mobt && mob_index[i].virtual < next_zone*100)
    i++;    
  if (mob_index[i-1].virtual >= next_zone*100 -1)
    {
      return(0);
    }
  
  if (mob_index[i-1].virtual < zone*100 ){
    write_index = TRUE;
    printf("No mobs in this zone. Creating mob # %d\n",zone*100);
  }
  if (!(mob_index = (struct index_data *) realloc (mob_index, sizeof(struct index_data)*(top_of_mobt + 2)))){
    printf("Realloc error - mob index\n");
    return;
  }
  if (!(mob_proto = (struct char_data *) realloc (mob_proto, sizeof(struct char_data)*(top_of_mobt + 2)))){
    printf("Realloc error - mob proto\n");
    return;
    }
  
  if ( i <= top_of_mobt ){
    memmove((mob_index+i+1),(mob_index+i),sizeof(struct index_data)*(top_of_mobt-i+1));
    memmove((mob_proto+i+1),(mob_proto+i),sizeof(struct char_data)*(top_of_mobt-i+1));
  }
  
  if (mob_index[i-1].virtual/100 < zone)
    mob_index[i].virtual = zone*100;
  else
    mob_index[i].virtual = mob_index[i-1].virtual + 1;
  
  mob_index[i].number = 0;
  mob_index[i].func = 0;
  mob_proto[i].nr = i;    
  mob_proto[i].desc = 0;
  
  clear_char(mob_proto + i);
  
  top_of_mobt++;
  
  for (ch = character_list;ch;ch = ch->next)
    if (ch->nr >=i)
      ch->nr++;
  
  for (j=0;j<=number_of_shops;j++)
    if (shop_index[j].keeper >= i)
      shop_index[j].keeper++;
  
  for (j=0 ;j<= top_of_mobt; j++)
    mob_proto[j].nr = j;
  /* correct the zone resets */
  for (zn = 0;zn <= top_of_zone_table;zn++)
    for (cmd_no = 0; ;cmd_no++)
      if (ZCMD.command == 'S')
	break;
      else if (ZCMD.command == 'M' || ZCMD.command == 'm'  && ZCMD.arg1 >= i)
	ZCMD.arg1++;
      else if (ZCMD.command == 'F' || ZCMD.command == 'f'  && ZCMD.arg1 >= i)
	ZCMD.arg1++;
      else if (ZCMD.command == 'R' || ZCMD.command == 'r'  && ZCMD.arg1 >= i)
	ZCMD.arg1++;
      else if (ZCMD.command == 'H' || ZCMD.command == 'h'  && ZCMD.arg1 >= i)
	ZCMD.arg1++;
  make_dummy_mob(i);
  if (write_index){
    save_mobiles(zone);
    write_db_index(DB_BOOT_MOB);
  }
  return(i);

}
void make_dummy_obj(int i)
{
    int j;
    char bf[MAX_STRING_LENGTH];
    strcpy(bf,"dummy object");
    CREATE(obj_proto[i].name, char, strlen(bf) + 1);
    strcpy(obj_proto[i].name,bf);
    strcpy(bf,"a dummy object");
    CREATE(obj_proto[i].short_description, char, strlen(bf) + 1);
    strcpy(obj_proto[i].short_description,bf);
    strcpy(bf,"A dangerous looking dummy object.\n");
    CREATE(obj_proto[i].description, char, strlen(bf) + 1);
    strcpy(obj_proto[i].description,bf);
    obj_proto[i].action_description = 0;
    obj_proto[i].item_number = i;
    obj_proto[i].next_content = 0;
    obj_proto[i].carried_by = 0;
    obj_proto[i].in_obj = 0;
    obj_proto[i].contains = 0;    
    obj_proto[i].in_room  = NOWHERE;
    obj_proto[i].obj_flags.type_flag = ITEM_TRASH;
    obj_proto[i].obj_flags.extra_flags = 0;
    obj_proto[i].obj_flags.bitvector = 0;
    obj_proto[i].obj_flags.wear_flags = 0;
    obj_proto[i].obj_flags.weight = 100;
    obj_proto[i].obj_flags.cost = 10000;
    obj_proto[i].obj_flags.timer = -1;
    obj_proto[i].obj_flags.cost_per_day = 10000;
    for (j = 0 ; j < MAX_OBJ_AFFECT ; j++) {
	obj_proto[i].affected[j].location = 0;
	obj_proto[i].affected[j].modifier = 0;
    }
    obj_proto[i].ex_description = 0;
    for (j=0;j<8;j++)
	obj_proto[i].obj_flags.value[j] = 0;

}
void make_dummy_room(int i)
{
    int j;
    char bf[MAX_STRING_LENGTH];
    strcpy(bf,"An empty Room");
    CREATE(world[i].name, char, strlen(bf) + 1);
    strcpy(world[i].name,bf);
    strcpy(bf,"You are standing in an empty room that needs to get a description\r\n");
    CREATE(world[i].description, char, strlen(bf) + 1);
    strcpy(world[i].description,bf);
    world[i].room_flags = UNFINISHED;
    world[i].sector_type = 0;
    world[i].light = 0;
    world[i].funct = 0;
    if (world[i].ex_description){
	world[i].ex_description = 0;
    }
    for (j=0;j<=5;j++)
	if (world[i].dir_option[j])
	    {
		world[i].dir_option[j]=0;
	    }
}
void free_mob(struct char_data *mob)
{
    free(mob->player.name);
    free(mob->player.short_descr);
    free(mob->player.long_descr);
    free(mob->player.description);
    free(mob);
}
void copy_mob(struct char_data *from, struct char_data *to)
{
    /*   if (to->player.name != from->player.name)
	free(to->player.name);
    if (to->player.short_descr != from->player.short_descr)
	free(to->player.short_descr);
    if (to->player.long_descr != from->player.long_descr)
	free(to->player.long_descr);    
    if (to->player.description != from->player.description)
	free(to->player.description);    
    
    to->player = from->player; */
    
    to->nr = from->nr;
    to->specials2 = from->specials2;
    to->specials = from->specials;    
    to->abilities = from->abilities;
    to->tmpabilities = from->tmpabilities;
    to->points = from->points;
    to->affected = from->affected;
    copy_stringinfo(&to->player.name, &from->player.name);
    copy_stringinfo(&to->player.short_descr, &from->player.short_descr);
    copy_stringinfo(&to->player.long_descr, &from->player.long_descr);
    copy_stringinfo(&to->player.description, &from->player.description);
    to->player.sex = from->player.sex;
    to->player.race = from->player.race;
    to->player.level = from->player.level;
    to->player.height = from->player.height;
    to->player.weight = from->player.weight;                

/*    to->player.name =0;
    to->player.short_descr =0;    
    to->player.long_descr =0;
    to->player.description =0;
    
    if (from->player.name){
	CREATE(to->player.name, char,strlen(from->player.name)+1);
	strcpy(to->player.name, from->player.name);}
    if (from->player.short_descr){
	CREATE(to->player.short_descr,char,strlen(from->player.short_descr)+1);
	strcpy(to->player.short_descr, from->player.short_descr);}
    if (from->player.long_descr){
	CREATE(to->player.long_descr, char,strlen(from->player.long_descr)+1);
	strcpy(to->player.long_descr, from->player.long_descr);}
    if (from->player.description){
	CREATE(to->player.description,char,strlen(from->player.description)+1);
	strcpy(to->player.description, from->player.description);} */
}
void make_dummy_mob(int i)
{
    int j;
    char bf[MAX_STRING_LENGTH];

    strcpy(bf,"dummy mobile mob scary tough monster");
    CREATE(mob_proto[i].player.name, char, strlen(bf) + 1);
    strcpy(mob_proto[i].player.name,bf);
    strcpy(bf,"a dummy mobile");
    CREATE(mob_proto[i].player.short_descr, char, strlen(bf) + 1);
    strcpy(mob_proto[i].player.short_descr,bf);
    strcpy(bf,"A really scary and tough looking monster is here.\n");
    CREATE(mob_proto[i].player.long_descr, char, strlen(bf) + 1);
    strcpy(mob_proto[i].player.long_descr,bf);
    strcpy(bf,"The really scary and tough looking monster is looking for a better description.\n");
    CREATE(mob_proto[i].player.description, char, strlen(bf) + 1);
    strcpy(mob_proto[i].player.description,bf);        
    mob_proto[i].player.title = 0;
    MOB_FLAGS(mob_proto +i) = 0;
    SET_BIT(MOB_FLAGS(mob_proto +i), MOB_ISNPC);
    mob_proto[i].specials.affected_by = 0;
    mob_proto[i].abilities.str   = 14;
    mob_proto[i].abilities.intel = 14;
    mob_proto[i].abilities.wis   = 14;
    mob_proto[i].abilities.dex   = 14;
    mob_proto[i].abilities.con   = 14;
    mob_proto[i].abilities.chr   = 14;
    mob_proto[i].abilities.per   = 14;
    mob_proto[i].abilities.gui   = 14;
    mob_proto[i].abilities.foc   = 14;
    mob_proto[i].abilities.dev   = 14;
    mob_proto[i].abilities.luc   = 14;
    
    GET_LEVEL(mob_proto + i) = 0;
    GET_GOLD(mob_proto + i) = 0;
    GET_ALIGNMENT(mob_proto + i) = 0;
    mob_proto[i].points.hitroll = 20;
    mob_proto[i].points.armor[0] = 100;
    mob_proto[i].points.armor[1] = 100;
    mob_proto[i].points.armor[2] = 100;
    mob_proto[i].points.armor[3] = 100;
    
    mob_proto[i].points.max_hit = 0;
    mob_proto[i].points.hit = 1;
    mob_proto[i].points.mana = 1;
    mob_proto[i].points.move = 1;

    mob_proto[i].points.max_mana = 100;
    mob_proto[i].points.max_move = 500;
    
    mob_proto[i].specials.damnodice = 1;
    mob_proto[i].specials.damsizedice = 1;
    mob_proto[i].points.damroll = 1;

    mob_proto[i].specials.position = 8;
    mob_proto[i].specials.default_pos = 8;
    mob_proto[i].player.sex = 0;
    
    mob_proto[i].player.race = 0;
    
    mob_proto[i].player.weight = 2000;
    mob_proto[i].player.height = 198;
    
    for (j = 0; j < 3; j++)
	GET_COND(mob_proto + i, j) = -1;
    
    for (j = 0; j < 5; j++)
	mob_proto[i].specials2.apply_saving_throw[j] = 
	    MAX(100 - GET_LEVEL(mob_proto + i), 2);
    
    mob_proto[i].tmpabilities = mob_proto[i].abilities;
    
    for (j = 0; j < MAX_WEAR; j++) /* Initialising Ok */
	mob_proto[i].equipment[j] = 0;
}
void clone_room(int from, int to)
{
    char bf[MAX_STRING_LENGTH];
    
    world[to].number = world[to-1].number +1;
    world[to].zone = world[to-1].zone;
    sprintf(bf,"%s",world[from].name);
    CREATE(world[to].name, char, strlen(bf) + 1);
    strcpy(world[to].name,bf);
    sprintf(bf,"%s",world[from].description);
    CREATE(world[to].description, char, strlen(bf) + 1);
    strcpy(world[to].description,bf);
    world[to].tele_delay = -1;
}
void free_shop(struct shop_data *shop)
{
  free(shop->no_such_item1);
  free(shop->no_such_item2);
  free(shop->do_not_buy);
  free(shop->missing_cash1);
  free(shop->missing_cash2);
  free(shop->message_buy);
  free(shop->message_sell);
  free(shop);
}
void free_zone(struct zone_data *zone){
  free(zone->name);
  free(zone);
}
void free_room(struct room_data *room)
{
    int i;
    
    free(room->name);
    free(room->description);
    free(room->tele_mesg1);
    free(room->tele_mesg2);
    free(room->tele_mesg3);        
    for (i=0;i<=5;i++)
	if (room->dir_option[i]){
	    free(room->dir_option[i]->general_description);
	    free(room->dir_option[i]->keyword);
	    free(room->dir_option[i]);
	}
    free(room);

}
void copy_stringinfo(char **to, char **from)
{
  if (*to){
    if (*from){
      if ((*to != *from) && strcmp(*to,*from)){
	RECREATE(*to, char, strlen(*from) + 1);
	strcpy(*to, *from);}
    }
    /*    else
      free(*to);	*/
  }
  else if (*from){
    CREATE(*to, char , strlen(*from) + 1);
    strcpy(*to, *from);}
  return;
}
void copy_zone(struct zone_data *from, struct zone_data *to)
{
  copy_stringinfo(&to->name, &from->name);
  to->top = from->top;
  to->number = from->number;
  to->reset_mode = from->reset_mode;
  to->lifespan = from->lifespan;  
  to->open = from->open;
}

void copy_shop(struct shop_data *from, struct shop_data *to)
{
  int i;
  
  for (i=0;i< MAX_PROD;i++)
    to->producing[i] = from->producing[i];
  to->virtual = from->virtual;
  to->in_room = from->in_room;
  to->temper1 = from->temper1;
  to->temper2 = from->temper2;
  to->open1 = from->open1;
  to->open2 = from->open2;
  to->close1 = from->close1;
  to->close2 = from->close2;    
  to->keeper = from->keeper;
  to->profit_buy = from->profit_buy;
  to->profit_sell = from->profit_sell;
  to->tradetype = from->tradetype;
  to->with_who = from->with_who;
  copy_stringinfo(&to->no_such_item1, &from->no_such_item1);
  copy_stringinfo(&to->no_such_item2, &from->no_such_item2);  
  copy_stringinfo(&to->do_not_buy, &from->do_not_buy);
  copy_stringinfo(&to->missing_cash1, &from->missing_cash1);
  copy_stringinfo(&to->missing_cash2, &from->missing_cash2);
  copy_stringinfo(&to->message_buy, &from->message_buy);
  copy_stringinfo(&to->message_sell, &from->message_sell);
}

void copy_room(struct room_data *from, struct room_data *to)
{
    int i;

    to->number = from->number;
    to->zone = from->zone;
    to->sector_type = from->sector_type;
    to->room_flags = from->room_flags;
    to->tele_delay = -1;
    to->funct = from->funct;
    if (from->tele_delay > -1){
        to->tele_delay = from->tele_delay;
        to->tele_to_room = from->tele_to_room;
	copy_stringinfo(&to->tele_mesg1, &from->tele_mesg1);
	copy_stringinfo(&to->tele_mesg2, &from->tele_mesg2);
	copy_stringinfo(&to->tele_mesg3, &from->tele_mesg3);	
    }	
    /* first copy the string info  making sure to free up old string info */
    /* and only copy if the pointers are different AJN 12th Oct 94*/
    copy_stringinfo(&to->name, &from->name);
    copy_stringinfo(&to->description, &from->description);
    /* if extra's have changed copy them freeing the old ones*/
    copy_extras(&to->ex_description, from->ex_description);
    /* now carefully copy exit info  freeing up any existing info*/
    for (i=0;i<=5;i++)    
	if (to->dir_option[i]){
	    free(to->dir_option[i]->keyword);
	    free(to->dir_option[i]);
	    to->dir_option[i]=0;
	}
    for (i=0;i<=5;i++)
	if (from->dir_option[i]){
	    CREATE(to->dir_option[i], struct room_direction_data, 1);
	    if (from->dir_option[i]->general_description){
		CREATE(to->dir_option[i]->general_description, char,
		       strlen(from->dir_option[i]->general_description)+1);
		strcpy(to->dir_option[i]->general_description,
		       from->dir_option[i]->general_description);}
	    if (from->dir_option[i]->keyword){
		    CREATE(to->dir_option[i]->keyword, char,
			   strlen(from->dir_option[i]->keyword)+1);
		    strcpy(to->dir_option[i]->keyword,
			   from->dir_option[i]->keyword);}
	    to->dir_option[i]->exit_info = from->dir_option[i]->exit_info;
	    to->dir_option[i]->key = from->dir_option[i]->key;
	    to->dir_option[i]->to_room = from->dir_option[i]->to_room;
	}
    return;
}
void copy_extras(struct extra_descr_data **target, struct extra_descr_data *exfrm )
{
    struct extra_descr_data *exto=0, *tmp=0;
    
    if (*target)
	for(exto = *target;exto;exto = tmp)
	  {
	    tmp = exto->next;
	    free(exto->keyword);
	    free(exto->description);
	    free(exto);
	  }
    *target = 0;
    
    while(exfrm)
      {
	CREATE(exto, struct extra_descr_data, 1);
	exto->keyword = 0;
	exto->description = 0;
	exto->next = 0;
	if (exfrm->keyword)
	  {
	    CREATE(exto->keyword, char, strlen(exfrm->keyword) +1);
	    strcpy(exto->keyword,exfrm->keyword);
	  }
	if (exfrm->description)
	  {
	    CREATE(exto->description, char, strlen(exfrm->description) +1);
	    strcpy(exto->description, exfrm->description);
	  }
	exto->next = *target;
	*target = exto;
	exfrm = exfrm->next;
    }
    
}

void free_objstruct(struct obj_data *obj)
{
    free(obj->name);
    free(obj->short_description);
    free(obj->description);
    free(obj->action_description);
    free(obj);

}
void  copy_obj(struct obj_data *from, struct obj_data *to, bool cpyextras)
{
    int i;
    
    to->item_number = from->item_number;
    to->obj_flags = from->obj_flags;
    to->obj_flags2 = from->obj_flags2;
    copy_stringinfo(&to->name,&from->name);
    copy_stringinfo(&to->short_description, &from->short_description);
    copy_stringinfo(&to->description, &from->description);
    copy_stringinfo(&to->action_description, &from->action_description); 
    for (i=0;i<MAX_OBJ_AFFECT;i++)
	to->affected[i] = from->affected[i];
    if (cpyextras)
      copy_extras(&to->ex_description,from->ex_description);
      /*    if (!to->ex_description)
	    to->ex_description = from->ex_description; */

}

void clone_mob(int from, int to)
{
    char bf[MAX_STRING_LENGTH];

    memcpy((mob_proto + to),(mob_proto + from),sizeof(struct char_data));
    mob_proto[to].nr = to;

    sprintf(bf,"clone %s",mob_proto[from].player.name);
    CREATE(mob_proto[to].player.name, char, strlen(bf) + 1);
    strcpy(mob_proto[to].player.name,bf);
    sprintf(bf,"a clone of %s",mob_proto[from].player.short_descr);
    CREATE(mob_proto[to].player.short_descr, char, strlen(bf) + 1);
    strcpy(mob_proto[to].player.short_descr,bf);
    sprintf(bf,"A clone of %s",mob_proto[from].player.long_descr);
    CREATE(mob_proto[to].player.long_descr, char, strlen(bf) + 1);
    strcpy(mob_proto[to].player.long_descr,bf);

    strcpy(bf,mob_proto[from].player.description);
    CREATE(mob_proto[to].player.description, char, strlen(bf) + 1);
    strcpy(mob_proto[to].player.description,bf);        
    
}
int     save_zone(int zone)
{
    int zn,ierr,cmd_no;
    FILE *zone_f;

   sprintf(buf2,"%s/%d.zon.bk",ZON_PREFIX,zone);
   sprintf(buf,"%s/%d.zon",ZON_PREFIX,zone);
   ierr = remove(buf2);
   ierr = rename(buf,buf2);
   
   sprintf(buf2," Error saving zone: %d",zone);
   
   if (!(zone_f = fopen(buf, "w"))){
       perror(buf2);
       exit(1);}
    

    for (zn = 0; zone_table[zn].number != zone && zn <= top_of_zone_table; zn++)
	if (zn > top_of_zone_table)
	    {
		/* illegal zone */
		return(-1);
	    }
    fprintf(zone_f,"#%d\n",zone_table[zn].number);
    fwrite_string(zone_f,zone_table[zn].name);
    fprintf(zone_f,"%d %d %d %d\n",zone_table[zn].top,zone_table[zn].lifespan,
       zone_table[zn].reset_mode, zone_table[zn].open);
    for (cmd_no = 0; ;cmd_no++){
	if (ZCMD.command == 'S'){
	    fprintf (zone_f,"S\n");
	    break;}
	else if (ZCMD.command == 'M')
	    fprintf(zone_f,"m %d %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,1,world[ZCMD.arg3].number, ZCMD.percent);
	else if (ZCMD.command == 'm')
	    fprintf(zone_f,"m %d %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,ZCMD.arg2,world[ZCMD.arg3].number, ZCMD.percent);
	else if (ZCMD.command == 'E')
	    fprintf(zone_f,"e %d %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,1,ZCMD.arg3, ZCMD.percent);
	else if (ZCMD.command == 'e')
	    fprintf(zone_f,"e %d %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,ZCMD.arg2,ZCMD.arg3, ZCMD.percent);
	else if (ZCMD.command == 'D')
	    fprintf(zone_f,"d %d %d %d %d %d\n",ZCMD.if_flag,world[ZCMD.arg1].number,ZCMD.arg2,ZCMD.arg3, ZCMD.percent);	
	else if (ZCMD.command == 'd')
	    fprintf(zone_f,"d %d %d %d %d %d\n",ZCMD.if_flag,world[ZCMD.arg1].number,ZCMD.arg2,ZCMD.arg3, ZCMD.percent);	
	else if (ZCMD.command == 'O')
	    fprintf(zone_f,"o %d %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,1,world[ZCMD.arg3].number, ZCMD.percent);	
	else if (ZCMD.command == 'o')
	    fprintf(zone_f,"o %d %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,ZCMD.arg2,world[ZCMD.arg3].number, ZCMD.percent);	
	else if (ZCMD.command == 'P')
	    fprintf(zone_f,"p %d %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,1,obj_index[ZCMD.arg3].virtual, ZCMD.percent);	
	else if (ZCMD.command == 'p')
	    fprintf(zone_f,"p %d %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,ZCMD.arg2,obj_index[ZCMD.arg3].virtual, ZCMD.percent);	
	else if (ZCMD.command == 'G')
	    fprintf(zone_f,"g %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,1, ZCMD.percent);
	else if (ZCMD.command == 'g')
	    fprintf(zone_f,"g %d %d %d %d\n",ZCMD.if_flag,obj_index[ZCMD.arg1].virtual,ZCMD.arg2, ZCMD.percent);
	else if (ZCMD.command == 'F')
	    fprintf(zone_f,"f %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,1, ZCMD.percent);	
	else if (ZCMD.command == 'f')
	    fprintf(zone_f,"f %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,ZCMD.arg2, ZCMD.percent);	
	else if (ZCMD.command == 'R')
	    fprintf(zone_f,"r %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,1, ZCMD.percent);	
	else if (ZCMD.command == 'r')
	    fprintf(zone_f,"r %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,ZCMD.arg2, ZCMD.percent);	
	else if (ZCMD.command == 'C')
	    fprintf(zone_f,"c %d %d %d &d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,1, ZCMD.percent);	
	else if (ZCMD.command == 'c')
	    fprintf(zone_f,"c %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,ZCMD.arg2, ZCMD.percent);	
	else if (ZCMD.command == 'H')
	    fprintf(zone_f,"h %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,1, ZCMD.percent);
	else if (ZCMD.command == 'h')
	    fprintf(zone_f,"h %d %d %d %d\n",ZCMD.if_flag,mob_index[ZCMD.arg1].virtual,ZCMD.arg2, ZCMD.percent);
	else
	  continue;
    }
    fprintf(zone_f,"#99999\n$~\n");
    fflush(zone_f);
    fclose(zone_f);
    return(cmd_no);
}
int     save_shops(int zone)
{

  int ierr, i,j, nsaved=0;
  FILE *shp_f;
  
  sprintf(buf2,"%s/%d.shp.bk",SHP_PREFIX,zone);
  sprintf(buf,"%s/%d.shp",SHP_PREFIX,zone);
  ierr = remove(buf2);
  ierr = rename(buf,buf2);
  
  sprintf(buf2," Error saving shops in zone: %d",zone);  

  if (!(shp_f = fopen(buf, "w"))){
       printf(buf2);
       exit(1);
  }
  for (i=0;i<= number_of_shops; i++)
      if (shop_index[i].virtual/100 == zone)
	  {
	    nsaved++;
	    fprintf(shp_f, "#%d NEW~\n", shop_index[i].virtual);
	    for (j=0;j< MAX_PROD; j++) {
	      if (shop_index[i].producing[j] >= 0)
		fprintf(shp_f, "%d \n",
			obj_index[shop_index[i].producing[j]].virtual);
	      else
		fprintf(shp_f, "%d \n",shop_index[i].producing[j]);
	    }
	    fprintf(shp_f, "%f \n", shop_index[i].profit_buy);
	    fprintf(shp_f, "%f \n", shop_index[i].profit_sell);
	    fprintf(shp_f, "%d \n", shop_index[i].tradetype);
	    fwrite_string(shp_f, shop_index[i].no_such_item1);
	    fwrite_string(shp_f, shop_index[i].no_such_item2);
	    fwrite_string(shp_f, shop_index[i].do_not_buy);
	    fwrite_string(shp_f, shop_index[i].missing_cash1);
	    fwrite_string(shp_f, shop_index[i].missing_cash2);
	    fwrite_string(shp_f, shop_index[i].message_buy);
	    fwrite_string(shp_f, shop_index[i].message_sell);
	    fprintf(shp_f, "%d \n", shop_index[i].temper1);
	    fprintf(shp_f, "%d \n", shop_index[i].temper2);
	    fprintf(shp_f, "%d \n", mob_index[shop_index[i].keeper].virtual);
	    fprintf(shp_f, "%d \n", shop_index[i].with_who);
	    fprintf(shp_f, "%d \n", shop_index[i].in_room); 
	    fprintf(shp_f, "%d \n", shop_index[i].open1);
	    fprintf(shp_f, "%d \n", shop_index[i].close1);
	    fprintf(shp_f, "%d \n", shop_index[i].open2);
	    fprintf(shp_f, "%d \n", shop_index[i].close2);
    	  }
  fprintf(shp_f, "$~\n");
  fflush(shp_f);
  fclose(shp_f);
  return(nsaved);
}
int   	save_mobiles(int zone)
{
   int	j,i,ii,jj, ierr, attck,zn,next_zone;
   FILE *mob_f;
   
   sprintf(buf2,"%s/%d.mob.bk",MOB_PREFIX,zone);
   sprintf(buf,"%s/%d.mob",MOB_PREFIX,zone);
   ierr = remove(buf2);
   ierr = rename(buf,buf2);
   
   sprintf(buf2," Error saving mobs in zone: %d",zone);
   
   if (!(mob_f = fopen(buf, "w"))){
       printf(buf2);
       exit(1);}
   for (zn = 0; zone_table[zn].number != zone && zn <= top_of_zone_table; zn++)
	   ;
   if (zn +1 <= top_of_zone_table)
       next_zone = zone_table[zn+1].number;
   else
       next_zone = zone +10;
   for (i = 0,j=0; mob_index[i].virtual < zone*100 && i <= top_of_mobt; i++)
       j++;
   for (i=j ; mob_index[i].virtual < next_zone*100 && i <= top_of_mobt; i++)
       {
	   fprintf(mob_f,"#%d\n",mob_index[i].virtual);
	   fwrite_string(mob_f,mob_proto[i].player.name);
	   fwrite_string(mob_f,mob_proto[i].player.short_descr);
	   fwrite_string(mob_f,mob_proto[i].player.long_descr);
	   fwrite_string(mob_f,mob_proto[i].player.description);


	 fprintf(mob_f, "%ld ", MOB_FLAGS(mob_proto +i));
	 fprintf(mob_f, "%ld %d E\n", mob_proto[i].specials.affected_by,GET_ALIGNMENT(mob_proto + i));

	   fprintf(mob_f ,"%d %d %d %d %d\n",
		   GET_RAW_STR(mob_proto + i),
		   GET_RAW_INT(mob_proto + i),
		   GET_RAW_WIS(mob_proto + i),
		   GET_RAW_DEX(mob_proto + i),
		   GET_RAW_CON(mob_proto + i));
	   
	   fprintf(mob_f ,"%d %d %d %d %d\n",
		   GET_RAW_CHR(mob_proto + i),
		   GET_RAW_PER(mob_proto + i),
		   GET_RAW_GUI(mob_proto + i),
		   GET_RAW_DEV(mob_proto + i),
		   GET_RAW_FOC(mob_proto + i));

	   fprintf(mob_f ,"%d %d %d\n",
		   mob_proto[i].points.max_power,
		   mob_proto[i].points.max_mana,
		   mob_proto[i].points.max_move);
		   
	   fprintf(mob_f, "%d %d %d %d\n",
		   mob_proto[i].player.race,
		   GET_RAW_LUC(mob_proto + i),
		   GET_LEVEL(mob_proto + i),
		   20 - mob_proto[i].points.hitroll);


	   if (mob_proto[i].specials.attack_type >= 300)
	       attck = mob_proto[i].specials.attack_type - 300;
	   else
	       attck = mob_proto[i].specials.attack_type;

	   fprintf(mob_f, "%d %d %d\n",
		   attck,
		   mob_proto[i].player.height,
		   mob_proto[i].player.weight);

	   fprintf(mob_f, "%dd%d+%d",
		   mob_proto[i].points.hit,
		   mob_proto[i].points.mana,
		   mob_proto[i].points.move);

	   fprintf(mob_f, " %dd%d+%d\n",
		   mob_proto[i].specials.damnodice,
		   mob_proto[i].specials.damsizedice,
		   mob_proto[i].points.damroll);

	   for (ii=0;ii<=3;ii++)
	       fprintf(mob_f,"%d ",mob_proto[i].points.armor[ii]);
	   fprintf(mob_f,"\n");
	   for (ii=0;ii<=3;ii++)
	       fprintf(mob_f,"%d ",mob_proto[i].points.stopping[ii]);
	   fprintf(mob_f,"\n");	   
	   for (ii=0;ii<=4;ii++)
	       fprintf(mob_f,"%d ",mob_proto[i].specials2.apply_saving_throw[ii]);
	   fprintf(mob_f,"\n");	   
	   
	   fprintf(mob_f, "%d %d\n",
		   GET_GOLD(mob_proto + i),
		   GET_FAME(mob_proto + i));

	    fprintf(mob_f, "%d %d %d\n",
		    mob_proto[i].specials.position,
		    mob_proto[i].specials.default_pos,
		    mob_proto[i].player.sex);
	   
	   for (jj = 0; jj < 3; jj++) 
	       fprintf(mob_f, "%d ", GET_COND(mob_proto + i, jj));
	   fprintf(mob_f,"\n");

       }
   fprintf(mob_f,"#99999\n$~\n");
   fflush(mob_f);
   fclose(mob_f);
   return(i-j);

}

int   	save_objects(int zone)
{
  int	j, i, zn;
  struct zone_obj_list *z_obj;
  int ierr,jj;
  struct extra_descr_data *desc;
  FILE *obj_f;
  
  sprintf(buf2,"%s/%d.obj.bk",OBJ_PREFIX,zone);
  sprintf(buf,"%s/%d.obj",OBJ_PREFIX,zone);
  ierr = remove(buf2);
  ierr = rename(buf,buf2);
  
  sprintf(buf2," Error saving zone: %d",zone);
  
   if (!(obj_f = fopen(buf, "w"))){
     printf(buf2);
     exit(1);}
   for (zn = 0; zone_table[zn].number != zone && zn <= top_of_zone_table; zn++)
     ;
   for(j=0,z_obj = zone_table[zn].list;z_obj;z_obj= z_obj->next,j++){
     i = real_object(z_obj->vnum);
     fprintf(obj_f,"#%d %d\n",obj_index[i].virtual, PROTO_REVISION);
     fwrite_string(obj_f,obj_proto[i].name);
     fwrite_string(obj_f,obj_proto[i].short_description);
     fwrite_string(obj_f,obj_proto[i].description);
     fwrite_string(obj_f,obj_proto[i].action_description);
     
     fprintf(obj_f, "%d %ld %d %ld\n",
	     obj_proto[i].obj_flags.type_flag,
	     obj_proto[i].obj_flags.extra_flags,
	     obj_proto[i].obj_flags.wear_flags,
	     obj_proto[i].obj_flags2.perm_aff);
     
     fprintf(obj_f, "%d %d %d %d\n%d %d %d %d\n", 
	     obj_proto[i].obj_flags.value[0],
	     obj_proto[i].obj_flags.value[1],
	     obj_proto[i].obj_flags.value[2],
	     obj_proto[i].obj_flags.value[3],
	     obj_proto[i].obj_flags.value[4],
	     obj_proto[i].obj_flags.value[5],
	     obj_proto[i].obj_flags.value[6],
	     obj_proto[i].obj_flags.value[7]);
     
     
     fprintf(obj_f, "%d %d %d %d %d %d\n", 
	     obj_proto[i].obj_flags.weight,
	     obj_proto[i].obj_flags.cost,
	     obj_proto[i].obj_flags.cost_per_day,
	     obj_proto[i].obj_flags2.aff_dur,
	     obj_proto[i].obj_flags2.no_use_dur,
	     obj_proto[i].obj_flags2.light);
     
     for (desc = obj_proto[i].ex_description;desc; desc = desc->next)
       {
	 fprintf(obj_f, "E\n") ;
	 fwrite_string(obj_f, desc->keyword);
	 fwrite_string(obj_f, desc->description);
       }
     for (jj = 0 ; jj < MAX_OBJ_AFFECT; jj++)
       if (obj_proto[i].affected[jj].modifier)
	 {
	   fprintf(obj_f, "A\n") ;
	   fprintf(obj_f, "%d %d\n", 
		   obj_proto[i].affected[jj].location ,
		   obj_proto[i].affected[jj].modifier);
	 }
     
     if(obj_proto[i].obj_flags.bitvector){
       fprintf(obj_f, "B\n") ;
       fprintf(obj_f, "%ld\n", obj_proto[i].obj_flags.bitvector);
     }
   }
   fprintf(obj_f,"#99999\n$~\n");
   fflush(obj_f);
   fclose(obj_f); 
   return(j);
}
/* read all objects from obj file; generate index and prototypes */
void	load_objects(FILE *obj_f)
{
   static int	i = 0;
   long tmp9=0;
   int	tmp=0, tmp2=0, tmp3=0, tmp4=0, j=0, nr=0, tmp5=0, tmp6=0, tmp7=0, tmp8=0, revision=0;
   char	chk[50], *tmpptr=0;
   struct extra_descr_data *new_descr=0;
   static int convert = FALSE;

   if (!fscanf(obj_f, "%s\n", chk)) {
      perror("load_objects");
      exit(1);
   }

   for (; ; ) {
      if (*chk == '#') {
	 sscanf(chk, "#%d\n", &nr);
	 if (nr >= 99999) 
	    break;

	 obj_index[i].virtual = nr;
	 obj_index[i].number  = 0;
	 obj_index[i].func    = 0;

	 clear_object(obj_proto + i);

	 sprintf(buf2, "object #%d", nr);

	 /* *** string data *** */
	 fscanf(obj_f,"%d\n",&revision);
	 tmpptr = obj_proto[i].name = fread_string(obj_f, buf2);
	 if (!tmpptr) 
	     obj_proto[i].name = "null name list";

	 tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
	 if (*tmpptr)
	    if (!str_cmp(fname(tmpptr), "a") || 
	        !str_cmp(fname(tmpptr), "an") || 
	        !str_cmp(fname(tmpptr), "the"))
	       *tmpptr = tolower(*tmpptr);
	 tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
	 if (tmpptr && *tmpptr)
	    *tmpptr = toupper(*tmpptr);
	 
	 obj_proto[i].action_description = fread_string(obj_f, buf2);

	 /* *** numeric data *** */

	 fscanf(obj_f, " %d %d %d %d", &tmp, &tmp2, &tmp3, &tmp4);
	 obj_proto[i].obj_flags.type_flag = tmp;
	 obj_proto[i].obj_flags.extra_flags = tmp2;
	 obj_proto[i].obj_flags.wear_flags = tmp3;
	 obj_proto[i].obj_flags2.perm_aff = tmp4;
	 
	 fscanf(obj_f, " %d %d %d %d %d %d %d %d",
		&tmp, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &tmp7, &tmp8);
	 obj_proto[i].obj_flags.value[0] = tmp;
	 obj_proto[i].obj_flags.value[1] = tmp2;
	 obj_proto[i].obj_flags.value[2] = tmp3;
	 obj_proto[i].obj_flags.value[3] = tmp4;
	 obj_proto[i].obj_flags.value[4] = tmp5;
	 obj_proto[i].obj_flags.value[5] = tmp6;
	 obj_proto[i].obj_flags.value[6] = tmp7;
	 obj_proto[i].obj_flags.value[7] = tmp8;

	 fscanf(obj_f, " %d %d %d %d %d %d",
		&tmp, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6);
	 obj_proto[i].obj_flags.weight = tmp;
	 obj_proto[i].obj_flags.cost = tmp2;
	 obj_proto[i].obj_flags.cost_per_day = tmp3;
	 obj_proto[i].obj_flags.timer = -1;	 
	 obj_proto[i].obj_flags2.aff_dur = tmp4;
	 obj_proto[i].obj_flags2.no_use_dur = tmp5;
	 obj_proto[i].obj_flags2.light = tmp6;
	 
	 /* *** extra descriptions *** */

	 obj_proto[i].ex_description = 0;

	 sprintf(buf2, "%s - extra desc. section", buf2);

	 while (fscanf(obj_f, " %s \n", chk), *chk == 'E') {
	    CREATE(new_descr, struct extra_descr_data, 1);
	    new_descr->keyword = fread_string(obj_f, buf2);
	    new_descr->description = fread_string(obj_f, buf2);
	    new_descr->next = obj_proto[i].ex_description;
	    obj_proto[i].ex_description = new_descr;
	 }

	 for (j = 0 ; (j < MAX_OBJ_AFFECT) && (*chk == 'A') ; j++) {
	    fscanf(obj_f, " %d %d ", &tmp, &tmp2);
	    obj_proto[i].affected[j].location = tmp;
	    obj_proto[i].affected[j].modifier = tmp2;
	    fscanf(obj_f, " %s \n", chk);
	 }
	 if (*chk == 'B')
	     {
	    fscanf(obj_f, " %ld ", &tmp9);
	    obj_proto[i].obj_flags.bitvector = tmp9;
	    fscanf(obj_f, " %s \n", chk);
	 }

	 for (; (j < MAX_OBJ_AFFECT); j++) {
	    obj_proto[i].affected[j].location = APPLY_NONE;
	    obj_proto[i].affected[j].modifier = 0;
	 }

	 obj_proto[i].in_room = NOWHERE;
	 obj_proto[i].next_content = 0;
	 obj_proto[i].carried_by = 0;
	 obj_proto[i].in_obj = 0;
	 obj_proto[i].contains = 0;
	 obj_proto[i].item_number = i;


	 i++;
      } else if (*chk == '$') /* EOF */
	 break;
      else {
         sprintf(buf2, "Format error in obj file at or near obj #%d", nr);
         logg(buf2);
         exit(1);
      }
   }
   top_of_objt = i - 1;
}

int   	save_rooms(int zone)
{
   int	j,i,ii, ierr,zn;
   struct extra_descr_data *desc=0;
   int tmp;
   FILE *room_f;
   
   sprintf(buf2,"%s/%d.wld.bk",WLD_PREFIX,zone);
   sprintf(buf,"%s/%d.wld",WLD_PREFIX,zone);
   ierr = remove(buf2);
   ierr = rename(buf,buf2);
   
   sprintf(buf2," Error saving rooms in zone: %d",zone);
   
   if (!(room_f = fopen(buf, "w"))){
       printf(buf2);
       exit(1);}
   j = zone;
   zn = zone;
   for (zone = 0; zone_table[zone].number != j && zone <= top_of_zone_table; zone++)
       ;
       if (zone > top_of_zone_table)
	       return(-2);

   for (i = 0,j=0; world[i].zone < zone && i <= top_of_world; i++)
       j++;
   for (i=j ; world[i].zone < (zone+1) && i <= top_of_world; i++)
       {
	   fprintf(room_f,"#%d\n",world[i].number);
	   fwrite_string(room_f,world[i].name);
	   fwrite_string(room_f,world[i].description);
	   if (IS_MARKED(i))
	       UNMARK(i);
	   fprintf(room_f,"%d %ld %d\n",
		   zn,
		   world[i].room_flags,
		   world[i].sector_type);
	   for (ii=0;ii<=5;ii++)
	       if (world[i].dir_option[ii]){
		   fprintf(room_f,"d%d\n",ii);
		   fwrite_string(room_f,
				 world[i].dir_option[ii]->general_description);
		   fwrite_string(room_f,
				 world[i].dir_option[ii]->keyword);
		   fprintf(room_f,"%d %d %d\n", world[i].dir_option[ii]->exit_info,
			   world[i].dir_option[ii]->key,
			   world[i].dir_option[ii]->to_room);}
	   if (world[i].tele_delay > -1){
	       fprintf(room_f, "T\n") ;
	       fprintf(room_f,"%d %d\n",world[i].tele_to_room,
		       world[i].tele_delay);
	       fwrite_string(room_f, world[i].tele_mesg1);
	       fwrite_string(room_f, world[i].tele_mesg2);
	       fwrite_string(room_f, world[i].tele_mesg3);}
	   for (desc = world[i].ex_description;desc; desc = desc->next)
	       {
		   fprintf(room_f, "E\n") ;
		   fwrite_string(room_f, desc->keyword);
		   fwrite_string(room_f, desc->description);
	       }

	   fprintf(room_f,"S\n");
			   
	       }
   
   fprintf(room_f,"#99999\n$~\n");
   fflush(room_f);
   fclose(room_f);
   return(i-j);
}
/* load the rooms */
void	load_rooms(FILE *fl)
{
   static int	room_nr = 0, zone = 0, virtual_nr, flag, tmp,tmp2;
   char	*temp, chk[50];
   struct extra_descr_data *new_descr=0;

   do {
      fscanf(fl, " #%d\n", &virtual_nr);

      sprintf(buf2, "room #%d", virtual_nr);

      temp = fread_string(fl, buf2);
      if ( !(temp) || (flag = (*temp != '$'))) {	/* a new record to be read */
	 world[room_nr].number = virtual_nr;
	 if (!(temp))
	     world[room_nr].name = "null";
	 else
	     world[room_nr].name = temp;
	 world[room_nr].description = fread_string(fl, buf2);
	 if (!(world[room_nr].description))
	     	 world[room_nr].description = "null";
	 if (top_of_zone_table >= 0) {
	    fscanf(fl, " %*d ");

	    /* OBS: Assumes ordering of input rooms */

	    if (world[room_nr].number <= (zone ? zone_table[zone-1].top : -1)) {
	       fprintf(stderr, "Room nr %d is below zone %d.\n",
	           room_nr, zone);
	       exit(0);
	    }
	    while (world[room_nr].number > zone_table[zone].top)
	       if (++zone > top_of_zone_table) {
		  fprintf(stderr, "Room %d is outside of any zone.\n",
		      virtual_nr);
		  exit(0);
	       }
	    world[room_nr].zone = zone;
	 }
	 fscanf(fl, " %d ", &tmp);
	 world[room_nr].room_flags = tmp;
	 fscanf(fl, " %d ", &tmp);
	 world[room_nr].sector_type = tmp;
	 world[room_nr].tele_delay = -1;
	 world[room_nr].funct = 0;
	 world[room_nr].contents = 0;
	 world[room_nr].people = 0;
	 world[room_nr].light = 0; /* Zero light sources */
	 if (strlen(world[room_nr].description) < 150)
	     SET_BIT(world[room_nr].room_flags, UNFINISHED);
	 else if (IS_SET(world[room_nr].room_flags, UNFINISHED))
	       REMOVE_BIT(world[room_nr].room_flags, UNFINISHED);
	 if (!strcmp(world[room_nr].name,"An empty Room") ||
	     !strcmp(world[room_nr].name,"A dummy room"))
	     SET_BIT(world[room_nr].room_flags, UNFINISHED);
	 for (tmp = 0; tmp <= 5; tmp++)
	    world[room_nr].dir_option[tmp] = 0;

	 world[room_nr].ex_description = 0;

	 for (; ; ) {
	    fscanf(fl, " %s \n", chk);

	    if (*chk == 'D')  /* direction field */
	       setup_dir(fl, room_nr, atoi(chk + 1),0);
	    else if (*chk == 'd')  /* new direction field */
	      setup_dir(fl, room_nr, atoi(chk + 1),1);
	    else if (*chk == 'E')  /* extra description field */ {
	       CREATE(new_descr, struct extra_descr_data, 1);
	       new_descr->keyword = fread_string(fl, buf2);
	       new_descr->description = fread_string(fl, buf2);
	       new_descr->next = world[room_nr].ex_description;
	       world[room_nr].ex_description = new_descr;
	    }else if (*chk == 'T')  /* extra description field */ {
		fscanf(fl,"%d %d\n",&tmp,&tmp2);
		world[room_nr].tele_to_room = tmp;
		world[room_nr].tele_delay = tmp2;
		world[room_nr].tele_mesg1 = fread_string(fl,buf2);
		world[room_nr].tele_mesg2 = fread_string(fl,buf2);
		world[room_nr].tele_mesg3 = fread_string(fl,buf2);	
	    } else if (*chk == 'S')	/* end of current room */
	       break;
	 }

	 room_nr++;
      }
   } while (flag);

   free(temp);	/* cleanup the area containing the terminal $  */

   top_of_world = room_nr - 1;
}




/* read direction data */
void	setup_dir(FILE *fl, int room, int dir, int mode)
{
   int	tmp;

   sprintf(buf2, "Room #%d, direction D%d", world[room].number, dir);

   CREATE(world[room].dir_option[dir], struct room_direction_data , 1);

   world[room].dir_option[dir]->general_description = 
       fread_string(fl, buf2);
   world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

   fscanf(fl, " %d ", &tmp);
   if (mode == 0){
     if (tmp == 1)
       world[room].dir_option[dir]->exit_info = EX_ISDOOR;
     else if (tmp == 2)
       world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
     else
       world[room].dir_option[dir]->exit_info = 0;
   }
   else{
     world[room].dir_option[dir]->exit_info = tmp;
   }
   fscanf(fl, " %d ", &tmp);
   world[room].dir_option[dir]->key = tmp;

   fscanf(fl, " %d ", &tmp);
   world[room].dir_option[dir]->to_room = tmp;
}


void fwrite_string(FILE *wfile,char *wstr)
{
  char *tempstr;
  int  i,j;
  if(wstr!=NULL)
  {
    CREATE(tempstr,char ,strlen(wstr)+1);
    j=0;
    for(i=0;i<=strlen(wstr);i++)
    {
      if(wstr[i]!='\r')
      {
	tempstr[j]=wstr[i];
	j++;
      }
    }

    fprintf(wfile,"%s~\n",tempstr);
    free(tempstr);
  }
  else
    fprintf(wfile,"~\n");
}
int create_shop(int zone)
{
  bool write_index=FALSE;
  int i, last=-1;
  
  for (i = 0;i<= number_of_shops;i++)
    if (shop_index[i].virtual/100 == zone)
      last = shop_index[i].virtual;
  
  if (last > 0)
    last++;
  else{
    write_index = TRUE;
    last = zone*100;
  }
  if (!(shop_index = (struct shop_data *) realloc (shop_index, sizeof(struct shop_data)*(number_of_shops + 2)))) {
    printf("Realloc error - shops\n");
    return (-1);
  }
  number_of_shops++;
  shop_index[number_of_shops].virtual = last;
  strcpy(buf,"%s I don't have that item.");
  CREATE(shop_index[number_of_shops].no_such_item1, char, strlen(buf) + 1);
  strcpy(shop_index[number_of_shops].no_such_item1,buf);
  strcpy(buf,"%s You don't have that item.");
  CREATE(shop_index[number_of_shops].no_such_item2, char, strlen(buf) + 1);
  strcpy(shop_index[number_of_shops].no_such_item2,buf);
  strcpy(buf,"%s I don't buy that sort of item.");
  CREATE(shop_index[number_of_shops].do_not_buy, char, strlen(buf) + 1);
  strcpy(shop_index[number_of_shops].do_not_buy,buf);
  strcpy(buf,"%s I can't afford it.");
  CREATE(shop_index[number_of_shops].missing_cash1, char, strlen(buf) + 1);
  strcpy(shop_index[number_of_shops].missing_cash1,buf);
  strcpy(buf,"%s You can't afford it.");
  CREATE(shop_index[number_of_shops].missing_cash2, char, strlen(buf) + 1);
  strcpy(shop_index[number_of_shops].missing_cash2,buf);
  strcpy(buf,"%s That costs %s.");
  CREATE(shop_index[number_of_shops].message_buy, char, strlen(buf) + 1);
  strcpy(shop_index[number_of_shops].message_buy,buf);
  strcpy(buf,"%s I'll give you %s for it.");
  CREATE(shop_index[number_of_shops].message_sell, char, strlen(buf) + 1);
  strcpy(shop_index[number_of_shops].message_sell,buf);
  for (i=0;i< MAX_PROD;i++)
    shop_index[number_of_shops].producing[i] = -1;
  shop_index[number_of_shops].close1 = 28;
  shop_index[number_of_shops].profit_buy  = 1.0;
  shop_index[number_of_shops].profit_sell = 1.0;
  if (write_index){
    save_shops(zone);
    write_db_index(DB_BOOT_SHP);
  }
  return(last);
}

int create_room(int zone)
{
  bool write_index=FALSE;
  int j,i,cmd_no,zn;
  struct char_data *ch;
  struct obj_data *obj;
  j = zone;
  zn = zone;
  for (zn = 0; zone_table[zn].number != zone && zn <= top_of_zone_table; zn++)
    ;
  if (zn > top_of_zone_table)
    return(-2);
  
  /* locate last room in the zone */
  
  for (i=0 ; world[i].zone < (zn+1) && i <= top_of_world; i++)
    ;
  /* if we didn't find a room then zone must be new higest zone */
  if (i == top_of_world && world[i].zone < zn+1)
    i++;

  if (world[i-1].number >= zone_table[zn].top)
    {
      return(0);
    }
  
  if (world[i-1].zone != zn)
    {
      sprintf(buf,"No rooms in this zone. Creating room #%d\n",zone*100);
      mudlog(buf, CMP, LEVEL_BUILDER, TRUE);
      write_index = TRUE;
    }
  
  
  if (!(world = (struct room_data *) realloc (world, sizeof(struct room_data)*(top_of_world + 2))))
    printf("Realloc error - world\n");
  
  if ( i <= top_of_world )
    memmove((world+i+1),(world+i),sizeof(struct room_data)*(top_of_world-i+1));
  clear_room(world + i);
  
  world[i].zone  = zn;
  if (world[i-1].zone < zn)
    world[i].number = zone*100;
  else
    world[i].number = world[i-1].number + 1;
  
  world[i].tele_delay = -1;
  world[i].funct = 0;
  world[i].contents = 0;    
  world[i].people = 0;
  
  
  top_of_world++;
  
  for (ch = character_list;ch;ch = ch->next)
    {
      if (ch->in_room >=i)
	ch->in_room++;
      if  (!IS_NPC(ch) && ch->specials.was_in_room >=i)
	ch->specials.was_in_room++;
    }
  for (obj = object_list;obj;obj = obj->next)
    if (obj->in_room >=i)
      obj->in_room++;
  /* check start rooms */
  if (r_immort_start_room >=i)
    r_immort_start_room++;
  if (r_mortal_start_room >=i)
    r_mortal_start_room++;
  if (r_frozen_start_room >=i)
    r_frozen_start_room++;    
  /* correct the zone resets */
  j = zn;
  for (zn = 0;zn <= top_of_zone_table;zn++)
    if (zone_table[zn].number >= j)
      for (cmd_no = 0; ;cmd_no++)
	if (ZCMD.command == 'S')
	  break;
	else if (ZCMD.command == 'M' || ZCMD.command == 'm'   && ZCMD.arg3 >= i)
	  ZCMD.arg3++;
	else if (ZCMD.command == 'O' || ZCMD.command == 'o'  && ZCMD.arg3 >= i)
	  ZCMD.arg3++;
	else if (ZCMD.command == 'D' || ZCMD.command == 'd'  && ZCMD.arg1 >= i)
	  ZCMD.arg1++;
  make_dummy_room(i);
  if (write_index){
    save_rooms(zone);
    write_db_index(DB_BOOT_WLD);
  }
  return(i);

}

















