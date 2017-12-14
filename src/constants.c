/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
/* Archipelago changes by Alastair J. Neil Copyright (C) 1993, 94, 95, 96 */

#include "structs.h"
#include "limits.h"
#include "screen.h"

const char	circlemud_version[] = { 
   "CircleMUD, version 2.20\r\n" };
struct dual_list_type attack_hit_text[] = 
{
   { "hit",   "hits" },
   { "pound", "pounds" },
   { "pierce", "pierces" },
   { "slash", "slashes" },
   { "smite", "smites" },
   { "cleave", "cleaves" },
   { "stab", "stabs" },
   { "claw", "claws" },
   { "bite", "bites" },
   { "sting", "stings" },
   { "crush", "crushes" },
   { "peck", "pecks" },
   { "butt", "butts" },
   { "kick", "kicks" },
   { "lash", "lashes"}
};
struct dual_list_type nonreg_plurals[] = 
{
   { "fish",   "fish" },
   { "sheep",   "sheep" },   
   { "mouse",   "mice" },
   { "ox",   "oxen" },
   { "octopus", "octopi"},
   { "goose", "geese"},
   { "fox", "foxes"},
   { "box", "boxes"},   
   {"hippopotamus", "hippopotami"},
   {"squid", "squid"},
   { "\n",   "\n" },
};
struct color_list_type colors[] =
{
    {KNRM,KNRM},
    {KRED,KBRED},
    {KGRN,KBGRN},
    {KYEL,KBYEL},
    {KBLU,KBBLU},
    {KMAG,KBMAG},
    {KCYN,KBCYN},
    {KWHT,KBWHT},
    {"\n","\n"},
};

const char *damage_state[]={
    "..in perfect condition.",
    "..in excellent condition.",
    "..in excellent condition.",
    "..in good condition.",
    "..in good condition.",
    "..in fair condition.",
    "..in fair condition.",       
    "..in fair condition.",
    "..in poor condition.",
    "..in poor condition.",
    "..in poor condition.",       
    "..falling apart.",
    "\n"};
const char *weapon_type[]={
    "hit",
    "pound",
    "pierce",
    "slash",
    "smite",
    "cleave",
    "stab",
    "claw",
    "bite",
    "sting",
    "crush",
    "peck",
    "butt",
    "kick",
    "whip",
    "\n"};

const char       *race_abbrevs[] = {
   "--",
   "Hu",
   "El",
   "Ha",
   "Gi",
   "St",
   "Hf",
   "Or",
   "Df",
   "Sk",
   "Px",
   "Aa",
   "Tl",
};

struct list_index_type npc_races[] = {
  {"Undead",126,"\0"},
  {"Other",1,"\0"},
  {"Humanoid",2,"\0"},
  {"Animal",3,"\0"},
  {"Dragon",4,"\0"},
  {"Giant",5,"\0"},
  {"Plant",6,"\0"},
  {"Equine",7,"\0"},
  {"Ichthyoid",8,"\0"},
  {"Arthopod",9,"\0"},
  {"\n",-1,"\0"}
};

const char	*spell_wear_off_msg[] = {
   "RESERVED DB.C",
   "Your skin feels less protected.",
   "!Teleport!",
   "Your arms feel less limber.",
   "You feel a cloak of blindness disolve.",
   "!Burning Hands!",
   "!Call Lightning",
   "You feel more self-confident.",
   "!Chill Touch!",
   "!Clone!",
   "!Color Spray!",
   "!Control Weather!",
   "!Create Food!",
   "!Create Water!",
   "!Cure Blind!",
   "!Cure Critic!",
   "!Cure Light!",
   "You feel better.",
   "You sense the red in your vision disappear.",
   "The detect invisible wears off.",
   "The detect magic wears off.",
   "The detect poison wears off.",
   "!Dispel Evil!",
   "!Earthquake!",
   "!Enchant Weapon!",
   "!Energy Drain!",
   "!Fireball!",
   "You feel the more obvious.",
   "You feel the more obvious.",
   "You feel yourself exposed.",
   "!Lightning Bolt!",
   "!Locate object!",
   "!Magic Missile!",
   "You feel less sick.",
   "You feel less protected.",
   "!Remove Curse!",
   "The white aura around your body fades.",
   "!Shocking Grasp!",
   "You feel less tired.",
   "You feel weaker.",
   "!Summon!",
   "!Ventriloquate!",
   "!Word of Recall!",
   "!Remove Poison!",
   "You feel less aware of your suroundings.",
   "!UNUSED!",
   "You sink to the ground as gravity reasserts itself.",
   "The air feels thinner.",
   "You don't feel quite so smart.",
   "You don't feel quite so wise.",
   "You don't feel quite so hale.",
   "You don't feel quite so pretty.",
   "You feel a bit more clumsy.",
   "Your senses are dulled.",
   "You feel more gullable.",
   "!CREATE_LIGHT",
   "You don't feel quite so cool anymore.",
   "You don't feel quite so warm anymore.",
   "!CONEOFCOLD!",
   "\n"
};
const char *object_sizes[] = {
    "Normal",
    "Huge",
    "Large",
    "Normal",
    "Small",
    "Very Small",
    "Tiny"};

