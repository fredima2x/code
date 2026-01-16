#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Spieler-Eigenschaften
int characterPosition = 0;
int lastButtonState = HIGH;
int lives = 3;

// Gegner-Struktur
struct Enemy {
    int position;
    int row;
    boolean active;
    int animationFrame;
};

#define MAX_ENEMIES 4
Enemy enemies[MAX_ENEMIES];

// Spiel-Zustand
int score = 0;
int highScore = 0;
unsigned long lastEnemyTime = 0;
unsigned long gameOverTime = 0;
unsigned long lastAnimationTime = 0;
boolean gameOver = false;
boolean gameRunning = false;
int gameState = 0; // 0 = Menu, 1 = Spiel, 2 = Game Over

// Custom Characters für Animationen
byte playerChar[8] = {
    B00100,
    B01110,
    B10101,
    B00100,
    B00100,
    B00100,
    B00100,
    B00000
};

byte enemyChar1[8] = {
    B00000,
    B01110,
    B10101,
    B01110,
    B00100,
    B00000,
    B00000,
    B00000
};

byte enemyChar2[8] = {
    B00000,
    B01110,
    B10101,
    B01110,
    B00100,
    B01010,
    B00000,
    B00000
};

byte heartChar[8] = {
    B01010,
    B11111,
    B11111,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000
};

void INIT_PINS() {
    pinMode(13, OUTPUT);
    pinMode(2, INPUT_PULLUP);
}

void INIT_LCD() {
    lcd.init();
    lcd.backlight();
    
    // Custom characters definieren
    lcd.createChar(0, playerChar);
    lcd.createChar(1, enemyChar1);
    lcd.createChar(2, enemyChar2);
    lcd.createChar(3, heartChar);
    
    // Highscore aus EEPROM laden
    highScore = EEPROM.read(0) | (EEPROM.read(1) << 8);
    
    showMenu();
}

void showMenu() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DODGER");
    lcd.setCursor(0, 1);
    lcd.print("Press to Start!");
    gameState = 0;
}

void showStartScreen() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Level: ");
    lcd.print((score / 10) + 1);
    lcd.setCursor(0, 1);
    lcd.print("Get ready!");
    delay(1500);
}
void initEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
        enemies[i].animationFrame = 0;
    }
}

void displayHud() {
    // Score und Leben in HUD anzeigen
    lcd.setCursor(0, 1);
    lcd.print("S:");
    if (score < 10) lcd.print("0");
    lcd.print(score);
    lcd.print(" ");
    
    // Leben anzeigen
    for (int i = 0; i < lives; i++) {
        lcd.write(byte(3)); // Herz-Character
    }
}

void displayGame() {
    lcd.clear();
    
    // Spielfeld (Reihe 0)
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            int animFrame = enemies[i].animationFrame;
            lcd.setCursor(enemies[i].position, enemies[i].row);
            lcd.write(byte(animFrame % 2 + 1)); // Gegner-Animation
        }
    }
    
    // Spieler anzeigen
    lcd.setCursor(characterPosition, 0);
    lcd.write(byte(0)); // Spieler-Character
    
    // HUD
    displayHud();
}

void spawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].position = 15;
            enemies[i].row = (random(2) == 0) ? 0 : 1;
            enemies[i].animationFrame = 0;
            return;
        }
    }
}

void updateEnemies() {
    unsigned long currentTime = millis();
    
    // Animation Update
    if (currentTime - lastAnimationTime > 200) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                enemies[i].animationFrame++;
            }
        }
        lastAnimationTime = currentTime;
    }
    
    // Gegner-Bewegung
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].position--;
            
            // Kollisionsprüfung
            if (enemies[i].position == characterPosition && enemies[i].row == 0) {
                // Spiel-Schwierigkeit erhöhen (schneller Spawn)
                int difficulty = min((score / 10) * 50, 400);
                hitAnimation();
                lives--;
                
                if (lives <= 0) {
                    gameOver = true;
                    gameOverTime = millis();
                    saveHighScore();
                    gameState = 2;
                } else {
                    // Player blinkt
                    for (int j = 0; j < 3; j++) {
                        delay(100);
                    }
                }
                
                // Gegner entfernen
                enemies[i].active = false;
            }
            
            // Gegner ist vorbei - Punkt gewonnen
            if (enemies[i].position < 0) {
                enemies[i].active = false;
                score++;
            }
        }
    }
}

void hitAnimation() {
    // Blitz-Effekt
    for (int i = 0; i < 2; i++) {
        lcd.noBacklight();
        delay(100);
        lcd.backlight();
        delay(100);
    }
}

void saveHighScore() {
    if (score > highScore) {
        highScore = score;
        EEPROM.write(0, highScore & 0xFF);
        EEPROM.write(1, (highScore >> 8) & 0xFF);
    }
}

void displayGameOver() {
    static unsigned long animTime = 0;
    
    if (millis() - animTime > 300) {
        lcd.clear();
        if ((millis() / 400) % 2 == 0) {
            lcd.setCursor(2, 0);
            lcd.print("GAME OVER!");
        }
        animTime = millis();
    }
    
    lcd.setCursor(0, 1);
    lcd.print("S:");
    if (score < 10) lcd.print("0");
    lcd.print(score);
    lcd.print(" H:");
    if (highScore < 10) lcd.print("0");
    lcd.print(highScore);
}
void setup() {
    INIT_PINS();
    INIT_LCD();
    randomSeed(analogRead(0));
}

void loop() {
    int buttonState = digitalRead(2);
    
    // Menü-Zustand
    if (gameState == 0) {
        if (buttonState == LOW && lastButtonState == HIGH) {
            gameState = 1;
            gameRunning = true;
            score = 0;
            lives = 3;
            characterPosition = 0;
            initEnemies();
            lastEnemyTime = millis();
            showStartScreen();
            delay(50);
        }
        lastButtonState = buttonState;
        delay(100);
        return;
    }
    
    // Game Over-Zustand
    if (gameState == 2) {
        displayGameOver();
        
        if (millis() - gameOverTime > 4000) {
            gameState = 0;
            showMenu();
        }
        
        if (buttonState == LOW && lastButtonState == HIGH) {
            gameState = 0;
            showMenu();
        }
        lastButtonState = buttonState;
        delay(100);
        return;
    }
    
    // Spiel läuft
    if (gameRunning) {
        // Button-Eingabe
        if (buttonState == LOW && lastButtonState == HIGH) {
            characterPosition = (characterPosition == 0) ? 1 : 0;
            delay(50);
        }
        lastButtonState = buttonState;
        
        // Gegner spawnen (Schwierigkeit erhöht sich)
        int spawnDelay = max(400, 800 - (score / 5) * 50);
        if (millis() - lastEnemyTime > spawnDelay) {
            spawnEnemy();
            lastEnemyTime = millis();
        }
        
        // Gegner bewegen und prüfen
        updateEnemies();
        
        // Spiel anzeigen
        displayGame();
        
        // Spiel-Geschwindigkeit erhöhen mit Score
        int gameSpeed = max(50, 150 - (score / 10) * 20);
        delay(gameSpeed);
    }
}