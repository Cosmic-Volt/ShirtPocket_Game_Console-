  #include <ESP8266WiFi.h>
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64
  #define OLED_RESET -1
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  #define BTN_UP D1
  #define BTN_DOWN D2
  #define BTN_LEFT D4
  #define BTN_RIGHT D3
  #define BUZZER D0
  // #define BTN_SELECT D0


  enum GameState { 
    MENU, PONG, SNAKE, MUSIC_FUR_ELISE, MUSIC_PINK_PANTHER, 
    MARIO, FLAPPY_BIRD, DOODLE_JUMP, PUZZLE, 
    COIN_FLIPPER, MORSE_CODE, FORTUNE_COOKIE, SIMON_SAYS 
  };
  GameState currentState = MENU;

  // Menu
  const char* menuItems[] = {
    "Pong", "Snake", "Fur Elise", "Pink Panther", "Dino", 
    "Flappy Square", "Doodle Jump", "Puzzle", "Coin Flip",
    "Morse Code", "Fortune", "Simon Says"
  };
  int menuSelection = 0;
  const int maxMenuItemsVisible = 4;   // Maximum number of items visible on the screen
  int menuStartIndex = 0;              // Index of the first menu item currently displayed


  // Pong
  #define BALL_SIZE 2  // New constant for ball size
  int ballX, ballY, ballSpeedX, ballSpeedY;
  int paddle1Y, paddle2Y;
  int player1Score, player2Score;

  // Snake
  const int maxSnakeLength = 50;
  int snakeX[maxSnakeLength], snakeY[maxSnakeLength];
  int snakeLength, foodX, foodY;
  int snakeDirectionX, snakeDirectionY;
  unsigned long lastMoveTime = 0;
  const unsigned long moveInterval = 150;
  unsigned long lastDebounceTime[5] = {0, 0, 0, 0, 0};
  const unsigned long debounceDelay = 50;

  // Music
  const int furEliseLength = 40;
  const int pinkPantherLength = 40;
  bool musicPlaying = false;
  unsigned long musicStartTime = 0;

  const PROGMEM uint16_t furEliseMelody[] = {
    // Fur Elise melody (first 40 notes)
    659, 622, 659, 622, 659, 494, 587, 523, 440, 262, 330, 440,
    494, 330, 415, 494, 523, 330, 659, 622, 659, 622, 659, 494,
    587, 523, 440, 262, 330, 440, 494, 330, 523, 494, 440, 440,
    440, 494, 523, 587
  };

  const PROGMEM uint16_t pinkPantherMelody[] = {
    // Pink Panther melody (first 40 notes)
    0, 0, 0, 311, 330, 0, 370, 392, 0, 311, 330, 370, 392, 523,
    494, 330, 392, 494, 466, 440, 392, 330, 311, 330, 0, 0, 311,
    330, 0, 370, 392, 0, 311, 330, 370, 392, 523, 494, 392, 494
  };

  const PROGMEM uint8_t furEliseDurations[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 8, 16, 16, 16,
    8, 16, 16, 16, 8, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 8, 16, 16, 16, 8, 16, 16, 16, 4, 8,
    16, 16, 16, 16
  };

  const PROGMEM uint8_t pinkPantherDurations[] = {
    2, 4, 8, 8, 4, 8, 8, 4, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 16, 16, 16, 16, 2, 4, 8, 8,
    4, 8, 8, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8
  };

  // Mario Game
  const int MARIO_WIDTH = 8;
  const int MARIO_HEIGHT = 8;
  const int GROUND_HEIGHT = 8;
  const int OBSTACLE_WIDTH = 8;
  const int OBSTACLE_HEIGHT = 16;
  const int MAX_OBSTACLES = 3;
  const int MIN_OBSTACLE_DISTANCE = 5 * MARIO_WIDTH; // Minimum distance between obstacles

  int marioY;
  bool isJumping;
  unsigned long jumpStartTime;
  const unsigned long JUMP_DURATION = 600;
  const int JUMP_HEIGHT = 30;

  int obstacleX[MAX_OBSTACLES];
  int obstacleY[MAX_OBSTACLES];
  int obstacleSpeed[MAX_OBSTACLES]; // New: Variable speed for each obstacle
  int score;

  const unsigned char PROGMEM marioSprite[] = {
    0x1F, // 00011111
    0x17, // 00010111
    0x1F, // 00011111
    0x3C, // 00111100
    0x7C, // 01111100
    0xFC, // 11111100
    0xFC, // 11111100
    0x14  // 00010100
  };

  const unsigned char PROGMEM obstacleSprite[] = {
    0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C
  };

  // Flappy Bird Variables
  int birdX, birdY;
  float birdVelocityY;
  float gravity = 0.25;  // Further reduce gravity strength
  float lift = -4;  // Lower the lift when pressing the button
  float damping = 0.9;  // Added damping to smoothen the fall and jump
  int pipeX[2];
  int pipeGapY[2];
  const int pipeWidth = 15;
  const int pipeGapSize = 30;
  int scoreFlappy;

  unsigned long lastFrameTime = 0;
  const unsigned long frameInterval = 30; // Adjust for game speed

  // Doodle Jump Variables
  #define DOODLER_WIDTH 8
  #define DOODLER_HEIGHT 8
  #define PLATFORM_WIDTH 20
  #define PLATFORM_HEIGHT 3
  #define MAX_PLATFORMS 5
  #define MAX_MONSTERS 2
  #define POWER_UP_SIZE 8

  enum PlatformType { STATIC, MOVING, DISAPPEARING };
  enum PowerUpType { NONE, SPRING, JETPACK };

  struct Doodler {
    int16_t x, y;
    float velocityY;
    bool hasPowerUp;
    PowerUpType powerUpType;
    uint8_t powerUpTimer;
  };

  struct Platform {
    int16_t x, y;
    PlatformType type;
    int8_t movingDirection;
    uint8_t disappearingTimer;
  };

  struct Monster {
    int16_t x, y;
    bool active;
    uint8_t type;
  };

  struct PowerUp {
    int16_t x, y;
    PowerUpType type;
    bool active;
  };

  Doodler doodler;
  Platform platforms[MAX_PLATFORMS];
  Monster monsters[MAX_MONSTERS];
  PowerUp powerUp;
  uint16_t doodleScore = 0;

  // Sprite definitions
  const uint8_t PROGMEM doodlerSprite[] = {
    B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100
  };

  const uint8_t PROGMEM monsterSprite1[] = {
    B01111110,
    B11111111,
    B10011001,
    B11111111,
    B01111110,
    B00111100,
    B00011000,
    B00011000
  };

  const uint8_t PROGMEM monsterSprite2[] = {
    B00011000,
    B00111100,
    B01111110,
    B11011011,
    B11111111,
    B01111110,
    B01011010,
    B10011001
  };

  const uint8_t PROGMEM springSprite[] = {
    B00111100,
    B01000010,
    B01000010,
    B00111100,
    B00011000,
    B00011000,
    B00111100,
    B01111110
  };

  const uint8_t PROGMEM jetpackSprite[] = {
    B11000011,
    B11100111,
    B01111110,
    B00111100,
    B00111100,
    B00111100,
    B00111100,
    B01111110
  };
  //puzzle
  const int buttonPins[4] = {5, 4, 0, 2}; // GPIO pins for buttons
