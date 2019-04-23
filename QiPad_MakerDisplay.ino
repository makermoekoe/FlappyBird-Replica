#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid     = "maker";
const char* password = "maker123";
IPAddress destIP(192, 168, 4, 1);
const int port = 2245;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
WiFiUDP udp;



int r[] = {0, 0, 0}; //value
const int s[] = {14, 12, 13}; //pins
const int pin_read = 16;
boolean button_state[8];
boolean old_button_state[8];


const int led = 2;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 3; i++)
    pinMode(s[i], OUTPUT);
  pinMode(pin_read, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connect to maker ");
  PrintText("Connect...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(". ");
  }
  Serial.println();
  Serial.println("Connected.");
  Serial.print(WiFi.localIP());
  PrintText("Connected");
  delay(500);

  PrintText("Qi-Pad v1.0", "@maker.moekoe");
  delay(1000);

  udp.begin(port);
}

void loop() {

  readButtons();

  for (int i = 0; i < 8; i++) {
    if (button_state[i] != old_button_state[i]) {
      String str = String(i) + "-" + String(button_state[i]) + "E";
      PrintText(str);
      Serial.println(str);
      udp.beginPacket(destIP, port);
      udp.print(str);
      udp.endPacket();
      delay(10);
      old_button_state[i] = button_state[i];
    }
  }
}


void readButtons() {
  for (int y_count = 0; y_count < 8; y_count++) {
    for (int i = 0; i < 3; i++) r[i] = bitRead(y_count, i);
    for (int i = 0; i < 3; i++) digitalWrite(s[i], r[i]);

    boolean state = !digitalRead(pin_read);
    button_state[y_count] = state;
    //Serial.print(state);
    //Serial.print("\t");
  }
  //Serial.println();
}


void PrintText(String txt1) {
  //Zeigt den angegebenen Text mittig auf dem OLED bildschirm an (eine Zeile)

  int g;
  int n = txt1.length();

  g = display.width() / (6 * n);
  if (g > 6) g = 6;

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(g);
  display.setCursor(X(g, n), Y(g, 0.5));
  display.println(txt1);
  display.display();
}//end void PrintText

void PrintText(String txt1, String txt2) {
  //Zeigt den angegebenen Text mittig auf dem OLED bildschirm an (zwei Zeilen)

  int g;
  int n1 = txt1.length();
  int n2 = txt2.length();

  if (n1 > n2) g = 128 / (6 * n1);
  else g = 128 / (6 * n2);

  if (g > 4) g = 4;

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(g);
  display.setCursor(X(g, n1), Y(g, 0.25));
  display.println(txt1);
  display.setCursor(X(g, n2), Y(g, 0.75));
  display.println(txt2);
  display.display();
}//end void PrintText

void PrintTextFor(String txt1, int time) {
  //Zeigt den angegebenen Text mittig auf dem OLED bildschirm an für eine bestimmte Zeit (eine Zeile)
  PrintText(txt1);
  delay(time * 1000);
}//end void PrintText

void PrintTextFor(String txt1, String txt2, int time) {
  //Zeigt den angegebenen Text mittig auf dem OLED bildschirm an für eine bestimmte Zeit (zwei Zeilen)
  PrintText(txt1, txt2);
  delay(time * 1000);
}//end void PrintText


int X(int textgroesse, int n) {
  //gibt die X koordinate aus, damit text mit n zeichen mittig ist
  return (0.5 * (display.width() - textgroesse * (6 * n - 1)));
}//end int X

int Y(int textgroesse, float f) {
  //gibt die Y koordinate aus, damit text mittig ist
  return (f * display.height() - (textgroesse * 4));
}//end int Y
