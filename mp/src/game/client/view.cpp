//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "view.h"
#include "hud.h"
#include "iviewrender.h"
#include "iviewrender_beams.h"
#include "view_shared.h"
#include "iinput.h"
#include "iclientmode.h"
#include "prediction.h"
#include "viewrender.h"
#include "c_te_legacytempents.h"
#include "cl_mat_stub.h"
#include "tier0/vprof.h"
#include "mathlib/vmatrix.h"
#include "c_world.h"
#include "smoke_fog_overlay.h"
#include "bitmap/tgawriter.h"
#include "hltvcamera.h"
#include "input.h"
#include "filesystem.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "toolframework_client.h"
#include "ienginevgui.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "ScreenSpaceEffects.h"
#include "util/engine_patch.h"

#if defined( HL2_CLIENT_DLL ) || defined( CSTRIKE_DLL ) || defined ( SDK_DLL )
#define USE_MONITORS
#endif

#ifdef PORTAL
#include "c_prop_portal.h" //portal surface rendering functions
#endif

	
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
		  
void ToolFramework_AdjustEngineViewport( int& x, int& y, int& width, int& height );
bool ToolFramework_SetupEngineView( Vector &origin, QAngle &angles, float &fov );
bool ToolFramework_SetupEngineMicrophone( Vector &origin, QAngle &angles );


extern ConVar default_fov;
extern bool g_bRenderingScreenshot;

#define SAVEGAME_SCREENSHOT_WIDTH	180
#define SAVEGAME_SCREENSHOT_HEIGHT	100

extern ConVar sensitivity;

ConVar zoom_sensitivity_ratio( "zoom_sensitivity_ratio", "1.0", 0, "Additional mouse sensitivity scale factor applied when FOV is zoomed in." );

CViewRender g_DefaultViewRender;
IViewRender *view = NULL;	// set in cldll_client_init.cpp if no mod creates their own

#if _DEBUG
bool g_bRenderingCameraView = false;
#endif

static void(*R_LoadSkys)() = nullptr;

// These are the vectors for the "main" view - the one the player is looking down.
// For stereo views, they are the vectors for the middle eye.
static Vector g_vecRenderOrigin(0,0,0);
static QAngle g_vecRenderAngles(0,0,0);
static Vector g_vecPrevRenderOrigin(0,0,0);	// Last frame's render origin
static QAngle g_vecPrevRenderAngles(0,0,0); // Last frame's render angles
static Vector g_vecVForward(0,0,0), g_vecVRight(0,0,0), g_vecVUp(0,0,0);
static VMatrix g_matCamInverse;

extern ConVar cl_forwardspeed;

static ConVar v_centermove( "v_centermove", "0.15");
static ConVar v_centerspeed( "v_centerspeed","500" );

ConVar v_viewmodel_fov( "fov_viewmodel", "65", FCVAR_ARCHIVE, "Sets the field-of-view for the viewmodel.", true, 0.1, true, 179.9 );
ConVar mat_viewportscale( "mat_viewportscale", "1.0", FCVAR_ARCHIVE, "Scale down the main viewport (to reduce GPU impact on CPU profiling)", true, (1.0f / 640.0f), true, 1.0f );
ConVar mat_viewportupscale( "mat_viewportupscale", "1", FCVAR_ARCHIVE, "Scale the viewport back up" );
ConVar cl_leveloverview( "cl_leveloverview", "0", FCVAR_CHEAT );

static ConVar r_mapextents( "r_mapextents", "16384", FCVAR_CHEAT, 
						   "Set the max dimension for the map.  This determines the far clipping plane" );

// UNDONE: Delete this or move to the material system?
ConVar	gl_clear( "gl_clear", "0");
ConVar	gl_clear_randomcolor( "gl_clear_randomcolor", "0", FCVAR_CHEAT, "Clear the back buffer to random colors every frame. Helps spot open seams in geometry." );

static ConVar r_farz( "r_farz", "-1", FCVAR_CHEAT, "Override the far clipping plane. -1 means to use the value in env_fog_controller." );
static ConVar cl_demoviewoverride( "cl_demoviewoverride", "0", 0, "Override view during demo playback" );

static Vector s_DemoView;
static QAngle s_DemoAngle;

static void CalcDemoViewOverride( Vector &origin, QAngle &angles )
{
	engine->SetViewAngles( s_DemoAngle );

	input->ExtraMouseSample( gpGlobals->absoluteframetime, true );

	engine->GetViewAngles( s_DemoAngle );

	Vector forward, right, up;

	AngleVectors( s_DemoAngle, &forward, &right, &up );

	float speed = gpGlobals->absoluteframetime * cl_demoviewoverride.GetFloat() * 320;
	
	s_DemoView += speed * input->KeyState (&in_forward) * forward  ;
	s_DemoView -= speed * input->KeyState (&in_back) * forward ;

	s_DemoView += speed * input->KeyState (&in_moveright) * right ;
	s_DemoView -= speed * input->KeyState (&in_moveleft) * right ;

	origin = s_DemoView;
	angles = s_DemoAngle;
}

