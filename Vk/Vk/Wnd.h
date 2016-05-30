#pragma once
#include <string>

class Window
{
public:
  std::string m_name;

  Window(std::string const &name);
  virtual ~Window();

  virtual bool Init() = 0;
  virtual void Done() = 0;

  virtual uintptr_t GetID() = 0;
};

