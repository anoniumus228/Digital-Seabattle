#include "SeaBattleGame.h"

SeaBattleGame::SeaBattleGame(uint8_t matrixPin, uint8_t rxPin, uint8_t txPin,
                           uint8_t joy1X, uint8_t joy1Y, uint8_t joy1Btn,
                           uint8_t joy2X, uint8_t joy2Y, uint8_t joy2Btn) :
    matrix(NUM_LEDS, matrixPin, NEO_GRB + NEO_KHZ800),
    joy1XPin(joy1X), joy1YPin(joy1Y), joy1BtnPin(joy1Btn),
    joy2XPin(joy2X), joy2YPin(joy2Y), joy2BtnPin(joy2Btn) {
    softwareSerialMP3 = new SoftwareSerial(rxPin, txPin);
}

void SeaBattleGame::begin() {
    randomSeed(analogRead(A7));
    player = random(2);
    
    softwareSerialMP3->begin(9600);
    if (!mp3.begin(*softwareSerialMP3, true, false)) {
        Serial.println("Ошибка: MP3 не активен");
    } else {
        Serial.println("Успех: MP3 активен");
    }
    
    matrix.begin();
    matrix.setBrightness(BRIGHTNESS);
    matrix.clear();
    
    pinMode(joy1XPin, INPUT);
    pinMode(joy1YPin, INPUT);
    pinMode(joy1BtnPin, INPUT_PULLUP);
    
    pinMode(joy2XPin, INPUT);
    pinMode(joy2YPin, INPUT);
    pinMode(joy2BtnPin, INPUT_PULLUP);
    
    clearFields();
    generateFields();
    
    mp3.volume(VOLUME);
    mp3.play(3);
    
    startGame = true;
    gameOver = false;
    lastMusic = millis();
}

