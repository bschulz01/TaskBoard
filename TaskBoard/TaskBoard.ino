#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//////// NETWORK INTERFACE VARIABLES //////// 

const char* ssid = "SSID";//type your ssid
const char* password = "PSSWD";//type your password

String host = "https://c4jxx1c9ia.execute-api.us-west-2.amazonaws.com/default/TaskBoard";
char* fingerprint = "E1:0B:3B:EC:6D:81:A4:4E:D0:CB:05:03:2F:F0:02:9C:8E:D1:73:08";
const char* token = "YOUR_TOKEN_HERE";
const int NUM_ROOMS = 5;
char* room_array[] = {"Kitchen", "Bedroom", "Office", "Outside", "Toiletries"};
int main_room = 2;


//////// TASK VARIABLES ////////

// Globabl variables to store state of tasks
const int MAX_TASKS = 10; // maximum number of tasks accepted from Todoist
String tasks[MAX_TASKS] = {"", "", "", "", "", "", "", "", "", ""};
int tasksTodo = 0; // the number of tasks to do
int taskIndex = 0; // the index of the currently selected task
int indexOffset = 0; // offset for scrolling down the display
int completedTask = -1; // the task that is completed; -1 for no task finished


//////// LED VARIABLES //////// 

// colors for the map LEDs
const int NUM_LEDS = 5;
int colors[NUM_LEDS][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
short ledStates = 0; // stores state of LED registers

// LED pins
const int latchPin1 = 18;
const int latchPin2 = 5;
const int clockPin = 17;
const int dataPin = 19;
const int ledEnablePin = 16;


//////// BUTTON VARIABLES //////// 

// button control
bool pressed1 = false;
bool pressed2 = false;

const int button1 = 12;
const int button2 = 13;

int counter = 0;

//////// IR VARIABLES ////////
const int IRPin = 0;
int idleCounts = 0; // count the number of idle cycles
const int idle_to_sleep = 15; // cycles before going to sleep
bool idle = false;
int prevReading = 0; // previous IR reading for calculating change
const int readingInterval = 5; // time delay between readings
int jumpThresh = 25
; // threshold in a jump in IR readings to reactivate

//////// EXTERNAL DEVICE DECLARATION //////// 

HTTPClient http;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);


//////// FUNCTION DECLARATIONS //////// 

// Get updated data
void accessdb(int finishedTask);
// Update the display
void updateDisplay();
//set the map LEDs
void setMap();
// update the LED masks
void updateLED(int roomID, int *leds);
// set the LEDs
void updateMap(const short mapData);

