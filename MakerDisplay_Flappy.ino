#include <Wire.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_IS31FL3731.h>
#include <IS31FL3731.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


const char* ssid     = "maker";
const char* password = "maker123";
IPAddress IP(192, 168, 4, 0);
IPAddress local;

int broadcast = 255;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
const int port = 2245;
WiFiUDP udp;

int destPort = port;
IPAddress destIP = IP;

const int led = 2;


//Adafruit_IS31FL3731 matrixLL = Adafruit_IS31FL3731();
//Adafruit_IS31FL3731 matrixLR = Adafruit_IS31FL3731();

IS31FL3731 matrixLL = IS31FL3731();
IS31FL3731 matrixLR = IS31FL3731();
IS31FL3731 matrixRL = IS31FL3731();
IS31FL3731 matrixRR = IS31FL3731();



//FLAPPY
#define COLOR_OFF     0
#define COLOR_WALL    1
#define COLOR_FLAPPY  2
#define COLOR_GAMEOVER  3

int t_step = 100; // time

const byte height = 9;
const byte width = 64;
const byte width_game = 48;
uint8_t matrix[width][height];
byte wall[height];
byte gen_wall[height];

int wall_every_x = 10;
int wall_counter = 0;


double flappy_bounce = 0;
double flappy_pos = 0;
byte flappy_matrix = height / 2;
float counter = 0.0;
int H = 10;
float start_point = 0.0;

int wall_gap = 4;

boolean flappy_run = false;
boolean game_over = false;
int score_counter = 0;


boolean button_pressed = false;
boolean button_state[] = {false, false, false, false, false, false, false};




void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);

  twi_setClock(400000);
  if (! matrixLL.begin(0x74)) {
    Serial.println("IS31 not found");
    while (1);
  }
  if (! matrixLR.begin(0x77)) {
    Serial.println("IS31 not found");
    while (1);
  }
  if (! matrixRL.begin(0x75)) {
    Serial.println("IS31 not found");
    while (1);
  }
  if (! matrixRR.begin(0x76)) {
    Serial.println("IS31 not found");
    while (1);
  }

  matrixLR.setTextWrap(false);
  matrixLL.setTextWrap(false);
  matrixRL.setTextWrap(false);
  matrixRR.setTextWrap(false);
  matrixLL.setTextColor(100, 0);
  matrixLR.setTextColor(100, 0);
  matrixRL.setTextColor(100, 0);
  matrixRR.setTextColor(100, 0);
  matrixLL.setRotation(0);
  matrixLR.setRotation(0);
  matrixRL.setRotation(0);
  matrixRR.setRotation(0);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  udp.begin(port);


  int _position = 0;
  char txt[] = "makermoekoe";
  matrixLL.setCursor(_position, 0);
  matrixLL.print(txt);
  matrixLR.setCursor(_position - 16, 0);
  matrixLR.print(txt);
  matrixRL.setCursor(_position - 32, 0);
  matrixRL.print(txt);
  matrixRR.setCursor(_position - 48, 0);
  matrixRR.print(txt);

  matrixLL.display();
  matrixLR.display();
  matrixRL.display();
  matrixRR.display();
  delay(2000);

}



unsigned long t = millis();
unsigned long t_flappy = millis();


void loop() {

  if (flappy_run) {
    run_flappy();
  }

  if (game_over) {
    Serial.println(score_counter);


    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        matrix[x][y] = COLOR_WALL;
        matrix[width - x - 1][y] = COLOR_WALL;
      }
      showMatrix_all();
      delay(10);
    }

    int _position = 0;

    clearDisp();
    String a = "SCORE: ";
    String b = String(score_counter);
    a = a + b;
    char txt[20];
    a.toCharArray(txt, 20);
    matrixLL.setCursor(_position, 1);
    matrixLL.print(txt);
    matrixLR.setCursor(_position - 16, 1);
    matrixLR.print(txt);
    matrixRL.setCursor(_position - 32, 1);
    matrixRL.print(txt);
    matrixRR.setCursor(_position - 48, 1);
    matrixRR.print(txt);

    matrixLL.display();
    matrixLR.display();
    matrixRL.display();
    matrixRR.display();

    game_over = false;
    score_counter = 0;
  }

  int packetSize = udp.parsePacket();
  if (packetSize) {
    digitalWrite(led, LOW);
    destPort = udp.remotePort();
    destIP = udp.remoteIP();
    String command = readUdp();
    int button = -1;
    int value = handleCommand(command, button);

    for (int i = 0; i < 7; i++) if (button == i) button_state[i] = value;


    if (button_state[5] == 1) {
      flappy_run = !flappy_run;
      Serial.println("run_flappy");
      if (flappy_run) {
        clear_matrix();
        clearDisp();
        wall_every_x = 10;
        t_step = 100;
        //score_counter = 0;

        char txt[20];
        String(score_counter).toCharArray(txt, 20);

        matrixRR.fillRect(0, 0, 16, 9, 0);
        matrixRR.setCursor(4, 1);
        matrixRR.print(txt);
        matrixRR.display();

        t = millis();
        t_flappy = millis();
      }
    }
    digitalWrite(led, HIGH);
  }



}