//-----------------------------------------------------------------------------
// Accessors to return the main view (where the player's looking)
//-----------------------------------------------------------------------------
const Vector &MainViewOrigin()
{
	return g_vecRenderOrigin;
}

const QAngle &MainViewAngles()
{
	return g_vecRenderAngles;
}

const Vector &MainViewForward()
{
	return g_vecVForward;
}

const Vector &MainViewRight()
{
	return g_vecVRight;
}

const Vector &MainViewUp()
{
	return g_vecVUp;
}

const VMatrix &MainWorldToViewMatrix()
{
	return g_matCamInverse;
}

const Vector &PrevMainViewOrigin()
{
	return g_vecPrevRenderOrigin;
}

const QAngle &PrevMainViewAngles()
{
	return g_vecPrevRenderAngles;
}

//-----------------------------------------------------------------------------
// Compute the world->camera transform
//-----------------------------------------------------------------------------
void ComputeCameraVariables( const Vector &vecOrigin, const QAngle &vecAngles, 
	Vector *pVecForward, Vector *pVecRight, Vector *pVecUp, VMatrix *pMatCamInverse )
{
	// Compute view bases
	AngleVectors( vecAngles, pVecForward, pVecRight, pVecUp );

	for (int i = 0; i < 3; ++i)
	{
		(*pMatCamInverse)[0][i] = (*pVecRight)[i];	
		(*pMatCamInverse)[1][i] = (*pVecUp)[i];	
		(*pMatCamInverse)[2][i] = -(*pVecForward)[i];	
		(*pMatCamInverse)[3][i] = 0.0F;
	}
	(*pMatCamInverse)[0][3] = -DotProduct( *pVecRight, vecOrigin );
	(*pMatCamInverse)[1][3] = -DotProduct( *pVecUp, vecOrigin );
	(*pMatCamInverse)[2][3] =  DotProduct( *pVecForward, vecOrigin );
	(*pMatCamInverse)[3][3] = 1.0F;
}


