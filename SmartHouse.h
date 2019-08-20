#ifndef SMART_HOUSE_H

#define SMART_HOUSE_H

/*
 * Ports
 * A4(SDA) A5(SCL) - display
 */
#define recv               2  // IR
#define servo              3  // Servi
#define HDX_0              5  // HDX 0
#define HDX_1              6  // HDX 1
#define bell               9  // Shitty sound
#define relay_1            8  // Relay 1
#define relay_2            11 // Relay 2
#define relay_3            12 //Relay 3
#define pump               7  // Water pump
#define light_resistance   A0 // Photoresistor
#define water_sensor       A2 // Water level
#define humidity_sensor    A3 // Humodity level

enum StatesG {  // Alphabet 1 of automate G
  SG_ZERO,
  SG_ONE,
  SG_TWO,
  SG_THREE,
  SG_FOUR,
  SG_FIVE,
  SG_SIX,
  SG_SEVEN,
  SG_EIGHT,
  SG_NINE,
  SG_WILLINGNESS,
  SG_RIGHT, // >>
  SG_LEFT,  // <<
  SG_PAUSE,
  SG_EQ,
  SG_SHOW_TIME_G,
  SG_RELAY_1_ON,
  SG_RELAY_1_OFF,
  SG_RELAY_2_ON,
  SG_RELAY_2_OFF,
  SG_RELAY_3_ON,
  SG_RELAY_3_OFF,
  SG_WATER_FLOWERS,
  SG_LOW_WATER
};
enum StatesM {  // Alphabet 2 of automate M
  SM_MENU_ALARM,
  SM_MENU_TOGGLE_SWITCH,
  SM_MENU_TIME_G,
  SM_MENU_TIMER
};
enum StatesI {  // Alphabet 3 of automate I
  SI_HOURS_DECADE,
  SI_HOURS_UNIT,
  SI_MINUTES_DECADE,
  SI_MINUTES_UNIT,
  SI_SECONDS_DECADE,
  SI_SECONDS_UNIT
};

class smart_house
{
public:
  smart_house(StatesG _condition, StatesM _condition_main_menu, StatesI _condition_index);//Конструктор
  void primarySettings();
  void automateInput();
  void automateOutput();
private:
  StatesG condition;
  StatesM condition_main_menu;
  StatesI condition_index;
  //Inputs
  void globalTime();
  void enterRemote();
  void DHXOne();
  void DHXZero();
  void lightInput();
  void waterLevelSensor();
  void humiditySoilSensor();
  //Outputs
  void relayOne();
  void relayTwo();
  void relayThree();
  void openAuto();
  void sleep();
  void moveMainMenuRight();
  void moveMainMenuLeft();
  void setSomeTime();
  void signalWaterLevel();
  void waterFlowers();
  //All the rest
  void checkActs();
  void confirmation();
  void relayIndex();
  void relayIndexRight();
  void relayIndexLeft();
  void setHalfTime();
  void showGlobalTime(int row);
  void displayHalfValue(int row);
  void displayText(int row);
};

#endif
