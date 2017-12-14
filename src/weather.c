/* ************************************************************************
*   File: weather.c                                     Part of CircleMUD *
*  Usage: functions handling time and the weather                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"

extern struct time_info_data time_info;
extern struct obj_data *object_list;
extern struct room_data *world;

void	weather_and_time(int mode);
void	another_hour(int mode);
void	weather_change(void);


void	weather_and_time(int mode)
{
   another_hour(mode);
   if (mode)
      weather_change();
}


void	another_hour(int mode)
{
  time_info.hours++;


  if (mode) {
    switch (time_info.hours) {
    case 0:
      weather_info.moonlight = MOON_LIGHT;
      break;
    case 5 :
      weather_info.sunlight = SUN_RISE;
      send_to_outdoor("The sun rises in the east.\r\n");
      break;
    case 6 :
      weather_info.sunlight = SUN_LIGHT;
      send_to_outdoor("The day has begun.\r\n");
      break;
    case 8:
      weather_info.moonlight = MOON_SET;
      switch(weather_info.moon_phase) {
      case 0:
	send_to_outdoor("The new moon disappears in the west.\r\n");
	break;
      case 1:
	send_to_outdoor("A thin crescent moon sets in the west.\r\n");
	break;
      case 2:
	send_to_outdoor("The crescent moon sinks below the western horizon.\r\n");
	break;
      case 3:
	send_to_outdoor("A gibbous moon dips below the western horizon.\r\n");
	break;
      case 4:
	send_to_outdoor("The full moon sets in the west.\r\n");
	break;
      case 5:
	send_to_outdoor("A gibbous moon disappears below the western horizon.\r\n");
	break;	
      case 6:
	send_to_outdoor("The crescent moon sets in the west.\r\n");
	break;
      case 7:
	send_to_outdoor("The thin crescent moon dips below the western horizon.\r\n");
	break;
      case 8:
	send_to_outdoor("The blue moon sets.\r\n");
	break;
      }
      break;
    case 10:
      weather_info.moonlight = MOON_DARK;
      break;
    case 21 :
      weather_info.sunlight = SUN_SET;
      send_to_outdoor("The sun slowly disappears in the west.\r\n");
      break;
    case 22 :
      weather_info.sunlight = SUN_DARK;
      send_to_outdoor("The night has begun.\r\n");
      break;
    case 23 :
      weather_info.moonlight = MOON_RISE;
      switch(weather_info.moon_phase) {
      case 0:
	send_to_outdoor("The new moon rises in the east.\r\n");
	break;
      case 1:
	send_to_outdoor("A thin crescent moon rises in the east.\r\n");
	break;
      case 2:
	send_to_outdoor("The crescent moon rises in the east.\r\n");
	break;
      case 3:
	send_to_outdoor("A gibbous moon rises in the east.\r\n");
	break;
      case 4:
	send_to_outdoor("The full moon rises in the east.\r\n");
	break;
      case 5:
	send_to_outdoor("A gibbous moon rises in the east.\r\n");
	break;	
      case 6:
	send_to_outdoor("The crescent moon rises in the east.\r\n");
	break;
      case 7:
	send_to_outdoor("A thin crescent moon rises in the east.\r\n");
	break;
      case 8:
	send_to_outdoor("A blue moon rises in the east.\r\n");
	break;
      }
      break;
    default :
      break;
    }
  }

  if (time_info.hours > 23) {  /* Changed by HHS due to bug ???*/
    time_info.hours -= 24;
    time_info.day++;
    
    if (time_info.day > 34) {
      time_info.day = 0;
      time_info.month++;
      if (!(time_info.day % 4)) {
	weather_info.moon_phase++;
	if (weather_info.moon_phase == 8) 
	  weather_info.moon_phase == 0;
	if ((weather_info.moon_phase == 5) && !number(0,10))
	  	weather_info.moon_phase == 8;
      	if (weather_info.moon_phase == 9) 
	  	weather_info.moon_phase == 6;
      }
      if (time_info.month > 16) {
	time_info.month = 0;
	time_info.year++;
      }
    }
  }
}


void	weather_change(void)
{
   int	diff, change;
   if ((time_info.month >= 9) && (time_info.month <= 16))
      diff = (weather_info.pressure > 985 ? -2 : 2);
   else
      diff = (weather_info.pressure > 1015 ? -2 : 2);

   weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

   weather_info.change = MIN(weather_info.change, 12);
   weather_info.change = MAX(weather_info.change, -12);

   weather_info.pressure += weather_info.change;

   weather_info.pressure = MIN(weather_info.pressure, 1040);
   weather_info.pressure = MAX(weather_info.pressure, 960);

   change = 0;

   switch (weather_info.sky) {
   case SKY_CLOUDLESS :
      if (weather_info.pressure < 990)
	 change = 1;
      else if (weather_info.pressure < 1010)
	 if (dice(1, 4) == 1)
	    change = 1;
      break;
   case SKY_CLOUDY :
      if (weather_info.pressure < 970)
	 change = 2;
      else if (weather_info.pressure < 990)
	 if (dice(1, 4) == 1)
	    change = 2;
	 else
	    change = 0;
      else if (weather_info.pressure > 1030)
	 if (dice(1, 4) == 1)
	    change = 3;

      break;
   case SKY_RAINING :
      if (weather_info.pressure < 970)
	 if (dice(1, 4) == 1)
	    change = 4;
	 else
	    change = 0;
      else if (weather_info.pressure > 1030)
	 change = 5;
      else if (weather_info.pressure > 1010)
	 if (dice(1, 4) == 1)
	    change = 5;

      break;
   case SKY_LIGHTNING :
      if (weather_info.pressure > 1010)
	 change = 6;
      else if (weather_info.pressure > 990)
	 if (dice(1, 4) == 1)
	    change = 6;

      break;
   default :
      change = 0;
      weather_info.sky = SKY_CLOUDLESS;
      break;
   }

   switch (change) {
   case 0 :
      break;
   case 1 :
      send_to_outdoor("The sky starts to get cloudy.\r\n");
      weather_info.sky = SKY_CLOUDY;
      break;
   case 2 :
      send_to_outdoor("It starts to rain.\r\n");
      weather_info.sky = SKY_RAINING;
      break;
   case 3 :
      send_to_outdoor("The clouds disappear.\r\n");
      weather_info.sky = SKY_CLOUDLESS;
      break;
   case 4 :
      send_to_outdoor("Lightning starts to show in the sky.\r\n");
      weather_info.sky = SKY_LIGHTNING;
      break;
   case 5 :
      send_to_outdoor("The rain stops.\r\n");
      weather_info.sky = SKY_CLOUDY;
      break;
   case 6 :
      send_to_outdoor("The lightning stops.\r\n");
      weather_info.sky = SKY_RAINING;
      break;
   default :
      break;
   }
}