int buttonStates[4] = {0, 0, 0, 0};     // Array to hold button states
int emptyTileX = 3;                     // X position of the empty tile (last tile in a 4x4 grid)
int emptyTileY = 3;                     // Y position of the empty tile (last tile in a 4x4 grid)
int puzzle[4][4];                       // 4x4 puzzle grid
bool gameWon = false;                   // Flag to check if the game is won
unsigned long startTime;                // Time when the game starts
unsigned long endTime;                  // Time when the game is solved
unsigned long elapsedTime;              // Elapsed time during the game
  
 // Updated Coin Sprites - one for heads, one for tails
const unsigned char PROGMEM coinHeadsSprite[] = {
    0x00, 0x00, 0x3C, 0x42, 0x99, 0xA5, 0xA5, 0xBD,
    0xA5, 0xA5, 0x99, 0x42, 0x3C, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM coinTailsSprite[] = {
    0x00, 0x00, 0x3C, 0x42, 0xA5, 0x99, 0xBD, 0xA5,
    0xBD, 0x99, 0xA5, 0x42, 0x3C, 0x00, 0x00, 0x00
};

// Updated coin flipper variables
bool isFlipping = false;
unsigned long flipStartTime = 0;
const unsigned long FLIP_DURATION = 1000;
bool coinResult;
bool resultShown = false;


// Morse Code Variables
const int DOT_DURATION = 250;
const int DASH_DURATION = 750;
const int TIMEOUT = 1500;
String morseInput = "";
String currentLetter = "";
unsigned long lastPressTime = 0;
unsigned long pressStartTime = 0;
bool isPressed = false;

// Morse code lookup table
const char* morseCodes[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--", "--.."
};

// Fortune Cookie Variables
const char* fortunes[] PROGMEM = {
    "Great success awaits\nyour next project",
    "Beware of false\nfriends nearby",
    "A golden opportunity\nwill arise soon",
    "Travel plans may\nface delays",
    "Your creativity will\nbe rewarded soon",
    "Watch your spending\nin coming weeks",
    "A surprise gift\nawaits you",
    "Trust your instincts\nin love matters",
    "Career change brings\ngood fortune",
    "Avoid major decisions\nthis month",
    "An old friend will\nreturn to you",
    "Financial gains\nare coming",
    "Health requires\nmore attention",
    "A secret admirer\nwill reveal soon",
    "Family bonds grow\nstronger soon"
    "Your patience will\npay off greatly",
    "A small investment\nbrings big returns",
    "Danger lurks in\nfamiliar places",
    "New skills lead to\ngreat adventures",
    "Someone misses\nyour kindness",
    "Dark clouds bring\nrefreshing rain",
    "Your garden needs\nmore attention",
    "A letter brings\nunexpected news",
    "Trust the wisdom\nof your elders",
    "A dream reveals\nhidden truth",
    "Avoid impulse\npurchases now",
    "Your lucky color\nis blue today",
    "A stranger becomes\na close friend",
    "Past efforts bear\nfruit finally",
    "Caution with\nelectronics needed",
    "Your voice will\nbe heard soon",
    "A pet brings joy\nto your life",
    "Learn from past\nmistakes now",
    "Someone admires\nyour strength",
    "A journey begins\nwith one step",
    "Your cooking\nimproves greatly",
    "Meditation brings\ninner peace",
    "A lost item\nreturns to you",
    "Your art touches\nmany hearts",
    "Take that class\nyou've wanted",
    "A relative needs\nyour support",
    "Your jokes bring\nmore laughter",
    "Time to clean\nyour space",
    "Good news comes\nby phone soon",
    "Your kindness\nreturns threefold",
    "Exercise brings\nunexpected joy",
    "A book changes\nyour perspective",
    "Your music moves\nothers deeply",
    "Save money for\nfuture needs",
    "A child brings\nwisdom today",
    "Your garden will\nbloom brightly",
    "Listen to the\nwind's whispers",
    "A door closes but\nwindow opens",
    "Your smile heals\nothers today",
    "Time to start\nthat project",
    "An enemy becomes\na friend soon",
    "Dance when no one\nis watching",
    "Your words inspire\ngreat change",
    "Learn to cook\nsomething new",
    "Mystery solved\nby month's end",
    "Write down your\ndreams tonight",
    "A gift comes from\nafar soon",
    "Look beneath the\nsurface now",
    "Stars align for\nyour success",
    "Your spirit grows\nstronger daily"
};

const int NUM_FORTUNES = 65;  // Updated number of fortunes
int currentFortune = -1;

//simon says
// Constants for game configuration
const unsigned long SEQUENCE_SHOW_DELAY = 1000;    // Even longer show time for first elements
const unsigned long SEQUENCE_PAUSE_DELAY = 500;    // Longer pause between elements
const unsigned long INPUT_DEBOUNCE = 250;         // Input delay
const unsigned long INITIAL_SEQUENCE_DELAY = 1500; // Longer initial delay
const int MAX_SEQUENCE = 20;                     

// Enhanced Simon Says structure
struct SimonGame {
    int sequence[MAX_SEQUENCE];
    int sequenceLength;
    int currentShowIndex;
    int playerPos;
    bool showingSequence;
    unsigned long lastActionTime;
    int score;
    bool isGameOver;
    bool waitingForRelease;
    unsigned long lastInputTime;
    bool elementVisible;
    bool isFirstSequence;    
    bool sequenceStarting;   
} simonGame;

  void setup() {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    // pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);
    WiFi.mode(WIFI_OFF);

    Wire.begin(D6, D7);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();

    randomSeed(analogRead(A0));
  }

  void loop() {
    switch (currentState) {
      case MENU:
        handleMenu();
        break;
      case PONG:
        playPong();
        break;
      case SNAKE:
        playSnake();
        break;
      case MUSIC_FUR_ELISE:
        playMusic(furEliseMelody, furEliseDurations, furEliseLength);
        break;
      case MUSIC_PINK_PANTHER:
        playMusic(pinkPantherMelody, pinkPantherDurations, pinkPantherLength);
        break;
      case MARIO:
        playMario();
        break;
      case FLAPPY_BIRD:
        playFlappyBird();
        break;
      case DOODLE_JUMP:
        playDoodleJump();
        break;
      case PUZZLE:
            if (!gameWon) {
                updateGameLogic(); // Handle tile movement
                drawPuzzle(); // Redraw puzzle
                if (checkWinCondition()) {
                    gameWon = true;
                    endTime = millis();
                    displayWinScreen(); // Display win screen
                }
            } else {
                delay(2000);  // Display win message for 2 seconds
                initPuzzle();  // Reinitialize the puzzle for a new game
            }
            break;
      case COIN_FLIPPER:
        playCoinFlipper();
        break;
      case MORSE_CODE:
        playMorseCode();
        break;
      case FORTUNE_COOKIE:
        playFortuneCookie();
        break;
      case SIMON_SAYS:
        playSimonSays();
        break;
      
    }
  }

 void handleMenu() {
    static bool buttonReleased = true;  // Track if button has been released
    
    // Handle navigation through the menu
    if (buttonPressed(BTN_UP) && menuSelection > 0) {
        menuSelection--;
        if (menuSelection < menuStartIndex) {
            menuStartIndex--;
        }
    }

    if (buttonPressed(BTN_DOWN) && menuSelection < (sizeof(menuItems) / sizeof(menuItems[0])) - 1) {
        menuSelection++;
        if (menuSelection >= menuStartIndex + maxMenuItemsVisible) {
            menuStartIndex++;
        }
    }

    // Only process game selection if left button was previously released
    if (buttonPressed(BTN_LEFT) && buttonReleased) {
        buttonReleased = false;  // Mark button as pressed
        switch (menuSelection) {
            case 0: initPong(); break;
            case 1: initSnake(); break;
            case 2: currentState = MUSIC_FUR_ELISE; initMusic(); break;
            case 3: currentState = MUSIC_PINK_PANTHER; initMusic(); break;
            case 4: initMario(); break;
            case 5: initFlappyBird(); break;
            case 6: initDoodleJump(); break;
            case 7: initPuzzle(); break;
            case 8: initCoinFlipper(); break;
            case 9: initMorseCode(); break;
            case 10: initFortuneCookie(); break;
            case 11: initSimonSays(); break;
        }
    } else if (!buttonPressed(BTN_LEFT)) {
        buttonReleased = true;  // Mark button as released when it's not pressed
    }

    // Clear the display
    display.clearDisplay();

    // Add a title "Game Console" at the top
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Nintendo Switch 2");

    // Display only visible menu items
    for (int i = menuStartIndex; i < menuStartIndex + maxMenuItemsVisible && i < (sizeof(menuItems) / sizeof(menuItems[0])); i++) {
        display.setCursor(0, 16 + (i - menuStartIndex) * 10);
        if (i == menuSelection) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.println(menuItems[i]);
    }6

    display.display();
    delay(100);
}



  void initPong() {
    currentState = PONG;
    ballX = (SCREEN_WIDTH - BALL_SIZE) / 2;
    ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2;
    ballSpeedX = 2;
    ballSpeedY = 1;
    paddle1Y = SCREEN_HEIGHT / 2 - 8;
    paddle2Y = SCREEN_HEIGHT / 2 - 8;
    player1Score = player2Score = 0;
  }

  void playPong() {
  // Update paddle positions
  if (buttonPressed(BTN_UP) && paddle1Y > 0) paddle1Y -= 2;
    if (buttonPressed(BTN_DOWN) && paddle1Y < SCREEN_HEIGHT - 16) paddle1Y += 2;
    if (buttonPressed(BTN_LEFT) && paddle2Y > 0) paddle2Y -= 2;
    if (buttonPressed(BTN_RIGHT) && paddle2Y < SCREEN_HEIGHT - 16) paddle2Y += 2;

    // Update ball position
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Ball collision with top and bottom
    // Account for ball size in boundary checking
    if (ballY <= 0 || ballY >= SCREEN_HEIGHT - BALL_SIZE) {
        ballSpeedY = -ballSpeedY;
        // Adjust position to prevent sticking to boundaries
        if (ballY <= 0) ballY = 0;
        if (ballY >= SCREEN_HEIGHT - BALL_SIZE) ballY = SCREEN_HEIGHT - BALL_SIZE;
    }

    // Ball collision with paddles
    // Modified collision detection to account for ball size
    if (ballX <= 4 && 
        ballY + BALL_SIZE > paddle1Y && 
        ballY < paddle1Y + 16) {
        ballSpeedX = -ballSpeedX;
        // Adjusted bounce angle calculation for 2x2 ball
        ballSpeedY += (ballY + BALL_SIZE/2 - (paddle1Y + 8)) / 4;
    }
    
    if (ballX >= SCREEN_WIDTH - 5 - BALL_SIZE && 
        ballY + BALL_SIZE > paddle2Y && 
        ballY < paddle2Y + 16) {
        ballSpeedX = -ballSpeedX;
        // Adjusted bounce angle calculation for 2x2 ball
        ballSpeedY += (ballY + BALL_SIZE/2 - (paddle2Y + 8)) / 4;
    }

    // Score - adjusted for ball size
    if (ballX <= 0) { 
        player2Score++; 
        ballX = (SCREEN_WIDTH - BALL_SIZE) / 2; 
        ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2; 
    }
    if (ballX >= SCREEN_WIDTH - BALL_SIZE) { 
        player1Score++; 
        ballX = (SCREEN_WIDTH - BALL_SIZE) / 2; 
        ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2; 
    }

    // Draw
    display.clearDisplay();
    
    // Draw paddles
    display.drawRect(0, paddle1Y, 2, 16, SSD1306_WHITE);
    display.drawRect(SCREEN_WIDTH - 2, paddle2Y, 2, 16, SSD1306_WHITE);
    
    // Draw 2x2 ball using fillRect instead of drawPixel
    display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, SSD1306_WHITE);
    
    // Draw scores
    display.setCursor(SCREEN_WIDTH / 4, 0);
    display.print(player1Score);
    display.setCursor(3 * SCREEN_WIDTH / 4, 0);
    display.print(player2Score);
    
    display.display();

    delay(50); // Control game speed
}

  void initSnake() {

    currentState = SNAKE;
    snakeLength = 1;
    snakeX[0] = ((SCREEN_WIDTH / 2) / 3) * 3;  // Align to 3px grid
    snakeY[0] = ((SCREEN_HEIGHT / 2) / 3) * 3;  // Align to 3px grid
    snakeDirectionX = 1;
    snakeDirectionY = 0;
    placeFood();
  }
  void playSnake() {
    unsigned long currentTime = millis();
    
    // Check for button presses more frequently
    if (buttonPressed(BTN_UP) && snakeDirectionY == 0) { snakeDirectionX = 0; snakeDirectionY = -1; }
    if (buttonPressed(BTN_DOWN) && snakeDirectionY == 0) { snakeDirectionX = 0; snakeDirectionY = 1; }
    if (buttonPressed(BTN_LEFT) && snakeDirectionX == 0) { snakeDirectionX = -1; snakeDirectionY = 0; }
    if (buttonPressed(BTN_RIGHT) && snakeDirectionX == 0) { snakeDirectionX = 1; snakeDirectionY = 0; }

    // Move snake at fixed intervals
    if (currentTime - lastMoveTime >= moveInterval) {
      lastMoveTime = currentTime;

      // Move snake
      for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i-1];
        snakeY[i] = snakeY[i-1];
      }
      snakeX[0] += snakeDirectionX * 3;
      snakeY[0] += snakeDirectionY * 3;

      // Check collision with walls
      if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH || snakeY[0] < 0 || snakeY[0] >= SCREEN_HEIGHT) {
        gameOver();
        return;
      }

      // Check collision with self
      for (int i = 1; i < snakeLength; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
          gameOver();
          return;
        }
      }

      // Check if food eaten
      if (abs(snakeX[0] - foodX) < 3 && abs(snakeY[0] - foodY) < 3) {
        snakeLength++;
        placeFood();
      }

      // Draw
      display.clearDisplay();
      for (int i = 0; i < snakeLength; i++) {
        display.fillRect(snakeX[i], snakeY[i], 3, 3, SSD1306_WHITE);
      }
      display.fillRect(foodX, foodY, 3, 3, SSD1306_WHITE);
      display.display();
    }

    // Small delay to prevent button bouncing
    delay(10);
  }


  void placeFood() {
    foodX = random(0, SCREEN_WIDTH - 2);
    foodY = random(0, SCREEN_HEIGHT - 2);
    
    // Ensure food aligns with the 3px grid
    foodX = (foodX / 3) * 3;
    foodY = (foodY / 3) * 3;
  }

