/**
 *  Adam Stebbing
 *  11/23/2015
 *
 *  Dueling Tanks flash game implemented on an mbed microprocessor with
 *  additional hardware components.
 */

#include "mbed.h"

#include "SDFileSystem.h"
#include "wave_player.h"
#include "game_synchronizer.h"
#include "tank.h"
#include "bullet.h"
#include "globals.h"
#include "wave_player.h"
#include "playSound.h"
#include "RGBLed.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalIn pb_u(p21);                        // Up Button
DigitalIn pb_r(p22);                        // Right Button
DigitalIn pb_d(p23);                        // Down Button
DigitalIn pb_l(p24);                        // Left Button

Serial pc(USBTX, USBRX);                    // Serial connection to PC. Useful for debugging!
MMA8452 acc(p28, p27, 100000);              // Accelerometer (SDA, SCL, Baudrate)
uLCD_4DGL uLCD(p9,p10,p11);                 // LCD (tx, rx, reset)
SDFileSystem sd(p5, p6, p7, p8, "sd");      // SD  (mosi, miso, sck, cs)
AnalogOut DACout(p18);                      // speaker
wave_player player(&DACout);                // wav player
Game_Synchronizer sync(PLAYER1);            // Game_Synchronizer (PLAYER)
Timer frame_timer;                          // Timer

RGBLed rgb(p26, p25);                           // RGBLed

// Global variables go here.

int winner = -1;
int whose_turn = PLAYER1;
int map = 1;
int tankColors = 1;
int t1color = TANK_RED;
int t2color = TANK_BLUE;
int t1kill = 0;
int t2kill = 0;
char temp;
int type = 1;

// Ask the user whether to run the game in Single- or Multi-Player mode.
// Note that this function uses uLCD instead of sync because it is only
// writing to the local (Player1) lcd. Sync hasn't been initialized yet,
// so you can't use it anyway. For the same reason, you must access
// the buttons directly e.g. if( !pb_r ) { do something; }.


// return MULTI_PLAYER if the user selects multi-player.
// return SINGLE_PLAYER if the user selects single-player.
int game_menu(void) {

    uLCD.baudrate(BAUD_3000000);

    // the locate command tells the screen where to place the text.
    uLCD.locate(0,0);
    uLCD.puts("Choose a mode:");
    uLCD.locate(2,2);
    uLCD.puts("Single-Player:");
    uLCD.locate(3,3);
    uLCD.puts("Left Button");
    uLCD.locate(2,4);
    uLCD.puts("Multi-Player:");
    uLCD.locate(3,5);
    uLCD.puts("Right Button");
    uLCD.locate(0,7);
    uLCD.puts("Map:");
    uLCD.locate(5,7);
    uLCD.puts("1");
    uLCD.locate(0,8);
    uLCD.puts("Tank Colors:");
    uLCD.locate(1,9);
    uLCD.puts("Red/Blue");

    uLCD.locate(0,14);
    uLCD.puts("P1 Kills:");
    uLCD.locate(10,14);
    temp = t1kill + '0';
    uLCD.putc(temp);
    uLCD.locate(0,15);
    uLCD.puts("P2 Kills:");
    uLCD.locate(10,15);
    temp = t2kill + '0';
    uLCD.putc(temp);

    while(1) {
        if (!pb_u) {
            uLCD.locate(1,9);
            if (tankColors == 3) {
                tankColors = 1;
                t1color = TANK_RED;
                t2color = TANK_BLUE;
                uLCD.puts("Red/Blue     ");
            } else {
                tankColors++;
            }
            if (tankColors == 2) {
                t1color = 0xFFC0CB;
                t2color = 0x00006f;
                uLCD.puts("Pink/Navy    ");
            } else if (tankColors == 3) {
                t1color = 0x8d0000;
                t2color = 0x6c76ff;
                uLCD.puts("Maroon/Violet");
            }
            wait(0.25);
        }
        if (!pb_d) {
            if (map == 3) {
                map = 1;
            } else {
                map++;
            }
            uLCD.locate(5,7);
            temp = map + '0';
            uLCD.putc(temp);
            wait(0.25);
        }
        if (!pb_l) {
            return SINGLE_PLAYER;
        }
        if (!pb_r) {
            return MULTI_PLAYER;
        }
    }
}

