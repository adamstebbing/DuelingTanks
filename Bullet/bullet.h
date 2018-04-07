#ifndef BULLET_H__
#define BULLET_H__

#include "tank.h"

#define BULLET_NO_COLLISION -1
#define BULLET_OFF_SCREEN   -2

class Bullet{
    public:
        
        // (x0, y0) is the position of the bullet at the start of its trajectory.
        int x0, y0;
        float theta;
        
        // The components of the bullet's initial velocity.
        float vx0, vy0;
        
        // The current position of the bullet.
        int x, y;
        
        // How fast is the bullet?
        int speed;
        
        // Keep track of the time since the bullet was shot.
        float time;
        
        // Keeps track of the type of bullet: 1 = regular, 2 = bounce, 3 = roll
        int type;
        
        
        // Keep track of whether the bullet is currently in flight.
        // If it is in_flight, update its position. Otherwise, ignore it.
        // Set in_flight when the bullet is shot.
        // Reset in_flight when the bullet hits something.
        bool in_flight;
        
        // Keep a pointer to the tank that shot this bullet. You'll need
        // it to access information about the tank.
        Tank* tank;
        Tank* tankOther;
        
        // Create a bullet!
        Bullet(Tank* t, Tank* t2);
        
        // Shoot the bullet!
        void shoot(int speedIn, int typeIn);
        
        void explode(void);
        
        // given a time delta, calculate the new (x, y) coords of the bullet.
        void update_position(float dt);
        
        // Once per frame, plug in the time since the last frame.
        // Clear the old bullet and redraw it in the new place.
        // Do collision detection here. Return a code describing what has
        // (or hasn't) been hit.
        int time_step(float dt);    
};

#endif