#include "SmartHouse.h"

#include  < Arduino.h >
#include  < Wire.h >
#include  < LiquidCrystal_I2C.h >
#include  < NewTone.h >
#include  < IRremote.h >

IRrecv irrecv(recv);
decode_results results;
LiquidCrystal_I2C lcd(0x3F, 16, 2);
/*
 * Times
 */
extern int my_user_timer=0;
extern int pump_timer=0;
extern int wait=0;
extern int hours_G=0;      extern int minutes_G=0;      extern int seconds_G=0;   // System
extern int hours_A=0;      extern int minutes_A=0;                                // Alarm
extern int hours_S=0;      extern int minutes_S=0;                                // Relay
extern int half_hours=0;   extern int half_minutes=0;   extern int half_seconds=0;
/*
 * Analogs
 */
extern int water_level=500;
extern int humidity_soil_level=600;
extern int light_level=560;
/*
 * Switchers
 */
extern bool open_auto=true;
extern bool alarm_on=false;
extern bool time_switch_on=false;
extern bool user_timer_on=false;
extern bool sleep_on=false;
extern bool door_0_on=false;
extern bool door_1_on=false;
extern bool no_water_indication=false;

smart_house::smart_house(StatesG _condition, StatesM _condition_main_menu, StatesI _condition_index)
{
  smart_house::condition = _condition;
  smart_house::condition_main_menu = _condition_main_menu;
  smart_house::condition_index = _condition_index;
}

void smart_house::primarySettings()
{
  pinMode(recv, INPUT);
  pinMode(HDX_0, INPUT);
  pinMode(HDX_1, INPUT);
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(bell, OUTPUT);
  pinMode(servo, OUTPUT);
  pinMode(light_resistance, INPUT);
  pinMode(water_sensor, INPUT);
  pinMode(humidity_sensor, INPUT);
  for(int i=0; i < 25; i++)
    {
      digitalWrite(servo, HIGH);
      delayMicroseconds(544);
      digitalWrite(servo, LOW);
      delayMicroseconds(456);
      delay(19);
    }
  irrecv.enableIRIn();
  lcd.init();
  lcd.backlight();
  displayText(1);  // Friendly UI
}

void smart_house::automateInput()
{// Order is important
  smart_house::lightInput();
  smart_house::humiditySoilSensor();
  smart_house::waterLevelSensor();
  smart_house::DHXZero();
  smart_house::DHXOne();
  smart_house::enterRemote();
  smart_house::globalTime();
}

void smart_house::automateOutput()
{
  switch(smart_house::condition)
  {
    case SG_ONE:           smart_house::relayOne();             break;
    case SG_RELAY_1_ON:    smart_house::relayOne();             break;
    case SG_RELAY_1_OFF:   smart_house::relayOne();             break;
    case SG_TWO:           smart_house::relayTwo();             break;
    case SG_RELAY_2_ON:    smart_house::relayTwo();             break;
    case SG_RELAY_2_OFF:   smart_house::relayTwo();             break;
    case SG_THREE:         smart_house::relayThree();           break;
    case SG_RELAY_3_ON:    smart_house::relayThree();           break;
    case SG_RELAY_3_OFF:   smart_house::relayThree();           break;
    case SG_SEVEN:         smart_house::sleep_on();             break;
    case SG_NINE:          smart_house::openAuto();             break;
    case SG_RIGHT:         smart_house::moveMainMenuRight();    break;
    case SG_LEFT:          smart_house::moveMainMenuLeft();     break;
    case SG_PAUSE:         smart_house::setSomeTime();          break;
    case SG_LOW_WATER:     smart_house::signalWaterLevel();     break;
    case SG_WATER_FLOWERS: smart_house::waterFlowers();         break;
    case SG_SHOW_TIME_G:   smart_house::showGlobalTime(0);      break;
  }
}

void smart_house::globalTime()
{
  long now = millis() % 1000;
  if(now < 3 || now > 997)
  {
    delay(5);
    seconds_G++;
    Serial.print("water: ");
    Serial.println(water_level);
    Serial.print("humidity: ");
    Serial.println(humidity_soil_level);
    Serial.print("light: ");
    Serial.println(light_level);
    smart_house::condition = SG_SHOW_TIME_G;
    if(wait == 0)
    smart_house::checkActs();
    if(seconds_G > 59)
    {
      seconds_G=0;
      minutes_G++;
      if(minutes_G > 59)
      {
        minutes_G=0;
        hours_G++;
        if(hours_G > 23)
        {
          hours_G=0;
        }
      }
    }
  }
}

