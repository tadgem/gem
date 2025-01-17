#include "gem/input.h"
#include "gem/profile.h"

namespace gem {

bool Input::get_gamepad_button(int index, GamepadButton button) {
  ZoneScoped;
  // AASSERT(index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
  return s_gamepad_state[index].current_frame_gamepad_button_state[button];
}

glm::vec2 Input::get_gamepad_stick(int index, GamepadStick stick) {
  ZoneScoped;
  // AASSERT(index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
  return s_gamepad_state[index].current_frame_stick_state[stick];
}

bool Input::get_keyboard_key(KeyboardKey key) {
  ZoneScoped;
  return s_keyboard_state.current_frame_key_state[key];
}

bool Input::get_mouse_button(MouseButton button) {
  ZoneScoped;
  return s_mouse_state.current_frame_mouse_button_state[button];
}

glm::vec2 Input::get_mouse_position() {
  ZoneScoped;
  return s_mouse_state.current_frame_mouse_location;
}

glm::vec2 Input::get_mouse_velocity() {
  ZoneScoped;
  return s_mouse_state.current_frame_mouse_velocity;
}

float Input::get_mouse_scroll() {
  ZoneScoped;
  return s_mouse_state.current_frame_scroll;
}

GamepadStick Input::get_stick_from_sdl(SDL_GamepadAxis &sdlAxis) {
  ZoneScoped;
  switch (sdlAxis) {
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTX:
    return GamepadStick::LS;
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTY:
    return GamepadStick::LS;
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTX:
    return GamepadStick::RS;
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTY:
    return GamepadStick::RS;
  }
  return GamepadStick::invalid;
}

GamepadTrigger Input::get_trigger_from_sdl(SDL_GamepadAxis &sdlAxis) {
  ZoneScoped;
  switch (sdlAxis) {
  case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
    return GamepadTrigger::LT;
  case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
    return GamepadTrigger::RT;
  }
  return GamepadTrigger::invalid;
}

GamepadButton Input::get_button_from_sdl(uint8_t sdlButton) {
  ZoneScoped;
  switch (sdlButton) {
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_SOUTH:
    return GamepadButton::face_south;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_EAST:
    return GamepadButton::face_east;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_WEST:
    return GamepadButton::face_west;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_NORTH:
    return GamepadButton::face_north;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_UP:
    return GamepadButton::up;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_DOWN:
    return GamepadButton::down;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_LEFT:
    return GamepadButton::left;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
    return GamepadButton::right;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
    return GamepadButton::right_bumper;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
    return GamepadButton::left_bumper;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_STICK:
    return GamepadButton::LS_click;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_STICK:
    return GamepadButton::RS_click;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_START:
    return GamepadButton::start;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_BACK:
    return GamepadButton::home;
  default:
    return GamepadButton::invalid;
  }
}

KeyboardKey Input::get_key_from_sdl(SDL_Keycode keyCode) {
  ZoneScoped;
  {
    if (keyCode >= SDLK_0 && keyCode <= SDLK_9) {
      int diff = keyCode - SDLK_0;
      return static_cast<KeyboardKey>((int)KeyboardKey::zero + diff);
    }

    if (keyCode >= SDLK_A && keyCode <= SDLK_Z) {
      int diff = keyCode - SDLK_A;
      return static_cast<KeyboardKey>((int)KeyboardKey::a + diff);
    }

    if (keyCode >= SDLK_F1 && keyCode <= SDLK_F12) {
      int diff = keyCode - SDLK_F1;
      return static_cast<KeyboardKey>((int)KeyboardKey::F1 + diff);
    }

    switch (keyCode) {
    case SDLK_MINUS:
      return KeyboardKey::minus;
    case SDLK_UNDERSCORE:
      return KeyboardKey::underscore;
    case SDLK_EQUALS:
      return KeyboardKey::equals;
    case SDLK_PLUS:
      return KeyboardKey::plus;
    case SDLK_BACKSPACE:
      return KeyboardKey::backspace;
    case SDLK_RETURN:
      return KeyboardKey::enter;
    case SDLK_SPACE:
      return KeyboardKey::space;
    case SDLK_TAB:
      return KeyboardKey::tab;
    case SDLK_CAPSLOCK:
      return KeyboardKey::caps_lock;
    case SDLK_LSHIFT:
      return KeyboardKey::left_shift;
    case SDLK_LCTRL:
      return KeyboardKey::left_control;
    case SDLK_LALT:
      return KeyboardKey::left_alt;
    case SDLK_RSHIFT:
      return KeyboardKey::right_shift;
    case SDLK_RCTRL:
      return KeyboardKey::right_control;
    case SDLK_RALT:
      return KeyboardKey::right_alt;
    case SDLK_INSERT:
      return KeyboardKey::insert;
    case SDLK_HOME:
      return KeyboardKey::home;
    case SDLK_PAGEUP:
      return KeyboardKey::page_up;
    case SDLK_PAGEDOWN:
      return KeyboardKey::page_down;
    case SDLK_DELETE:
      return KeyboardKey::delete_;
    case SDLK_END:
      return KeyboardKey::end;
    case SDLK_UP:
      return KeyboardKey::up;
    case SDLK_DOWN:
      return KeyboardKey::down;
    case SDLK_LEFT:
      return KeyboardKey::left;
    case SDLK_RIGHT:
      return KeyboardKey::right;
    case SDLK_ESCAPE:
      return KeyboardKey::escape;
    case SDLK_GRAVE:
      return KeyboardKey::tilde;
    default:
      return KeyboardKey::invalid;
    }
  }
}

void Input::update_gamepad_button(int gamepad_index, GamepadButton b,
                                  bool value) {
  ZoneScoped;
  // AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index
  // Provided");
  s_gamepad_state->current_frame_gamepad_button_state[b] = value;
}

void Input::update_gamepad_trigger(int gamepad_index, GamepadTrigger b,
                                   float value) {
  ZoneScoped;
  // AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index
  // Provided");
  s_gamepad_state->current_frame_trigger_state[b] = value;
}

void Input::update_gamepad_stick(int gamepad_index, GamepadStick b,
                                 glm::vec2 value) {
  ZoneScoped;
  // AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index
  // Provided");
  s_gamepad_state->current_frame_stick_state[b] = value;
}

void Input::update_mouse_button(MouseButton b, bool value) {
  ZoneScoped;
  s_mouse_state.current_frame_mouse_button_state[b] = value;
}

void Input::update_mouse_scroll(float value) {
  ZoneScoped;
  s_mouse_state.current_frame_scroll = value;
}

void Input::update_mouse_position(glm::vec2 screen_dim, glm::vec2 value) {
  ZoneScoped;
  glm::vec2 val = value;
  val.x = glm::min(value.x, screen_dim.x);
  val.x = glm::max(value.x, 0.0f);
  val.y = glm::min(value.y, screen_dim.y);
  val.y = glm::max(value.y, 0.0f);
  s_mouse_state.current_frame_mouse_location = value;
  s_mouse_state.current_frame_mouse_velocity =
      value - s_mouse_state.last_frame_mouse_location;
}

void Input::update_keyboard_key(KeyboardKey k, bool value) {
  ZoneScoped;
  s_keyboard_state.current_frame_key_state[k] = value;
}

void Input::update_last_frame() {
  ZoneScoped;
  s_keyboard_state.last_frame_key_state =
      s_keyboard_state.current_frame_key_state;

  s_mouse_state.last_frame_mouses_button_state =
      s_mouse_state.current_frame_mouse_button_state;
  s_mouse_state.last_frame_mouse_location =
      s_mouse_state.current_frame_mouse_location;
  s_mouse_state.last_frame_mouse_velocity =
      s_mouse_state.current_frame_mouse_velocity;
  s_mouse_state.last_frame_scroll = s_mouse_state.current_frame_scroll;

  for (int i = 0; i < MAX_SUPPORTED_GAMEPADS; i++) {
    s_gamepad_state[i].last_frame_stick_state =
        s_gamepad_state->current_frame_stick_state;
    s_gamepad_state[i].last_frame_gamepad_button_state =
        s_gamepad_state->current_frame_gamepad_button_state;
    s_gamepad_state[i].last_frame_stick_state =
        s_gamepad_state->current_frame_stick_state;
  }
}
} // namespace gem