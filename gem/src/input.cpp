#include "gem/input.h"
#include "gem/profile.h"

namespace gem {

bool Input::GetGamepadButton(int index, GamepadButton button) {
  ZoneScoped;
  // AASSERT(index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
  return kGamepadState[index].current_frame_gamepad_button_state[button];
}

glm::vec2 Input::GetGamepadStick(int index, GamepadStick stick) {
  ZoneScoped;
  // AASSERT(index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
  return kGamepadState[index].current_frame_stick_state[stick];
}

bool Input::GetKey(KeyboardKey key) {
  ZoneScoped;
  return kKeyboardState.current_frame_key_state[key];
}

bool Input::GetMouseButton(MouseButton button) {
  ZoneScoped;
  return kMouseState.current_frame_mouse_button_state[button];
}

glm::vec2 Input::GetMousePosition() {
  ZoneScoped;
  return kMouseState.current_frame_mouse_location;
}

glm::vec2 Input::GetMouseVelocity() {
  ZoneScoped;
  return kMouseState.current_frame_mouse_velocity;
}

float Input::GetMouseScroll() {
  ZoneScoped;
  return kMouseState.current_frame_scroll;
}

GamepadStick Input::GetStickSDL(SDL_GamepadAxis &sdlAxis) {
  ZoneScoped;
  switch (sdlAxis) {
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTX:
    return GamepadStick::kLS;
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTY:
    return GamepadStick::kLS;
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTX:
    return GamepadStick::kRS;
  case SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTY:
    return GamepadStick::kRS;
  }
  return GamepadStick::kInvalid;
}

GamepadTrigger Input::GetTriggerSDL(SDL_GamepadAxis &sdlAxis) {
  ZoneScoped;
  switch (sdlAxis) {
  case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
    return GamepadTrigger::kLT;
  case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
    return GamepadTrigger::kRT;
  }
  return GamepadTrigger::kInvalid;
}

GamepadButton Input::GetButtonSDL(uint8_t sdlButton) {
  ZoneScoped;
  switch (sdlButton) {
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_SOUTH:
    return GamepadButton::kFaceSouth;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_EAST:
    return GamepadButton::kFaceEast;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_WEST:
    return GamepadButton::kFaceWest;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_NORTH:
    return GamepadButton::kFaceNorth;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_UP:
    return GamepadButton::kDpadUp;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_DOWN:
    return GamepadButton::kDpadDown;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_LEFT:
    return GamepadButton::kDpadLeft;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
    return GamepadButton::kDpadRight;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
    return GamepadButton::kRightBumper;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
    return GamepadButton::kLeftBumper;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_LEFT_STICK:
    return GamepadButton::kClickLS;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_RIGHT_STICK:
    return GamepadButton::kClickRS;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_START:
    return GamepadButton::kStart;
  case SDL_GamepadButton::SDL_GAMEPAD_BUTTON_BACK:
    return GamepadButton::kHome;
  default:
    return GamepadButton::kInvalid;
  }
}

KeyboardKey Input::GetKeySDL(SDL_Keycode keyCode) {
  ZoneScoped;
  {
    if (keyCode >= SDLK_0 && keyCode <= SDLK_9) {
      int diff = keyCode - SDLK_0;
      return static_cast<KeyboardKey>((int)KeyboardKey::k0 + diff);
    }

    if (keyCode >= SDLK_A && keyCode <= SDLK_Z) {
      int diff = keyCode - SDLK_A;
      return static_cast<KeyboardKey>((int)KeyboardKey::kA + diff);
    }

    if (keyCode >= SDLK_F1 && keyCode <= SDLK_F12) {
      int diff = keyCode - SDLK_F1;
      return static_cast<KeyboardKey>((int)KeyboardKey::kF1 + diff);
    }

    switch (keyCode) {
    case SDLK_MINUS:
      return KeyboardKey::kMinus;
    case SDLK_UNDERSCORE:
      return KeyboardKey::kUnderscore;
    case SDLK_EQUALS:
      return KeyboardKey::kEquals;
    case SDLK_PLUS:
      return KeyboardKey::kPlus;
    case SDLK_BACKSPACE:
      return KeyboardKey::kBackspace;
    case SDLK_RETURN:
      return KeyboardKey::kEnter;
    case SDLK_SPACE:
      return KeyboardKey::kSpace;
    case SDLK_TAB:
      return KeyboardKey::kTab;
    case SDLK_CAPSLOCK:
      return KeyboardKey::kCapsLock;
    case SDLK_LSHIFT:
      return KeyboardKey::kLeftShift;
    case SDLK_LCTRL:
      return KeyboardKey::kLeftControl;
    case SDLK_LALT:
      return KeyboardKey::kLeftAlt;
    case SDLK_RSHIFT:
      return KeyboardKey::kRightShift;
    case SDLK_RCTRL:
      return KeyboardKey::kRightControl;
    case SDLK_RALT:
      return KeyboardKey::kRightAlt;
    case SDLK_INSERT:
      return KeyboardKey::kInsert;
    case SDLK_HOME:
      return KeyboardKey::kHome;
    case SDLK_PAGEUP:
      return KeyboardKey::kPageUp;
    case SDLK_PAGEDOWN:
      return KeyboardKey::kPageDown;
    case SDLK_DELETE:
      return KeyboardKey::kDelete;
    case SDLK_END:
      return KeyboardKey::kEnd;
    case SDLK_UP:
      return KeyboardKey::kUp;
    case SDLK_DOWN:
      return KeyboardKey::kDown;
    case SDLK_LEFT:
      return KeyboardKey::kLeft;
    case SDLK_RIGHT:
      return KeyboardKey::kRight;
    case SDLK_ESCAPE:
      return KeyboardKey::kEscape;
    case SDLK_GRAVE:
      return KeyboardKey::kTilde;
    default:
      return KeyboardKey::kInvalid;
    }
  }
}

void Input::UpdateGamepadButton(int gamepad_index, GamepadButton b,
                                  bool value) {
  ZoneScoped;
  // AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index
  // Provided");
  kGamepadState->current_frame_gamepad_button_state[b] = value;
}

void Input::UpdateGamepadTrigger(int gamepad_index, GamepadTrigger b,
                                   float value) {
  ZoneScoped;
  // AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index
  // Provided");
  kGamepadState->current_frame_trigger_state[b] = value;
}

void Input::UpdateGamepadStick(int gamepad_index, GamepadStick b,
                                 glm::vec2 value) {
  ZoneScoped;
  // AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index
  // Provided");
  kGamepadState->current_frame_stick_state[b] = value;
}

void Input::UpdateMouseButton(MouseButton b, bool value) {
  ZoneScoped;
  kMouseState.current_frame_mouse_button_state[b] = value;
}

void Input::UpdateMouseScroll(float value) {
  ZoneScoped;
  kMouseState.current_frame_scroll = value;
}

void Input::UpdateMousePosition(glm::vec2 screen_dim, glm::vec2 value) {
  ZoneScoped;
  glm::vec2 val = value;
  val.x = glm::min(value.x, screen_dim.x);
  val.x = glm::max(value.x, 0.0f);
  val.y = glm::min(value.y, screen_dim.y);
  val.y = glm::max(value.y, 0.0f);
  kMouseState.current_frame_mouse_location = value;
  kMouseState.current_frame_mouse_velocity =
      value - kMouseState.last_frame_mouse_location;
}

void Input::UpdateKey(KeyboardKey k, bool value) {
  ZoneScoped;
  kKeyboardState.current_frame_key_state[k] = value;
}

void Input::UpdateLastFrame() {
  ZoneScoped;
  kKeyboardState.last_frame_key_state =
      kKeyboardState.current_frame_key_state;

  kMouseState.last_frame_mouses_button_state =
      kMouseState.current_frame_mouse_button_state;
  kMouseState.last_frame_mouse_location =
      kMouseState.current_frame_mouse_location;
  kMouseState.last_frame_mouse_velocity =
      kMouseState.current_frame_mouse_velocity;
  kMouseState.last_frame_scroll = kMouseState.current_frame_scroll;

  for (int i = 0; i < MAX_SUPPORTED_GAMEPADS; i++) {
    kGamepadState[i].last_frame_stick_state =
        kGamepadState->current_frame_stick_state;
    kGamepadState[i].last_frame_gamepad_button_state =
        kGamepadState->current_frame_gamepad_button_state;
    kGamepadState[i].last_frame_stick_state =
        kGamepadState->current_frame_stick_state;
  }
}
} // namespace gem