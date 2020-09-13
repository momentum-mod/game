#pragma once

#include <vgui_controls/QueryBox.h>

// Popup dialog with a text entry, extending the QueryBox, which extends the MessageBox
// originally by Matthew D. Campbell (matt@turtlerockstudios.com), 2003
namespace vgui
{
	class TextEntryBox : public QueryBox
	{
	public:
		DECLARE_CLASS_SIMPLE(TextEntryBox, vgui::QueryBox);

		TextEntryBox(const char *title, const char *labelText, const char *entryText, bool isCvar, Panel *parent = nullptr);

		TextEntry *GetTextEntry() const { return m_pEntry; }
		bool IsCVarEntryBox() const { return m_pCvarEntry != nullptr; }

        void ShowWindow(Panel *pFrameOver = nullptr) override;

	protected:
		void OnCommand(const char *command) override;
        void PerformLayout() override;

	private:
        CvarTextEntry *m_pCvarEntry;
        TextEntry *m_pEntry;
	};
}