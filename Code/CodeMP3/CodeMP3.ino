#include <SPI.h>
#include <SD.h>
#include <I2S.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

#include <vector>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define BCK_PIN 20
#define LRCK_PIN 21 
#define DIN_PIN 22

#define SAMPLE_RATE 44100
#define AMPLITUDE   32767
float volume = 0.50f;
float volume_inc = 0.05f;

bool is_playing = false;

// 1 for volume up, 2 for volume down by 10%. 3 for skipping through playlist left, 4 for right. 5 for play or pause.
const int NUM_BUTTONS = 5;
const int buttonPins[NUM_BUTTONS] = {2, 3, 4, 5, 6}; 

int buttonStates[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH, HIGH}; 
int lastButtonStates[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH, HIGH}; 
unsigned long lastDebounceTimes[NUM_BUTTONS] = {0, 0, 0, 0, 0};
unsigned long debounceDelay = 50; // Debounce time in milliseconds

const int chipSelect = 17;
#define SPI0_MISO 16
#define SPI0_MOSI 19
#define SPI0_SCK  18

AudioOutputI2S *out;
AudioFileSourceSD *file = NULL;
AudioGeneratorMP3 *mp3 = NULL;
AudioGeneratorWAV *wav = NULL;

std::vector<String> fileList;
int file_index = 0;

void readButtons();
void play_file(String fileName);
void stop_audio();
void updateOled();
void advance_track();
float calculate_adc();

// adc variables
#define GPIO_PIN_ADC 26

float r1 = 100000.0f;
float r2 =330000.0f;

// Vout = Vin × R2 / (R1 + R2)
// Vout = vin_adc / (R2 / (R1 + R2)) = vin_adc * 0.76

float multiply_adc_constant = r2 / (r1 + r2);

int last_adc_time = 0;

float calculate_adc() {
  int raw = analogRead(GPIO_PIN_ADC);
  float v_out = (raw * 3.3f) / 4096.0f; 
  
  float v_in = v_out * ((r1 + r2) / r2); 
  return v_in;
}

float voltage = 0.0f;

void setup() {
  for (int i = 0; i < NUM_BUTTONS; i++) { 
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  Serial.begin(115200);
 

  SPI.setRX(SPI0_MISO);
  SPI.setTX(SPI0_MOSI);
  SPI.setSCK(SPI0_SCK);

  Serial.print("SD Starting... ");
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Failed");

  } else {
    Serial.println("SD Done");
  }


  Wire.setSDA(8);
  Wire.setSCL(9);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.display();

  out = new AudioOutputI2S();
  out->setPinout(BCK_PIN, LRCK_PIN, DIN_PIN);
  out->SetGain(volume);

  File root = SD.open("/");
  if (root) {
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;

      if (entry.isDirectory()) {
        entry.close();
        continue;
      }

      String fileName = entry.name();
      String lowerName = fileName;
      lowerName.toLowerCase();

      if (lowerName.endsWith(".mp3") || lowerName.endsWith(".wav")) {
        fileList.push_back("/" + fileName);
      }
      entry.close();
    }
    root.close();
  }

  if (fileList.size() > 0) {
    play_file(fileList[file_index]);
  } else {
    Serial.println("No audio files found.");
    updateOled();
  }
}

void loop() {
  readButtons();
  

  

  if (millis() - last_adc_time > 1000) {
    last_adc_time = millis();
    voltage = calculate_adc();
    updateOled();

  }
  if (is_playing) {
    if (mp3 && mp3->isRunning()) {
      if (!mp3->loop()) { 
 
        advance_track(); 
      }
    } else if (wav && wav->isRunning()) {
      if (!wav->loop()) { 

        advance_track();
      }
    }
  }
}

void advance_track() {
  if (fileList.size() > 0) {
    file_index = (file_index + 1) % fileList.size();

    play_file(fileList[file_index]);

  } else {
    stop_audio();
  }
}
void updateOled() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Top Row: song counter

  if (fileList.size() > 0) {
    display.printf("Track: %d / %d", file_index + 1, fileList.size());
  } else { 
    display.println("No files found");
  }
  
  // Dividing line
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  // Midle row: Song title
  display.setCursor(0, 22);
  if (fileList.size() > 0) {
    String name = fileList[file_index].substring(1);
  
  
    if (name.length() > 20) {
        name = name.substring(0, 17) + "...";
      }
      
    display.print(name);
  }

  // Dividing line above the status bar
  display.drawFastHLine(0, 48, 128, SSD1306_WHITE);
  
  // Bottom row: status bar

  display.setCursor(0, 54);
  display.printf("Vol:%d%% %s", (int)(volume * 100), is_playing ? "PLAY" : "PAUS");

  // Voltage readings right

  display.setCursor(85, 54);
  display.printf("%.2fV", voltage);
  
  display.display();


}

void stop_audio() {
  if (mp3) { mp3->stop(); delete mp3; mp3 = NULL; }
  if (wav) { wav->stop(); delete wav; wav = NULL; }
  if (file) { delete file; file = NULL; }
}

void play_file(String fileName) {
  stop_audio();
  
  file = new AudioFileSourceSD(fileName.c_str());
  String lowerName = fileName;
  lowerName.toLowerCase();

  if (lowerName.endsWith(".wav")) {
    Serial.print("Playing wav: ");
    Serial.println(fileName);
    wav = new AudioGeneratorWAV();
    if (wav->begin(file, out)) {
      is_playing = true;
    }
  } else if (lowerName.endsWith(".mp3")) {
    Serial.print("Playing MP3: ");
    Serial.println(fileName);
    mp3 = new AudioGeneratorMP3();
    if (mp3->begin(file, out)) {
      is_playing = true;
    }
  }
  
  updateOled();
}

void readButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    
    int reading = digitalRead(buttonPins[i]);

    if (reading != lastButtonStates[i]) {
      lastDebounceTimes[i] = millis();

    }

    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;

        // Buttons was pressed, i am using input pullup so lowe means pressed
        if (buttonStates[i] == LOW) {
          switch (i) {
            case 0: // volume up
              volume += volume_inc;
              if (volume > 1.0f) volume = 1.0f;
              out->SetGain(volume);
              updateOled();
              break;

            case 1: // volume down
              volume -= volume_inc;
              if (volume < 0.0f) volume = 0.0f;
              out->SetGain(volume); 
              updateOled();
              break;

            case 2: // play prev
              if (fileList.size() > 0) {
                file_index--;
                if (file_index < 0) {file_index = fileList.size() - 1;}
                play_file(fileList[file_index]);
              }
              break;
            
            case 3: // play next
              if (fileList.size() > 0) {
                file_index = (file_index + 1) % fileList.size();
                play_file(fileList[file_index]);
              }
              break;
            
            case 4: // pause/play
              is_playing = !is_playing;
              updateOled();
              break;

              S
            } 
        }
      } 
    } 
    lastButtonStates[i] = reading;
  } 
} 
