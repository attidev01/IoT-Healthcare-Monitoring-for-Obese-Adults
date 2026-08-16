
// IoT 
// Lahcen Atti, Mouhamed Dib, Nacer Boubkraoui, Saber
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi Credentials
const char* ssid = "inwi Home 4G 84CDB7";
const char* password = "60968588";

// MQTT Broker (ThingsBoard Server)
const char* mqtt_server = "demo.thingsboard.io";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32_Client";  // Unique client ID
const char* mqtt_username = "jwlvKI335Iog6NiwPGdr";  // ThingsBoard device token

WiFiClient espClient;
PubSubClient client(espClient);


// Sensors Part .....................................................
#define REPORTING_PERIOD_MS     1000

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Create a PulseOximeter object
PulseOximeter pox;

// Time at which the last beat occurred
uint32_t tsLastReport = 0;
unsigned long lastUpdateTime = 0;
// End Config Sensors ...............................................

// LCD setup
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20x4 LCD

// Keypad setup
#define KEY_NUM 4
#define PIN_KEY_1 27
#define PIN_KEY_2 26
#define PIN_KEY_3 12
#define PIN_KEY_4 14

ezButton keypad_1x4[] = {
  ezButton(PIN_KEY_1),
  ezButton(PIN_KEY_2),
  ezButton(PIN_KEY_3),
  ezButton(PIN_KEY_4)
};

// Add this variable declaration
bool keyProcessed = false; // Ensure this is declared globally

// Menu states
#define MENU_PATIENT 0
#define MENU_PATIENT_CODE 1
#define MENU_MAIN 2
#define MENU_SPO2 3
#define MENU_BPM 4
#define MENU_TEMP 5
#define MENU_DATA_TRANSFER 6

int currentMenu = MENU_PATIENT; // Change initial menu to PATIENT
String patientCode = "";
bool patientLoggedIn = false;
String currentPatient = "";

// Sensor data variables
float currentSpO2 = 0;
int currentBPM = 0;
float currentTemp = 0;
float oldSpO2 = 0;
int oldBPM = 0;
float oldTemp = 0;

// Screen state variables
bool isMeasuring = false;
unsigned long measurementStartTime = 0;
const unsigned long measurementDuration = 5000; // 5 seconds for measurement

// Non-blocking animation variables
unsigned long animationLastUpdate = 0;
int animationStep = 0;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("Beat!");
}

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(mqtt_client_id, mqtt_username, NULL)) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(9600);
  
  // Initialize the LCD
  //lcd.init();
  lcd.begin();
  lcd.backlight();
  //lcd.setCursor(0, 0); // Set cursor to the first column, first row
  //lcd.print("IAE FSAA!");
  
  connectToWiFi();
  client.setServer(mqtt_server, mqtt_port);
  connectToMQTT();

  // Initialize keypad buttons
  for (byte i = 0; i < KEY_NUM; i++) {
    keypad_1x4[i].setDebounceTime(50); // Set debounce time to 50ms
  }

  // Show welcome screen
  showWelcomeScreen();
  delay(2000); // This delay is ok since it's only in setup
  
  // Initialize sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Sensor Failed!");
    delay(2000); // This delay is ok since it's only in setup
  } else {
    Serial.println("SUCCESS");
  }
  
  // Configure sensor
  pox.setIRLedCurrent(MAX30100_LED_CURR_46_8MA);
  
  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);
  
  // Start up the DS18B20 temperature sensor library
  sensors.begin();
  
  showPatientMenu(); // Change to show patient menu first
}

void loop() {
  // Always update the pulse oximeter in each loop iteration
  pox.update();
  /*
  // Update temperature occasionally
  if (millis() - lastUpdateTime > 5000) { // Check every 5 seconds
    sensors.requestTemperatures(); // Request temperature readings
    currentTemp = sensors.getTempCByIndex(0);
    lastUpdateTime = millis();
  }
  */
  // Update our readings every reporting period
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    currentSpO2 = pox.getSpO2();
    currentBPM = pox.getHeartRate();
    
    // Print debug info to serial
    Serial.print("Heart rate: ");
    Serial.print(currentBPM);
    Serial.print("bpm / SpO2: ");
    Serial.print(currentSpO2);
    Serial.print("% / Temperature: ");
    Serial.print(currentTemp);
    Serial.println("C");
    
    tsLastReport = millis();
  }
  
  // Handle keypad input
  int key = getKeyPressed();
  if (key && !keyProcessed) {
    keyProcessed = true;
    handleKeyPress(key);
  }
  if (key == 0 && keyProcessed) {
    keyProcessed = false;
  }
  
  // Handle measurement screens
  if (isMeasuring) {
    updateMeasurementScreen();
  }
}

