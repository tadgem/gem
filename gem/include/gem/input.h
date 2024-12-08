#pragma once
#include "SDL3/SDL.h"
#include "glm.hpp"
#include "unordered_map"

namespace gem {

enum class gamepad_stick {
  invalid = -1,
  LS,
  RS,
};

enum class gamepad_trigger {
  invalid = -1,
  LT,
  RT,
};

enum class gamepad_button {
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

enum class mouse_button {
  invalid = -1,
  left,
  middle,
  right,
  extra1,
  extra2,
  extra3
};

enum class keyboard_key {
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

class input {
public:
  static bool get_gamepad_button(int index, gamepad_button button);
  static glm::vec2 get_gamepad_stick(int index, gamepad_stick stick);
  static bool get_keyboard_key(keyboard_key key);
  static bool get_mouse_button(mouse_button button);
  static glm::vec2 get_mouse_position();
  static glm::vec2 get_mouse_velocity();
  static float get_mouse_scroll();

protected:
  friend class gl_backend;
  static gamepad_stick get_stick_from_sdl(SDL_GamepadAxis &sdlAxis);
  static gamepad_trigger get_trigger_from_sdl(SDL_GamepadAxis &sdlAxis);
  static gamepad_button get_button_from_sdl(uint8_t sdlButton);
  static keyboard_key get_key_from_sdl(SDL_Keycode keyCode);

  static void update_gamepad_button(int gamepad_index, gamepad_button b,
                                    bool value);
  static void update_gamepad_trigger(int gamepad_index, gamepad_trigger b,
                                     float value);
  static void update_gamepad_stick(int gamepad_index, gamepad_stick b,
                                   glm::vec2 value);
  static void update_mouse_button(mouse_button b, bool value);
  static void update_mouse_scroll(float value);
  static void update_mouse_position(glm::vec2 screen_dim, glm::vec2 value);
  static void update_keyboard_key(keyboard_key k, bool value);
  static void update_last_frame();

  struct gamepad_state {
    std::unordered_map<gamepad_button, bool> current_frame_gamepad_button_state;
    std::unordered_map<gamepad_button, bool> last_frame_gamepad_button_state;
    std::unordered_map<gamepad_trigger, float> current_frame_trigger_state;
    std::unordered_map<gamepad_trigger, float> last_frame_trigger_state;
    std::unordered_map<gamepad_stick, glm::vec2> current_frame_stick_state;
    std::unordered_map<gamepad_stick, glm::vec2> last_frame_stick_state;
  };

  struct mouse_state {
    std::unordered_map<mouse_button, bool> current_frame_mouse_button_state;
    std::unordered_map<mouse_button, bool> last_frame_mouses_button_state;
    float current_frame_scroll;
    float last_frame_scroll;
    glm::vec2 current_frame_mouse_location;
    glm::vec2 last_frame_mouse_location;
    glm::vec2 current_frame_mouse_velocity;
    glm::vec2 last_frame_mouse_velocity;
  };

  struct keyboard_state {
    std::unordered_map<keyboard_key, bool> current_frame_key_state;
    std::unordered_map<keyboard_key, bool> last_frame_key_state;
  };

  static constexpr int MAX_SUPPORTED_GAMEPADS = 4;

  inline static mouse_state s_mouse_state;
  inline static keyboard_state s_keyboard_state;
  inline static gamepad_state s_gamepad_state[MAX_SUPPORTED_GAMEPADS];
};
} // namespace gem