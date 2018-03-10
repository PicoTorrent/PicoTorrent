#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <memory>

class wxDataViewEvent;
class wxDataViewListCtrl;

namespace pt
{
    class Configuration;
    class Translator;

    class EditPresetsDialog : public wxDialog
    {
    public:
        EditPresetsDialog(
            wxWindow* parent,
            std::shared_ptr<Configuration> cfg,
            std::shared_ptr<Translator> translator);

    private:
        enum
        {
            ptID_PRESETS_LIST
        };

        void OnAdd(wxCommandEvent&);
        void OnRemove(wxCommandEvent&);
        void OnSelectionChanged(wxDataViewEvent&);

        wxDECLARE_EVENT_TABLE();

        std::shared_ptr<Configuration> m_cfg;
        std::shared_ptr<Translator> m_translator;

        wxButton* m_addButton;
        wxButton* m_closeButton;
        wxButton* m_removeButton;
        wxDataViewListCtrl* m_presetsList;
    };
}
