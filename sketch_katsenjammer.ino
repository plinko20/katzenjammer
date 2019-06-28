/*----(ImportLibrary)----*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
//i2c pins
LiquidCrystal_I2C lcd(0x27,16,2); //sets display to 16 char and 2 lines

//Constants and buttons
//The following set the pin numbers for the different components
//DO NOT USE A4 and A5! These are reserved for the display
const int buttonUp = 2;
const int buttonDown = 3;
const int buttonEnter = 4;
const int buttonBack = 5;
const int pump1 = 6;
const int pump2 = 7;
const int pump3 = 8;
const int pump4 = 9;
const int pump5 = 10;
const int pump6 = 11;
const int pump7 = 12;
const int pump8 = 13;

//variables that will change

//Button States - Records whether the a button is press or not
int buttonStateUp = 0;
int buttonStateDown = 0;
int buttonStateEnter = 0;
int buttonStateBack = 0;
int lstButtonStateUp = 0;
int lstButtonStateDown = 0;
int lstButtonStateEnter = 0;
int lstButtonStateBack = 0;

/*Action - Fucntion changes based on menu. This shows the action
 *to take if a button is pushed. 0 means do nothing.
 */
int action = 0;

/*Menu State - This will tell the program what mode the Katzenjammer
 *is in. 
 *The first element in the array is for the main menu state
 *0 = Default. Shows a welcome message - navigating here will let you choose 
 *  maintenance or party mode.
 *1 = Party - This will be the mode that will show the drink menu
 *2 = Cleaning - this will give options for cleaning,
 *3 = Inventory - This will let you change the recorded number of oz you have left in your bag
 *
 *The second element is for the sub menu - this will change based on the main
 *menu state.
 *
 *The third element is for the options within each item in the sub menu (if used).
 */
int menuState[3] = {0,1,0}; 
int lstMenuState[3] = {0,1,0}; //used for the back button
int lstMenuState1[3] = {0,0,0}; //temporary storage so menuState & lstMenuState can be used before being changed

/*Insert menu options here
 * Array 0 = main menu
 * Array 1 = Drink Menu
 * Array 2 = Cleaning Menu
 * Array 3 = Inventory Menu
 * Array 5 = Menu Titles
 */
char *menus [][9] = {
  {"Lock Screen", "PARTY!", "Cleaning","Inventory"},
  {"0-Lock Screen", "1-Whisky & Coke", "2-Rum & Coke", "3-Whisky Shot", "4-Rum Shot", "5-Coke"},
  {"Lock Screen", "Pump 1", "Pump 2", "Pump 3", "Pump 4", "Pump 5", "Pump 6", "Pump 7", "Pump 8"},
  {"Lock Screen", "Pump 1", "Pump 2", "Pump 3", "Pump 4", "Pump 5", "Pump 6", "Pump 7", "Pump 8"},
  {"Main Menu","PARTY!", "Cleaning","Inventory"}
};

//recipies for coctails. after first item in array is the number of miliseconds the pump needs to run
long recipie[][8] = {
  {3000, 5000, 10000, 0, 0, 0, 0, 0},
  {1000, 60000, 2000, 0, 0, 0, 0, 0},
  {10000, 0, 2000, 5000, 0, 0, 0, 0},
  {0, 5000, 1000, 9000, 0, 0, 0, 0},
  {1000, 0, 7000, 0, 0, 0, 0, 0}
};

/*
 * list of pumps for the Party and cleanPump functions
 * first spot intentionally left blank as spot 0 is always reserved for Lock Screen
 */

int pump[9] = {0,pump1,pump2,pump3,pump4,pump5,pump6,pump7,pump8};

/*
 * menuitems keeps track of how many options are in each menu so that you
 * don't increment past the last option. This can also be used to hide options from the public.
 * Remember to increase these numbers as you add new features!
 * spot 0 = main menu
 * spot 1 = Drink Menu
 * spot 2 = Cleaning menu
 * spot 3 = Inventory Menu
 */
int menuItems[4] = {3, 5, 4, 4};

/*
 * Inventory (inv) keeps track of the number of ml. left in each pouch so that you don't run out.
 * lim is the maximum amount of liquid that can be carried in each bag
 */
int inv[9];
int lim[9] = {0, 3000, 3000, 3000, 3000, 0, 0, 0, 0};

int pmax = 6;
int pmin = 3;
int p = pmax; // used for choosing the current spot the cursor will be placed in
char volc[4]; // Creates an array used to display the current volume
char cnum;
  
