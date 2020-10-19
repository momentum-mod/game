#pragma once

class CHudMenuStaticSystem : public CAutoGameSystem
{
  public:
    CHudMenuStaticSystem();

    void ShowMenu(const char *pszFileName);
    void HideMenu();
    bool IsMenuDisplayed(const char *pszMenuName);
};

extern CHudMenuStaticSystem *g_pHudMenuStaticSystem;
