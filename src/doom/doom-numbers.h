#pragma once

#include <mgdl.h>
#include "../bunny-sector-types.h"

// Numbers for things that are spawned into the map during game
enum DOOM_SPAWN_NUMBER
{
	spawnnumber_NONE                  = 0,
	spawnnumber_shotguy               = 1,
	spawnnumber_chainguy              = 2,
	spawnnumber_baron                 = 3,
	spawnnumber_zombie                = 4,
	spawnnumber_imp                   = 5,
	spawnnumber_arachnotron           = 6,
	spawnnumber_spidermastermind      = 7,
	spawnnumber_demon                 = 8,
	spawnnumber_spectre               = 9,
	spawnnumber_impfireball           = 10,
	spawnnumber_clip                  = 11,
	spawnnumber_shells                = 12,
	spawnnumber_cacodemon             = 19,
	spawnnumber_revenant              = 20,
	spawnnumber_bridge                = 21,
	spawnnumber_armorbonus            = 22,
	spawnnumber_stimpack              = 23,
	spawnnumber_medkit                = 24,
	spawnnumber_soulsphere            = 25,
	spawnnumber_shotgun               = 27,
	spawnnumber_chaingun              = 28,
	spawnnumber_rocketlauncher        = 29,
	spawnnumber_plasmagun             = 30,
	spawnnumber_bfg                   = 31,
	spawnnumber_chainsaw              = 32,
	spawnnumber_supershotgun          = 33,
	spawnnumber_plasmabolt            = 51,
	spawnnumber_tracer                = 53,
	spawnnumber_greenarmor            = 68,
	spawnnumber_bluearmor             = 69,
	spawnnumber_cell                  = 75,
	spawnnumber_bluekeycard           = 85,
	spawnnumber_redkeycard            = 86,
	spawnnumber_yellowkeycard         = 87,
	spawnnumber_yellowskullkey        = 88,
	spawnnumber_redskullkey           = 89,
	spawnnumber_blueskullkey          = 90,
	spawnnumber_templargeflame        = 98,
	spawnnumber_stealthbaron          = 100 ,
	spawnnumber_stealthknight         = 101 ,
	spawnnumber_stealthzombie         = 102 ,
	spawnnumber_stealthshotguy        = 103 ,
	spawnnumber_lostsoul              = 110 ,
	spawnnumber_vile                  = 111 ,
	spawnnumber_mancubus              = 112 ,
	spawnnumber_hellknight            = 113 ,
	spawnnumber_cyberdemon            = 114 ,
	spawnnumber_painelemental         = 115 ,
	spawnnumber_wolfss                = 116 ,
	spawnnumber_stealtharachnotron    = 117 ,
	spawnnumber_stealthvile           = 118 ,
	spawnnumber_stealthcacodemon      = 119 ,
	spawnnumber_stealthchainguy       = 120 ,
	spawnnumber_stealthsergeant       = 121 ,
	spawnnumber_stealthimp            = 122 ,
	spawnnumber_stealthmancubus       = 123 ,
	spawnnumber_stealthrevenant       = 124 ,
	spawnnumber_barrel                = 125 ,
	spawnnumber_cacodemonshot         = 126 ,
	spawnnumber_rocket                = 127 ,
	spawnnumber_bfgshot               = 128 ,
	spawnnumber_arachnotronplasma     = 129 ,
	spawnnumber_blood                 = 130 ,
	spawnnumber_puff                  = 131 ,
	spawnnumber_megasphere            = 132 ,
	spawnnumber_invulnerability       = 133 ,
	spawnnumber_berserk               = 134 ,
	spawnnumber_invisibility          = 135 ,
	spawnnumber_radiationsuit         = 136 ,
	spawnnumber_computermap           = 137 ,
	spawnnumber_lightamp              = 138 ,
	spawnnumber_ammobox               = 139 ,
	spawnnumber_rocketammo            = 140 ,
	spawnnumber_rocketbox             = 141 ,
	spawnnumber_battery               = 142 ,
	spawnnumber_shellbox              = 143 ,
	spawnnumber_backpack              = 144 ,
	spawnnumber_guts                  = 145 ,
	spawnnumber_bloodpool             = 146 ,
	spawnnumber_bloodpool1            = 147 ,
	spawnnumber_bloodpool2            = 148 ,
	spawnnumber_flamingbarrel         = 149 ,
	spawnnumber_brains                = 150 ,
	spawnnumber_scriptedmarine        = 151 ,
	spawnnumber_healthbonus           = 152 ,
	spawnnumber_mancubusshot          = 153 ,
	spawnnumber_baronball             = 154
};

