//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose: General matching functions for things like wildcards and !=.
//
// $NoKeywords: $
//=============================================================================//

#pragma once

// Compares with != and the like. Basically hijacks the response system matching.
// This also loops back around to Matcher_NamesMatch.
// pszQuery = The value that should have the operator(s) at the beginning.
// szValue = The value tested against the criterion.
bool Matcher_Match( const char *pszQuery, const char *szValue );
bool Matcher_Match( const char *pszQuery, int iValue );
bool Matcher_Match( const char *pszQuery, float flValue );

// Regular expressions based off of the std library.
// pszQuery = The regex text.
// szValue = The value that should be matched.
bool Matcher_Regex( const char *pszQuery, const char *szValue );

// Compares two strings with support for wildcards or regex. This code is an expanded version of baseentity.cpp's NamesMatch().
// pszQuery = The value that should have the wildcard.
// szValue = The value tested against the query.
// Use Matcher_Match if you want <, !=, etc. as well.
bool Matcher_NamesMatch( const char *pszQuery, const char *szValue );

// Identical to baseentity.cpp's original NamesMatch().
// pszQuery = The value that should have the wildcard.
// szValue = The value tested against the query.
bool Matcher_NamesMatch_Classic( const char *pszQuery, const char *szValue );

// Identical to Matcher_NamesMatch_Classic(), but either value could use a wildcard.
// pszQuery = The value that serves as the query. This value can use wildcards.
// szValue = The value tested against the query. This value can use wildcards as well.
bool Matcher_NamesMatch_MutualWildcard( const char *pszQuery, const char *szValue );

// Taken from the Response System.
// Checks if the specified string appears to be a number of some sort.
bool AppearsToBeANumber( char const *token );
