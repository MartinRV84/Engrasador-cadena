#include <SparkFun_TB6612.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>

#define BUTTON_A  9 //Pusador "-"
#define BUTTON_B  6 //Pulsador "+"
#define BUTTON_C  5 //Pulsador "OK"
#define AIN1 2
#define AIN2 4
#define PWMA 5
#define STBY 9

int address1 = 0; //Direccion EEPROM "intervalo"
int address2 = 5; //Direccion EEPROM "gota"
//int intervalo = 0; 
//int gota = 0;

const int offsetA = 1;

//int temperatura = 0;
//int humedad = 0;
//int presion = 0;
//int inyector = 0; //Variable trabajo de engrase

//int envioDeDatos[6] = {temperatura, humedad, presion, gota, intervalo, inyector}; //Array para el envio de datos por Serial1/Bluetooth

struct MyData 
{
   int temperatura;
   int humedad;
   int presion;
   int gota;
   int intervalo;
   int inyector;
};

MyData datos;

unsigned long prev_millis = 0;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
Adafruit_BME280 bme;
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);

void setup()
{
  Serial.begin(9600); //Inicio puerto serial
  Serial1.begin(9600); //Inicio puerto bluetooth
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
    Serial.println("Fallo en sensores"); 
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
  delay(100);

  p_principal(); //Pantalla principal

  ajuste(); //Pantalla de ajuste

  accionamiento(); //Bombeo aceite

  datos.temperatura = bme.readTemperature();
  datos.humedad = bme.readHumidity();
  datos.presion = bme.readPressure() / 100;
  datos.intervalo = EEPROM.read(address1);  
  datos.gota = EEPROM.read(address2);
  Serial.print("Temp: ");
  Serial.print(datos.temperatura);
  Serial.print("\t\tHum: ");
  Serial.print(datos.humedad);
  Serial.print("\t\tPres: ");
  Serial.println(datos.presion);

  if(Serial1.available())
  {
    Serial1.write(&datos, sizeof(datos));    
  }

  if (!digitalRead(BUTTON_C)) //Cebado de circuito
  {
    while (digitalRead(!BUTTON_C))
    {
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.print("CEBADO");
      display.display();

      motor1.drive(255,0);
      datos.inyector = 1;
    }
    motor1.brake();
    datos.inyector = 0;
  }    
}

void ajuste()
{
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

      if (datos.intervalo != nuevoIntervalo)
      {
        EEPROM.update(address1, nuevoIntervalo);
      }
    }
  }    

    if (!digitalRead(BUTTON_B)) //Ajuste del tiempo necesario para el bombeo de una gota
  {
    int nuevoGota = EEPROM.read(address2);

    while (digitalRead(BUTTON_C))
    {
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setCursor(50, 10);
      display.setTextSize(3);
      display.print(gota);
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

      if (datos.gota != nuevoGota)
      {
        EEPROM.update(address2, nuevoGota);
      }
    }
  }    
}

void p_principal()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 8);
  display.setTextSize(1);
  display.print("Hu:");
  display.print(datos.humedad);
  display.setCursor(64, 8);
  display.print("Te:");
  display.print(datos.temperatura);
  display.setCursor(0, 16);
  display.print("Pres:");
  display.print(datos.presion);
  display.setCursor(64, 16);
  display.print("Int:");
  display.print(datos.intervalo);
  display.setCursor(0, 24);
  display.print("Gota:");
  display.print(datos.gota);
  display.display();
}

void accionamiento()
{
  if ((prev_millis + (datos.intervalo * 1000)) >= millis)
  {
    motor1.drive(255,gota);
    datos.inyector = 1;    
    motor1.brake();
    datos.inyector = 0;

    prev_millis = millis;
  }
}
