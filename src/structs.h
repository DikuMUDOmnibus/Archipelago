/* ************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include <sys/types.h>

typedef signed char	sbyte;
typedef unsigned char	ubyte;
typedef unsigned char	bool;
typedef char	byte;

#define LEVEL_IMPL      227
#define LEVEL_ASS_IMPL	226
#define LEVEL_GRGOD	223
#define LEVEL_GOD	220
#define LEVEL_MBUILDER  215
#define LEVEL_BUILDER   210
#define LEVEL_IMMORT	210

#define LEVEL_FREEZE	LEVEL_GRGOD

#define SPEC_ARRIVE    -1
#define SPEC_MOBACT    -2

#define NUM_OF_DIRS	6
#define PULSE_ZONE      40
#define PULSE_MOBILE    40
#define PULSE_ROOM      32
#define PULSE_OBJECT    50
#define PULSE_VIOLENCE  6
#define WAIT_SEC	4
#define WAIT_ROUND	4

#define SMALL_BUFSIZE	    1024
#define LARGE_BUFSIZE	   32768
#define MAX_STRING_LENGTH  16284
#define MAX_INPUT_LENGTH    1024
#define MAX_MESSAGES         200
#define MAX_ITEMS            153
#define MAX_NAME_LENGTH	      15
#define MAX_PWD_LENGTH	      10 /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LEN	      30 /* Used in char_file_u *DO*NOT*CHANGE* */

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  75
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10 
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_FOUNTAIN  23
#define ITEM_ROD       24
#define ITEM_CANTRIP   25
#define ITEM_PHILTRE   26
#define ITEM_EXLIXIR   27
#define ITEM_SRTLIGHT  28
#define ITEM_SCABBARD  29
#define ITEM_BELT      30
#define ITEM_VIS       31
/* Bitvector For 'wear_flags' */

#define ITEM_TAKE           (1 << 0) 
#define ITEM_WEAR_FINGER    (1 << 1)
#define ITEM_WEAR_NECK      (1 << 2)
#define ITEM_WEAR_BODY      (1 << 3)
#define ITEM_WEAR_HEAD      (1 << 4)
#define ITEM_WEAR_LEGS      (1 << 5)
#define ITEM_WEAR_FEET      (1 << 6)
#define ITEM_WEAR_HANDS     (1 << 7) 
#define ITEM_WEAR_ARMS      (1 << 8)
#define ITEM_WEAR_SHIELD    (1 << 9)
#define ITEM_WEAR_ABOUT     (1 << 10)
#define ITEM_WEAR_WAIST     (1 << 11)
#define ITEM_WEAR_WRIST     (1 << 12)
#define ITEM_WIELD          (1 << 13)
#define ITEM_HOLD           (1 << 14)
#define ITEM_THROW          (1 << 15)
#define ITEM_WEAR_EYES      (1 << 16)
#define ITEM_WEAR_EARS      (1 << 17)
#define ITEM_WEAR_MOUTH     (1 << 18)
#define ITEM_WEAR_BACK      (1 << 19)
#define ITEM_WEAR_SCABBARD   (1 << 20)

/* UNUSED, CHECKS ONLY FOR ITEM_LIGHT #define ITEM_LIGHT_SOURCE  65536 */

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            (1 << 0)
#define ITEM_HUM             (1 << 1)
#define ITEM_DARK            (1 << 2)
#define ITEM_LOCK            (1 << 3)
#define ITEM_EVIL	     (1 << 4)
#define ITEM_INVISIBLE       (1 << 5)
#define ITEM_MAGIC           (1 << 6)
#define ITEM_NODROP          (1 << 7)
#define ITEM_BLESS           (1 << 8)
#define ITEM_ANTI_GOOD       (1 << 9)  /* not usable by good people    */
#define ITEM_ANTI_EVIL       (1 << 10) /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL    (1 << 11) /* not usable by neutral people */
#define ITEM_NORENT	     (1 << 12) /* not allowed to rent the item */
#define ITEM_NODONATE	     (1 << 13) /* not allowed to donate the item */
#define ITEM_NOINVIS	     (1 << 14) /* not allowed to cast invis on */
#define ITEM_2HANDED         (1 << 15) /* 2-handed weapon flag */
#define ITEM_EQUIPED         (1 << 16) /* equipedab flag */
#define ITEM_FRAGILE         (1 << 17) /* item especially succeptible to damage */
#define ITEM_RESIZED         (1 << 18)  /* whether item has been resized or not*/
#define ITEM_FERROUS         (1 << 19) /* is item made from iron.. */
#define ITEM_GODONLY         (1 << 20) /* to flag god items */
#define ITEM_PULLABLE        (1 << 21) /* item can be pulled like a cart */
#define ITEM_TEST_ONLY       (1 << 22) /* item cannot be rented */