//lockScreen - can be exited by hitting a button
int lockScreen = 2;

void setup() {
  //Pin Setup
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(pump3, OUTPUT);
  pinMode(pump4, OUTPUT);
  pinMode(pump5, OUTPUT);
  pinMode(pump6, OUTPUT);
  pinMode(pump7, OUTPUT);
  pinMode(pump8, OUTPUT);
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonEnter, INPUT);
  pinMode(buttonBack, INPUT);
  
  //LCD Setup
  lcd.init(); //initialize the lcd
  lcd.backlight(); // Powers the backlight

  for (int i = 0; i < 8; i++){
    inv[i+1] = EEPROM.read(i);
  }

}

void loop() {
  //check to see if Lockscreen is on, if yes, display welcome message.
  //If not, then show current menu
  buttonCheck();
  if (lockScreen == 2){
    lcd.noBacklight();
    lcd.setCursor(0,0);
    lcd.print ("Katzenjammer");
    lcd.setCursor(0,1);
    lcd.print ("Drinks on the go"); 
    //buttonCheck();
    if (action != 0 ){
      lockScreen = 1;
    }
  }
  else if (lockScreen == 1) {
    lcd.backlight();
    if (action != 0 ){
      lockScreen = 0;
      lcd.clear();
    }
  }
  else if (lockScreen == 0){
    lcd.setCursor(0,0);
    lcd.print(menus[4][menuState[0]]);
    lcd.setCursor(0,1);
    lcd.print(menus[menuState[0]][menuState[1]]);
    takeAction();    
  }
}

void buttonCheck(){
  //Read the state of the pushbutton values
  buttonStateUp = digitalRead(buttonUp);
  buttonStateDown = digitalRead(buttonDown);
  buttonStateEnter = digitalRead(buttonEnter);
  buttonStateBack = digitalRead(buttonBack);

  //check if button is pressed, if it is, return appropriate value
  if (buttonStateUp != lstButtonStateUp){
    if (buttonStateUp == HIGH) {
      action = 1;
    }
  }
  
  else if (buttonStateDown != lstButtonStateDown){
    if (buttonStateDown == HIGH) {
      action = 2;
    }
  }

  else if (buttonStateBack != lstButtonStateBack){
    if (buttonStateBack == HIGH) {
      action = 3;
    }
  }
  
  else if (buttonStateEnter != lstButtonStateEnter){
    if (buttonStateEnter == HIGH) {
      action = 4;
    }
  }
  
  else {action = 0;}
  lstButtonStateUp = buttonStateUp;
  lstButtonStateDown = buttonStateDown;
  lstButtonStateEnter = buttonStateEnter;
  lstButtonStateBack = buttonStateBack;
}

void takeAction (){
  //first action is to increment the menu
  if (action == 1) {
    if (menuState[1] == menuItems[menuState[0]]){
        menuState[1] = 0;
    }
    else {
      menuState[1]++;
    }
    lcd.clear();
  }
  //Second action is to decrement the menu
  else if (action == 2){
    if (menuState[1] == 0) {
      menuState[1] = menuItems[menuState[0]];
    }
    else {
      menuState[1]--;
    }
    lcd.clear();
   }
   //Third action is to go back to the last screen
   else if (action == 3) {
    if (menuState[0] != 0){
      for (int i = 0; i < 3; i++){
        lstMenuState1[i] = menuState[i];
      } 
      menuState [0] = 0;
      menuState [1] = lstMenuState1[0];
//      for (int i = 0; i < 3; i++){
//        menuState[i] = lstMenuState[i];
//      } 
//      for (int i = 0; i < 3; i++){
//        lstMenuState[i] = lstMenuState1[i];
//      }
    }
    lcd.clear();
   }
   /* Fourth action is to execute whatever you are on.
    *  If you are on a menu of menus, you should go to the menu selected.
    *  If you are on a menu of actions, run the program that is selected.
    */
   else if (action == 4) {
      if (menuState[1] == 0) {
        lockScreen = 2;
      }
      else {
        if (menuState[0] == 0) {
          for (int i = 0; i < 3; i++){
            lstMenuState[i] = menuState[i];
          }
          menuState[0] = menuState[1];
          menuState[1] = 1;
        }
        else if (menuState[0] == 1){
          pourDrink();
        }
        else if (menuState[0] == 2){
            cleanPump();
        }
        else if (menuState[0] == 3){
          for (int i = 0; i < 3; i++){
            lstMenuState[i] = menuState[i];
          }
            manageInventory();
        }
      }
      lcd.clear();
   }
}


