/* ************************************************************************
*   File: shopedit.c                                  Part of Archipelago *
*  Usage: online object shop commands. A Neil - August 1995               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Archipelago is based on                                                *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*  Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 * 
 ************************************************************************ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "limits.h"
#include "screen.h"
#include "shopedit.h"
#include "spells.h"
#include "shop.h"

/* extern variables */
extern int    top_of_world;
extern struct room_data *world;
extern struct shop_data *shop_index;
extern struct obj_data  *obj_proto;
extern struct char_data  *mob_proto;
extern struct index_data *obj_index;
extern struct index_data *mob_index;
extern char   *item_types[];

int	save_shops(int zone);
void copy_shop(struct shop_data *from, struct shop_data *to);
void free_shop(struct shop_data *shop);
SPECIAL(shop_keeper);
/* local prototypes */
void print_shop(struct descriptor_data *d);
void print_keeper(struct descriptor_data *d);
void print_sroom(struct descriptor_data *d);
int  print_objt(struct descriptor_data *d);
void print_messages(struct descriptor_data *d);
void print_product(struct descriptor_data *d) ;
void print_products(struct descriptor_data *d) ;

void shopedit(struct descriptor_data *d, char *str)
{
  char bufme[MAX_STRING_LENGTH];
  int rshop, points, num, ja_flag;

  switch(d->shedit_mode) {

  case MAIN_MODE:
    for (; isspace(*str); str++)
      ;
    if (!*str){
      d->oedit_mode = MAIN_MODE;
      print_shop(d);	    
      break;}
    *str = LOWER(*str);
    if (*str == 'q'){
      if ((rshop = real_shop(d->shop_edit->virtual)) < 0){
	SEND_TO_Q("Yikes shop doesn't exist.\r\nReport this!\r\n",d);
	free_shop(d->shop_edit);
	d->shop_edit = 0;
	d->virtual = 0;
	d->prompt_mode = 1;
	d->iedsc = 0;
	d->ex_i_dir = 0;
	d->ia_flag = 0;
	d->r_dir = 0;
	d->to_room = 0;
	REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
	act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
	return;}
      if (d->shop_edit->keeper > 0)
	mob_index[d->shop_edit->keeper].func = shop_keeper;
      copy_shop(d->shop_edit, shop_index + rshop);
      free_shop(d->shop_edit);	    
      d->shop_edit = 0;
      d->virtual = 0;
      d->iedsc = 0;
      d->ex_i_dir = 0;
      d->ia_flag = 0;
      d->r_dir = 0;
      d->to_room = 0;	    
      d->prompt_mode = 1;
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_BUILDING);
      act("$n returns from creating part of the world.",TRUE, d->character,0,0,TO_ROOM);
      save_shops(d->character->specials2.edit_zone);
      return;}
    else
      switch (*str){
      case 'k':
	d->shedit_mode = SK_EDIT;
	print_keeper(d);
	return;
      case 'r':
	d->shedit_mode = SR_EDIT;
	print_sroom(d);
	return;
      case 't':
	d->shedit_mode = ST_EDIT;
	d->ia_flag = print_objt(d);
	return;
      case 'b':
	d->shedit_mode = SB_EDIT;
	sprintf(bufme,"Shop profit on item bought by players in %%\r\n");
	send_to_char(bufme,d->character);
	sprintf(bufme, "Shop profit_buy: %s%d%s New profit: ",
		cy,(int) (100*d->shop_edit->profit_buy),nrm);
	send_to_char(bufme,d->character);
	return;
      case 's':
	d->shedit_mode = SS_EDIT;
	sprintf(bufme,"Shop profit on item sold by players in %%\r\n");
	send_to_char(bufme,d->character);
	sprintf(bufme, "Shop profit_sell: %s%d%s New profit: ",
		cy,(int) (100*d->shop_edit->profit_sell),nrm);
	send_to_char(bufme,d->character);
	return;
      case 'e':
	d->shedit_mode = STE1_EDIT;
	sprintf(bufme,
		"Keeper temper 1: %s%d%s\r\nEnter new temper: 1 or 0:",
		rd,d->shop_edit->temper1,nrm);
	send_to_char(bufme,d->character);
	return;	      
      case 'o':
	d->shedit_mode = SO1_EDIT;
	sprintf(bufme,
		"Shop opens 1: %s%d%s\r\nEnter open time: [0,6]:",
		gn,d->shop_edit->open1,nrm);
	send_to_char(bufme,d->character);
	return;
      case 'm':
	d->shedit_mode = SM_EDIT;
	print_messages(d);
	return;
      case 'p':
	d->shedit_mode = SP_EDIT;
	print_products(d);
	return;
      default:
	print_shop(d);
	return;
      }
  case SP_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      print_products(d);
      return;
    }
    if (*str == 'q' || *str == 'Q') {
      print_shop(d);
      d->shedit_mode = MAIN_MODE;
      return;
    }
    *str = LOWER(*str);
    switch (*str) {
    case '0':
      d->ia_flag = 0;
      break;
    case '1':
      d->ia_flag = 1;
      break;
    case '2':
      d->ia_flag = 2;
       break;
    case '3':
      d->ia_flag = 3;
      break;
    case '4':
      d->ia_flag = 4;
      break;
    case 'q':
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    default:
      SEND_TO_Q("No such item.\r\n",d);
      return;      
    }
    d->shedit_mode = SPC_EDIT;
    print_product(d);
    return;
  case SPC_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      print_product(d);
      break;}
    if (*str == 'q' || *str == 'Q') {
      print_products(d);
      d->shedit_mode = SP_EDIT;
      return;
    }
    points = atoi(str);
    if (!(num = real_object(points))) {
      d->shop_edit->producing[d->ia_flag] = -1;
      sprintf(bufme,"%sNo such object%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      print_product(d);
      break;}
    else {
      d->shop_edit->producing[d->ia_flag] = num;
      print_products(d);
      d->shedit_mode = SP_EDIT;      
      return;
    }
    
  case SM_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      print_messages(d);
      return;
    }
    *str = LOWER(*str);
    if (*str != '1' && *str != '2' && *str != '3' && *str != '4' &&
	*str != '5' && *str != '6' && *str != '7' && *str != 'q') {
      SEND_TO_Q("No such string.\r\n",d);
      return;
    }
    switch (*str) {
    case '1':
      d->str = &(d->shop_edit->no_such_item1);
      free(*d->str);
      *d->str = 0;
      d->max_str = 65;
      SEND_TO_Q("Enter new no_such_item1 string.  No crlf.  End with @@\r\n",d);
      return;
    case '2':
      d->str = &(d->shop_edit->no_such_item2);
      free(*d->str);
      *d->str = 0;
      d->max_str = 65;
      SEND_TO_Q("Enter new no_such_item2 string.  No crlf.  End with @@\r\n",d);
      return;
    case '3':
      d->str = &(d->shop_edit->do_not_buy);
      free(*d->str);
      *d->str = 0;
      d->max_str = 65;
      SEND_TO_Q("Enter new do_not_buy string.  No crlf.  End with @@\r\n",d);
      return;
    case '4':
      d->str = &(d->shop_edit->missing_cash1);
      free(*d->str);
      *d->str = 0;
      d->max_str = 65;
      SEND_TO_Q("Enter new missing_cash1 string.  No crlf.  End with @@\r\n",d);
      return;
    case '5':
      d->str = &(d->shop_edit->missing_cash2);
      free(*d->str);
      *d->str = 0;
      d->max_str = 65;
      SEND_TO_Q("Enter new missing_cash2 string.  No crlf.  End with @@\r\n",d);
      return;
    case '6':
      d->str = &(d->shop_edit->message_buy);
      free(*d->str);
      *d->str = 0;
      d->max_str = 65;
      SEND_TO_Q("Enter new message_buy string.  No crlf.  End with @@\r\n",d);
      return;
    case '7':
      d->str = &(d->shop_edit->message_sell);
      free(*d->str);
      *d->str = 0;
      d->max_str = 65;
      SEND_TO_Q("Enter new message_sell string.  No crlf.  End with @@\r\n",d);
      return;
    case 'q':
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
  case SS_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Profit sell unchanged\r\n",d);
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
    points = atoi(str);
    if (points < 1 || points > 1000) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Profit sell: %s%d%s\r\nEnter new profit: [1,1000]:",
	      gn,(int) (100*d->shop_edit->profit_sell),nrm);
      send_to_char(bufme,d->character);
      return;}
    else {
      d->shop_edit->profit_sell = ((float) points)/100. ;
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
  case SB_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Profit buy unchanged\r\n",d);
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
    points = atoi(str);
    if (points < 1 || points > 1000) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Profit buy: %s%d%s\r\nEnter new profit: [1,1000]:",
	      gn,(int) (100*d->shop_edit->profit_buy),nrm);
      send_to_char(bufme,d->character);
      return;}
    else {
      d->shop_edit->profit_buy = ((float) points)/100. ;
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
  case SO1_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Open 1 unchanged\r\n",d);
      sprintf(bufme,"Open 2: %s%d%s\r\nEnter new open time: [14,20]:",
	      gn,d->shop_edit->open2,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = SO2_EDIT;
      return;
    }
    points = atoi(str);
    if (points < 0 || points > 6) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Shop opens 1: %s%d%s\r\nEnter open time: [0,6]:",
	      gn,d->shop_edit->open1,nrm);
      send_to_char(bufme,d->character);
      return;}
    else {
      d->shop_edit->open1 = points;
      sprintf(bufme,"Open 2: %s%d%s\r\nEnter new open time: [14,20]:",
	      gn,d->shop_edit->open2,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = SO2_EDIT;
      return;
    }
  case SO2_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Open 2 unchanged\r\n",d);
      sprintf(bufme,"Closing 1: %s%d%s\r\nEnter new closing time: [7,13]:",
	      rd,d->shop_edit->close1,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = SC1_EDIT;
      return;
    }
    points = atoi(str);
    if (points < 14 || points > 20) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Shop opens 2: %s%d%s\r\nEnter open time: [14,20]:",
	      gn,d->shop_edit->open2,nrm);
      send_to_char(bufme,d->character);
      return;}
    else {
      d->shop_edit->open2 = points;
      sprintf(bufme,"Closing 1: %s%d%s\r\nEnter new closing time: [7,13]:",
	      rd,d->shop_edit->close1,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = SC1_EDIT;
      return;
    }
  case SC1_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Close 1 unchanged\r\n",d);
      sprintf(bufme,"Closing 2: %s%d%s\r\nEnter new open time: [21,28]:",
	      rd,d->shop_edit->close2,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = SC2_EDIT;
      return;
    }
    points = atoi(str);
    if (points < 7 || points > 13) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Closing 1: %s%d%s\r\nEnter close time: [7,13]:",
	      rd,d->shop_edit->close1,nrm);
      send_to_char(bufme,d->character);
      return;}
    else {
      d->shop_edit->close1 = points;
      sprintf(bufme,"Closeing 2: %s%d%s\r\nEnter new close time: [21,28]:",
	      rd,d->shop_edit->close2,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = SC2_EDIT;
      return;
    }
  case SC2_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Close 2 unchanged\r\n",d);
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
    points = atoi(str);
    if (points < 21 || points > 28) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Closing 2: %s%d%s\r\nEnter close time: [21,28]:",
	      rd,d->shop_edit->close2,nrm);
      send_to_char(bufme,d->character);
      return;}
    else {
      d->shop_edit->close2 = points;
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
  case STE1_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Temper 1 unchanged\r\n",d);
      sprintf(bufme,"Keeper temper 2: %s%d%s\r\nEnter new temper: 1 or 0:",
	      rd,d->shop_edit->temper2,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = STE2_EDIT;
      return;
    }
    points = atoi(str);
    if (points < 0 || points > 1) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Keeper temper 1: %s%d%s\r\nEnter new temper: 1 or 0:",
	      rd,d->shop_edit->temper1,nrm);
      send_to_char(bufme,d->character);
      return;}
    else {
      d->shop_edit->temper1 = points;
      sprintf(bufme,"Keeper temper 2: %s%d%s\r\nEnter new temper: 1 or 0:",
	      rd,d->shop_edit->temper2,nrm);
      send_to_char(bufme,d->character);
      d->shedit_mode = STE2_EDIT;
      return;
    }
  case STE2_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      SEND_TO_Q("Temper 2 unchanged\r\n",d);
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
    points = atoi(str);
    if (points < 0 || points > 1) {
      sprintf(bufme,"%sIllegal value%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      sprintf(bufme,"Keeper temper 2: %s%d%s\r\nEnter new temper: 1 or 0:",
	      rd,d->shop_edit->temper2,nrm);
      send_to_char(bufme,d->character);
      return;
    }
    else {
      d->shop_edit->temper2 = points;
      d->shedit_mode = MAIN_MODE;
      print_shop(d);
      return;
    }
  case ST_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      print_objt(d);
      return;}
    if (is_number(str)){
      ja_flag = atoi(str);
      if (ja_flag > 0 && ja_flag <= d->ia_flag )
	{
	  if (d->shop_edit->tradetype < 0)
	    d->shop_edit->tradetype = 0;
	  if (!IS_SET(d->shop_edit->tradetype, 1<<ja_flag))
	    SET_BIT(d->shop_edit->tradetype, 1<<ja_flag);
	  else
	    REMOVE_BIT(d->shop_edit->tradetype, 1<<ja_flag);
	  d->ia_flag =  print_objt(d);
	  return;
	}
      else if (ja_flag == 0) {
	SEND_TO_Q("Setting to trade in Anything.\r\n",d);
	d->shop_edit->tradetype = 0;
	d->ia_flag =  print_objt(d);
	return;
      }
      else {
	SEND_TO_Q("Setting to Sell only.\r\n",d);
	d->shop_edit->tradetype = -1;
	d->ia_flag =  print_objt(d);
	return;
      }
    }
    else if (*str == 'Q' || *str == 'q'){
      print_shop(d);	
      d->shedit_mode = MAIN_MODE;
      break;
    }
    else{
      SEND_TO_Q("Setting to Sell only.\r\n",d);
      d->shop_edit->tradetype = -1;
      d->ia_flag =  print_objt(d);
      break;
    }
  case SK_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      print_keeper(d);
      break;}
    if (*str == 'q' || *str == 'Q') {
      print_shop(d);
      d->shedit_mode = MAIN_MODE;
      return;
    }
    points = atoi(str);
    if (!(num = real_mobile(points))) {
      d->shop_edit->keeper = 0;
      sprintf(bufme,"%sNo such mobile%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      print_keeper(d);
      break;}
    else {
      d->shop_edit->keeper = num;
      print_keeper(d);
      return;
    }
  case SR_EDIT:
    for ( ; isspace(*str); str++)
      ;
    if (!*str){
      print_sroom(d);
      break;}
    if (*str == 'q' || *str == 'Q') {
      print_shop(d);
      d->shedit_mode = MAIN_MODE;
      return;
    }
    points = atoi(str);
    if (!(num = real_room(points))) {
      d->shop_edit->in_room = -1;
      sprintf(bufme,"%sNo such room setting to wanderer%s\r\n",rd,nrm);
      SEND_TO_Q(bufme,d);
      print_sroom(d);
      break;}
    else {
      d->shop_edit->in_room = points;
      print_sroom(d);
      return;
    }
    
  }
  }
