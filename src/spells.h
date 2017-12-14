/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#define SPELL_MULT 20

#define MAX_SPELL_NO                90 /* these are the last numbers usable */
#define MAX_ARM_SPELL               100
#define MAX_SKILL_NO                200 /* for spells and skills */

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_FORTITUDE_BEAR          1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BREATH_OF_INVIGORATION  2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_SWORDSMAN     3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MILKY_EYES              4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CARBUNCLE_OT_EARTH      5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM_PERSON            7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOUR_SPRAY           10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FEAST_FOR_FIVE         12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SOOTHE_PAINS_OT_BEAST  13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WEB                    14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_EVIL            18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVISIBLE       19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_NATURE_OF_VIS    20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGICAL_BALANCE        21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PERCEIVE_MAGIC         22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTH_SHOCK            23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_INVIS           24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVIS1                 27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVIS2                 28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVIS3                 29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT1               30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_RAIN_OF_STONES         32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROTECT_FROM_EVIL      34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MIGHT                  39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
/* NEW SPELLS are to be inserted here */
#define SPELL_IDENTIFY               45
#define SPELL_FLY                    46
#define SPELL_LUNGS_OT_FISH          47
#define SPELL_GRANT_LUNGS_OT_FISH    48
#define SPELL_MEDITATION             49
#define SPELL_HEALTH                 50
#define SPELL_BEAUTY                 51
#define SPELL_NIMBLENESS             52
#define SPELL_ALERTNESS              53
#define SPELL_CRAFTYNESS             54 
#define SPELL_CREATE_LIGHT           55
#define SPELL_RESIST_HEAT            56
#define SPELL_RESIST_COLD            57
#define SPELL_CONE_OF_COLD           58
#define SPELL_CAUSE_LIGHT            59
#define SPELL_CAUSE_CRITIC           60
#define SPELL_CREATE_OBJECT          61

#define SPELL_LAMP_WO_FLAME          91
#define SPELL_PALM_OF_FLAME          92
#define SPELL_FLASH_OF_SCARLET_FLAME 93
#define SPELL_PILUM_OF_FIRE          94
#define SPELL_ARC_OF_FIERY_RIBBONS   95
#define SPELL_BALL_OF_ABYSMAL_FLAME  96
#define SPELL_BREATH_OF_VULCAN       97
#define SPELL_LANCE_OF_SOLAR_FURY    98
#define SPELL_LAST_FLIGHT_OT_POENIX  99
#define SPELL_DOUSE_THE_FIRE         300
#define SPELL_QUENCH_THE_THIRST      301
#define SPELL_FOOTSTEPS_OT_SLIPPERY  302
#define SPELL_DELUGE_OF_RUSHING      320
#define SPELL_MIGHTY_TORRENT         321
#define SPELL_BLADE_OF_SHIVERING_ICE 322
#define SPELL_ENCASE_IN_ICE          323
#define SPELL_LUNGS_OF_WATER_DEATH   324
#define SPELL_POOL_OF_JENNY_GREENTEETH 325
#define SPELL_CALL_OF_THE_WATERY_GRAVE 326

#define SPELL_BIND_WOUNDS            330
#define SPELL_BLACKSMITH_MIGHT       331
#define SPELL_FREE_ACCURSED_BODY     332
#define SPELL_HEALING_TOUCH          333
#define SPELL_RESTORATION            334
#define SPELL_YOUTHFUL_BEAUTY        335
#define SPELL_CAT_NIMBLE             336
#define SPELL_HEARTY_HEALTH          337
#define SPELL_CONJURE_HOMONCULUS     338
#define SPELL_BODY_MADE_WHOLE        339
#define SPELL_CONJURE_GOLEM          340
#define SPELL_SPASMS                 341
#define SPELL_LEAP                   342
#define SPELL_GIFT_OF_VIGOUR         343
#define SPELL_ENDURANCE              344
#define SPELL_WALKING_CORPSE         345
#define SPELL_SEVEN_LEAGUE           346
#define SPELL_FROST_BREATH           401
#define SPELL_FIRE_BREATH            402
#define SPELL_ACID_BREATH            403
#define SPELL_GAS_BREATH             404
#define SPELL_LIGHTNING_BREATH       405
/* types of attacks and skills must NOT use same numbers as spells! */

#define SKILL_SNEAK                  101 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                   102 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                  103 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BACKSTAB               104 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK              105 /* Reserved Skill[] DO NOT CHANGE */

