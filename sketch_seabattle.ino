#include <Adafruit_NeoPixel.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

//#include <Servo.h>

// Параметры матрицы
#define MATRIX_PIN 6                              // Пин для матрицы
#define MATRIX_ALL_WIDTH 64                       // Общая ширина (четыре матрицы 16x16)
#define MATRIX_WIDTH 16                           // Ширина одной матрицы
#define MATRIX_HEIGHT 16                          // Высота
#define NUM_LEDS MATRIX_ALL_WIDTH * MATRIX_HEIGHT  // Общее количество светодиодов
#define BRIGHTNESS 40                             // Яркость матрицы
#define SCREEN_FREQUENCY 60                       // Частота обновления экрана

#define SCREEN_PERIOD 1000 / SCREEN_FREQUENCY
#define period1 60

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(NUM_LEDS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);  // Создание объекта класса Adafruit_NeoPixel и передача в него информации о матрице

unsigned long matrixDelay = 0;
unsigned long delay1 = 0;
// Параметры джойстика
#define VRx_PIN A0    // Ось x  )
#define VRy_PIN A1    // Ось y   } 1 джойстик
#define BUTTON_PIN 2  // Кнопка )

#define VRx_PIN2 A2    // Ось x  )
#define VRy_PIN2 A3    // Ось y   } 2 джойстик
#define BUTTON_PIN2 3  //Кнопка  )

#define THRESHOLD 512  // Порог для центра джойстика
#define DEAD_ZONE 200  // Зона мертвой зоны вокруг центра

#define JOYSTICK_DELAY 100  // Задержка между нажатиями джойстика

unsigned long lastPressLeft1 = 0;
unsigned long lastPressRight1 = 0;
unsigned long lastPressUp1 = 0;
unsigned long lastPressDown1 = 0;
unsigned long lastPressButton1 = 0;

unsigned long lastPressLeft2 = 0;
unsigned long lastPressRight2 = 0;
unsigned long lastPressUp2 = 0;
unsigned long lastPressDown2 = 0;
unsigned long lastPressButton2 = 0;

int tmp = 1;

//Параметры mp3
#define RX 10               //  }Пины подключения mp3
#define TX 11               // )
#define VOLUME 10           // Громкость
#define MUSIC_DELAY 47000  // Время в миллисикундах, через которое музыка будет играть заново

unsigned long lastMusic = 0;  // Переменная для millis в музыке

DFRobotDFPlayerMini mp3;
SoftwareSerial *softwareSerialMP3;


//Параметры игры
#define SEL_FIELDS 5  // Количество полей для выбора

int currentField1 = 0;
int currentField2 = 0;

int player;

int loser = 0;

bool playerFlag1 = false;
bool playerFlag2 = false;

int cursorX1 = 0;
int cursorY1 = 0;
int cursorX2 = 0;
int cursorY2 = 0;

bool redPixels1[16][16];  // Массив для хранения позиций красных пикселей
bool redPixels2[16][16];

bool destroyedShips1[16][16];
bool destroyedShips2[16][16];

bool startGame = true;
bool gameOver = false;

bool fields1[SEL_FIELDS][MATRIX_WIDTH][MATRIX_HEIGHT];
bool fields2[SEL_FIELDS][MATRIX_WIDTH][MATRIX_HEIGHT];

bool surroundCells1[16][16] = {false};  // Для первого игрока
bool surroundCells2[16][16] = {false};  // Для второго игрока

int getIndex1(int x, int y, bool secondMatrix = false) {
  int base = secondMatrix ? 256 : 0;  // Смещение индекса для второй половины матрицы
  if (y % 2 == 0) {
    return y * 16 + x + base;  // Четная строка, слева направо
  } else {
    return y * 16 + (15 - x) + base;  // Нечетная строка, справа налево
  }
}
int getIndex2(int x, int y, bool secondMatrix = false) {
  int base = secondMatrix ? 768 : 512;  // Смещение индекса для второй половины матрицы
  if (y % 2 == 0) {
    return y * 16 + x + base;  // Четная строка, слева направо
  } else {
    return y * 16 + (15 - x) + base;  // Нечетная строка, справа налево
  }
}

