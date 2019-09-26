#include "cbase.h"

#include "mom_chat_commands.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

const command_alias_t ChatCommands::aliases[] = {
    {"restart", {"r"}}, 
    {"reset", {"back"}}, 
    {"bonus", {"b"}}, 
    {"stage", {"s"}},
    {"spectate", {"spec"}},
    {"start_mark_create", {"startpos", "setstart"}},
    {"zone_showmenu", {"zonemenu"}},
    {"saveloc_create", {"saveloc"}},
    {"saveloc_current", {"loadloc"}},
    {"saveloc_remove_current", {"delloc"}},
    {"settings_show", {"settings"}},

    {"weapon_momentum_pistol", {"usp", "glock", "pistol"}},
    {"weapon_momentum_rifle", {"rifle"}},
    {"weapon_momentum_smg", {"smg", "mp5", "p90"}},
    {"weapon_momentum_shotgun", {"shotgun", "m3"}},
    {"weapon_momentum_sniper", {"sniper", "awp"}},
    {"weapon_momentum_lmg", {"lmg", "negev", "m249"}},
    {"weapon_momentum_grenade", {"grenade"}},
    {"weapon_momentum_paintgun", {"paintgun"}},
    {"weapon_knife", {"knife"}}

};

void ChatCommands::ExecuteChatCommand(char *command)
{
    CCommand args;
    args.Tokenize(command);

    char com[128];
    Q_strcpy(com, args.Arg(0));

    MatchAliases(Q_strlower(com));

    if (Q_strcmp(com, "restart") == 0)
    {
        engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_restart 0");
    }
    else if (Q_strcmp(com, "bonus") == 0)
    {
        engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_restart %s",
                              args.ArgC() == 1 ? "1" : args.Arg(1));
    }
    else if (Q_strcmp(com, "stage") == 0)
    {
        if (args.ArgC() == 1)
            engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_reset");
        else
            engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_stage_tele %s", args.Arg(1));
    }
    //TODO: Implement pattern matching on map name to give the player a list of maps if multiple match
    else if (Q_strcmp(com, "map") == 0)
    {
        if (args.ArgC() == 1)
            ; //Warn player that command needs a parameter
        else
            engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "map %s", args.Arg(1));
    }
    //Handle weapon commands
    else if (Q_strnicmp(com, "weapon", 6) == 0)
    {
        engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "give %s", com);
    }

    //Just turns !command into mom_command, both reset and spectate are rolled into this
    else
    {
        if (args.ArgC() == 1)
            engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_%s", com);
        else
            engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_%s %s", com, args.Arg(1));
    }
}

void ChatCommands::MatchAliases(char *command)
{
    for (auto alias : aliases)
    {
        for (auto match : alias.alts)
        {
            if (match == nullptr)
                break;
            if (Q_strcmp(command, match) == 0)
            {
                Q_strcpy(command, alias.primary);
                return;
            }
        }
    }
}
