#include "cbase.h"

#include "ChatCommands.h"

#include "fmtstr.h"

#include "tier0/memdbgon.h"

abstract_class ChatCommand
{
public:
    virtual ~ChatCommand() = default;

    /// <summary>
    ///  Determines if this command should operate on the given input
    /// </summary>
    /// <param name="pInput">The input to verify; the text immediately following the '/' or '!' or '.', e.g. "s" or "stage"</param>
    /// <returns>true if so, else false</returns>
    virtual bool CanOperate(const char *pInput) = 0;

    /// <summary>
    /// Operates on the given input string from chat. Only called if CanOperate returns true.
    /// </summary>
    /// <param name="chatInputStr">The entire chat input string.</param>
    /// <param name="splitInputStr">The chat input pre-split based on the space (" ") character.</param>
    virtual void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) = 0;
};

class TrackTeleportCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "r") || FStrEq(pInput, "restart");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        engine->ClientCmd(CFmtStr("mom_restart %s", splitInputStr.Count() > 1 ? splitInputStr[1] : ""));
    }
};

class BonusTeleportCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "b") || FStrEq(pInput, "bonus");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        if (splitInputStr.Count() < 2)
        {
            ClientPrint(CBasePlayer::GetLocalPlayer(), HUD_PRINTTALK, R"(Usage: !b <bonus number> (i.e. "!b 2" or "!bonus 2"))");
            return;
        }

        engine->ClientCmd(CFmtStr("mom_restart %s", splitInputStr[1]));
    }
};

class StageTeleportCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "s") || FStrEq(pInput, "stage");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        engine->ClientCmd(CFmtStr("mom_restart_stage %s", splitInputStr.Count() > 1 ? splitInputStr[1] : ""));
    }
};

class SetStartMarkCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "setstart");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        engine->ClientCmd("mom_start_mark_create");
    }
};

class ShowPlayerClipsCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "clips") || FStrEq(pInput, "showclips");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        static ConVarRef drawclips("r_drawclipbrushes");

        drawclips.SetValue((drawclips.GetInt() + 1) % 3);
    }
};

class ShowTriggersCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "triggers") || FStrEq(pInput, "showtriggers");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        engine->ClientCmd("showtriggers_toggle");
    }
};

class SpecPlayerCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "spec") || FStrEq(pInput, "spectate");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        engine->ClientCmd(CFmtStr("mom_spectate %s", splitInputStr.Count() > 1 ? splitInputStr[1] : ""));
    }
};

class StopSpecCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "spawn") || FStrEq(pInput, "respawn") ||
               FStrEq(pInput, "specstop") || FStrEq(pInput, "stopspec");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        engine->ClientCmd("mom_spectate_stop");
    }
};

class GoToPlayerCommand : public ChatCommand
{
    bool CanOperate(const char *pInput) override
    {
        return FStrEq(pInput, "goto");
    }

    void Operate(const CUtlString &chatInputStr, const CSplitString &splitInputStr) override
    {
        if (splitInputStr.Count() < 2)
            return;

        engine->ClientCmd(CFmtStr("mom_lobby_teleport %s", splitInputStr[1]));
    }
};

static CUtlVector<ChatCommand *> g_vecCommands;

void ChatCommands::InitCommands()
{
    g_vecCommands.AddToTail(new TrackTeleportCommand);
    g_vecCommands.AddToTail(new BonusTeleportCommand);
    g_vecCommands.AddToTail(new StageTeleportCommand);
    g_vecCommands.AddToTail(new SetStartMarkCommand);
    g_vecCommands.AddToTail(new ShowPlayerClipsCommand);
    g_vecCommands.AddToTail(new ShowTriggersCommand);
    g_vecCommands.AddToTail(new SpecPlayerCommand);
    g_vecCommands.AddToTail(new StopSpecCommand);
    g_vecCommands.AddToTail(new GoToPlayerCommand);
}

bool ChatCommands::HandleChatCommand(const CUtlString &chatInputStr)
{
    if (chatInputStr[0] != '/' && chatInputStr[0] != '!' && chatInputStr[0] != '.')
        return false;

    CSplitString splitInputStr(chatInputStr.Get(), " ");
    auto pTagStr = splitInputStr[0];
    if (Q_strlen(pTagStr) < 2) // Needs to have at least *something*; can't be just "/" or "!" or "."
        return false;

    pTagStr++; // Skip over the "/" or "!" or "."

    FOR_EACH_VEC(g_vecCommands, i)
    {
        const auto pCmd = g_vecCommands[i];
        if (pCmd->CanOperate(pTagStr))
        {
            pCmd->Operate(chatInputStr, splitInputStr);
            return true;
        }
    }

    return false;
}