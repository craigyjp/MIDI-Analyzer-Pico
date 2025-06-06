#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#include "Parameters.h"
#include "HWControls.h"
#include <EEPROM.h>

#define MIDI_NAMESPACE midi

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

//MIDI 5 Pin DIN
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);

// USB MIDI object
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

Adafruit_NeoPixel pixelchan(16, NEOPIXEL_OUTPUT_CHAN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel pixelfunc(16, NEOPIXEL_OUTPUT_FUNC, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel pixelfunc1(8, NEOPIXEL_OUTPUT_FUNC1, NEO_RGB + NEO_KHZ800);

Bounce upButton = Bounce();
Bounce downButton = Bounce();
Bounce selButton = Bounce();

void setup() {

  tft.init();
  tft.setRotation(3);
  tft.invertDisplay(false);
  tft.fillScreen(TFT_BLACK);

  renderBootUpPage();
  delay(500);
  display_timer = millis();

  setupHardware();

  TinyUSBDevice.setManufacturerDescriptor("Craig Barnes");
  TinyUSBDevice.setProductDescriptor("MIDI Analyzer");
  TinyUSBDevice.begin();

  // Initialize MIDI, and listen to all MIDI channels
  // This will also call usb_midi's begin()
  MIDI.begin(MIDI_CHANNEL_OMNI);
  //MIDI.turnThruOff();  // turn off echo

  MIDI1.begin(MIDI_CHANNEL_OMNI);
  MIDI1.turnThruOn(midi::Thru::Mode::Off);

  upButton.attach(UP_BUTTON_PIN);
  upButton.interval(20);  // Debounce time in ms

  downButton.attach(DOWN_BUTTON_PIN);
  downButton.interval(20);

  selButton.attach(SELECT_BUTTON_PIN);
  selButton.interval(20);

  pixelchan.begin();
  pixelfunc.begin();
  pixelfunc1.begin();
  pixelfunc1.setPixelColor(POWER_LED, 0, 255, 0);

  EEPROM.begin(32);  // argument is the number of bytes you want to use

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  loadFilters();
}

void loadFilters() {

  note_filter = EEPROM.read(EEPROM_NOTES);
  if (note_filter < 0 || note_filter > 1) {
    EEPROM.write(EEPROM_NOTES, 0);
    EEPROM.commit();
  }

  aftertouch_filter = EEPROM.read(EEPROM_AFTERTOUCH);
  if (aftertouch_filter < 0 || aftertouch_filter > 1) {
    EEPROM.write(EEPROM_AFTERTOUCH, 0);
    EEPROM.commit();
  }

  bend_filter = EEPROM.read(EEPROM_PITCHBEND);
  if (bend_filter < 0 || bend_filter > 1) {
    EEPROM.write(EEPROM_PITCHBEND, 0);
    EEPROM.commit();
  }

  mod_filter = EEPROM.read(EEPROM_MODULATION);
  if (mod_filter < 0 || mod_filter > 1) {
    EEPROM.write(EEPROM_MODULATION, 0);
    EEPROM.commit();
  }

  program_filter = EEPROM.read(EEPROM_PROGRAM);
  if (program_filter < 0 || program_filter > 1) {
    EEPROM.write(EEPROM_PROGRAM, 0);
    EEPROM.commit();
  }

  realtime_filter = EEPROM.read(EEPROM_REALTIME);
  if (realtime_filter < 0 || realtime_filter > 1) {
    EEPROM.write(EEPROM_REALTIME, 0);
    EEPROM.commit();
  }

  system_filter = EEPROM.read(EEPROM_SYSTEM);
  if (system_filter < 0 || system_filter > 1) {
    EEPROM.write(EEPROM_SYSTEM, 0);
    EEPROM.commit();
  }

  controllers_filter = EEPROM.read(EEPROM_CONTROLLERS);
  if (controllers_filter < 0 || controllers_filter > 1) {
    EEPROM.write(EEPROM_CONTROLLERS, 0);
    EEPROM.commit();
  }

  active_filter = EEPROM.read(EEPROM_ACTIVE);
  if (active_filter < 0 || active_filter > 1) {
    EEPROM.write(EEPROM_ACTIVE, 0);
    EEPROM.commit();
  }

  sustain_filter = EEPROM.read(EEPROM_SUSTAIN);
  if (sustain_filter < 0 || sustain_filter > 1) {
    EEPROM.write(EEPROM_SUSTAIN, 0);
    EEPROM.commit();
  }

  all_notes_off_filter = EEPROM.read(EEPROM_NOTES_OFF);
  if (all_notes_off_filter < 0 || all_notes_off_filter > 1) {
    EEPROM.write(EEPROM_NOTES_OFF, 0);
    EEPROM.commit();
  }
}

void displayValue(byte value) {
  switch (value) {
    case 0:
      tft.setCursor(120, 25);
      tft.print("Off");
      break;

    case 1:
      tft.setCursor(120, 25);
      tft.print("On");
      break;
  }
}

void updateParam() {
  switch (paramNumber) {
    case 0:
      note_filter = !note_filter;
      EEPROM.write(EEPROM_NOTES, note_filter);
      break;

    case 1:
      aftertouch_filter = !aftertouch_filter;
      EEPROM.write(EEPROM_AFTERTOUCH, aftertouch_filter);
      break;

    case 2:
      bend_filter = !bend_filter;
      EEPROM.write(EEPROM_PITCHBEND, bend_filter);
      break;

    case 3:
      mod_filter = !mod_filter;
      EEPROM.write(EEPROM_MODULATION, mod_filter);
      break;

    case 4:
      program_filter = !program_filter;
      EEPROM.write(EEPROM_PROGRAM, program_filter);
      break;

    case 5:
      realtime_filter = !realtime_filter;
      EEPROM.write(EEPROM_REALTIME, realtime_filter);
      break;

    case 6:
      system_filter = !system_filter;
      EEPROM.write(EEPROM_SYSTEM, system_filter);
      break;

    case 7:
      controllers_filter = !controllers_filter;
      EEPROM.write(EEPROM_CONTROLLERS, controllers_filter);
      break;

    case 8:
      active_filter = !active_filter;
      EEPROM.write(EEPROM_ACTIVE, active_filter);
      break;

    case 9:
      sustain_filter = !sustain_filter;
      EEPROM.write(EEPROM_SUSTAIN, sustain_filter);
      break;

    case 10:
      all_notes_off_filter = !all_notes_off_filter;
      EEPROM.write(EEPROM_NOTES_OFF, all_notes_off_filter);
      break;
  }
  EEPROM.commit();
  oldparamNumber = -1;
}


void updateDisplay() {

  if (!sendData) {
    if (oldparamNumber != paramNumber) {

      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.print("Filtering");
      tft.setCursor(0, 25);
      tft.setTextColor(TFT_YELLOW);
      switch (paramNumber) {
        case 0:
          tft.println("Notes");
          displayValue(note_filter);
          break;

        case 1:
          tft.println("After");
          tft.setCursor(0, 50);
          tft.println("Touch");
          displayValue(aftertouch_filter);
          break;

        case 2:
          tft.println("PitchBend");
          displayValue(bend_filter);
          break;

        case 3:
          tft.println("Mod");
          tft.setCursor(0, 50);
          tft.println("Wheel");
          displayValue(mod_filter);
          break;

        case 4:
          tft.println("Program");
          displayValue(program_filter);
          break;

        case 5:
          tft.println("Realtime");
          displayValue(realtime_filter);
          break;

        case 6:
          tft.println("System");
          displayValue(system_filter);
          break;

        case 7:
          tft.println("All");
          tft.setCursor(0, 50);
          tft.println("Controllers");
          displayValue(controllers_filter);
          break;

        case 8:
          tft.println("Active");
          tft.setCursor(0, 50);
          tft.println("Sense");
          displayValue(active_filter);
          break;

        case 9:
          tft.println("Sustain");
          tft.setCursor(0, 50);
          tft.println("Pedal");
          displayValue(sustain_filter);
          break;

        case 10:
          tft.println("All Notes");
          tft.setCursor(0, 50);
          tft.println("Off");
          displayValue(all_notes_off_filter);
          break;
      }
    }
    oldparamNumber = paramNumber;
  }

  if (sendData) {
    if (oldparamNumber != paramNumber) {

      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.print("Transmitting");
      tft.setCursor(0, 25);
      tft.setTextColor(TFT_YELLOW);
      switch (paramNumber) {
        case 0:
          hasValue = false;
          tft.print("Note    ");
          if (data1Select) {
            tft.println(chosenMessage);
          }
          if (sendMessage) {
            MIDI1.sendNoteOn(chosenMessage, 127, outChannel);
            pixelfunc.setPixelColor(NOTE_ON_LED, 255, 0, 0);
            pixelfunc.show();
            noteOn_timer = millis();
            channelLED(outChannel);
            delay(200);
            MIDI1.sendNoteOff(chosenMessage, 0, outChannel);
            pixelfunc.setPixelColor(NOTE_OFF_LED, 0, 255, 0);
            noteOff_timer = millis();
            channelLED(outChannel);
            sendMessage = false;
          }
          break;

        case 1:
          hasValue = true;
          tft.print("CC Num  ");
          if (data1Select) {
            tft.println(chosenMessage);
          }
          if (data2Select) {
            tft.setCursor(0, 50);
            tft.print("Value   ");
            tft.print(valueMessage);
          }
          if (sendMessage) {
            MIDI1.sendControlChange(chosenMessage, valueMessage, outChannel);
            sendMessage = false;
            pixelfunc.setPixelColor(CONTROL_LED, 255, 0, 0);
            control_timer = millis();
            channelLED(outChannel);
          }
          break;

        case 2:
          hasValue = false;
          tft.print("Prg Chg ");
          if (data1Select) {
            tft.println(chosenMessage);
          }
          if (sendMessage) {
            MIDI1.sendProgramChange(chosenMessage, outChannel);
            sendMessage = false;
            pixelfunc.setPixelColor(PROGRAM_LED, 255, 0, 0);
            program_timer = millis();
            channelLED(outChannel);
          }
          break;

        case 3:
          hasValue = false;
          tft.print("Pressure ");
          if (data1Select) {
            tft.println(chosenMessage);
          }
          if (sendMessage) {
            //Serial.println(1);
            MIDI1.sendAfterTouch(chosenMessage, outChannel);
            sendMessage = false;
            pixelfunc.setPixelColor(CHANNEL_PRESSURE_LED, 255, 0, 0);
            channelPressure_timer = millis();
            channelLED(outChannel);
          }
          break;

        case 4:
          hasValue = false;
          tft.print("Set Chan ");
          if (data1Select) {
            tft.println(chosenMessage);
          }
          if (sendMessage) {
            outChannel = chosenMessage;
            sendMessage = false;
            tft.setCursor(0, 50);
            tft.print("Set");
          }
          break;
      }
    }
    oldparamNumber = paramNumber;
  }
}

void checkSwitches() {

  upButton.update();
  downButton.update();
  selButton.update();

  if (upButton.fell()) {
    if (!sendData) {
      paramNumber++;
      if (paramNumber > 10) {
        paramNumber = 0;
      }
    }

    if (sendData && !data1Select) {
      paramNumber++;
      if (paramNumber > 4) {
        paramNumber = 0;
      }
    }

    if (sendData && data1Select && !data2Select) {
      chosenMessage++;
      if (paramNumber == 4) {
        if (chosenMessage > 16) {
          chosenMessage = 1;
        }
      } else {
        if (chosenMessage > 127) {
          chosenMessage = 0;
        }
      }
      oldparamNumber = -1;
    }

    if (sendData && data1Select && data2Select) {
      valueMessage++;
      if (valueMessage > 127) {
        valueMessage = 0;
      }
      oldparamNumber = -1;
    }
    display_timer = millis();
  }


  if (downButton.fell()) {
    if (!sendData) {
      paramNumber--;
      if (paramNumber < 0) {
        paramNumber = 10;
      }
    }

    if (sendData && !data1Select) {
      paramNumber--;
      if (paramNumber < 0) {
        paramNumber = 4;
      }
    }

    if (sendData && data1Select && !data2Select) {
      chosenMessage--;
      if (paramNumber == 4) {
        if (chosenMessage < 1) {
          chosenMessage = 16;
        }
      } else {
        if (chosenMessage < 0) {
          chosenMessage = 127;
        }
      }
      oldparamNumber = -1;
    }

    if (sendData && data1Select && data2Select) {
      valueMessage--;
      if (valueMessage < 0) {
        valueMessage = 127;
      }
      oldparamNumber = -1;
    }
    display_timer = millis();
  }

  if (selButton.fell()) {
    selectPressTime = millis();
    selectWasHeld = false;
  }
  if (selButton.read() == LOW && !selectWasHeld) {
    if (millis() - selectPressTime > holdThreshold) {
      selectWasHeld = true;
      sendData = !sendData;
      paramNumber = 0;
      display_timer = millis();
      oldparamNumber = -1;
      if (data1Select) {
        data1Select = !data1Select;
        sendData = true;
      }
      if (data2Select) {
        data2Select = !data2Select;
      }
    }
  }
  if (selButton.rose()) {  // button released
    if (!selectWasHeld) {
      if (!sendData) {
        display_timer = millis();
        updateParam();
      }

      if ((sendData && data1Select && data2Select && hasValue) || (sendData && data1Select && !data2Select && !hasValue)) {
        sendMessage = true;
        display_timer = millis();
        oldparamNumber = -1;
      }

      if (sendData && data1Select && data2Select) {
        sendMessage = true;
        display_timer = millis();
        oldparamNumber = -1;
      }

      if (sendData && data1Select && !data2Select && hasValue) {
        data2Select = true;
        display_timer = millis();
        oldparamNumber = -1;
      }

      if (sendData && !data1Select && !data2Select) {
        data1Select = true;
        display_timer = millis();
        oldparamNumber = -1;
      }
    }
  }
}

void displayIncoming(byte type, int16_t data1, int data2, byte channel) {

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);

  tft.setCursor(0, 0);
  tft.print("Note On");
  tft.setCursor(84, 0);
  tft.print("Velocity");

  tft.setCursor(0, 13);
  tft.print("Note Off");
  tft.setCursor(84, 13);
  tft.print("Velocity");

  tft.setCursor(0, 26);
  tft.print("PitchBend");

  tft.setCursor(0, 39);
  tft.print("CC Message");
  tft.setCursor(91, 39);
  tft.print("Value");

  tft.setCursor(0, 52);
  tft.print("PGM Change");

  tft.setCursor(0, 65);
  tft.print("Pressure");

  refresh_screen = false;


  tft.setTextColor(TFT_YELLOW);

  switch (type) {
    case midi::NoteOff:
      tft.setTextColor(TFT_YELLOW);
      tft.setCursor(60, 13);
      tft.print(data1);
      tft.setCursor(140, 13);
      tft.print(data2);
      break;

    case midi::NoteOn:
      tft.setCursor(60, 0);
      tft.print(data1);
      tft.setCursor(140, 0);
      tft.print(data2);
      break;

    case midi::ControlChange:
      tft.setTextColor(TFT_WHITE);
      switch (data1) {
        case 1:
          tft.setTextColor(TFT_WHITE);
          tft.setCursor(0, 39);
          break;

        case 64:
          tft.setTextColor(TFT_WHITE);
          tft.setCursor(0, 39);
          break;

        case 123:
          tft.setTextColor(TFT_WHITE);
          tft.setCursor(0, 39);
          break;

        default:
          tft.setTextColor(TFT_WHITE);
          tft.setCursor(0, 39);
          break;
      }
      tft.setTextColor(TFT_YELLOW);
      tft.setCursor(70, 39);
      tft.print(data1);
      tft.setCursor(135, 39);
      tft.print(data2);
      break;

    case midi::ProgramChange:
      tft.setCursor(70, 52);
      tft.print(data1);
      break;

    case midi::AfterTouchChannel:
      tft.setCursor(70, 65);
      tft.print(data1);
      break;

    case midi::PitchBend:
      tft.setCursor(70, 26);
      //tft.print("Value ");
      tft.print(data1);
      break;
  }
  display_timer = millis();
}

void loop() {

  bool activity = false;

  // DIN MIDI
  if (MIDI1.read()) {
    // get a MIDI IN1 (Serial) message
    byte type = MIDI1.getType();
    byte channel = MIDI1.getChannel();
    byte data1 = MIDI1.getData1();
    byte data2 = MIDI1.getData2();

    switch (type) {
      case midi::NoteOff:  // 0x80
        if (!note_filter) {
          MIDI1.send(MIDI1.getType(), MIDI1.getData1(), MIDI1.getData2(), MIDI1.getChannel());
        }
        pixelfunc.setPixelColor(NOTE_OFF_LED, 0, 255, 0);
        noteOff_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, data2, channel);
        break;

      case midi::NoteOn:  // 0x90
        if (!note_filter) {
          MIDI1.send(MIDI1.getType(), MIDI1.getData1(), MIDI1.getData2(), MIDI1.getChannel());
        }
        pixelfunc.setPixelColor(NOTE_ON_LED, 255, 0, 0);
        noteOn_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, data2, channel);
        break;

      case midi::AfterTouchPoly:  // 0xA0
        pixelfunc.setPixelColor(POLY_PRESSURE_LED, 255, 0, 0);
        polyPressure_timer = millis();
        channelLED(channel);
        break;

      case midi::ControlChange:  // 0xB0
        switch (data1) {

          case 123:
            if (!all_notes_off_filter) {
              MIDI1.sendControlChange(data1, data2, channel);
            }
            pixelfunc.setPixelColor(ALL_NOTES_OFF_LED, 255, 0, 0);
            allNotes_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;

          case 64:
            if (!sustain_filter) {
            }
            pixelfunc.setPixelColor(SUSTAIN_LED, 255, 0, 0);
            sustain_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;

          case 1:
            if (!mod_filter) {
              MIDI1.sendControlChange(data1, data2, channel);
            }
            pixelfunc.setPixelColor(MODULATION_LED, 255, 0, 0);
            modulation_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;

          default:
            if (!controllers_filter) {
              MIDI1.sendControlChange(data1, data2, channel);
            }
            pixelfunc.setPixelColor(CONTROL_LED, 255, 0, 0);
            control_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;
        }
        break;

      case midi::ProgramChange:  // 0xC0
        if (!program_filter) {
          MIDI1.sendProgramChange(data1, channel);
        }
        pixelfunc.setPixelColor(PROGRAM_LED, 255, 0, 0);
        program_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, 0, channel);
        break;

      case midi::AfterTouchChannel:  // 0xD0
        if (!aftertouch_filter) {
          MIDI1.sendAfterTouch(data1, channel);
        }
        pixelfunc.setPixelColor(CHANNEL_PRESSURE_LED, 255, 0, 0);
        channelPressure_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, 0, channel);
        break;

      case midi::PitchBend:  // 0xE0
        pitchBendValue = (data2 << 7) | data1;
        signedPitchBendValue = pitchBendValue - 8192;
        if (!bend_filter) {
          MIDI1.sendPitchBend(signedPitchBendValue, channel);
        }
        pixelfunc.setPixelColor(PITCHBEND_LED, 255, 0, 0);
        pitchbend_timer = millis();
        channelLED(channel);
        displayIncoming(type, signedPitchBendValue, 0, channel);
        break;

      case midi::SystemExclusive:  // 0xF0
        pixelfunc.setPixelColor(SYS_EXE_LED, 255, 0, 0);
        sysex_timer = millis();
        channelLED(channel);
        break;

      case midi::SongPosition:  // 0xF2
        if (!system_filter) {
          MIDI1.sendSongPosition(data1);
        }
        pixelfunc.setPixelColor(SONG_POS_LED, 255, 0, 0);
        songpos_timer = millis();
        break;

      case midi::SongSelect:  // 0xF3
        if (!system_filter) {
          MIDI1.sendSongSelect(data1);
        }
        pixelfunc.setPixelColor(SONG_SEL_LED, 255, 0, 0);
        songselect_timer = millis();
        break;

      case midi::TuneRequest:  // 0xF6
        if (!system_filter) {
          MIDI1.sendTuneRequest();
        }
        pixelfunc.setPixelColor(TUNE_REQ_LED, 255, 0, 0);
        tuneRequest_timer = millis();
        break;

      case midi::Clock:  // 0xF8
        if (!realtime_filter) {
          MIDI1.sendRealTime(MIDI1.getType());
        }
        midiClockCounter++;
        if (midiClockCounter >= 24) {
          pixelfunc.setPixelColor(TIMING_CLOCK_LED, 255, 255, 255);
          clock_timer = millis();
          midiClockCounter = 0;
        }
        break;

      case midi::Start:  // 0xFA
        if (!realtime_filter) {
          MIDI1.sendRealTime(MIDI1.getType());
        }
        pixelfunc1.setPixelColor(START_LED, 255, 0, 0);
        songstart_timer = millis();
        break;

      case midi::Continue:  // 0xFB
        if (!realtime_filter) {
          MIDI1.sendRealTime(MIDI1.getType());
        }
        pixelfunc1.setPixelColor(CONTINUE_LED, 255, 0, 0);
        songcontinue_timer = millis();
        break;

      case midi::Stop:  // 0xFC
        if (!realtime_filter) {
          MIDI1.sendRealTime(MIDI1.getType());
        }
        pixelfunc1.setPixelColor(STOP_LED, 0, 255, 0);
        songstop_timer = millis();
        break;

      case midi::ActiveSensing:  // 0xFE
        if (!active_filter) {
          MIDI1.sendRealTime(MIDI1.getType());
        }
        pixelfunc1.setPixelColor(ACT_SENSE_LED, 255, 0, 0);
        activeSensing_timer = millis();
        break;

      case midi::SystemReset:  // 0xFF
        if (!realtime_filter) {
          MIDI1.sendRealTime(MIDI1.getType());
        }
        pixelfunc1.setPixelColor(SYSTEM_RESET_LED, 255, 0, 0);
        systemReset_timer = millis();
        break;

      default:
        pixelfunc1.setPixelColor(ERROR_LED, 255, 0, 0);
        error_timer = millis();
    }
    activity = true;
  }

  // this is now the USB
  if (MIDI.read()) {
    // get a MIDI IN1 (Serial) message
    byte type = MIDI.getType();
    byte channel = MIDI.getChannel();
    byte data1 = MIDI.getData1();
    byte data2 = MIDI.getData2();


    switch (type) {
      case midi::NoteOff:  // 0x80
        if (!note_filter) {
          MIDI1.sendNoteOff(data1, data2, channel);
        }
        pixelfunc.setPixelColor(NOTE_OFF_LED, 0, 255, 0);
        noteOff_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, data2, channel);
        break;

      case midi::NoteOn:  // 0x90
        if (!note_filter) {
          MIDI1.sendNoteOn(data1, data2, channel);
        }
        pixelfunc.setPixelColor(NOTE_ON_LED, 255, 0, 0);
        pixelfunc.show();
        noteOn_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, data2, channel);
        break;

      case midi::AfterTouchPoly:  // 0xA0
        //MIDI1.sendPolyPressure(data1, data2, channel);
        pixelfunc.setPixelColor(POLY_PRESSURE_LED, 255, 0, 0);
        polyPressure_timer = millis();
        channelLED(channel);
        break;

      case midi::ControlChange:  // 0xB0
        switch (data1) {

          case 123:
            if (!all_notes_off_filter) {
              MIDI1.sendControlChange(data1, data2, channel);
            }
            pixelfunc.setPixelColor(ALL_NOTES_OFF_LED, 255, 0, 0);
            allNotes_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;

          case 64:
            if (!sustain_filter) {
              MIDI1.sendControlChange(data1, data2, channel);
            }
            pixelfunc.setPixelColor(SUSTAIN_LED, 255, 0, 0);
            sustain_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;

          case 1:
            if (!mod_filter) {
              MIDI1.sendControlChange(data1, data2, channel);
            }
            pixelfunc.setPixelColor(MODULATION_LED, 255, 0, 0);
            modulation_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;

          default:
            if (!controllers_filter) {
              MIDI1.sendControlChange(data1, data2, channel);
            }
            pixelfunc.setPixelColor(CONTROL_LED, 255, 0, 0);
            control_timer = millis();
            channelLED(channel);
            displayIncoming(type, data1, data2, channel);
            break;
        }
        break;

      case midi::ProgramChange:  // 0xC0
        if (!program_filter) {
          MIDI1.sendProgramChange(data1, channel);
        }
        pixelfunc.setPixelColor(PROGRAM_LED, 255, 0, 0);
        program_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, 0, channel);
        break;

      case midi::AfterTouchChannel:  // 0xD0
        if (!aftertouch_filter) {
          MIDI1.sendAfterTouch(data1, channel);
        }
        pixelfunc.setPixelColor(CHANNEL_PRESSURE_LED, 255, 0, 0);
        channelPressure_timer = millis();
        channelLED(channel);
        displayIncoming(type, data1, 0, channel);
        break;

      case midi::PitchBend:  // 0xE0
        pitchBendValue = (data2 << 7) | data1;
        signedPitchBendValue = pitchBendValue - 8192;
        if (!bend_filter) {
          MIDI1.sendPitchBend(signedPitchBendValue, channel);
        }
        pixelfunc.setPixelColor(PITCHBEND_LED, 255, 0, 0);
        pitchbend_timer = millis();
        channelLED(channel);
        displayIncoming(type, signedPitchBendValue, 0, channel);
        break;

      case midi::SystemExclusive:  // 0xF0
        pixelfunc.setPixelColor(SYS_EXE_LED, 255, 0, 0);
        sysex_timer = millis();
        channelLED(channel);
        break;

      case midi::SongPosition:  // 0xF2
        if (!system_filter) {
          MIDI1.sendSongPosition(data1);
        }
        pixelfunc.setPixelColor(SONG_POS_LED, 255, 0, 0);
        songpos_timer = millis();
        break;

      case midi::SongSelect:  // 0xF3
        if (!system_filter) {
          MIDI1.sendSongSelect(data1);
        }
        pixelfunc.setPixelColor(SONG_SEL_LED, 255, 0, 0);
        songselect_timer = millis();
        break;

      case midi::TuneRequest:  // 0xF6
        if (!system_filter) {
          MIDI1.sendTuneRequest();
        }
        pixelfunc.setPixelColor(TUNE_REQ_LED, 255, 0, 0);
        tuneRequest_timer = millis();
        break;

      case midi::Clock:  // 0xF8
        //MIDI1.sendRealTime(MIDI.getType());
        midiClockCounter++;
        if (midiClockCounter >= 24) {
          pixelfunc.setPixelColor(TIMING_CLOCK_LED, 255, 255, 255);
          clock_timer = millis();
          midiClockCounter = 0;
        }
        break;

      case midi::Start:  // 0xFA
        //MIDI1.sendRealTime(MIDI.getType());
        pixelfunc1.setPixelColor(START_LED, 255, 0, 0);
        songstart_timer = millis();
        break;

      case midi::Continue:  // 0xFB
        //MIDI1.sendRealTime(MIDI.getType());
        pixelfunc1.setPixelColor(CONTINUE_LED, 255, 0, 0);
        songcontinue_timer = millis();
        break;

      case midi::Stop:  // 0xFC
        //MIDI.sendRealTime(MIDI.getType());
        pixelfunc1.setPixelColor(STOP_LED, 255, 0, 0);
        songstop_timer = millis();
        break;

      case midi::ActiveSensing:  // 0xFE
        //MIDI1.sendRealTime(MIDI.getType());
        pixelfunc1.setPixelColor(ACT_SENSE_LED, 255, 0, 0);
        activeSensing_timer = millis();
        break;

      case midi::SystemReset:  // 0xFF
        //MIDI1.sendRealTime(MIDI.getType());
        pixelfunc1.setPixelColor(SYSTEM_RESET_LED, 255, 0, 0);
        systemReset_timer = millis();
        break;

      default:
        pixelfunc.setPixelColor(ERROR_LED, 255, 0, 0);
        error_timer = millis();
    }
    activity = true;
  }

  ledsOff();
  checkSwitches();
  updateDisplay();


  pixelfunc.show();
  pixelfunc1.show();
  pixelchan.show();

  // blink the LED when any activity has happened
  if (activity) {
    pixelfunc1.setPixelColor(MIDI_LED, 0, 0, 255);
    pixelfunc1.show();
    ledOn_timer = millis();
  }
}

