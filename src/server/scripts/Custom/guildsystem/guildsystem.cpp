/*
 * Copyright (C) 2016-2019 AtieshCore <https://at-wow.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Chat.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "CommandScript.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "WorldSession.h"

using namespace Acore::ChatCommands;

class GuildSystem_player : public PlayerScript
{
public:
    GuildSystem_player() : PlayerScript("GuildSystem_player") { }

    void OnGiveXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource)
    {
        if (Guild* guildtarget = player->GetGuild())
        {
            if (guildtarget->GetGuildLevel() >= 1)
                amount *= 2;
        }
    }

    void OnLogin(Player* player)
    {
        if (Guild* guildtarget = player->GetGuild())
            player->AddGuildAurasForPlr(guildtarget->GetGuildLevel());
    }

    void OnLogout(Player* player) override
    {
        if (Guild* guildtarget = player->GetGuild())
            player->RemoveGuildAurasForPlr();
    }
};

class GuildSystem : public GuildScript
{
public:
    GuildSystem() : GuildScript("GuildSystem") { }

    std::string GetNewReachedLevel(uint32 level, Player* player) const
    {
        std::ostringstream str;
        if (player)
            str << "Congratulations! The guild has reached a " << level << " level. The last experience has gained by " << player->GetName();
        else
            str << "Congratulations! The guild has reached a " << level << " level";
        return str.str();
    }

    std::string GetNewReachedExp(uint32 value, Player* player) const
    {
        std::ostringstream str;
        if (player)
            str << "The guild has received a " << value << " experience. Points has gained by " << player->GetName();
        else
            str << "The guild has received a " << value << " experience points";
        return str.str();
    }

    void OnAddMember(Guild* guild, Player* player, uint8& /*plRank*/) override
    {
        player->AddGuildAurasForPlr(guild->GetGuildLevel());
    }

    void OnRemoveMember(Guild* guild, Player* player, bool /*isDisbanding*/, bool /*isKicked*/) override
    {
        player->RemoveGuildAurasForPlr();
    }

    void OnLevelUp(Guild* guild, Player* player, uint32 receivedLevel) override
    {
        guild->BroadcastToGuildNote(GetNewReachedLevel(receivedLevel, player));
        guild->CastGuildLevelAuras(receivedLevel);
        switch (receivedLevel)
        {
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
                break;
        }
    }

    void OnExpReceived(Guild* guild, Player* player, uint32 receivedExp) override
    {
        guild->BroadcastToGuildNote(GetNewReachedExp(receivedExp, player));
    }

    void OnArenaWon(Guild* guild, Player* player) override
    {
        uint32 receivedExp = 5;
        guild->AddGuildExp(receivedExp, player);
    }

    void OnBattlegroundWon(Guild* guild, Player* player) override
    {
        uint32 receivedExp = 1;
        guild->AddGuildExp(receivedExp, player);
    }

    void OnLFGComplete(Guild* guild, Player* player) override
    {
        uint32 receivedExp = 1;
        guild->AddGuildExp(receivedExp, player);
    }
};

class guildprogress_commandscript : public CommandScript
{
public:
    guildprogress_commandscript() : CommandScript("guildprogress_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable guildProgressCommandTable =
        {
            { "addexperience",  HandleGuildAddExperienceCommand, SEC_GAMEMASTER, Console::Yes },
            { "addlevel",       HandleGuildAddLevelCommand,      SEC_GAMEMASTER, Console::Yes },
            { "removelevel",    HandleGuildRemoveLevelCommand,   SEC_GAMEMASTER, Console::Yes },
            { "repair",         HandleGuildRepairCommand,        SEC_GAMEMASTER, Console::Yes },
            { "mybank",         HandleGuildMyBankCommand,        SEC_GAMEMASTER, Console::Yes },
            { "",               HandleGuildProgressCommand,      SEC_GAMEMASTER, Console::Yes },
        };
        static ChatCommandTable commandTable =
        {
            { "gprogress", guildProgressCommandTable }
        };
        return commandTable;
    }

