//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "qlimits.h"
#include "baseautocompletefilelist.h"
#include "utlsymbol.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Fills in a list of commands based on specified subdirectory and extension into the format:
//  commandname subdir/filename.ext
//  commandname subdir/filename2.ext
// Returns number of files in list for autocompletion
//-----------------------------------------------------------------------------
int CBaseAutoCompleteFileList::AutoCompletionFunc( char const *partial, char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	char const *cmdname = m_pszCommandName;

	char *substring = (char *)partial;
	if ( Q_strstr( partial, cmdname ) )
	{
		substring = (char *)partial + strlen( cmdname ) + 1;
	}

	// Search the directory structure.
	char searchpath[MAX_QPATH];
	if ( m_pszSubDir && m_pszSubDir[0] && Q_strcasecmp( m_pszSubDir, "NULL" ) )
	{
		Q_snprintf(searchpath,sizeof(searchpath),"%s/*.%s", m_pszSubDir, m_pszExtension );
	}
	else
	{
		Q_snprintf(searchpath,sizeof(searchpath),"*.%s", m_pszExtension );
	}

	CUtlSymbolTable entries( 0, 0, true );
	CUtlVector< CUtlSymbol > symbols;
    FileFindHandle_t hfind = FILESYSTEM_INVALID_FIND_HANDLE;
    const char* findfn = g_pFullFileSystem->FindFirst(searchpath, &hfind);

    const size_t pSubstring = strlen(substring);

	while ( findfn )
	{
		char sz[ MAX_QPATH ];
		Q_snprintf( sz, sizeof( sz ), "%s", findfn );

		bool add = false;
		// Insert into lookup
		if ( substring[0] )
		{
            if (!Q_strncasecmp(findfn, substring, pSubstring))
			{
				add = true;
			}
		}
		else
		{
			add = true;
		}

		if ( add )
		{
			CUtlSymbol sym = entries.AddString( findfn );

			int idx = symbols.Find( sym );
			if ( idx == symbols.InvalidIndex() )
			{
				symbols.AddToTail( sym );
			}
		}

        findfn = g_pFullFileSystem->FindNext(hfind);

		// Too many
		if ( symbols.Count() >= COMMAND_COMPLETION_MAXITEMS )
			break;
	}

    g_pFullFileSystem->FindClose(hfind);
    hfind = FILESYSTEM_INVALID_FIND_HANDLE;

	for ( int i = 0; i < symbols.Count(); i++ )
	{
		char const *filename = entries.String( symbols[ i ] );

		Q_snprintf( commands[ i ], sizeof( commands[ i ] ), "%s %s", cmdname, filename );
		// Remove ".(extension)"
		commands[ i ][ Q_strlen( commands[ i ] ) - (Q_strlen(m_pszExtension) + 1)] = 0;
	}

	return symbols.Count();
}
