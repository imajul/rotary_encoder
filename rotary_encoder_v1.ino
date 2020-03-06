#include "SevenSegmentTM1637.h"

volatile unsigned long temp, counter = 0;
int aux, bias = 0; 
uint8_t texto[4];

#define PIN_CLK 9       // TM1637 CLK pin 
#define PIN_DIO 8       // TM1637 DIO pin 
#define PULSE_RATE 34   // 34 pulsos de rise por cada 10 mm de avance
#define ERROR_RATE 1.003    // error sistemático en la medición      
#define RESET_PIN 6
#define MODE_PIN 7
#define MODO_CORTAR_LED 4
#define MODO_MEDIR_LED 5
#define BIAS 30        // distancia en cm entre el encoder y el borde de la tela    

typedef enum
{
  CORTAR,
  MEDIR
} ModoDeTrabajo;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
ModoDeTrabajo modo;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

SevenSegmentTM1637    display(PIN_CLK, PIN_DIO);

void setup()
{
  Serial.begin (9600);
  pinMode(2, INPUT_PULLUP); // internal pullup input pin 2
  pinMode(3, INPUT_PULLUP); // internal pullup input pin 3
  pinMode(RESET_PIN, INPUT);
  pinMode(MODE_PIN, INPUT);
  pinMode(MODO_CORTAR_LED, OUTPUT);
  pinMode(MODO_MEDIR_LED, OUTPUT);
  //Setting up interrupt
  //A rising pulse from encodenren activated ai0(). AttachInterrupt 0 is DigitalPin nr 2 on moust Arduino.
  attachInterrupt(0, ai0, RISING);

  //B rising pulse from encodenren activated ai1(). AttachInterrupt 1 is DigitalPin nr 3 on moust Arduino.
  attachInterrupt(1, ai1, RISING);

  display.begin();            // initializes the display
  display.setBacklight(100);  // set the brightness to 100 %
  display.print("HOLA");      // display INIT on the display
  delay(1000);
  display.clear();
  display.setColonOn(1);
  display.print("    ");

  modo = CORTAR;
  digitalWrite(MODO_CORTAR_LED, HIGH);
  digitalWrite(MODO_MEDIR_LED, LOW);

}

void loop()
{
  // Verifico si hubo cambio en la posicion del encoder

  if ( counter != temp )
  {
    temp = counter;
    aux = ( counter / PULSE_RATE / ERROR_RATE ) + bias;

    texto[0] = display.encode(aux / 1000);
    texto[1] = display.encode((aux % 1000) / 100);
    texto[2] = display.encode((aux % 100) / 10);
    // texto[3] = display.encode(((aux % 10) / 5) * 5);  // muestra de 5 en 5 cm
    texto[3] = display.encode(aux % 10);
    // Serial.println (aux);
    display.printRaw(texto, 4, 0);

  }

  if (digitalRead(RESET_PIN))
  {
    counter = 0;
    temp = 0;
    display.clear();
  }


  int reading = digitalRead(MODE_PIN);
  if (reading != lastButtonState)
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState)
    {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH)
      {
        switch (modo)
        {
          case CORTAR:
            modo = MEDIR;
            bias = BIAS;
            digitalWrite(MODO_MEDIR_LED, HIGH);
            digitalWrite(MODO_CORTAR_LED, LOW);
            break;

          case MEDIR:
            modo = CORTAR;
            bias = 0;
            digitalWrite(MODO_CORTAR_LED, HIGH);
            digitalWrite(MODO_MEDIR_LED, LOW);
            break;
        }
        counter = 0;
        temp = 0;
        display.clear();
      }
    }
  }
  // set the LED:
  //digitalWrite(13, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void ai0()
{
  // ai0 is activated if DigitalPin nr 2 is going from LOW to HIGH
  // Check pin 3 to determine the direction

  if (digitalRead(3) == LOW) {
    counter++;
  } else {
    if (counter > 0) counter--;
  }
}

void ai1()
{
  // ai0 is activated if DigitalPin nr 3 is going from LOW to HIGH
  // Check with pin 2 to determine the direction

  if (digitalRead(2) == LOW) {
    if (counter > 0) counter--;
  } else {
    counter++;
  }
}
