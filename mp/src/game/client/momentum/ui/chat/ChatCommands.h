#pragma once

class CUtlString;
class ChatCommand;

/// <summary>
///  Chat commands are only being implemented as a quick way to do a command, and for backwards compatibility for
///  most players coming from servers, where dedicated commands were few and far between.
///
///  Given that we have dedicated UI, console commands, and even customizable HUD static menus, this can be explored for
///  expansion in the future, but will be hardcoded for now.
/// </summary>
class ChatCommands
{
public:
    static void InitCommands();

    static bool HandleChatCommand(const CUtlString &chatInputStr);
};