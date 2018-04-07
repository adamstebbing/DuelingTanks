#include "tank.h"
#include "globals.h"
#include "math.h"
#include "game_synchronizer.h"
#include "wave_player.h"
#include "playSound.h"

extern Game_Synchronizer sync;
 
// sx is the x-coord of the bottom left corner of the tank
// sy is the y-coord of the same corner
// width is the width of the tank
// height is the height of the tank
Tank::Tank(int sx, int sy, int width, int height, int color) {
    x = sx; y = sy; w = width; h = height;
    tank_color = color;
    barrel_theta = PI/4.0;
    barrel_length = w;
    wheel_rad = 2.0;
    draw();
}

// Return the minimum x-coord of your tank's bounding box.
int Tank::min_x(void) { 
    return x - w/2;
}    

// Return the minimum y-coord of your tank's bounding box.
int Tank::min_y(void) {
    return y;
}

// Return the maximum x-coord of your tank's bounding box.
int Tank::max_x(void) { 
    return x + 3*w/2;
}

// Return the maximum y-coord of your tank's bounding box.
int Tank::max_y(void) {  
    return y + h + w;
}

void Tank::barrel_end(int* bx, int* by) {
    *bx = x + w/2.0 + barrel_length*cos(barrel_theta);
    *by = y + h + wheel_rad + barrel_length*sin(barrel_theta);
}

void Tank::reposition(int dx, int dy, float dtheta) {
    float barrel_temp = barrel_theta + dtheta;
    if (barrel_temp < 0 || barrel_temp > PI) { return; }
    if (dx > 0) {
        int pixel = sync.read_pixel(x+w+dx, y+h);
        if (!sync.pixel_eq(pixel, SKY_COLOR) || x+w+dx > 127 || x+dx < 0 || y+h > 127 || y+h < 0) {
            return;
        }
    }
    if (dx < 0) {
        int pixel = sync.read_pixel(x+dx, y+h);
        if (!sync.pixel_eq(pixel, SKY_COLOR) || x+w+dx > 127 || x+dx < 0 || y+h > 127 || y+h < 0) {
            return;
        }
    }
    erase();
    x += dx; y += dy; barrel_theta += dtheta;
    draw();
}


void Tank::draw() {
    sync.line(x + w/2.0, y+h+wheel_rad, x + w/2.0 + barrel_length*cos(barrel_theta), y+h+wheel_rad + barrel_length*sin(barrel_theta), tank_color);
    sync.filled_rectangle(x, y+wheel_rad, x+w, y+h+wheel_rad, tank_color);
    sync.filled_circle(x+wheel_rad, y+wheel_rad, wheel_rad, tank_color);
    sync.filled_circle(x+w-wheel_rad, y+wheel_rad, wheel_rad, tank_color);
}     

void Tank::erase() { 
    sync.line(x + w/2.0, y+h+wheel_rad, x + w/2.0 + barrel_length*cos(barrel_theta), y+h+wheel_rad + barrel_length*sin(barrel_theta), SKY_COLOR);
    sync.filled_rectangle(x, y+wheel_rad, x+w, y+h+wheel_rad, SKY_COLOR);
    sync.filled_circle(x+wheel_rad, y+wheel_rad, wheel_rad, SKY_COLOR);
    sync.filled_circle(x+w-wheel_rad, y+wheel_rad, wheel_rad, SKY_COLOR);
}     