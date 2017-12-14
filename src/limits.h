/* ************************************************************************
*   File: limits.h                                      Part of CircleMUD *
*  Usage: header file: protoypes of functions in limits.c                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

/* Public Procedures */

int	hit_limit(struct char_data *ch);
int	move_limit(struct char_data *ch);
int	hit_gain(struct char_data *ch);
int	move_gain(struct char_data *ch);
void	advance_level(struct char_data *ch);
void	set_title(struct char_data *ch);
void	gain_exp(struct char_data *ch, int gain, int mode);
void	gain_exp_regardless(struct char_data *ch, int gain, int mode);
void	gain_condition(struct char_data *ch, int condition, int value);
void	check_idling(struct char_data *ch);
void	point_update(void);
void	update_pos( struct char_data *victim );


int levels_table[251];

#define MODE_KILL 1
#define MODE_GROUP_KILL 2
#define MODE_STEAL 3
#define MODE_FLEE 4
#define MODE_DIE 5


















