#pragma once

class CHudMenuStaticSystem : public CAutoGameSystem
{
  public:
    CHudMenuStaticSystem();

    void ShowMenu(const char *pszFileName);
    void HideMenu();
    bool IsMenuDisplayed(const char *pszMenuName);

    void ReloadMenuList();

    const char *GetMenuName(int index) { return m_vecMenuNames[index]; }
    int GetNumMenus() { return m_vecMenuNames.Size(); }

  protected:
    void PostInit() override;

  private:
    CUtlStringList m_vecMenuNames;
};

extern CHudMenuStaticSystem *g_pHudMenuStaticSystem;
