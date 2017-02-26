#include "gameui2_interface.h"
#include "basepanel.h"
#include <icommandline.h>
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static CDllDemandLoader g_GameUI("GameUI");

static CGameUI2 g_GameUI2;
CGameUI2& GameUI2()
{
	return g_GameUI2;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameUI2, IGameUI2, GAMEUI2_DLL_INTERFACE_VERSION, g_GameUI2);

void CGameUI2::Initialize(CreateInterfaceFn appFactory)
{
	MEM_ALLOC_CREDIT();
	ConnectTier1Libraries(&appFactory, 1);
	ConnectTier2Libraries(&appFactory, 1);
	ConVar_Register(FCVAR_CLIENTDLL);
	ConnectTier3Libraries(&appFactory, 1);

    m_pEngine = static_cast<IVEngineClient*>(appFactory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr));
	m_pEngineSound = static_cast<IEngineSound*>(appFactory(IENGINESOUND_CLIENT_INTERFACE_VERSION, nullptr));
	m_pEngineVGUI = static_cast<IEngineVGui*>(appFactory(VENGINE_VGUI_VERSION, nullptr));
	m_pSoundEmitterBase = static_cast<ISoundEmitterSystemBase*>(appFactory(SOUNDEMITTERSYSTEM_INTERFACE_VERSION, nullptr));
	m_pRenderView = static_cast<IVRenderView*>(appFactory(VENGINE_RENDERVIEW_INTERFACE_VERSION, nullptr));
    m_pMaterialSystem = static_cast<IMaterialSystem*>(appFactory(MATERIAL_SYSTEM_INTERFACE_VERSION, nullptr));

	CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
	if (gameUIFactory)
		m_pGameUI = static_cast<IGameUI*>(gameUIFactory(GAMEUI_INTERFACE_VERSION, nullptr));

    if (!m_pEngineSound || !m_pEngineVGUI || !m_pEngine || !m_pSoundEmitterBase || !m_pRenderView || !m_pGameUI || !m_pMaterialSystem)
		Error("CGameUI2::Initialize() failed to get necessary interfaces.\n");

    if (!CommandLine()->FindParm("-shaderedit"))
    {
        GetBasePanel()->Create();
        if (GetBasePanel())
        {
            m_pGameUI->SetMainMenuOverride(GetBasePanel()->GetMainMenu()->GetVPanel()); 

            m_pAnimationController = new vgui::AnimationController(GetBasePanel());
            m_pAnimationController->SetProportional(false);
            m_pAnimationController->SetScheme(GetBasePanel()->GetScheme());
        } 
    }
}

void CGameUI2::Shutdown()
{
    if (GetBasePanel())
    {
        GetBasePanel()->GetMainMenu()->DeletePanel();
        GetBasePanel()->DeletePanel();
    }

	ConVar_Unregister();
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
}

void CGameUI2::OnInitialize()
{

}

void CGameUI2::OnShutdown()
{

}

void CGameUI2::OnUpdate()
{

    if (m_pAnimationController)
        m_pAnimationController->UpdateAnimations(m_pEngine->Time());
}

void CGameUI2::OnLevelInitializePreEntity()
{
    if (!GetBasePanel())
        return;

    GetBasePanel()->SetVisible(false);

    if (m_pAnimationController)
    {
        m_pAnimationController->UpdateAnimations(m_pEngine->Time());
        m_pAnimationController->RunAllAnimationsToCompletion();
    }
}

void CGameUI2::OnLevelInitializePostEntity()
{
    if (!GetBasePanel())
        return;

    GetBasePanel()->SetVisible(true);
    GetBasePanel()->GetMainMenu()->SetVisible(true);
}

void CGameUI2::OnLevelShutdown()
{
    if (m_pAnimationController)
    {
        m_pAnimationController->UpdateAnimations(m_pEngine->Time());
        m_pAnimationController->RunAllAnimationsToCompletion();
    }
}

bool CGameUI2::IsInLevel()
{
    return m_pEngine->IsInGame() && !m_pEngine->IsLevelMainMenuBackground();
}

bool CGameUI2::IsInBackgroundLevel()
{
    return (m_pEngine->IsInGame() && m_pEngine->IsLevelMainMenuBackground()) || !m_pEngine->IsInGame();
}

bool CGameUI2::IsInMultiplayer()
{
    return (IsInLevel() && m_pEngine->GetMaxClients() > 1);
}

bool CGameUI2::IsInLoading()
{
    return (m_pEngine->IsDrawingLoadingImage() || m_pEngine->GetLevelName() == nullptr) || (!IsInLevel() && !IsInBackgroundLevel());
}

wchar_t* CGameUI2::GetLocalizedString(const char* text)
{
	wchar_t* localizedString = static_cast<wchar_t*>(malloc(sizeof(wchar_t) * 2048));
	
	if (text[0] == '#')
	{
		wchar_t* tempString = g_pVGuiLocalize->Find(text);
		if (tempString)
		{
			const size_t cSizeText = wcslen(tempString) + 1;
			wcsncpy(localizedString, tempString, cSizeText);
			localizedString[cSizeText - 1] = 0;
		}
		else
		{
			const size_t cSizeText = strlen(text) + 1;
			mbstowcs(localizedString, text, cSizeText);
		}
	}
	else
	{
		const size_t cSizeText = strlen(text) + 1;
		mbstowcs(localizedString, text, cSizeText);
	}

	return localizedString;
}

Vector2D CGameUI2::GetViewport() const
{
	int32 viewportX, viewportY;
    m_pEngine->GetScreenSize(viewportX, viewportY);
	return Vector2D(viewportX, viewportY);
}

vgui::VPANEL CGameUI2::GetRootPanel() const
{
	return m_pEngineVGUI->GetPanel(PANEL_GAMEUIDLL);
}

vgui::VPANEL CGameUI2::GetVPanel() const
{
	return GetBasePanel()->GetVPanel();
}

void CGameUI2::SetView(const CViewSetup& view)
{
	m_pView = view;
}

void CGameUI2::SetFrustum(VPlane* frustum)
{
	m_pFrustum = frustum;
}

void CGameUI2::SetMaskTexture(ITexture* maskTexture)
{
	m_pMaskTexture = maskTexture;
}

CON_COMMAND(gameui2_version, "")
{
	Msg("\n");
	ConColorMsg(Color(0, 148, 255, 255), "Label:\t");
		Msg("%s\n", GAMEUI2_DLL_INTERFACE_VERSION);
	ConColorMsg(Color(0, 148, 255, 255), "Date:\t");
		Msg("%s\n", __DATE__);
	ConColorMsg(Color(0, 148, 255, 255), "Time:\t");
		Msg("%s\n", __TIME__);
}