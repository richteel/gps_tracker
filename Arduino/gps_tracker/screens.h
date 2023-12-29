#ifndef SCREENS_H
#define SCREENS_H

#define NUM_OF_SCREENS 6

struct Screen {
  char line1[22];
  char line2[22];
  char line3[22];
  char line4[22];
  char line5[22];
};

Screen screens[NUM_OF_SCREENS];


#endif  // SCREENS_H