void renderBootUpPage() {
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, 160, 60, TFT_WHITE);
  //tft.fillRect(88, 30, 61, 11, TFT_WHITE);
  tft.setCursor(3, 10);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.println("MIDI ANALYSER");
  tft.setCursor(3, 35);
  tft.setTextSize(2);
  tft.print("& FILTER ");
  tft.setTextColor(TFT_RED);
  tft.println(VERSION);
}

void ledsOff() {


  if ((ledOn_timer > 0) && (millis() - ledOn_timer > 20)) {
    pixelfunc1.setPixelColor(MIDI_LED, 0, 0, 0);
    ledOn_timer = 0;
  }

  if ((noteOn_timer > 0) && (millis() - noteOn_timer > 60)) {
    pixelfunc.setPixelColor(NOTE_ON_LED, 0, 0, 0);
    noteOn_timer = 0;
  }

  if ((noteOff_timer > 0) && (millis() - noteOff_timer > 60)) {
    pixelfunc.setPixelColor(NOTE_OFF_LED, 0, 0, 0);
    noteOff_timer = 0;
  }

  if ((control_timer > 0) && (millis() - control_timer > 60)) {
    pixelfunc.setPixelColor(CONTROL_LED, 0, 0, 0);
    control_timer = 0;
  }

  if ((polyPressure_timer > 0) && (millis() - polyPressure_timer > 60)) {
    pixelfunc.setPixelColor(POLY_PRESSURE_LED, 0, 0, 0);
    polyPressure_timer = 0;
  }

  if ((pitchbend_timer > 0) && (millis() - pitchbend_timer > 60)) {
    pixelfunc.setPixelColor(PITCHBEND_LED, 0, 0, 0);
    pitchbend_timer = 0;
  }

  if ((allNotes_timer > 0) && (millis() - allNotes_timer > 60)) {
    pixelfunc.setPixelColor(ALL_NOTES_OFF_LED, 0, 0, 0);
    allNotes_timer = 0;
  }

  if ((program_timer > 0) && (millis() - program_timer > 60)) {
    pixelfunc.setPixelColor(PROGRAM_LED, 0, 0, 0);
    program_timer = 0;
  }

  if ((modulation_timer > 0) && (millis() - modulation_timer > 60)) {
    pixelfunc.setPixelColor(MODULATION_LED, 0, 0, 0);
    modulation_timer = 0;
  }

  if ((channelPressure_timer > 0) && (millis() - channelPressure_timer > 60)) {
    pixelfunc.setPixelColor(CHANNEL_PRESSURE_LED, 0, 0, 0);
    channelPressure_timer = 0;
  }

  if ((sysex_timer > 0) && (millis() - sysex_timer > 60)) {
    pixelfunc.setPixelColor(SYS_EXE_LED, 0, 0, 0);
    sysex_timer = 0;
  }

  if ((songpos_timer > 0) && (millis() - songpos_timer > 60)) {
    pixelfunc.setPixelColor(SONG_POS_LED, 0, 0, 0);
    songpos_timer = 0;
  }

  if ((songselect_timer > 0) && (millis() - songselect_timer > 60)) {
    pixelfunc.setPixelColor(SONG_SEL_LED, 0, 0, 0);
    songselect_timer = 0;
  }

  if ((songstart_timer > 0) && (millis() - songstart_timer > 60)) {
    pixelfunc1.setPixelColor(START_LED, 0, 0, 0);
    songstart_timer = 0;
  }

  if ((songstop_timer > 0) && (millis() - songstop_timer > 60)) {
    pixelfunc1.setPixelColor(STOP_LED, 0, 0, 0);
    songstop_timer = 0;
  }

  if ((songcontinue_timer > 0) && (millis() - songcontinue_timer > 60)) {
    pixelfunc1.setPixelColor(CONTINUE_LED, 0, 0, 0);
    songcontinue_timer = 0;
  }

  if ((activeSensing_timer > 0) && (millis() - activeSensing_timer > 30)) {
    pixelfunc1.setPixelColor(ACT_SENSE_LED, 0, 0, 0);
    activeSensing_timer = 0;
  }

  if ((systemReset_timer > 0) && (millis() - systemReset_timer > 60)) {
    pixelfunc1.setPixelColor(SYSTEM_RESET_LED, 0, 0, 0);
    systemReset_timer = 0;
  }

  if ((tuneRequest_timer > 0) && (millis() - tuneRequest_timer > 60)) {
    pixelfunc.setPixelColor(TUNE_REQ_LED, 0, 0, 0);
    tuneRequest_timer = 0;
  }

  if ((clock_timer > 0) && (millis() - clock_timer > 10)) {
    pixelfunc.setPixelColor(TIMING_CLOCK_LED, 0, 0, 0);
    clock_timer = 0;
  }

  if ((sustain_timer > 0) && (millis() - sustain_timer > 60)) {
    pixelfunc.setPixelColor(SUSTAIN_LED, 0, 0, 0);
    sustain_timer = 0;
  }

  if ((error_timer > 0) && (millis() - error_timer > 60)) {
    pixelfunc1.setPixelColor(ERROR_LED, 0, 0, 0);
    error_timer = 0;
  }

  if ((channel1_timer > 0) && (millis() - channel1_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_1_LED, 0, 0, 0);
    channel1_timer = 0;
  }

  if ((channel2_timer > 0) && (millis() - channel2_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_2_LED, 0, 0, 0);
    channel2_timer = 0;
  }

  if ((channel3_timer > 0) && (millis() - channel3_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_3_LED, 0, 0, 0);
    channel3_timer = 0;
  }

  if ((channel4_timer > 0) && (millis() - channel4_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_4_LED, 0, 0, 0);
    channel4_timer = 0;
  }

  if ((channel5_timer > 0) && (millis() - channel5_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_5_LED, 0, 0, 0);
    channel5_timer = 0;
  }

  if ((channel6_timer > 0) && (millis() - channel6_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_6_LED, 0, 0, 0);
    channel6_timer = 0;
  }

  if ((channel7_timer > 0) && (millis() - channel7_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_7_LED, 0, 0, 0);
    channel7_timer = 0;
  }

  if ((channel8_timer > 0) && (millis() - channel8_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_8_LED, 0, 0, 0);
    channel8_timer = 0;
  }

  if ((channel9_timer > 0) && (millis() - channel9_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_9_LED, 0, 0, 0);
    channel9_timer = 0;
  }

  if ((channel10_timer > 0) && (millis() - channel10_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_10_LED, 0, 0, 0);
    channel10_timer = 0;
  }

  if ((channel11_timer > 0) && (millis() - channel11_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_11_LED, 0, 0, 0);
    channel11_timer = 0;
  }

  if ((channel12_timer > 0) && (millis() - channel12_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_12_LED, 0, 0, 0);
    channel12_timer = 0;
  }

  if ((channel13_timer > 0) && (millis() - channel13_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_13_LED, 0, 0, 0);
    channel13_timer = 0;
  }

  if ((channel14_timer > 0) && (millis() - channel14_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_14_LED, 0, 0, 0);
    channel14_timer = 0;
  }

  if ((channel15_timer > 0) && (millis() - channel15_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_15_LED, 0, 0, 0);
    channel15_timer = 0;
  }

  if ((channel16_timer > 0) && (millis() - channel16_timer > 60)) {
    pixelchan.setPixelColor(CHANNEL_16_LED, 0, 0, 0);
    channel16_timer = 0;
  }

  if ((display_timer > 0) && (millis() - display_timer > 2000)) {
    tft.fillScreen(TFT_BLACK);
    refresh_screen = true;
    display_timer = 0;
  }
}