void initMusic() {
    musicPlaying = true;
    musicStartTime = millis();
}

void playMusic(const uint16_t* melody, const uint8_t* durations, int length) {
    static int currentNote = 0;
    static unsigned long noteStartTime = 0;
    
    // Check for stop button first
    if (buttonPressed(BTN_LEFT)) {
        noTone(BUZZER);
        musicPlaying = false;
        currentNote = 0;
        currentState = MENU;
        delay(200); // Debounce delay
        return;
    }

    // If music isn't playing, don't proceed
    if (!musicPlaying) {
        currentState = MENU;
        return;
    }

    // Display status
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Playing Music");
    display.println("Press LEFT to stop");
    display.display();

    unsigned long currentTime = millis();
    
    // Initialize first note if we're just starting
    if (currentNote == 0 && noteStartTime == 0) {
        noteStartTime = currentTime;
        tone(BUZZER, pgm_read_word(&melody[currentNote]));
    }

    // Check if it's time for the next note
    int noteDuration = 1000 / pgm_read_byte(&durations[currentNote]);
    if (currentTime - noteStartTime >= noteDuration * 1.3) {
        noTone(BUZZER);
        currentNote++;
        
        if (currentNote >= length) {
            // Music finished
            musicPlaying = false;
            currentNote = 0;
            currentState = MENU;
            delay(1000);
            return;
        }
        
        // Play next note
        noteStartTime = currentTime;
        tone(BUZZER, pgm_read_word(&melody[currentNote]));
    }

    delay(1);
}

  void initMario() {
    currentState = MARIO;
    marioY = SCREEN_HEIGHT - MARIO_HEIGHT - GROUND_HEIGHT;
    isJumping = false;
    score = 0;
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      obstacleX[i] = SCREEN_WIDTH + i * (SCREEN_WIDTH / MAX_OBSTACLES);
      obstacleY[i] = SCREEN_HEIGHT - OBSTACLE_HEIGHT - GROUND_HEIGHT;
    }
  }

  void playMario() {
    unsigned long currentTime = millis();

    // Handle jump
    if (buttonPressed(BTN_UP) && !isJumping) {
      isJumping = true;
      jumpStartTime = currentTime;
    }

    // Update Mario's position
    if (isJumping) {
      unsigned long jumpTime = currentTime - jumpStartTime;
      if (jumpTime < JUMP_DURATION) {
        float jumpProgress = (float)jumpTime / JUMP_DURATION;
        // Modified jump curve for a higher, more effective jump
        marioY = SCREEN_HEIGHT - MARIO_HEIGHT - GROUND_HEIGHT - sin(jumpProgress * PI) * JUMP_HEIGHT;
      } else {
        isJumping = false;
        marioY = SCREEN_HEIGHT - MARIO_HEIGHT - GROUND_HEIGHT;
      }
    }

    // Update obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      obstacleX[i] -= 2;
      if (obstacleX[i] < -OBSTACLE_WIDTH) {
        obstacleX[i] = SCREEN_WIDTH;
        score++;
      }

      // Check for collision
      if (obstacleX[i] < MARIO_WIDTH && obstacleX[i] + OBSTACLE_WIDTH > 0 &&
          marioY + MARIO_HEIGHT > obstacleY[i]) {
        gameOver();
        return;
      }
    }

    // Draw game
    display.clearDisplay();
    display.drawBitmap(0, marioY, marioSprite, MARIO_WIDTH, MARIO_HEIGHT, SSD1306_WHITE);
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      display.drawBitmap(obstacleX[i], obstacleY[i], obstacleSprite, OBSTACLE_WIDTH, OBSTACLE_HEIGHT, SSD1306_WHITE);
    }
    display.drawFastHLine(0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Score: ");
    display.print(score);
    display.display();

    delay(20); // Adjust for desired game speed
  }


  void initFlappyBird() {
    currentState = FLAPPY_BIRD;
    birdX = SCREEN_WIDTH / 4;
    birdY = SCREEN_HEIGHT / 2;
    birdVelocityY = 0;
    for (int i = 0; i < 2; i++) {
      pipeX[i] = SCREEN_WIDTH + i * (SCREEN_WIDTH / 2);
      pipeGapY[i] = random(10, SCREEN_HEIGHT - pipeGapSize - 10);
    }
    scoreFlappy = 0;
  }

  void playFlappyBird() {
    unsigned long currentTime = millis();
    if (currentTime - lastFrameTime >= frameInterval) {
      lastFrameTime = currentTime;

      // Handle input
      if (buttonPressed(BTN_UP)) {
        birdVelocityY = lift;  // Apply upward force
      }

      // Apply gravity over time
      birdVelocityY += gravity;
      birdVelocityY *= damping;  // Apply damping to smooth the fall
      birdY += birdVelocityY;

      // Limit bird's fall speed
      if (birdVelocityY > 4) {  // Reduced terminal velocity
        birdVelocityY = 4;
      }

      // Move pipes
      for (int i = 0; i < 2; i++) {
        pipeX[i] -= 2;
        if (pipeX[i] < -pipeWidth) {
          pipeX[i] = SCREEN_WIDTH;
          pipeGapY[i] = random(10, SCREEN_HEIGHT - pipeGapSize - 10);
          scoreFlappy++;
        }
      }

      // Check for collisions
      for (int i = 0; i < 2; i++) {
        if (birdX + 5 > pipeX[i] && birdX < pipeX[i] + pipeWidth) {
          if (birdY < pipeGapY[i] || birdY > pipeGapY[i] + pipeGapSize) {
            gameOver();  // Bird hit the pipe
            return;
          }
        }
      }

      // Check if bird hits the ground or flies too high
      if (birdY < 0 || birdY > SCREEN_HEIGHT) {
        gameOver();
        return;
      }

      // Draw the game
      display.clearDisplay();
      display.fillRect(birdX, birdY, 5, 5, SSD1306_WHITE);  // Draw the bird
      for (int i = 0; i < 2; i++) {
        display.fillRect(pipeX[i], 0, pipeWidth, pipeGapY[i], SSD1306_WHITE);  // Top pipe
        display.fillRect(pipeX[i], pipeGapY[i] + pipeGapSize, pipeWidth, SCREEN_HEIGHT - pipeGapY[i] - pipeGapSize, SSD1306_WHITE);  // Bottom pipe
      }
      display.setCursor(0, 0);
      display.print("Score: ");
      display.print(scoreFlappy);
      display.display();
    }

    delay(10);  // Small delay to smooth out the game
  }

  void gameOverFlappy() {
    display.clearDisplay();
    display.setCursor(20, 20);
    display.print("Game Over!");
    display.setCursor(20, 30);
    display.print("Score: ");
    display.print(scoreFlappy);
    display.display();
    delay(2000);
    currentState = MENU;
  }
  void initDoodleJump() {
    currentState = DOODLE_JUMP;
    doodler = {SCREEN_WIDTH / 2 - DOODLER_WIDTH / 2, SCREEN_HEIGHT - DOODLER_HEIGHT - 10, -2, false, NONE, 0};
    doodleScore = 0;

    for (int i = 0; i < MAX_PLATFORMS; i++) {
      platforms[i] = {random(0, SCREEN_WIDTH - PLATFORM_WIDTH),
                      SCREEN_HEIGHT - (i * (SCREEN_HEIGHT / MAX_PLATFORMS)),
                      static_cast<PlatformType>(random(3)),
                      static_cast<int8_t>(random(2) * 2 - 1),
                      60};
    }

    for (int i = 0; i < MAX_MONSTERS; i++) {
      monsters[i] = {0, 0, false, 0};
    }

    powerUp = {0, 0, NONE, false};
  }

  void playDoodleJump() {
    moveDoodler();
    movePlatforms();
    spawnMonster();
    spawnPowerUp();

    // Draw game elements
    display.clearDisplay();
    drawPlatforms();
    drawMonsters();
    drawPowerUp();
    drawDoodler();
    displayScore();

    display.display();
    delay(16);  // Adjusted for smoother gameplay (about 60 FPS)
  }

  void moveDoodler() {
    if (buttonPressed(BTN_LEFT)) {
      doodler.x -= 4;
      if (doodler.x < 0) doodler.x = SCREEN_WIDTH - DOODLER_WIDTH;
    }
    if (buttonPressed(BTN_RIGHT)) {
      doodler.x += 4;
      if (doodler.x > SCREEN_WIDTH - DOODLER_WIDTH) doodler.x = 0;
    }

    if (doodler.hasPowerUp) {
      doodler.velocityY = (doodler.powerUpType == SPRING) ? -8.0 : -6.0;
      if (--doodler.powerUpTimer <= 0) {
        doodler.hasPowerUp = false;
        doodler.powerUpType = NONE;
      }
    } else {
      doodler.velocityY += 0.2;
    }
    
    float nextY = doodler.y + doodler.velocityY;

    // Platform collisions
    bool collision = false;
    for (int i = 0; i < MAX_PLATFORMS; i++) {
      if (doodler.velocityY > 0 &&
          doodler.x < platforms[i].x + PLATFORM_WIDTH &&
          doodler.x + DOODLER_WIDTH > platforms[i].x &&
          doodler.y + DOODLER_HEIGHT <= platforms[i].y &&
          nextY + DOODLER_HEIGHT > platforms[i].y) {
        if (platforms[i].type != DISAPPEARING || platforms[i].disappearingTimer > 0) {
          doodler.velocityY = -5.0;
          doodleScore++;
          if (platforms[i].type == DISAPPEARING) platforms[i].disappearingTimer = 60;
          collision = true;
          break;
        }
      }
    }

    if (!collision) {
      doodler.y = nextY;
    }

    // Monster collisions
    for (int i = 0; i < MAX_MONSTERS; i++) {
      if (monsters[i].active &&
          doodler.x < monsters[i].x + DOODLER_WIDTH &&
          doodler.x + DOODLER_WIDTH > monsters[i].x &&
          doodler.y < monsters[i].y + DOODLER_HEIGHT &&
          doodler.y + DOODLER_HEIGHT > monsters[i].y) {
        gameOver();
        return;
      }
    }

    // Power-up collision
    if (powerUp.active &&
        doodler.x < powerUp.x + POWER_UP_SIZE &&
        doodler.x + DOODLER_WIDTH > powerUp.x &&
        doodler.y < powerUp.y + POWER_UP_SIZE &&
        doodler.y + DOODLER_HEIGHT > powerUp.y) {
      doodler.hasPowerUp = true;
      doodler.powerUpType = powerUp.type;
      doodler.powerUpTimer = 100;
      powerUp.active = false;
    }

    // Scroll screen
    if (doodler.y < SCREEN_HEIGHT / 2) {
      int16_t offset = SCREEN_HEIGHT / 2 - doodler.y;
      doodler.y += offset;
      for (int i = 0; i < MAX_PLATFORMS; i++) {
        platforms[i].y += offset;
        if (platforms[i].y > SCREEN_HEIGHT) {
          platforms[i].y = 0;
          platforms[i].x = random(0, SCREEN_WIDTH - PLATFORM_WIDTH);
          platforms[i].type = static_cast<PlatformType>(random(3));
          platforms[i].movingDirection = random(2) * 2 - 1;
          platforms[i].disappearingTimer = 60;
        }
      }
      for (int i = 0; i < MAX_MONSTERS; i++) {
        if (monsters[i].active) {
          monsters[i].y += offset;
          if (monsters[i].y > SCREEN_HEIGHT) monsters[i].active = false;
        }
      }
      if (powerUp.active) {
        powerUp.y += offset;
        if (powerUp.y > SCREEN_HEIGHT) powerUp.active = false;
      }
    }

    if (doodler.y > SCREEN_HEIGHT) gameOver();
  }

  void movePlatforms() {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
      if (platforms[i].type == MOVING) {
        platforms[i].x += platforms[i].movingDirection;
        if (platforms[i].x <= 0 || platforms[i].x >= SCREEN_WIDTH - PLATFORM_WIDTH) {
          platforms[i].movingDirection *= -1;
        }
      } else if (platforms[i].type == DISAPPEARING && platforms[i].disappearingTimer > 0) {
        platforms[i].disappearingTimer--;
      }
    }
  }

  void spawnMonster() {
    if (random(100) < 2) {
      for (int i = 0; i < MAX_MONSTERS; i++) {
        if (!monsters[i].active) {
          monsters[i] = {random(0, SCREEN_WIDTH - DOODLER_WIDTH), 0, true, static_cast<uint8_t>(random(2))};
          break;
        }
      }
    }
  }

  void spawnPowerUp() {
    if (!powerUp.active && random(100) < 1) {
      powerUp = {random(0, SCREEN_WIDTH - POWER_UP_SIZE), 0, static_cast<PowerUpType>(random(1, 3)), true};
    }
  }

  void drawSprite(int16_t x, int16_t y, const uint8_t* sprite, uint8_t w, uint8_t h) {
    display.drawBitmap(x, y, sprite, w, h, SSD1306_WHITE);
  }

  void drawDoodler() {
    drawSprite(doodler.x, doodler.y, doodlerSprite, DOODLER_WIDTH, DOODLER_HEIGHT);
  }

  void drawPlatforms() {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
      if (platforms[i].type != DISAPPEARING || platforms[i].disappearingTimer > 0) {
        display.fillRect(platforms[i].x, platforms[i].y, PLATFORM_WIDTH, PLATFORM_HEIGHT, SSD1306_WHITE);
      }
    }
  }

  void drawMonsters() {
    for (int i = 0; i < MAX_MONSTERS; i++) {
      if (monsters[i].active) {
        drawSprite(monsters[i].x, monsters[i].y, 
                  monsters[i].type == 0 ? monsterSprite1 : monsterSprite2, 
                  DOODLER_WIDTH, DOODLER_HEIGHT);
      }
    }
  }

  void drawPowerUp() {
    if (powerUp.active) {
      drawSprite(powerUp.x, powerUp.y, 
                powerUp.type == SPRING ? springSprite : jetpackSprite, 
                POWER_UP_SIZE, POWER_UP_SIZE);
    }
  }

  void displayScore() {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("Score: "));
    display.print(doodleScore);
  }
  //puzzle
  void initPuzzle() {
    currentState = PUZZLE;
    initializePuzzle(); // Call the puzzle initialization function from the original code
    startTime = millis(); // Start the timer
    drawPuzzle(); // Draw the initial puzzle state
}

  void initializePuzzle() {
    int nums[15];
    for (int i = 0; i < 15; i++) {
        nums[i] = i + 1;
    }

    // Shuffle the numbers
    for (int i = 0; i < 15; i++) {
        int j = random(0, 15);
        int temp = nums[i];
        nums[i] = nums[j];
        nums[j] = temp;
    }

    // Fill the puzzle grid with the shuffled numbers
    int index = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == 3 && j == 3) {
                puzzle[i][j] = 0; // Empty tile
            } else {
                puzzle[i][j] = nums[index++];
            }
        }
    }
    emptyTileX = 3;
    emptyTileY = 3;
    startTime = millis();  // Reset the start time when the puzzle is initialized
}