/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15

/* for containers  - value[1] */

#define CONT_CLOSEABLE      (1 << 0)
#define CONT_PICKPROOF      (1 << 1)
#define CONT_CLOSED         (1 << 2)
#define CONT_LOCKED         (1 << 3)
#define CONT_SEETHRU        (1 << 4)
#define CONT_ONEWAY         (1 << 5)

/* event types */

#define EVENT_IGNORE        0
#define EVENT_COMBAT        1
#define EVENT_SPELL         2
#define EVENT_TELEPORT      3
#define EVENT_ARRIVE        4
#define EVENT_LEAVE         5
#define EVENT_CART_ARRIVE   6
#define EVENT_CART_LEAVE    7
#define EVENT_LOOT          8
#define EVENT_ROOM_DAMAGE   9
#define EVENT_OBJTIMER      10
#define EVENT_ATTACK        11
#define EVENT_REBOOT        12

struct event_type {
    int event;
    int info1;
    int info2;
    int info3;
    int info4;
    char *arg;
    void *subject;
    void *target;
    struct event_type *next;};

struct list_index_type {
  char *entry;
  int index;
  char* components;
};

struct dual_list_type {
   char	*singular;
   char	*plural;
};
struct color_list_type {
   char	*low;
   char	*high;
};

struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 4         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data {
    int	        value[8];	/* Values of the item (see list)    */
    byte        type_flag;	/* Type of item                     */
    int 	wear_flags;	/* Where you can wear it            */
    long        extra_flags;	/* If it hums,glows etc             */
    int	        weight;		/* Weigt what else                  */
    int	        cost;		/* Value when sold (gp.)            */
    int	        cost_per_day;	/* Cost to keep pr. real day        */
    int	        timer;		/* Timer for object                 */
    long	bitvector;	/* To set chars bits                */
};
struct obj_flag2_data {
  sbyte       light;          /* how much light it produces       */
  byte        aff_timer;      /* timer for char affects           */
  byte        no_use_timer;   /* timer for object down time       */
  byte        aff_dur;        /* max duration of char affects     */
  byte        no_use_dur;     /* max duration of object down time */
  long	      bitvector_aff;  /* Store the bitvector here         */
  long        perm_aff;       /*permanant bitvector affects*/
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte  modifier;     /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data {
  short item_number;            /* Where in data-base               */
  short in_room;                /* In what room -1 when conta/carr  */
  
  struct obj_flag_data obj_flags;/* Object information               */
  struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */
  
  char	*name;                    /* Title of object :get etc.        */
  char	*description ;            /* When in room                     */
  char	*short_description;       /* when worn/carry/in cont.         */
  char	*action_description;      /* What to write when used          */
  struct extra_descr_data *ex_description; /* extra descriptions     */
  struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
  struct char_data *worn_by;     /* worn by :NULL in room/conta/invent*/   
  struct follow_type *pulled_by;   /* pulled by: for carts */
  struct obj_data *in_obj;       /* In what object NULL when none    */
  struct obj_data *contains;     /* Contains objects                 */
  
  struct obj_data *next_content; /* For 'contains' lists             */
  struct obj_data *next;         /* For the object list              */
  struct obj_flag2_data obj_flags2; /* new object stuff */
  struct reset_com *reset;             /* reset for objects */  
};

/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */
#define NOBODY    -1    /* nil reference for mobile id's    */
/* Bitvector For 'room_flags' */

#define DARK           (1 << 0)
#define DEATH          (1 << 1)
#define NO_MOB         (1 << 2)
#define INDOORS        (1 << 3)
#define SPIN           (1 << 4)
#define UNFINISHED     (1 << 5)
#define PAIN           (1 << 6)
#define NO_MAGIC       (1 << 7)
#define NO_AUTOFORMAT  (1 << 8)
#define PRIVATE        (1 << 9)
#define GODROOM        (1 << 10)
#define BFS_MARK       (1 << 11)
#define PEACEFULL      (1 << 12)
#define FAST_HP_GAIN   (1 << 13)
#define RECALL         (1 << 14)
#define FAST_MOVE_GAIN (1 << 15)
#define HOT            (1 << 16)
#define COLD           (1 << 17)
#define WARM           (1 << 18)
#define COOL           (1 << 19)
#define NO_SCAN        (1 << 20)
#define SMALL          (1 << 21)
#define TINY           (1 << 22)
#define CLIMB_EASY     (1 << 23)
#define CLIMB_MODERATE (1 << 24)
#define CLIMB_HARD     (1 << 25)
#define CLIMB_SEVERE   (1 << 26)
#define CLIMB_EXTREME  (1 << 27)
#define NO_EXITS       (1 << 28)
#define MOB_DEATH      (1 << 29)
#define RECALL_DISTORT (1 << 30)

#define BFS_ERROR	   -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH	   -3

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR		1
#define EX_CLOSED		2
#define EX_LOCKED		4
#define EX_RSCLOSED		8
#define EX_RSLOCKED		16
#define EX_PICKPROOF		32
#define EX_SECRET               64
#define EX_DARK                 128

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_FLY             8
#define SECT_UNDER_WATER     9
#define SECT_THICKET        10
#define SECT_FIRE           11
#define SECT_ICE            12
#define SECT_DESERT_HOT     13
#define SECT_DESERT_COLD    14

struct room_direction_data {
   char	*general_description;       /* When look DIR.                  */

