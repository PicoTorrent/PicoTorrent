#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <memory>

namespace pt
{
    class Translator;

    class FileContextMenu : public wxMenu
    {
    public:
        enum {
            ptID_PRIO_MAXIMUM = wxID_HIGHEST,
            ptID_PRIO_HIGH,
            ptID_PRIO_NORMAL,
            ptID_PRIO_SKIP
        };

        FileContextMenu(std::shared_ptr<Translator> translator);
    };
}
