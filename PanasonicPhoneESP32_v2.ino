// 2024-07-30
// Tested for WEMOS ESP8266
// not tested for ESP32
// *********************************************************************************
// BRANCH - some fix

#include <esp-fs-webserver.h>   // https://github.com/cotestatnt/esp-fs-webserver
#include <FS.h>
#include <LittleFS.h>
#define FILESYSTEM LittleFS
#include "custom_html.h"

FSWebServer myWebServer(FILESYSTEM, 80);


#include <WiFiHttpClient.h>
char serverAddress_[16];
// = "100.100.100.100";  // server address
int port = 8000;
int mp3Volume = 20;
int mp3InitFile = 15;
int mp3Good = 17;
int mp3Bad = 14;
int mp3Default = 21;
String requiredNumber = "0904932233";
String serverAddress = "192.168.1.1";
int serverPort = 8000;

//serverAddress.toCharArray(serverAddress_, serverAddress.length()+1);
WiFiClient      client;
WiFiHttpClient  httpClient(client, serverAddress_, port);




#include <DFPlayerMini_Fast.h>
DFPlayerMini_Fast myMP3;

//// Need this for the lower level access to set them up.
//#include <HardwareSerial.h>
////Define two Serial devices mapped to the two internal UARTs
//HardwareSerial MySerial0(0);
////HardwareSerial MySerial1(1);

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define BTN_SAVE  5

// Test "options" values
uint8_t ledPin = LED_BUILTIN;
bool boolVar = true;
uint32_t longVar = 1234567890;
float floatVar = 15.5F;
String stringVar = "Test option String";

// ************** KEYPAD *******************************
#include <Keypad.h>
const byte ROWS = 3; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'3', '2', '1', 'x'},
  {'7', '6', '5', '4'},
  {'*', '0', '9', '8'}


};

byte rowPins[ROWS] = {D0, D2, D1};
byte colPins[COLS] = { D8, D7, D6, D5};

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
// ************** KEYPAD *******************************

// In order to show a dropdown list box in /setup page
// we need a list ef values and a variable to store the selected option
#define LIST_SIZE  7
const char* dropdownList[LIST_SIZE] = {
  "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
};
String dropdownSelected;

// Var labels (in /setup webpage)
#define LED_LABEL "The LED pin number"
#define BOOL_LABEL "A bool variable"
#define LONG_LABEL "A long variable"
#define FLOAT_LABEL "A float variable"
#define STRING_LABEL "A String variable"
#define DROPDOWN_LABEL "A dropdown listbox"


#define SERVER_LABEL "Server address"
#define PORT_LABEL "Server port"
#define VOLUME_LABEL "MP3 Volume"
#define INITFILE_LABEL "Initial mp3#"
#define MP3_DEFAULT "Default number mp3#"
#define MP3_GOOD "Good number mp3#" // folder ADVERT
#define MP3_BAD "Bad number mp3#" // folder ADVERT
#define REQUIRED_NUMBER "Phone number"

#define DEVICE_ID "Device ID"
#define VAR_ID "Var ID"
#define API_PATH "API Path"

int deviceID=0;
String varID, apiPath;

////////////////////////////////  Filesystem  /////////////////////////////////////////
void startFilesystem() {
  // FILESYSTEM INIT
  if ( !FILESYSTEM.begin()) {
    Serial.println("ERROR on mounting filesystem. It will be formmatted!");
    FILESYSTEM.format();
    ESP.restart();
  }
  myWebServer.printFileList(LittleFS, Serial, "/", 2);
}

/*
  Getting FS info (total and free bytes) is strictly related to
  filesystem library used (LittleFS, FFat, SPIFFS etc etc) and ESP framework
  ESP8266 FS implementation has methods for total and used bytes (only label is missing)
*/
#ifdef ESP32
void getFsInfo(fsInfo_t* fsInfo) {
  fsInfo->fsName = "LittleFS";
  fsInfo->totalBytes = LittleFS.totalBytes();
  fsInfo->usedBytes = LittleFS.usedBytes();
}
#else
void getFsInfo(fsInfo_t* fsInfo) {
  fsInfo->fsName = "LittleFS";
}
#endif


