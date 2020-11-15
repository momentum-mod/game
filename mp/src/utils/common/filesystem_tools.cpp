//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#if defined( _WIN32 )
#include <windows.h>
#include <direct.h>
#include <io.h> // _chmod
#elif _LINUX
#include <unistd.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include "tier1/strtools.h"
#include "filesystem_tools.h"
#include "tier0/icommandline.h"
#include "KeyValues.h"
#include "tier2/tier2.h"
#include "tier1/utlbuffer.h"
#include "tier1/fmtstr.h"

#ifdef MPI
	#include "vmpi.h"
	#include "vmpi_tools_shared.h"
	#include "vmpi_filesystem.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>


// ---------------------------------------------------------------------------------------------------- //
// Module interface.
// ---------------------------------------------------------------------------------------------------- //

IBaseFileSystem *g_pFileSystem = NULL;

// These are only used for tools that need the search paths that the engine's file system provides.
CSysModule			*g_pFullFileSystemModule = NULL;

// ---------------------------------------------------------------------------
//
// These are the base paths that everything will be referenced relative to (textures especially)
// All of these directories include the trailing slash
//
// ---------------------------------------------------------------------------

// This is the the path of the initial source file (relative to the cwd)
char		qdir[1024];

// This is the base engine + mod-specific game dir (e.g. "c:\tf2\mytfmod\")
char		gamedir[1024];	

class CSteamGameDB
{
public:
	CSteamGameDB()
	{
		HKEY steam;
		if ( RegOpenKeyExA( HKEY_LOCAL_MACHINE, R"(SOFTWARE\Valve\Steam)", 0, KEY_QUERY_VALUE, &steam ) != ERROR_SUCCESS )
			return;

		char steamLocation[MAX_PATH*2];
		DWORD dwSize = sizeof( steamLocation );
		if ( RegQueryValueExA( steam, "InstallPath", NULL, NULL, (LPBYTE)steamLocation, &dwSize ) != ERROR_SUCCESS )
			return;

		RegCloseKey( steam );

		libFolders.AddToTail( steamLocation );

		V_strcat_safe( steamLocation, R"(\steamapps\libraryfolders.vdf)" );

		{
			CUtlBuffer lib( 0, 0, CUtlBuffer::TEXT_BUFFER );
			if ( !g_pFullFileSystem->ReadFile( steamLocation, nullptr, lib ) )
				return;

			KeyValuesAD libFolder( "LibraryFolders" );
			libFolder->UsesEscapeSequences( true );
			libFolder->UsesConditionals( false );
			if ( !libFolder->LoadFromBuffer( "LibraryFolders", lib ) )
				return;

			FOR_EACH_SUBKEY( libFolder, folder )
			{
				auto name = folder->GetName();
				if ( !V_stricmp( name, "TimeNextStatsReport" ) || !V_stricmp( name, "ContentStatsID" ) )
					continue;
				if ( auto p = folder->GetString() )
					libFolders.AddToTail( p );
			}
		}

		for ( auto& folder : libFolders )
			folder = CUtlString::PathJoin( folder, "steamapps" );

		for ( const auto& folder : libFolders )
		{
			g_pFullFileSystem->AddSearchPath( folder, "__HAMMER_HACK__" );

			FileFindHandle_t h = 0;
			auto manifest = g_pFullFileSystem->FindFirstEx( "appmanifest_*.acf", "__HAMMER_HACK__", &h );
			while ( manifest )
			{
				games.AddToTail( Game{ static_cast<uint32>( V_atoi64( manifest + 12 ) ), CUtlString::PathJoin( folder, manifest ), &folder } );
				manifest = g_pFullFileSystem->FindNext( h );
			}

			g_pFullFileSystem->FindClose( h );

			g_pFullFileSystem->RemoveSearchPath( folder, "__HAMMER_HACK__" );
		}

		for ( auto& folder : libFolders )
			folder = CUtlString::PathJoin( folder, "common" );

		games.Sort( []( const Game* a, const Game* b ) { return int( a->appid ) - int( b->appid ); } );
	}