void SeaBattleGame::update() {
    if (startGame) {
        if (millis() - lastMusic >= MUSIC_DELAY) {
            lastMusic = millis();
            mp3.play(3);
        }
        
        if (millis() - matrixDelay >= SCREEN_PERIOD) {
            matrixDelay = millis();
            matrix.clear();
            displayField1();
            displayField2();
            matrix.show();
        }
        
        int xValue1 = analogRead(joy1XPin);
        int xValue2 = analogRead(joy2XPin);
        
        if (xValue1 < THRESHOLD - DEAD_ZONE && (millis() - lastPressLeft1 > JOYSTICK_DELAY)) {
            lastPressLeft1 = millis();
            currentField1--;
            currentField1 = max(currentField1, 0);
        } else if (xValue1 > THRESHOLD + DEAD_ZONE && (millis() - lastPressRight1 > JOYSTICK_DELAY)) {
            lastPressRight1 = millis();
            currentField1++;
            currentField1 = min(currentField1, SEL_FIELDS - 1);
        }
        
        if (xValue2 < THRESHOLD - DEAD_ZONE && (millis() - lastPressLeft2 > JOYSTICK_DELAY)) {
            lastPressLeft2 = millis();
            currentField2--;
            currentField2 = max(currentField2, 0);
        } else if (xValue2 > THRESHOLD + DEAD_ZONE && (millis() - lastPressRight2 > JOYSTICK_DELAY)) {
            lastPressRight2 = millis();
            currentField2++;
            currentField2 = min(currentField2, SEL_FIELDS - 1);
        }
        
        if (digitalRead(joy1BtnPin) == LOW && millis() - lastPressButton1 > JOYSTICK_DELAY) {
            lastPressButton1 = millis();
            playerFlag1 = true;
        }
        if (digitalRead(joy2BtnPin) == LOW && millis() - lastPressButton2 > JOYSTICK_DELAY) {
            lastPressButton2 = millis();
            playerFlag2 = true;
        }
        
        if (playerFlag1 && playerFlag2) {
            startGame = false;
            delay(300);
            mp3.pause();
            
            cursorX1 = cursorY1 = cursorX2 = cursorY2 = 0;
            mp3.play(7);
            delay(500);
            playerFlag1 = playerFlag2 = false;
        }
    } else if (!gameOver) {
        if (millis() - matrixDelay >= SCREEN_PERIOD) {
            matrixDelay = millis();
            matrix.clear();
            displayField1();
            displayField2();
            displayHits1();
            displayHits2();
            
            if (player == 0) displayCursor1();
            else displayCursor2();
            
            matrix.show();
        }
        
        int xValue1 = analogRead(joy1XPin);
        int yValue1 = analogRead(joy1YPin);
        int xValue2 = analogRead(joy2XPin);
        int yValue2 = analogRead(joy2YPin);
        
        if (player == 0) {
            if (digitalRead(joy1BtnPin) == LOW && millis() - lastPressButton1 > JOYSTICK_DELAY) {
                lastPressButton1 = millis();
                if (!redPixels1[cursorX1][cursorY1]) {
                    redPixels1[cursorX1][cursorY1] = true;
                    if (shot1()) player = 0;
                    else player = 1;
                } else {
                    player = 0;
                }
            }
            
            if (millis() - delay1 > period1) {
                delay1 = millis();
                if (xValue1 < THRESHOLD - DEAD_ZONE && (millis() - lastPressLeft1 > JOYSTICK_DELAY)) {
                    cursorX1--;
                    cursorX1 = max(cursorX1, 0);
                } else if (xValue1 > THRESHOLD + DEAD_ZONE && (millis() - lastPressRight1 > JOYSTICK_DELAY)) {
                    cursorX1++;
                    cursorX1 = min(cursorX1, 15);
                } else if (yValue1 < THRESHOLD - DEAD_ZONE && (millis() - lastPressDown1 > JOYSTICK_DELAY)) {
                    cursorY1--;
                    cursorY1 = max(cursorY1, 0);
                } else if (yValue1 > THRESHOLD + DEAD_ZONE && (millis() - lastPressUp1 > JOYSTICK_DELAY)) {
                    cursorY1++;
                    cursorY1 = min(cursorY1, 15);
                }
            }
        } else {
            if (digitalRead(joy2BtnPin) == LOW && millis() - lastPressButton2 > JOYSTICK_DELAY) {
                lastPressButton2 = millis();
                if (!redPixels2[cursorX2][cursorY2]) {
                    redPixels2[cursorX2][cursorY2] = true;
                    if (shot2()) player = 1;
                    else player = 0;
                } else {
                    player = 1;
                }
            }
            
            if (millis() - delay1 > period1) {
                delay1 = millis();
                if (xValue2 < THRESHOLD - DEAD_ZONE && (millis() - lastPressLeft2 > JOYSTICK_DELAY)) {
                    cursorX2--;
                    cursorX2 = max(cursorX2, 0);
                } else if (xValue2 > THRESHOLD + DEAD_ZONE && (millis() - lastPressRight2 > JOYSTICK_DELAY)) {
                    cursorX2++;
                    cursorX2 = min(cursorX2, 15);
                } else if (yValue2 < THRESHOLD - DEAD_ZONE && (millis() - lastPressDown2 > JOYSTICK_DELAY)) {
                    cursorY2--;
                    cursorY2 = max(cursorY2, 0);
                } else if (yValue2 > THRESHOLD + DEAD_ZONE && (millis() - lastPressUp2 > JOYSTICK_DELAY)) {
                    cursorY2++;
                    cursorY2 = min(cursorY2, 15);
                }
            }
        }
    } else {
        displayGameOverMessage();
    }
}

// Остальные методы класса остаются практически такими же, как в исходном коде,
// но адаптируются под структуру класса (добавляется префикс SeaBattleGame::)

// Например:
void SeaBattleGame::clearFields() {
    for (int f = 0; f < SEL_FIELDS; f++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            for (int y = 0; y < MATRIX_HEIGHT; y++) {
                fields1[f][x][y] = false;
                fields2[f][x][y] = false;
            }
        }
    }
}

// ... и так далее для всех остальных методов