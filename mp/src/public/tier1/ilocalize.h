
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef TIER1_ILOCALIZE_H
#define TIER1_ILOCALIZE_H

#ifdef _WIN32
#pragma once
#endif

#include "appframework/IAppSystem.h"
#include <tier1/KeyValues.h>

// unicode character type
// for more unicode manipulation functions #include <wchar.h>
#if !defined(_WCHAR_T_DEFINED) && !defined(GNUC)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

class CLocalizedStringArg;

// direct references to localized strings
typedef unsigned long StringIndex_t;
const unsigned long INVALID_LOCALIZE_STRING_INDEX = (StringIndex_t) -1;

//-----------------------------------------------------------------------------
// Purpose: Handles localization of text
//			looks up string names and returns the localized unicode text
//-----------------------------------------------------------------------------
abstract_class ILocalize
{
public:
	// adds the contents of a file to the localization table
	virtual bool AddFile( const char *fileName, const char *pPathID = NULL, bool bIncludeFallbackSearchPaths = false ) = 0;

	// Remove all strings from the table
	virtual void RemoveAll() = 0;

	// Finds the localized text for tokenName
	virtual wchar_t *Find(char const *tokenName) = 0;

	// finds the index of a token by token name, INVALID_STRING_INDEX if not found
	virtual StringIndex_t FindIndex(const char *tokenName) = 0;

	// gets the values by the string index
	virtual const char *GetNameByIndex(StringIndex_t index) = 0;
	virtual wchar_t *GetValueByIndex(StringIndex_t index) = 0;

	///////////////////////////////////////////////////////////////////
	// the following functions should only be used by localization editors

	// iteration functions
	virtual StringIndex_t GetFirstStringIndex() = 0;
	// returns the next index, or INVALID_STRING_INDEX if no more strings available
	virtual StringIndex_t GetNextStringIndex(StringIndex_t index) = 0;

	// adds a single name/unicode string pair to the table
	virtual void AddString( const char *tokenName, wchar_t *unicodeString, const char *fileName ) = 0;

	// changes the value of a string
	virtual void SetValueByIndex(StringIndex_t index, wchar_t *newValue) = 0;

	// saves the entire contents of the token tree to the file
	virtual bool SaveToFile( const char *fileName ) = 0;

	// iterates the filenames
	virtual int GetLocalizationFileCount() = 0;
	virtual const char *GetLocalizationFileName(int index) = 0;

	// returns the name of the file the specified localized string is stored in
	virtual const char *GetFileNameByIndex(StringIndex_t index) = 0;

	// for development only, reloads localization files
	virtual void ReloadLocalizationFiles( ) = 0;

	virtual const char *FindAsUTF8( const char *pchTokenName ) = 0;

	// need to replace the existing ConstructString with this
	virtual void ConstructString(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const char *tokenName, KeyValues *localizationVariables) = 0;
	virtual void ConstructString(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, StringIndex_t unlocalizedTextSymbol, KeyValues *localizationVariables) = 0;

    template <size_t len>
    FORCEINLINE wchar_t *FindSafe(const char(&token)[len])
    {
        if (wchar_t *find = Find(token))
            return find;
        static wchar_t fallback[len];
        ConvertANSIToUnicode(token, fallback, sizeof(fallback));
        return fallback;
    }

	///////////////////////////////////////////////////////////////////
	// static interface

	// converts an english string to unicode
	// returns the number of wchar_t in resulting string, including null terminator
	static int ConvertANSIToUnicode(const char *ansi, OUT_Z_BYTECAP(unicodeBufferSizeInBytes) wchar_t *unicode, int unicodeBufferSizeInBytes);

    // Converts given multi byte character array to wchar, whatever that is on this system.
    // Output parameter *utf16 must be nullptr before getting passed in here as it will be allocated within.
    // Returns amount of characters that the *utf16 buffer contains, including null terminator, not size in bytes (which would be multiply sizeof(wchar_t)).
    static int ConvertUTF8ToUTF16(const char *utf8, wchar_t **utf16);

	// converts an unicode string to an english string
	// unrepresentable characters are converted to system default
	// returns the number of characters in resulting string, including null terminator
	static int ConvertUnicodeToANSI(const wchar_t *unicode, OUT_Z_BYTECAP(ansiBufferSize) char *ansi, int ansiBufferSize);

    // Converts from a wchar character array to a dynamic multi byte array.
    // Output parameter *utf8 must be nullptr before getting passed in here as it will be allocated within.
    // Returns amount of characters that the *utf8 buffer contains, including null terminator, not size in bytes.
    static int ConvertUTF16ToUTF8(const wchar_t *utf16, char **utf8);

	// builds a localized formatted string
	// uses the format strings first: %s1, %s2, ...  unicode strings (wchar_t *)
	template < typename T >
	static void ConstructString(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) T *unicodeOuput, int unicodeBufferSizeInBytes, const T *formatString, int numFormatParameters, ...)
	{
		va_list argList;
		va_start( argList, numFormatParameters );

		ConstructStringVArgsInternal( unicodeOuput, unicodeBufferSizeInBytes, formatString, numFormatParameters, argList );

		va_end( argList );
	}

	template < typename T >
	static void ConstructStringVArgs(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) T *unicodeOuput, int unicodeBufferSizeInBytes, const T *formatString, int numFormatParameters, va_list argList)
	{
		ConstructStringVArgsInternal( unicodeOuput, unicodeBufferSizeInBytes, formatString, numFormatParameters, argList );
	}

	template < typename T >
	static void ConstructStringArgs(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) T *unicodeOuput, int unicodeBufferSizeInBytes, const T *formatString, int numFormatParameters, const CLocalizedStringArg* argList)
	{
		ConstructStringArgsInternal( unicodeOuput, unicodeBufferSizeInBytes, formatString, numFormatParameters, argList );
	}

	template < typename T >
	static void ConstructString(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) T *unicodeOutput, int unicodeBufferSizeInBytes, const T *formatString, KeyValues *localizationVariables)
	{
		ConstructStringKeyValuesInternal( unicodeOutput, unicodeBufferSizeInBytes, formatString, localizationVariables );
	}

