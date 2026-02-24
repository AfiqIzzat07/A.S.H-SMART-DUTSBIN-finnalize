#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <NewPing.h>
#include <Firebase_ESP_Client.h>
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>

/* ================= WIFI ================= */
const char* WIFI_SSID = "kmk";
const char* WIFI_PASS = "J1T12007";

/* ================= FIREBASE ================= */
#define API_KEY "AIzaSyBD990zggrFpi5Z2DwAOLShTjJUDN8ydBo"
#define DATABASE_URL "https://smart-dustbin-150307-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/* ================= TELEGRAM ================= */
const char* BOT_TOKEN = "8456561690:AAGdyXXTWpM_9LKriH6sjsaAz5CSYpiuXF4";
const char* CHAT_ID  = "-5249451947";
WiFiClientSecure client;

/* ================= PINS ================= */
#define TRIG_PIN 5
#define ECHO_PIN 18
#define PIR_PIN  19
#define SERVO_PIN 23
#define LED_FULL 26
#define LED_OK   27

/* ================= BIN CONFIG ================= */
#define BIN_HEIGHT_CM 40
#define FULL_THRESHOLD 80
#define PICKUP_THRESHOLD 20

/* ================= OBJECTS ================= */
Servo binServo;
NewPing sonar(TRIG_PIN, ECHO_PIN, BIN_HEIGHT_CM);

/* ================= STATES ================= */
enum BinState { NOT_FULL, FULL };
BinState binState = NOT_FULL;

bool binOpen = false;
bool cleanerControl = false;

/* ================= TELEGRAM SAFE SEND ================= */
bool sendTelegram(String msg) {
  for (int i = 0; i < 3; i++) {
    client.setInsecure();
    if (client.connect("api.telegram.org", 443)) {
      String url = "/bot" + String(BOT_TOKEN) +
                   "/sendMessage?chat_id=" + CHAT_ID +
                   "&text=" + msg;
      client.print(
        "GET " + url + " HTTP/1.1\r\n"
        "Host: api.telegram.org\r\n"
        "Connection: close\r\n\r\n"
      );
      delay(500);
      return true;
    }
    delay(1000);
  }
  return false;
}

/* ================= ULTRASONIC ================= */
int getFullness() {
  int d = sonar.ping_cm();
  if (d == 0) return 0;
  int p = map(d, BIN_HEIGHT_CM, 0, 0, 100);
  return constrain(p, 0, 100);
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_FULL, OUTPUT);
  pinMode(LED_OK, OUTPUT);

  binServo.attach(SERVO_PIN);
  binServo.write(0);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = "afiqizzat1105@gmail.com";
  auth.user.password = "J1T12007";
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  sendTelegram("🟢 Smart Dustbin Online");
}

/* ================= LOOP ================= */
void loop() {
  int fullness = getFullness();

  Firebase.RTDB.setInt(&fbdo, "/dustbin/fullness", fullness);

  /* ===== LED ===== */
  digitalWrite(LED_FULL, fullness >= FULL_THRESHOLD);
  digitalWrite(LED_OK, fullness < FULL_THRESHOLD);

  /* ===== TELEGRAM STATE MACHINE ===== */
  if (fullness >= FULL_THRESHOLD && binState == NOT_FULL) {
    if (sendTelegram("🚨 Bin FULL (" + String(fullness) + "%)")) {
      binState = FULL;
    }
  }

  if (fullness <= PICKUP_THRESHOLD && binState == FULL) {
    if (sendTelegram("✅ Bin cleaned (" + String(fullness) + "%)")) {
      binState = NOT_FULL;
    }
  }

  /* ===== CLEANER BUTTON ===== */
  if (Firebase.RTDB.getString(&fbdo, "/dustbin/servo")) {
    String cmd = fbdo.stringData();
    if (cmd == "OPEN") {
      cleanerControl = true;
      binOpen = true;
      binServo.write(90);
    }
    if (cmd == "CLOSE") {
      cleanerControl = false;
      binOpen = false;
      binServo.write(0);
    }
    Firebase.RTDB.setString(&fbdo, "/dustbin/servo", "");
  }

  /* ===== PIR (BLOCKED IF CLEANER OR FULL) ===== */
  if (!cleanerControl && fullness < FULL_THRESHOLD &&
      digitalRead(PIR_PIN) == HIGH && !binOpen) {

    binOpen = true;
    binServo.write(90);
    delay(10000);
    binServo.write(0);
    binOpen = false;
  }

  delay(700);
}
