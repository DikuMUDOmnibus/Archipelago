/* ************************************************************************
*   File: shop.c                                        Part of CircleMUD *
*  Usage: spec-procs and other funcs for shops and shopkeepers            *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "shop.h"


ACMD(do_tell);
ACMD(do_action);
ACMD(do_emote);
ACMD(do_say);
void    drop_excess_gold(struct char_data *ch, int amount);
char    *report_cost(int gold);
int     report_money_weight(int amount);
int     report_highest_value_for_weight(int weight);
int     report_pennies(int amount);
int     report_groats(int amount);
int     report_crowns(int amount);
int     real_shop(int virtual);

extern struct str_app_type str_app[];
extern struct index_data *mob_index;
extern const char *object_sizes[];
extern struct index_data *obj_index;
extern struct char_data *martha;
extern struct char_data *reimund;
extern struct char_data *aelfric;

extern struct room_data *world;
extern struct time_info_data time_info;

struct shop_data *shop_index;
int	number_of_shops = 0;

int real_shop(int virtual)
{
    int i,j=-1;
    
    for (i =0;i<=number_of_shops;i++)
      if (shop_index[i].virtual == virtual) {
	j = i;
	break;
      }
    return(j);
}

int	is_ok(struct char_data *keeper, struct char_data *ch, int shop_nr)
{
   if (shop_index[shop_nr].open1 > time_info.hours) {
      do_say(keeper, "Come back later!", 17, 0);
      return(FALSE);
   } else if (shop_index[shop_nr].close1 < time_info.hours)
      if (shop_index[shop_nr].open2 > time_info.hours) {
	 do_say(keeper, "Sorry, we have closed, but come back later.", 17, 0);
	 return(FALSE);
      } 
      else if (shop_index[shop_nr].close2 < time_info.hours) {
	 do_say(keeper, "Sorry, come back tomorrow.", 17, 0);
	 return(FALSE);
      };

   if (!(CAN_SEE(keeper, ch))) {
      do_say(keeper, "I don't trade with someone I can't see!", 17, 0);
      return(FALSE);
   };

   switch (shop_index[shop_nr].with_who) {
   case 0  : 
      return(TRUE);
   case 1  : 
      return(TRUE);
   default : 
      return(TRUE);
   };
}


int	trade_with(struct char_data *keeper, struct char_data *customer, struct obj_data *item, int shop_nr)
{
  
   if (item->obj_flags.cost < 1)
      return(FALSE);
   if ((shop_index[shop_nr].tradetype > 0
	&& IS_SET(shop_index[shop_nr].tradetype,
		  (1 << item->obj_flags.type_flag)))
       || shop_index[shop_nr].tradetype == 0) {
     if ((GET_OBJ_SIZE(item) == 1 || GET_OBJ_SIZE(item) == 0 || GET_OBJ_SIZE(item) == 3 || GET_OBJ_SIZE(item) == 2)
	 &&  (mob_index[keeper->nr].virtual == 1834)) {
       sprintf(buf2, "%s That item is far too big!" , GET_NAME(customer));
       do_tell(keeper, buf2, 0, 0);
       sprintf(buf2, "%s My customers would never buy the like.",
	       GET_NAME(customer));
       do_tell(keeper, buf2, 0, 0);
       return(FALSE);
     }
     return(TRUE);
   }
   return(FALSE);
}


int	shop_producing(struct obj_data *item, int shop_nr)
{
   int	counter;

   if (item->item_number < 0)
      return(FALSE);

   for (counter = 0; counter < MAX_PROD; counter++)
      if (shop_index[shop_nr].producing[counter] == item->item_number)
	 return(TRUE);
   return(FALSE);
}


void	shopping_buy( char *arg, struct char_data *ch,
struct char_data *keeper, int shop_nr)
{
   char	argm[100], *ptr=0;
   int objs=0, chars=0;
   struct obj_data *temp1;
   int amount, number=1, tot_amount=0;
   
   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   one_argument(arg, argm);
   if (!(*argm)) {
      sprintf(buf2, "%s what do you want to buy??", GET_NAME(ch));
      do_tell(keeper, buf2, 19, 0);
      return;
   };
   if ((ptr =  strchr(argm,'*')) != 0) {
     *ptr = '\0';
     ptr++;
     if (is_number(argm))
       number = atoi(argm);
   }
   else
     ptr = argm;
   if (!( temp1 =  get_obj_in_list_vis(ch, ptr, keeper->inventory))) {
     sprintf(buf2, shop_index[shop_nr].no_such_item1 , GET_NAME(ch));
     do_tell(keeper, buf2, 19, 0);
     return;
   }
   if (IS_GODITEM(temp1) && GET_LEVEL(ch) < LEVEL_BUILDER){
     sprintf(buf2, "%s You aint holy enough to buy this item.\r\n", GET_NAME(ch));
     do_tell(keeper, buf2, 19, 0);
     return;
   }
   if (temp1->obj_flags.cost <= 0) {
     sprintf(buf2, shop_index[shop_nr].no_such_item1 , GET_NAME(ch));
     do_tell(keeper, buf2, 19, 0);
     extract_obj(temp1,0);
     return;
   }
   while (number--) {
     if (!( temp1 =  get_obj_in_list_vis(ch, ptr, keeper->inventory))) {
       sprintf(buf2, shop_index[shop_nr].no_such_item1 , GET_NAME(ch));
       do_tell(keeper, buf2, 19, 0);
       sprintf(buf2, shop_index[shop_nr].message_buy, GET_NAME(ch),
	       report_cost(tot_amount));
       do_tell(keeper, buf2, 19, 0);
       return;
     }
     
     amount = (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_buy);
     if ((amount < 10) ||
	 ((keeper == martha) || (keeper == aelfric) || (keeper == reimund)))
       amount = 0;
     if (GET_GOLD(ch) < amount && GET_LEVEL(ch)
	 < LEVEL_GOD && amount) {
       sprintf(buf2, shop_index[shop_nr].message_buy, GET_NAME(ch),
	       report_cost(amount));
       do_tell(keeper, buf2, 19, 0);
       sprintf(buf2, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
       do_tell(keeper, buf2, 19, 0);
       
       switch (shop_index[shop_nr].temper1) {
       case 0:
	 do_action(keeper, GET_NAME(ch), 116, 0);
	 return;
       case 1:
	 switch(GET_SEX(keeper)){
	 case SEX_MALE:    do_emote(keeper, "smokes on his joint.", 36, 0);
	   break;
	 case SEX_FEMALE:  do_emote(keeper, "smokes on her joint.", 36, 0);
	   break;
	 case SEX_NEUTRAL: do_emote(keeper, "smokes on its joint.", 36, 0);
	   break;
	 }
	 return;
       default:
	 return;
       }
       
     }

     if ((IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
       sprintf(buf2, "%s : You can't carry that many items.\r\n",  fname(temp1->name));
       send_to_char(buf2, ch);
       sprintf(buf2, shop_index[shop_nr].message_buy, GET_NAME(ch),
	       report_cost(tot_amount));
       do_tell(keeper, buf2, 19, 0);
       return;
     }
     
     if ((IS_CARRYING_W(ch) + temp1->obj_flags.weight) > CAN_CARRY_W(ch)) {
       sprintf(buf2, "%s : You can't carry that much weight.\r\n",  fname(temp1->name));
       send_to_char(buf2, ch);
       sprintf(buf2, shop_index[shop_nr].message_buy, GET_NAME(ch),
	       report_cost(tot_amount));
       do_tell(keeper, buf2, 19, 0);
       return;
     }
     
     act("$n buys $p.", FALSE, ch, temp1, 0, TO_ROOM);
     
     sprintf(buf2, "You now have %s.\r\n", temp1->short_description);
     send_to_char(buf2, ch);
     if (GET_LEVEL(ch) < LEVEL_GOD){
       amount = (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy);
       if (amount < 10)
	 amount = 0;
       tot_amount += amount;
       change_gold(ch, -amount);

     }
     change_gold(keeper, amount);
     
     /* If the shopkeeper has more than 15000 coins, put it in the bank! */
     /* disabled because keepers have so many HP now -je
	if (GET_GOLD(keeper) > 15000) {
	shop_index[shop_nr].bankAccount += (GET_GOLD(keeper) - 15000);
	GET_GOLD(keeper) = 15000;
	} 
	*/
     
     /* Test if producing shop ! */
     if (shop_producing(temp1, shop_nr))
       temp1 = read_object(temp1->item_number, REAL,0);
     else
       obj_from_char(temp1,0);
     if ((GET_ITEM_TYPE(temp1) != ITEM_FOOD) &&
	 (GET_ITEM_TYPE(temp1) != ITEM_WEAPON) &&
	 (GET_ITEM_TYPE(temp1) != ITEM_BOAT) &&
	 (GET_ITEM_TYPE(temp1) != ITEM_PHILTRE) &&
	 (GET_ITEM_TYPE(temp1) != ITEM_CANTRIP) &&
	 (GET_ITEM_TYPE(temp1) != ITEM_ROD) &&
	 (GET_ITEM_TYPE(temp1) != ITEM_DRINKCON) &&
	 ((GET_ITEM_TYPE(temp1) == ITEM_CONTAINER) &&
	   (temp1->obj_flags.wear_flags > 0))){
       objs = temp1->obj_flags.value[5];
       chars = GET_SIZE(ch);
       if (IS_SET(temp1->obj_flags.extra_flags, ITEM_RESIZED))
	 {
	   do_say(keeper,"This object has already been worked on.",0,0);
	   do_say(keeper,"I can't resize it for you, sorry.",0,0);
	 }
       else if (chars < objs && abs(chars - objs) > 3){
	 do_say(keeper,"Sorry it is too small to make fit.",0,0);
	 do_say(keeper,"I can't resize it for you, sorry.",0,0);
       }
       else {
	 temp1->obj_flags.value[5] = GET_SIZE(ch);
	 GET_OBJ_WEIGHT(temp1)
	   = (GET_OBJ_WEIGHT(temp1)*(9 - GET_OBJ_SIZE(temp1)))/7;
	 SET_BIT(temp1->obj_flags.extra_flags,ITEM_RESIZED);
	 act("$N sizes $n.",FALSE,ch,temp1,keeper,TO_ROOM);
	 act("$N sizes you.",FALSE,ch,temp1,keeper,TO_CHAR);	
	 act("$N fiddles with $p for a while.",FALSE,ch,temp1,keeper,TO_ROOM);
	 act("$N fiddles with $p for a while.",FALSE,ch,temp1,keeper,TO_CHAR);
	 do_say(keeper,"There all sized to fit.",0,0);
       }
     }
     obj_to_char(temp1, ch,0);
   }
   sprintf(buf2, shop_index[shop_nr].message_buy, GET_NAME(ch),
	   report_cost(tot_amount));
   do_tell(keeper, buf2, 19, 0);
   return;
}


