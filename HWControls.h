// New LEDs

#define ALL_NOTES_OFF_LED 0
#define NOTE_ON_LED 1
#define NOTE_OFF_LED 2
#define POLY_PRESSURE_LED 3
#define CONTROL_LED 4
#define CHANNEL_PRESSURE_LED 5
#define PITCHBEND_LED 6
#define SYS_EXE_LED 7
#define SONG_POS_LED 8
#define SONG_SEL_LED 9
#define TUNE_REQ_LED 10
#define SUSTAIN_LED 11
#define TIMING_CLOCK_LED 12
#define PROGRAM_LED 13
#define MODULATION_LED 14
#define SYSTEM_RESET_LED 15

#define CHANNEL_16_LED 0
#define CHANNEL_15_LED 1
#define CHANNEL_14_LED 2
#define CHANNEL_13_LED 3
#define CHANNEL_12_LED 4
#define CHANNEL_11_LED 5
#define CHANNEL_10_LED 6
#define CHANNEL_9_LED 7
#define CHANNEL_8_LED 8
#define CHANNEL_7_LED 9
#define CHANNEL_6_LED 10
#define CHANNEL_5_LED 11
#define CHANNEL_4_LED 12
#define CHANNEL_3_LED 13
#define CHANNEL_2_LED 14
#define CHANNEL_1_LED 15

#define POWER_LED 7
#define MIDI_LED 6
#define ERROR_LED 5
#define START_LED 4
#define STOP_LED 3
#define CONTINUE_LED 2
#define ACT_SENSE_LED 1
#define SPARE  0

//RP2040 Pins

#define NEOPIXEL_OUTPUT_FUNC 14
#define NEOPIXEL_OUTPUT_FUNC1 13
#define NEOPIXEL_OUTPUT_CHAN 15

#define UP_BUTTON_PIN     6
#define DOWN_BUTTON_PIN   7
#define SELECT_BUTTON_PIN 8

void setupHardware() {

  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);

}