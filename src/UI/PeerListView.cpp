#include "PeerListView.hpp"

#include "../Models/Peer.hpp"
#include "../resources.h"
#include "../Scaler.hpp"
#include "../Translator.hpp"

#include <strsafe.h>

#define LV_COL_IP 1
#define LV_COL_CLIENT 2
#define LV_COL_FLAGS 3
#define LV_COL_DOWNLOAD 4
#define LV_COL_UPLOAD 5
#define LV_COL_PROGRESS 6

using UI::PeerListView;

PeerListView::PeerListView(HWND hWnd)
    : ListView::ListView(hWnd)
{
    AddColumn(LV_COL_IP, TRW("ip"), SX(110), ColumnType::Text);
    AddColumn(LV_COL_CLIENT, TRW("client"), SX(140), ColumnType::Text);
    AddColumn(LV_COL_FLAGS, TRW("flags"), SX(80), ColumnType::Text);
    AddColumn(LV_COL_DOWNLOAD, TRW("dl"), SX(80), ColumnType::Number);
    AddColumn(LV_COL_UPLOAD, TRW("ul"), SX(80), ColumnType::Number);
    AddColumn(LV_COL_PROGRESS, TRW("progress"), SX(100), ColumnType::Progress);
}

PeerListView::~PeerListView()
{
}

void PeerListView::Add(const Models::Peer& model)
{
    m_models.push_back(model);
}

void PeerListView::Remove(const Models::Peer& model)
{
    auto f = std::find(m_models.begin(), m_models.end(), model);
    if (f != m_models.end()) { m_models.erase(f); }
}

void PeerListView::Update(const Models::Peer& model)
{
    auto f = std::find(m_models.begin(), m_models.end(), model);

    if (f != m_models.end())
    {
        auto index = std::distance(m_models.begin(), f);
        m_models.at(index) = model;
    }
}

float PeerListView::GetItemProgress(int columnId, int itemIndex)
{
    switch (columnId)
    {
    case LV_COL_PROGRESS:
        return m_models.at(itemIndex).progress;
    }

    return -1;
}

std::wstring PeerListView::GetItemText(int columnId, int itemIndex)
{
    switch (columnId)
    {
    case LV_COL_IP:
        return m_models.at(itemIndex).endpoint;
    case LV_COL_CLIENT:
        return m_models.at(itemIndex).client;
    case LV_COL_FLAGS:
        return m_models.at(itemIndex).flags;
    case LV_COL_DOWNLOAD:
    case LV_COL_UPLOAD:
    {
        int rate = columnId == LV_COL_DOWNLOAD
            ? m_models.at(itemIndex).downloadRate
            : m_models.at(itemIndex).uploadRate;

        if (rate < 1024)
        {
            return TEXT("-");
        }

        TCHAR result[100];
        StrFormatByteSize64(rate, result, ARRAYSIZE(result));
        StringCchPrintf(result, ARRAYSIZE(result), L"%s/s", result);
        return result;
    }
    }

    return L"?unknown column?";
}

void PeerListView::ShowContextMenu(POINT p, const std::vector<int>& selectedIndices)
{
    
}