// Function to update measurement screens without blocking
void updateMeasurementScreen() {
  // Update animation non-blockingly
  if (millis() - animationLastUpdate > 300) {
    animationStep = (animationStep + 10) % 100;
    
    switch (currentMenu) {
      case MENU_SPO2:
        lcd.setCursor(12, 1);
        lcd.print(animationStep);
        lcd.print("%   ");
        break;
      case MENU_BPM:
        lcd.setCursor(12, 1);
        lcd.print(animationStep);
        lcd.print("%   ");
        break;
      case MENU_TEMP:
        lcd.setCursor(12, 1);
        lcd.print(animationStep);
        lcd.print("%   ");
        break;
    }
    
    animationLastUpdate = millis();
  }
  
  // Check if measurement time is complete
  if (millis() - measurementStartTime > measurementDuration) {
    isMeasuring = false;
    
    // Display the results based on current menu
    switch (currentMenu) {
      case MENU_SPO2:
        showSpO2Results();
        break;
      case MENU_BPM:
        showBPMResults();
        break;
      case MENU_TEMP:
        showTempResults();
        break;
    }
  }
}

// Function to get the pressed key
int getKeyPressed() {
  for (byte i = 0; i < KEY_NUM; i++) {
    keypad_1x4[i].loop();
  }
  
  for (byte i = 0; i < KEY_NUM; i++) {
    if (keypad_1x4[i].isPressed())
      return (i + 1);
  }
  return 0;
}

// Function to handle key presses based on the current menu
void handleKeyPress(int key) {
  switch (currentMenu) {
    case MENU_PATIENT:
      handlePatientKeys(key);
      break;
    case MENU_PATIENT_CODE:
      handlePatientCodeKeys(key);
      break;
    case MENU_MAIN:
      handleMainMenuKeys(key);
      break;
    case MENU_SPO2:
    case MENU_BPM:
    case MENU_TEMP:
    case MENU_DATA_TRANSFER:
      if (!isMeasuring) { // Only return to main menu if not measuring
        currentMenu = MENU_MAIN;
        showMainMenu();
      }
      break;
  }
}

// Function to handle keys in the patient menu
void handlePatientKeys(int key) {
  switch (key) {
    case 1:
      currentMenu = MENU_PATIENT_CODE;
      patientCode = "";
      showPatientCodeScreen();
      break;
    case 2:
      // Option to exit patient menu without logging in
      showWelcomeScreen();
      delay(1000); // Shorter delay
      showPatientMenu();
      break;
  }
}

// Function to handle keys in the patient code entry screen
void handlePatientCodeKeys(int key) {
  if (patientCode.length() < 4) {
    patientCode += String(key);
    lcd.setCursor(8 + patientCode.length() - 1, 1);
    lcd.print(key);
    
    if (patientCode.length() == 4) {
      verifyPatientCode();
    }
  }
}

// Function to verify the entered patient code
void verifyPatientCode() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("  Verifying Code...");
  
  // Non-blocking way to handle delay
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
    // Keep updating sensor during this "delay"
    pox.update();
  }
  
  patientLoggedIn = true;
  currentPatient = "Patient " + patientCode;
  
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("  Welcome Patient");
  lcd.setCursor(0, 2);
  lcd.print("      " + patientCode);
  
  // Non-blocking way to handle delay
  startTime = millis();
  while (millis() - startTime < 1500) {
    // Keep updating sensor during this "delay"
    pox.update();
  }
  
  currentMenu = MENU_MAIN;
  showMainMenu();
}