void drawPuzzle() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Draw the timer in the format: minutes on top, seconds below
    unsigned long currentTime = millis();
    elapsedTime = (currentTime - startTime) / 1000; // Elapsed time in seconds
    int minutes = elapsedTime / 60;
    int seconds = elapsedTime % 60;

    // Display minutes and seconds
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(minutes);

    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(seconds);

    // Draw the puzzle grid
    display.setTextSize(1);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (puzzle[i][j] != 0) {
                display.setCursor(j * 24 + 30, i * 16); // Adjusted positioning for 4x4 grid and timer
                display.print(puzzle[i][j]);
            }
        }
    }
    display.display();
}

void updateGameLogic() {
  if (buttonPressed(BTN_UP)) {
    moveUp();
  }
  if (buttonPressed(BTN_DOWN)) {
    moveDown();
  }
  if (buttonPressed(BTN_LEFT)) {
    moveLeft();
  }
  if (buttonPressed(BTN_RIGHT)) {
    moveRight();
  }
   // Redraw puzzle after movement
  drawPuzzle();
}
void moveUp() {
  if (emptyTileY < 3) {  // Check if there's room to move up
    swapTiles(emptyTileX, emptyTileY, emptyTileX, emptyTileY + 1);  // Swap empty space with the tile below
    emptyTileY++;  // Update empty space location
  }
  delay(150);
}