    static bool HandleGuildProgressCommand(ChatHandler* handler, char const* args)
    {
        Guild* guild = nullptr;

        if (args && args[0] != '\0')
        {
            if (isNumeric(args))
            {
                uint32 guildId = uint32(atoi(args));
                guild = sGuildMgr->GetGuildById(guildId);
            }
            else
            {
                std::string guildName = args;
                guild = sGuildMgr->GetGuildByName(guildName);
            }
        }
        else if (Player * target = handler->getSelectedPlayerOrSelf())
            guild = target->GetGuild();

        if (!guild)
            return false;

        // Display Guild Information
        handler->PSendSysMessage(LANG_GUILD_INFO_NAME, guild->GetName().c_str(), guild->GetId()); // Guild Id + Name

        handler->PSendSysMessage("Guild has %u level, and %u / 1500 experience for next level", guild->GetGuildLevel(), guild->GetGuildExperience()); // Extra Information
        return true;
    }

    static bool HandleGuildAddLevelCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
            return false;

        Guild* targetGuild = target->GetGuild();
        if (!targetGuild)
            return false;

        char* value = strtok((char*)args, " ");
        uint32 addedlvl = 1;
        if (value)
            addedlvl = atoi(value);

        targetGuild->AddGuildLevel(addedlvl, target);
        handler->PSendSysMessage("Guild %s has received %u additional levels, and now has %u level", targetGuild->GetName().c_str(), addedlvl, targetGuild->GetGuildLevel());
        return true;
    }

    static bool HandleGuildRemoveLevelCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
            return false;

        Guild* targetGuild = target->GetGuild();
        if (!targetGuild)
            return false;

        char* value = strtok((char*)args, " ");
        uint32 removedlvl = 1;
        if (value)
            removedlvl = atoi(value);

        targetGuild->RemoveGuildLevel(removedlvl);
        handler->PSendSysMessage("Guild %s has lost %u additional levels, and now has %u level", targetGuild->GetName().c_str(), removedlvl, targetGuild->GetGuildLevel());
        return true;
    }

    static bool HandleGuildAddExperienceCommand(ChatHandler* handler, char const* args)
    {
        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
            return false;

        Guild* targetGuild = target->GetGuild();
        if (!targetGuild)
            return false;

        char* value = strtok((char*)args, " ");
        uint32 addedExp = 1;
        if (value)
            addedExp = atoi(value);

        targetGuild->AddGuildExp(addedExp, target);
        handler->PSendSysMessage("Guild %s has received %u additional experience, and now has %u Exp", targetGuild->GetName().c_str(), addedExp, targetGuild->GetGuildExperience());
        return true;
    }

    static bool HandleGuildRepairCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        Guild* targetGuild = player->GetGuild();
        if (!targetGuild)
            return false;

        if (targetGuild->GetGuildLevel() < 3)
        {
            handler->PSendSysMessage("Your guild level are lower then 3 level");
            return true;
        }

        player->DurabilityRepairAll(false, 0, false);
        handler->PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, handler->GetNameLink(player).c_str());
        return true;
    }

    static bool HandleGuildMyBankCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        Guild* targetGuild = player->GetGuild();
        if (!targetGuild)
            return false;

        if (targetGuild->GetGuildLevel() < 5)
        {
            handler->PSendSysMessage("Your guild level are lower then 5 level");
            return true;
        }

        if (player->IsInCombat())
        {
            handler->SendSysMessage(LANG_YOU_IN_COMBAT);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if (player->IsInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return true;
        }
/*
        if (player->GetMap()->IsBattlegroundOrArena())
        {
            handler->SendSysMessage(LANG_VIP_BG);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if (player->HasStealthAura())
        {
            handler->SendSysMessage(LANG_VIP_STEALTH);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if (player->isDead() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        {
            handler->SendSysMessage(LANG_VIP_DEAD);
            handler->SetSentErrorMessage(true);
            return true;
        }
        */
        handler->GetSession()->SendShowBank(player->GetGUID());
        return true;
    }
};

void AddSC_GuildSystem()
{
    new GuildSystem_player();
    new GuildSystem();
    new guildprogress_commandscript();
}