void print_product(struct descriptor_data *d) {
  if (d->shop_edit->producing[d->ia_flag] >= 0) 
    sprintf(buf,"Object vnum = %s%d%s %s%s%s\r\n",gn,
	   obj_index[d->shop_edit->producing[d->ia_flag]].virtual,
	    nrm,cy,
	    obj_proto[d->shop_edit->producing[d->ia_flag]].short_description,
	    nrm);
  else
    sprintf(buf,"Object vnum = %sNone set%s\r\n",gn,nrm);
  
  send_to_char(buf,d->character);
  sprintf(buf,"Enter new Vnum, -1 to clear, Q to quit: ");
  send_to_char(buf,d->character);  
}
void print_products(struct descriptor_data *d)
  {
    int i;
    for (i=0;i< MAX_PROD;i++) {
      if (d->shop_edit->producing[i] >=0) 
	sprintf(buf,"(%s%d%s) [%s%d%s]  %s%s%s\r\n",gn,i,nrm,cy,
		obj_index[d->shop_edit->producing[i]].virtual,
		nrm,yl, obj_proto[d->shop_edit->producing[i]].short_description,
		nrm);
      else
	sprintf(buf,"(%s%d%s) %sNot set%s\r\n",gn,i,nrm,cy,nrm);
      send_to_char(buf,d->character);
    }
    sprintf(buf,"(%sQ%s)uit\r\n",gn,nrm);
    send_to_char(buf,d->character);
  }

