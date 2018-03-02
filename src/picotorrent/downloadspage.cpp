#include "downloadspage.hpp"

#include "config.hpp"
#include "translator.hpp"

#include <wx/filepicker.h>
#include <wx/statline.h>

using pt::DownloadsPage;

DownloadsPage::DownloadsPage(wxWindow* parent, std::shared_ptr<pt::Configuration> cfg, std::shared_ptr<pt::Translator> tran)
    : wxPanel(parent, wxID_ANY),
    m_cfg(cfg)
{
    wxStaticBoxSizer* transfersSizer = new wxStaticBoxSizer(wxVERTICAL, this, i18n(tran, "transfers"));
    wxFlexGridSizer* transfersGrid = new wxFlexGridSizer(2, 10, 10);

    m_savePathCtrl = new wxDirPickerCtrl(transfersSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDirSelectorPromptStr, wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE | wxDIRP_SMALL);
    m_savePathCtrl->SetPath(m_cfg->DefaultSavePath().string());

    transfersGrid->AddGrowableCol(1, 1);
    transfersGrid->Add(new wxStaticText(transfersSizer->GetStaticBox(), wxID_ANY, i18n(tran, "save_path")), 0, wxALIGN_CENTER_VERTICAL);
    transfersGrid->Add(m_savePathCtrl, 1, wxEXPAND);
    transfersSizer->Add(transfersGrid, 1, wxEXPAND | wxALL, 5);


    wxStaticBoxSizer* limitsSizer = new wxStaticBoxSizer(wxVERTICAL, this, i18n(tran, "limits"));

    /* Rate limits */
    wxFlexGridSizer* transferLimitsGrid = new wxFlexGridSizer(3, 10, 10);

    m_enableDownloadLimit = new wxCheckBox(limitsSizer->GetStaticBox(), wxID_ANY, i18n(tran, "dl_limit"));
    m_enableDownloadLimit->SetValue(m_cfg->Session()->EnableDownloadRateLimit());

    m_downloadLimit = new wxTextCtrl(limitsSizer->GetStaticBox(), wxID_ANY);
    m_downloadLimit->Enable(m_cfg->Session()->EnableDownloadRateLimit());
    m_downloadLimit->SetValidator(wxTextValidator(wxFILTER_DIGITS));
    m_downloadLimit->SetValue(wxString::Format("%i", m_cfg->Session()->DownloadRateLimit()));

    m_enableUploadLimit = new wxCheckBox(limitsSizer->GetStaticBox(), wxID_ANY, i18n(tran, "ul_limit"));
    m_enableUploadLimit->SetValue(m_cfg->Session()->EnableUploadRateLimit());

    m_uploadLimit = new wxTextCtrl(limitsSizer->GetStaticBox(), wxID_ANY);
    m_uploadLimit->Enable(m_cfg->Session()->EnableUploadRateLimit());
    m_uploadLimit->SetValidator(wxTextValidator(wxFILTER_DIGITS));
    m_uploadLimit->SetValue(wxString::Format("%i", m_cfg->Session()->UploadRateLimit()));

    m_enableDownloadLimit->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) { m_downloadLimit->Enable(m_enableDownloadLimit->GetValue()); });
    m_enableUploadLimit->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) { m_uploadLimit->Enable(m_enableUploadLimit->GetValue()); });

    transferLimitsGrid->AddGrowableCol(1, 1);
    transferLimitsGrid->Add(m_enableDownloadLimit, 0, wxALIGN_CENTER_VERTICAL);
    transferLimitsGrid->Add(m_downloadLimit, 0, wxALIGN_RIGHT);
    transferLimitsGrid->Add(new wxStaticText(limitsSizer->GetStaticBox(), wxID_ANY, "KB/s"), 0, wxALIGN_CENTER_VERTICAL);
    transferLimitsGrid->Add(m_enableUploadLimit, 0, wxALIGN_CENTER_VERTICAL);
    transferLimitsGrid->Add(m_uploadLimit, 0, wxALIGN_RIGHT);
    transferLimitsGrid->Add(new wxStaticText(limitsSizer->GetStaticBox(), wxID_ANY, "KB/s"), 0, wxALIGN_CENTER_VERTICAL);

    /* Active limits */
    wxFlexGridSizer* activeLimitsGrid = new wxFlexGridSizer(2, 10, 10);

    m_activeLimit = new wxTextCtrl(limitsSizer->GetStaticBox(), wxID_ANY);
    m_activeLimit->SetValidator(wxTextValidator(wxFILTER_DIGITS));
    m_activeLimit->SetValue(wxString::Format("%i", m_cfg->Session()->ActiveLimit()));

    m_activeDownloadsLimit = new wxTextCtrl(limitsSizer->GetStaticBox(), wxID_ANY);
    m_activeDownloadsLimit->SetValidator(wxTextValidator(wxFILTER_DIGITS));
    m_activeDownloadsLimit->SetValue(wxString::Format("%i", m_cfg->Session()->ActiveDownloads()));

    m_activeSeedsLimit = new wxTextCtrl(limitsSizer->GetStaticBox(), wxID_ANY);
    m_activeSeedsLimit->SetValidator(wxTextValidator(wxFILTER_DIGITS));
    m_activeSeedsLimit->SetValue(wxString::Format("%i", m_cfg->Session()->ActiveSeeds()));

    activeLimitsGrid->AddGrowableCol(0, 1);
    activeLimitsGrid->Add(new wxStaticText(limitsSizer->GetStaticBox(), wxID_ANY, i18n(tran, "total_active")));
    activeLimitsGrid->Add(m_activeLimit, 0, wxALIGN_RIGHT);
    activeLimitsGrid->Add(new wxStaticText(limitsSizer->GetStaticBox(), wxID_ANY, i18n(tran, "active_downloads")));
    activeLimitsGrid->Add(m_activeDownloadsLimit, 0, wxALIGN_RIGHT);
    activeLimitsGrid->Add(new wxStaticText(limitsSizer->GetStaticBox(), wxID_ANY, i18n(tran, "active_seeds")));
    activeLimitsGrid->Add(m_activeSeedsLimit, 0, wxALIGN_RIGHT);

    limitsSizer->Add(transferLimitsGrid, 0, wxEXPAND | wxALL, 5);
    limitsSizer->Add(new wxStaticLine(limitsSizer->GetStaticBox(), wxID_ANY), 0, wxEXPAND | wxALL, 5);
    limitsSizer->Add(activeLimitsGrid, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(transfersSizer, 0, wxEXPAND);
    sizer->AddSpacer(10);
    sizer->Add(limitsSizer, 0, wxEXPAND);
    sizer->AddStretchSpacer();

    this->SetSizerAndFit(sizer);
}