void setup() {
  Serial.begin(9600);  // Serial port для выявления ошибок

  randomSeed(analogRead(A7));  // Использование помех с аналогового порта для более случайных чисел
  player = random(2);

  softwareSerialMP3 = new SoftwareSerial(RX, TX);
  softwareSerialMP3->begin(9600);
  if (!mp3.begin(*softwareSerialMP3, true, false)) {
    Serial.println("Ошибка: MP3 не активен");
  } else {
    Serial.println("Успех: MP3 активен");
  }

  matrix.begin();                    // Инициализация работы матрицы
  matrix.setBrightness(BRIGHTNESS);  // Установка яркости матрицы
  matrix.clear();                    // Очистка дисплея

  pinMode(VRx_PIN, INPUT);
  pinMode(VRy_PIN, INPUT);  // Подключение джойстика 1
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(VRx_PIN2, INPUT);
  pinMode(VRy_PIN2, INPUT);  // Подключение джойстика 2
  pinMode(BUTTON_PIN2, INPUT_PULLUP);

  clearFields();     // Очистка массивов с полями
  generateFields();  // Генерация полей
  displayField1();
  displayField2();

  mp3.volume(VOLUME);  // Установка громости музыки
  mp3.play(3);
}

void loop() {
  if (startGame) {
            // Serial.println(mp3.readType());
    if (millis() - lastMusic >= MUSIC_DELAY) {
      lastMusic = millis();
      mp3.play(3);
    }
    // playMusic(); // Проигрывание музыки

    // Перерисовка экрана частотой 60 раз в секунду (Частоту можно настроить в SCREEN_FREQUENCY)
    if (millis() - matrixDelay >= SCREEN_PERIOD) {
      matrixDelay = millis();
      matrix.clear();
      displayField1();
      displayField2();
      matrix.show();
    }

    // Считывания показаний с джойстика
    int xValue1 = analogRead(VRx_PIN);
    int xValue2 = analogRead(VRx_PIN2);

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

    if (digitalRead(BUTTON_PIN) == LOW) {
      playerFlag1 = true;
    }
    if (digitalRead(BUTTON_PIN2) == LOW) {
      playerFlag2 = true;
    }

    // Фиксация поля и активация курсора при нажатии обоих кнопок
    if (playerFlag1 && playerFlag2) {
      startGame = false;
      delay(300);
      mp3.pause();

      cursorX1 = 0;
      cursorY1 = 0;
      cursorX2 = 0;
      cursorY2 = 0;
      mp3.pause();
      mp3.play(7);
      delay(500);
    }
  } else if (!startGame && !gameOver) {
    // Перерисовка экрана частотой 60 раз в секунду (Частоту можно настроить в SCREEN_FREQUENCY)
    if (millis() - matrixDelay >= SCREEN_PERIOD) {
      matrixDelay = millis();
      matrix.clear();
      displayField1();
      displayField2();
      displayHits1();
      displayHits2();

      if (player == 0) {
        displayCursor1();
      } else {
        displayCursor2();
      }
      matrix.show();
    }

    int xValue1 = analogRead(VRx_PIN);
    int yValue1 = analogRead(VRy_PIN);

    int xValue2 = analogRead(VRx_PIN2);
    int yValue2 = analogRead(VRy_PIN2);

    if (player == 0) {
      if (digitalRead(BUTTON_PIN) == LOW && millis() - lastPressButton1 > JOYSTICK_DELAY) {
        lastPressButton1 = millis();
        // mp3.play(1);
        // delay(500);
        if (redPixels1[cursorX1][cursorY1] == 0) {
        redPixels1[cursorX1][cursorY1] = true;  // Сохраняем позицию красного пикселя
        if (shot1() == true) {
          player = 0;
        }
        else {
          player = 1;
        }
        }
        else {
          player = 0;
        }
      }
      if (millis() - delay1 > period1) {
      delay1 = millis();
      if (xValue1 < THRESHOLD - DEAD_ZONE && (millis() - lastPressLeft1 > JOYSTICK_DELAY)) {
        cursorX1--;
        cursorX1 = max(cursorX1, 0);  // Влево
      } else if (xValue1 > THRESHOLD + DEAD_ZONE && (millis() - lastPressRight1 > JOYSTICK_DELAY)) {
        cursorX1++;
        cursorX1 = min(cursorX1, 15);  // Вправо
      } else if (yValue1 < THRESHOLD - DEAD_ZONE && (millis() - lastPressDown1 > JOYSTICK_DELAY)) {
        cursorY1--;
        cursorY1 = max(cursorY1, 0);  // Вниз
      } else if (yValue1 > THRESHOLD + DEAD_ZONE && (millis() - lastPressUp1 > JOYSTICK_DELAY)) {
        cursorY1++;
        cursorY1 = min(cursorY1, 15);  // Вверх
      }
      }
    }

    if (player == 1) {
      if (digitalRead(BUTTON_PIN2) == LOW && millis() - lastPressButton2 > JOYSTICK_DELAY) {
        lastPressButton2 = millis();
        // mp3.play(1);
        // delay(500);
        if (redPixels2[cursorX2][cursorY2] == 0) {
        redPixels2[cursorX2][cursorY2] = true;
        if (shot2() == true) {
          player = 1;
        }
        else {
          player = 0;
        }
        }
        else {
          player = 1;
        }
      }
      if (millis() - delay1 > period1) {
      delay1 = millis();
      if (xValue2 < THRESHOLD - DEAD_ZONE && (millis() - lastPressLeft2 > JOYSTICK_DELAY)) {
        cursorX2--;
        cursorX2 = max(cursorX2, 0);  // Влево
      } else if (xValue2 > THRESHOLD + DEAD_ZONE && (millis() - lastPressRight2 > JOYSTICK_DELAY)) {
        cursorX2++;
        cursorX2 = min(cursorX2, 15);  // Вправо
      } else if (yValue2 < THRESHOLD - DEAD_ZONE && (millis() - lastPressDown2 > JOYSTICK_DELAY)) {
        cursorY2--;
        cursorY2 = max(cursorY2, 0);  // Вниз
      } else if (yValue2 > THRESHOLD + DEAD_ZONE && (millis() - lastPressUp2 > JOYSTICK_DELAY)) {
        cursorY2++;
        cursorY2 = min(cursorY2, 15);  // Вверх
      }
      }
    }
  } else {
    displayGameOverMessage();
    if (tmp == 1) {
      mp3.play(8);
      tmp = 0;
    }
  }
  //  Serial.print(gameOver);
}