   char	*keyword;                   /* for open/close                  */

   short exit_info;                /* Exit info                       */
   short key;		            /* Key's number (-1 for no key)    */
   short to_room;                  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data {
   short number;               /* Rooms number                       */
   short zone;                 /* Room zone (for resetting)          */
   int	sector_type;            /* sector type (move/hide)            */
   char	*name;                  /* Rooms name 'You are ...'           */
   char	*description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   long room_flags;           /* DEATH,DARK ... etc                 */

   int  light;                  /* Number of lightsources in room     */
   int	(*funct)();             /* special procedure                  */

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */
   short tele_to_room;     /* room to teleport to                */
   short tele_delay;       /* delay b4 teleport                  */
   char *tele_mesg1;     /* message to teleportee              */
   char *tele_mesg2;     /* message to room                    */
   char *tele_mesg3;     /* message to destination room          */
   struct obj_data *obj; /* pointer to object (for cart etc) */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_SCABBARD   0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
#define WEAR_EYES      18
#define WEAR_EARS_R    19
#define WEAR_EARS_L    20
#define WEAR_MOUTH     21
#define WEAR_BACK      22
#define WEAR_WAIST_2   23

/* For 'char_payer_data' */

#define MAX_TOUNGE  3     /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_SKILLS  528   /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR    24
#define MAX_AFFECT  32    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             (1 << 0)
#define AFF_INVISIBLE         (1 << 1)
#define AFF_DETECT_EVIL       (1 << 2)
#define AFF_DETECT_INVISIBLE  (1 << 3)
#define AFF_DETECT_MAGIC      (1 << 4)
#define AFF_SENSE_LIFE        (1 << 5)
#define AFF_SPELLBOTCH        (1 << 6)
#define AFF_SANCTUARY         (1 << 7)
#define AFF_GROUP             (1 << 8)
#define AFF_CURSE             (1 << 9)
#define AFF_CONFUSION         (1 << 10)
#define AFF_POISON            (1 << 11)
#define AFF_PROTECT_EVIL      (1 << 12)
#define AFF_PARALYSIS         (1 << 13)
#define AFF_RESIST_HEAT       (1 << 14)
#define AFF_RESIST_COLD       (1 << 15)
#define AFF_SLEEP             (1 << 16)
#define AFF_DODGE             (1 << 17)
#define AFF_SNEAK             (1 << 18)
#define AFF_HIDE              (1 << 19)
#define AFF_FEAR              (1 << 20)
#define AFF_CHARM             (1 << 21)
#define AFF_FOLLOW            (1 << 22)
#define AFF_WIMPY	      (1 << 23)
#define AFF_INFRARED          (1 << 24)
#define AFF_FLY               (1 << 25)
#define AFF_WATER_BREATH      (1 << 26)
#define AFF_FREE_ACTION       (1 << 27)
#define AFF_BASH              (1 << 28)
#define AFF_SLIPPY            (1 << 29)
#define AFF_HEALING           (1 << 30)
/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_RACE              7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_BODY_AC          17
#define APPLY_BODY_ARMOR       17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_CHR              25
#define APPLY_PER              26
#define APPLY_GUI              27
#define APPLY_LUC              28
#define APPLY_LEGS_AC          29
#define APPLY_ARMS_AC          30
#define APPLY_HEAD_AC          31
#define APPLY_BODY_STOPPING    32
#define APPLY_LEGS_STOPPING    33
#define APPLY_ARMS_STOPPING    34
#define APPLY_HEAD_STOPPING    35
#define APPLY_ALL_STOPPING     36
#define APPLY_POWER            37
#define APPLY_FOCUS            38
#define APPLY_DEVOTION         39
#define APPLY_ALL_AC           40
#define APPLY_ALL_SAVE         41
#define APPLY_HITROLL2         42
#define APPLY_DAMROLL2         43
/* 'class' for PC's */
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_THIEF       3
#define CLASS_WARRIOR     4

/* 'race' for PC's */
#define RACE_HUMAN        1
#define RACE_ELVEN        2
#define RACE_HALFLING     3
#define RACE_GIANT        4
#define RACE_GNOME        5
#define RACE_HALF_ELF     6
#define RACE_OGIER        7
#define RACE_DWARF        8
#define RACE_MERMAN       9
#define RACE_PIXIE       10
#define RACE_AMARYA      11
#define RACE_TROLL       12


/* 'class' for NPC's */
#define CLASS_UNDEAD    126 /* These are not yet used!   */
#define CLASS_OTHER       1 /* But may soon be so        */
#define CLASS_HUMANOID    2 /* Idea is to use general    */
#define CLASS_ANIMAL      3 /* monster classes           */
#define CLASS_DRAGON      4 /* Then for example a weapon */
#define CLASS_GIANT       5 /* of dragon slaying is pos. */
#define CLASS_PLANT       6 /* plants */
#define CLASS_EQUINE      7 /* horses etc. */
#define CLASS_ICHTHYOID   8 /* fish */
#define CLASS_ARTHOPOD    9 /* insects etc */
#define CLASS_DEMON      10 /* demons  etc */
/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_STANDING   7
#define POSITION_FIGHTING   8



/* for mobile actions: specials.act */
#define MOB_SPEC         (1 << 0)  /* spec-proc to be called if exist   */
#define MOB_SENTINEL     (1 << 1)  /* this mobile not to be moved       */
#define MOB_SCAVENGER    (1 << 2)  /* pick up stuff lying around        */
#define MOB_ISNPC        (1 << 3)  /* This bit is set for use with IS_NPC()*/
#define MOB_NICE_THIEF   (1 << 4)  /* Set if a thief should NOT be killed  */
#define MOB_AGGRESSIVE   (1 << 5)  /* Set if automatic attack on NPC's     */
#define MOB_STAY_ZONE    (1 << 6)  /* MOB Must stay inside its own zone    */
#define MOB_WIMPY        (1 << 7)  /* MOB Will flee when injured, and if   */
/* aggressive only attack sleeping players */

/*
 * For MOB_AGGRESSIVE_XXX, you must also set MOB_AGGRESSIVE.
 * These switches can be combined, if none are selected, then
 * the mobile will attack any alignment (same as if all 3 were set)
 */
#define MOB_AGGRESSIVE_EVIL	(1 << 8) /* auto attack evil PC's only	*/
#define MOB_AGGRESSIVE_GOOD	(1 << 9) /* auto attack good PC's only	*/
#define MOB_AGGRESSIVE_NEUTRAL	(1 << 10) /* auto attack neutral PC's only	*/
#define MOB_MEMORY		(1 << 11) /* remember attackers if struck first */
#define MOB_HELPER		(1 << 12) /* attack chars attacking a PC in room */
#define MOB_CITIZEN             (1 << 13)
#define MOB_CAN_SPEAK           (1 << 14)
#define MOB_WILL_LOOT           (1 << 15) /* mob will loot corpses */
#define MOB_MOODS               (1 << 16)
#define MOB_HAPPY               (1 << 17) /* +700 to mood*/
#define MOB_SAD                 (1 << 18) /* -700 to mood*/
#define MOB_STAY_SECTOR         (1 << 19) /* mob will stay in sector type*/
#define MOB_DOCILE              (1 << 20) /* mob may be ridden without charm */
#define MOB_TETHERED            (1 << 21)
#define MOB_SPELL_CASTER        (1 << 22)
#define MOB_POISONOUS           (1 << 23) 
/* For players : specials.act */
#define PLR_KILLER	(1 << 0)
#define PLR_THIEF	(1 << 1)
#define PLR_FROZEN	(1 << 2)
#define PLR_DONTSET     (1 << 3)   /* Dont EVER set (ISNPC bit) */
#define PLR_WRITING	(1 << 4)
#define PLR_MAILING	(1 << 5)
#define PLR_CRASH	(1 << 6)
#define PLR_SITEOK	(1 << 7)
#define PLR_NOSHOUT	(1 << 8)
#define PLR_NOTITLE	(1 << 9)
#define PLR_DELETED	(1 << 10)
#define PLR_LOADROOM	(1 << 11)
#define PLR_NOWIZLIST	(1 << 12)
#define PLR_NODELETE	(1 << 13)
#define PLR_INVSTART	(1 << 14)
#define PLR_CRYO	(1 << 15)
#define PLR_BUILDING    (1 << 16)
#define PLR_AFK         (1 << 17)
#define PLR_NEEDCRLF    (1 << 18)

/* for players: preference bits */
#define PRF_BRIEF       (1 << 0)
#define PRF_COMPACT     (1 << 1)
#define PRF_DEAF	(1 << 2)
#define PRF_NOTELL      (1 << 3)
#define PRF_DISPHP	(1 << 4)
#define PRF_DISPMANA	(1 << 5)
#define PRF_DISPMOVE	(1 << 6)
#define PRF_DISPAUTO	(1 << 7)
#define PRF_NOHASSLE	(1 << 8)
#define PRF_QUEST	(1 << 9)
#define PRF_SUMMONABLE	(1 << 10)
#define PRF_NOREPEAT	(1 << 11)
#define PRF_HOLYLIGHT	(1 << 12)
#define PRF_COLOR_1	(1 << 13)
#define PRF_COLOR_2	(1 << 14)
#define PRF_NOWIZ	(1 << 15)
#define PRF_LOG1	(1 << 16)
#define PRF_LOG2	(1 << 17)
#define PRF_NOAUCT	(1 << 18)
#define PRF_NOGOSS	(1 << 19)
#define PRF_NOGRATZ	(1 << 20)
#define PRF_ROOMFLAGS	(1 << 21)
#define PRF_NOBRAG      (1 << 22)
#define PRF_NOCOND      (1 << 23)  /* not see the condition on items */
#define PRF_DEBUG       (1 << 24)  /* see damage from spells and weapons */


#define BUILD_ZONE       (1<<0)
#define BUILD_SHOPS      (1<<1)
#define BUILD_MOBS       (1<<2)
#define BUILD_ROOMS      (1<<3)
#define BUILD_OBJS       (1<<4)

struct memory_rec_struct {
   long	id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   byte hours, day, month;
   short year;
};
#define MAX_ALIASES 5

struct pc_stat_parameters {
  byte min, max;
};

struct alias_data {
  char *alias;
  char *text;
};

/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   int	played;     /* This is the total accumulated time played in secs */
};

struct char_player_data {
  char	*name;	       /* PC / NPC s name (kill ...  )         */
  char	*short_descr;  /* for 'actions'                        */
  char	*long_descr;   /* for 'look'.. Only here for testing   */
  char	*description;  /* Extra descriptions                   */
  char	*title;        /* PC / NPC s title                     */
  char	*prmpt;        /* prompt string                        */
  byte sex;           /* PC / NPC s sex                       */
  byte race;         /* PC s race or NPC alignment          */
  ubyte  level;         /* PC / NPC s level                     */
  int	hometown;      /* PC s Hometown (zone)                 */
  bool talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC           */
  struct time_data time;  /* PC s AGE in days                 */
  short weight;       /* PC / NPC s weight                    */
  short height;       /* PC / NPC s height                    */
  long body_vector;
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte chr;
   sbyte per;
   sbyte gui;
   sbyte foc;
   sbyte dev;
   sbyte luc;
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data {
   short mana;
   short max_mana;     /* Max move for PC/NPC			   */
   short sub_level;
   short max_power;     /* Max move for PC/NPC			   */
   short hit;
   short max_hit;      /* Max hit for PC/NPC                      */
   short move;
   short max_move;     /* Max move for PC/NPC                     */

   short armor[4];        /* Internal -100..100, external -10..10 AC */
   short stopping[4];
   int	gold;           /* Money carried                           */
   int	bank_gold;	/* Gold the char has in a bank account	   */
   int	exp;            /* The experience of the player            */

   sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
   sbyte damroll;       /* Any bonus or penalty to the damage roll */
};

/* char_special_data's fields are fields which are needed while the game
   is running, but are not stored in the playerfile.  In other words,
   a struct of type char_special_data appears in char_data but NOT
   char_file_u.  If you want to add a piece of data which is automatically
   saved and loaded with the playerfile, add it to char_special2_data.
   */
struct char_special_data {
  struct char_data *fighting; /* Opponent                             */
  struct char_data *hunting;  /* Hunting person..                     */