// Function to handle keys in the main menu
void handleMainMenuKeys(int key) {
  if (!patientLoggedIn) {
    showPleaseLogin();
    // Non-blocking way to handle delay
    unsigned long startTime = millis();
    while (millis() - startTime < 1500) {
      // Keep updating sensor during this "delay"
      pox.update();
    }
    showMainMenu();
    return;
  }
  
  switch (key) {
    case 1:
      currentMenu = MENU_SPO2;
      startMeasurement("=== SpO2 MEASUREMENT ===");
      break;
    case 2:
      currentMenu = MENU_BPM;
      startMeasurement("=== BPM MEASUREMENT ===");
      break;
    case 3:
      currentMenu = MENU_TEMP;
      startMeasurement("=== TEMP MEASUREMENT ===");
      break;
    case 4:
      currentMenu = MENU_DATA_TRANSFER;
      showDataTransferScreen();
      break;
  }
}

// Function to start a measurement (common for all sensor readings)
void startMeasurement(String title) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(title);
  lcd.setCursor(0, 1);
  lcd.print("Measuring...");
  
  // Start the measurement
  isMeasuring = true;
  measurementStartTime = millis();
  animationStep = 0;
}

// Function to display the SpO2 results
void showSpO2Results() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== SpO2 RESULTS ===");
  lcd.setCursor(0, 1);
  oldSpO2=currentSpO2;
  lcd.print("SpO2: " + String(oldSpO2) + "%");
  lcd.setCursor(0, 2);
  lcd.print("Any key to return");
}

// Function to display the BPM results
void showBPMResults() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== BPM RESULTS ===");
  lcd.setCursor(0, 1);
  oldBPM=currentBPM;
  lcd.print("BPM: " + String(oldBPM));
  lcd.setCursor(0, 2);
  lcd.print("Any key to return");
}

// Function to display the temperature results
void showTempResults() {
  sensors.requestTemperatures(); // Request temperature readings
  currentTemp = sensors.getTempCByIndex(0);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== TEMP RESULTS ===");
  lcd.setCursor(0, 1);
  oldTemp=currentTemp;
  lcd.print("Temp: " + String(oldTemp) + "C");
  lcd.setCursor(0, 2);
  lcd.print("Any key to return");
}

// Function to display the welcome screen
void showWelcomeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("======================");
  lcd.setCursor(0, 1);
  lcd.print("  Health Monitoring");
  lcd.setCursor(0, 2);
  lcd.print("      System");
  lcd.setCursor(0, 3);
  lcd.print("======================");
}

// Function to display the main menu
void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== MAIN MENU ===");
  
  if (patientLoggedIn) {
    lcd.setCursor(0, 3);
    lcd.print("User: " + currentPatient);
  }
  
  lcd.setCursor(0, 1);
  lcd.print("1:SpO2    2:BPM");
  lcd.setCursor(0, 2);
  lcd.print("3:Temp    4:Data");
}

// Function to display the "please login" message
void showPleaseLogin() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Please login first");
}

// Function to display the patient menu
void showPatientMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== PATIENT MENU ===");
  lcd.setCursor(0, 1);
  lcd.print("1: New Login");
  lcd.setCursor(0, 2);
  lcd.print("2: Exit");
}

// Function to display the patient code entry screen
void showPatientCodeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== PATIENT LOGIN ===");
  lcd.setCursor(0, 1);
  lcd.print("Code: ");
  lcd.setCursor(0, 3);
  lcd.print("Enter 4-digit code");
}

// Function to display the data transfer screen
void showDataTransferScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== DATA TRANSFER ===");
  lcd.setCursor(0, 1);
  lcd.print("Sending data...");

  // Ensure MQTT is connected
  if (!client.connected()) {
    connectToMQTT();
  }

  // Prepare JSON payload
  String payload = "{";
  payload += "\"patient_id\": \"" + patientCode + "\",";
  payload += "\"SpO2\": " + String(oldSpO2) + ",";
  payload += "\"BPM\": " + String(oldBPM) + ",";
  payload += "\"Temperature\": " + String(oldTemp);
  payload += "}";

  // Publish to ThingsBoard
  client.publish("v1/devices/me/telemetry", payload.c_str());

  Serial.println("Data sent to ThingsBoard:");
  Serial.println(payload);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=== DATA TRANSFER ===");
  lcd.setCursor(0, 1);
  lcd.print("Data sent!");
  lcd.setCursor(0, 2);
  lcd.print("Any key to return");
}