void moveDown() {
  if (emptyTileY > 0) {  // Check if there's room to move down
    swapTiles(emptyTileX, emptyTileY, emptyTileX, emptyTileY - 1);  // Swap empty space with the tile above
    emptyTileY--;  // Update empty space location
  }
  delay(150);
}

void moveLeft() {
  if (emptyTileX < 3) {  // Check if there's room to move left
    swapTiles(emptyTileX, emptyTileY, emptyTileX + 1, emptyTileY);  // Swap empty space with the tile to the right
    emptyTileX++;  // Update empty space location
  }
  delay(150);
}

void moveRight() {
  if (emptyTileX > 0) {  // Check if there's room to move right
    swapTiles(emptyTileX, emptyTileY, emptyTileX - 1, emptyTileY);  // Swap empty space with the tile to the left
    emptyTileX--;  // Update empty space location
  }
  delay(150);
}


void swapTiles(int x1, int y1, int x2, int y2) {
  int temp = puzzle[y1][x1];  // Temporarily store the value at (x1, y1)
  puzzle[y1][x1] = puzzle[y2][x2];  // Move the value at (x2, y2) to (x1, y1)
  puzzle[y2][x2] = temp;  // Move the stored value to (x2, y2)
}


bool checkWinCondition() {
    int correctNum = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == 3 && j == 3) {
                if (puzzle[i][j] != 0) return false; // The last tile must be empty
            } else if (puzzle[i][j] != correctNum++) {
                return false;
            }
        }
    }
    return true; // Puzzle is in correct order
}