  long	affected_by;        /* Bitvector for spells/skills affected by */

  byte position;           /* Standing or ...                         */
  byte default_pos;        /* Default position for NPC                */

  int	carry_weight;       /* Carried weight                          */
  byte carry_items;        /* Number of items carried                 */
  int	timer;              /* Timer for update                        */
  short was_in_room;      /* storage of location for linkdeads*/

  byte damnodice;          /* The number of damage dice's	       */
  byte damsizedice;        /* The size of the damage dice's           */
  byte last_direction;     /* The last direction the monster went     */
  int	attack_type;        /* The Attack Type Bitvector for NPC's     */

  char	*poofIn;	    /* Description on arrival of a god.	       */
  char	*poofOut; 	    /* Description upon a god's exit.	       */
  short mood; 	    /* mood for mobs		       */
  
  sbyte hitroll;
  sbyte damroll;
  
  memory_rec *memory;	    /* List of attackers to remember	       */
  struct obj_data *cart;   /* to allow mobs/pc to pull carts etc.    */
  struct char_data *mount;
  struct char_data *rider;
  struct char_data *carrying;
  struct char_data *carried_by;
  struct alias_data aliases[MAX_ALIASES+1];
};


struct char_special2_data {
   long	idnum;			/* player's idnum			*/
   short load_room;            /* Which room to place char in		*/
   int spells_to_learn;	        /* How many can you learn yet this level*/
   int	alignment;		/* +-1000 for alignments                */
   long	act;			/* act flag for NPC's; player flag for PC's */
   long	pref;			/* preference flags for PC's.		*/
   int	wimp_level;		/* Below this # of hit points, flee!	*/
   ubyte freeze_level;		/* Level of god who froze char, if any	*/
   ubyte bad_pws;		/* number of bad password attemps	*/
   short apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
   sbyte conditions[3];         /* Drunk full etc.			*/

