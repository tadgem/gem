#pragma once
#include "SDL3/SDL.h"
#include "glm.hpp"
#include "unordered_map"

namespace gem {

enum class GamepadStick {
  kInvalid = -1,
  kLS,
  kRS,
};

enum class GamepadTrigger {
  kInvalid = -1,
  kLT,
  kRT,
};

enum class GamepadButton {
  kInvalid = -1,
  kFaceNorth,
  kFaceSouth,
  kFaceEast,
  kFaceWest,
  kDpadUp,
  kDpadDown,
  kDpadLeft,
  kDpadRight,
  kRightBumper,
  kLeftBumper,
  kHome,
  kSelect,
  kStart,
  kClickLS,
  kClickRS
};

enum class MouseButton {
  kInvalid = -1,
  kLeft,
  kMiddle,
  kRight,
  kExtra1,
  kExtra2,
  kExtra3
};

enum class KeyboardKey {
  kInvalid = -1,
  kA,
  kB,
  kC,
  kD,
  kE,
  kF,
  kG,
  kH,
  kI,
  kJ,
  kK,
  kL,
  kM,
  kN,
  kO,
  kP,
  kQ,
  kR,
  kS,
  kT,
  kU,
  kV,
  kW,
  kX,
  kY,
  kZ,
  k0,
  k1,
  k2,
  k3,
  k4,
  k5,
  k6,
  k7,
  k8,
  k9,
  kMinus,
  kUnderscore,
  kEquals,
  kPlus,
  kBackspace,
  kEnter,
  kSpace,
  kTab,
  kCapsLock,
  kTilde,
  kLeftShift,
  kLeftControl,
  kLeftAlt,
  kRightShift,
  kRightControl,
  kRightAlt,
  kF1,
  kF2,
  kF3,
  kF4,
  kF5,
  kF6,
  kF7,
  kF8,
  kF9,
  kF10,
  kF11,
  kF12,
  kInsert,
  kHome,
  kPageUp,
  kPageDown,
  kDelete,
  kEnd,
  kUp,
  kDown,
  kLeft,
  kRight,
  kEscape
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

  inline static MouseState kMouseState;
  inline static KeyboardState kKeyboardState;
  inline static GamepadState kGamepadState[MAX_SUPPORTED_GAMEPADS];
};
} // namespace gem