	bool GetAppInstallDir( uint32 appid, CUtlString& path ) const
	{
		const auto game = games.FindMatch( [appid]( const Game& g ) { return g.appid == appid; } );
		if ( !games.IsValidIndex( game ) )
			return false;

		CUtlBuffer b( 0, 0, CUtlBuffer::TEXT_BUFFER );
		if ( !g_pFullFileSystem->ReadFile( games[game].manifest, nullptr, b ) )
			return false;

		auto f = b.String();
		auto instD = V_stristr( f, "installdir" );
		if ( !instD )
			return false;

		auto dirStart = strchr( instD + ARRAYSIZE( "installdir" ), '"' );
		auto dirEnd = strchr( dirStart + 1, '"' );

		path = CUtlString::PathJoin( *games[game].library, CUtlString( dirStart + 1, dirEnd - dirStart - 1 ) );
		return true;
	}

private:
	struct Game
	{
		uint32 appid;
		CUtlString manifest;
		const CUtlString* library;
	};
	CUtlVector<CUtlString> libFolders;
	CUtlVector<Game> games;
};

void FileSystem_SetupStandardDirectories( const char *pFilename, const char *pGameInfoPath )
{
	// Set qdir.
	if ( !pFilename )
	{
		pFilename = ".";
	}

	Q_MakeAbsolutePath( qdir, sizeof( qdir ), pFilename, NULL );
	Q_StripFilename( qdir );
	Q_strlower( qdir );
	if ( qdir[0] != 0 )
	{
		Q_AppendSlash( qdir, sizeof( qdir ) );
	}

	// Set gamedir.
	Q_MakeAbsolutePath( gamedir, sizeof( gamedir ), pGameInfoPath );
	Q_AppendSlash( gamedir, sizeof( gamedir ) );

	if (!g_pFullFileSystem)
		return;

	KeyValuesAD pkv("gameinfo.txt");
	if (!pkv->LoadFromFile(g_pFullFileSystem, CFmtStrN<MAX_PATH>( "%s%cgameinfo.txt", pGameInfoPath, CORRECT_PATH_SEPARATOR ), nullptr))
		return;

	auto mounts = pkv->FindKey("mount");
	if (!mounts)
		return;

	const CSteamGameDB steamDB;

	CUtlVector<CUtlString> dirs;
	FOR_EACH_TRUE_SUBKEY(mounts, pMount)
	{
		CUtlString path;
		const auto appId = static_cast<uint32>(V_atoui64(pMount->GetName()));
		if (!steamDB.GetAppInstallDir(appId, path))
		{
			Warning("AppId %lu not installed!\n", appId);
			continue;
		}
		const SearchPathAdd_t head = pMount->GetBool("head") ? PATH_ADD_TO_HEAD : PATH_ADD_TO_TAIL;
		FOR_EACH_TRUE_SUBKEY(pMount, pModDir)
		{
			const char *const modDir = pModDir->GetName();
			const CFmtStr mod("%s" CORRECT_PATH_SEPARATOR_S "%s", path.Get(), modDir);
			dirs.AddToTail(mod.Get());
			FOR_EACH_VALUE(pModDir, pPath)
			{
				const char *const keyName = pPath->GetName();
				if (V_stricmp(keyName, "vpk") == 0)
				{
					const CFmtStr file("%s" CORRECT_PATH_SEPARATOR_S "%s.vpk", mod.Get(), pPath->GetString());
					g_pFullFileSystem->AddSearchPath(file, "GAME", head);
				}
				else if (V_stricmp(keyName, "dir") == 0)
				{
					const CFmtStr folder("%s" CORRECT_PATH_SEPARATOR_S "%s", mod.Get(), pPath->GetString());
					g_pFullFileSystem->AddSearchPath(folder, "GAME", head);
				}
				else
					Warning("Unknown key \"%s\" in mounts\n", keyName);
			}
		}
	}

	for (const auto &dir : dirs)
	{
		g_pFullFileSystem->AddSearchPath(dir, "GAME");
	}
}