// Initialize the world map. I've provided a basic map here,
// but as part of the assignment you must create more
// interesting map(s).
// Note that calls to sync.function() will run function()
// on both players' LCDs (assuming you are in multiplayer mode).
// In single player mode, only your lcd will be modified. (Makes sense, right?)
void map_init() {
    // Fill the entire screen with sky blue.
    sync.background_color(SKY_COLOR);

    // Call the clear screen function to force the LCD to redraw the screen
    // with the new background color.
    sync.cls();

    // Draw the ground in green.
    sync.filled_rectangle(0,0,128,20, GND_COLOR);
    for (int x = 3; x > 0; x--) {
        sync.filled_circle(70 + x*10, 125, 3, t1color);
    }
    for (int x = 3; x > 0; x--) {
        sync.filled_circle(70 + x*10, 115, 3, t2color);
    }

    rgb.write(TANK_RED);

    // Draw obstacles and background.
    if (map == 1) {
        sync.triangle(44, 60, 84, 60, 64, 90, BLACK);
        sync.filled_circle(65, 72, 5, WHITE);
        sync.filled_circle(66, 73, 2, BLACK);
        sync.filled_circle(119, 119, 7, 0xFFFF00);
        sync.line(112, 123, 126, 123, BLACK);
        sync.line(115, 115, 123, 115, BLACK);
        sync.filled_circle(116, 121, 3, BLACK);
        sync.filled_circle(122, 121, 3, BLACK);
        sync.filled_rectangle(54,20,74,30, 0x660066);

        sync.locate(0,15);
        sync.textbackground_color(SKY_COLOR);
        char title[] = "Illuminati";
        sync.puts(title, sizeof(title));
        char title2[] = "Confirmed";
        sync.locate(0,14);
        sync.puts(title2, sizeof(title2));
    } else if (map == 2) {
        sync.triangle(44, 20, 84, 20, 64, 60, WHITE);
        sync.triangle(45, 21, 83, 21, 64, 59, WHITE);
        sync.triangle(46, 22, 82, 22, 64, 58, WHITE);
        sync.triangle(47, 23, 81, 23, 64, 57, WHITE);
        sync.triangle(48, 24, 80, 24, 64, 56, WHITE);
        sync.triangle(49, 25, 79, 25, 64, 55, WHITE);
        sync.triangle(50, 26, 78, 26, 64, 54, WHITE);
        sync.triangle(51, 27, 77, 27, 64, 53, WHITE);
        sync.triangle(52, 28, 76, 28, 64, 52, WHITE);
        sync.triangle(53, 29, 75, 29, 64, 51, WHITE);
        sync.triangle(54, 30, 74, 30, 64, 50, WHITE);
        sync.triangle(55, 31, 73, 31, 64, 49, WHITE);
        sync.triangle(56, 32, 72, 32, 64, 48, WHITE);
        sync.triangle(57, 33, 71, 33, 64, 47, WHITE);
        sync.triangle(58, 34, 70, 34, 64, 46, WHITE);
        sync.triangle(59, 35, 69, 35, 64, 45, WHITE);
        sync.filled_rectangle(60, 36, 68, 44, WHITE);
        sync.line(114, 124, 124, 114, WHITE);
        sync.line(124, 124, 114, 114, WHITE);
        sync.line(112, 119, 126, 119, WHITE);
        sync.line(119, 112, 119, 126, WHITE);
        sync.filled_rectangle(61, 56, 67, 72, 0x4b2200);
        sync.filled_circle(64, 75, 6, 0x00a900);

        sync.locate(0,15);
        sync.textbackground_color(SKY_COLOR);
        char title[] = "Winter";
        sync.puts(title, sizeof(title));
        char title2[] = "Wonderland";
        sync.locate(0,14);
        sync.puts(title2, sizeof(title2));
    } else if (map == 3) {
        sync.triangle(54, 80, 74, 80, 64, 100, BLACK);
        sync.line(54, 80, 44, 60, BLACK);
        sync.line(74, 80, 84, 60, BLACK);
        sync.line(78, 80, 92, 80, BLACK);
        sync.line(85, 74, 85, 88, BLACK);
        sync.filled_circle(64, 25, 5, 0x0004ff);

        sync.locate(0,15);
        sync.textbackground_color(SKY_COLOR);
        char title[] = "The Grade I";
        sync.puts(title, sizeof(title));
        char title2[] = "Hope For";
        sync.locate(0,14);
        sync.puts(title2, sizeof(title2));
    }
    sync.locate(0,13);
    char power[] = "Power:";
    sync.puts(power, sizeof(power));
    sync.locate(6,13);
    char powerX[] = "50";
    sync.puts(powerX, sizeof(powerX));
    sync.textbackground_color(SKY_COLOR);
    sync.locate(0,12);
    char temp = type + '0';
    sync.putc(temp);

    sync.update();
    playSound("/sd/wavfiles/gamestart.wav");
}

