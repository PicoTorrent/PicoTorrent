#include "application.hpp"

#include "environment.hpp"
#include "mainframe.hpp"
#include "translator.hpp"

using pt::Application;

Application::Application()
{
    m_mainFrame = new MainFrame(
        std::make_shared<Environment>(),
        Translator::Load(GetModuleHandle(NULL))
    );
}

bool Application::OnInit()
{
    m_mainFrame->Show(true);
    return true;
}