const char *skill_wear_off_mess[] = {
    "",  /* NO MESSAGE FOR SNEAK*/
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",
    "!Rescue!",
    "\n"
};

const int	rev_dir[] = 
{
   2,
   3,
   0,
   1, 	
   5,
   4
};


const int	movement_loss[] = 
{
   0,  /* Inside     */
   1,  /* City       */
   2,  /* Field      */
   3,  /* Forest     */
   4,  /* Hills      */
   5,  /* Mountains  */
   7,  /* Swimming   */
   8,  /* Unswimable */
   7,  /* flying */
   4,  /* under water */
   4,  /* briar */
   4,  /* fire */
   4,  /* ice */
   3,  /* desert hot */
   3,  /* desert cold */
};


const char	*dirs[] = 
{
   "north",
   "east",
   "south",
   "west",
   "up",
   "down",
   "\n"
};
const char	*revdirs[] = 
{
   "the south",
   "the west",
   "the north",
   "the east",
   "below",
   "above",
   "\n"
};

const char	*weekdays[7] = {
   "the Day of the Moon",
   "the Day of the Bull",
   "the Day of the Deception",
   "the Day of Thunder",
   "the Day of Freedom",
   "the day of Small Gods",
   "the Day of the Sun" };


const char	*month_name[17] = {
   "Month of Winter",           /* 0 */
   "Month of the Winter Wolf",
   "Month of the Frost Giant",
   "Month of the Old Forces",
   "Month of the Grand Struggle",
   "Month of the Spring",
   "Month of Nature",
   "Month of Futility",
   "Month of the Dragon",
   "Month of the Sun",
   "Month of the Heat",
   "Month of the Battle",
   "Month of the Dark Shades",
   "Month of the Shadows",
   "Month of the Long Shadows",
   "Month of the Ancient Darkness",
   "Month of the Great Evil"
};

const int hyper_soc[]={9,22,24,26,27,28,29,49
		       ,50,51,52,94,97,98,
		       107,110
		       ,112,113,117,118
		       ,120,123,126,127,129,
		       131,134,138,140,
		       143,144,145,146,159,161,
		       162,163,165,181,185,
		       193,198,260,261,262,
		       263,264,331,332,333,343,
		       351,353,354,355,356,
		       360,364,365,366,375,
		       382,387,394,397,400,410,
		       405,415,417,418,419,-1};
const int happy_soc[]={9,23,27,28,29,49,94,97
		       ,98,107
		       ,110,113,117,
		       126,129,131,
		       138,143,144,145,146,
		       161,162,163,165,181,184,
		       185,193,198,260,262,
		       264,330,331,332,333,343,
		       351,353,354,355,356,358,
		       360,364,365,366,375,
		       382,387,394,397,400,
		       410,415,417,418,419, -1};
const int cheery_soc[]={23,29,94,97,98
			,107,108,110,
			126,128,129,131,
			141,142,143,144,146,161,
			162,163,165,181,185,
			195,263,328,329,330,
			331,332,333,334,343,
			351,353,354,355,356,358,
			360,364,365,366,375,
			382,387,394,397,400,
			-1};
const int neutral_soc[]={94,98,
			 110,113,114,
			 124,128,132,139
			 ,141,142,143,161,176,
			 181,186,187,195,328,329,
			 330,332,333,334,420,416,400,348,
			 392,382,376,359,358,357,356,355-1};
