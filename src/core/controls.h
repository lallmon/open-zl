#ifndef CONTROLS_H
#define CONTROLS_H

#include <unordered_map>

#include "glfw3.h"

class Button {
    unsigned int downCount = 0;
    unsigned int lastDownCount = 0;

public:
    void update(bool held) { lastDownCount = downCount; downCount = held ? downCount + 1 : 0; }
    bool pressed() const { return downCount == 1; }
    bool held() const { return downCount > 0; }
    bool justReleased() const { return downCount == 0 && lastDownCount != 0;}

    unsigned int holdCount() const { return downCount; }
    bool releasedWithin(int count) const { return lastDownCount > 0 && lastDownCount < count && downCount == 0; }
    bool downEvery(int initial, int thereafter) const {
        return downCount == 1 ||
                initial == downCount ||
                (downCount > initial && ((downCount - initial) % thereafter) == 0);
    }
};

struct Controls {
  float dx = 0.0f, dy = 0.0f;

  Button get(std::string name) const;

  void rebind(std::string name, std::vector<int> keybinds);

  //TODO: add alias options? Depends on user preference
  //void addAlias(std::string new_name, std::string existing_to_bind);

  void update(GLFWwindow * window);

  Controls();

private:
  enum ActionID {
      Left, Up, Down, Right,

      Confirm, Cancel, Start,

      ActionIDCount
  };

  ActionID getActionID(std::string name) const;

  Button buttons[ActionIDCount];

  typedef std::vector<int> Keybindings;
  Keybindings keybindings[ActionIDCount];
};

#endif // CONTROLS_H