void displayWinScreen() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);  // Center the text
    display.print("You Win!");

    // Display the total time taken to solve the puzzle
    int totalMinutes = (endTime - startTime) / 60000;
    int totalSeconds = ((endTime - startTime) / 1000) % 60;

    // Display total minutes and seconds
    display.setTextSize(2);
    display.setCursor(40, 40);
    display.print(totalMinutes);

    display.setCursor(40, 60);
    display.print(totalSeconds);

    display.display();
}
void initCoinFlipper() {
    currentState = COIN_FLIPPER;
    isFlipping = false;
    resultShown = false;
}

void playCoinFlipper() {
    display.clearDisplay();
    
    if (buttonPressed(BTN_UP)) {
        isFlipping = true;
        flipStartTime = millis();
        coinResult = random(2) == 1;
        resultShown = false;
    }
    
    if (isFlipping) {
        unsigned long currentTime = millis();
        if (currentTime - flipStartTime < FLIP_DURATION) {
            // Animation effect - alternate between heads and tails sprites
            int frame = (currentTime / 100) % 2;
            display.drawBitmap(
                (SCREEN_WIDTH - 16) / 2, 
                (SCREEN_HEIGHT - 16) / 2,
                frame ? coinHeadsSprite : coinTailsSprite, 
                16, 16, 
                SSD1306_WHITE
            );
        } else {
            isFlipping = false;
            resultShown = true;
        }
    }
    
    if (resultShown) {
        // Show the final result with appropriate sprite
        display.drawBitmap(
            (SCREEN_WIDTH - 16) / 2, 
            (SCREEN_HEIGHT - 16) / 2,
            coinResult ? coinHeadsSprite : coinTailsSprite, 
            16, 16, 
            SSD1306_WHITE
        );
        
        // Display text result
        display.setTextSize(1);
        display.setCursor((SCREEN_WIDTH - 30) / 2, SCREEN_HEIGHT - 10);
        display.print(coinResult ? "HEADS" : "TAILS");
    } else if (!isFlipping) {
        // Initial state
        display.drawBitmap(
            (SCREEN_WIDTH - 16) / 2, 
            (SCREEN_HEIGHT - 16) / 2,
            coinHeadsSprite, 
            16, 16, 
            SSD1306_WHITE
        );
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Press UP to flip");
    }
    
    display.display();
    delay(50);
}

