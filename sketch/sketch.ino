#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
#include "time.h"

#define WIFI_SSID "Hidden Network"
#define WIFI_PASSWORD "11223344"

#define NTP_SERVER "id.pool.ntp.org"
#define GMT_OFFSET 0
#define DAYLIGHT_OFFSET 28800

#define DATABASE_URL "smart-streetlight-167e7-default-rtdb.firebaseio.com"

#define LDR_PIN 27
#define RAIN_PIN 26
#define MIST_PIN 25

#define RED_PIN 14
#define GREEN_PIN 12
#define BLUE_PIN 13

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

struct tm* ptr;
time_t t;

char timeOnly[32];
char dateOnly[32];

String _state = "mati";
String _color = "putih";
String _tmp_state;
String _tmp_color;

int getPercen(int pin) {
  int value = analogRead(pin);
  int percen = map(value, 0, 4095, 0, 100);
  return percen;
}

int getPercenRev(int pin) {
  int value = analogRead(pin);
  int percen = map(value, 0, 4095, 100, 0);
  return percen;
}

void setState(String state) {
  if (state == "nyala") {
    _tmp_state = "nyala";
    setColor(_color);
  } else if (state == "mati") {
    _tmp_state = "mati";
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
  }
}

void setColor(String color) {
  if (color == "putih") {
    _tmp_color = "putih";
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 255);
    analogWrite(BLUE_PIN, 255);
  } else if (color == "kuning") {
    _tmp_color = "kuning";
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 128);
    analogWrite(BLUE_PIN, 0);
  }
}

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("WiFi Connected with IP: ");
  Serial.println(WiFi.localIP());

  Serial.println();

  configTime(GMT_OFFSET, DAYLIGHT_OFFSET, NTP_SERVER);

  pinMode(LDR_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(MIST_PIN, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  setState(_state);

  config.database_url = DATABASE_URL;
  config.signer.test_mode = true;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  t = time(NULL);
  ptr = localtime(&t);
  strftime(timeOnly, 32, "%H:%M", ptr);
  strftime(dateOnly, 32, "%A, %B %d %Y", ptr);

  int light = getPercenRev(LDR_PIN);
  int rain = getPercenRev(RAIN_PIN);
  int mist = getPercen(MIST_PIN);

  Serial.printf("Date: %s | Time: %s | Light: %d%% | Rain: %d%% | Mist: %d%%\n", dateOnly, timeOnly, light, rain, mist);

  if (Firebase.setString(fbdo, "/date", dateOnly)) {
    Serial.printf("Set date... ok\n");
  } else {
    Serial.printf("Set date... %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.setString(fbdo, "/sensors/time/value", timeOnly)) {
    Serial.printf("Set time... ok\n");
  } else {
    Serial.printf("Set time... %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.setInt(fbdo, "/sensors/light/value", light)) {
    Serial.printf("Set light... ok\n");
  } else {
    Serial.printf("Set light... %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.setInt(fbdo, "/sensors/rain/value", rain)) {
    Serial.printf("Set rain... ok\n");
  } else {
    Serial.printf("Set rain... %s\n", fbdo.errorReason().c_str());
  }

  if (Firebase.setInt(fbdo, "/sensors/mist/value", mist)) {
    Serial.printf("Set mist... ok\n");
  } else {
    Serial.printf("Set mist... %s\n", fbdo.errorReason().c_str());
  }

  Serial.println();
  
  Firebase.getString(fbdo, "/streetlight/state", &_state);
  Firebase.getString(fbdo, "/streetlight/color", &_color);

  if (_tmp_state == "nyala") {
    if (_state != _tmp_state) {
      setState(_state);
    }
    if (_color != _tmp_color) {
      setColor(_color);
    }
  } else if (_tmp_state == "mati") {
    if (_state != _tmp_state) {
      setState(_state);
    }
  }
  
  delay(500);
}