void smart_house::checkActs()
{
  if(pump_timer > 0)
  pump_timer--;
  if(time_switch_on)
  {
    if(hours_G == hours_S && minutes_G == minutes_S)
    {
      smart_house::condition=SG_RELAY_2_ON;
      smart_house::relayTwo();
      time_switch_on=false;
    }
  }
  if(alarm_on)
  {
    if(hours_G == hours_A && minutes_G == minutes_A)
    {
      smart_house::condition=SG_WILLINGNESS;
      wait = 60;
      NewTone(bell, 400);  // Sounds
      while(smart_house::condition == SG_WILLINGNESS)
      {
        smart_house::enterRemote();
        smart_house::globalTime();
        if(smart_house::condition == SG_SHOW_TIME_G)
        {//Если прошла секуда
          wait--;
          smart_house::showGlobalTime(0);  // Return SG_WILLINGNESS
          if(wait == 0)
          {
            minutes_A += 10;
            if(minutes_A >= 0 && minutes_A <= 9)
            hours_A++;
            noNewTone();
            break;
          }
        }
        if(smart_house::condition != SG_WILLINGNESS)
        {
          alarm_on = false;
          wait = 0;
          noNewTone();
          break;
        }
      }
      smart_house::condition=SG_WILLINGNESS;
    }
  }
  if(user_timer_on)
  {
    my_user_timer--;
    if(my_user_timer == 0)
    {
      smart_house::condition = SG_WILLINGNESS;
      wait = 60;
      NewTone(bell, 500);
      while(smart_house::condition == SG_WILLINGNESS)
      {
        smart_house::enterRemote();
        smart_house::globalTime();
        if(smart_house::condition == SG_SHOW_TIME_G)
        {
          wait--;
          smart_house::showGlobalTime(0);
          if(wait == 0)
          break;
        }
      }
      user_timer_on = false;
      noNewTone();
      wait = 0;
      smart_house::condition = SG_WILLINGNESS;
    }
  }
}
void smart_house::showGlobalTime(int row)
{
  if(hours_G > 9)
  {
    lcd.setCursor(4, row);
    lcd.print(hours_G);
  }
  else
  {
    lcd.setCursor(4, row);
    lcd.print("0");
    lcd.setCursor(5, row);
    lcd.print(hours_G);
  }
  lcd.setCursor(6, row);
  lcd.print(":");
  if(minutes_G > 9)
  {
    lcd.setCursor(7, row);
    lcd.print(minutes_G);
  }
  else
  {
    lcd.setCursor(7, row);
    lcd.print("0");
    lcd.setCursor(8,row);
    lcd.print(minutes_G);
  }
  lcd.setCursor(9, row);
  lcd.print(":");
  if(seconds_G > 9)
  {
    lcd.setCursor(10, row);
    lcd.print(seconds_G);
  }
  else
  {
    lcd.setCursor(10, row);
    lcd.print("0");
    lcd.setCursor(11,row);
    lcd.print(seconds_G);
  }
  smart_house::condition = SG_WILLINGNESS;
}

void smart_house::enterRemote()
{
  if(irrecv.decode(&results))
  {
    switch(results.value)
    {
      case 0xFF6897:  smart_house::condition = SG_ZERO;    break; //0
      case 0xFF30CF:  smart_house::condition = SG_ONE;     break; //1
      case 0xFF18E7:  smart_house::condition = SG_TWO;     break; //2
      case 0xFF7A85:  smart_house::condition = SG_THREE;   break; //3
      case 0xFF10EF:  smart_house::condition = SG_FOUR;    break; //4
      case 0xFF38C7:  smart_house::condition = SG_FIVE;    break; //5
      case 0xFF5AA5:  smart_house::condition = SG_SIX;     break; //6
      case 0xFF42BD:  smart_house::condition = SG_SEVEN;   break; //7
      case 0xFF4AB5:  smart_house::condition = SG_EIGHT;   break; //8
      case 0xFF52AD:  smart_house::condition = SG_NINE;    break; //9
      case 0xFF22DD:  smart_house::condition = SG_LEFT;    break; // <  <
      case 0xFF02FD:  smart_house::condition = SG_RIGHT;   break; // >  >
      case 0xFFC23D:  smart_house::condition = SG_PAUSE;   break; // >  >
      case 0xFF906F:  smart_house::condition = SG_EQ;      break; //EQ
    }
    delay(30);
    irrecv.resume();
  }
}