const int sad_soc[]={35,36,53,94,96,98
		     ,113,114,115,116,
		     118,124,128,130,132
		     ,139,142,176,180,348,350,357,367,371,372,
		     186,187,188,191,195,196,328,373,374,376,377,
		     378,379,380,381,383,384,388,392,393,395,396,
		     406,416,329,332,333,334,335,342,-1};
const int grouchy_soc[]={31,33,35,36,53,96
			 ,113,114,115,
			 116,118,122,124,128,
			 130,132,137,139,361,
			 171,176,180,182,183,188,
			 189,190,191,192,194,196,
			 327,332,333,335,342,406,416,
			 378,379,380,381,383,384,388,
			 392,393,395,396,-1};
const int homicidal_soc[]={31,32,33,53,
			   113,114,
			   115,116,118,122,130,
			   132,137,139,171,
			   176,180,182,183,
			   190,192,194,196,361,
			   327,332,333,342,399,391,390,406,416,
			   378,379,380,381,383,384,388,
			   392,393,395,396,-1};
const int opp_sex_soc[]={9,24,50,51,52,
			 105,113,118,120,
			 121,123,125,138,
			 144,145,260,261,
			 364,366,375,394,410,419,
			 262,263,-1};
		       

const int	sharp[] = {
   0,
   0,
   0,
   1,    /* Slashing */
   0,
   0,
   0,
   0,    /* Bludgeon */
   0,
   0,
   0,
   0 };  /* Pierce   */


const char	*where[] = {
   "<worn as scabbard>",
   "<worn on finger>  ",
   "<worn on finger>  ",
   "<worn about neck> ",
   "<worn about neck> ",
   "<worn on body>    ",
   "<worn on head>    ",
   "<worn on legs>    ",
   "<worn on feet>    ",
   "<worn on hands>   ",
   "<worn on arms>    ",
   "<worn as shield>  ",
   "<worn about body> ",
   "<worn about waist>",
   "<worn about wrist>",
   "<worn about wrist>",
   "<primary hand>    ",
   "<secondary hand>  ",
   "<worn over eyes>  ",
   "<worn on ear>     ",   
   "<worn on ear>     ",
   "<worn in mouth>   ",
   "<worn on back>    ",
   "<worn about waist>"
};
const char	*where_equine[] = {
   "<worn as scabbard>",
   "<worn on hoof>    ",
   "<worn on hoof>    ",
   "<worn about neck> ",
   "<worn about neck> ",
   "<worn on body>    ",
   "<worn on head>    ",
   "<worn on hindlegs>",
   "<worn on hoof>    ",
   "<worn on hoof>    ",
   "<worn on forelegs>",
   "<worn as shield>  ",
   "<worn about body> ",
   "<worn as girth>   ",
   "<worn on fetlock> ",
   "<worn on fetlock> ",
   "<primary hoof>    ",
   "<secondary hoof>  ",
   "<worn over eyes>  ",
   "<worn on ear>     ",   
   "<worn on ear>     ",
   "<worn in mouth>   ",
   "<worn on back>    "
};
const char	*where_animal[] = {
   "<worn as scabbard>",
   "<worn on toes>    ",
   "<worn on toes>    ",
   "<worn about neck> ",
   "<worn about neck> ",
   "<worn on body>    ",
   "<worn on head>    ",
   "<worn on hindlegs>",
   "<worn on backpaws>",
   "<worn on forepaws>",
   "<worn on forelegs>",
   "<worn as shield>  ",
   "<worn about body> ",
   "<worn about waist>",
   "<worn about wrist>",
   "<worn about wrist>",
   "<primary paw>     ",
   "<secondary paw>   ",
   "<worn over eyes>  ",
   "<worn on ear>     ",   
   "<worn on ear>     ",
   "<worn in mouth>   ",
   "<worn on back>    "
};
const char	*where_plant[] = {
   "<worn as scabbard> ",
   "<worn on tendril>  ",
   "<worn on tendril>  ",
   "<worn about stem>  ",
   "<worn about stem>  ",
   "<worn on stalk>    ",
   "<worn on flower>   ",
   "<worn about roots> ",
   "<worn on roots>    ",
   "<worn on leafs>    ",
   "<worn on stalks>   ",
   "<worn as shield>   ",
   "<worn about stalk> ",
   "<worn about waist> ",
   "<worn about tendril>",
   "<worn about tendril>",
   "<primary leaf>      ",
   "<secondary leaf>    ",
   "<worn over petal>   ",
   "<worn on sepal>     ",   
   "<worn on sepal>     ",
   "<worn on stamen>    ",
   "<worn on over stalk>"
};