////////////////////  Load application options from filesystem  ////////////////////
bool loadOptions() {
  if (FILESYSTEM.exists(myWebServer.getConfigFilepath())) {
    // Config file will be opened on the first time we call this method
    myWebServer.getOptionValue(LED_LABEL, ledPin);
    myWebServer.getOptionValue(BOOL_LABEL, boolVar);
    myWebServer.getOptionValue(LONG_LABEL, longVar);
    myWebServer.getOptionValue(FLOAT_LABEL, floatVar);
    myWebServer.getOptionValue(STRING_LABEL, stringVar);
    myWebServer.getOptionValue(DROPDOWN_LABEL, dropdownSelected);

    myWebServer.getOptionValue(VOLUME_LABEL, mp3Volume);
    myWebServer.getOptionValue(INITFILE_LABEL, mp3InitFile);
    myWebServer.getOptionValue(MP3_DEFAULT, mp3Default);
    myWebServer.getOptionValue(MP3_GOOD, mp3Good);
    myWebServer.getOptionValue(MP3_BAD, mp3Bad);
    myWebServer.getOptionValue(REQUIRED_NUMBER, requiredNumber);
    myWebServer.getOptionValue(SERVER_LABEL, serverAddress);
    myWebServer.getOptionValue(PORT_LABEL, serverPort);

    myWebServer.getOptionValue(DEVICE_ID, deviceID);
    myWebServer.getOptionValue(VAR_ID, varID);
    myWebServer.getOptionValue(API_PATH, apiPath);

    // Close configuration file and release memory
    myWebServer.closeConfiguration(false);

    Serial.println("\nThis are the current values stored: \n");
    Serial.printf("LED pin value: %d\n", ledPin);
    Serial.printf("Bool value: %s\n", boolVar ? "true" : "false");
    Serial.printf("Long value: %d\n", longVar);
    Serial.printf("Float value: %d.%d\n", (int)floatVar, (int)(floatVar * 1000) % 1000);
    Serial.printf("String value: %s\n", stringVar.c_str());
    Serial.printf("Dropdown selected value: %s\n\n", dropdownSelected.c_str());
    Serial.printf("MP3 Volume value: %d\n", mp3Volume);
    Serial.printf("MP3 Init file value: %d\n", mp3InitFile);
    Serial.printf("Server address value: %s\n", serverAddress.c_str());
    Serial.printf("Server port value: %d\n", serverPort);
    return true;
  }
  else
    Serial.println(F("Config file not exist"));
  return false;
}

void saveOptions() {
  // Config file will be opened on the first time we call this method
  myWebServer.saveOptionValue(LED_LABEL, ledPin);
  myWebServer.saveOptionValue(BOOL_LABEL, boolVar);
  myWebServer.saveOptionValue(LONG_LABEL, longVar);
  myWebServer.saveOptionValue(FLOAT_LABEL, floatVar);
  myWebServer.saveOptionValue(STRING_LABEL, stringVar);
  myWebServer.saveOptionValue(DROPDOWN_LABEL, dropdownSelected);
  myWebServer.saveOptionValue(VOLUME_LABEL, mp3Volume);
  myWebServer.saveOptionValue(INITFILE_LABEL, mp3InitFile);
  myWebServer.saveOptionValue(SERVER_LABEL, serverAddress);
  myWebServer.saveOptionValue(PORT_LABEL, serverPort);
  myWebServer.saveOptionValue(MP3_DEFAULT, mp3Default);
  myWebServer.saveOptionValue(MP3_GOOD, mp3Good);
  myWebServer.saveOptionValue(MP3_BAD, mp3Bad);
  myWebServer.saveOptionValue(REQUIRED_NUMBER, requiredNumber);

  myWebServer.saveOptionValue(DEVICE_ID, deviceID);
  myWebServer.saveOptionValue(VAR_ID, varID);
  myWebServer.saveOptionValue(API_PATH, apiPath);

  // Close config file and release memory
  myWebServer.closeConfiguration(true);
  Serial.println(F("Application options saved."));
}

////////////////////////////  HTTP Request Handlers  ////////////////////////////////////
void handleLoadOptions() {
  myWebServer.send(200, "text/plain", "Options loaded");
  loadOptions();
  Serial.println("Application option loaded after web request");
}

void sendGET(String message ) {
  //  Serial.println("making GET request");
  //  Serial.println(serverAddress.c_str());
  //  httpClient.get("/send/a_" + message);
  //  // read the  code and body of the response
  //  int statusCode = httpClient.responseStatusCode();
  //  String response = httpClient.responseBody();
  //
  //  Serial.print("Status code: ");
  //  Serial.println(statusCode);
  //  Serial.print("Response: ");
  //  Serial.println(response);
}