int print_objt(struct descriptor_data *d)
{
  int i;
  if (d->shop_edit->tradetype < 0)
    sprintf(buf1 ,"%sTrade in:%s %sSells only%s\r\n\r\n",cy,nrm,gn,nrm);
  else if (d->shop_edit->tradetype == 0)
    sprintf(buf1 ,"%sTrade in:%s %sANYTHING%s\r\n\r\n",cy,nrm,gn,nrm);
  else {
    sprintbit(d->shop_edit->tradetype, item_types, buf);
    sprintf(buf1 ,"%sTrade in:%s %s%s%s\r\n\r\n",cy,nrm,gn,buf,nrm);
  }
  SEND_TO_Q(buf1,d);
  i = 0;
  sprintf(buf,"%s","ANYTHING");
  sprintf(buf1,"[%2d]  %s%-15.15s%s      ",i,gn,buf,nrm);
    SEND_TO_Q(buf1,d);
    i = 1;
    sprintf(buf1,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,item_types[i],nrm,i+1,gn,item_types[i+1],nrm);
    i +=2;
    SEND_TO_Q(buf1,d);
    while (*item_types[i] != '\n')
	{
	    if (*item_types[i+2] != '\n' && *item_types[i+1] != '\n'){
		sprintf(buf1,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,item_types[i],nrm,i+1,gn,item_types[i+1],nrm,i+2,gn,item_types[i+2],nrm);
		i +=3;}
	    else if (*item_types[i+1] != '\n') {
		sprintf(buf1,"[%2d]  %s%-15.15s%s      [%2d]  %s%-15.15s%s \r\n",i,gn,item_types[i],nrm,i+1,gn,item_types[i+1],nrm);
		i +=2;}
	    else{
	      sprintf(buf1,"[%2d]  %s%-15.15s%s\r\n",i,gn,item_types[i],nrm);
		i +=1;}
	    SEND_TO_Q(buf1,d);        
	}
    
    sprintf(buf1 ,"\r\n%sEnter item type # to toggle or -1 for sell only, Q to quit.%s ",cy,nrm);
    SEND_TO_Q(buf1,d);
    return(i-1);


}
void print_sroom(struct descriptor_data *d)
{
  char bufme[200];
  if (d->shop_edit->in_room > 0)
    sprintf(bufme,"Room: (%s%d%s) %s%s%s\r\n",gn,
	    d->shop_edit->in_room,nrm,byl,
	    world[real_room(d->shop_edit->in_room)].name,nrm);
  else
    sprintf(bufme,"Room vnum : %sWander%s\r\n",gn,nrm);
  send_to_char(bufme, d->character);
  sprintf(bufme, "Enter vnum of new room, -ve num to clear, Q to quit: ");
  send_to_char(bufme, d->character);  
}
void print_keeper(struct descriptor_data *d)
{
  char bufme[200];
  
  if (d->shop_edit->keeper > 0)
    sprintf(bufme,"(%sK%s)eeper: [%s%d%s] %s%s%s\r\n",cy,nrm,gn,
	    mob_index[d->shop_edit->keeper].virtual,nrm,
	    cy,mob_proto[d->shop_edit->keeper].player.short_descr,nrm
	    );
  else
        sprintf(bufme,"(%sK%s)eeper: %sNone set%s\r\n",cy,nrm,gn,nrm);
  send_to_char(bufme, d->character);
  sprintf(bufme, "Enter vnum of new keeper, -ve num to clear, Q to quit: ");
  send_to_char(bufme, d->character);
}
void print_messages(struct descriptor_data *d)
{
  sprintf(buf1,"%sMessages:%s\r\n",cy,nrm);
  send_to_char(buf1, d->character);
  sprintf(buf1, " (%s1%s)  No such item 1: %s%s%s\r\n",gn,nrm,
	  bwht,d->shop_edit->no_such_item1,nrm);
  send_to_char(buf1,d->character);
  sprintf(buf1, " (%s2%s)  No such item 2: %s%s%s\r\n",gn,nrm,
	  bwht,d->shop_edit->no_such_item2,nrm);
  send_to_char(buf1,d->character);
  sprintf(buf1, " (%s3%s)  Do not buy    : %s%s%s\r\n",gn,nrm,
	  bwht,d->shop_edit->do_not_buy,nrm);
  send_to_char(buf1,d->character);
  sprintf(buf1, " (%s4%s)  Missing cash 1: %s%s%s\r\n",gn,nrm,
	  bwht,d->shop_edit->missing_cash1,nrm);
  send_to_char(buf1,d->character);
  sprintf(buf1, " (%s5%s)  Missing cash 2: %s%s%s\r\n",gn,nrm,
	  bwht,d->shop_edit->missing_cash2,nrm);
  send_to_char(buf1,d->character);
  sprintf(buf1, " (%s6%s)  Message buy   : %s%s%s\r\n",gn,nrm,
	  bwht,d->shop_edit->message_buy,nrm);
  send_to_char(buf1,d->character);
  sprintf(buf1, " (%s7%s)  Message sell  : %s%s%s\r\n",gn,nrm,
	  bwht,d->shop_edit->message_sell,nrm);
  send_to_char(buf1,d->character);
    sprintf(buf1, " (%sQ%s)uit.\r\n",gn,nrm);
  send_to_char(buf1,d->character);
}
void print_shop(struct descriptor_data *d)
{
  int i,j;
  
  sprintf(cy,"%s",CCBBLU(d->character,C_NRM));
  sprintf(wh,"%s",CCWHT(d->character,C_NRM));
  sprintf(ma,"%s",CCMAG(d->character,C_NRM));
  sprintf(nrm,"%s",CCNRM(d->character,C_NRM));
  sprintf(gn,"%s",CCGRN(d->character,C_NRM));
  sprintf(yl,"%s",CCYEL(d->character,C_NRM));
  sprintf(rd,"%s",CCRED(d->character,C_NRM));
  sprintf(bgn,"%s",CCBGRN(d->character,C_NRM));
  sprintf(byl,"%s",CCBYEL(d->character,C_NRM));
  sprintf(brd,"%s",CCBRED(d->character,C_NRM));
  sprintf(bwht,"%s",CCBWHT(d->character,C_NRM));  

  sprintf(buf,"%sShop Number: %d%s\r\n",bgn, d->shop_edit->virtual,nrm);
  send_to_char(buf,d->character);
  if (d->shop_edit->keeper > 0)
    sprintf(buf,"(%sK%s)eeper: (%s%d%s) %s%s%s\r\n",cy,nrm,gn,
	    mob_index[d->shop_edit->keeper].virtual,nrm, byl,
	    mob_proto[d->shop_edit->keeper].player.short_descr, nrm);
  else
        sprintf(buf,"(%sK%s)eeper: %sNone set%s  ",cy,nrm,gn,nrm);
  send_to_char(buf, d->character);
  if (d->shop_edit->in_room > 0)
    sprintf(buf,"(%sR%s)oom: (%s%d%s) %s%s%s\r\n",cy,nrm,gn,
	    d->shop_edit->in_room,nrm,byl,
	    world[real_room(d->shop_edit->in_room)].name,nrm);
  else
        sprintf(buf,"(%sR%s)oom: %sWander%s\r\n",cy,nrm,gn,nrm);
  send_to_char(buf, d->character);
  if (d->shop_edit->tradetype > 0) {
    sprintbit(d->shop_edit->tradetype,item_types, buf2);
    sprintf(buf, "(%sT%s)rades: %s%s%s\r\n",cy,nrm,gn, buf2,nrm);
  }
  else if (d->shop_edit->tradetype == 0) 
    sprintf(buf, "(%sT%s)rades: %sANYTHING%s\r\n",cy,nrm,gn,nrm);
  else
    sprintf(buf, "(%sT%s)rades: %sOnly Sells%s\r\n",cy,nrm,gn,nrm);
  send_to_char(buf,d->character);
  sprintf(buf,"Profit (%sB%s)uy: %s%f%s,  (%sS%s)ell: %s%f%s\r\n",
	  cy,nrm,gn,d->shop_edit->profit_buy,nrm,
	  cy,nrm,gn,d->shop_edit->profit_sell,nrm);
  send_to_char(buf,d->character);
  sprintf(buf, "T(%sE%s)mpers: (1) %s%d%s  (2) %s%d%s\r\n",cy,nrm,rd,
	  d->shop_edit->temper1,nrm,rd,d->shop_edit->temper2,nrm);
  send_to_char(buf, d->character);
  sprintf(buf, "(%sO%s)pen: (1) %s%d%s  (2) %s%d%s",cy,nrm,gn,
	  d->shop_edit->open1,nrm,gn,d->shop_edit->open2,nrm);
  send_to_char(buf, d->character);
  sprintf(buf, "   Close: (1) %s%d%s  (2) %s%d%s\r\n",rd,
	  d->shop_edit->close1,nrm,rd,d->shop_edit->close2,nrm);
  send_to_char(buf, d->character);
  sprintf(buf,"(%sM%s)essages:\r\n",cy,nrm);
  send_to_char(buf, d->character);
  sprintf(buf, "   No such item 1: %s%s%s\r\n",
	  bwht,d->shop_edit->no_such_item1,nrm);
  send_to_char(buf,d->character);
  sprintf(buf, "   No such item 2: %s%s%s\r\n",
	  bwht,d->shop_edit->no_such_item2,nrm);
  send_to_char(buf,d->character);
  sprintf(buf, "   Do not buy    : %s%s%s\r\n",
	  bwht,d->shop_edit->do_not_buy,nrm);
  send_to_char(buf,d->character);
  sprintf(buf, "   Missing cash 1: %s%s%s\r\n",
	  bwht,d->shop_edit->missing_cash1,nrm);
  send_to_char(buf,d->character);
  sprintf(buf, "   Missing cash 2: %s%s%s\r\n",
	  bwht,d->shop_edit->missing_cash2,nrm);
  send_to_char(buf,d->character);
  sprintf(buf, "   Message buy   : %s%s%s\r\n",
	  bwht,d->shop_edit->message_buy,nrm);
  send_to_char(buf,d->character);
  if (d->shop_edit->tradetype >=0) {
    sprintf(buf, "   Message sell  : %s%s%s\r\n",
	    bwht,d->shop_edit->message_sell,nrm);
    send_to_char(buf,d->character);
  }
  sprintf(buf,"(%sP%s)roducing:\r\n",cy,nrm);
  send_to_char(buf,d->character);
  for (i=0,j=0;i< MAX_PROD;i++) {
    if (d->shop_edit->producing[i] >=0) {
      sprintf(buf,"(%s%d%s) [%s%d%s]  %s%s%s\r\n",gn,j,nrm,cy,
	      obj_index[d->shop_edit->producing[i]].virtual,
	      nrm,yl, obj_proto[d->shop_edit->producing[i]].short_description,
	      nrm);
        send_to_char(buf,d->character);
	j++;
    }
  } 
  
}