// Numbers for what things are in the editor
// and what type actors and things are
enum DOOM_EDITOR_NUMBER
{
	editorNumber_none = 0,
	editorNumber_player_start_1,
	editorNumber_player_start_2,
	editorNumber_player_start_3,
	editorNumber_player_start_4,
	editorNumber_blue_card ,
	editorNumber_yellow_card ,
	editorNumber_spider_mastermind , //<< Boss enemy
	editorNumber_backpack ,
	editorNumber_shotgun_guy ,
	editorNumber_gibbed_marine ,
	editorNumber_deathmatch_start ,
	editorNumber_gibbed_marine_extra ,
	editorNumber_red_card ,

	editorNumber_teleport_destination ,

	// Decorations
	editorNumber_dead_marine = 15,
	editorNumber_cyberdemon ,   //<< Enemy
	editorNumber_cell_pack , // Energy pack

	editorNumber_dead_zombie_man ,
	editorNumber_dead_shotgun_guy ,
	editorNumber_dead_doom_imp ,
	editorNumber_dead_demon ,
	editorNumber_dead_cacodemon ,
	editorNumber_dead_lost_soul,

	editorNumber_gibs,
	editorNumber_dead_stick,
	editorNumber_live_stick,
	editorNumber_head_candles,
	editorNumber_tall_green_column,
	editorNumber_short_green_column,
	editorNumber_tall_red_column,
	editorNumber_short_red_column,
	editorNumber_candlestick,
	editorNumber_candelabra,
	editorNumber_heart_column,

	// etc...

	// Skull keys
	editorNumber_red_skull_key = 38,
	editorNumber_yellow_skull_key,
	editorNumber_blue_skull_key,

	// Torches
	editorNumber_blue_torch = 44,
	editorNumber_green_torch,
	editorNumber_red_torch,

	editorNumber_helper_dog = 888,

	// items and weapons

	editorNumber_shotgun = 2001,
	editorNumber_chaingun,
	editorNumber_rocket_launcher,
	editorNumber_plasma_rifle ,
	editorNumber_chainsaw ,
	editorNumber_bfg9000 ,
	editorNumber_clip ,
	editorNumber_shell ,
	editorNumber_rocket_ammo ,
	editorNumber_stimpack ,
	editorNumber_medikit ,
	editorNumber_soulsphere ,
	editorNumber_health_bonus ,
	editorNumber_armor_bonus,
	editorNumber_green_armor,
	editorNumber_blue_armor,
	editorNumber_invulnerability_sphere,
	editorNumber_berserk_item,
	editorNumber_blur_sphere,
	editorNumber_radiation_suit,

	editorNumber_all_map_reveal = 2026,

	// 2027 no value

	editorNumber_column = 2028,
	editorNumber_explosive_barrel,
	editorNumber_infrared_goggles,
	editorNumber_rocket_box,
	editorNumber_cell,
	editorNumber_clip_box,
	editorNumber_shell_box,

	// Enemies
	editorNumber_doom_imp = 3001,
	editorNumber_demon,
	editorNumber_baron_of_hell,
	editorNumber_zombie_man,
	editorNumber_cacodemon,
	editorNumber_lost_soul,

	// player start points 5-8
	editorNumber_player_start_5 = 4001,
	editorNumber_player_start_6,
	editorNumber_player_start_7,
	editorNumber_player_start_8,

	// points that affet actor positions
	editorNumber_point_pusher = 5001,
	editorNumber_point_puller,

	// One more weapon
	editorNumber_pistol = 5010,

	// Coloring the sector
	editorNumber_color_setter = 9038,
	editorNumber_water_zone = 9045, //<< Makes sector underwater
};

#define NAMED_DOOM_TYPE_AMOUNT 6

void InitDoomTypeNames();
void SetDoomTypeCustomName(DOOM_EDITOR_NUMBER typenumber, zstr customName);

const char* DoomTypeToStringChar(DOOM_EDITOR_NUMBER typeNumber);
zstr* DoomTypeToStringZstr(DOOM_EDITOR_NUMBER typeNumber);
DOOM_EDITOR_NUMBER NameToDoomEditorNumberChar(const char* name);

ActorType GetActorTypeOfDoomType(DOOM_EDITOR_NUMBER typeNumber);
