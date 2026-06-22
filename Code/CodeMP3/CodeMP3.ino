#include <SPI.h>
#include <SD.h>

const int chipSelect = 17;


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Serial.print("Starting")

  if (!SD.behin(chipSelect)) {
    Serial.println("Failed")
    return;
  }

  Serial.println("Done")


  File root = SD.open("/");
  
  if (!root) {
    Serial.println("Failed to open root dir")
    return;

  }

  while (true) {
    File entry = root.openNextFile();

    if (!entry) {
      // End of files
      break;
    }

    // Checking if the next file is folder or not 

    if (entry.isDirectory()) {
      entry.close();
      continue;
    }

    // Find file and list its name
    String fileName = entry.name();
    Serial.print("File found: ");
    Serial.println(fileName);

    // Convert filename to lowercase 

    fileName.toLowerCase();

    if (fileName.endsWith(".wav")) {
      PassToAudio(SD.open(entry.name()));
    }

    if (fileName.endsWith(".mp3")) {
      PassToAudio(SD.open(entry.name()));
    }


  }
}

void loop() {
  // put your main code here, to run repeatedly:

}


void PassToAudio(File AudioFile) {
  continue;
}
