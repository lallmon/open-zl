#include "controls.h"

#include <iostream>

Button Controls::get(std::string name) const
{
    ActionID id = getActionID(name);
    if (id < ActionIDCount) {
        return buttons[id];
    }
    return Button(); // invalid request; Button() initializes to no activity
}

void Controls::rebind(std::string name, std::vector<int> keybinds)
{
    ActionID id = getActionID(name);
    if (id < ActionIDCount) {
        keybindings[id] = keybinds;
    }
}

void Controls::update(GLFWwindow *window)
{
    bool action_requested[ActionIDCount] = {false};

    for(int i = 0; i < ActionIDCount; ++i) {
        for(int key : keybindings[i]) {
            action_requested[i] |= glfwGetKey(window, key) == GLFW_PRESS;
        }
    }

    // read gamepad states
    GLFWgamepadstate gamepad;
    bool analogue_active = false;
    float analogue_x, analogue_y;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad)) {

        action_requested[Confirm] |= gamepad.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
        action_requested[Cancel] |= gamepad.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS;
        action_requested[Start] |= gamepad.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS;

        { // left analogue
            analogue_x = gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            analogue_y = gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
            if (std::fabs(analogue_x) > 0.2f) {
                analogue_active = true;
                action_requested[Left] |= analogue_x < 0;
                action_requested[Right] |= analogue_x > 0;
            }
            if (std::fabs(analogue_y) > 0.2f) {
                analogue_active = true;
                action_requested[Up] |= analogue_y < 0;
                action_requested[Down] |= analogue_y > 0;
            }
        }

        { // dpad
            int x = (gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS ? -1 : 0) + (gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS ? 1 : 0);
            action_requested[Left] |= x < 0;
            action_requested[Right] |= x > 0;

            int y = (gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS ? -1 : 0) + (gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS ? 1 : 0);
            action_requested[Up] |= y < 0;
            action_requested[Down] |= y > 0;
        }
    }

    if (analogue_active) {
        dx = analogue_x;
        dy = analogue_y;
    } else {
        dx = float(action_requested[Right]) - float(action_requested[Left]);
        dy = float(action_requested[Down]) - float(action_requested[Up]);
    }

    for(int i = 0; i < ActionIDCount; ++i) {
        buttons[i].update(action_requested[i]);
    }
}

Controls::Controls()
{
    rebind("up", {GLFW_KEY_UP, GLFW_KEY_W});
    rebind("left", {GLFW_KEY_LEFT, GLFW_KEY_A});
    rebind("right", {GLFW_KEY_RIGHT, GLFW_KEY_D});
    rebind("down", {GLFW_KEY_DOWN, GLFW_KEY_S});
    rebind("confirm", {GLFW_KEY_Z, GLFW_KEY_SLASH});
    rebind("cancel", {GLFW_KEY_X, GLFW_KEY_PERIOD});
    rebind("start", {GLFW_KEY_ENTER});
}

Controls::ActionID Controls::getActionID(std::string name) const
{
    const char * names[ActionIDCount] = {
        "left", "up", "down", "right", "confirm", "cancel", "start"
    };
    for(int i = 0; i < ActionIDCount; ++i) {
        if (names[i] == name) {
            return ActionID(i);
        }
    }
    std::clog << "WARNING: Requested Action ID (" << name.c_str() << ") does not exist." << std::endl;
    return ActionIDCount;
}
