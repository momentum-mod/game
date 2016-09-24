#include "gameui2_interface.h"
#include "basepanel.h"
#include <icommandline.h>
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static CDllDemandLoader g_GameUI("GameUI");

IVEngineClient* engine = nullptr;
IEngineSound* enginesound = nullptr;
IEngineVGui* enginevgui = nullptr;
ISoundEmitterSystemBase* soundemitterbase = nullptr;
IVRenderView* render = nullptr;
IGameUI* gameui = nullptr;

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

	engine = static_cast<IVEngineClient*>(appFactory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr));
	enginesound = static_cast<IEngineSound*>(appFactory(IENGINESOUND_CLIENT_INTERFACE_VERSION, nullptr));
	enginevgui = static_cast<IEngineVGui*>(appFactory(VENGINE_VGUI_VERSION, nullptr));
	soundemitterbase = static_cast<ISoundEmitterSystemBase*>(appFactory(SOUNDEMITTERSYSTEM_INTERFACE_VERSION, nullptr));
	render = static_cast<IVRenderView*>(appFactory(VENGINE_RENDERVIEW_INTERFACE_VERSION, nullptr));

	CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
	if (gameUIFactory)
		gameui = static_cast<IGameUI*>(gameUIFactory(GAMEUI_INTERFACE_VERSION, nullptr));

	if (!enginesound || !enginevgui || !engine || !soundemitterbase || !render || !gameui)
		Error("CGameUI2::Initialize() failed to get necessary interfaces.\n");

    if (!CommandLine()->FindParm("-shaderedit"))
    {
        GetBasePanel()->Create();
        if (GetBasePanel())
            gameui->SetMainMenuOverride(GetBasePanel()->GetMainMenu()->GetVPanel());
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

}

void CGameUI2::OnLevelInitializePreEntity()
{

}

void CGameUI2::OnLevelInitializePostEntity()
{

}

void CGameUI2::OnLevelShutdown()
{

}

bool CGameUI2::IsInLevel()
{
    return engine->IsInGame() && !engine->IsLevelMainMenuBackground();
}

bool CGameUI2::IsInBackgroundLevel()
{
    return (engine->IsInGame() && engine->IsLevelMainMenuBackground()) || !engine->IsInGame();
}

bool CGameUI2::IsInMultiplayer()
{
	return (IsInLevel() && engine->GetMaxClients() > 1);
}

bool CGameUI2::IsInLoading()
{
	return (engine->IsDrawingLoadingImage() || engine->GetLevelName() == nullptr) || (!IsInLevel() && !IsInBackgroundLevel());
}

wchar_t* CGameUI2::GetLocalizedString(const char* text)
{
	wchar_t* localizedString = static_cast<wchar_t*>(malloc(sizeof(wchar_t) * 2048));
//	wchar_t* localizedString = new wchar_t[2048];
	
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

Vector2D CGameUI2::GetViewport()
{
	int32 viewportX, viewportY;
	engine->GetScreenSize(viewportX, viewportY);
	return Vector2D(viewportX, viewportY);
}

float CGameUI2::GetTime()
{
	return Plat_FloatTime();
}

vgui::VPANEL CGameUI2::GetRootPanel()
{
	return enginevgui->GetPanel(PANEL_GAMEUIDLL);
}

vgui::VPANEL CGameUI2::GetVPanel()
{
	return GetBasePanel()->GetVPanel();
}

CViewSetup CGameUI2::GetView()
{
	return m_pView;
}

VPlane* CGameUI2::GetFrustum()
{
	return m_pFrustum;
}

ITexture* CGameUI2::GetMaskTexture()
{
	return m_pMaskTexture;
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

void CGameUI2::SendMainMenuCommand(const char* cmd)
{
    gameui->SendMainMenuCommand(cmd);
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