void DownloadsPage::ApplyConfiguration()
{
    long dlLimit = 0;
    m_downloadLimit->GetValue().ToLong(&dlLimit);

    long ulLimit = 0;
    m_uploadLimit->GetValue().ToLong(&ulLimit);

    m_cfg->DefaultSavePath(m_savePathCtrl->GetPath().ToStdString());
    m_cfg->Session()->EnableDownloadRateLimit(m_enableDownloadLimit->GetValue());
    m_cfg->Session()->DownloadRateLimit(static_cast<int>(dlLimit));
    m_cfg->Session()->EnableUploadRateLimit(m_enableUploadLimit->GetValue());
    m_cfg->Session()->UploadRateLimit(static_cast<int>(ulLimit));

    // Active limits
    long activeLimit = 0;
    m_activeLimit->GetValue().ToLong(&activeLimit);

    long activeDownloads = 0;
    m_activeDownloadsLimit->GetValue().ToLong(&activeDownloads);

    long activeSeeds = 0;
    m_activeSeedsLimit->GetValue().ToLong(&activeSeeds);

    m_cfg->Session()->ActiveLimit(static_cast<int>(activeLimit));
    m_cfg->Session()->ActiveDownloads(static_cast<int>(activeDownloads));
    m_cfg->Session()->ActiveSeeds(static_cast<int>(activeSeeds));
}

bool DownloadsPage::ValidateConfiguration(wxString& error)
{
    return true;
}