void setup() {
  pinMode(button1, INPUT); 
  pinMode(button2, INPUT); 
  
  // intialize pins
  pinMode(latchPin1, OUTPUT);
  pinMode(latchPin2, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(ledEnablePin, OUTPUT);

  digitalWrite(ledEnablePin, LOW);
  digitalWrite(latchPin1, HIGH);
  digitalWrite(latchPin2, HIGH);
  
  Serial.begin(115200);
  delay(10);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the display
  display.display();
  display.clearDisplay();
  display.setCursor(0, 32);
  display.print("Connecting to ");
  display.println(ssid);
  display.display();
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  // Initialize the screen
  accessdb();
  updateDisplay();

  // get a preliminary IR reading
  prevReading = analogRead(IRPin);
}

void loop() {
  // poll the buttons
  // update the tasks every 200 cycles
  // don't update or display if the user has been idle for long enough 

  // read button 1 - toggle selected task
  if (digitalRead(button1)) {
    pressed1 = false;
  } else {
    if (!pressed1) {
      if (idle) {
        idle = false;
      } else {
        taskIndex++;
        if (taskIndex >= tasksTodo) {
          taskIndex = 0;
          indexOffset = 0;
        }
        if (taskIndex >= 6) {
          indexOffset = taskIndex - 5;
        }
      }
      updateDisplay();
      counter = 0;
    }
    pressed1 = true;
    idleCounts = 0;
  }

  // read button 2 - complete task
  if (digitalRead(button2)) {
    pressed2 = false;
  } else {
    if (!pressed2 && !idle) {
      completedTask = taskIndex;
    }
    counter = 0;
    pressed2 = true;
    idleCounts = 0;
  }
  

  // read the IR sensor 
  if (counter % readingInterval == 0) {
    int reading = analogRead(IRPin);
    if (reading - prevReading > jumpThresh) {
      idleCounts = 0;
      if (idle) {
        accessdb();
        updateDisplay();
      }
      idle = false;
    }
    prevReading = reading;
  }

  counter++;
  // update from database
  if (counter > 200) {
    if (idleCounts < idle_to_sleep) {
      idle = false;
      accessdb();
      updateDisplay();
    } else {
      idle = true;
      digitalWrite(ledEnablePin, HIGH);
      display.clearDisplay();
      display.println("");
      display.display();
    }
    idleCounts++;
    counter = 0;
  }
  delay(10);
}

void accessdb() {
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
   
    // Encode the request
    String request = "?token=";
    request += token;
    request += "&room_id=";
    request += String(main_room);
    request += "&completed=";
    request += String(completedTask);
    request += "&rooms=%5B'";
    for (int i = 0; i < NUM_ROOMS-1; i++) {
      request += room_array[i];
      request += "','";
    }
    request += room_array[NUM_ROOMS-1];
    request += "'%5D";

    // Reset the task completion
    completedTask = -1;

    // Connect to the server
    http.begin(host+request, fingerprint); //Specify the URL

    // Read the response
    int httpCode = http.GET();
    if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();

        DynamicJsonDocument doc(1024);
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();;
      
        // Test if parsing succeeds.
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

//        Print the raw json output
//        String printStr;
//        serializeJson(doc, printStr);
//        Serial.println(printStr);
        // extract the number of tasks
        tasksTodo = constrain(obj["tasks"], 0, MAX_TASKS);
        // extract the names of the tasks
        for (int i = 0; i < tasksTodo; i++) {
          tasks[i] = obj["display"][i].as<String>();
        }
        // extract the colors for the LEDs
        for (int i = 0; i < NUM_LEDS; i++) {
          for (int j = 0 ; j < 3; j++) {
            colors[i][j] = obj["colors"][String(i)][j].as<int>();
          }
        }
    }
    else {
      Serial.println("Error on HTTP request: " + String(httpCode));
    }
 
    http.end(); //Free the resources
  }
}

void updateDisplay() {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.print(tasksTodo);
  display.println(" tasks remaining\n");
  // Print the names of the tasks
  for(int i = indexOffset; i < tasksTodo; i++) {
    display.print("  ");
    display.println(tasks[i]);
    if (i >= indexOffset + 6) {
      break;
    }
  }
  // Add a dot to show the currently selected tasks
  int dotPos = 19 + 8*(taskIndex-indexOffset); // each line takes 8 pixels + offset of 3 pixels
  display.fillCircle(3, dotPos, 2, SSD1306_WHITE);
  display.display();
  setMap();
}

// Set the colors on the map
void setMap() {
  ledStates = 0;
  for(int i=0; i < NUM_LEDS; i++)  {
    updateLED(i, colors[i]);
  }
  updateMap(ledStates);
}

// update bits in ledStates corresponding to a particular room
void updateLED(int roomID, int *leds) {
  int mask = 0 | *(leds+2);
  mask = (mask << 1) | *(leds+1);
  mask = (mask << 1) | *leds;
  mask = mask << (roomID * 3);

  ledStates = ledStates | mask;
}

// writes data passed in through mapData to led registers
void updateMap(const short mapData) {
  
  digitalWrite(ledEnablePin, LOW);
  digitalWrite(latchPin1, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, (mapData >> 8));
  digitalWrite(latchPin1, HIGH);

  digitalWrite(latchPin2, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, mapData);
  digitalWrite(latchPin2, HIGH);
}