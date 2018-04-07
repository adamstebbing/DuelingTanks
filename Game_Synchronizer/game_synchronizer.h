#ifndef GAME_SYNC_H__
#define GAME_SYNC_H__

#include <cmath>
#include "uLCD_4DGL.h"
#include "EthernetInterface.h"
#include "MMA8452.h"

#define PLAYER1 0
#define PLAYER2 1

#define SINGLE_PLAYER 0
#define MULTI_PLAYER  1

#define PLAYER1_IP      "192.168.2.1"
#define PLAYER2_IP      "192.168.2.2"
#define SERVER_PORT                7
#define ETH_PACKET_SIZE         1024

#define U_BUTTON 0
#define R_BUTTON 1
#define D_BUTTON 2
#define L_BUTTON 3


class Game_Synchronizer
{

public :

    int p1_p2;
    
    enum Color
    {
        NOP_CMD,
        CLS_CMD,
        BG_COLOR_CMD,
        LINE_CMD,
        CIRCLE_CMD,
        FILLED_CIRCLE_CMD,
        TRI_CMD,
        RECT_CMD,
        FILLED_RECT_CMD,
        PIX_CMD,
        LOCATE_CMD,
        PUTC_CMD,
        PUTS_CMD,
        TEXT_BACKGROUND_COLOR_CMD
    };

    Game_Synchronizer(bool player);
    
    void init(uLCD_4DGL*, MMA8452*, DigitalIn*, DigitalIn*, DigitalIn*, DigitalIn*, int);
    
    // Yes, this sucks. If you're smart, find a good way to do variable args and show me!
    // Look into template metaprogramming!
    
    void  draw(int CMD);
    void  draw(int CMD, int a);
    void  draw(int CMD, int a, int b);
    void  draw(int CMD, int a, int b, int c);
    void  draw(int CMD, int a, int b, int c, int d);
    void  draw(int CMD, int a, int b, int c, int d, int e);
    void  draw(int CMD, int a, int b, int c, int d, int e, int f);
    void  draw(int CMD, int a, int b, int c, int d, int e, int f, int g); 
    void _draw(int CMD, int a, int b, int c, int d, int e, int f, int g, char nArgs);
    
    void nop(void);
    void background_color(int color);
    void line(int sx, int sy, int ex, int ey, int color);
    void circle(int x , int y , int radius, int color);
    void filled_circle(int x , int y , int radius, int color);
    void triangle(int a, int b, int c, int d , int e, int f, int col);
    void rectangle(int a, int b, int c, int d, int col);
    void filled_rectangle(int a, int b, int c, int d, int col);
    void pixel(int a, int b, int col);
    void cls(void);
    void locate(int x, int y);
    void puts(char*, int);
    void putc(char);
    void textbackground_color(int col);
    
    //void BLIT(int x1, int y1, int x2, int y2, int *colors);       // I'll get to this one later.

    // Reads don't need to be done on the slave side. Hopefully both sides match!
    int  read_pixel(int x, int y);
    int  CONVERT_24_TO_16_BPP(int col_24);
    bool pixel_eq(int color1, int color2);
    
    void set_p1_inputs(void);
    void set_p2_inputs(void);
    
    int* get_p1_buttons();
    int* get_p2_buttons();
    
    void get_p1_accel_data(float*, float*, float*);
    void get_p2_accel_data(float*, float*, float*);
    
    void update(void);
    
    ~Game_Synchronizer();
      
    int play_mode;
    int buffer[ETH_PACKET_SIZE];
    int  buffer_idx;
    
    TCPSocketServer* server;
    TCPSocketConnection*  sock;
    EthernetInterface* eth;
    
    uLCD_4DGL* LCD;
    MMA8452* acc; 
    
    int p2_inputs[7];
    int p1_inputs[7];
    
    DigitalIn* pb_u;  // up button
    DigitalIn* pb_r;  // right button
    DigitalIn* pb_d;  // down button
    DigitalIn* pb_l;  // left button
};

#endif 