#include <USB.h>
#include <USBMSC.h>
#include <SD.h>

// USB Mass Storage Class (MSC) object
USBMSC msc;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize){
  uint32_t secSize = SD.sectorSize();
  if (!secSize) return false; // disk error
  log_v("Write lba: %ld\toffset: %ld\tbufsize: %ld", lba, offset, bufsize);
  for (int x=0; x< bufsize/secSize; x++) {
    uint8_t blkbuffer[secSize];
    memcpy(blkbuffer, (uint8_t*)buffer + x*secSize, secSize);
    if (!SD.writeRAW(blkbuffer, lba + x)) return false;
  }
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize){
  uint32_t secSize = SD.sectorSize();
  if (!secSize) return false; // disk error
  log_v("Read lba: %ld\toffset: %ld\tbufsize: %ld\tsector: %lu", lba, offset, bufsize, secSize);
  for (int x=0; x < bufsize/secSize; x++) {
    //uint8_t blkbuffer[secSize];
    if (!SD.readRAW((uint8_t*)buffer + (x * secSize), lba + x)) return false; // outside of volume boundary
  }
  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  log_i("Start/Stop power: %u\tstart: %d\teject: %d", power_condition, start, load_eject);
  return true;
}

void setup(){
  Serial.begin(115200);
  Serial.println("Starting Serial");

  Serial.println("Mounting SDcard");
  //SPI.begin(12, 13, 11, 10); //default pins for S3
  //if(!SD.begin(10)){
  if(!SD.begin()){
    Serial.println("Mount Failed");
  }

  Serial.println("Initializing MSC");
  // Initialize USB metadata and callbacks for MSC (Mass Storage Class)
  msc.vendorID("ESP32");
  msc.productID("USB_MSC");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);
  msc.begin(SD.numSectors(), SD.sectorSize());

  Serial.println("Initializing USB");

  USB.begin();

  Serial.printf("SD Size: %lluMB", SD.totalBytes()/1024/1024);
}

void loop(){
  delay(-1);
}
