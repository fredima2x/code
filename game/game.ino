#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

int characterPosition = 0;
int lastButtonState = HIGH;

struct Enemy {
    int position;
    int row;
    boolean active;
};
#define MAX_ENEMIES 3
Enemy enemies[MAX_ENEMIES];
int score = 0;
unsigned long lastEnemyTime = 0;
unsigned long gameOverTime = 0;
boolean gameOver = false;

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
    initEnemies();
}
void initEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
}
void displayGame() {
    lcd.clear();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            lcd.setCursor(enemies[i].position, enemies[i].row);
            lcd.print("o");
        }
    }
    lcd.setCursor(characterPosition, 0);
    lcd.print("*");
}
void spawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].position = 15;
            enemies[i].row = (random(2) == 0) ? 0 : 1;
            return;
        }
    }
}
void updateEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].position--;
            if (enemies[i].position == characterPosition && enemies[i].row == 0) {
                gameOver = true;
                gameOverTime = millis();
            }
            if (enemies[i].position < 0) {
                enemies[i].active = false;
                score++;
            }
        }
    }
}
void displayGameOver() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game Over!");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
}
void setup() {
    INIT_PINS();
    INIT_LCD();
    displayGame();
}
void loop() {
    if (gameOver) {
        displayGameOver();
        if (millis() - gameOverTime > 3000) {
            gameOver = false;
            score = 0;
            initEnemies();
            characterPosition = 0;
        }
        delay(100);
        return;
    }
    int buttonState = digitalRead(2);
    if (buttonState == LOW && lastButtonState == HIGH) {
        characterPosition = (characterPosition == 0) ? 1 : 0;
        delay(50);
    }
    lastButtonState = buttonState;
    if (millis() - lastEnemyTime > 800) {
        spawnEnemy();
        lastEnemyTime = millis();
    }
    updateEnemies();
    displayGame();
    delay(150);
}