   ubyte invis_level;
   ubyte rent_zone;
   ubyte spare2;
   ubyte spare3;
   ubyte spare4;
   ubyte spare5;
   ubyte spare6;
   ubyte spare7;
   ubyte spare8;
   ubyte spare9;
   ubyte spare10;
   ubyte spare11;
   long	edit_zone;
   long	builder_flag;
   long	edit_zone2;
   long	edit_zone3;
   long	last_tell_id; /* idnum of last player to tell		*/  
   long	fame;
   long	spare18;
   long	spare19;
   long	spare20;
   long	spare21;
};



/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type {
  int type;           /* The type of spell that caused this      */
  short duration;      /* For howl ong its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  long	bitvector;       /* Tells which bits to set (AFF_XXX)       */
  int level;
  struct affected_type *next;
};

struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== */
struct char_data {
  short nr;                            /* monster nr (pos in file)      */
  short in_room;                       /* Location                      */
  int light;
  
  struct char_player_data player;       /* Normal data                   */
  struct char_ability_data abilities;   /* Abilities                     */
  struct char_ability_data tmpabilities;/* The abilities we will use     */
  struct char_point_data points;        /* Points                        */
  struct char_special_data specials;    /* Special plaing constants      */
  struct char_special2_data specials2;  /* Additional special constants  */
  byte *skills;			 /* dynam. alloc. array of skills */
  
