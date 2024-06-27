#include "ruleset.h"

char RuleStateManager(char state, char cState) {
  if (cState == 0) {
    switch (state){
      case 3:
        return 1;
      default:
        return 0;
    }
  } else {
    switch (state){
      case 2:
        return 1;
      case 3:
        return 1;
      default:
        return 0;
    }
  }
}