#define SKILL_KICK                   106 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                   107 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                 108
#define SKILL_CLIMB                  109
#define SKILL_QUESTION               110
#define SKILL_BEHEAD                 111
/*#define SKILL_DISARM                 112
#define SKILL_DODGE                  113
#define SKILL_PARRY                  114
*/
#define SKILL_CREO                   112
#define SKILL_INTELLEGO              113
#define SKILL_MUTO                   114
#define SKILL_PERDO                  115
#define SKILL_REGO                   116
#define SKILL_ANIMAL                 117
#define SKILL_AQUAM                  118
#define SKILL_AURAM                  119
#define SKILL_CORPUS                 120
#define SKILL_HERBAM                 121
#define SKILL_IGNEM                  122
#define SKILL_IMAGONEM               123
#define SKILL_MENTEM                 124
#define SKILL_TERRAM                 125
#define SKILL_VIM                    126
#define SKILL_MAGICTHEORY            127
#define SKILL_CAVALRY                128
#define SKILL_MOUNT                  129
#define SKILL_RIDING                 130
#define SKILL_TRACK                  131
#define SKILL_TAME                   132
#define SKILL_FISTICUFFS             133
#define SKILL_DUAL_WIELD             134
#define SKILL_BLESS                  135
#define SKILL_TURN_UNDEAD            136

/* weapon proficiencies go here */

#define PROF_SWORD                   201   /* slashing */
#define PROF_2H_SWORD                202   /* slash 2h */
#define PROF_DAGGER                  203   /* pierce */
#define PROF_CLUB                    204   /* bludgeon */
#define PROF_2H_CLUB                 205   /* bludgeon 2h */
#define PROF_HAMMER                  206   /* smite */
#define PROF_2H_HAMMER               207   /* smite 2h */
#define PROF_AXE                     208   /* cleave */
#define PROF 2H_AXE                  209   /* 2h cleave */
#define PROF_SPEAR                   210   /* stab   */
#define PROF_WHIP                    211   /* whip (not in yet) */
#define PROF_CLAW                    212   /* claw (not in yet) */

#define TYPE_HIT                     300
#define TYPE_BLUDGEON                301
#define TYPE_PIERCE                  302
#define TYPE_SLASH                   303
#define TYPE_SMITE		     304
#define TYPE_CLEAVE                  305
#define TYPE_NO_BS_PIERCE	     306
#define TYPE_CLAW                    307
#define TYPE_BITE                    308
#define TYPE_STING                   309
#define TYPE_CRUSH                   310
#define TYPE_PECK                    311
#define TYPE_BUTT                    312
#define TYPE_KICK                    313
#define TYPE_WHIP                    314

#define TYPE_SUFFERING               400
/* More anything but spells and weapontypes can be insterted here! */





#define MAX_TYPES 70

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define MAX_SPL_LIST	512

#define TAR_IGNORE        (1 << 0)
#define TAR_CHAR_ROOM     (1 << 2)
#define TAR_CHAR_WORLD    (1 << 3)
#define TAR_FIGHT_SELF    (1 << 4)
#define TAR_FIGHT_VICT    (1 << 5)
#define TAR_SELF_ONLY     (1 << 6)       /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO     (1 << 7)       /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV       (1 << 8)
#define TAR_OBJ_ROOM      (1 << 9)
#define TAR_OBJ_WORLD     (1 << 10)
#define TAR_OBJ_EQUIP     (1 << 11)

#define TEACH_HUMAN        (1 << 1)
#define TEACH_ELF          (1 << 2)
#define TEACH_HALF         (1 << 3)
#define TEACH_GIANT        (1 << 4)
#define TEACH_GNOME         (1 << 5)
#define TEACH_HELF         (1 << 6)
#define TEACH_OGIER        (1 << 7)
#define TEACH_DWARF        (1 << 8)
#define TEACH_MERMAN       (1 << 9)
#define TEACH_PIXIE        (1 << 10)
#define TEACH_AMARYA       (1 << 11)
#define TEACH_TROLL        (1 << 12)
#define TEACH_ALL   (TEACH_HUMAN | TEACH_ELF | TEACH_HALF | TEACH_GIANT \
		     | TEACH_GNOME | TEACH_HELF | TEACH_OGIER | TEACH_DWARF \
		     | TEACH_MERMAN | TEACH_PIXIE | TEACH_AMARYA \
		     | TEACH_TROLL)

#define STAT_NNE          0
#define STAT_STR          1
#define STAT_INT          2
#define STAT_WIS          3
#define STAT_CON          3
#define STAT_DEX          5
#define STAT_CHR          6
#define STAT_PER          7
#define STAT_GUI          8
#define STAT_LUC          9
#define STAT_FOC         10
#define STAT_DEV         11

#define RETE_GUILD        0
#define ARSMAGICA_GUILD   1
#define THIEF_GUILD       2
#define WARRIOR_GUILD     3
#define PEIM_GUILD        4
#define CLERIC_GUILD      5
#define RANGER_GUILD      6
#define CRIG_GUILD        7
#define CRAQ_GUILD        8
#define CRCO_GUILD        9
#define RECO_GUILD       10
#define MUCO_GUILD       11
#define RIDING_GUILD     12
#define PECO_GUILD       13
#define MUAQ_GUILD       14
#define CRAN_GUILD       15
#define INIM_GUILD       16
#define INVM_GUILD       17
#define CRIM_GUILD       18
#define MUTE_GUILD       19
#define MUAU_GUILD       20