// Initialize the game hardware.
// Call game_menu to find out which mode to play the game in (Single- or Multi-Player)
// Initialize the game synchronizer.
void game_init(void) {

    led1 = 0; led2 = 0; led3 = 0; led4 = 0;

    pb_u.mode(PullUp);
    pb_r.mode(PullUp);
    pb_d.mode(PullUp);
    pb_l.mode(PullUp);

    int mode = game_menu();
    sync.init(&uLCD, &acc, &pb_u, &pb_r, &pb_d, &pb_l, mode); // Connect to the other player.
    map_init();
}

// Display game over screen which lets us know who won.
// Play a cool sound!
void game_over() {
    sync.background_color(BLACK);
    sync.cls();
    char gameover[]= "Game Over!";
    sync.locate(2,7);
    sync.puts(gameover, sizeof(gameover));
    char win[]= "Winner:";
    sync.locate(2,6);
    sync.puts(win, sizeof(win));
    char playerW = winner + '0';
    sync.locate(9,6);
    sync.putc(playerW);
    sync.update();
    wait(3);
    playSound("/sd/wavfiles/gameover.wav");
}

void play_game(void) {
    int* p1_buttons;
    int* p2_buttons;

    float ax1, ay1, az1;
    float ax2, ay2, az2;
    game_init();

    // Create your tanks.
    Tank t1(4, 21, 12, 8, t1color);            // (min_x, min_y, width, height, color)
    Tank t2(111, 21, 12, 8, t2color);         // (min_x, min_y, width, height, color)

    // For each tank, create a bullet.
    Bullet b1(&t1, &t2);
    Bullet b2(&t2, &t1);

    int t1life = 3;
    int t2life = 3;
    int speed = 30;     // Sets the initial speed to thirty
    whose_turn = 0;     // Reset the turn to player one

    frame_timer.start();

    while(true) {

        // Get a pointer to the buttons for both sides.
        // From now on, you can access the buttons for player x using
        //
        // px_buttons[U_BUTTON]
        // px_buttons[R_BUTTON]
        // px_buttons[D_BUTTON]
        // px_buttons[L_BUTTON]

        p1_buttons = sync.get_p1_buttons();
        p2_buttons = sync.get_p2_buttons();

        led1 = p1_buttons[0] ^ p2_buttons[0];
        led2 = p1_buttons[1] ^ p2_buttons[1];
        led3 = p1_buttons[2] ^ p2_buttons[2];
        led4 = p1_buttons[3] ^ p2_buttons[3];

        // Get the accelerometer values.
        sync.get_p1_accel_data(&ax1, &ay1, &az1);
        sync.get_p2_accel_data(&ax2, &ay2, &az2);

        int player = whose_turn; // To keep track of if the turn changes

        if(whose_turn == PLAYER1) {
            if (!b1.in_flight) { // Only allow input if the bullet isn't in flight
                if(ax1 >  0.3) {
                    // Move the tank to the left if the accelerometer is tipped far enough to the left.
                    t1.reposition(-1, 0, 0);
                }
                if(ax1 <  -0.3) {
                    // Move the tank to the right if the accelerometer is tipped far enough to the right.
                    t1.reposition(+1, 0, 0);
                }
                if(ay1 >  0.3) {
                    // Move the cannon to the right if the accelerometer is tipped far enough to the right.
                    t1.reposition(0, 0, +0.05);
                }
                if(ay1 <  -0.3) {
                    // Move the cannon to the left if the accelerometer is tipped far enough to the left.
                    t1.reposition(0, 0, -0.05);
                }

                if(p1_buttons[D_BUTTON]) {
                    b1.shoot(speed, type);
                }

                if(p1_buttons[U_BUTTON]) {
                    if (speed == 70) {
                        speed = 30;
                    } else {
                        speed += 10;
                    }
                    sync.textbackground_color(SKY_COLOR);
                    sync.locate(6,13);
                    char temp = (speed / 10) + '0';
                    sync.putc(temp);
                    sync.update();
                    wait(0.25);
                }

                if(p1_buttons[R_BUTTON]) {
                    if (type == 2) {
                        type = 1;
                    } else {
                        type++;
                    }
                    sync.textbackground_color(SKY_COLOR);
                    sync.locate(0,12);
                    char temp = type + '0';
                    sync.putc(temp);
                    sync.update();
                    wait(0.25);
                }
            }
            float dt = frame_timer.read();
            int intersection_code = BULLET_NO_COLLISION;
            if (b1.in_flight) {
                intersection_code = b1.time_step(dt);
            }

            if(intersection_code != BULLET_NO_COLLISION || intersection_code == BULLET_OFF_SCREEN) {
                whose_turn = PLAYER2;
            }

            // If you shot yourself, you lost.
            if(sync.pixel_eq(intersection_code, sync.CONVERT_24_TO_16_BPP(t1.tank_color))) {
                t2kill++;
                t1life--;
                if (t1life == 0) {
                    winner = PLAYER2;
                    break;
                }
                t2.draw();
            }

            // If you shot the other guy, you won!
            if(sync.pixel_eq(intersection_code, sync.CONVERT_24_TO_16_BPP(t2.tank_color))) {
                t1kill++;
                t2life--;
                if (t2life == 0) {
                    winner = PLAYER1;
                    break;
                }
                t1.draw();
            }
        } else if(whose_turn == PLAYER2) {
            int mode = sync.play_mode;
            if(mode == 1){ // Input for two player mode
                if (!b1.in_flight) { // Only allow inout if the bullet isn't in flight
                    if(ax2 >  0.3) {
                        // Move the tank to the right if the accelerometer is tipped far enough to the right.
                        t2.reposition(-1, 0, 0);
                    }
                    if(ax2 <  -0.3) {
                        // Move the tank to the right if the accelerometer is tipped far enough to the right.
                        t2.reposition(+1, 0, 0);
                    }
                    if(ay2 >  0.3) {
                        // Move the tank to the right if the accelerometer is tipped far enough to the right.
                        t2.reposition(0, 0, +0.05);
                    }
                    if(ay2 <  -0.3) {
                    // Move the tank to the right if the accelerometer is tipped far enough to the right.
                        t2.reposition(0, 0, -0.05);
                    }

                    if(p2_buttons[D_BUTTON]) {
                        b2.shoot(speed, type);
                    }

                    if(p2_buttons[U_BUTTON]) {
                        if (speed == 70) {
                            speed = 30;
                        } else {
                            speed += 10;
                        }
                        sync.textbackground_color(SKY_COLOR);
                        sync.locate(6,13);
                        char temp = (speed / 10) + '0';
                        sync.putc(temp);
                        sync.update();
                        wait(0.25);
                    }

                    if(p2_buttons[R_BUTTON]) {
                        if (type == 2) {
                            type = 1;
                        } else {
                            type++;
                        }
                        sync.textbackground_color(SKY_COLOR);
                        sync.locate(0,12);
                        char temp = type + '0';
                        sync.putc(temp);
                        sync.update();
                        wait(0.25);
                    }
                }

                float dt = frame_timer.read();
                int intersection_code = BULLET_NO_COLLISION;
                if (b2.in_flight) {
                    intersection_code = b2.time_step(dt);
                }

                if(intersection_code != BULLET_NO_COLLISION || intersection_code == BULLET_OFF_SCREEN) {
                    whose_turn = PLAYER1;
                }

                if(sync.pixel_eq(intersection_code, sync.CONVERT_24_TO_16_BPP(t1.tank_color))) {
                    t2kill++;
                    t1life--;
                    if (t1life == 0) {
                        winner = PLAYER2;
                        break;
                    }
                    t1.draw();
                }

                if(sync.pixel_eq(intersection_code, sync.CONVERT_24_TO_16_BPP(t2.tank_color))) {
                    t1kill++;
                    t2life--;
                    if (t2life == 0) {
                        winner = PLAYER1;
                        break;
                    }
                    t2.draw();
                }
            } else {
                if (!b2.in_flight) { // Only allow input if the bullet isn't in flight
                    if(ax1 >  0.3) {
                        // Move the tank to the right if the accelerometer is tipped far enough to the right.
                        t2.reposition(-1, 0, 0);
                    }
                    if(ax1 <  -0.3) {
                        // Move the tank to the left if the accelerometer is tipped far enough to the left.
                        t2.reposition(+1, 0, 0);
                    }
                    if(ay1 >  0.3) {
                        // Move the cannon to the left if the accelerometer is tipped far enough to the left.
                        t2.reposition(0, 0, +0.05);
                    }
                    if(ay1 <  -0.3) {
                        // Move the cannon to the right if the accelerometer is tipped far enough to the right.
                        t2.reposition(0, 0, -0.05);
                    }

                    if(p1_buttons[D_BUTTON]) {
                        b2.shoot(speed, type);
                    }

                    if(p1_buttons[U_BUTTON]) {
                        if (speed == 70) {
                            speed = 30;
                        } else {
                            speed += 10;
                        }
                        sync.textbackground_color(SKY_COLOR);
                        sync.locate(6,13);
                        char temp = (speed / 10) + '0';
                        sync.putc(temp);
                        sync.update();
                        wait(0.25);
                    }

                    if(p1_buttons[R_BUTTON]) {
                        if (type == 2) {
                            type = 1;
                        } else {
                            type++;
                        }
                        sync.textbackground_color(SKY_COLOR);
                        sync.locate(0,12);
                        char temp = type + '0';
                        sync.putc(temp);
                        sync.update();
                        wait(0.25);
                    }
                }
                float dt = frame_timer.read();
                int intersection_code = BULLET_NO_COLLISION;
                if (b2.in_flight) {
                    intersection_code = b2.time_step(dt);
                }

                if(intersection_code != BULLET_NO_COLLISION || intersection_code == BULLET_OFF_SCREEN) {
                    whose_turn = PLAYER1;
                }

                if(sync.pixel_eq(intersection_code, sync.CONVERT_24_TO_16_BPP(t1.tank_color))) {
                    t2kill++;
                    t1life--;
                    if (t1life == 0) {
                        winner = PLAYER2;
                        break;
                    }
                    t1.draw();
                }

                if(sync.pixel_eq(intersection_code, sync.CONVERT_24_TO_16_BPP(t2.tank_color))) {
                    t1kill++;
                    t2life--;
                    if (t2life == 0) {
                        winner = PLAYER1;
                        break;
                    }
                    t2.draw();
                }
            }

        }

        frame_timer.reset();
        if (player != whose_turn) { // If it changes turns, update lives and the turn indicator on the RGB LED
            sync.filled_rectangle(70, 110, 110, 128, SKY_COLOR);
            for (int x = t1life; x > 0; x--) {
                sync.filled_circle(70 + x*10, 125, 3, t1.tank_color);
            }
            for (int x = t2life; x > 0; x--) {
                sync.filled_circle(70 + x*10, 115, 3, t2.tank_color);
            }
            if (whose_turn == 0) {
                rgb.write(TANK_RED);
            } else {
                rgb.write(TANK_BLUE);
            }
        }
        sync.update();

    }

    game_over(); // Displays the game over screen
}

int main (void) {
    while(true) {
        play_game();
    }
}