void smart_house::relayOne()
{
  if(smart_house::condition == SG_ONE)
  digitalWrite(relay_1, !digitalRead(relay_1));
  if(smart_house::condition == SG_RELAY_1_ON)
  digitalWrite(relay_1, HIGH);
  if(smart_house::condition == SG_RELAY_1_OFF)
  digitalWrite(relay_1, LOW);
  smart_house::condition=SG_WILLINGNESS;
}
void smart_house::relayTwo()
{
  if(smart_house::condition == SG_TWO)
  digitalWrite(relay_2, !digitalRead(relay_2));
  if(smart_house::condition == SG_RELAY_2_ON)
  digitalWrite(relay_2, HIGH);
  if(smart_house::condition == SG_RELAY_2_OFF)
  digitalWrite(relay_2, LOW);
  smart_house::condition=SG_WILLINGNESS;
}
void smart_house::relayThree()
{
  if(smart_house::condition == SG_THREE)
  digitalWrite(relay_3, !digitalRead(relay_3));
  if(smart_house::condition == SG_RELAY_3_ON)
  digitalWrite(relay_3, HIGH);
  if(smart_house::condition == SG_RELAY_3_OFF)
  digitalWrite(relay_3, LOW);
  smart_house::condition=SG_WILLINGNESS;
}

void smart_house::openAuto()
{
  open_auto=!open_auto;
  smart_house::condition=SG_WILLINGNESS;
}

void smart_house::moveMainMenuRight()
{
  smart_house::condition = SG_WILLINGNESS;
  switch(smart_house::condition_main_menu)
  {
    case SM_MENU_ALARM:         smart_house::condition_main_menu=SM_MENU_TOGGLE_SWITCH;  break;
    case SM_MENU_TOGGLE_SWITCH: smart_house::condition_main_menu=SM_MENU_TIME_G;         break;
    case SM_MENU_TIME_G:        smart_house::condition_main_menu=SM_MENU_TIMER;          break;
    case SM_MENU_TIMER:         smart_house::condition_main_menu=SM_MENU_ALARM;          break;
  }
  smart_house::displayText(1);
}
void smart_house::moveMainMenuLeft()
{
  smart_house::condition = SG_WILLINGNESS;
  switch(smart_house::condition_main_menu)
  {
    case SM_MENU_ALARM:         smart_house::condition_main_menu=SM_MENU_TIMER;          break;
    case SM_MENU_TOGGLE_SWITCH: smart_house::condition_main_menu=SM_MENU_ALARM;          break;
    case SM_MENU_TIME_G:        smart_house::condition_main_menu=SM_MENU_TOGGLE_SWITCH;  break;
    case SM_MENU_TIMER:         smart_house::condition_main_menu=SM_MENU_TIME_G;         break;
  }
  smart_house::displayText(1);
}