void markAroundShip(bool field[16][16], bool surroundCells[16][16], int startX, int startY) {
    bool visited[16][16] = {false};
    struct Point { int x, y; };
    Point stack[256];
    int stackSize = 0;

    stack[stackSize++] = {startX, startY};

    while (stackSize > 0) {
        Point p = stack[--stackSize];
        int x = p.x, y = p.y;

        if (x < 0 || x >= 16 || y < 0 || y >= 16 || visited[x][y] || !field[x][y]) {
            continue;
        }

        visited[x][y] = true;

        // Помечаем окружающие клетки
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < 16 && ny >= 0 && ny < 16 && !field[nx][ny]) {
                    surroundCells[nx][ny] = true;
                }
            }
        }

        // Добавляем соседние клетки корабля в стек
        for (int i = 0; i < 4; i++) {
            int nx = x + (i == 0) - (i == 1);
            int ny = y + (i == 2) - (i == 3);
            stack[stackSize++] = {nx, ny};
        }
    }
}


void markAroundHit(bool field[16][16], bool surroundCells[16][16], int x, int y, bool hits[16][16]) {
    // Проверяем направление корабля
    bool isHorizontal = false;
    bool isVertical = false;
    
    // Ищем первое попадание в этот корабль
    int firstHitX = x;
    int firstHitY = y;
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            if(field[i][j] && hits[i][j] && isPartOfSameShip(field, x, y, i, j)) {
                if(hits[i][j] && (i < firstHitX || j < firstHitY)) {
                    firstHitX = i;
                    firstHitY = j;
                }
            }
        }
    }

    // Если это не первое попадание в корабль
    if(x != firstHitX || y != firstHitY) {
        // Определяем направление корабля
        if(x != firstHitX) {
            isHorizontal = true;
        } else if(y != firstHitY) {
            isVertical = true;
        }
        
        if(isHorizontal) {
            // Отмечаем клетки сверху и снизу
            int minX = min(x, firstHitX);
            int maxX = max(x, firstHitX);
            for(int i = minX; i <= maxX; i++) {
                if(y > 0) surroundCells[i][y-1] = true;
                if(y < 15) surroundCells[i][y+1] = true;
            }
        } else if(isVertical) {
            // Отмечаем клетки слева и справа
            int minY = min(y, firstHitY);
            int maxY = max(y, firstHitY);
            for(int j = minY; j <= maxY; j++) {
                if(x > 0) surroundCells[x-1][j] = true;
                if(x < 15) surroundCells[x+1][j] = true;
            }
        }
    } else {
        // Для первого попадания отмечаем только диагонали
        if(x > 0 && y > 0) surroundCells[x-1][y-1] = true;
        if(x > 0 && y < 15) surroundCells[x-1][y+1] = true;
        if(x < 15 && y > 0) surroundCells[x+1][y-1] = true;
        if(x < 15 && y < 15) surroundCells[x+1][y+1] = true;
    }
}