void run_flappy() {
  if (button_state[0] == 1 && button_pressed == false) {  //pushing up
    counter = 0.0;
    start_point = flappy_pos;
    button_pressed = true;
  }
  else if (button_state[0] == 0 && button_pressed == true) {
    button_pressed = false;
  }


  if (millis() > t_flappy + 3) {
    flappy_bounce = H * counter - H / 8 * counter * counter;

    flappy_pos = constrain(start_point + flappy_bounce, 0, 100);
    counter = counter + 1.0 / 3.0;

    if (flappy_pos < 0) {
      counter = 0.1;
      start_point = flappy_pos;
    }

    flappy_matrix = byte(map(flappy_pos, 0, 100, height - 1, 1));
    setFlappy(flappy_matrix);

    showMatrix();

    t_flappy = millis();
  }


  if (millis() > t + t_step) {

    if (score_counter == 20) {
      wall_every_x = 8;
    }
    if (score_counter == 30) {
      t_step = 80;
    }
    if (score_counter == 50) {
      wall_every_x = 5;
      t_step = 60;
    }


    wall_counter++;
    if (wall_counter == wall_every_x) {
      shift_matrix(generate_wall(true));
      wall_counter = 0;
    }
    else {
      shift_matrix(generate_wall(false));
    }

    if (matrix[1][0] == COLOR_WALL) {
      score_counter++;

      char txt[20];
      String(score_counter).toCharArray(txt, 20);

      matrixRR.fillRect(0, 0, 16, 9, 0);
      matrixRR.setCursor(4, 1);
      matrixRR.print(txt);
      matrixRR.display();
    }

    //showMatrix();
    t = millis();
  }


}

void setFlappy(byte _y) {
  if (matrix[2][_y] == COLOR_OFF) {
    matrix[2][_y] = COLOR_FLAPPY;
  }
  else if (matrix[2][_y] == COLOR_WALL) {
    Serial.println("GAME OVER");
    game_over = true;
    flappy_run = false;
  }
}

void showSerial() {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Serial.print(matrix[x][y]);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();
}


void showMatrix() {
  matrixLL.fillRect(0, 0, 16, 9, 0);
  matrixLR.fillRect(0, 0, 16, 9, 0);
  matrixRL.fillRect(0, 0, 16, 9, 0);
  //matrixRR.fillRect(0, 0, 16, 9, 0);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width_game; x++) {
      setPixelXY(x, y, matrix[x][y]);
    }
  }
  matrixLL.display();
  matrixLR.display();
  matrixRL.display();
  //matrixRR.display();
}

void showMatrix_all() {
  matrixLL.fillRect(0, 0, 16, 9, 0);
  matrixLR.fillRect(0, 0, 16, 9, 0);
  matrixRL.fillRect(0, 0, 16, 9, 0);
  matrixRR.fillRect(0, 0, 16, 9, 0);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      setPixelXY(x, y, matrix[x][y]);
    }
  }
  matrixLL.display();
  matrixLR.display();
  matrixRL.display();
  matrixRR.display();
}


void shift_matrix(byte * _wall) {
  for (int x = 1; x < width_game; x++) {
    for (int y = 0; y < height; y++) {
      matrix[x - 1][y] = matrix[x][y];
    }
  }
  for (int y = 0; y < height; y++)
    matrix[width_game - 1][y] = _wall[y];
}


void clear_matrix() {
  for (int x = 1; x < width; x++) {
    for (int y = 0; y < height; y++) {
      matrix[x][y] = COLOR_OFF;
    }
  }
}


byte* generate_wall(boolean generate) {
  if (generate) {
    for (int y = 0; y < height; y++)
      gen_wall[y] = COLOR_WALL;
    byte ran = random(1, height - wall_gap);

    for (int y = 0; y < wall_gap; y++)
      gen_wall[ran + y] = COLOR_OFF;
  }
  else {
    for (int y = 0; y < height; y++)
      gen_wall[y] = COLOR_OFF;
  }
  return gen_wall;
}



int handleCommand(String c, int &but) {
  int firstsep = c.indexOf("-");
  int secondsep = c.indexOf("E");
  but = c.substring(0, firstsep).toInt();

  return c.substring(firstsep + 1, secondsep).toInt();
}

String readUdp() {
  udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
  String reply = String(packetBuffer);
  for (int i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++) packetBuffer[i] = 0;
  return reply;
}//end String readUdp()


void clearDisp() {
  matrixLL.fillRect(0, 0, 16, 9, 0);
  matrixLR.fillRect(0, 0, 16, 9, 0);
  matrixRL.fillRect(0, 0, 16, 9, 0);
  matrixRR.fillRect(0, 0, 16, 9, 0);
  matrixLL.display();
  matrixLR.display();
  matrixRL.display();
  matrixRR.display();
}


void setPixelXY(byte x, byte y, byte value) {
  if (x >= 48) {
    x = x - 48;
    if (value == COLOR_OFF) matrixRR.drawPixel(x, y, 0);
    if (value == COLOR_FLAPPY) matrixRR.drawPixel(x, y, 120);
    if (value == COLOR_WALL) matrixRR.drawPixel(x, y, 10);
  }
  else if (x >= 32) {
    x = x - 32;
    if (value == COLOR_OFF) matrixRL.drawPixel(x, y, 0);
    if (value == COLOR_FLAPPY) matrixRL.drawPixel(x, y, 120);
    if (value == COLOR_WALL) matrixRL.drawPixel(x, y, 10);
  }
  else if (x >= 16) {
    x = x - 16;
    if (value == COLOR_OFF) matrixLR.drawPixel(x, y, 0);
    if (value == COLOR_FLAPPY) matrixLR.drawPixel(x, y, 120);
    if (value == COLOR_WALL) matrixLR.drawPixel(x, y, 10);
  }
  else {
    if (value == COLOR_OFF) matrixLL.drawPixel(x, y, 0);
    if (value == COLOR_FLAPPY) matrixLL.drawPixel(x, y, 120);
    if (value == COLOR_WALL) matrixLL.drawPixel(x, y, 10);
  }
}