void	shopping_sell( char *arg, struct char_data *ch,
		       struct char_data *keeper, int shop_nr)
{
   char	argm[100], *ptr;
   struct obj_data *temp1, *tmp;
   int amount, tot_amount=0, number=1;

   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   one_argument(arg, argm);

   if (!(*argm)) {
      sprintf(buf2, "%s What do you want to sell??" , GET_NAME(ch));
      do_tell(keeper, buf2, 19, 0);
      return;
   }
   if ((ptr = strchr(argm,'*')) != 0) {
     *ptr = '\0';
     ptr++;
     if (is_number(argm))
       number = atoi(argm);
   }
   else
     ptr = argm;
   if (!( temp1 = get_obj_in_list_vis(ch, ptr, ch->inventory))) {
     sprintf(buf2, shop_index[shop_nr].no_such_item2 , GET_NAME(ch));
     do_tell(keeper, buf2, 19, 0);
     return;
   }
   if (IS_SET(temp1->obj_flags.extra_flags, ITEM_NODROP) && (GET_LEVEL(ch) < LEVEL_BUILDER)) {
     sprintf(buf2, "You can't let go of %s, it must be CURSED!\r\n", OBJS(temp1, ch));
     send_to_char(buf2, ch);
     return;
   }
   if (!(trade_with(keeper,ch,temp1, shop_nr)) || (temp1->obj_flags.cost < 1)) {
     sprintf(buf2, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
     do_tell(keeper, buf2, 19, 0);
     return;
   }
   while (number--) {
     if (!( temp1 = get_obj_in_list_vis(ch, ptr, ch->inventory))) {
       sprintf(buf2, shop_index[shop_nr].no_such_item2 , GET_NAME(ch));
       do_tell(keeper, buf2, 19, 0);
       sprintf(buf2, shop_index[shop_nr].message_sell, GET_NAME(ch),
	       report_cost(tot_amount));
       do_tell(keeper, buf2, 19, 0);
       return;
     }
     
     amount = (int) (temp1->obj_flags.cost *  shop_index[shop_nr].profit_sell);
     if ((amount < 10)
	 || ((keeper == martha) || (keeper == aelfric) || (keeper == reimund)))
       amount = 0;
     if ((GET_GOLD(keeper) + shop_index[shop_nr].bankAccount) <  amount) {
       sprintf(buf2, shop_index[shop_nr].missing_cash1 , GET_NAME(ch));
       do_tell(keeper, buf2, 19, 0);
       sprintf(buf2, shop_index[shop_nr].message_sell, GET_NAME(ch),
	       report_cost(tot_amount));
       do_tell(keeper, buf2, 19, 0);
       return;
     }     
     act("$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM);
     sprintf(buf2,"The shopkeeper now has %s.\r\n", temp1->short_description);
     send_to_char(buf2, ch);
     drop_excess_gold(ch,amount);
     tot_amount += amount;
     /* Get money from the bank, buy the obj, then put money back. */
     change_gold(keeper, shop_index[shop_nr].bankAccount);
     shop_index[shop_nr].bankAccount = 0;
     change_gold(keeper, -amount);
     
     /* If the shopkeeper has more than 15000 coins, put it in the bank! */
     /* disabled since keepers have so many HP now
        if (GET_GOLD(keeper) > 15000) {
	shop_index[shop_nr].bankAccount += (GET_GOLD(keeper) - 15000);
	GET_GOLD(keeper) = 15000;
        } 
	*/
     obj_from_char(temp1,0);
     if (GET_ITEM_TYPE(temp1) == ITEM_TRASH) {
       extract_obj(temp1,0);
       temp1=0;
     }
     else {
       for (tmp = keeper->inventory; tmp; tmp = tmp->next_content) {
	 if (obj_index[temp1->item_number].virtual ==
	     obj_index[tmp->item_number].virtual) {
	   extract_obj(temp1,0);
	   temp1=0;
	   break;
	 }
       }
     }
     if (temp1 != 0)
       obj_to_char(temp1, keeper,0);
   }
   sprintf(buf2, shop_index[shop_nr].message_sell, GET_NAME(ch),
	   report_cost(tot_amount));
   do_tell(keeper, buf2, 19, 0);
   return;
}


void	shopping_value( char *arg, struct char_data *ch, 
struct char_data *keeper, int shop_nr)
{
   char	argm[100];
   struct obj_data *temp1;

   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   one_argument(arg, argm);

   if (!(*argm)) {
      sprintf(buf2, "%s What do you want me to valuate??", GET_NAME(ch));
      do_tell(keeper, buf2, 19, 0);
      return;
   }

   if (!( temp1 = get_obj_in_list_vis(ch, argm, ch->inventory))) {
      sprintf(buf2, shop_index[shop_nr].no_such_item2, GET_NAME(ch));
      do_tell(keeper, buf2, 19, 0);
      return;
   }

   if (!(trade_with(keeper, ch, temp1, shop_nr))) {
      sprintf(buf2, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
      do_tell(keeper, buf2, 19, 0);
      return;
   }

   sprintf(buf2, "%s I'll give you %s for that!", GET_NAME(ch), report_cost( (int)(temp1->obj_flags.cost *  shop_index[shop_nr].profit_sell)));
   do_tell(keeper, buf2, 19, 0);

   return;
}


void	shopping_list( char *arg, struct char_data *ch,
struct char_data *keeper, int shop_nr)
{
   char	buf[MAX_STRING_LENGTH], buf2[100], buf3[100], buf4[100];
   struct obj_data *temp1;
   int	found_obj;

   if (!(is_ok(keeper, ch, shop_nr)))
      return;

   strcpy(buf, "You can buy:\r\n");
   found_obj = FALSE;
   if (keeper->inventory)
      for (temp1 = keeper->inventory; temp1; temp1 = temp1->next_content)
	 if ((CAN_SEE_OBJ(ch, temp1)) && (temp1->obj_flags.cost > 0)) {
	    found_obj = TRUE;
	    if (temp1->obj_flags.type_flag != ITEM_DRINKCON){
		if (IS_SET(temp1->obj_flags.extra_flags, ITEM_RESIZED))
		    sprintf(buf4,"(%s, resized)",object_sizes[GET_OBJ_SIZE(temp1)]);
		else
		    sprintf(buf4,"(%s)",object_sizes[GET_OBJ_SIZE(temp1)]);
	       sprintf(buf2, "%s %s for %s.\r\n" , (temp1->short_description),buf4,report_cost( (int)(temp1->obj_flags.cost *
	           shop_index[shop_nr].profit_buy)));
	    }
	    else {
/*	       if (temp1->obj_flags.value[1])
		  sprintf(buf3, "%s of %s", (temp1->short_description), drinks[temp1->obj_flags.value[2]]);
	       else
	       removed the drink deal by general request AJN 30th Sept. 94*/
		sprintf(buf3, "%s", (temp1->short_description));
		sprintf(buf2, "%s for %s.\r\n", buf3, report_cost((int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy)));
	    }
	    strcat(buf, CAP(buf2));
	 };

   if (!found_obj)
      strcat(buf, "Nothing!\r\n");

   send_to_char(buf, ch);
   return;
}


void	shopping_kill( char *arg, struct char_data *ch,
struct char_data *keeper, int shop_nr)
{
   char	buf[100];

   switch (shop_index[shop_nr].temper2) {
   case 0:
      sprintf(buf, "%s Don't ever try that again!", GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;

   case 1:
      sprintf(buf, "%s Scram - midget!", GET_NAME(ch));
      do_tell(keeper, buf, 19, 0);
      return;

   default :
      return;
   }
}


int	shop_keeper(struct char_data *ch, void *me, int cmd, char *arg)
{
   char	argm[100];
   struct char_data *keeper;
   int	shop_nr;

   keeper = (struct char_data *)me;

   if (cmd < -1)
       return 0;
   if (cmd == -1) {
     if (mob_index[keeper->nr].virtual == 1834) {
       do_say(keeper,"A customer!  It seems like an eternity since I last did any business!",0,0);
       do_say(keeper,"What fine armour can I interest you in today?",0,0);
       return(0);
     }
   }
   for (shop_nr = 0 ; shop_index[shop_nr].keeper != keeper->nr && shop_nr <= number_of_shops; shop_nr++)
      ;

   if ((cmd == 56) && (ch->in_room ==  real_room(shop_index[shop_nr].in_room) || (shop_index[shop_nr].in_room == 0))) {/* Buy */

      shopping_buy(arg, ch, keeper, shop_nr);
      return(TRUE);
   }

   if ((cmd == 57 ) &&
       ((ch->in_room ==  real_room(shop_index[shop_nr].in_room))
	|| shop_index[shop_nr].in_room == 0)) {/* Sell */

      shopping_sell(arg, ch, keeper, shop_nr);
      return(TRUE);
   }

   if ((cmd == 58) && ((ch->in_room ==  real_room(shop_index[shop_nr].in_room))|| (shop_index[shop_nr].in_room == 0))) {/* value */

      shopping_value(arg, ch, keeper, shop_nr);
      return(TRUE);
   }

   if ((cmd == 59) && ((ch->in_room ==  real_room(shop_index[shop_nr].in_room))|| (shop_index[shop_nr].in_room == 0))) {/* List */

      shopping_list(arg, ch, keeper, shop_nr);
      return(TRUE);
   }

   if ((cmd == 25) || (cmd == 70))   /* Kill or Hit */ {
      one_argument(arg, argm);

      if (keeper == get_char_room(argm, ch->in_room) && shop_index[shop_nr].in_room != 0) {
	 shopping_kill(arg, ch, keeper, shop_nr);
	 return(TRUE);
      }
   }/* Cast, recite, use */
   /*   else if (((cmd == 344) || (cmd == 207) || (cmd == 172))
       && shop_index[shop_nr].in_room != 0) {   
       act("$N tells you 'No magic here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
       return TRUE;
       } */
   if (shop_index[shop_nr].shp_func)
       if ((*shop_index[shop_nr].shp_func)(keeper,keeper,cmd,""))
	   return TRUE;

   return(FALSE);
}


void	boot_the_shops(FILE *shop_f, char *filename)
{
   char	*buf, buf2[150];
   int	temp, count, nr;
   long templ;
   bool new = FALSE;

   sprintf(buf2, "beginning of shop file %s", filename);

   for (; ; ) {
      buf = fread_string(shop_f, buf2);
      if (*buf == '#')	/* a new shop */ {
	 if (!number_of_shops)	/* first shop */
	    CREATE(shop_index, struct shop_data, 1);
	 else if (!(shop_index =  (struct shop_data *) realloc( shop_index, (number_of_shops + 1) *  sizeof(struct shop_data )))) {
	    perror("Error in boot shop\n");
	    exit(0);
	 }
	 new = FALSE;
	 if (strstr(buf,"NEW")){
	     new = TRUE;
	     sscanf(buf, "#%d %s\n", &nr, buf2);
	 }
	 else
	     sscanf(buf, "#%d\n", &nr);
	 sprintf(buf2, "shop #%d in shop file %s", nr, filename);
	 shop_index[number_of_shops].virtual = nr;
	 for (count = 0; count < MAX_PROD; count++) {
	    fscanf(shop_f, "%d \n", &temp);
	    if (temp >= 0)
	       shop_index[number_of_shops].producing[count] =  real_object(temp);
	    else
	       shop_index[number_of_shops].producing[count] = temp;
	 }
	 fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_buy);
	 fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_sell);
	 if (!new){
	   shop_index[number_of_shops].tradetype = 0;
	   for (count = 0; count < MAX_TRADE; count++) {
	     fscanf(shop_f, "%d \n", &temp);
	     if (temp > 0 &&
		 !IS_SET(shop_index[number_of_shops].tradetype,(1 << temp)))
	       SET_BIT(shop_index[number_of_shops].tradetype,(1 << temp));
	   }
	 }
	 else{
	   fscanf(shop_f, "%ld \n", &templ);
	   shop_index[number_of_shops].tradetype =  templ;
	 }
	 shop_index[number_of_shops].no_such_item1= fread_string(shop_f, buf2);
	 shop_index[number_of_shops].no_such_item2= fread_string(shop_f, buf2);
	 shop_index[number_of_shops].do_not_buy =  fread_string(shop_f, buf2);
	 shop_index[number_of_shops].missing_cash1= fread_string(shop_f, buf2);
	 shop_index[number_of_shops].missing_cash2= fread_string(shop_f, buf2);
	 shop_index[number_of_shops].message_buy =  fread_string(shop_f, buf2);
	 shop_index[number_of_shops].message_sell = fread_string(shop_f, buf2);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].temper1);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].temper2);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].keeper);

	 shop_index[number_of_shops].keeper =  real_mobile(shop_index[number_of_shops].keeper);

	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].with_who);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].in_room);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].open1);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].close1);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].open2);
	 fscanf(shop_f, "%d \n", &shop_index[number_of_shops].close2);
	 shop_index[number_of_shops].shp_func = 0;
	 shop_index[number_of_shops].bankAccount = 0;
	 number_of_shops++;
      } else if (*buf == '$')	/* EOF */
	 break;
   }
}


void	assign_the_shopkeepers(void)
{
    int	temp1;
    
    for (temp1 = 0 ; temp1 < number_of_shops ; temp1++)
	mob_index[shop_index[temp1].keeper].func = shop_keeper;
    
}