bool FileSystem_Init_Normal( const char *pFilename, FSInitType_t initType, bool bOnlyUseDirectoryName )
{
	if ( initType == FS_INIT_FULL )
	{
		// First, get the name of the module
		char fileSystemDLLName[MAX_PATH];
		bool bSteam;
		if ( FileSystem_GetFileSystemDLLName( fileSystemDLLName, MAX_PATH, bSteam ) != FS_OK )
			return false;

		// Next, load the module, call Connect/Init.
		CFSLoadModuleInfo loadModuleInfo;
		loadModuleInfo.m_pFileSystemDLLName = fileSystemDLLName;
		loadModuleInfo.m_pDirectoryName = pFilename;
		loadModuleInfo.m_bOnlyUseDirectoryName = bOnlyUseDirectoryName;
		loadModuleInfo.m_ConnectFactory = Sys_GetFactoryThis();
		loadModuleInfo.m_bSteam = bSteam;
		loadModuleInfo.m_bToolsMode = true;
		if ( FileSystem_LoadFileSystemModule( loadModuleInfo ) != FS_OK )
			return false;

		// Next, mount the content
		CFSMountContentInfo mountContentInfo;
		mountContentInfo.m_pDirectoryName=  loadModuleInfo.m_GameInfoPath;
		mountContentInfo.m_pFileSystem = loadModuleInfo.m_pFileSystem;
		mountContentInfo.m_bToolsMode = true;
		if ( FileSystem_MountContent( mountContentInfo ) != FS_OK )
			return false;
		
		// Finally, load the search paths.
		CFSSearchPathsInit searchPathsInit;
		searchPathsInit.m_pDirectoryName = loadModuleInfo.m_GameInfoPath;
		searchPathsInit.m_pFileSystem = loadModuleInfo.m_pFileSystem;
		if ( FileSystem_LoadSearchPaths( searchPathsInit ) != FS_OK )
			return false;

		// Store the data we got from filesystem_init.
		g_pFileSystem = g_pFullFileSystem = loadModuleInfo.m_pFileSystem;
		g_pFullFileSystemModule = loadModuleInfo.m_pModule;

		FileSystem_AddSearchPath_Platform( g_pFullFileSystem, loadModuleInfo.m_GameInfoPath );

		FileSystem_SetupStandardDirectories( pFilename, loadModuleInfo.m_GameInfoPath );
	}
	else
	{
		if ( !Sys_LoadInterface(
			"filesystem_stdio",
			FILESYSTEM_INTERFACE_VERSION,
			&g_pFullFileSystemModule,
			(void**)&g_pFullFileSystem ) )
		{
			return false;
		}

		if ( g_pFullFileSystem->Init() != INIT_OK )
			return false;

		g_pFullFileSystem->RemoveAllSearchPaths();
		g_pFullFileSystem->AddSearchPath( "../platform", "PLATFORM" );
		g_pFullFileSystem->AddSearchPath( ".", "GAME" );

		g_pFileSystem = g_pFullFileSystem;
	}

	return true;
}


bool FileSystem_Init( const char *pBSPFilename, int maxMemoryUsage, FSInitType_t initType, bool bOnlyUseFilename )
{
	Assert( CommandLine()->GetCmdLine() != NULL ); // Should have called CreateCmdLine by now.

	// If this app uses VMPI, then let VMPI intercept all filesystem calls.
#if defined( MPI )
	if ( g_bUseMPI )
	{
		if ( g_bMPIMaster )
		{
			if ( !FileSystem_Init_Normal( pBSPFilename, initType, bOnlyUseFilename ) )
				return false;

			g_pFileSystem = g_pFullFileSystem = VMPI_FileSystem_Init( maxMemoryUsage, g_pFullFileSystem );
			SendQDirInfo();
		}
		else
		{
			g_pFileSystem = g_pFullFileSystem = VMPI_FileSystem_Init( maxMemoryUsage, NULL );
			RecvQDirInfo();
		}
		return true;
	}
#endif

	return FileSystem_Init_Normal( pBSPFilename, initType, bOnlyUseFilename );
}


void FileSystem_Term()
{
#if defined( MPI )
	if ( g_bUseMPI )
	{
		g_pFileSystem = g_pFullFileSystem = VMPI_FileSystem_Term();
	}
#endif

	if ( g_pFullFileSystem )
	{
		g_pFullFileSystem->Shutdown();
		g_pFullFileSystem = NULL;
		g_pFileSystem = NULL;
	}

	if ( g_pFullFileSystemModule )
	{
		Sys_UnloadModule( g_pFullFileSystemModule );
		g_pFullFileSystemModule = NULL;
	}
}


CreateInterfaceFn FileSystem_GetFactory()
{
#if defined( MPI )
	if ( g_bUseMPI )
		return VMPI_FileSystem_GetFactory();
#endif
	return Sys_GetFactory( g_pFullFileSystemModule );
}
