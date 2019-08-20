#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewTone.h>
#include <IRremote.h>

#include "SmartHouse.h"

/*
 * Three automates
 */
smart_house room (SG_WILLINGNESS, SM_MENU_ALARM, SI_HOURS_DECADE);

void setup()
{
  room.primarySettings ();
  Serial.begin (9600);
}

void loop()
{
  room.automateInput();
  room.automateOutput();
}
