#include <SeaBattleGame.h>

// Пины подключения
#define MATRIX_PIN 6
#define RX_PIN 10
#define TX_PIN 11

// Параметры джойстика 1
#define JOY1_X A0
#define JOY1_Y A1
#define JOY1_BTN 2

// Параметры джойстика 2
#define JOY2_X A2
#define JOY2_Y A3
#define JOY2_BTN 3

SeaBattleGame game(MATRIX_PIN, RX_PIN, TX_PIN, 
                  JOY1_X, JOY1_Y, JOY1_BTN,
                  JOY2_X, JOY2_Y, JOY2_BTN);

void setup() {
    Serial.begin(9600);
    game.begin();
}

void loop() {
    game.update();
    
    if (game.isGameOver()) {
        Serial.print("Game over! Winner: Player ");
        Serial.println(game.getWinner());
        delay(1000);
    }
}