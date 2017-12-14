/* ************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

/*Linux does not have a SIGBUS 
#ifdef LINUX
#define SIGBUS SIGUNUSED
#endif
*/

extern struct weather_data weather_info;

/* public functions */
char	*str_dup(const char *source);
int	str_cmp(char *arg1, char *arg2);
int	strn_cmp(char *arg1, char *arg2, int n);
void	logg(char *str);
void	mudlog(char *str, char type, ubyte level, byte file);
void	log_death_trap(struct char_data *ch);
int	MAX(int a, int b);
int	MIN(int a, int b);
int	number(int from, int to);
int	dice(int number, int size);
void	sprintbit(long vektor, char *names[], char *result);
void	sprinttype(int type, char *names[], char *result);
struct time_info_data age(struct char_data *ch);
void assertmsg(char *file, unsigned int line, char *messg);

/* in magic.c */
bool	circle_follow(struct char_data *ch, struct char_data * victim);

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif
/*my ASSERT macro */
#ifdef DEBUG
#define ASSERT(f, message) ((f) ? NULL: assertmsg(__FILE__,__LINE__,message))
#else
#define ASSERT(f, message)  NULL
#endif
/* defines for mudlog() */
    
#define OFF	0
#define BRF	1
#define NRM	2

#define CMP	3

#define TRUE  1

#define FALSE 0

#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

/* Functions in utility.c                     */
/* #define MAX(a,b) (((a) > (b)) ? (a) : (b)) */
/* #define MIN(a,b) (((a) < (b)) ? (a) : (b)) */

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("realloc failure"); abort(); } } while(0)

#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

#define IS_NPC(ch)  ((ch) && IS_SET((ch)->specials2.act, MOB_ISNPC))
#define CAN_SPEAK(ch) ( !IS_MOB((ch)) || GET_INT((ch)) > 10 || \
			IS_SET((ch)->specials2.act, MOB_CAN_SPEAK))
#define IS_MOB(ch)  (IS_NPC((ch)) && ((ch)->nr >-1))
#define HASROOM(obj) ((GET_ITEM_TYPE((obj)) == ITEM_CONTAINER) && \
		       ((obj)->obj_flags.value[3] > 0)) && \
                       (real_room((obj)->obj_flags.value[3]) != -1)
#define MOB_FLAGS(ch) ((ch)->specials2.act)
#define PLR_FLAGS(ch) ((ch)->specials2.act)
#define PRF_FLAGS(ch) ((ch)->specials2.pref)

#define MOB_FLAGGED(ch, flag) (IS_NPC((ch)) && IS_SET(MOB_FLAGS((ch)), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC((ch)) && IS_SET(PLR_FLAGS((ch)), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET(PRF_FLAGS((ch)), (flag)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch), (flag))) & (flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))
#define IS_GODITEM(item) (IS_SET((item)->obj_flags.extra_flags, ITEM_GODONLY))
#define IS_CART(item) (IS_SET((item)->obj_flags.extra_flags, ITEM_PULLABLE))   
#define IS_TWOHANDED(ch,weapon) ((IS_SET((weapon)->obj_flags.extra_flags,ITEM_2HANDED)\
				  && !(GET_SIZE(ch) < GET_OBJ_SIZE(weapon)-1)\
				 || ((GET_SIZE(ch) > GET_OBJ_SIZE(weapon)+1))\
				     && !IS_SET((weapon)->obj_flags.extra_flags,ITEM_2HANDED)))
    
#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define IS_ANIMAL(ch) (IS_NPC(ch) && ((GET_RACE(ch) == CLASS_ANIMAL || \
				       !CAN_SPEAK(ch))))
#define IS_UNDEAD(ch) (IS_NPC(ch) && ((GET_RACE(ch) == CLASS_UNDEAD)))
				       
#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->specials.affected_by, (skill)) )

#define IS_DARK(room)  ( !world[(room)].light && \
                         (IS_SET(world[(room)].room_flags, DARK) || \
                          ( ( world[(room)].sector_type != SECT_INSIDE && \
                              world[(room)].sector_type != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
			     weather_info.sunlight == SUN_DARK)) ) )

#define IS_LIGHT(room)  (!IS_DARK(room))
#define GET_MOOD(ch) ((ch)->specials.mood)
#define GET_INVIS_LEV(ch) ((ch)->specials2.invis_level)
#define IS_LIMITED(obj)  ((obj)->obj_flags.value[6] >0 ? 1: 0)

/* Can subject see character "obj"? */