// Morse Code Implementation
void initMorseCode() {
    currentState = MORSE_CODE;
    morseInput = "";
    currentLetter = "";
}

void playMorseCode() {
    display.clearDisplay();
    display.setTextSize(1);
    
    // Handle button input
    if (digitalRead(BTN_UP) == LOW) {
        if (!isPressed) {
            isPressed = true;
            pressStartTime = millis();
        }
    } else if (isPressed) {
        unsigned long duration = millis() - pressStartTime;
        if (duration < DOT_DURATION) {
            currentLetter += ".";
        } else {
            currentLetter += "-";
        }
        isPressed = false;
        lastPressTime = millis();
    }
    
    // Check for letter completion
    if (!currentLetter.isEmpty() && millis() - lastPressTime > TIMEOUT) {
        // Convert morse to letter
        for (int i = 0; i < 26; i++) {
            if (currentLetter == morseCodes[i]) {
                morseInput += (char)('A' + i);
                break;
            }
        }
        currentLetter = "";
    }
    
    // Display
    display.setCursor(0, 0);
    display.println("Morse Code Translator");
    display.println();
    display.println("Current: " + currentLetter);
    display.println();
    display.println("Message: " + morseInput);
    
    display.display();
    delay(50);
}

// Fortune Cookie Implementation
void initFortuneCookie() {
    currentState = FORTUNE_COOKIE;
    currentFortune = -1;
}


void playFortuneCookie() {
    display.clearDisplay();
    display.setTextSize(1);
    
    if (buttonPressed(BTN_UP) || currentFortune == -1) {
        currentFortune = random(NUM_FORTUNES);
    }
    
    // Display fortune cookie art
    display.drawRect(SCREEN_WIDTH/2 - 20, SCREEN_HEIGHT/2 - 10, 40, 20, SSD1306_WHITE);
    display.drawLine(SCREEN_WIDTH/2 - 20, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 + 20, SCREEN_HEIGHT/2, SSD1306_WHITE);
    
    // Display fortune text
    display.setCursor(0, 0);
    display.println("Your Fortune:");
    
    // Get the fortune text
    String fortune = FPSTR(fortunes[currentFortune]);
    
    // Split the fortune at the newline character
    int newlinePos = fortune.indexOf('\n');
    String line1 = fortune.substring(0, newlinePos);
    String line2 = fortune.substring(newlinePos + 1);
    
    // Display both lines
    display.setCursor(0, SCREEN_HEIGHT - 16);
    display.println(line1);
    display.setCursor(0, SCREEN_HEIGHT - 8);
    display.println(line2);
    
    display.display();
    delay(50);
}


void initSimonSays() {
    // Initialize random seed using internal clock
    randomSeed(millis() * micros());  // Combining millis and micros for better randomness
    
    currentState = SIMON_SAYS;
    simonGame.sequenceLength = 1;
    simonGame.currentShowIndex = 0;
    simonGame.playerPos = 0;
    simonGame.showingSequence = true;
    simonGame.score = 0;
    simonGame.isGameOver = false;
    simonGame.lastActionTime = millis();
    simonGame.waitingForRelease = false;
    simonGame.lastInputTime = 0;
    
    // Generate initial sequence
    generateNewSequence();
}

// New function to generate random sequence
void generateNewSequence() {
    for (int i = 0; i < MAX_SEQUENCE; i++) {
        simonGame.sequence[i] = random(4);
    }
}


