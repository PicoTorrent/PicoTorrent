#include "AboutDialog.hpp"

#include <shellapi.h>
#include <strsafe.h>

#include "../Scaler.hpp"
#include "../Translator.hpp"
#include "../VersionInformation.hpp"

using Dialogs::AboutDialog;

AboutDialog::AboutDialog()
    : m_font(CreateFont(SY(24), 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, L"MS Shell Dlg"))
{
}

AboutDialog::~AboutDialog()
{
    DeleteObject(m_font);
}

LRESULT AboutDialog::OnClick(LPNMHDR pnmh)
{
    PNMLINK pNMLink = reinterpret_cast<PNMLINK>(pnmh);
    LITEM item = pNMLink->item;

    if ((pnmh->idFrom == ID_GITHUB_LINK) && (item.iLink == 0))
    {
        ShellExecute(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
    }

    return FALSE;
}

void AboutDialog::OnEndDialog(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    EndDialog(nID);
}

BOOL AboutDialog::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
    CenterWindow();

    RECT dlgRc;
    GetClientRect(&dlgRc);

    HANDLE hImage = LoadImage(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON,
        SX(128),
        SY(128),
        LR_DEFAULTCOLOR);

    SendDlgItemMessage(ID_PICO_LOGO, STM_SETICON, (WPARAM)hImage, 0);

    int x = ((dlgRc.right - dlgRc.left) - SX(128)) / 2;
    GetDlgItem(ID_PICO_LOGO).SetWindowPos(HWND_TOPMOST, x, SY(10), -1, -1, SWP_NOSIZE | SWP_NOZORDER);

    SendDlgItemMessage(ID_PICOTORRENT_V_FORMAT, WM_SETFONT, (WPARAM)m_font, TRUE);

    TCHAR picoVersion[1024];
    StringCchPrintf(
        picoVersion,
        ARRAYSIZE(picoVersion),
        TRW("picotorrent_v_format"),
        ToWideString(VersionInformation::GetCurrentVersion()).c_str());

    TCHAR picoBuild[1024];
    StringCchPrintf(
        picoBuild,
        ARRAYSIZE(picoBuild),
        TRW("build_info_format"),
        ToWideString(VersionInformation::GetBranch()).c_str(),
        ToWideString(VersionInformation::GetCommitHash()).c_str());

    // Localize
    SetWindowText(TRW("about_picotorrent"));
    SetDlgItemText(ID_PICOTORRENT_V_FORMAT, picoVersion);
    SetDlgItemText(ID_BUILD_INFO_FORMAT, picoBuild);
    SetDlgItemText(ID_PICOTORRENT_DESCRIPTION, TRW("picotorrent_description"));
    SetDlgItemText(ID_GITHUB_LINK, TRW("github_link"));

    return FALSE;
}