void manageInventory(){
  menuState[0]++;
  String target = menus[3][menuState[1]]; //Take the required pump and turn into a string
  String invmessage = target + " has"; //Append " has" to the pump name
  bool edit = false; // used to track editing mode. If 1, edit mode on, if 0 off.
  int tempVolC;  
  sprintf(volc, "%04d", inv[menuState[1]]); // break the current volume into separate letters and put those letters into volc
  lcd.clear();
  int stime = millis();
  cnum = volc[p-3];
  do {
    //print top line.
    lcd.setCursor(2,0);
    lcd.print(invmessage);
    //print volume into the second line
    for (int i=0; i<4; i++){
      lcd.setCursor (i+3,1);
      lcd.print(volc[i]);
    }
    lcd.setCursor(7,1);
    lcd.print(" ml.");
    lcd.setCursor(p,1);
    
    /*
     * Set the current cursor position for adjusting the inventory
     * Blink the number on and off to indicate which number is to be changed
     */
     
    stime = cursorLocation(stime);
    buttonCheck();
    
    if (action == 3){
      tempVolC = atoi(volc);
      if (tempVolC <= lim[menuState[1]]) {
        inv[menuState[1]] = tempVolC;
        menuState[0] = 3;
        lcd.clear();
        for (int i = 0; i < 8; i++){
          EEPROM.update(i, inv[i+1]);
        }
        action = 5;
      }
      else {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ERROR: OVER CAP");
        lcd.setCursor(0,1);
        lcd.print("PRESS ANY BUTTON");
        do {
          buttonCheck();
        } while (action == 0);
        lcd.clear();
      }
    }
    else if (action == 1){
      lcd.print(volc[p-3]);
      if (p == pmax) {
        p = pmin;
      }
      else {
        p++;
      }
    }
    else if (action == 2){
      lcd.print(volc[p-3]);
      if (p == pmin) {
        p = pmax;
      }
      else {
        p--;
      }
    }
    else if (action == 4){
      lcd.setCursor(0,1);
      lcd.print("e");
      volc[p-3] = newval();
    }
  } while (action != 5);
}
  
char newval(){
  int stime = millis();
  cnum = volc[p-3];
  do{
    stime = cursorLocation(stime);
    int num = cnum - '0';
    buttonCheck();
    if (action == 1){
      if (num == 9) {
        num = 0;
      }
      else {
        num ++;
      }
      cnum = num + '0';
      lcd.setCursor(p,1);
      lcd.print(cnum);
    }
    else if (action == 2) {
      if (num == 0) {
        num = 9;
      }
      else{
        num --;
      }
      cnum = num +'0';
      lcd.setCursor(p,1);
      lcd.print(cnum);
    }
    else if (action == 4) {
      lcd.setCursor(0,1);
      lcd.print(" ");
    }
  }while (action != 4);
  return cnum;
}

int cursorLocation(int stime){
    int curTime = millis();
    lcd.setCursor(p,1);
    if (curTime > stime + 1000){
      lcd.print(cnum);
      stime = millis();
    }
    else if (curTime > stime + 700) {
      lcd.print(" ");
    }
    return stime;
}

void cleanPump(){
  lcd.clear();
  lcd.setCursor(0,0);
  String clean1 = "Cleaning Pump ";
  String cleanTop = clean1 + menuState[1];
  lcd.print(cleanTop);
  lcd.setCursor(0,1);
  lcd.print("Press E to stop");
  do {
    buttonCheck();
    digitalWrite(pump[menuState[1]], HIGH);
  } while (action != 4);
  digitalWrite(pump[menuState[1]], LOW);
  lcd.clear();
}

void pourDrink(){
  int i;
  long stime[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  long curtime;
  int count = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pouring Drink:");
  lcd.setCursor(0,1);
  lcd.print(menus[1][menuState[1]]);
  for (i = 0; i < 8; i++){
    if(recipie[menuState[1]-1][i] > 0){
      stime[i] = millis();
      digitalWrite(pump[i+1],HIGH);
    }
  }
  do {
    count = 0;
    for (i = 0; i < 8; i++){
      curtime = millis();
      if (curtime - stime[i] > recipie[menuState[1]-1][i]) {
        digitalWrite(pump[i+1],LOW);
        count ++;
      }
    }
  }while(count < 8);
  action = 0;
  EEPROM.update(i,inv[i+1]);
}