  struct affected_type *affected;       /* affected by what spells       */
  struct obj_data *equipment[MAX_WEAR]; /* Equipment array               */
  
  struct obj_data *inventory;            /* Head of list                  */
  struct descriptor_data *desc;         /* NULL for mobiles              */
  
  struct char_data *next_in_room;     /* For room->people - list         */
  struct char_data *next;             /* For either monster or ppl-list  */
  struct char_data *next_fighting;    /* For fighting list               */
  
  struct follow_type *followers;        /* List of chars followers       */
  struct char_data *master;             /* Who is char following?        */
  struct reset_com *reset;             /* reset for mobiles */
};


/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3

#define MOON_DARK       1
#define MOON_RISE       2
#define MOON_LIGHT      3
#define MOON_SET        4

#define MOON_NEW         0
#define MOON_QUART_WAX   1
#define MOON_HALF_WAX    2
#define MOON_3QUART_WAX  3
#define MOON_FULL        4
#define MOON_3QUART_WANE 5
#define MOON_HALF_WANE   6
#define MOON_QUART_WANE  7
#define MOON_BLUE        8

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3

struct weather_data {
  int	pressure;	/* How is the pressure ( Mb ) */
  int	change;	/* How fast and what way does it change. */
  int	sky;	/* How is the sky. */
  int	sunlight;	/* And how much sun. */
  int   moonlight;      /* How much moon. */
  int   moon_phase;
};


/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */


struct char_file_u {
  byte sex;
  byte race;
  ubyte  level;
  time_t birth;  /* Time of birth of character     */
  int	played;    /* Number of secs played in total */
  
