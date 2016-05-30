#pragma once

#include <vector>
#include <string>

class Window;
class App
{
public:
  std::string m_name = "Application";
  uint32_t m_ver = 0;
  std::vector<std::string> m_args;
  std::vector<Window *> m_windows;

  App();
  virtual ~App();

  static App *Get() { return s_instance; }

  virtual Window *NewWindow(std::string const &wndName) = 0;
  virtual bool ProcessMessages() = 0;
  virtual uintptr_t GetID() = 0;

  static std::wstring StrToWstr(std::string const &str);
  static std::string WStrToStr(std::wstring const &wstr);

  void SetArgs(int argc, const char **args);
  void ParseArgs(std::string const &cmdLine);

  static App *s_instance;
};

int Main(App &app);