#ifndef STRUCTSANDENUMS
#define STRUCTSANDENUMS

#include <map>

enum class SwitchStates {
  Up = 0,
  Pressed = 1,
  Down = 2,
  Released = 3
};

/*****************************************************************************
 *                                  MAPPINGS                                 *
 *****************************************************************************/
// Mapping for string lookup
static std::map<SwitchStates, const char*> switchStateName{
  { SwitchStates::Up, "Up" },
  { SwitchStates::Pressed, "Pressed" },
  { SwitchStates::Down, "Down" },
  { SwitchStates::Released, "Released" }
};

struct Switch {
  int index;
  int pin;
  SwitchStates state;
  int last;
  int current;
};  // Switch;


#endif  // STRUCTSANDENUMS