#include "input.h"
#include "util/time.h"


namespace platform {

void Input::KeyEvent(InputEvent event, Key key)
{
  ASSERT(event == InputEvent::KeyPress || event == InputEvent::KeyRelease);
  ASSERT(key != Key::Invalid);
  KeyState &state = _keyState[key];
  bool pressed = event == InputEvent::KeyPress;
  auto now = clock::now();
  if (pressed) {
    state._pressTime = now;
    state._releaseTime = clock::time_point();
  } else {
    state._releaseTime = now;
  }
}

void Input::KeyInput(std::string character, Key key)
{
  _input += character;
}

bool Input::IsPressed(Key key)
{
  return GetKeyState(key).IsPressed();
}

bool Input::IsJustPressed(Key key)
{
  auto state = GetKeyState(key);
  return state.IsPressed() && state._pressTime >= _frameStart;
}

bool Input::IsReleased(Key key)
{
  return !GetKeyState(key).IsPressed();
}

bool Input::IsJustReleased(Key key)
{
  auto state = GetKeyState(key);
  return !state.IsPressed() && state._releaseTime >= _frameStart;
}


void Input::Update()
{
  _frameStart = clock::now();
  _input.clear();
}

}