private:
	// internal "interface"
	static void ConstructStringVArgsInternal(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) char *unicodeOutput, int unicodeBufferSizeInBytes, const char *formatString, int numFormatParameters, va_list argList);
	static void ConstructStringArgsInternal(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) char *unicodeOutput, int unicodeBufferSizeInBytes, const char *formatString, int numFormatParameters, const CLocalizedStringArg *argList);
	static void ConstructStringVArgsInternal(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const wchar_t *formatString, int numFormatParameters, va_list argList);
	static void ConstructStringArgsInternal(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const wchar_t *formatString, int numFormatParameters, const CLocalizedStringArg *argList);

	static void ConstructStringKeyValuesInternal(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) char *unicodeOutput, int unicodeBufferSizeInBytes, const char *formatString, KeyValues *localizationVariables);
	static void ConstructStringKeyValuesInternal(OUT_Z_BYTECAP(unicodeBufferSizeInBytes) wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const wchar_t *formatString, KeyValues *localizationVariables);
};

#ifdef GC

	typedef char locchar_t;

	#define loc_snprintf	Q_snprintf
	#define loc_sprintf_safe V_sprintf_safe
	#define loc_sncat		Q_strncat
	#define loc_scat_safe	V_strcat_safe
	#define loc_sncpy		Q_strncpy
	#define loc_scpy_safe	V_strcpy_safe
	#define loc_strlen		Q_strlen
	#define LOCCHAR( x )	x

#else

	typedef wchar_t locchar_t;

	#define loc_snprintf	V_snwprintf
	#define loc_sprintf_safe V_swprintf_safe
	#define loc_sncat		V_wcsncat
	#define loc_scat_safe	V_wcscat_safe
	#define loc_sncpy		Q_wcsncpy
	#define loc_scpy_safe	V_wcscpy_safe
	#define loc_strlen		Q_wcslen
	#define LOCCHAR(x)		L ## x

#endif

// --------------------------------------------------------------------------
// Purpose:
// --------------------------------------------------------------------------

template < typename T >
class TypedKeyValuesStringHelper
{
public:
	static const T *Read( KeyValues *pKeyValues, const char *pKeyName, const T *pDefaultValue );
	static void	Write( KeyValues *pKeyValues, const char *pKeyName, const T *pValue );
};

// --------------------------------------------------------------------------

template < >
class TypedKeyValuesStringHelper<char>
{
public:
	static const char *Read( KeyValues *pKeyValues, const char *pKeyName, const char *pDefaultValue ) { return pKeyValues->GetString( pKeyName, pDefaultValue ); }
	static void Write( KeyValues *pKeyValues, const char *pKeyName, const char *pValue ) { pKeyValues->SetString( pKeyName, pValue ); }
};

// --------------------------------------------------------------------------

template < >
class TypedKeyValuesStringHelper<wchar_t>
{
public:
	static const wchar_t *Read( KeyValues *pKeyValues, const char *pKeyName, const wchar_t *pDefaultValue ) { return pKeyValues->GetWString( pKeyName, pDefaultValue ); }
	static void Write( KeyValues *pKeyValues, const char *pKeyName, const wchar_t *pValue ) { pKeyValues->SetWString( pKeyName, pValue ); }
};

