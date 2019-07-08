#pragma once

struct command_alias_t
{
    const char* primary;
    const char* alts[4];
};

class ChatCommands
{
  private:
    static const command_alias_t aliases[];

    static void MatchAliases(char *command);

  public:

    static void ExecuteChatCommand(char* command);
};
