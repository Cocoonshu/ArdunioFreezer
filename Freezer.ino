#include <Arduino_FreeRTOS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <semphr.h>

#define SERIAL_RATE       9600
#define DS18020_PIN       4
#define ENCODER_LEFT_PIN  2
#define ENCODER_RIGHT_PIN 3
#define TEMPERATURE_STEP  0.1f
#define LEFT              true
#define RIGHT             false
#define PRIO_MAX          3
#define PRIO_HIGT         2
#define PRIO_NORMAL       1
#define PRIO_LOW          0
#define TASK_BEGIN        for (;;) {
#define TASK_END          }
#define INCREASE          {setupTemperature += TEMPERATURE_STEP;}
#define DECREASE          {setupTemperature -= TEMPERATURE_STEP;}

volatile float    setupTemperature(0);
volatile float    currentTemperature(0);
OneWire           oneWire(DS18020_PIN);
DallasTemperature sensor(&oneWire);
SemaphoreHandle_t xSerialSemaphore;

void taskUpdateTemperature(void *param);
void onLeftInterrupted();
void onRightInterrupted();
void handleInterrupted(boolean side);

void setup(void)
{
  Serial.begin(SERIAL_RATE);
  Serial.println("Dallas Temperature Control Library Demo - TwoPin_DS18B20");
  sensor.begin();
  createSemaphores();
  createTasks();
  createInterrupts();
}

void createSemaphores()
{
  if (xSerialSemaphore == NULL ) {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if (xSerialSemaphore != NULL) {
      xSemaphoreGive(xSerialSemaphore);
    }
  }
}

void createTasks()
{
  // Create temperature updating task
  xTaskCreate(
    taskUpdateTemperature, (const portCHAR *) "Temperature",
    128, NULL, PRIO_NORMAL, NULL);
}

void createInterrupts()
{
  pinMode(ENCODER_LEFT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT_PIN), onLeftInterrupted, RISING);
  //attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT_PIN), onRightInterrupted, CHANGE);
}

void taskUpdateTemperature(void* param __attribute__((unused)))
{
  TASK_BEGIN
  sensor.requestTemperatures();
  currentTemperature = sensor.getTempCByIndex(0);
  if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
    Serial.print("Temperatures: ");
    Serial.println(currentTemperature);
    xSemaphoreGive(xSerialSemaphore);
  }
  vTaskDelay( 1000 / portTICK_PERIOD_MS );
  TASK_END
}

void onLeftInterrupted()
{ handleInterrupted(LEFT); }

void onRightInterrupted()
{ handleInterrupted(RIGHT); }

void handleInterrupted(boolean side)
{
  int leftPin  = digitalRead(ENCODER_LEFT_PIN);
  int rightPin = digitalRead(ENCODER_RIGHT_PIN);
  if (side == LEFT) {
    if ((leftPin == HIGH && rightPin == HIGH) || (leftPin == LOW && rightPin == LOW)) {
      DECREASE; // <-
    } else {
      INCREASE; // ->
    }
  } else {
    if ((leftPin == HIGH && rightPin == HIGH) || (leftPin == LOW && rightPin == LOW)) {
      INCREASE; // ->
    } else {
      DECREASE; // <-
    }
  }

  if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
    Serial.print("Setup: ");
    Serial.println(setupTemperature);
    xSemaphoreGive(xSerialSemaphore);
  }
}

void loop(void)
{ /* Empty implemented */
}