// --------------------------------------------------------------------------
// Purpose: CLocalizedStringArg<> is a class that will take a variable of any
//			arbitrary type and convert it to a string of whatever character type
//			we're using for localization (locchar_t).
//
//			Independently it isn't very useful, though it can be used to sort-of-
//			intelligently fill out the correct format string. It's designed to be
//			used for the arguments of CConstructLocalizedString, which can be of
//			arbitrary number and type.
//
//			If you pass in a (non-specialized) pointer, the code will assume that
//			you meant that pointer to be used as a localized string. This will
//			still fail to compile if some non-string type is passed in, but will
//			handle weird combinations of const/volatile/whatever automatically.
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

#ifdef _WIN32
#define ___WIDECHAR_PRINT_FORMAT_WIDECHAR LOCCHAR("%s")
#define ___WIDECHAR_PRINT_FORMAT_ANSICHAR LOCCHAR("%S") // Thanks, microsoft
#elif defined(POSIX)
#define ___WIDECHAR_PRINT_FORMAT_WIDECHAR LOCCHAR("%ls")
#define ___WIDECHAR_PRINT_FORMAT_ANSICHAR LOCCHAR("%s")
#else
#warning "ILocalize needs some string macros defined!"
#endif

class CLocalizedStringArg
{
public:
	template <typename U, std::enable_if_t<std::conjunction_v<std::is_integral<U>, std::is_unsigned<U>, std::bool_constant<sizeof(U) <= 4>>, int> = 0>
	CLocalizedStringArg(U value) : CLocalizedStringArg(value, LOCCHAR("%u")) {}
	template <typename U, std::enable_if_t<std::conjunction_v<std::is_integral<U>, std::is_unsigned<U>, std::bool_constant<sizeof(U) == 8>>, int> = 1>
	CLocalizedStringArg(U value) : CLocalizedStringArg(value, LOCCHAR("%llu")) {}
	template <typename U, std::enable_if_t<std::conjunction_v<std::is_integral<U>, std::is_signed<U>, std::bool_constant<sizeof(U) <= 4>>, int> = 2>
	CLocalizedStringArg(U value) : CLocalizedStringArg(value, LOCCHAR("%d")) {}
	template <typename U, std::enable_if_t<std::conjunction_v<std::is_integral<U>, std::is_signed<U>, std::bool_constant<sizeof(U) == 8>>, int> = 3>
	CLocalizedStringArg(U value) : CLocalizedStringArg(value, LOCCHAR("%lld")) {}

	template <typename U, std::enable_if_t<std::is_enum_v<U>, int> = 4>
	CLocalizedStringArg(U value) : CLocalizedStringArg(static_cast<std::underlying_type_t<U>>(value)) {}

	template <typename U, std::enable_if_t<std::is_floating_point_v<U>, int> = 5>
	CLocalizedStringArg(U value) : CLocalizedStringArg(value, LOCCHAR("%.2f")) {}

	template <typename U, std::enable_if_t<std::disjunction_v<std::is_same<U, char*>, std::is_same<U, const char*>>, int> = 6>
	CLocalizedStringArg(U pszValue) : CLocalizedStringArg(pszValue, ___WIDECHAR_PRINT_FORMAT_ANSICHAR) {}
	template <typename U, std::enable_if_t<std::disjunction_v<std::is_same<U, wchar_t*>, std::is_same<U, const wchar_t*>>, int> = 7>
	CLocalizedStringArg(U pszValue) : CLocalizedStringArg(pszValue, ___WIDECHAR_PRINT_FORMAT_WIDECHAR) {}

public:
	const locchar_t *GetLocArg() const { return m_cBuffer; }

private:
	template <typename T> CLocalizedStringArg(T value, const locchar_t *loc_Format) { loc_snprintf(m_cBuffer, kBufferSize, loc_Format, value); }

	static constexpr int kBufferSize = 128;
	locchar_t m_cBuffer[kBufferSize];
};

// --------------------------------------------------------------------------
// Purpose:
// --------------------------------------------------------------------------
class CConstructLocalizedString
{
public:
	template < typename... T, typename = std::enable_if_t<sizeof...(T) != 0> >
	CConstructLocalizedString( const locchar_t *loc_Format, T... args )
	{
		m_loc_Buffer[0] = '\0';

		if ( loc_Format )
		{
			const CLocalizedStringArg locArgs[] = { args... };
			::ILocalize::ConstructStringArgs( m_loc_Buffer, sizeof( m_loc_Buffer ), loc_Format, ARRAYSIZE( locArgs ), locArgs );
		}
	}

	CConstructLocalizedString( const locchar_t *loc_Format, KeyValues *pKeyValues )
	{
		m_loc_Buffer[0] = '\0';

		if ( loc_Format && pKeyValues )
		{
			::ILocalize::ConstructString( m_loc_Buffer, sizeof( m_loc_Buffer ), loc_Format, pKeyValues );
		}
	}

	operator const locchar_t *() const
	{
		return m_loc_Buffer;
	}

private:
	enum { kBufferSize = 512, };
	locchar_t m_loc_Buffer[ kBufferSize ];
};

#endif // TIER1_ILOCALIZE_H