bool R_CullSphere(
	VPlane const *pPlanes,
	int nPlanes,
	Vector const *pCenter,
	float radius)
{
	for(int i=0; i < nPlanes; i++)
		if(pPlanes[i].DistTo(*pCenter) < -radius)
			return true;
	
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void StartPitchDrift( void )
{
	view->StartPitchDrift();
}

static ConCommand centerview( "centerview", StartPitchDrift );

extern ConVar default_fov;



//-----------------------------------------------------------------------------
// Purpose: Initializes all view systems
//-----------------------------------------------------------------------------
void CViewRender::Init( void )
{
	memset( &m_PitchDrift, 0, sizeof( m_PitchDrift ) );

	m_bDrawOverlay = false;

	m_pDrawEntities		= cvar->FindVar( "r_drawentities" );
	m_pDrawBrushModels	= cvar->FindVar( "r_drawbrushmodels" );

	beams->InitBeams();
	tempents->Init();

	m_TranslucentSingleColor.Init( "debug/debugtranslucentsinglecolor", TEXTURE_GROUP_OTHER );
	m_ModulateSingleColor.Init( "engine/modulatesinglecolor", TEXTURE_GROUP_OTHER );
	
	extern CMaterialReference g_material_WriteZ;
	g_material_WriteZ.Init( "engine/writez", TEXTURE_GROUP_OTHER );

	// FIXME:  
	QAngle angles;
	engine->GetViewAngles( angles );
	AngleVectors( angles, &m_vecLastFacing );

    // Grab the R_LoadSkys pointer from engine
#ifdef _WIN32
    R_LoadSkys = (void(*)())CEngineBinary::FindPattern("\x55\x8B\xEC\x81\xEC\x88\x00\x00\x00\x8D\x4D\xF8", "xxxxxxxxxxxx");
#else // POSIX
    R_LoadSkys = (void(*)())CEngineBinary::FindPattern("\x55\x89\xE5\x81\xEC\xA8\x00\x00\x00\x89\x5D\xF8", "xxxxxxxxxxxx");
#endif // WIN32

#if defined( CSTRIKE_DLL )
	m_flLastFOV = default_fov.GetFloat();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called once per level change
//-----------------------------------------------------------------------------
void CViewRender::LevelInit( void )
{
	beams->ClearBeams();
	tempents->Clear();

	m_BuildWorldListsNumber = 0;
	m_BuildRenderableListsNumber = 0;

	m_rbTakeFreezeFrame = false;
	m_flFreezeFrameUntil = 0;

	// Clear our overlay materials
	m_ScreenOverlayMaterial.Init( NULL );

	// Init all IScreenSpaceEffects
	g_pScreenSpaceEffects->InitScreenSpaceEffects( );
}

//-----------------------------------------------------------------------------
// Purpose: Called once per level change
//-----------------------------------------------------------------------------
void CViewRender::LevelShutdown( void )
{
	g_pScreenSpaceEffects->ShutdownScreenSpaceEffects( );
}

//-----------------------------------------------------------------------------
// Purpose: Called at shutdown
//-----------------------------------------------------------------------------
void CViewRender::Shutdown( void )
{
	m_TranslucentSingleColor.Shutdown( );
	m_ModulateSingleColor.Shutdown( );
	m_ScreenOverlayMaterial.Shutdown();
	m_UnderWaterOverlayMaterial.Shutdown();
	beams->ShutdownBeams();
	tempents->Shutdown();
}


//-----------------------------------------------------------------------------
// Returns the worldlists build number
//-----------------------------------------------------------------------------

int CViewRender::BuildWorldListsNumber( void ) const
{
	return m_BuildWorldListsNumber;
}

//-----------------------------------------------------------------------------
// Purpose: Start moving pitch toward ideal
//-----------------------------------------------------------------------------
void CViewRender::StartPitchDrift (void)
{
	if ( m_PitchDrift.laststop == gpGlobals->curtime )
	{
		// Something else is blocking the drift.
		return;		
	}

	if ( m_PitchDrift.nodrift || !m_PitchDrift.pitchvel )
	{
		m_PitchDrift.pitchvel	= v_centerspeed.GetFloat();
		m_PitchDrift.nodrift	= false;
		m_PitchDrift.driftmove	= 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CViewRender::StopPitchDrift (void)
{
	m_PitchDrift.laststop	= gpGlobals->curtime;
	m_PitchDrift.nodrift	= true;
	m_PitchDrift.pitchvel	= 0;
}

//-----------------------------------------------------------------------------
// Purpose: Moves the client pitch angle towards cl.idealpitch sent by the server.
// If the user is adjusting pitch manually, either with lookup/lookdown,
//   mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.
//-----------------------------------------------------------------------------
void CViewRender::DriftPitch (void)
{
	float		delta, move;

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	if ( engine->IsHLTV() || ( player->GetGroundEntity() == NULL ) || engine->IsPlayingDemo() )
	{
		m_PitchDrift.driftmove = 0;
		m_PitchDrift.pitchvel = 0;
		return;
	}

	// Don't count small mouse motion
	if ( m_PitchDrift.nodrift )
	{
		if ( fabs( input->GetLastForwardMove() ) < cl_forwardspeed.GetFloat() )
		{
			m_PitchDrift.driftmove = 0;
		}
		else
		{
			m_PitchDrift.driftmove += gpGlobals->frametime;
		}
	
		if ( m_PitchDrift.driftmove > v_centermove.GetFloat() )
		{
			StartPitchDrift ();
		}
		return;
	}
	
	// How far off are we
	delta = prediction->GetIdealPitch() - player->GetAbsAngles()[ PITCH ];
	if ( !delta )
	{
		m_PitchDrift.pitchvel = 0;
		return;
	}

	// Determine movement amount
	move = gpGlobals->frametime * m_PitchDrift.pitchvel;
	// Accelerate
	m_PitchDrift.pitchvel += gpGlobals->frametime * v_centerspeed.GetFloat();
	
	// Move predicted pitch appropriately
	if (delta > 0)
	{
		if ( move > delta )
		{
			m_PitchDrift.pitchvel = 0;
			move = delta;
		}
		player->SetLocalAngles( player->GetLocalAngles() + QAngle( move, 0, 0 ) );
	}
	else if ( delta < 0 )
	{
		if ( move > -delta )
		{
			m_PitchDrift.pitchvel = 0;
			move = -delta;
		}
		player->SetLocalAngles( player->GetLocalAngles() - QAngle( move, 0, 0 ) );
	}
}




// This is called by cdll_client_int to setup view model origins. This has to be done before
// simulation so entities can access attachment points on view models during simulation.
void CViewRender::OnRenderStart()
{
	VPROF_("CViewRender::OnRenderStart", 2, VPROF_BUDGETGROUP_OTHER_UNACCOUNTED, false, 0);

    SetUpViews();

	// Adjust mouse sensitivity based upon the current FOV
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( player )
	{
		default_fov.SetValue( player->m_iDefaultFOV );

		//Update our FOV, including any zooms going on
		int iDefaultFOV = default_fov.GetInt();
		int	localFOV	= player->GetFOV();
		int min_fov		= player->GetMinFOV();

		// Don't let it go too low
		localFOV = MAX( min_fov, localFOV );

		gHUD.m_flFOVSensitivityAdjust = 1.0f;
		if ( gHUD.m_flMouseSensitivityFactor )
		{
			gHUD.m_flMouseSensitivity = sensitivity.GetFloat() * gHUD.m_flMouseSensitivityFactor;
		}
		else
		{
			// No override, don't use huge sensitivity
			if ( localFOV == iDefaultFOV )
			{
				// reset to saved sensitivity
				gHUD.m_flMouseSensitivity = 0;
			}
			else
			{  
				// Set a new sensitivity that is proportional to the change from the FOV default and scaled
				//  by a separate compensating factor
				if ( iDefaultFOV == 0 )
				{
					Assert(0); // would divide by zero, something is broken with iDefatulFOV
					iDefaultFOV = 1;
				}
				gHUD.m_flFOVSensitivityAdjust = 
					((float)localFOV / (float)iDefaultFOV) * // linear fov downscale
					zoom_sensitivity_ratio.GetFloat(); // sensitivity scale factor

				gHUD.m_flMouseSensitivity = gHUD.m_flFOVSensitivityAdjust * sensitivity.GetFloat(); // regular sensitivity
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : const CViewSetup
//-----------------------------------------------------------------------------
const CViewSetup *CViewRender::GetViewSetup( void ) const
{
	return &m_CurrentView;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : const CViewSetup
//-----------------------------------------------------------------------------
const CViewSetup *CViewRender::GetPlayerViewSetup( void ) const
{
	const CViewSetup &viewSetup = m_View;
    return &viewSetup;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CViewRender::DisableVis( void )
{
	m_bForceNoVis = true;
}

#ifdef DBGFLAG_ASSERT
static Vector s_DbgSetupOrigin;
static QAngle s_DbgSetupAngles;
#endif

//-----------------------------------------------------------------------------
// Gets znear + zfar
//-----------------------------------------------------------------------------
float CViewRender::GetZNear()
{
	return VIEW_NEARZ;
}

float CViewRender::GetZFar()
{
	// Initialize view structure with default values
	float farZ;
	if ( r_farz.GetFloat() < 1 )
	{
		// Use the far Z from the map's parameters.
		farZ = r_mapextents.GetFloat() * 1.73205080757f;
		
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if( pPlayer && pPlayer->GetFogParams() )
		{
			if ( pPlayer->GetFogParams()->farz > 0 )
			{
				farZ = pPlayer->GetFogParams()->farz;
			}
		}
	}
	else
	{
		farZ = r_farz.GetFloat();
	}

	return farZ;
}


//-----------------------------------------------------------------------------
// Sets up the view parameters
//-----------------------------------------------------------------------------
void CViewRender::SetUpViews()
{
	VPROF("CViewRender::SetUpViews");

	// Initialize view structure with default values
	float farZ = GetZFar();

    // Set up the mono/middle view.
    CViewSetup &viewSetup = m_View;

	viewSetup.zFar				= farZ;
	viewSetup.zFarViewmodel	    = farZ;
	// UNDONE: Make this farther out? 
	//  closest point of approach seems to be view center to top of crouched box
	viewSetup.zNear			    = GetZNear();
	viewSetup.zNearViewmodel	    = 1;
	viewSetup.fov				= default_fov.GetFloat();

	viewSetup.m_bOrtho			= false;

	// Enable spatial partition access to edicts
	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	// You in-view weapon aim.
	bool bCalcViewModelView = false;
	Vector ViewModelOrigin;
	QAngle ViewModelAngles;

	if ( engine->IsHLTV() )
	{
		HLTVCamera()->CalcView( viewSetup.origin, viewSetup.angles, viewSetup.fov );
	}
	else
	{
		// FIXME: Are there multiple views? If so, then what?
		// FIXME: What happens when there's no player?
		if (pPlayer)
		{
			pPlayer->CalcView( viewSetup.origin, viewSetup.angles, viewSetup.zNear, viewSetup.zFar, viewSetup.fov );

			// If we are looking through another entities eyes, then override the angles/origin for view
			int viewentity = render->GetViewEntity();

			if ( !g_nKillCamMode && (pPlayer->entindex() != viewentity) )
			{
				C_BaseEntity *ve = cl_entitylist->GetEnt( viewentity );
				if ( ve )
				{
					VectorCopy( ve->GetAbsOrigin(), viewSetup.origin );
					VectorCopy( ve->GetAbsAngles(), viewSetup.angles );
				}
			}

			// There is a viewmodel.
			bCalcViewModelView = true;
			ViewModelOrigin = viewSetup.origin;
			ViewModelAngles = viewSetup.angles;
		}
		else
		{
			viewSetup.origin.Init();
			viewSetup.angles.Init();
		}

		// Even if the engine is paused need to override the view
		// for keeping the camera control during pause.
		g_pClientMode->OverrideView( &viewSetup );
	}

	// give the toolsystem a chance to override the view
	ToolFramework_SetupEngineView( viewSetup.origin, viewSetup.angles, viewSetup.fov );

	if ( engine->IsPlayingDemo() )
	{
		if ( cl_demoviewoverride.GetFloat() > 0.0f )
		{
			// Retrieve view angles from engine ( could have been set in IN_AdjustAngles above )
			CalcDemoViewOverride( viewSetup.origin, viewSetup.angles );
		}
		else
		{
			s_DemoView = viewSetup.origin;
			s_DemoAngle = viewSetup.angles;
		}
	}

	//Find the offset our current FOV is from the default value
	float fDefaultFov = default_fov.GetFloat();
	float flFOVOffset = fDefaultFov - viewSetup.fov;

	//Adjust the viewmodel's FOV to move with any FOV offsets on the viewer's end
	viewSetup.fovViewmodel = abs(g_pClientMode->GetViewModelFOV() - flFOVOffset);

	if ( bCalcViewModelView )
	{
		Assert ( pPlayer != NULL );
		pPlayer->CalcViewModelView ( ViewModelOrigin, ViewModelAngles );
	}

	// Disable spatial partition access
	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );

	// Enable access to all model bones
	C_BaseAnimating::PopBoneAccess( "OnRenderStart->CViewRender::SetUpView" ); // pops the (true, false) bone access set in OnRenderStart
	C_BaseAnimating::PushAllowBoneAccess( true, true, "CViewRender::SetUpView->OnRenderEnd" ); // pop is in OnRenderEnd()

	// Compute the world->main camera transform
    // This is only done for the main "middle-eye" view, not for the various other views.
	ComputeCameraVariables( viewSetup.origin, viewSetup.angles, 
		&g_vecVForward, &g_vecVRight, &g_vecVUp, &g_matCamInverse );

	// set up the hearing origin...
	AudioState_t audioState;
	audioState.m_Origin = viewSetup.origin;
	audioState.m_Angles = viewSetup.angles;
	audioState.m_bIsUnderwater = pPlayer && pPlayer->AudioStateIsUnderwater( viewSetup.origin );

	ToolFramework_SetupAudioState( audioState );

    // TomF: I wonder when the audio tools modify this, if ever...
    Assert ( viewSetup.origin == audioState.m_Origin );
    Assert ( viewSetup.angles == audioState.m_Angles );
	viewSetup.origin = audioState.m_Origin;
	viewSetup.angles = audioState.m_Angles;

	engine->SetAudioState( audioState );

	g_vecPrevRenderOrigin = g_vecRenderOrigin;
	g_vecPrevRenderAngles = g_vecRenderAngles;
	g_vecRenderOrigin = viewSetup.origin;
	g_vecRenderAngles = viewSetup.angles;

#ifdef DBGFLAG_ASSERT
	s_DbgSetupOrigin = viewSetup.origin;
	s_DbgSetupAngles = viewSetup.angles;
#endif
}




void CViewRender::WriteSaveGameScreenshotOfSize( const char *pFilename, int width, int height, bool bCreatePowerOf2Padded/*=false*/,
												 bool bWriteVTF/*=false*/ )
{
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PushMatrix();
	
	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PushMatrix();

	g_bRenderingScreenshot = true;

	// Push back buffer on the stack with small viewport
	pRenderContext->PushRenderTargetAndViewport( NULL, 0, 0, width, height );

	// render out to the backbuffer
	CViewSetup viewSetup = m_View;
	viewSetup.x = 0;
	viewSetup.y = 0;
	viewSetup.width = width;
	viewSetup.height = height;
	viewSetup.fov = ScaleFOVByWidthRatio( viewSetup.fov, ( (float)width / (float)height ) / ( 4.0f / 3.0f ) );
	viewSetup.m_bRenderToSubrectOfLargerScreen = true;

	// draw out the scene
	// Don't draw the HUD or the viewmodel
	RenderView( viewSetup, VIEW_CLEAR_DEPTH | VIEW_CLEAR_COLOR, 0 );

	// get the data from the backbuffer and save to disk
	// bitmap bits
	unsigned char *pImage = ( unsigned char * )malloc( width * height * 3 );

	// Get Bits from the material system
	pRenderContext->ReadPixels( 0, 0, width, height, pImage, IMAGE_FORMAT_RGB888 );

	// Some stuff to be setup dependent on padded vs. not padded
	int nSrcWidth, nSrcHeight;
	unsigned char *pSrcImage;

	// Create a padded version if necessary
	unsigned char *pPaddedImage = NULL;
	if ( bCreatePowerOf2Padded )
	{
		// Setup dimensions as needed
		int nPaddedWidth = SmallestPowerOfTwoGreaterOrEqual( width );
		int nPaddedHeight = SmallestPowerOfTwoGreaterOrEqual( height );

		// Allocate
		int nPaddedImageSize = nPaddedWidth * nPaddedHeight * 3;
		pPaddedImage = ( unsigned char * )malloc( nPaddedImageSize );
		
		// Zero out the entire thing
		V_memset( pPaddedImage, 255, nPaddedImageSize );

		// Copy over each row individually
		for ( int nRow = 0; nRow < height; ++nRow )
		{
			unsigned char *pDst = pPaddedImage + 3 * ( nRow * nPaddedWidth );
			const unsigned char *pSrc = pImage + 3 * ( nRow * width );
			V_memcpy( pDst, pSrc, 3 * width );
		}

		// Setup source data
		nSrcWidth = nPaddedWidth;
		nSrcHeight = nPaddedHeight;
		pSrcImage = pPaddedImage;
	}
	else
	{
		// Use non-padded info
		nSrcWidth = width;
		nSrcHeight = height;
		pSrcImage = pImage;
	}

	// allocate a buffer to write the tga into
	CUtlBuffer buffer;

	bool bWriteResult;
	if ( bWriteVTF )
	{
		// Create and initialize a VTF texture
		IVTFTexture *pVTFTexture = CreateVTFTexture();
		const int nFlags = TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_SRGB;
		if ( pVTFTexture->Init( nSrcWidth, nSrcHeight, 1, IMAGE_FORMAT_RGB888, nFlags, 1, 1 ) )
		{
			// Copy the image data over to the VTF
			unsigned char *pDestBits = pVTFTexture->ImageData();
			int nDstSize = nSrcWidth * nSrcHeight * 3;
			V_memcpy( pDestBits, pSrcImage, nDstSize );

			// Allocate output buffer
			int iMaxVTFSize = 1024 + ( nSrcWidth * nSrcHeight * 3 );
			void *pVTF = malloc( iMaxVTFSize );
			buffer.SetExternalBuffer( pVTF, iMaxVTFSize, 0 );

			// Serialize to the buffer
			bWriteResult = pVTFTexture->Serialize( buffer );
		
			// Free the VTF texture
			DestroyVTFTexture( pVTFTexture );
		}
		else
		{
			bWriteResult = false;
		}
	}
	else
	{
		// Write TGA format to buffer
		int iMaxTGASize = 1024 + ( nSrcWidth * nSrcHeight * 4 );
		void *pTGA = malloc( iMaxTGASize );
		buffer.SetExternalBuffer( pTGA, iMaxTGASize, 0 );

		bWriteResult = TGAWriter::WriteToBuffer( pSrcImage, buffer, nSrcWidth, nSrcHeight, IMAGE_FORMAT_RGB888, IMAGE_FORMAT_RGB888 );
	}

	if ( !bWriteResult )
	{
		Error( "Couldn't write bitmap data snapshot.\n" );
	}
	
	free( pImage );
	free( pPaddedImage );

	// async write to disk (this will take ownership of the memory)
	char szPathedFileName[_MAX_PATH];
	Q_snprintf( szPathedFileName, sizeof(szPathedFileName), "//MOD/%s", pFilename );

	filesystem->AsyncWrite( szPathedFileName, buffer.Base(), buffer.TellPut(), true );

	// restore our previous state
	pRenderContext->PopRenderTargetAndViewport();
	
	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PopMatrix();
	
	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PopMatrix();

	g_bRenderingScreenshot = false;
}

//-----------------------------------------------------------------------------
// Purpose: takes a screenshot of the save game
//-----------------------------------------------------------------------------
void CViewRender::WriteSaveGameScreenshot( const char *pFilename )
{
	WriteSaveGameScreenshotOfSize( pFilename, SAVEGAME_SCREENSHOT_WIDTH, SAVEGAME_SCREENSHOT_HEIGHT );
}


float ScaleFOVByWidthRatio( float fovDegrees, float ratio )
{
	float halfAngleRadians = fovDegrees * ( 0.5f * M_PI / 180.0f );
	float t = tan( halfAngleRadians );
	t *= ratio;
	float retDegrees = ( 180.0f / M_PI ) * atan( t );
	return retDegrees * 2.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Sets view parameters for level overview mode
// Input  : *rect - 
//-----------------------------------------------------------------------------
void CViewRender::SetUpOverView()
{
	static int oldCRC = 0;

	m_View.m_bOrtho = true;

	float aspect = (float)m_View.width/(float)m_View.height;

	int size_y = 1024.0f * cl_leveloverview.GetFloat(); // scale factor, 1024 = OVERVIEW_MAP_SIZE
	int	size_x = size_y * aspect;	// standard screen aspect 

	m_View.origin.x -= size_x / 2;
	m_View.origin.y += size_y / 2;

	m_View.m_OrthoLeft   = 0;
	m_View.m_OrthoTop    = -size_y;
	m_View.m_OrthoRight  = size_x;
	m_View.m_OrthoBottom = 0;

	m_View.angles = QAngle( 90, 90, 0 );

	// simple movement detector, show position if moved
	int newCRC = m_View.origin.x + m_View.origin.y + m_View.origin.z;
	if ( newCRC != oldCRC )
	{
		Msg( "Overview: scale %.2f, pos_x %.0f, pos_y %.0f\n", cl_leveloverview.GetFloat(),
			m_View.origin.x, m_View.origin.y );
		oldCRC = newCRC;
	}

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->ClearColor4ub( 0, 255, 0, 255 );

	// render->DrawTopView( true );
}

//-----------------------------------------------------------------------------
// Purpose: Render current view into specified rectangle
// Input  : *rect - is computed by CVideoMode_Common::GetClientViewRect()
//-----------------------------------------------------------------------------
void CViewRender::Render( vrect_t *rect )
{
	Assert(s_DbgSetupOrigin == m_View.origin);
	Assert(s_DbgSetupAngles == m_View.angles);

	VPROF_BUDGET( "CViewRender::Render", "CViewRender::Render" );
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s", __FUNCTION__ );

	vrect_t vr = *rect;

	// Stub out the material system if necessary.
	CMatStubHandler matStub;

    // Reload the 2D skybox in case sv_skyname was changed
    if (R_LoadSkys)
    {
        (R_LoadSkys)();
    }

	engine->EngineStats_BeginFrame();
	
	// Assume normal vis
	m_bForceNoVis			= false;
	
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();


    // Set for console commands, etc.
    render->SetMainView ( m_View.origin, m_View.angles );

	#if 0 && defined( CSTRIKE_DLL )
		const bool bPlayingBackReplay = g_pEngineClientReplay && g_pEngineClientReplay->IsPlayingReplayDemo();
		if ( pPlayer && !bPlayingBackReplay )
		{
			C_BasePlayer *pViewTarget = pPlayer;

			if ( pPlayer->IsObserver() && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
			{
				pViewTarget = dynamic_cast<C_BasePlayer*>( pPlayer->GetObserverTarget() );
			}

			if ( pViewTarget )
			{
				float targetFOV = (float)pViewTarget->m_iFOV;

				if ( targetFOV == 0 )
				{
					// FOV of 0 means use the default FOV
					targetFOV = g_pGameRules->DefaultFOV();
				}

				float deltaFOV = view.fov - m_flLastFOV;
				float FOVDirection = targetFOV - pViewTarget->m_iFOVStart;

				// Clamp FOV changes to stop FOV oscillation
				if ( ( deltaFOV < 0.0f && FOVDirection > 0.0f ) ||
					( deltaFOV > 0.0f && FOVDirection < 0.0f ) )
				{
					view.fov = m_flLastFOV;
				}

				// Catch case where FOV overshoots its target FOV
				if ( ( view.fov < targetFOV && FOVDirection <= 0.0f ) ||
					( view.fov > targetFOV && FOVDirection >= 0.0f ) )
				{
					view.fov = targetFOV;
				}

				m_flLastFOV = view.fov;
			}
		}
	#endif

	static ConVarRef sv_restrict_aspect_ratio_fov( "sv_restrict_aspect_ratio_fov" );
	float aspectRatio = engine->GetScreenAspectRatio() * 0.75f;	 // / (4/3)
	float limitedAspectRatio = aspectRatio;
	if ( ( sv_restrict_aspect_ratio_fov.GetInt() > 0 && engine->IsWindowedMode() && gpGlobals->maxClients > 1 ) ||
		sv_restrict_aspect_ratio_fov.GetInt() == 2 )
	{
		limitedAspectRatio = MIN( aspectRatio, 1.85f * 0.75f ); // cap out the FOV advantage at a 1.85:1 ratio (about the widest any legit user should be)
	}

	m_View.fov = ScaleFOVByWidthRatio(m_View.fov, limitedAspectRatio );
	m_View.fovViewmodel = ScaleFOVByWidthRatio(m_View.fovViewmodel, aspectRatio );

	// Let the client mode hook stuff.
	g_pClientMode->PreRender(&m_View);

	g_pClientMode->AdjustEngineViewport( vr.x, vr.y, vr.width, vr.height );

	ToolFramework_AdjustEngineViewport( vr.x, vr.y, vr.width, vr.height );

	float flViewportScale = mat_viewportscale.GetFloat();

	m_View.m_nUnscaledX = vr.x;
	m_View.m_nUnscaledY = vr.y;
	m_View.m_nUnscaledWidth = vr.width;
	m_View.m_nUnscaledHeight = vr.height;

#if 0
	// Good test mode for debugging viewports that are not full-size.
	view.width = vr.width * flViewportScale * 0.75f;
	view.height = vr.height * flViewportScale * 0.75f;
	view.x = vr.x + view.width * 0.10f;
	view.y = vr.y + view.height * 0.20f;
#else
	m_View.x = vr.x * flViewportScale;
	m_View.y = vr.y * flViewportScale;
	m_View.width = vr.width * flViewportScale;
	m_View.height = vr.height * flViewportScale;
#endif
	float engineAspectRatio = engine->GetScreenAspectRatio();
	m_View.m_flAspectRatio = (engineAspectRatio > 0.0f) ? engineAspectRatio : ((float)m_View.width / (float)m_View.height);

	// if we still don't have an aspect ratio, compute it from the view size
	if(m_View.m_flAspectRatio <= 0.f )
		m_View.m_flAspectRatio	= (float)m_View.width / (float)m_View.height;

	int nClearFlags = VIEW_CLEAR_DEPTH | VIEW_CLEAR_STENCIL;

	if( gl_clear_randomcolor.GetBool() )
	{
		CMatRenderContextPtr pRenderContext( materials );
		pRenderContext->ClearColor3ub( rand()%256, rand()%256, rand()%256 );
		pRenderContext->ClearBuffers( true, false, false );
		pRenderContext->Release();
	}
	else if ( gl_clear.GetBool() )
	{
		nClearFlags |= VIEW_CLEAR_COLOR;
	}
	else if ( IsPosix() )
	{
		MaterialAdapterInfo_t adapterInfo;
		materials->GetDisplayAdapterInfo( materials->GetCurrentAdapter(), adapterInfo );

		// On Posix, on ATI, we always clear color if we're antialiasing
		if ( adapterInfo.m_VendorID == 0x1002 )
		{
			if ( g_pMaterialSystem->GetCurrentConfigForVideoCard().m_nAASamples > 0 )
			{
				nClearFlags |= VIEW_CLEAR_COLOR;
			}
		}
	}

	// Determine if we should draw view model ( client mode override )
	bool drawViewModel = g_pClientMode->ShouldDrawViewModel();

	if ( cl_leveloverview.GetFloat() > 0 )
	{
		SetUpOverView();		
		nClearFlags |= VIEW_CLEAR_COLOR;
		drawViewModel = false;
	}

	// Apply any player specific overrides
	if ( pPlayer )
	{
		// Override view model if necessary
		if ( !pPlayer->m_Local.m_bDrawViewmodel )
		{
			drawViewModel = false;
		}
	}

	int flags = RENDERVIEW_DRAWHUD;

	if ( drawViewModel )
	{
		flags |= RENDERVIEW_DRAWVIEWMODEL;
	}

	RenderView(m_View, nClearFlags, flags );

	g_pClientMode->PostRender();
	engine->EngineStats_EndFrame();

	// Stop stubbing the material system so we can see the budget panel
	matStub.End();

	// Draw all of the UI stuff "fullscreen"
    // (this is not health, ammo, etc. Nor is it pre-game briefing interface stuff - this is the stuff that appears when you hit Esc in-game)
	{
		CViewSetup view2d;
		view2d.x				= rect->x;
		view2d.y				= rect->y;
		view2d.width			= rect->width;
		view2d.height			= rect->height;

		render->Push2DView( view2d, 0, NULL, GetFrustum() );
		render->VGui_Paint( PAINT_UIPANELS | PAINT_CURSOR );
		render->PopView( GetFrustum() );
	}
}

static void GetPos( const CCommand &args, Vector &vecOrigin, QAngle &angles )
{
	vecOrigin = MainViewOrigin();
	angles = MainViewAngles();
	if ( args.ArgC() == 2 && atoi( args[1] ) == 2 )
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pPlayer )
		{
			vecOrigin = pPlayer->GetAbsOrigin();
			angles = pPlayer->GetAbsAngles();
		}
	}
}

CON_COMMAND( spec_pos, "dump position and angles to the console" )
{
	Vector vecOrigin;
	QAngle angles;
	GetPos( args, vecOrigin, angles );
	Warning( "spec_goto %.1f %.1f %.1f %.1f %.1f\n", vecOrigin.x, vecOrigin.y, 
		vecOrigin.z, angles.x, angles.y );
}

CON_COMMAND( getpos, "dump position and angles to the console" )
{
	Vector vecOrigin;
	QAngle angles;
	GetPos( args, vecOrigin, angles );

	const char *pCommand1 = "setpos";
	const char *pCommand2 = "setang";
	if ( args.ArgC() == 2 && atoi( args[1] ) == 2 )
	{
		pCommand1 = "setpos_exact";
		pCommand2 = "setang_exact";
	}

	Warning( "%s %f %f %f;", pCommand1, vecOrigin.x, vecOrigin.y, vecOrigin.z );
	Warning( "%s %f %f %f\n", pCommand2, angles.x, angles.y, angles.z );
}