  short weight;
  short height;
  long body_vector;
  char	title[80];
  char prmpt[240];
  short hometown;
  char	description[240];
  bool talks[MAX_TOUNGE];
  
  
  struct char_ability_data abilities;
  
  struct char_point_data points;
  
  byte skills[MAX_SKILLS];
  
  struct affected_type affected[MAX_AFFECT];
  
  struct char_special2_data specials2;
  
  time_t last_logon;		/* Time (in secs) of last logon */
  char host[HOST_LEN+1];	/* host of last logon */
  
  /* char data */
  char	name[20];
  char	pwd[MAX_PWD_LENGTH+1];
  char poofin[240];
  char poofout[240];   
};



/* ************************************************************************
*  file element for object file. BEWARE: Changing it will ruin rent files *
************************************************************************ */

struct obj_file_elem_0 {
   short item_number;

   char name[80];
   char description[960];
   char short_description[80];
   char action_description[1920];
   byte bag[2];
   int	value[8];
   long	extra_flags;
   int	weight;
   int	timer;
   long	bitvector;
   struct obj_affected_type affected[MAX_OBJ_AFFECT];
};
struct obj_file_elem_1 {
   short item_number;

   char  name[80];
   char  description[960];
   char  short_description[80];
   char  action_description[1920];
   byte  bag[2];
   sbyte light;
   byte  aff_timer;
   byte  no_use_timer;
   int	 value[8];
   long	 extra_flags;
   int	 weight;
   int	 timer;
   long	 bitvector;
   struct obj_affected_type affected[MAX_OBJ_AFFECT];
};
struct obj_file_elem {
  short item_number;
  
