/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

extern struct room_data *world;
extern int top_of_world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void	assign_mobiles(void)
{
    SPECIAL(mime);
    SPECIAL(postmaster);
    SPECIAL(gaspode);
    SPECIAL(cityguard);
    SPECIAL(receptionist);
    SPECIAL(c_receptionist);   
    SPECIAL(cryogenicist);
    SPECIAL(blacksmith);
    SPECIAL(lorshroompest);
    SPECIAL(warrior);
    SPECIAL(leviathan);
    SPECIAL(satyr_toadie);
    SPECIAL(princess);    
/*   SPECIAL(guild_guard); */
    SPECIAL(tomcat);
    SPECIAL(guild);    
    SPECIAL(puff);
    SPECIAL(fido);
    SPECIAL(garm);
    SPECIAL(satyrguard);        
    SPECIAL(janitor);
    SPECIAL(mayor);
    SPECIAL(snake);
    SPECIAL(thief);
    SPECIAL(tax);
    SPECIAL(puppy);
    SPECIAL(kitten);
    SPECIAL(brass_dragon);
    SPECIAL(dragon);    
    SPECIAL(ticket_collector);
    SPECIAL(armourer);
    SPECIAL(librarian);
    SPECIAL(magician);
    SPECIAL(bianca);
    SPECIAL(water_dragon);
    SPECIAL(pigeon);
    SPECIAL(diels);
    SPECIAL(banana);
    SPECIAL(guild_guard);
    SPECIAL(blue_caterpillar);
    SPECIAL(satyr_skald);
    SPECIAL(pet_shop);
    
    ASSIGNMOB(1, puff);
/* Lamman*/
    ASSIGNMOB(1016, warrior);
    ASSIGNMOB(1840, warrior);
    ASSIGNMOB(1832, armourer);
    ASSIGNMOB(2079, princess);    
    ASSIGNMOB(2036, gaspode);
    ASSIGNMOB(2022, janitor);
    ASSIGNMOB(2015, guild_guard);
    ASSIGNMOB(2012, guild);
    ASSIGNMOB(2083, guild);
    ASSIGNMOB(2011, guild);
    ASSIGNMOB(1206, guild);    
    ASSIGNMOB(2080, guild);    
    ASSIGNMOB(2013, guild);
    ASSIGNMOB(2014, guild);
    ASSIGNMOB(2054, guild);
    ASSIGNMOB(2070, guild);    
    ASSIGNMOB(1800, guild);
    ASSIGNMOB(1219, guild);
    ASSIGNMOB(1221, guild);
    ASSIGNMOB(1222, guild);
    ASSIGNMOB(1223, guild);
    ASSIGNMOB(2067, guild);            
    ASSIGNMOB(2069, pet_shop);
    ASSIGNMOB(2007, receptionist);
    ASSIGNMOB(2019, tax);
    ASSIGNMOB(2035  , thief);
    ASSIGNMOB(1823  , thief);    
    ASSIGNMOB(2056  , thief);
    ASSIGNMOB(2038  , thief);
    ASSIGNMOB(2039  , thief);
    ASSIGNMOB(2043  , thief);
    ASSIGNMOB(2047  , thief);
    ASSIGNMOB(2020, cityguard);
    ASSIGNMOB(2071, cityguard);
    ASSIGNMOB(2068, cityguard);    
    ASSIGNMOB(2021, cityguard);
    ASSIGNMOB(2028, cityguard);
    ASSIGNMOB(2040, cityguard);
    ASSIGNMOB(2027, cityguard);
    ASSIGNMOB(2050, cityguard);
    ASSIGNMOB(2055, cityguard);
    ASSIGNMOB(2010, postmaster);
    ASSIGNMOB(2031, puppy);
    ASSIGNMOB(2024, warrior);
    ASSIGNMOB(1840, warrior);   
    ASSIGNMOB(2060, tomcat);
    ASSIGNMOB(2030, kitten);    
    ASSIGNMOB(2051, blacksmith);
    ASSIGNMOB(2052, blacksmith);

}



/* assign special procedures to objects */
void	assign_objects(void)
{
   SPECIAL(bank);
   SPECIAL(gen_board);
   SPECIAL(animate_book);
   
   ASSIGNOBJ(2096, gen_board);  /* builders board */
   ASSIGNOBJ(2097, gen_board);  /* bugs board */
   ASSIGNOBJ(2098, gen_board);	/* immortal board */
   ASSIGNOBJ(2099, gen_board);  /* mortal board */
   ASSIGNOBJ(1227, gen_board);  /* builder2 board */
   ASSIGNOBJ(1241, gen_board);  /* Staff Meeting */   

}



/* assign special procedures to rooms */
void	assign_rooms(void)
{
    SPECIAL(spinner);
    SPECIAL(ferry);
    SPECIAL(to_arg_isle);
    extern int dts_are_dumps;
    int i;
    
    SPECIAL(dump);
    SPECIAL(bank);
    SPECIAL(pet_shops);
    SPECIAL(echo_room);

    ASSIGNROOM(2042, bank);
    
   if (dts_are_dumps)
      for (i = 0; i < top_of_world; i++)
         if (IS_SET(world[i].room_flags, DEATH))
            world[i].funct = dump;
}






