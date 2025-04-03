#ifndef SeaBattleGame_h
#define SeaBattleGame_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

class SeaBattleGame {
public:
    SeaBattleGame(uint8_t matrixPin, uint8_t rxPin, uint8_t txPin,
                uint8_t joy1X, uint8_t joy1Y, uint8_t joy1Btn,
                uint8_t joy2X, uint8_t joy2Y, uint8_t joy2Btn);
    
    void begin();
    void update();
    bool isGameOver() const;
    uint8_t getWinner() const; // 0 - нет победителя, 1 - игрок 1, 2 - игрок 2

private:
    // Параметры матрицы
    static const uint8_t MATRIX_ALL_WIDTH = 64;
    static const uint8_t MATRIX_WIDTH = 16;
    static const uint8_t MATRIX_HEIGHT = 16;
    static const uint16_t NUM_LEDS = MATRIX_ALL_WIDTH * MATRIX_HEIGHT;
    static const uint8_t BRIGHTNESS = 40;
    static const uint8_t SCREEN_FREQUENCY = 60;
    static const uint16_t SCREEN_PERIOD = 1000 / SCREEN_FREQUENCY;
    
    // Параметры джойстика
    static const uint16_t THRESHOLD = 512;
    static const uint16_t DEAD_ZONE = 200;
    static const uint16_t JOYSTICK_DELAY = 100;
    
    // Параметры игры
    static const uint8_t SEL_FIELDS = 5;
    static const uint16_t MUSIC_DELAY = 47000;
    static const uint8_t VOLUME = 10;
    
    // Объекты
    Adafruit_NeoPixel matrix;
    DFRobotDFPlayerMini mp3;
    SoftwareSerial *softwareSerialMP3;
    
    // Параметры джойстиков
    uint8_t joy1XPin, joy1YPin, joy1BtnPin;
    uint8_t joy2XPin, joy2YPin, joy2BtnPin;
    
    // Переменные состояния
    bool startGame;
    bool gameOver;
    uint8_t player;
    uint8_t loser;
    
    // Поля игроков
    int currentField1;
    int currentField2;
    bool fields1[SEL_FIELDS][MATRIX_WIDTH][MATRIX_HEIGHT];
    bool fields2[SEL_FIELDS][MATRIX_WIDTH][MATRIX_HEIGHT];
    
    // Курсоры
    int cursorX1, cursorY1;
    int cursorX2, cursorY2;
    
    // Попадания и разрушения
    bool redPixels1[MATRIX_WIDTH][MATRIX_HEIGHT];
    bool redPixels2[MATRIX_WIDTH][MATRIX_HEIGHT];
    bool destroyedShips1[MATRIX_WIDTH][MATRIX_HEIGHT];
    bool destroyedShips2[MATRIX_WIDTH][MATRIX_HEIGHT];
    bool surroundCells1[MATRIX_WIDTH][MATRIX_HEIGHT];
    bool surroundCells2[MATRIX_WIDTH][MATRIX_HEIGHT];
    
    // Таймеры
    unsigned long matrixDelay;
    unsigned long lastMusic;
    unsigned long lastPressLeft1, lastPressRight1, lastPressUp1, lastPressDown1, lastPressButton1;
    unsigned long lastPressLeft2, lastPressRight2, lastPressUp2, lastPressDown2, lastPressButton2;
    
    // Приватные методы
    void clearFields();
    void generateFields();
    void placeShip(bool field[MATRIX_WIDTH][MATRIX_HEIGHT], int length);
    bool canPlaceShip(bool field[MATRIX_WIDTH][MATRIX_HEIGHT], int x, int y, int length, bool horizontal);
    
    void displayField1();
    void displayField2();
    void displayCursor1();
    void displayCursor2();
    void displayHits1();
    void displayHits2();
    void displayGameOverMessage();
    
    bool shot1();
    bool shot2();
    bool updateShot(int x, int y, bool hits[MATRIX_WIDTH][MATRIX_HEIGHT], 
                   bool enemyField[MATRIX_WIDTH][MATRIX_HEIGHT], 
                   bool destroyedShips[MATRIX_WIDTH][MATRIX_HEIGHT], 
                   bool surroundCells[MATRIX_WIDTH][MATRIX_HEIGHT]);
    
    bool isShipDestroyed(bool field[MATRIX_WIDTH][MATRIX_HEIGHT], bool hits[MATRIX_WIDTH][MATRIX_HEIGHT], int startX, int startY);
    void addDestroyed(bool field[MATRIX_WIDTH][MATRIX_HEIGHT], int startX, int startY, bool destroyedShips[MATRIX_WIDTH][MATRIX_HEIGHT]);
    bool hasShipsLeft1();
    bool hasShipsLeft2();
    
    void markAroundShip(bool field[MATRIX_WIDTH][MATRIX_HEIGHT], bool surroundCells[MATRIX_WIDTH][MATRIX_HEIGHT], int startX, int startY);
    void markAroundHit(bool field[MATRIX_WIDTH][MATRIX_HEIGHT], bool surroundCells[MATRIX_WIDTH][MATRIX_HEIGHT], 
                      int x, int y, bool hits[MATRIX_WIDTH][MATRIX_HEIGHT]);
    bool isPartOfSameShip(bool field[MATRIX_WIDTH][MATRIX_HEIGHT], int startX, int startY, int x, int y);
    
    int getIndex1(int x, int y, bool secondMatrix = false);
    int getIndex2(int x, int y, bool secondMatrix = false);
    
    void playMusic();
};

#endif