// Вспомогательная функция для проверки, является ли клетка частью того же корабля
bool isPartOfSameShip(bool field[16][16], int startX, int startY, int x, int y) {
    if (!field[startX][startY] || !field[x][y]) return false;
    
    // Если клетки на одной горизонтали
    if (startY == y) {
        int minX = min(startX, x);
        int maxX = max(startX, x);
        // Проверяем, есть ли между ними пустые клетки
        for (int i = minX; i <= maxX; i++) {
            if (!field[i][y]) return false;
        }
        return true;
    }
    
    // Если клетки на одной вертикали
    if (startX == x) {
        int minY = min(startY, y);
        int maxY = max(startY, y);
        // Проверяем, есть ли между ними пустые клетки
        for (int i = minY; i <= maxY; i++) {
            if (!field[x][i]) return false;
        }
        return true;
    }
    
    return false;
}

// void findShipCells(bool field[16][16], bool visited[16][16], int x, int y, int shipCells[16][2], int &numCells) {
//     if(x < 0 || x >= 16 || y < 0 || y >= 16 || visited[x][y] || !field[x][y]) {
//         return;
//     }
    
//     visited[x][y] = true;
//     shipCells[numCells][0] = x;
//     shipCells[numCells][1] = y;
//     numCells++;
    
//     // Проверяем соседние клетки
//     findShipCells(field, visited, x+1, y, shipCells, numCells);
//     findShipCells(field, visited, x-1, y, shipCells, numCells);
//     findShipCells(field, visited, x, y+1, shipCells, numCells);
//     findShipCells(field, visited, x, y-1, shipCells, numCells);
// }

// Функция для очистки массива
void clearFields() {
  for (int f = 0; f < SEL_FIELDS; f++) {
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 16; y++) {
        fields1[f][x][y] = false;
        fields2[f][x][y] = false;
      }
    }
  }
}

// Функция для генерации кораблей в массивах
void generateFields() {
  for (int f = 0; f < SEL_FIELDS; f++) {
    placeShip(fields1[f], 4);  // Линкор (4 клетки)
    placeShip(fields2[f], 4);

    for (int i = 0; i < 2; i++) {
      placeShip(fields1[f], 3);  // Крейсеры (3 клетки)
      placeShip(fields2[f], 3);
    }

    for (int i = 0; i < 3; i++) {
      placeShip(fields1[f], 2);  // Эсминцы (2 клетки)
      placeShip(fields2[f], 2);
    }
    for (int i = 0; i < 4; i++) {
      placeShip(fields1[f], 1);  // Катера (1 клетка)
      placeShip(fields2[f], 1);
    }
  }
}

// Функция для размещения корабля
void placeShip(bool field[16][16], int length) {
  bool placed = false;
  while (!placed) {
    int x = random(0, 16);
    int y = random(0, 16);
    bool horizontal = (random(2) == 0);

    // Проверка, можно ли разместить корабль
    if (canPlaceShip(field, x, y, length, horizontal)) {
      // Размещение корабля
      for (int i = 0; i < length; i++) {
        int nx = x + (horizontal ? i : 0);
        int ny = y + (horizontal ? 0 : i);
        field[nx][ny] = true;
      }
      placed = true;
    }
  }
}

// Функция для проверки возможности размещения корабля
bool canPlaceShip(bool field[16][16], int x, int y, int length, bool horizontal) {
  for (int i = 0; i < length; i++) {
    int nx = x + (horizontal ? i : 0);
    int ny = y + (horizontal ? 0 : i);

    // Проверка выхода за границы
    if (nx >= 16 || ny >= 16) return false;

    // Проверка на наличие корабля в текущей позиции
    if (field[nx][ny]) return false;

    // Проверка всех клеток вокруг корабля на наличие других кораблей
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        int ax = nx + dx;
        int ay = ny + dy;
        if (ax >= 0 && ax < 16 && ay >= 0 && ay < 16 && field[ax][ay]) {
          return false;  // Если соседняя клетка занята, размещение невозможно
        }
      }
    }
  }
  return true;
}

