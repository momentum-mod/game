#pragma once

#include "platform.h"
#include <RmlUi/Core/SystemInterface.h>

class RmlSystemInterface : public Rml::Core::SystemInterface
{
  public:
    // Get the number of seconds elapsed since the start of the application.
    virtual double GetElapsedTime() OVERRIDE;

    // TODO: Handle localizations here
    // Translate the input string into the translated string.
    // virtual int TranslateString(Rml::Core::String &translated, const Rml::Core::String &input) OVERRIDE;

    // Log the specified message.
    virtual bool LogMessage(Rml::Core::Log::Type type, const Rml::Core::String &message) OVERRIDE;

    // TODO: Handle mouse cursor
    // Set the mouse cursor.
    // virtual void SetMouseCursor(const Rml::Core::String &cursor_name) OVERRIDE;
};