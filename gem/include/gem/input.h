#pragma once
#include "SDL3/SDL.h"
#include "glm.hpp"
#include "unordered_map"

namespace gem {

enum class GamepadStick {
  invalid = -1,
  LS,
  RS,
};

enum class GamepadTrigger {
  invalid = -1,
  LT,
  RT,
};

enum class GamepadButton {
  invalid = -1,
  face_north,
  face_south,
  face_east,
  face_west,
  up,
  down,
  left,
  right,
  right_bumper,
  left_bumper,
  home,
  select,
  start,
  LS_click,
  RS_click
};

enum class MouseButton {
  invalid = -1,
  left,
  middle,
  right,
  extra1,
  extra2,
  extra3
};

enum class KeyboardKey {
  invalid = -1,
  a,
  b,
  c,
  d,
  e,
  f,
  g,
  h,
  i,
  j,
  k,
  l,
  m,
  n,
  o,
  p,
  q,
  r,
  s,
  t,
  u,
  v,
  w,
  x,
  y,
  z,
  zero,
  one,
  two,
  three,
  four,
  five,
  six,
  seven,
  eight,
  nine,
  minus,
  underscore,
  equals,
  plus,
  backspace,
  enter,
  space,
  tab,
  caps_lock,
  tilde,
  left_shift,
  left_control,
  left_alt,
  right_shift,
  right_control,
  right_alt,
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
  insert,
  home,
  page_up,
  page_down,
  delete_, // delete is reserved
  end,
  up,
  down,
  left,
  right,
  escape
};

class Input {
public:
  static bool GetGamepadButton(int index, GamepadButton button);
  static glm::vec2 GetGamepadStick(int index, GamepadStick stick);
  static bool GetKey(KeyboardKey key);
  static bool GetMouseButton(MouseButton button);
  static glm::vec2 GetMousePosition();
  static glm::vec2 GetMouseVelocity();
  static float GetMouseScroll();

protected:
  friend class GLBackend;
  static GamepadStick GetStickSDL(SDL_GamepadAxis &sdlAxis);
  static GamepadTrigger GetTriggerSDL(SDL_GamepadAxis &sdlAxis);
  static GamepadButton GetButtonSDL(uint8_t sdlButton);
  static KeyboardKey GetKeySDL(SDL_Keycode keyCode);

  static void UpdateGamepadButton(int gamepad_index, GamepadButton b,
                                    bool value);
  static void UpdateGamepadTrigger(int gamepad_index, GamepadTrigger b,
                                     float value);
  static void UpdateGamepadStick(int gamepad_index, GamepadStick b,
                                   glm::vec2 value);
  static void UpdateMouseButton(MouseButton b, bool value);
  static void UpdateMouseScroll(float value);
  static void UpdateMousePosition(glm::vec2 screen_dim, glm::vec2 value);
  static void UpdateKey(KeyboardKey k, bool value);
  static void UpdateLastFrame();

  struct GamepadState {
    std::unordered_map<GamepadButton, bool> current_frame_gamepad_button_state;
    std::unordered_map<GamepadButton, bool> last_frame_gamepad_button_state;
    std::unordered_map<GamepadTrigger, float> current_frame_trigger_state;
    std::unordered_map<GamepadTrigger, float> last_frame_trigger_state;
    std::unordered_map<GamepadStick, glm::vec2> current_frame_stick_state;
    std::unordered_map<GamepadStick, glm::vec2> last_frame_stick_state;
  };

  struct MouseState {
    std::unordered_map<MouseButton, bool> current_frame_mouse_button_state;
    std::unordered_map<MouseButton, bool> last_frame_mouses_button_state;
    float current_frame_scroll;
    float last_frame_scroll;
    glm::vec2 current_frame_mouse_location;
    glm::vec2 last_frame_mouse_location;
    glm::vec2 current_frame_mouse_velocity;
    glm::vec2 last_frame_mouse_velocity;
  };

  struct KeyboardState {
    std::unordered_map<KeyboardKey, bool> current_frame_key_state;
    std::unordered_map<KeyboardKey, bool> last_frame_key_state;
  };

  static constexpr int MAX_SUPPORTED_GAMEPADS = 4;

  inline static MouseState s_mouse_state;
  inline static KeyboardState s_keyboard_state;
  inline static GamepadState s_gamepad_state[MAX_SUPPORTED_GAMEPADS];
};
} // namespace gem