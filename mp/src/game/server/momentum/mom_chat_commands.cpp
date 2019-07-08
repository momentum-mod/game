#include "cbase.h"

#include "mom_chat_commands.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

const command_alias_t ChatCommands::aliases[] = {
    {"restart", {"r"}}, 
    {"reset", {"back"}}, 
    {"bonus", {"b"}}, 
    {"stage", {"s"}},
    {"spectate", {"spec"}}
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

    //Just turns !command into mom_command, both reset and spectate are rolled into this
    else
    {
        if (args.ArgC() == 1)
            engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_%s", args.Arg(0));
        else
            engine->ClientCommand(CMomentumPlayer::GetLocalPlayer()->edict(), "mom_%s %s", args.Arg(0), args.Arg(1));
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
