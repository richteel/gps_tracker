#ifndef CONFIG
#define CONFIG

#define NETWORKINFOARRAYSIZE 8

struct Config {
  int gmtOffset;
  char tzAbbr[4];
  bool metric;
  int displayOffSecs;
};

#endif  // CONFIG