#include "stdafx.h"
#include "App.h"
#include "Wnd.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <locale>
#include <codecvt>
#include <string>

App *App::s_instance = nullptr;

App::App()
{
  assert(!s_instance);
  s_instance = this;
}

App::~App()
{
  for (Window *w : m_windows) {
    w->Done();
    delete w;
  }
  assert(s_instance == this);
  s_instance = nullptr;
}

std::wstring App::StrToWstr(std::string const &str)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wstr = converter.from_bytes(str);
  return wstr;
}

std::string App::WStrToStr(std::wstring const &wstr)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::string str = converter.to_bytes(wstr);
  return str;
}

void App::SetArgs(int argc, const char **args)
{
  m_args.resize(argc);
  for (int i = 0; i < argc; ++i)
    m_args[i] = std::string(args[i]);
}

void App::ParseArgs(std::string const &cmdLine)
{
  m_args.clear();
  std::istringstream input(cmdLine);
  std::copy(std::istream_iterator<std::string>(input), std::istream_iterator<std::string>(), std::back_inserter(m_args));
}
