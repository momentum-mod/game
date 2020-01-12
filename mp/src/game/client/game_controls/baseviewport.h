//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#pragma once

// viewport interface for the rest of the dll
#include <game/client/iviewport.h>
#include <vgui_controls/Frame.h>
#include <igameevents.h>

class IBaseFileSystem;
class IGameUIFuncs;
class IGameEventManager;

//==============================================================================
class CBaseViewport : public vgui::EditablePanel, public IViewPort, public IGameEventListener2
{
	DECLARE_CLASS_SIMPLE( CBaseViewport, vgui::EditablePanel );

public: 
	CBaseViewport();
	virtual ~CBaseViewport();

	virtual IViewPortPanel* CreatePanelByName(const char *szPanelName);
	virtual IViewPortPanel* FindPanelByName(const char *szPanelName);
	virtual IViewPortPanel* GetActivePanel( void );
	virtual void RemoveAllPanels( void);

	virtual void ShowPanel( const char *pName, bool state );
	virtual void ShowPanel( IViewPortPanel* pPanel, bool state );
	virtual bool AddNewPanel( IViewPortPanel* pPanel, char const *pchDebugName );
	virtual void CreateDefaultPanels( void );
	virtual void UpdateAllPanels( void );
	virtual void PostMessageToPanel( const char *pName, KeyValues *pKeyValues );

	virtual void Start( IGameUIFuncs *pGameUIFuncs, IGameEventManager2 *pGameEventManager );
	virtual void SetParent(vgui::VPANEL parent);

	virtual void ReloadScheme(const char *fromFile);
	virtual void ActivateClientUI();
	virtual void HideClientUI();
	virtual bool AllowedToPrintText( void );
	
#ifndef _XBOX
	virtual int GetViewPortScheme() { return m_pBackGround->GetScheme(); }
	virtual vgui::VPANEL GetViewPortPanel() { return m_pBackGround->GetVParent(); }
#endif
	virtual vgui::AnimationController *GetAnimationController() { return m_pAnimController; }

	virtual void ShowBackGround(bool bShow) 
	{ 
#ifndef _XBOX
		m_pBackGround->SetVisible( bShow ); 
#endif
	}

	virtual int GetDeathMessageStartHeight( void );	

	// virtual void ChatInputPosition( int *x, int *y );
	
public: // IGameEventListener:
	virtual void FireGameEvent( IGameEvent * event);


protected:

	bool LoadHudAnimations( void );

#ifndef _XBOX
	class CBackGroundPanel : public vgui::Frame
	{
	public:
		DECLARE_CLASS_SIMPLE(CBackGroundPanel, vgui::Frame);
        CBackGroundPanel(vgui::Panel *parent);

	protected:
		void ApplySchemeSettings(vgui::IScheme *pScheme) override;
		void PerformLayout() override;
	    virtual void OnMousePressed(vgui::MouseCode code) { }// don't respond to mouse clicks
		virtual vgui::VPANEL IsWithinTraverse(int x, int y, bool traversePopups)
		{
			return (vgui::VPANEL)0;
		}
	};
#endif
protected:

	virtual void Paint();
	virtual void OnThink(); 
	virtual void OnScreenSizeChanged(int iOldWide, int iOldTall);
	void PostMessageToPanel( IViewPortPanel* pPanel, KeyValues *pKeyValues );

protected:
	IGameUIFuncs*		m_GameuiFuncs; // for key binding details
	IGameEventManager2*	m_GameEventManager;
#ifndef _XBOX
	CBackGroundPanel	*m_pBackGround;
#endif
	CUtlVector<IViewPortPanel*> m_Panels;
	
	bool				m_bHasParent; // Used to track if child windows have parents or not.
	bool				m_bInitialized;
	IViewPortPanel		*m_pActivePanel;
	IViewPortPanel		*m_pLastActivePanel;
	vgui::HCursor		m_hCursorNone;
	vgui::AnimationController *m_pAnimController;
	int					m_OldSize[2];
};