struct spell_info_type {
   void	(*spell_pointer) (int level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj);
   void	(*spll_pointer) (int spell_no, int level, struct char_data *ch,
       char *arg, int type,struct char_data *tar_ch, struct obj_data *tar_obj);
   byte minimum_position;  /* Position for caster	 */
   ubyte min_usesmana;     /* Amount of mana used by a spell	 */
   byte beats;             /* Heartbeats until ready for next */
   ubyte  min_level;        /* Level required               */
   ubyte  max_level;        /* Max Level guild will teach to  */   
   byte which_guild;      /* GUild where it is taught     */
   byte difficulty;       /* base diffuculty level 0,1,2,3 */
   byte gold;             /* gold cost per practice      */
   int key_stats;      /* key stats affecting skill performance */
   int sec_stats;      /* secondary stats affecting skill performance */
   int ter_stats;      /* tertiary stats affecting skill performance */
   short targets;         /* See below for use with TAR_XXX  */
   bool use_bas_lev;
   byte dam_duration;  /* some spells linger for a certain number of ticks */
    short race_flag;  /* a flag to set race specific spells */ 
};


#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4
#define SPELL_TYPE_ROD     5
#define SPELL_TYPE_PHILTRE 6
#define SPELL_TYPE_CANTRIP 7

/* Attacktypes with grammar */

#define ASPELLN(spellname) \
void	spellname(int spell_no, int level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj)    

#define ACASTN(castname) \
void	castname(int spell_no, int level, struct char_data *ch, \
		 char *arg, int type, \
		 struct char_data *victim, struct obj_data *tar_obj)
ASPELLN(spell_enchant1);
ASPELLN(spell_detect_magic1);
ASPELLN(spell_detect_magic2);
ASPELLN(spell_detect_magic3);
ASPELLN(spell_detect_invis);
ASPELLN(spell_dispel_invis);
ASPELLN(spell_invis1);
ASPELLN(spell_invis2);
ASPELLN(spell_invis3);
ASPELLN(spell_lungs_ot_fish);
ASPELLN(spell_soothe_pains_ot_beast);
ASPELLN(spell_web);
ASPELLN(spell_web);
ASPELLN(spell_feast_for_five);
ASPELLN(spell_breath_of_vigor);
ASPELLN(spell_invok_ot_milky_eyes);
ASPELLN(spell_lightning_swordsman);
ASPELLN(spell_milky_eyes);
ASPELLN(spell_leap);
ASPELLN(spell_seven_league);
ASPELLN(spell_gift_of_vigour);
ASPELLN(spell_endurance);
ASPELLN(spell_walking_corpse);
ASPELLN(spell_spasm);
ASPELLN(spell_restoration);
ASPELLN(spell_gen_dam);
ASPELLN(spell_encase_in_ice);
ASPELLN(spell_quench_thirst);
ASPELLN(spell_footstep_slippery);
ASPELLN(spell_lamp_wo_flame);
ASPELLN(spell_bind_wounds);
ASPELLN(spell_blacksmith_might);
ASPELLN(spell_healing_touch);
ASPELLN(spell_body_made_whole);
ASPELLN(spell_youthful_beauty);
ASPELLN(spell_nimble_cat);
ASPELLN(spell_hearty_health);
ASPELLN(spell_fortitude_ot_bear);
ACASTN(cast_invok_ot_milky_eyes);
ACASTN(cast_breath_of_vigor);
ACASTN(cast_lamp_wo_flame);
ACASTN(cast_gen_dam);
ACASTN(cast_encase_in_ice);
ACASTN(cast_quench_thirst);
ACASTN(cast_footstep_slippery);
ACASTN(cast_bind_wounds);
ACASTN(cast_blacksmith_might);
ACASTN(cast_healing_touch);
ACASTN(cast_body_made_whole);
ACASTN(cast_youthful_beauty);
ACASTN(cast_nimble_cat);
ACASTN(cast_hearty_health);
ACASTN(cast_leap);
ACASTN(cast_gift_of_vigour);
ACASTN(cast_endurance);
ACASTN(cast_walking_corpse);
ACASTN(cast_spasm);
ACASTN(cast_restoration);
ACASTN(cast_seven_league);
ACASTN(cast_fortitude_ot_bear);
ACASTN(cast_lightning_swordsman);
ACASTN(cast_milky_eyes);
ACASTN(cast_lungs_ot_fish);
ACASTN(cast_feast_for_five);
ACASTN(cast_soothe_pains_ot_beast);
ACASTN(cast_web);
ACASTN(cast_invis1);
ACASTN(cast_invis2);
ACASTN(cast_invis3);
ACASTN(cast_detect_invis);
ACASTN(cast_detect_magic1);
ACASTN(cast_detect_magic2);
ACASTN(cast_detect_magic3);
ACASTN(cast_dispel_invis);
ACASTN(cast_enchant1);