// Отобразить корабли на экране 1 игрока
void displayField1() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      if (fields1[currentField1][x][y]) {
        matrix.setPixelColor(getIndex1(x, y, false), matrix.Color(0, 0, 255));  // Синий для кораблей
      }
    }
  }
}

// Отобразить корабли на экране 2 игрока
void displayField2() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      if (fields2[currentField2][x][y]) {
        matrix.setPixelColor(getIndex2(x, y, false), matrix.Color(0, 0, 255));  // Синий для кораблей
      }
    }
  }
}

// Отобразить курсор у 1 игрока
void displayCursor1() {
  matrix.setPixelColor(getIndex1(cursorX1, cursorY1, true), matrix.Color(0, 255, 0));
}

// Отобразить курсор у 2 игрока
void displayCursor2() {
  matrix.setPixelColor(getIndex2(cursorX2, cursorY2, true), matrix.Color(0, 255, 0));
}

// Отобразить все попадания у игроков
void displayHits1() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      if (surroundCells1[x][y]) {
                // Отмечаем клетки вокруг корабля как промахи
                matrix.setPixelColor(getIndex1(x, y, true), matrix.Color(255, 255, 255));
            }
      if (redPixels1[x][y] && !fields2[currentField2][x][y]) {
        matrix.setPixelColor(getIndex1(x, y, true), matrix.Color(255, 255, 255));
        matrix.setPixelColor(getIndex2(x, y, false), matrix.Color(255, 255, 255));
      }
      if (redPixels1[x][y] && fields2[currentField2][x][y] && !destroyedShips2[x][y]) {
        matrix.setPixelColor(getIndex1(x, y, true), matrix.Color(255, 165, 0));
        matrix.setPixelColor(getIndex2(x, y, false), matrix.Color(255, 165, 0));
      }
      if (destroyedShips2[x][y]) {
        matrix.setPixelColor(getIndex1(x, y, true), matrix.Color(255, 0, 0));
        matrix.setPixelColor(getIndex2(x, y, false), matrix.Color(255, 0, 0));
      }
    }
  }
}

// Отобразить все попадания у игроков
void displayHits2() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      if (surroundCells2[x][y]) {
                // Отмечаем клетки вокруг корабля как промахи
                matrix.setPixelColor(getIndex2(x, y, true), matrix.Color(255, 255, 255));
            }
      if (redPixels2[x][y] && !fields1[currentField1][x][y]) {
        matrix.setPixelColor(getIndex2(x, y, true), matrix.Color(255, 255, 255));
        matrix.setPixelColor(getIndex1(x, y, false), matrix.Color(255, 255, 255));
      }
      if (redPixels2[x][y] && fields1[currentField1][x][y] && !destroyedShips1[x][y]) {
        matrix.setPixelColor(getIndex2(x, y, true), matrix.Color(255, 165, 0));
        matrix.setPixelColor(getIndex1(x, y, false), matrix.Color(255, 165, 0));
      }
      if (destroyedShips1[x][y]) {
        matrix.setPixelColor(getIndex2(x, y, true), matrix.Color(255, 0, 0));
        matrix.setPixelColor(getIndex1(x, y, false), matrix.Color(255, 0, 0));
      }
    }
  }
}

// Выстрел 1 игрока
bool shot1() {
  if (gameOver) return;
  bool hit = false;
    if (updateShot(cursorX1, cursorY1, redPixels1, fields2[currentField2], destroyedShips2, surroundCells1) == true) {
      hit = true;
    }
  // updateShot(cursorX1, cursorY1, redPixels1, fields2[currentField2], destroyedShips2, surroundCells1);
  if (!hasShipsLeft1()) {
    gameOver = true;
    Serial.println("Game Over!");
  }
  
  return hit;
}

// Выстрел 2 игрока
bool shot2() {
  if (gameOver) return;
  bool hit = false;
    if (updateShot(cursorX2, cursorY2, redPixels2, fields1[currentField1], destroyedShips1, surroundCells2) == true) {
      hit = true;
    }
  // updateShot(cursorX2, cursorY2, redPixels2, fields1[currentField1], destroyedShips1, surroundCells2);

  if (!hasShipsLeft2()) {
    gameOver = true;
    Serial.println("Game Over!");
  }
  
  return hit;
}

