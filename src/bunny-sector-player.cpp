#include "bunny-sector-players.h"
#include "bunny-sector-config.h"
#include "gameplay/player.h"
#include "tinyxml2/tinyxml2.h"

static Player prefabs[BUNNY_SECTOR_MAX_PLAYERS + 1]; // 0 is the default for all

PlayerManager::PlayerManager()
{
	prefabs[0].number = -1;
	prefabs[1].number = -1;
	prefabs[2].number = -1;
	prefabs[3].number = -1;
	prefabs[4].number = -1;
}


bool PlayerManager::ReadXML(const char* filename)
{
	if (mgdl_DoesFileExist(filename) == false)
	{
		Log_ErrorF("No file for player configuration. Tried %s\n", filename);
		return false;
	}
	tinyxml2::XMLDocument players;
	tinyxml2::XMLError loadresult = players.LoadFile(filename);
	if (loadresult != tinyxml2::XML_SUCCESS)
	{
		Log_Error("Failed to load xml file\n");
	}

	tinyxml2::XMLElement* inventoryElement = nullptr; // Must be here because goto

	tinyxml2::XMLElement* playerElement = players.FirstChildElement()->FirstChildElement("player");
	if (playerElement == nullptr)
	{
		Log_Error("Did not find any <player> from xml\n");
		return false;
	}
	while(playerElement)
	{
		int playerNumber = -1;
		Player* prefab = nullptr;
		if (XML_ReadChildInt(playerElement, "number", &playerNumber))
		{
			if (playerNumber >= 0 && playerNumber <= BUNNY_SECTOR_MAX_PLAYERS)
			{
				prefab = &prefabs[playerNumber];
				prefab->number = playerNumber;
			}
			else
			{
				Log_ErrorF("Invalid player index %d\n", playerNumber);
				goto player_end;
			}
		}
		// Default values
		prefab->turnSpeedMultiplier = 1.0f;
		prefab->walkSpeedMultiplier = 1.0f;
		prefab->headBobAmplitude = 0.0f;
		prefab->headBobFreq = 0.0f;

		if (XML_ReadChildInt(playerElement, "height", &prefab->standingHeight) == false)
		{
			Log_Error("No player height in XML\n");
		}
		if (XML_ReadChildInt(playerElement, "size", &prefab->size) == false)
		{
			Log_Error("No player size in XML\n");
		}
		// MOVE
		if (XML_ReadChildInt(playerElement, "movespeed", &prefab->moveSpeed) == false)
		{
			Log_Error("No player movespeed in XML\n");
		}
		if (XML_ReadChildInt(playerElement, "moveacceleration", &prefab->moveAcceleration) == false)
		{
			prefab->moveAcceleration = prefab->moveSpeed;
		}
		if (XML_ReadChildInt(playerElement, "moveslowdown", &prefab->moveSlowdown) == false)
		{
			prefab->moveSlowdown = prefab->moveSpeed;
		}
		// STRAFE
		if (XML_ReadChildInt(playerElement, "strafespeed", &prefab->strafeSpeed) == false)
		{
			prefab->strafeSpeed = prefab->moveSpeed;
		}
		if (XML_ReadChildInt(playerElement, "strafeacceleration", &prefab->strafeAcceleration) == false)
		{
			prefab->strafeAcceleration = prefab->strafeSpeed;
		}
		if (XML_ReadChildInt(playerElement, "strafeslowdown", &prefab->strafeSlowdown) == false)
		{
			prefab->strafeSlowdown = prefab->strafeSpeed;
		}
		// TURN
		if (XML_ReadChildInt(playerElement, "turnspeed", &prefab->turnSpeedDegrees) == false)
		{
			Log_Error("No player turnspeed in XML\n");
		}
		if (XML_ReadChildInt(playerElement, "turnacceleration", &prefab->turnAccelerationDegrees) == false)
		{
			prefab->turnAccelerationDegrees = prefab->turnSpeedDegrees;
		}
		if (XML_ReadChildInt(playerElement, "turnslowdown", &prefab->turnSlowdownDegrees) == false)
		{
			prefab->turnSlowdownDegrees = prefab->turnSpeedDegrees;
		}
		// JUMP
		if (XML_ReadChildInt(playerElement, "jumpspeed", &prefab->jumpSpeed) == false)
		{
			prefab->jumpSpeed = 0; // Cannot jump
		}
		if (XML_ReadChildInt(playerElement, "jumpacceleration", &prefab->jumpAcceleration) == false)
		{
			prefab->jumpAcceleration = 0; // Cannot jump
		}
		int jumpDurationMS;
		if (XML_ReadChildInt(playerElement, "jumpduration", &jumpDurationMS))
		{
			prefab->jumpDuration = (float)jumpDurationMS / 1000.0f;
		}
		else
		{
			prefab->jumpDuration = 0; // Cannot jump
		}
		if (XML_ReadChildInt(playerElement, "jumpacceleration", &prefab->jumpAcceleration) == false)
		{
			prefab->jumpAcceleration = 0; // Cannot jump
		}
		if (XML_ReadChildInt(playerElement, "climbheight", &prefab->climbHeight) == false)
		{
			prefab->climbHeight= 0; // Cannot climb
		}

		// HEAD BOB
		if (XML_ReadChildFloat(playerElement, "headbobfreq", &prefab->headBobFreq) == false)
		{
			prefab->strafeSpeed = prefab->moveSpeed;
		}
		if (XML_ReadChildFloat(playerElement, "headbobamplitude", &prefab->headBobAmplitude) == false)
		{
			prefab->strafeAcceleration = prefab->strafeSpeed;
		}

		// INVENTORY

		inventoryElement = playerElement->FirstChildElement()->FirstChildElement("inventory");
		if (inventoryElement)
		{
			tinyxml2::XMLElement* itemElement = inventoryElement->FirstChildElement()->FirstChildElement("item");
			while (itemElement)
			{
				int typeNumber = 0;
				if (XML_ReadChildInt(itemElement, "number", &typeNumber) == false)
				{

					const char* typeName = XML_ReadChildString(itemElement, "name");
					if (typeName == nullptr)
					{
						Log_Error("No name or number for inventory item in XML\n");
					}
					else
					{
						typeNumber = NameToDoomEditorNumberChar(typeName);
					}
				}

				int itemAmount = 1;
				XML_ReadChildInt(itemElement, "amount", &itemAmount);
				prefab->GiveItem((DOOM_EDITOR_NUMBER)typeNumber, itemAmount);

				itemElement = itemElement->NextSiblingElement("item");
			}
		}

player_end:
		playerElement = playerElement->NextSiblingElement("player");
	}
	return true;
}


bool PlayerManager::LoadPlayer(Player* player, int playerNumber)
{
	if (playerNumber < 1 || playerNumber > BUNNY_SECTOR_MAX_PLAYERS)
	{
		Log_ErrorF("Invalid player number, must be 1 to %d\n", BUNNY_SECTOR_MAX_PLAYERS);
		return false;
	}
	// Copy from
	if (prefabs[0].number == 0)
	{
		*player = prefabs[0];
	}
	if (prefabs[playerNumber].number == playerNumber)
	{
		*player = prefabs[playerNumber];
	}
	player->number = playerNumber; // NOTE This is copied too so reset
	return true;
}
