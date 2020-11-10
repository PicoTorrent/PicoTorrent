#include "translator.hpp"

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include "../core/utils.hpp"

using json = nlohmann::json;
using pt::UI::Translator;

Translator::Translator()
    : m_selectedLocale("en")
{
}

Translator& Translator::GetInstance()
{
    static Translator translator;
    return translator;
}

std::string Translator::GetLocale()
{
    return m_selectedLocale;
}

void Translator::LoadDatabase(std::filesystem::path const& filePath)
{
    std::string convertedPath = Utils::toStdString(filePath.wstring());

    BOOST_LOG_TRIVIAL(info) << "Loading translations from " << convertedPath;

    sqlite3* db = nullptr;

    if (sqlite3_open(convertedPath.c_str(), &db) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load translations database";
        return;
    }

    sqlite3_exec(
        db,
        "SELECT locale, key, value FROM translations ORDER BY locale, key ASC",
        &Translator::LoadDatabaseCallback,
        this,
        nullptr);

    sqlite3_close(db);
}

int Translator::LoadDatabaseCallback(void* user, int, char** data, char**)
{
    Translator* tr = static_cast<Translator*>(user);
#ifdef _WIN32
    std::string loc = data[0];
    std::string key = data[1];
    std::wstring val = Utils::toStdWString(data[2]);
    TCHAR localeNameBuffer[1024];

    if (tr->m_languages.find(loc) == tr->m_languages.end())
    {
        Language l;
        l.locale = loc;


        int res = GetLocaleInfoEx(
            Utils::toStdWString(l.locale).c_str(),
            LOCALE_SLOCALIZEDLANGUAGENAME,
            localeNameBuffer,
            ARRAYSIZE(localeNameBuffer));

        if (res > 0)
        {
            l.name = std::wstring(localeNameBuffer, res);
        }
        else
        {
            BOOST_LOG_TRIVIAL(error) << "GetLocaleInfoEx returned " << res << " for " << l.locale;
        }

        tr->m_languages.insert({ loc, l });
    }

    tr->m_languages.at(loc).translations.insert({ key, val });
#endif
    return SQLITE_OK;
}

std::vector<Translator::Language> Translator::Languages()
{
    std::vector<Language> result;

    for (auto& p : m_languages)
    {
        result.push_back(p.second);
    }

    std::sort(
        result.begin(),
        result.end(),
        [](Language const& lhs, Language const& rhs)
        {
            return lhs.name < rhs.name;
        });

    return result;
}

std::wstring Translator::Translate(std::string const& key)
{
    return std::wstring(key.begin(), key.end());

    // Try to find the language we have selected
    auto lang = m_languages.find(m_selectedLocale);

    // Didn't find our selected language, try with 'en' instead
    if (lang == m_languages.end()) { lang = m_languages.find("en"); }

    // Didn't find 'en', return the key
    if (lang == m_languages.end()) { return Utils::toStdWString(key); }

    auto translation = lang->second.translations.find(key);

    if (translation == lang->second.translations.end())
    {
        return Utils::toStdWString(key);
    }

    return translation->second;
}

void Translator::SetLocale(std::string const& locale)
{
    // a locale can be en-SV (english language but swedish format on dates etc)
    // in this case, we want to use the 'en' translations. check if we have an
    // exact match, otherwise remove the -SV part and check again

    if (m_languages.find(locale) == m_languages.end())
    {
        std::string tmpLocale = locale;

        if (locale.find_first_of('-') != std::string::npos)
        {
            tmpLocale = locale.substr(0, locale.find_first_of('-'));
        }

        if (m_languages.find(tmpLocale) != m_languages.end())
        {
            BOOST_LOG_TRIVIAL(info) << "Adjusting locale from " << locale << " to " << tmpLocale << " in order to match available language";
            m_selectedLocale = tmpLocale;
        }
        else
        {
            // loop through and see if we have other locales which might match. take first
            for (auto const& lang : m_languages)
            {
                if (lang.first.size() >= tmpLocale.size()
                    && lang.first.substr(0, tmpLocale.size()) == tmpLocale)
                {
                    // we found a match on language with a different culture... use it
                    BOOST_LOG_TRIVIAL(info) << "Adjusting locale from " << locale << " to " << lang.first << " after substring matching";
                    m_selectedLocale = lang.first;
                }
            }
        }
    }
    else
    {
        m_selectedLocale = locale;
    }
}
