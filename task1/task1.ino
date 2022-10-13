#define LEDS 4
#include <avr/sleep.h>
#include <stdbool.h>
#include <EnableInterrupt.h>


enum State { OFF,
             BLINKING,
             WAITING_USER_INPUT,
             GAME_OVER,
             SLEEP };

int debug_led = 11;
int debug_led_brightness = 0;
int fadeAmount = 5;
int count = 0;
int penality = 0;
long prevts = 0;
long ts;

int led_states[LEDS];
int user_input[LEDS];
int leds[LEDS] = { 10, 9, 8, 7 };
int buttons[LEDS] = { 6, 5, 4, 3 };
State currentState;

void setup() {
  Serial.begin(9600);
  setup_hw();
  setup_current_state();
  randomSeed(analogRead(0));
}

void setup_hw() {
  //configuring game leds and buttons
  for (int i = 0; i < LEDS; i++) {
    //leds
    led_states[i] = 0;
    pinMode(leds[i], OUTPUT);
    //buttons
    user_input[i] = 0;
    pinMode(buttons[i], INPUT);
    enableInterrupt(buttons[i], interruptCheckState, RISING);
  }
  //configuring debug led
  pinMode(debug_led, OUTPUT);
}


void setup_current_state() {
  currentState = OFF;
}

void sleep_setup() {
  currentState = SLEEP;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
}

void handle_off_state() {
  ts = micros();
  if (ts - prevts > 10000000) {
    prevts = ts;
    debug_led_brightness = 0;
    analogWrite(debug_led, debug_led_brightness);
    sleep_setup();
  }
  analogWrite(debug_led, debug_led_brightness);
  debug_led_brightness += fadeAmount;
  if (debug_led_brightness <= 0 || debug_led_brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  delay(30);
}

void blinking() {
  for (int i = 0; i < LEDS; i++) {
    led_states[i] = random(0, 2);
  }
  for (int i = 0; i < LEDS; i++) {
    analogWrite(leds[i], (led_states[i] == 0) ? 0 : 255);
  }
  delay(1000);
  for (int i = 0; i < LEDS; i++) {
    analogWrite(leds[i], 0);
  }
  currentState = WAITING_USER_INPUT;
}

void waiting_user_input() {
  enableInterrupt(buttons[0], interrupt0Check, RISING);
  enableInterrupt(buttons[1], interrupt1Check, RISING);
  enableInterrupt(buttons[2], interrupt2Check, RISING);
  enableInterrupt(buttons[3], interrupt3Check, RISING);
  delay(5000);
  currentState = GAME_OVER;
}

void interruptCheck(int n) {
  user_input[n] = 1;
}

void interrupt0Check() {
  interruptCheck(0);
}

void interrupt1Check() {
  interruptCheck(1);
}

void interrupt2Check() {
  interruptCheck(2);
}

void interrupt3Check() {
  interruptCheck(3);
}

void game_over() {
  if (memcmp(led_states, user_input, LEDS) == 0) {
    count++;
    Serial.print("you won!!");
    Serial.println(count);
    currentState = BLINKING;
  } else {
    Serial.print("you lost!!");
    Serial.println(count);
    penality++;
    if (penality == 3) {
      Serial.println("game over!!");
      penality = 0;
      count = 0;
      currentState = OFF;
    }
    currentState = BLINKING;
  }
  for (int i = 0; i < LEDS; i++) {
    enableInterrupt(buttons[i], interruptCheckState, RISING);
    user_input[i] = 0;
  }
}

void interruptCheckState() {
  switch (currentState) {
    case OFF:
      currentState = BLINKING;
      debug_led_brightness = 0;
      analogWrite(debug_led, debug_led_brightness);
      break;
    case BLINKING:
      penality++;
      if (penality == 3) {
        Serial.println("game over!!");
        penality = 0;
        count = 0;
        currentState = OFF;
      }
      break;
    case WAITING_USER_INPUT:
      break;
    case GAME_OVER:
      break;
    case SLEEP:
      sleep_disable();
      currentState = OFF;
      break;
    default:
      break;
  }
}

void loop() {
  switch (currentState) {
    case OFF:
      handle_off_state();
      break;
    case BLINKING:
      blinking();
      break;
    case WAITING_USER_INPUT:
      waiting_user_input();
      break;
    case GAME_OVER:
      game_over();
      break;
    case SLEEP:
      break;
    default:
      handle_off_state();
      break;
  }
}