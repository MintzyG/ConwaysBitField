#include "rulseset.h"

char RuleStateManager(char state, char cState) {
  if (cState == 0) {
    if (state == 23) {
      return 1;
    }    else if (state == 3) {
      return 1;
    } else {
      return 0;
    }
  } else {
  }
}