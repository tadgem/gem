#include "input.h"
#include "io.h"
#include "tracy/Tracy.hpp"

bool input::get_gamepad_button(int index, gamepad_button button)
{
    ZoneScoped;
    // AASSERT(index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
    return s_gamepad_state[index].current_frame_gamepad_button_state[button];
}

glm::vec2 input::get_gamepad_stick(int index, gamepad_stick stick)
{
    ZoneScoped;
    // AASSERT(index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
    return s_gamepad_state[index].current_frame_stick_state[stick];
}

bool input::get_keyboard_key(keyboard_key key)
{
    ZoneScoped;
    return s_keyboard_state.current_frame_key_state[key];
}

bool input::get_mouse_button(mouse_button button)
{
    ZoneScoped;
    return s_mouse_state.current_frame_mouse_button_state[button];
}

glm::vec2 input::get_mouse_position()
{
    ZoneScoped;
    return s_mouse_state.current_frame_mouse_location;
}

glm::vec2 input::get_mouse_velocity()
{
    ZoneScoped;
    return s_mouse_state.current_frame_mouse_velocity;
}

float input::get_mouse_scroll()
{
    ZoneScoped;
    return s_mouse_state.current_frame_scroll;
}

gamepad_stick input::get_stick_from_sdl(SDL_GameControllerAxis& sdlAxis)
{
    ZoneScoped;
    switch (sdlAxis) {
    case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX:
        return gamepad_stick::LS;
    case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY:
        return gamepad_stick::LS;
    case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX:
        return gamepad_stick::RS;
    case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY:
        return gamepad_stick::RS;
    }
    return gamepad_stick::invalid;
}

gamepad_trigger input::get_trigger_from_sdl(SDL_GameControllerAxis& sdlAxis)
{
    ZoneScoped;
    switch (sdlAxis) {
    case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        return gamepad_trigger::LT;
    case SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        return gamepad_trigger::RT;
    }
    return gamepad_trigger::invalid;
}

gamepad_button input::get_button_from_sdl(uint8_t sdlButton)
{
    ZoneScoped;
    switch (sdlButton) {
    case SDL_CONTROLLER_BUTTON_A:
        return gamepad_button::face_south;
    case SDL_CONTROLLER_BUTTON_B:
        return gamepad_button::face_east;
    case SDL_CONTROLLER_BUTTON_X:
        return gamepad_button::face_west;
    case SDL_CONTROLLER_BUTTON_Y:
        return gamepad_button::face_north;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        return gamepad_button::up;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        return gamepad_button::down;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        return gamepad_button::left;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        return gamepad_button::right;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        return gamepad_button::right_bumper;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        return gamepad_button::left_bumper;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        return gamepad_button::LS_click;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        return gamepad_button::RS_click;
    case SDL_CONTROLLER_BUTTON_START:
        return gamepad_button::start;
    case SDL_CONTROLLER_BUTTON_BACK:
        return gamepad_button::home;
    default:
        return gamepad_button::invalid;
    }
}