// Функция проверки сбили ли мы корабль
bool updateShot(int x, int y, bool hits[16][16], bool enemyField[16][16], bool destroyedShips[16][16], bool surroundCells[16][16]) {
  bool hit = false;

  if (enemyField[x][y]) {
    hit = true;
    if (isShipDestroyed(enemyField, hits, x, y)) {
      mp3.play(4);
      delay(500);
      addDestroyed(enemyField, x, y, destroyedShips);  // Сохраняем корабль в массив с сбитыми кораблями
      markAroundShip(enemyField, surroundCells, x, y);
    } else {
      Serial.println("Корабль не разрушен");
       markAroundHit(enemyField, surroundCells, x, y, hits);
    }
  }
  else {
    mp3.play(6);
    delay(500);
  }
return hit;
}

// Функция для проверки был ли корабль разрушен
bool isShipDestroyed(bool field[16][16], bool hits[16][16], int startX, int startY) {
  bool visited[16][16] = { false };
  int dx[] = { 1, -1, 0, 0 };
  int dy[] = { 0, 0, 1, -1 };

  struct Point {
    int x, y;
  };
  Point stack[1024];
  int stackSize = 0;

  stack[stackSize++] = { startX, startY };

  while (stackSize > 0) {
    Point p = stack[--stackSize];
    int x = p.x, y = p.y;

    if (x < 0 || x >= 16 || y < 0 || y >= 16 || visited[x][y] || !field[x][y]) {
      continue;
    }

    visited[x][y] = true;

    if (!hits[x][y]) {
      mp3.play(5);
      delay(500);
      return false;  // Корабль ещё не полностью уничтожен
    }

    for (int i = 0; i < 4; i++) {
      stack[stackSize++] = { x + dx[i], y + dy[i] };
    }
  }

  return true;
}

// Функция для добавления разрушенного корабля в массив со сбитыми кораблями
void addDestroyed(bool field[16][16], int startX, int startY, bool destroyedShips[16][16]) {
  bool visited[16][16] = { false };
  int dx[] = { 1, -1, 0, 0 };
  int dy[] = { 0, 0, 1, -1 };

  struct Point {
    int x;
    int y;
  };
  Point stack[256];
  int stackSize = 0;

  stack[stackSize++] = { startX, startY };

  while (stackSize > 0) {
    Point p = stack[--stackSize];
    int x = p.x, y = p.y;

    if (x < 0 || x >= 16 || y < 0 || y >= 16 || visited[x][y] || !field[x][y]) {
      continue;
    }

    visited[x][y] = true;
    destroyedShips[x][y] = true;
    Serial.println("Корабль разрушен: " + String(x) + " " + String(y));

    for (int i = 0; i < 4; i++) {
      stack[stackSize++] = { x + dx[i], y + dy[i] };
    }
  }
}

// Функция для проверки оставшихся кораблей
bool hasShipsLeft1() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      if (fields2[currentField2][x][y] && !redPixels1[x][y]) {
        return true;  // Остались неуничтоженные корабли
      }
    }
  }
  loser = 2;
  return false;
}

bool hasShipsLeft2() {
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      if (fields1[currentField1][x][y] && !redPixels2[x][y]) {
        return true;  // Остались неуничтоженные корабли
      }
    }
  }
  loser = 1;
  return false;
}

// Проигрывать музыку
void playMusic() {
  if (millis() - lastMusic > MUSIC_DELAY) {
    lastMusic = millis();
    mp3.play(3);
  }
}

// Отображение победного экрана
void displayGameOverMessage() {
  if (loser == 1) {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, matrix.Color(0, 255, 0));  // Зелёный цвет для победного сообщения
  }
  for (int i = 0; i < 512; i++) {
    matrix.setPixelColor(i, matrix.Color(255, 0, 0));  // Зелёный цвет для победного сообщения
  }
  }
  if (loser == 2) {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, matrix.Color(0, 255, 0));  // Зелёный цвет для победного сообщения
  }
  for (int i = 512; i < 1024; i++) {
    matrix.setPixelColor(i, matrix.Color(255, 0, 0));  // Зелёный цвет для победного сообщения
  }
  }
  matrix.show();
}