void channelLED(byte channel) {
  switch (channel) {

    case 1:
      pixelchan.setPixelColor(CHANNEL_1_LED, 255, 0, 0);
      channel1_timer = millis();
      break;

    case 2:
      pixelchan.setPixelColor(CHANNEL_2_LED, 255, 0, 0);
      channel2_timer = millis();
      break;

    case 3:
      pixelchan.setPixelColor(CHANNEL_3_LED, 255, 0, 0);
      channel3_timer = millis();
      break;

    case 4:
      pixelchan.setPixelColor(CHANNEL_4_LED, 255, 0, 0);
      channel4_timer = millis();
      break;

    case 5:
      pixelchan.setPixelColor(CHANNEL_5_LED, 255, 0, 0);
      channel5_timer = millis();
      break;

    case 6:
      pixelchan.setPixelColor(CHANNEL_6_LED, 255, 0, 0);
      channel6_timer = millis();
      break;

    case 7:
      pixelchan.setPixelColor(CHANNEL_7_LED, 255, 0, 0);
      channel7_timer = millis();
      break;

    case 8:
      pixelchan.setPixelColor(CHANNEL_8_LED, 255, 0, 0);
      channel8_timer = millis();
      break;

    case 9:
      pixelchan.setPixelColor(CHANNEL_9_LED, 255, 0, 0);
      channel9_timer = millis();
      break;

    case 10:
      pixelchan.setPixelColor(CHANNEL_10_LED, 255, 0, 0);
      channel10_timer = millis();
      break;

    case 11:
      pixelchan.setPixelColor(CHANNEL_11_LED, 255, 0, 0);
      channel11_timer = millis();
      break;

    case 12:
      pixelchan.setPixelColor(CHANNEL_12_LED, 255, 0, 0);
      channel12_timer = millis();
      break;

    case 13:
      pixelchan.setPixelColor(CHANNEL_13_LED, 255, 0, 0);
      channel13_timer = millis();
      break;

    case 14:
      pixelchan.setPixelColor(CHANNEL_14_LED, 255, 0, 0);
      channel14_timer = millis();
      break;

    case 15:
      pixelchan.setPixelColor(CHANNEL_15_LED, 255, 0, 0);
      channel15_timer = millis();
      break;

    case 16:
      pixelchan.setPixelColor(CHANNEL_16_LED, 255, 0, 0);
      channel16_timer = millis();
      break;
  }
}