void smart_house::setSomeTime()
{
  smart_house::condition = SG_WILLINGNESS;
  smart_house::condition_index = SI_HOURS_DECADE;
  smart_house::displayHalfValue(1);
  while(smart_house::condition!=SG_PAUSE || smart_house::condition!=SG_EQ)
  {
    while(smart_house::condition == SG_WILLINGNESS || smart_house::condition == SG_SHOW_TIME_G)
    {
      smart_house::globalTime();
      if(smart_house::condition == SG_SHOW_TIME_G)
      smart_house::showGlobalTime(0);
      smart_house::enterRemote();
    }
    if(smart_house::condition == SG_EQ)
    {
      lcd.setCursor(0,0);
      lcd.print(" ");
      smart_house::displayText(1);
      break;
    }
    if(smart_house::condition == SG_PAUSE)
    {
      smart_house::confirmation();
      break;
    }
    if(smart_house::condition == SG_RIGHT || smart_house::condition == SG_LEFT)
      smart_house::relayIndex();
    if(smart_house::condition > =0 && condition < 10)
    {
      smart_house::setHalfTime();
      smart_house::relayIndexRight();
      smart_house::displayHalfValue(1);
    }
  }
}
void smart_house::confirmation()
{
  switch(smart_house::condition_main_menu)
  {
    case SM_MENU_TIME_G:
    hours_G = half_hours; minutes_G = half_minutes; seconds_G = half_seconds; break;
    case SM_MENU_ALARM: alarm_on = true;
    hours_A = half_hours; minutes_A = half_minutes; break;
    case SM_MENU_TOGGLE_SWITCH: time_switch_on = true;
    hours_S = half_hours; minutes_S = half_minutes; break;
    case SM_MENU_TIMER: user_timer_on = true;
    my_user_timer = half_seconds + half_minutes*60 + half_hours*3600; break;
  }
  lcd.setCursor(0,0);
  lcd.print(" ");
  smart_house::displayText(1);
  smart_house::condition = SG_WILLINGNESS;
}
void smart_house::relayIndex()
{
  switch(smart_house::condition)
  {
    case SG_RIGHT:  smart_house::relayIndexRight();   break;
    case SG_LEFT:   smart_house::relayIndexLeft();    break;
  }
  condition = SG_WILLINGNESS;
}
void smart_house::relayIndexRight()
{
  smart_house::condition = SG_WILLINGNESS;
  switch(smart_house::condition_index)
  {
    case SI_HOURS_DECADE:    smart_house::condition_index=SI_HOURS_UNIT;       break;
    case SI_HOURS_UNIT:      smart_house::condition_index=SI_MINUTES_DECADE;   break;
    case SI_MINUTES_DECADE:  smart_house::condition_index=SI_MINUTES_UNIT;     break;
    case SI_MINUTES_UNIT:    smart_house::condition_index=SI_SECONDS_DECADE;   break;
    case SI_SECONDS_DECADE:  smart_house::condition_index=SI_SECONDS_UNIT;     break;
    case SI_SECONDS_UNIT:    smart_house::condition_index=SI_HOURS_DECADE;     break;
  }
}
void smart_house::relayIndexLeft()
{
  smart_house::condition = SG_WILLINGNESS;
  switch(smart_house::condition_index)
  {
    case SI_HOURS_DECADE:    smart_house::condition_index=SI_SECONDS_UNIT;     break;
    case SI_HOURS_UNIT:      smart_house::condition_index=SI_HOURS_DECADE;     break;
    case SI_MINUTES_DECADE:  smart_house::condition_index=SI_HOURS_UNIT;       break;
    case SI_MINUTES_UNIT:    smart_house::condition_index=SI_MINUTES_DECADE;   break;
    case SI_SECONDS_DECADE:  smart_house::condition_index=SI_MINUTES_UNIT;     break;
    case SI_SECONDS_UNIT:    smart_house::condition_index=SI_SECONDS_DECADE;   break;
  }
}
void smart_house::setHalfTime()
{
  switch(smart_house::condition_index)
  {
    case SI_HOURS_DECADE:     if(condition == 2) half_hours = 20;
                              else half_hours = (condition%2)*10;    break;
    case SI_HOURS_UNIT:      half_hours += condition;          break;
    case SI_MINUTES_DECADE:  half_minutes = (condition%6)*10;  break;
    case SI_MINUTES_UNIT:    half_minutes += condition;        break;
    case SI_SECONDS_DECADE:  half_seconds = (condition%6)*10;  break;
    case SI_SECONDS_UNIT:    half_seconds += condition;        break;
  }
  smart_house::condition = SG_WILLINGNESS;
}