  char  name[80];
  char  description[960];
  char  short_description[80];
  char  action_description[1920];
  byte  bag[2];
  sbyte light;
  byte  aff_timer;
  byte  no_use_timer;
  int	 value[8];
  long	 extra_flags;
  int	 weight;
  int	 timer;
  long	 bitvector;
  int	        cost;		/* Value when sold (gp.)            */
  int	        cost_per_day;	/* Cost to keep pr. real day        */
  struct obj_affected_type affected[MAX_OBJ_AFFECT];
};


#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5

/* header block for rent files */
struct rent_info {
   int	time;
   int	rentcode;
   int	net_cost_per_diem;
   int	gold;
   int	account;
   int	nitems;
   int	version;
   int	spare1;
   int	spare2;
   int	spare3;
   int	spare4;
   int	spare5;
   int	spare6;
   int	spare7;
};


/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */



struct txt_block {
   char	*text;
   struct txt_block *next;
};

struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG   0
#define CON_NME     1
#define CON_NMECNF  2
#define CON_PWDNRM  3
#define CON_PWDGET  4
#define CON_PWDCNF  5
#define CON_QSEX    6
#define CON_RMOTD   7
#define CON_SLCT    8
#define CON_EXDSCR  9
#define CON_QRACE   10
#define CON_LDEAD   11
#define CON_PWDNQO  12
#define CON_PWDNEW  13
#define CON_PWDNCNF 14
#define CON_CLOSE   15
#define CON_DELCNF1 16
#define CON_DELCNF2 17
#define CON_QSTAT   18
#define CON_QSTATS   19
#define CON_QSTATI   20
#define CON_QSTATW   21
#define CON_QSTATD   22
#define CON_QSTATC   23
#define CON_QSTATH   24
#define CON_QSTATP   25
#define CON_QSTATG   26
#define CON_MOTD     27
#define CON_QALIGN   28
#define CON_QCOLOR   29
#define CON_QSTATDE  30
#define CON_QSTATF   31
#define CON_QBANCON1 32
#define CON_QBANCON2 33


struct snoop_data {
   struct char_data *snooping;	/* Who is this char snooping		*/
   struct char_data *snoop_by;	/* And who is snooping this char	*/
};

#define MAX_USER_ID 12

struct descriptor_data {
  int	descriptor;		/* file descriptor for socket		*/
  unsigned short int   port;    /* remote port id                       */
  char	*name;			/* ptr to name for mail system		*/
  char	host[50];		/* hostname                             */
  long  addr;
  char  user_id[MAX_USER_ID];
  char	pwd[MAX_PWD_LENGTH+1];	/* password				*/
  byte	bad_pws;		/* number of bad pw attemps this login	*/
  int	pos;			/* position in player-file		*/
  int	connected;		/* mode of 'connectedness'		*/
  int	wait;			/* wait for how many loops		*/
  int	desc_num;		/* unique num assigned to desc		*/
  long	login_time;		/* when the person connected		*/
  char	*showstr_head;		/* for paging through texts		*/
  char	*showstr_point;		/*		-			*/
  char	**str;			/* for the modify-str system		*/
  int	max_str;		/*		-			*/
  int	prompt_mode;		/* control of prompt-printing		*/
  char	buf[MAX_STRING_LENGTH];	/* buffer for raw input			*/
  char	last_input[MAX_INPUT_LENGTH];/* the last input			*/
  char  small_outbuf[SMALL_BUFSIZE]; /* standard output bufer		*/
  char  *output;		/* ptr to the current output buffer	*/
  int   bufptr;			/* ptr to end of current output		*/
  int	bufspace;               /* space left in the output buffer	*/
  int   stat_points;
  bool  color;
  struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
  struct txt_q input;		/* q of unprocessed input		*/
  struct char_data *character;	/* linked to char			*/
  struct char_data *original;	/* original char if switched		*/
  struct snoop_data snoop;	/* to snoop people			*/
  struct descriptor_data *next; /* link to next descriptor		*/
  struct char_data *mob_edit;   /* pointer to mob proto being edited   */
  int medit_mode;
  struct obj_data *obj_edit;    /* pointer to obj proto being edited   */
  int oedit_mode;   
  struct room_data *room_edit;  /* pointer to room being edited        */
  int redit_mode;
  struct shop_data *shop_edit;  /* pointer to shop being edited        */
  int shedit_mode;
  struct zone_data *zone_edit;
  int zedit_mode;
  time_t last_logon;
  int virtual;                  /* virtual number of mob/object  */
  bool cpyextras;
  int  iedsc;
  int ex_i_dir;
  int ia_flag;
  int r_dir;
  int to_room;
  int iaff;
  int level;
  int n_att;
  int isave;
  int iloc;
  char *replace;
  char *with;
  struct list_index_type *list;
};

struct msg_type {
   char	*attacker_msg;  /* message to attacker */
   char	*victim_msg;    /* message to victim   */
   char	*room_msg;      /* message to room     */
};

struct message_type {
   struct msg_type die_msg;      /* messages when death			*/
   struct msg_type miss_msg;     /* messages when miss			*/
   struct msg_type hit_msg;      /* messages when hit			*/
   struct msg_type sanctuary_msg;/* messages when hit on sanctuary	*/
   struct msg_type god_msg;      /* messages when hit on god		*/
   struct message_type *next;	/* to next messages of this kind.	*/
};

struct message_list {
   int	a_type;			/* Attack type				*/
   int	number_of_attacks;	/* How many attack messages to chose from. */
   struct message_type *msg;	/* List of messages.			*/
};

struct dex_skill_type {
   short p_pocket;
   short p_locks;
   short traps;
   short sneak;
   short hide;
};

struct dex_app_type {
   short reaction;
   short miss_att;
   short defensive;
};

struct str_app_type {
   short tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   short todam;    /* Damage Bonus/Penalty                */
   int carry_w;  /* Maximum weight that can be carrried */
   short wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type {
   byte bonus;       /* how many bonus skills a player can */
		     /* practice pr. level                 */
};

struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type {
   short hitp;
   short shock;
};