keyboard_key input::get_key_from_sdl(SDL_Keycode keyCode)
{
    ZoneScoped;
    {
        if (keyCode >= SDLK_0 && keyCode <= SDLK_9) {
            int diff = keyCode - SDLK_0;
            return static_cast<keyboard_key>((int)keyboard_key::zero + diff);
        }

        if (keyCode >= SDLK_a && keyCode <= SDLK_z) {
            int diff = keyCode - SDLK_a;
            return static_cast<keyboard_key>((int)keyboard_key::a + diff);
        }

        if (keyCode >= SDLK_F1 && keyCode <= SDLK_F12) {
            int diff = keyCode - SDLK_F1;
            return static_cast<keyboard_key>((int)keyboard_key::F1 + diff);
        }

        switch (keyCode) {
        case SDLK_MINUS:
            return keyboard_key::minus;
        case SDLK_UNDERSCORE:
            return keyboard_key::underscore;
        case SDLK_EQUALS:
            return keyboard_key::equals;
        case SDLK_PLUS:
            return keyboard_key::plus;
        case SDLK_BACKSPACE:
            return keyboard_key::backspace;
        case SDLK_RETURN:
            return keyboard_key::enter;
        case SDLK_SPACE:
            return keyboard_key::space;
        case SDLK_TAB:
            return keyboard_key::tab;
        case SDLK_CAPSLOCK:
            return keyboard_key::caps_lock;
        case SDLK_LSHIFT:
            return keyboard_key::left_shift;
        case SDLK_LCTRL:
            return keyboard_key::left_control;
        case SDLK_LALT:
            return keyboard_key::left_alt;
        case SDLK_RSHIFT:
            return keyboard_key::right_shift;
        case SDLK_RCTRL:
            return keyboard_key::right_control;
        case SDLK_RALT:
            return keyboard_key::right_alt;
        case SDLK_INSERT:
            return keyboard_key::insert;
        case SDLK_HOME:
            return keyboard_key::home;
        case SDLK_PAGEUP:
            return keyboard_key::page_up;
        case SDLK_PAGEDOWN:
            return keyboard_key::page_down;
        case SDLK_DELETE:
            return keyboard_key::delete_;
        case SDLK_END:
            return keyboard_key::end;
        case SDLK_UP:
            return keyboard_key::up;
        case SDLK_DOWN:
            return keyboard_key::down;
        case SDLK_LEFT:
            return keyboard_key::left;
        case SDLK_RIGHT:
            return keyboard_key::right;
        case SDLK_ESCAPE:
            return keyboard_key::escape;
        case SDLK_BACKQUOTE:
            return keyboard_key::tilde;
        default:
            return keyboard_key::invalid;
        }

    }
}

void input::update_gamepad_button(int gamepad_index, gamepad_button b, bool value)
{
    ZoneScoped;
    // AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
    s_gamepad_state->current_frame_gamepad_button_state[b] = value;
}

void input::update_gamepad_trigger(int gamepad_index, gamepad_trigger b, float value)
{
    ZoneScoped;
    //AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
    s_gamepad_state->current_frame_trigger_state[b] = value;
}

void input::update_gamepad_stick(int gamepad_index, gamepad_stick b, glm::vec2 value)
{
    ZoneScoped;
    //AASSERT(gamepad_index < MAX_SUPPORTED_GAMEPADS, "Invalid Gamepad Index Provided");
    s_gamepad_state->current_frame_stick_state[b] = value;
}

void input::update_mouse_button(mouse_button b, bool value)
{
    ZoneScoped;
    s_mouse_state.current_frame_mouse_button_state[b] = value;
}

void input::update_mouse_scroll(float value)
{
    ZoneScoped;
    s_mouse_state.current_frame_scroll = value;
}

void input::update_mouse_position(glm::vec2 screen_dim, glm::vec2 value)
{
    ZoneScoped;
    glm::vec2 val = value;
    val.x = glm::min(value.x, screen_dim.x);
    val.x = glm::max(value.x, 0.0f);
    val.y = glm::min(value.y, screen_dim.y);
    val.y = glm::max(value.y, 0.0f);
    s_mouse_state.current_frame_mouse_location = value;
    s_mouse_state.current_frame_mouse_velocity = value - s_mouse_state.last_frame_mouse_location;
}

void input::update_keyboard_key(keyboard_key k, bool value)
{
    ZoneScoped;
    s_keyboard_state.current_frame_key_state[k] = value;
}

void input::update_last_frame()
{
    ZoneScoped;
    s_keyboard_state.last_frame_key_state = s_keyboard_state.current_frame_key_state;

    s_mouse_state.last_frame_mouses_button_state = s_mouse_state.current_frame_mouse_button_state;
    s_mouse_state.last_frame_mouse_location = s_mouse_state.current_frame_mouse_location;
    s_mouse_state.last_frame_mouse_velocity = s_mouse_state.current_frame_mouse_velocity;
    s_mouse_state.last_frame_scroll = s_mouse_state.current_frame_scroll;

    for (int i = 0; i < MAX_SUPPORTED_GAMEPADS; i++)
    {
        s_gamepad_state[i].last_frame_stick_state = s_gamepad_state->current_frame_stick_state;
        s_gamepad_state[i].last_frame_gamepad_button_state = s_gamepad_state->current_frame_gamepad_button_state;
        s_gamepad_state[i].last_frame_stick_state = s_gamepad_state->current_frame_stick_state;
    }
}

