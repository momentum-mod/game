#include "cbase.h"
#include "rml_system_interface.h"

#include "cdll_client_int.h"
#include "mom_shareddefs.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"
#include "vgui/IInput.h"
#include "vgui/Cursor.h"

double RmlSystemInterface::GetElapsedTime() { return gpGlobals->curtime; }

bool RmlSystemInterface::LogMessage(Rml::Core::Log::Type type, const Rml::Core::String &message)
{
    switch (type)
    {
    case Rml::Core::Log::LT_INFO:
    case Rml::Core::Log::LT_DEBUG:
        DevLog(message.CString());
        break;
    case Rml::Core::Log::LT_ALWAYS:
        Log(message.CString());
        break;
    case Rml::Core::Log::LT_WARNING:
        Warning(message.CString());
        break;
    case Rml::Core::Log::LT_ASSERT:
    case Rml::Core::Log::LT_ERROR:
        Error(message.CString());
        break;
    }

    return true;
}