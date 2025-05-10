#include <LiquidCrystal.h>
#include <Keypad.h>

const int ROW_NUM = 4;     //four rows
const int COLUMN_NUM = 4;  //four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pin_rows[ROW_NUM] = { 48, 46, 44, 42 };       //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = { 40, 38, 36, 34 };  //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// LCD pins <--> Arduino pins
const int RS = 11, EN = 12, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
bool score0,score1,score2,score3 = false;
int right = 0, up = 0;
int dir1 = 0, dir2 = 0;
byte customChar[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte customChar2[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b01110,
  0b01110,
  0b00000,
  0b00000,
  0b00000
};
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void checkScore()
{
  if (right == 5 && up == 1 && score0 == false)
  {
    score0 = !score0;
    Serial.println("You scored!");
  }
  else if (right == 9 && up == 1 && score1 == false)
  {
    score1 = !score1;
    Serial.println("You scored!");
  }
  else if (right == 6 && up == 0 && score2 == false)
  {
    score2 = !score2;
    Serial.println("You scored!");
  }
  else if (right == 15 && up == 1 && score3 == false)
  {
    score3 = !score3;
    Serial.println("You scored!");
  }
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);  // set up number of columns and rows

  lcd.createChar(1, customChar);  // create a new custom character

  lcd.setCursor(0, 0);  // move cursor to (2, 0)
  lcd.write((byte)1);   // print the custom char at (2, 0)

  lcd.createChar(2, customChar2);  // create a new custom character
  lcd.setCursor(5, 1);             // move cursor to (2, 0)
  lcd.write((byte)2);              // print the custom char at (2, 0)

  lcd.setCursor(9, 1);  // move cursor to (2, 0)
  lcd.write((byte)2);   // print the custom char at (2, 0)

  lcd.setCursor(6, 0);   // move cursor to (2, 0)
  lcd.write((byte)2);    // print the custom char at (2, 0)
  lcd.setCursor(15, 1);  // move cursor to (2, 0)
  lcd.write((byte)2);    // print the custom char at (2, 0)
}

void loop() {
  char key = keypad.getKey();
  if (key == '2') {
    lcd.setCursor(right, up);
    lcd.write(' ');
    if (up != 1) {
      up++;
    } else {
      up = 0;
    }
    lcd.setCursor(right, up);
    lcd.write((byte)1);
  } else if (key == '4') {
    lcd.setCursor(right, up);
    lcd.write(' ');
    if (right != 0) {
      right--;
    } else {
      right = 15;
    }
    lcd.setCursor(right, up);
    lcd.write((byte)1);
  } else if (key == '6') {
    lcd.setCursor(right, up);
    lcd.write(' ');
    if (right != 15) {
      right++;
    } else {
      right = 0;
    }
    lcd.setCursor(right, up);
    lcd.write((byte)1);
  } else if (key == '8') {
    lcd.setCursor(right, up);
    lcd.write(' ');
    if (up != 0) {
      up--;
    } else {
      up = 1;
    }
    lcd.setCursor(right, up);
    lcd.write((byte)1);
  } else if (key == 'A')
  {
    setup();
    up = right = 0;
    score0 = score1 = score2 = score3 = false;
  }
  checkScore();
}
