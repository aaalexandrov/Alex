#pragma once

#include <unordered_map>
#include <chrono>
#include "util/enumutl.h"

namespace platform {

enum class Key : int32_t {
  Invalid = -1,

  Tab = '\t',
  Enter = '\n',
  Space = ' ',
  Key0 = '0',
  Key1 = '1',
  Key2 = '2',
  Key3 = '3',
  Key4 = '4',
  Key5 = '5',
  Key6 = '6',
  Key7 = '7',
  Key8 = '8',
  Key9 = '9',
  A = 'A',
  B = 'B',
  C = 'C',
  D = 'D',
  E = 'E',
  F = 'F',
  G = 'G',
  H = 'H',
  I = 'I',
  J = 'J',
  K = 'K',
  L = 'L',
  M = 'M',
  N = 'N',
  O = 'O',
  P = 'P',
  Q = 'Q',
  R = 'R',
  S = 'S',
  T = 'T',
  U = 'U',
  V = 'V',
  W = 'W',
  X = 'X',
  Y = 'Y',
  Z = 'Z',
  Minus = '-',
  Plus = '+',
  LeftBracket = '[',
  RightBracket = ']',
  SemiColon = ';',
  Apostrophe = '\'',
  Backslash = '\\',
  Comma = ',',
  Dot = '.',
  Slash = '/',
  Asterisk = '*',
  Tilde = '~',

  LButton = 0x100,
  RButton,
  MButton,
  XButton1,
  XButton2,

  Backspace,
  Pause,
  CapsLock,
  Escape,
  PageUp,
  PageDown,
  End,
  Home,
  Left,
  Up,
  Right,
  Down,
  PrintScreen,
  Insert,
  Delete,
  NumLock,
  ScrollLock,
  LWin,
  RWin,
  LShift,
  RShift,
  LControl,
  RControl,
  LAlt,
  RAlt,
  ContextMenu,

  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  Numpad = 0x800, // added as bit to the base key code to indicate numpad variant
};

DEFINE_ENUM_BIT_OPERATORS(Key)

enum class InputEvent {
  KeyPress,
  KeyRelease,
};

class Input {
public:
  using clock = std::chrono::system_clock;

  struct KeyState {
    clock::time_point _pressTime;
    clock::time_point _releaseTime;

    bool IsPressed() const { return _pressTime > _releaseTime; }
  };

  std::unordered_map<Key, KeyState> _keyState;
  std::string _input;
  clock::time_point _frameStart;

  KeyState const &GetKeyState(Key key) { return _keyState[key]; }

  void KeyEvent(InputEvent event, Key key);
  void KeyInput(std::string character, Key key);

  bool IsPressed(Key key);
  bool IsJustPressed(Key key);

  bool IsReleased(Key key);
  bool IsJustReleased(Key key);

  void Update();
};

}