#define CAN_SEE(sub, obj) ( ((sub) == (obj)) || ( \
      ( (GET_LEVEL(sub) >= GET_INVIS_LEV(obj)) && \
      ( (PRF_FLAGGED((sub), PRF_HOLYLIGHT)) ||    \
           ((!IS_AFFECTED((sub), AFF_BLIND)) &&   \
           (can_see_char((sub), (obj))))))))

#define WIMP_LEVEL(ch) ((ch)->specials2.wimp_level)

#define GET_REQ(i) ((i)<2  ? "Awful" :((i)<4  ? "Bad"     :((i)<7  ? "Poor"      :\
((i)<10 ? "Average" :((i)<14 ? "Fair"    :((i)<20 ? "Good"    :((i)<24 ? "Very good" :\
        "Superb" )))))))

#define SIZE(i) (((i)==0) ?"Normal":(((i)==1) ? "Huge":(((i)==2)  ? "Large" :\
(((i)==3) ? "Normal":(((i)==4) ? "Small":(((i)==5) ? "Very Small": \
       (((i)==6) ? "Tiny" :  "Error" )))))))
#define GET_SIZE(ch) (GET_HEIGHT(ch) < 100 ? 6 :(GET_HEIGHT(ch) < 135 ? 5\
    :(GET_HEIGHT(ch) < 170 ? 4 :(GET_HEIGHT(ch) < 190 ? 3\
				 :(GET_HEIGHT(ch) < 220 ? 2: 1)))))
#define  GET_OBJ_SIZE(obj) ((obj)->obj_flags.value[5] == 0 ? 3 :\
                                                (obj)->obj_flags.value[5])
    
#define HSHR(ch) ((ch)->player.sex ?					\
                    (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ? 					\
	(((ch)->player.sex == 1) ? "him" : "her") : "it")	

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials2.conditions[(i)])

#define GET_LOADROOM(ch) ((ch)->specials2.load_room)

#define GET_NAME(ch)    (IS_NPC(ch) ? (ch)->player.short_descr : (ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)


#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_SUB_LEVEL(ch)   ((ch)->points.sub_level)
#define GET_SPELL_LEVEL(ch,sn) (spell_info[(sn)].min_level*(calc_difficulty((ch),(sn))+1))    

#define GET_CLASS(ch)   ((ch)->player.race)
#define GET_RACE(ch)   ((ch)->player.race)

#define GET_HOME(ch)   ((ch)->player.hometown)

#define GET_AGE(ch)     (age(ch).year)

#define GET_STR(ch)     ((ch)->tmpabilities.str)
#define GET_RAW_STR(ch)     ((ch)->abilities.str)

#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)

#define GET_DEX(ch)     ((ch)->tmpabilities.dex)
#define GET_RAW_DEX(ch)     ((ch)->abilities.dex)

#define GET_INT(ch)     ((ch)->tmpabilities.intel)
#define GET_RAW_INT(ch)     ((ch)->abilities.intel)

#define GET_WIS(ch)     ((ch)->tmpabilities.wis)
#define GET_RAW_WIS(ch)     ((ch)->abilities.wis)

#define GET_CON(ch)     ((ch)->tmpabilities.con)
#define GET_RAW_CON(ch)     ((ch)->abilities.con)

#define GET_CHR(ch)     ((ch)->tmpabilities.chr)
#define GET_RAW_CHR(ch)     ((ch)->abilities.chr)

#define GET_PER(ch)     ((ch)->tmpabilities.per)
#define GET_RAW_PER(ch)     ((ch)->abilities.per)

#define GET_GUI(ch)     ((ch)->tmpabilities.gui)
#define GET_RAW_GUI(ch)     ((ch)->abilities.gui)

#define GET_FOC(ch)     ((ch)->tmpabilities.foc)
#define GET_RAW_FOC(ch)     ((ch)->abilities.foc)

#define GET_DEV(ch)     ((ch)->tmpabilities.dev)
#define GET_RAW_DEV(ch)     ((ch)->abilities.dev)    

#define GET_LUC(ch)     ((ch)->tmpabilities.luc)
#define GET_RAW_LUC(ch)     ((ch)->abilities.luc)

#define STRENGTH_APPLY_INDEX(ch) ((ch)->tmpabilities.str)

#define GET_AC(ch)      ((ch)->points.armor[0])
#define GET_BODY_AC(ch)      ((ch)->points.armor[0])
#define GET_LEGS_AC(ch)      ((ch)->points.armor[1])
#define GET_ARMS_AC(ch)      ((ch)->points.armor[2])
#define GET_HEAD_AC(ch)      ((ch)->points.armor[3])
#define GET_BODY_STOPPING(ch)      ((ch)->points.stopping[0])
#define GET_LEGS_STOPPING(ch)      ((ch)->points.stopping[1])
#define GET_ARMS_STOPPING(ch)      ((ch)->points.stopping[2])
#define GET_HEAD_STOPPING(ch)      ((ch)->points.stopping[3])