void smart_house::displayText(int row)
{
  lcd.setCursor(0, row);
  switch(smart_house::condition_main_menu)
  {
    case SM_MENU_ALARM:           lcd.print("      Alarme    "); break;
    case SM_MENU_TOGGLE_SWITCH:   lcd.print("      Tache     "); break;
    case SM_MENU_TIME_G:          lcd.print("      Temps     "); break;
    case SM_MENU_TIMER:           lcd.print("    Minuteur    "); break;
  }
}
void smart_house::displayHalfValue(int row)
{
  lcd.setCursor(0, 0);
  switch(smart_house::condition_main_menu)
  {
    case SM_MENU_ALARM:         lcd.print("A"); break;
    case SM_MENU_TOGGLE_SWITCH: lcd.print("S"); break;
    case SM_MENU_TIME_G:        lcd.print("T"); break;
  }
  if(half_hours > 9)
  {
    lcd.setCursor(4, row);
    lcd.print(half_hours);
  }
  else
  {
    lcd.setCursor(4, row);
    lcd.print("0");
    lcd.setCursor(5,row);
    lcd.print(half_hours);
  }
  lcd.setCursor(6, row);
  lcd.print(":");
  if(half_minutes > 9)
  {
    lcd.setCursor(7, row);
    lcd.print(half_minutes);
  }
  else
  {
    lcd.setCursor(7, row);
    lcd.print("0");
    lcd.setCursor(8,row);
    lcd.print(half_minutes);
  }
  lcd.setCursor(9, row);
  lcd.print(":");
  if(half_seconds > 9)
  {
    lcd.setCursor(10, row);
    lcd.print(half_seconds);
  }
  else
  {
    lcd.setCursor(10, row);
    lcd.print("0");
    lcd.setCursor(11,row);
    lcd.print(half_seconds);
  }
  smart_house::condition = SG_WILLINGNESS;
}

void smart_house::lightInput()
{
  light_level=analogRead(light_resistance);
  if(open_auto)
  {
    if(light_level > 600 && !sleep_on)
    {
      if(hours_G > =6 && hours_G < 18)
      smart_house::condition = SG_SEVEN;
    }
    if(light_level < 400 && sleep_on)
    {
      if(hours_G >= 18 || hours_G < 6)
      smart_house::condition = SG_SEVEN;
    }
  }
}

void smart_house::sleep_on()
{
  if(sleep_on)
  {
    Serial.println("ferme");
    for(int i = 0; i < 25; i++)
    {
      digitalWrite(servo, HIGH);
      delayMicroseconds(544);
      digitalWrite(servo, LOW);
      delayMicroseconds(456);
      delay(19);
    }
    sleep_on = false;
  }
  else
  {
    Serial.println("ouvre");
    for(int i = 0; i < 25; i++)
    {
      digitalWrite(servo, HIGH);
      delayMicroseconds(2400);
      digitalWrite(servo, LOW);
      delayMicroseconds(600);
      delay(17);
    }
    sleep_on = true;
  }
  smart_house::condition = SG_WILLINGNESS;
}

void smart_house::DHXZero()
{
  door_0_on=!digitalRead(HDX_0);
  if(door_0_on && digitalRead(!relay_1))
    smart_house::condition = SG_RELAY_1_ON;
}
void smart_house::DHXOne()
{
  door_1_on =! digitalRead(HDX_1);
  if(door_1_on && digitalRead(!relay_3))
    smart_house::condition=SG_RELAY_3_ON;
}

void smart_house::waterLevelSensor()
{
  water_level = analogRead(water_sensor);
  if(water_level < 400 && !no_water_indication)
  smart_house::condition=SG_LOW_WATER;
  if(water_level > 450 && no_water_indication)
  smart_house::condition = SG_LOW_WATER;
}
void smart_house::signalWaterLevel()
{
  if(!no_water_indication && water_level < 400)
  {
    lcd.setCursor(14, 0);
    lcd.print("LW");
    no_water_indication=true;
  }
  if(no_water_indication && water_level > 450)
  {
    lcd.setCursor(14, 0);
    lcd.print("  ");
    no_water_indication=false;
  }
}
void smart_house::humiditySoilSensor()
{
  humidity_soil_level=analogRead(humidity_sensor);
  if(humidity_soil_level <= 460 && !digitalRead(pump))
    smart_house::condition=SG_WATER_FLOWERS;
  if(humidity_soil_level >= 670 && digitalRead(pump))
    smart_house::condition = SG_WATER_FLOWERS;
}
void smart_house::waterFlowers()
{
  if(pump_timer == 0 && humidity_soil_level < =460)
  {
    pump_timer=3;
    digitalWrite(pump, HIGH);
  }
  if(pump_timer == 0 && humidity_soil_level > =670)
  digitalWrite(pump, LOW);
}