const char	*sizes[] = 
{
   "Normal",
   "Huge",
   "Large",
   "Normal",
   "Small",
   "Very Small",
   "Tiny",
   "\n",
};

const char	*drinks[] = 
{
   "water",
   "beer",
   "wine",
   "ale",
   "dark ale",
   "whisky",
   "lemonade",
   "firebreather",
   "local speciality",
   "slime mold juice",
   "milk",
   "tea",
   "coffee",
   "blood",
   "salt water",
   "clear water",
   "\n"
};


const char	*drinknames[] = 
{
   "water",
   "beer",
   "wine",
   "ale",
   "ale",
   "whisky",
   "lemonade",
   "firebreather",
   "local",
   "juice",
   "milk",
   "tea",
   "coffee",
   "blood",
   "salt",
   "water"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int	drink_aff[][3] = {
   { 0, 0, 10 },
   { 3, 2, 5 },
   { 5, 2, 5 },
   { 2, 2, 5 },
   { 1, 2, 5 },
   { 6, 1, 4 },
   { 0, 1, 8 },
   { 10, 0, 0 },
   { 3, 3, 3 },
   { 0, 4, -8 },
   { 0, 3, 6 },
   { 0, 1, 6 },
   { 0, 1, 6 },
   { 0, 2, -1 },
   { 0, 1, -2 },
   { 0, 13, 13 }
};



const char	*genders[] =
{
   "neuter",
   "male",
   "female"
};



const char	*color_liquid[] = 
{
   "clear",
   "brown",
   "clear",
   "brown",
   "dark",
   "golden",
   "red",
   "green",
   "clear",
   "light green",
   "white",
   "brown",
   "black",
   "red",
   "clear",
   "crystal clear"
};


const char	*fullness[] = 
{
   "less than half ",
   "about half ",
   "more than half ",
   ""
};


const char	*item_types[] = {
   "UNDEFINED",
   "LIGHT",
   "SCROLL",
   "WAND",
   "STAFF",
   "WEAPON",
   "FIRE WEAPON *",
   "MISSILE *",
   "TREASURE",
   "ARMOR",
   "POTION",
   "WORN",
   "OTHER",
   "TRASH",
   "TRAP *",
   "CONTAINER",
   "NOTE",
   "LIQUID CONTAINER",
   "KEY",
   "FOOD",
   "MONEY",
   "PEN",
   "BOAT",
   "FOUNTAIN",
   "ROD",
   "CANTRIP",
   "PHILTRE",
   "ELIXIR",
   "STREET LIGHT",
   "SCABBARD",
   "BELT",
   "VIS",
   "\n"
};


const char	*wear_bits[] = {
   "TAKE",
   "FINGER",
   "NECK",
   "BODY",
   "HEAD",
   "LEGS",
   "FEET",
   "HANDS",
   "ARMS",
   "SHIELD",
   "ABOUT",
   "WAIST",
   "WRIST",
   "WIELD",
   "HOLD",
   "THROW",
   "EYES",
   "EARS",
   "MOUTH",
   "BACK",
   "SCABBARD",
   "\n"
};


const char	*extra_bits[] = {
   "GLOW",
   "HUM",
   "DARK",
   "LOCK",
   "EVIL",
   "INVISIBLE",
   "MAGIC",
   "NODROP",
   "BLESS",
   "!GOOD",
   "!EVIL",
   "!NEUTRAL",
   "!RENT",
   "!DONATE",
   "!INVIS",
   "2-HANDED",
   "EQUIPED *",
   "FRAGILE",
   "RE-SIZED",
   "FERROUS",
   "GOD-ONLY",
   "PULLABLE",
   "TEST-ONLY",
   "\n"
};


const char	*room_bits[] = {
   "DARK",
   "DEATH",
   "NO_MOB",
   "NO_WEATHER",
   "SPIN",
   "UNFINISHED",
   "PAIN",
   "NO_MAGIC",
   "NO_AUTOFORMAT",
   "PRIVATE",
   "GODROOM",
   "*",
   "PEACEFUL",
   "FAST_HP",
   "RECALL",
   "FAST_MOVE",
   "HOT",
   "COLD",
   "WARM",
   "COOL",
   "NO_SCAN",
   "SMALL",
   "TINY",
   "CLIMB EASY",
   "CLIMB MODERATE",   
   "CLIMB HARD",
   "CLIMB SEVERE",
   "CLIMB EXTREME",
   "NO EXITS",
   "MOB DEATH",
   "RECALL DISTORT",
   "\n"
};


const char	*exit_bits[] = {
   "DOOR",
   "CLOSED",
   "LOCKED",
   "SCT !LK",
   "SCT LK",   
   "!PICK",
   "SCT",
   "DARK",
   "\n"
};


const char	*sector_types[] = {
   "Inside",
   "City",
   "Field",
   "Forest",
   "Hills",
   "Mountains",
   "Water Swim",
   "Water NoSwim",
   "Air Fly",
   "Under Water",
   "Briar",
   "Fire",
   "Ice",
   "Desert Hot",
   "Desert Cold",
   "\n"
};


const char	*equipment_types[] = {
   "Special",
   "Worn on right finger",
   "Worn on left finger",
   "First worn around Neck",
   "Second worn around Neck",
   "Worn on body",
   "Worn on head",
   "Worn on legs",
   "Worn on feet",
   "Worn on hands",
   "Worn on arms",
   "Worn as shield",
   "Worn about body",
   "Worn around waist",
   "Worn around right wrist",
   "Worn around left wrist",
   "Wielded",
   "Held",
   "Worn over eyes",
   "Worn on right ear",
   "Worn on left  ear",
   "Worn in mouth",
   "Worn on back",         
   "\n"
};


const char     *container_bits[] =
{
    "CLOSEABLE",
    "PICKPROOF",
    "CLOSED",
    "LOCKED",
    "SEETHRU",
    "ONEWAY",
    "\n",
};

const char     *door_bits[] =
{
    "IS A DOOR",
    "CLOSED",
    "LOCKED",
    "UNCLOSABLE *",
    "UNLOCKABLE *",
    "PICKPROOF",
    "SECRET",    
    "DARK",    
    "\n",
};

const char	*affected_bits[] = 
{
   "BLIND",
   "INVIS",
   "DET-EVIL",
   "DET-INVIS",
   "DET-MAGIC",
   "SENSE-LIFE",
   "SPELL_BOTCH",
   "SANCT",
   "GROUP *",
   "CURSE",
   "FL-HANDS *",
   "POISON *",
   "PROT-EVIL",
   "PARAL",
   "RESIST-HEAT",
   "RESIST-COLD",
   "SLEEP",
   "DODGE *",
   "SNEAK",
   "HIDE",
   "FEAR *",
   "CHARM *",
   "FOLLOW *",
   "WIMPY",
   "INFRA",
   "FLY",
   "WATER BREATH",
   "FREE ACTION",
   "BASH *",
   "SLIPPY",
   "HEALING",
   "\n"
};


const char	*apply_types[] = {
   "NONE",
   "STR",
   "DEX",
   "INT",
   "WIS",
   "CON",
   "SEX",
   "CLASS",
   "LEVEL",
   "AGE",
   "CHAR_WEIGHT",
   "CHAR_HEIGHT",
   "MANA",
   "HIT",
   "MOVE",
   "GOLD",
   "EXP",
   "BODY AC",
   "HITROLL",
   "DAMROLL",
   "SAVING_PARA",
   "SAVING_ROD",
   "SAVING_PETRI",
   "SAVING_BREATH",
   "SAVING_SPELL",
   "CHR",
   "PER",
   "GUI",
   "LUC",
   "LEGS AC",
   "ARMS AC",
   "HEAD AC",
   "BODY STOPPING",
   "LEGS STOPPING",
   "ARMS STOPPING",
   "HEAD STOPPING",
   "ALL STOPPING",
   "POWER",
   "FOCUS",
   "DEVOTION",
   "ALL AC",
   "ALL SAVES",
   "HITROLL 2",
   "DAMROLL 2",
   "\n"
};

const char	*pc_race_types[] = {
   "UNDEFINED",
   "Human",
   "Elf",
   "Halfling",
   "Giant",
   "Gnome",
   "Half Elf",
   "Ogier",
   "Dwarf",
   "Selkie",
   "Pixie",
   "Amarya",
   "Troll",
   "\n"
};

const struct pc_stat_parameters  pc_str[]={
  {6,22},
  {6,22},  
  {4,18}, 
  {4,20},
  {15,26},  
  {4,20},
  {5,21},
  {9,24},
  {9,24},
  {6,22},  
  {2,12},
  {6,22},
  {9,24}  
};
const struct pc_stat_parameters  pc_int[]={
  {6,22},
  {6,22},  
  {7,23}, 
  {4,15},
  {3,12},  
  {4,19},
  {6,22},
  {9,24},
  {3,14},
  {6,22},  
  {12,24},
  {2,14},
  {1,10}  
};
const struct pc_stat_parameters  pc_wis[]={
  {6,22},
  {6,22},  
  {7,20}, 
  {5,20},
  {3,15},  
  {9,24},
  {6,22},
  {5,23},
  {3,14},
  {6,22},  
  {12,24},
  {3,15},
  {3,15}  
};
const struct pc_stat_parameters  pc_foc[]={
  {6,22},
  {6,22},  
  {5,20}, 
  {4,18},
  {3,15},  
  {5,23},
  {6,22},
  {9,24},
  {3,14},
  {6,22},  
  {12,24},
  {6,22},
  {3,14}  
};
const struct pc_stat_parameters  pc_per[]={
  {6,22},
  {6,22},  
  {7,23}, 
  {8,24},
  {3,12},  
  {5,23},
  {6,23},
  {9,24},
  {3,14},
  {6,22},  
  {12,24},
  {6,22},
  {1,10}  
};
const struct pc_stat_parameters  pc_chr[]={
  {6,22},
  {6,22},  
  {7,23}, 
  {6,22},
  {3,15},  
  {3,15},
  {6,23},
  {4,15},
  {4,18},
  {4,17},  
  {12,24},
  {6,22},
  {1,10}  
};
const struct pc_stat_parameters  pc_dex[]={
  {6,22},
  {6,22},  
  {7,23}, 
  {8,24},
  {1,12},  
  {3,15},
  {6,23},
  {4,15},
  {6,22},
  {3,17},  
  {15,26},
  {9,24},
  {1,12}  
};
const struct pc_stat_parameters  pc_con[]={
  {6,22},
  {6,22},  
  {4,17}, 
  {4,20},
  {12,25},  
  {5,18},
  {4,18},
  {4,18},
  {10,23},
  {4,19},  
  {2,14},
  {6,20},
  {9,24}  
};
const struct pc_stat_parameters  pc_gui[]={
  {6,22},
  {6,22},  
  {2,15}, 
  {10,25},
  {3,15},  
  {3,15},
  {4,20},
  {3,15},
  {6,22},
  {6,22},  
  {3,15},
  {8,24},
  {1,10}  
};

const char	*npc_class_types[] = {
   "Normal",
   "Undead",
   "\n"
};

const char	*action_bits[] = {
   "SPEC",
   "SENTINEL",
   "SCAVENGER",
   "ISNPC",
   "NICE_THIEF",
   "AGGR",
   "STAY_ZONE",
   "WIMPY",
   "AGGRESSIVE_EVIL",
   "AGGRESSIVE_GOOD",
   "AGGRESSIVE_NEUTRAL",
   "MEMORY",
   "HELPER",
   "CITIZEN",
   "CAN_SPEAK",
   "WILL_LOOT",
   "HAS MOOD",
   "HAPPY",
   "SAD",
   "STAY_SECTOR",
   "DOCILE",
   "TETHERED",
   "SPELL_CASTER",
   "POISONOUS",
   "\n"
};


const char	*player_bits[] = {
   "KILLER",
   "THIEF",
   "FROZEN",
   "DONTSET",
   "WRITING",
   "MAILING",
   "CSH",
   "SITEOK",
   "NOSHOUT",
   "NOTITLE",
   "DELETED",
   "LOADRM",
   "!WIZL",
   "!DEL",
   "INVST",
   "CRYO",
   "BUILDING",
   "AFK",
   "",
   "\n"
};


const char	*preference_bits[] = {
   "BRIEF",
   "COMPACT",
   "DEAF",
   "!TELL",
   "D_HP",
   "D_MANA",
   "D_MOVE",
   "D_AUTO",
   "!HASS",
   "QUEST",
   "SUMN",
   "!REP",
   "LIGHT",
   "C1",
   "C2",
   "!WIZ",
   "L1",
   "L2",
   "!AUC",
   "!GOS",
   "!GTZ",
   "RMFLG",
   "!BRAG",
   "!CONDITIONS",
   "DEBUG",
   "\n"
};


const char	*position_types[] = {
   "Dead",
   "Mortally wounded",
   "Incapacitated",
   "Stunned",
   "Sleeping",
   "Resting",
   "Sitting",
   "Standing",
   "\n"
};


const char	*connected_types[] = {
   "Playing",
   "Get name",
   "Confirm name",
   "Get password",
   "Get new PW",
   "Confirm new PW",
   "Select sex",
   "Read MOTD",
   "Main Menu",
   "Get descript.",
   "Select Race",
   "Linkless",
   "Changing PW 1",
   "Changing PW 2",
   "Changing PW 3",
   "Disconnecting",
   "Self-Delete 1",
   "Self-Delete 2",
   "Display Stats",
   "Change Str",
   "Change Int",
   "Change Wis",
   "Change Dex",
   "Change Con",
   "Change Chr",
   "Change Per",
   "Change Gui",
   "Page Motd",
   "Select Align",
   "Color Select",
   "Change Dev",
   "Change Foc",
   "\n"
};


const char	*ban_types[] = {
   "no",
   "new",
   "select",
   "all",
   "self",
   "ERROR"
};



/* [ch] strength apply (all) */
const struct str_app_type str_app[35] = {
   { -5, -4,  0,  0 },
   { -5, -4,120,  10 },
   { -3, -2,120,  20 },
   { -3, -1,250,  30 },
   { -2, -1,650,  40 },
   { -2, -1,950,  50 },
   { -1, 0,1200,  60 },
   { -1, 0,1400,  70 },
   { 0, 0, 1600,  80 },
   { 0, 0, 2000,  90 },
   { 0, 0, 2100, 100 },
   { 0, 0, 2200, 110 },
   { 0, 0, 2300, 120 },
   { 0, 0, 2400, 130 },
   { 0, 0, 2600, 140 },
   { 0, 0, 2800, 150 },
   { 0, 1, 3000, 160 },
   { 1, 1, 3300, 180 },
   { 1, 2, 3600, 220 },
   { 2, 3, 3900, 260 },
   { 2, 3, 4100, 300 },
   { 2, 3, 4400, 340 },
   { 3, 4, 4700, 380 },
   { 3, 4, 5000, 400 },
   { 3, 4, 5400, 430 },
   { 4, 5, 5500, 460 },
   { 4, 5, 5600, 490 },
   { 4, 5, 5700, 550 },
   { 5, 6, 5800, 590 },
   { 5, 6, 5900, 610 },
   { 6, 7, 6000, 640 },
   { 6, 7, 6100, 680 },
   { 7, 8, 6200, 700 },
   { 8, 9, 6300, 720 },
   { 9,10, 6400, 740 }  /* 18/100   (34) */
};



/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[36] = {
   { -9, -9, -9, -9, -6 },
   { -9, -9, -6, -9, -5 },
   { -8, -8, -4, -8, -4 },
   { -7, -7, -3, -7, -4 },
   { -6, -6, -3, -6, -3 },
   { -5, -5, -2, -5, -3 },
   { -4, -4, -2, -4, -2 },
   { -3, -3, -1, -3, -2 },
   { -2, -2, -1, -2, -1 },
   { -1, -1, -1, -2, -1 },
   { -1, 0, 0, -1, 0 },
   { 0,  0, 0, -1,  0 },
   { 0,  0,  0, 0,  0 },
   { 0,  0,  0,  0,  0 },
   { 0,  0,  0,  0,  0 },
   { 0,  0,  0,  0,  0 },
   { 0,  0,  0,  0,  0 },
   { 0,  1,  0,  0,  0 },
   { 1, 1, 1, 1, 1 },
   { 1, 2, 1, 1, 1 },
   { 1, 2, 1, 1, 1 },
   { 2, 2, 1, 1, 2 },
   { 2, 2, 1, 2, 2 },
   { 2, 2, 1, 2, 2 },
   { 2, 3, 1, 2, 2 },
   { 2, 3, 1, 2, 2 },
   { 3, 3, 2, 2, 2 },
   { 3, 3, 2, 2, 2 },
   { 3, 3, 2, 3, 3 },
   { 3, 3, 2, 3, 3 },
   { 3, 3, 2, 3, 3 },
   { 3, 3, 2, 3, 3 },
   { 3, 3, 2, 3, 3 },
   { 3, 4, 2, 3, 3 },
   { 3, 4, 3, 3, 3 },
   { 4, 4, 3, 3, 3 }    /* 35 */
};




/* [dex] apply (all) */
struct dex_app_type dex_app[36] = {
   { -7, -7, 6 },
   { -6, -6, 5 },
   { -4, -4, 5 },
   { -3, -3, 4 },
   { -2, -2, 3 },
   { -1, -1, 2 },
   { 0, 0, 1 },
   { 0, 0, 0 },
   { 0, 0, 0 },
   { 0, 0, 0 },
   { 0, 0, 0 },
   { 0, 0, 0 },
   { 0, 0, 0 },
   { 0, 0, 0 },
   { 0, 0, 0 },
   { 0, 0, -1 },
   { 1, 1, -2 },
   { 2, 2, -3 },
   { 2, 2, -4 },
   { 3, 3, -4 },
   { 3, 3, -4 },
   { 4, 4, -5 },
   { 4, 4, -5 },
   { 4, 4, -5 },
   { 5, 5, -6 },
   { 5, 5, -6 },
   { 5, 5, -6 },
   { 5, 5, -6 },
   { 6, 6, -7 },
   { 6, 6, -7 },
   { 6, 6, -7 },
   { 6, 6, -7 },
   { 7, 7, -8 },
   { 7, 7, -8 },
   { 7, 7, -8 },
   { 7, 7, -6 }    /* 35 */
};



/* [con] apply (all) */
struct con_app_type con_app[36] = {
   { -2, 20 },
   { -2, 25 },
   { -1, 30 },
   { 0, 35 },
   { 0, 40 },
   { 0, 45 },
   { 0, 50 },
   { 0, 55 },
   { 0, 60 },
   { 0, 65 },
   { 0, 70 },
   { 0, 75 },
   { 1, 80 },
   { 1, 85 },
   { 2, 88 },
   { 2, 90 },
   { 3, 95 },
   { 4, 97 },
   { 4, 99 },
   { 5, 99 },
   { 5, 99 },
   { 5, 99 },
   { 6, 99 },
   { 6, 99 },
   { 6, 99 },
   { 7, 99 },
   { 7, 100 },
   { 7, 100 },
   { 8, 100 },
   { 8, 100 },
   { 9, 100 },
   { 9, 100 },
   { 9, 100 },
   { 10, 100 },
   { 10, 100 },
   { 10, 100 }   /* 35 */
};



/* [int] apply (all) */
struct int_app_type int_app[36] = {
 {  3 },
 {  5 },    /* 1 */
 {  7 },
 {  8 },
 {  9 },
 { 10 },    /* 5 */
 { 11 },
 { 12 },
 { 13 },
 { 15 },
 { 17 },    /* 10 */
 { 19 },
 { 22 },
 { 25 },
 { 30 },
 { 35 },    /* 15 */
 { 40 },
 { 45 },
 { 50 },
 { 53 },
 { 55 },    /* 20 */
 { 56 },
 { 57 },
 { 58 },
 { 59 },
 { 60 },    /* 25 */
 { 61 },
 { 62 },
 { 63 },
 { 64 },
 { 65 },    /* 30 */
 { 66 },
 { 69 },
 { 70 },
 { 80 },
 { 99 }     /* 35 */
};


/* [wis] apply (all) */
struct wis_app_type wis_app[36] = {
 {  0 },   /* 0 */
 {  0 },   /* 1 */
 {  0 },
 {  0 },
 {  0 },
 {  0 },   /* 5 */
 {  0 },
 {  0 },
 {  0 },
 {  0 },
 {  0 },   /* 10 */
 {  0 },
 {  2 },
 {  2 },
 {  3 },
 {  3 },   /* 15 */
 {  3 },
 {  4 },
 {  5 },   /* 18 */
 {  6 },
 {  6 },   /* 20 */
 {  6 },
 {  6 },
 {  7 },
 {  7 },
 {  7 },   /* 25 */
 {  7 },
 {  8 },
 {  8 },
 {  8 },
 {  8 },   /* 30 */
 {  8 },
 {  8 },
 {  8 },
 {  9 },
 {  9 }    /* 35 */
};