/* The hit_limit, move_limit, and mana_limit functions are gone.  See
   limits.c for details.
*/

#define GET_HIT(ch)	((ch)->points.hit)
#define GET_MAX_HIT(ch)	((ch)->points.max_hit)

#define GET_MOVE(ch)	((ch)->points.move)
#define GET_MAX_MOVE(ch) ((ch)->points.max_move)

#define GET_MANA(ch)	((ch)->points.mana)
#define GET_MAX_MANA(ch) ((ch)->points.max_mana)

    
#define GET_GOLD(ch)	((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)

#define GET_EXP(ch)	((ch)->points.exp)

#define GET_HEIGHT(ch)	((ch)->player.height)

#define GET_WEIGHT(ch)	((ch)->player.weight)

#define GET_SEX(ch)	((ch)->player.sex)

#define GET_HITROLL(ch) ((ch)->points.hitroll)
#define GET_DAMROLL(ch) ((ch)->points.damroll)
#define GET_HITROLL2(ch) ((ch)->specials.hitroll)
#define GET_DAMROLL2(ch) ((ch)->specials.damroll)

#define SPELLS_TO_LEARN(ch) ((ch)->specials2.spells_to_learn)

#define GET_IDNUM(ch) (IS_NPC(ch) ? -1 : (ch)->specials2.idnum)
#define GET_LAST_TELL(ch) (IS_NPC(ch) ? -1 : (ch)->specials2.last_tell_id)
#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)

#define GET_SKILL(ch, i) ((ch)->skills ? (((ch)->skills)[i]) : 24)

#define SET_SKILL(ch, i, pct) { if ((ch)->skills) (ch)->skills[i] = pct; }

#define WAIT_STATE(ch, cycle) (((ch)->desc) ? (ch)->desc->wait = \
			       MAX((ch)->desc->wait, (cycle)): 0)
			 
#define CHECK_WAIT(ch) (((ch)->desc) ? ((ch)->desc->wait > 1) : 0)


/* Object And Carry related macros */

#define CAN_SEE_OBJ(sub, obj)                                    \
	(( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) ||   \
	     IS_AFFECTED((sub),AFF_DETECT_INVISIBLE) ) &&               \
	     !IS_AFFECTED((sub),AFF_BLIND)) && IS_LIGHT((sub)->in_room) ) \
	     || PRF_FLAGGED((sub), PRF_HOLYLIGHT) \
	    || (can_see_obj((sub),(obj))))


#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch) (20+GET_DEX(ch)/2+GET_LEVEL(ch)/5)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define IS_PULLING(ch) (((ch)->specials.cart) ? (ch)->specials.cart : 0)    
#define IS_RIDING(ch) (((ch)->specials.mount) ? (ch)->specials.mount: 0)
    
#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : ((GET_INT(ch) > 10) ? ((GET_LEVEL(ch) >= LEVEL_BUILDER) ? "someone" : "somebody") : "something"))

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(world[(ch)->in_room].room_flags,INDOORS))

#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door)  &&  (EXIT(ch,door)->to_room != NOWHERE) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define TO_RM(ch, cmd) (real_room(EXIT(ch,cmd)->to_room))
	
#define IS_CLIMB(ch,cmd) ((cmd == 4 || cmd == 5) &&  \
	(CAN_GO(ch,cmd)) &&  \
	(CLIMB_ROOM(TO_RM(ch,cmd)) || CLIMB_ROOM((ch)->in_room)))
    
#define CLIMB_ROOM(room) (IS_SET(world[room].room_flags,CLIMB_EASY) || \
	IS_SET(world[room].room_flags,CLIMB_MODERATE) || \
	IS_SET(world[room].room_flags,CLIMB_HARD) || \
	IS_SET(world[room].room_flags,CLIMB_SEVERE) || \
	IS_SET(world[room].room_flags,CLIMB_EXTREME))
    
#define GET_ALIGNMENT(ch) ((ch)->specials2.alignment)
#define GET_FAME(ch) ((ch)->specials2.fame)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : race_abbrevs[(int)GET_RACE(ch)])
#define RACE_ABBR(ch) (IS_NPC(ch) ? "--" : race_abbrevs[(int)GET_RACE(ch)])