void playSimonSays() {
    if (simonGame.isGameOver) {
        score = simonGame.score;
        displayGameOver();
        return;
    }

    display.clearDisplay();
    
    // Show score and game state indicator at top
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(simonGame.showingSequence ? "WATCH" : "PLAY");
    
    display.setCursor(SCREEN_WIDTH - 42, 0);
    display.print("Score:");
    display.print(simonGame.score);
    
    if (simonGame.showingSequence) {
        handleSequenceDisplay();
    } else {
        handlePlayerInput();
    }
    
    display.display();
    delay(16);  // Approximately 60fps for smoother animation
}

void handleSequenceDisplay() {
    unsigned long currentTime = millis();
    
    // Calculate if we should show or hide the current sequence element
    unsigned long timeInCycle = currentTime - simonGame.lastActionTime;
    bool showElement = timeInCycle < SEQUENCE_SHOW_DELAY;
    
    // If we've completed showing and hiding the current element
    if (timeInCycle >= (SEQUENCE_SHOW_DELAY + SEQUENCE_PAUSE_DELAY)) {
        simonGame.currentShowIndex++;
        simonGame.lastActionTime = currentTime;
        
        // If we've shown all elements, switch to player input
        if (simonGame.currentShowIndex >= simonGame.sequenceLength) {
            simonGame.showingSequence = false;
            simonGame.playerPos = 0;
            simonGame.currentShowIndex = 0;
        }
    }
    
    // Draw boxes with current sequence element highlighted if needed
    drawSimonBoxes(showElement ? simonGame.sequence[simonGame.currentShowIndex] : -1);
}

void handlePlayerInput() {
    unsigned long currentTime = millis();
    
    // Draw empty boxes for player input
    drawSimonBoxes(-1);
    
    // Only process input after debounce time
    if (currentTime - simonGame.lastInputTime >= INPUT_DEBOUNCE) {
        int playerInput = -1;
        
        if (!simonGame.waitingForRelease) {
            if (buttonPressed(BTN_UP)) playerInput = 0;
            else if (buttonPressed(BTN_RIGHT)) playerInput = 1;
            else if (buttonPressed(BTN_DOWN)) playerInput = 2;
            else if (buttonPressed(BTN_LEFT)) playerInput = 3;
            
            if (playerInput != -1) {
                simonGame.waitingForRelease = true;
                simonGame.lastInputTime = currentTime;
                
                // Visual feedback for button press
                drawSimonBoxes(playerInput);
                display.display();
                delay(150);  // Reduced for better responsiveness
                
                // Check if input matches sequence
                if (playerInput != simonGame.sequence[simonGame.playerPos]) {
                    simonGame.score = simonGame.sequenceLength - 1;
                    simonGame.isGameOver = true;
                    return;
                }
                
                simonGame.playerPos++;
                
                // Sequence completed correctly
                if (simonGame.playerPos >= simonGame.sequenceLength) {
                    simonGame.score = simonGame.sequenceLength;
                    simonGame.sequenceLength++;
                    
                    if (simonGame.sequenceLength > MAX_SEQUENCE) {
                        simonGame.isGameOver = true;
                        return;
                    }
                    
                    // Reset for next sequence
                    simonGame.showingSequence = true;
                    simonGame.currentShowIndex = 0;
                    simonGame.playerPos = 0;
                    simonGame.lastActionTime = currentTime;
                    
                    delay(400);  // Reduced pause before next sequence
                }
            }
        } else if (!buttonPressed(BTN_UP) && !buttonPressed(BTN_RIGHT) && 
                   !buttonPressed(BTN_DOWN) && !buttonPressed(BTN_LEFT)) {
            simonGame.waitingForRelease = false;
        }
    }
}

void drawSimonBoxes(int highlightBox) {
    const int BOX_SIZE = 15;  // Reduced from 16 to make boxes one pixel smaller
    const int CENTER_X = SCREEN_WIDTH / 2;
    const int CENTER_Y = SCREEN_HEIGHT / 2;
    const int SPACING = 19;   // Adjusted spacing for smaller boxes
    
    // Up box
    drawBox(CENTER_X - BOX_SIZE/2, CENTER_Y - SPACING - BOX_SIZE, BOX_SIZE, highlightBox == 0);
    
    // Right box
    drawBox(CENTER_X + SPACING, CENTER_Y - BOX_SIZE/2, BOX_SIZE, highlightBox == 1);
    
    // Down box
    drawBox(CENTER_X - BOX_SIZE/2, CENTER_Y + SPACING, BOX_SIZE, highlightBox == 2);
    
    // Left box
    drawBox(CENTER_X - SPACING - BOX_SIZE, CENTER_Y - BOX_SIZE/2, BOX_SIZE, highlightBox == 3);
}

void drawBox(int x, int y, int size, bool filled) {
    display.drawRect(x, y, size, size, SSD1306_WHITE);
    if (filled) {
        display.fillRect(x + 1, y + 1, size - 2, size - 2, SSD1306_WHITE);
    }
}

void displayGameOver() {
    display.clearDisplay();
    display.setTextSize(1);
    
    // Center "Game Over!" text
    display.setCursor(SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT/2 - 8);
    display.print("Game Over!");
    
    // Center score text
    display.setCursor(SCREEN_WIDTH/2 - 25, SCREEN_HEIGHT/2 + 4);
    display.print("Score: ");
    display.print(simonGame.score);
    
    display.display();
    delay(2000);
    currentState = MENU;
}

  bool buttonPressed(int pin) {
    int buttonIndex;
    switch(pin) {
      case BTN_UP: buttonIndex = 0; break;
      case BTN_DOWN: buttonIndex = 1; break;
      case BTN_LEFT: buttonIndex = 2; break;
      case BTN_RIGHT: buttonIndex = 3; break;
      // case BTN_SELECT: buttonIndex = 4; break;
      default: return false;
    }

    if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
      if (digitalRead(pin) == LOW) {
        lastDebounceTime[buttonIndex] = millis();
        return true;
      }
    }
    return false;
  }


  void gameOver() {
    display.clearDisplay();
    display.setCursor(20, 20);
    display.print("Game Over!");
    
    if (currentState == MARIO) {
      display.setCursor(20, 30);
      display.print("Score: ");
      display.print(score);
    } else if (currentState == DOODLE_JUMP) {
      display.setCursor(20, 30);
      display.print("Score: ");
      display.print(doodleScore);
    }
    
    display.display();
    delay(2000);
    currentState = MENU;
  }

