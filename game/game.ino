#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

int characterPosition = 0;
int lastButtonState = HIGH;

int test = 42;

void INIT_PINS() {
    pinMode(13, OUTPUT);
    pinMode(2, INPUT_PULLUP);
}
void INIT_LCD() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting...");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);

}

void displayCharacter() {
    lcd.clear();
    lcd.setCursor(characterPosition, 0);
    lcd.print("*");
}

void setup() {
    INIT_PINS();
    INIT_LCD();
    displayCharacter();
    lcd.print(test);
}

void loop() {
    int buttonState = digitalRead(2);
    if (buttonState == LOW && lastButtonState == HIGH) {
        characterPosition = (characterPosition == 0) ? 1 : 0;
        displayCharacter();
        delay(50);
    }
    lastButtonState = buttonState;
    delay(10);
}