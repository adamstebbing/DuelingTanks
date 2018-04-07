#include "uLCD_4DGL.h"
#include "bullet.h"
#include "game_synchronizer.h"
#include "globals.h"
#include "math.h"
#include "wave_player.h"
#include "playSound.h"

extern Game_Synchronizer sync;
extern Serial pc;

// Initialize the bullet. Don't have to do much here.
// Keep a pointer to this bullet's tank.
// Set the speed, and default the bullet to not in_flight.
Bullet::Bullet(Tank* t, Tank* t2) {
    tank = t;
    tankOther = t2;
    in_flight = false;
}

// If in_flight, do nothing. Otherwise,
// set the in_flight flag, and initialize values needed for
// the trajectory calculations. (x0, y0), (vx0, vy0), time
// Hint: tank->barrel_end(...) is useful here.
void Bullet::shoot(int speedIn, int typeIn) {
    if (in_flight == true) {
        return;
    }
    pc.printf("Bullet is live.\n");
    in_flight = true;
    tank->barrel_end(&x0, &y0);
    speed = speedIn;
    type = typeIn;
    theta = tank->barrel_theta;
    x = x0;
    y = y0;
    vx0 = (float)speed*cos(theta);
    vy0 = (float)speed*sin(theta);
    time = 0;
}

void Bullet::explode(void) {
    in_flight = false;
    for (int t = 0; t < 3; t++) {
        sync.filled_circle(x,y,t,0xffa500);
        sync.update();
        wait(0.1);
    }
    sync.filled_circle(x,y,3,SKY_COLOR);
    sync.update();
    playSound("/sd/wavfiles/explosion.wav");
}

// If the bullet is in flight, calculate its new position
// after a time delta dt.
void Bullet::update_position(float dt) {
    if(dt > 0.05) {
        dt = 0.05;
    }
    time += dt;
    x = floor(x0 + vx0*time);
    y = floor(y0 + vy0*time - 0.5*9.8*time*time);
}

int Bullet::time_step(float dt) {
    // If the bullet hasn't hit anything, 
    // redraw the bullet at its new location. 
    // If it has hit something (obstacle, tank, edge of the screen), 
    // set the in_flight flag back to false, explode the nearby area,
    // and return one of the following codes.
    //
    // return codes:
    //      BULLET_NO_COLLISION: no collision
    //      BULLET_OFF_SCREEN:   off the side of the screen
    //      Otherwise, return the color you've hit in 16bpp format.
    sync.circle(x,y,0,SKY_COLOR);
    sync.update();
    update_position(dt); 
    int pixel = sync.read_pixel(x, y);
    if (x > 127 || x < 0 || y > 127 || y < 0) { // If the bullet goes out of bounds
        in_flight = false;
        sync.filled_circle(x,y,2,SKY_COLOR);
        sync.update();
        playSound("/sd/wavfiles/explosion.wav");
        return BULLET_OFF_SCREEN;
    }
    if (!sync.pixel_eq(pixel, SKY_COLOR)) {
        if (type == 1) { // If its a normal bullet it explodes
            explode();
            pixel = sync.CONVERT_24_TO_16_BPP(pixel);
            return pixel;
        } else if (type == 2) { // If not it bounces if its not the ground or a tank
            if (!sync.pixel_eq(pixel, GND_COLOR) && !sync.pixel_eq(pixel, tank->tank_color) && !sync.pixel_eq(pixel, tankOther->tank_color)) {
                playSound("/sd/wavfiles/bounce.wav");
                x0 = x;
                y0 = y;
                vy0 = vy0*time;
                vx0 = -vx0;
                time = 0;
            } else {
                explode();
                pixel = sync.CONVERT_24_TO_16_BPP(pixel);
                return pixel;
            }
        }
    }
    sync.circle(x,y,0,BLACK);
    return BULLET_NO_COLLISION;
}