int n = 0;
void setup() {
  Serial.begin(9600);
  pinMode(BTN_SAVE, INPUT_PULLUP);

  // FILESYSTEM INIT
  startFilesystem();

  // Try to connect to stored SSID, start AP if fails after timeout
  myWebServer.setAP("ESP_AP", "123456789");
  IPAddress myIP = myWebServer.startWiFi(15000);

  // Load configuration (if not present, default will be created when webserver will start)
  if (loadOptions())
    Serial.println(F("Application option loaded"));
  else
    Serial.println(F("Application options NOT loaded!"));

  // Add custom page handlers to webserver
  myWebServer.on("/reload", HTTP_GET, handleLoadOptions);

  // Configure /setup page and start Web Server
  myWebServer.addOptionBox("Device");
  myWebServer.addOption(DEVICE_ID, deviceID);
  myWebServer.addOption(VAR_ID, varID);
  myWebServer.addOption(API_PATH, apiPath);
  
  myWebServer.addOptionBox("Phone Options");
  myWebServer.addOption(BOOL_LABEL, boolVar);
  myWebServer.addOption(LED_LABEL, ledPin);
  //  myWebServer.addOption(LONG_LABEL, longVar);
  //  myWebServer.addOption(FLOAT_LABEL, floatVar, 1.0, 100.0, 0.01);
  //  myWebServer.addOption(STRING_LABEL, stringVar);
  myWebServer.addOption(VOLUME_LABEL, mp3Volume);
  myWebServer.addOption(INITFILE_LABEL, mp3InitFile);
  myWebServer.addOption(SERVER_LABEL, serverAddress);
  myWebServer.addOption(PORT_LABEL, serverPort);
  myWebServer.addOption(MP3_DEFAULT, mp3Default);
  myWebServer.addOption(MP3_GOOD, mp3Good);
  myWebServer.addOption(MP3_BAD, mp3Bad);
  myWebServer.addOption(REQUIRED_NUMBER, requiredNumber);
  //myWebServer.addDropdownList(DROPDOWN_LABEL, dropdownList, LIST_SIZE);
  myWebServer.addHTML(save_btn_htm, "buttons");
  myWebServer.addJavascript(button_script, "script");

  // set /setup and /edit page authentication
  // myWebServer.setAuthentication("admin", "admin");

  // Enable ACE FS file web editor and add FS info callback function
  myWebServer.enableFsCodeEditor(getFsInfo);

  myWebServer.begin();
  Serial.print(F("ESP Web Server started on IP Address: "));
  Serial.println(myIP);
  Serial.println(F("Open /setup page to configure optional parameters"));
  Serial.println(F("Open /edit page to view and edit files"));
  Serial.println(F("Open /update page to upload firmware and filesystem updates"));

  Serial.println("myMP3.begin(MySerial0, true)");
  myMP3.begin(Serial, true);
  myMP3.volume(mp3Volume);
  myMP3.playFromMP3Folder(mp3InitFile);

  serverAddress.toCharArray(serverAddress_, serverAddress.length() + 1);
  //WiFiClient      client;
  WiFiHttpClient  httpClient(client, serverAddress_, port);

}



int active = 0;
String phoneNumber = "";
//String requiredNumber = "0904932233";
String sendNumber = "";

void loop() {
  myWebServer.run();

  if (! digitalRead(BTN_SAVE)) {
    saveOptions();
    delay(1000);
  }


  if (digitalRead(D3) == 0 && active == 0)
  {
    active = 1;
    n = 0;
    myMP3.playFromMP3Folder(mp3Default);
    Serial.println("active = 1 ");
    Serial.print("START playing #");
    Serial.println(mp3Default);
  }
  if (digitalRead(D3) == 1 && active == 1)
  {
    active = 0;
    n = 0;
    phoneNumber = "";
    myMP3.stop();
    Serial.print("active = 0 ");
    Serial.println("STOP playing");
  }

  char customKey = customKeypad.getKey();
  if (customKey && active == 1) {
    n++;
    Serial.println(customKey);
    phoneNumber += customKey;
    Serial.print("Phone Number: ");
    Serial.println(phoneNumber);
    if (customKey == '0') {
      myMP3.playAdvertisement(10);
    } else if (customKey == '*') {
      myMP3.playAdvertisement(11);
    }
    else {
      int num = customKey - '0';
      myMP3.playAdvertisement(num);

    }
    if (n == 10) {
      sendNumber = phoneNumber;
      if (phoneNumber == requiredNumber) {
        delay(200);
        Serial.print("playAdvertisement mp3Good ");
        Serial.println(mp3Good);
        myMP3.playAdvertisement(mp3Good);
      }
      else
      {
        n = 0;
        phoneNumber = "";
        delay(200);
        Serial.print("playAdvertisement mp3Bad ");
        Serial.println(mp3Bad);
        myMP3.playAdvertisement(mp3Bad);
      }
      //sendGET(stringVar + sendNumber);
      Serial.println("making GET request");
      Serial.println(serverAddress.c_str());
      // '/send/{"sender":101,"var":"pwm16","val":0}'
      httpClient.get(apiPath+"{\"sender\":"+String(deviceID)+",\"var\":\""+varID+"\",\"val\":\""+sendNumber+"\"}");
      //httpClient.get("/send/a_" + stringVar + sendNumber);
      // read the  code and body of the response
      int statusCode = httpClient.responseStatusCode();
      String response = httpClient.responseBody();

      Serial.print("Status code: ");
      Serial.println(statusCode);
      Serial.print("Response: ");
      Serial.println(response);
    }


  }

}
