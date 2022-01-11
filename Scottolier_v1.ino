#include <SparkFun_TB6612.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

#define BUTTON_A  9 //Pusador "-"
#define BUTTON_B  6 //Pulsador "+"
#define BUTTON_C  5 //Pulsador "OK"
#define AIN1 10
#define AIN2 11
#define PWMA 12
#define STBY 23

#define ledPin 13

int address1 = 0; //Direccion EEPROM "intervalo"
int address2 = 5; //Direccion EEPROM "gota"
int intervalo = 0; 
long gota = 0;

const int offsetA = 1;

int temperatura = 0;
int humedad = 0;
int presion = 0;
bool inyector = false; //Variable trabajo de engrase

unsigned long prev_millis = 0;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BME280 bme;
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);

void setup()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Inicio LCD
  bme.begin(0x77, &Wire); //Inicio sensor

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("CHAIN LUBE    V1");
  display.display();

  delay(3000);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  if (!bme.begin()) //Comprobacion de sensor
  {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Fallo en sensor.");
    display.display();
    while (1);
  }
}

void loop()
{
temperatura = bme.readTemperature();
  humedad = bme.readHumidity();
  presion = bme.readPressure() / 100;
  intervalo = EEPROM.read(address1);  
  gota = EEPROM.read(address2);
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 8);
  display.setTextSize(1);
  display.print("Hu:");
  display.print(humedad);
  display.setCursor(64, 8);
  display.print("Te:");
  display.print(temperatura);
  display.setCursor(0, 16);
  display.print("Pres:");
  display.print(presion);
  display.setCursor(64, 16);
  display.print("Int:");
  display.print(intervalo);
  display.setCursor(0, 24);
  display.print("Gota:");
  display.print(gota);
  display.setCursor(64, 24);
  display.print("Inyector:");
  display.print(inyector);
  display.display();

  if (!digitalRead(BUTTON_A)) //Ajuste de intevalo entre goteo
  {
    int nuevoIntervalo = EEPROM.read(address1);

    while (digitalRead(BUTTON_C))
    {
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setCursor(50, 10);
      display.setTextSize(3);
      display.print(nuevoIntervalo);
      display.display();

      if (!digitalRead(BUTTON_A))
      {
        nuevoIntervalo = nuevoIntervalo - 1;
        delay(100);
      }

      if (!digitalRead(BUTTON_B))
      {
        nuevoIntervalo = nuevoIntervalo + 1;
        delay(100);
      }
    }

      if (intervalo != nuevoIntervalo)
      {
        EEPROM.update(address1, nuevoIntervalo);
      }
  }    

    if (!digitalRead(BUTTON_B)) //Ajuste del tiempo necesario para el bombeo de una gota
  {
    long nuevoGota = EEPROM.read(address2);

    while (digitalRead(BUTTON_C))
    {
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setCursor(50, 10);
      display.setTextSize(3);
      display.print(nuevoGota);
      display.display();

      if (!digitalRead(BUTTON_A))
      {
        nuevoGota = nuevoGota - 1;
        delay(100);
      }

      if (!digitalRead(BUTTON_B))
      {
        nuevoGota = nuevoGota + 1;
        delay(100);
      }
    }

      if (gota != nuevoGota)
      {
        EEPROM.update(address2, nuevoGota);
      }
  }    

  if ((prev_millis + (intervalo * 1000)) >= millis)
  {
    motor1.drive(255, gota);
    prev_millis = millis;
  }
}
