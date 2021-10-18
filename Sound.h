// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds

enum Direction {FORWARDS, BACKWARDS};

void Sound_Init(void);
void flohwalzerSound(enum Direction d);
