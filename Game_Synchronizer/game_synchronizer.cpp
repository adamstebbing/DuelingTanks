#include "game_synchronizer.h"

extern Serial pc;

Game_Synchronizer::Game_Synchronizer(bool player) {
    p1_p2 = player;       
}

void Game_Synchronizer::init(uLCD_4DGL* lcd, MMA8452* accel, DigitalIn* up, DigitalIn* right, DigitalIn* down, DigitalIn* left, int mode) { 

    // Save a pointer to the local lcd.
    LCD = lcd;
    
    // Save a pointer to the local accelerometer.
    acc = accel;
    
    // Set up the local buttons.
    pb_u = up;
    pb_r = right;
    pb_d = down;
    pb_l = left;
    
    // Save the play mode. (Multi-player or Single-player)
    play_mode = mode;
    
    // Initialize the idx into the send_buffer to zero.
    buffer_idx = 0;
    
    // p2_inputs is the array containing player two's button (and accelerometer) values.
    // Initialize it to zero.
    memset(p1_inputs, 0, sizeof(p1_inputs));
    memset(p2_inputs, 0, sizeof(p2_inputs));
    
    switch (p1_p2) {
        case PLAYER1:                               // If I am p1...
        
            // If play_mode is set to multi-player, establish the ethernet connection.
            if(MULTI_PLAYER == play_mode) {
                eth = new EthernetInterface();
                eth->init(PLAYER1_IP, "255.255.255.0", "0.0.0.0"); 
                eth->connect();
                
                sock = new TCPSocketConnection();
                while(sock->connect(PLAYER2_IP, SERVER_PORT) < 0) {
                    //pc.printf("Trying to connect.\n");
                }
            }
            break;
        case PLAYER2:                               // If I am p2...
            
            // If I am player 2, play_mode doesn't matter. I have to assume it's 
            // set to multi-player and try to connect to p1.
            eth = new EthernetInterface();
            eth->init(PLAYER2_IP, "255.255.255.0", "0.0.0.0"); 
            eth->connect();

            server = new TCPSocketServer();
            server->bind(SERVER_PORT);
            server->listen();
            sock = new TCPSocketConnection();
            server->accept(*sock);
            sock->set_blocking(false, 1500);
            break;   
    }
}

// Yes, this sucks. If you're smart, find a good way to do variable args and show me!
// Look into templates!

void  Game_Synchronizer::draw(int CMD) {_draw(CMD, 0,0,0,0,0,0,0, 0); }
void  Game_Synchronizer::draw(int CMD, int a) { _draw(CMD, a, 0,0,0,0,0,0, 1); }
void  Game_Synchronizer::draw(int CMD, int a, int b) { _draw(CMD, a, b, 0,0,0,0,0, 2); }
void  Game_Synchronizer::draw(int CMD, int a, int b, int c) { _draw(CMD, a, b, c, 0,0,0,0, 3); }
void  Game_Synchronizer::draw(int CMD, int a, int b, int c, int d) { _draw(CMD, a, b, c, d, 0,0,0, 4); }
void  Game_Synchronizer::draw(int CMD, int a, int b, int c, int d, int e) { _draw(CMD, a, b, c, d, e, 0,0, 5); }
void  Game_Synchronizer::draw(int CMD, int a, int b, int c, int d, int e, int f) { _draw(CMD, a, b, c, d, e, f, 0, 6); }
void  Game_Synchronizer::draw(int CMD, int a, int b, int c, int d, int e, int f, int g) { _draw(CMD, a, b, c, d, e, f, g, 7); }

void Game_Synchronizer::_draw(int CMD, int a, int b, int c, int d, int e, int f, int g, char nArgs){
    
    // I haven't had time to deal with overflows of the buffer. If you are pushing tons of draw calls into the buffer,
    // you could overrun it. This will cause bad things. (At a minimum, your stuff won't be drawn.)
    // If you have this problem, try calling update in the middle of your draw calls to flush the buffer.
    // Alternatively, you can increase ETH_PACKET_SIZE.
    
    if(nArgs > 7) { 
        //pc.printf("Error in call to _draw(): nArgs > 7 not allowed!\n");
        return;
    }
    
    buffer[buffer_idx] = CMD;
    if(nArgs >= 1) buffer[buffer_idx+1] = a;
    if(nArgs >= 2) buffer[buffer_idx+2] = b;
    if(nArgs >= 3) buffer[buffer_idx+3] = c;
    if(nArgs >= 4) buffer[buffer_idx+4] = d;
    if(nArgs >= 5) buffer[buffer_idx+5] = e;
    if(nArgs >= 6) buffer[buffer_idx+6] = f;
    if(nArgs >= 7) buffer[buffer_idx+7] = g;
    // ERROR: nArgs > 7
    
    
    buffer_idx += nArgs+1;
}

void Game_Synchronizer::puts(char* str, int strlen) {
    buffer[buffer_idx] = PUTS_CMD;
    buffer[buffer_idx+1] = strlen;
    for(int i = 0; i < strlen; i++) {
        buffer[i+buffer_idx+2] = str[i];
        
    }
    buffer_idx += strlen + 2;
}

void Game_Synchronizer::nop(void)                                                    { draw(NOP_CMD); }
void Game_Synchronizer::background_color(int color)                                  { draw(BG_COLOR_CMD, color); }
void Game_Synchronizer::line(int sx, int sy, int ex, int ey, int color)              { draw(LINE_CMD, sx, sy, ex, ey, color); }
void Game_Synchronizer::circle(int x , int y , int radius, int color)                { draw(CIRCLE_CMD, x, y, radius, color); }
void Game_Synchronizer::filled_circle(int x , int y , int radius, int color)         { draw(FILLED_CIRCLE_CMD, x, y, radius, color); }
void Game_Synchronizer::triangle(int a, int b, int c, int d , int e, int f, int col) { draw(TRI_CMD, a, b, c, d, e, f, col); }
void Game_Synchronizer::rectangle(int a, int b, int c, int d, int col)               { draw(RECT_CMD, a, b, c, d, col); }
void Game_Synchronizer::filled_rectangle(int a, int b, int c, int d, int col)        { draw(FILLED_RECT_CMD, a, b, c, d, col); }
void Game_Synchronizer::pixel(int a, int b, int col)                                 { draw(PIX_CMD, a, b, col); }
void Game_Synchronizer::cls(void)                                                    { draw(CLS_CMD); }
void Game_Synchronizer::locate(int x, int y)                                         { draw(LOCATE_CMD, x, y); }
void Game_Synchronizer::putc(char a)                                                 { draw(PUTC_CMD, (int)a); }
void Game_Synchronizer::textbackground_color(int col)                                { draw(TEXT_BACKGROUND_COLOR_CMD, col); }

// Returns the 16bpp color of the screen at location (x, y)
int  Game_Synchronizer::read_pixel(int x, int y) { 
    return LCD->read_pixel(x, 128-y);
}

// Convert 24bpp colors to 16bpp colors
int Game_Synchronizer::CONVERT_24_TO_16_BPP(int col_24) {
    int b = col_24 & 0xFF;
    int g = (col_24 >> 8) & 0xFF;
    int r = (col_24 >> 16)& 0xFF;
    
    r >>= 3;
    g >>= 2;
    b >>= 3;
    
    return r<<11 | g<<5 | b;
}

// Compare two colors (16bpp or 24bpp)
bool Game_Synchronizer::pixel_eq(int color1, int color2) {
    return (CONVERT_24_TO_16_BPP(color1) == color2 || 
            CONVERT_24_TO_16_BPP(color2) == color1 ||
            color1 == color2);
}

// This allows the Player 2 code to set its inputs.
// Accelerometer values are stored in 16:16 fixed point representation.
void Game_Synchronizer::set_p2_inputs() { 
    if(p1_p2 == PLAYER1) { return; }
    
    p2_inputs[0] = !(*pb_u);
    p2_inputs[1] = !(*pb_r);
    p2_inputs[2] = !(*pb_d);
    p2_inputs[3] = !(*pb_l);

    double acc_x, acc_y, acc_z;
    acc->readXYZGravity(&acc_x,&acc_y,&acc_z);
    
    p2_inputs[4] = static_cast<int>(acc_x * 65536);
    p2_inputs[5] = static_cast<int>(acc_y * 65536);
    p2_inputs[6] = static_cast<int>(acc_z * 65536);
}

// Use this to update Player 1's inputs.
// Accelerometer values are stored in 16:16 fixed point representation.
void Game_Synchronizer::set_p1_inputs() {
    if(p1_p2 == PLAYER2) { return; }
    
    p1_inputs[0] = !(*pb_u);
    p1_inputs[1] = !(*pb_r);
    p1_inputs[2] = !(*pb_d);
    p1_inputs[3] = !(*pb_l);

    double acc_x, acc_y, acc_z;
    acc->readXYZGravity(&acc_x,&acc_y,&acc_z);
    
    p1_inputs[4] = (int)(acc_x * 65536);
    p1_inputs[5] = (int)(acc_y * 65536);
    p1_inputs[6] = (int)(acc_z * 65536);
}


void Game_Synchronizer::get_p1_accel_data(float* ax, float* ay, float* az) {
    *ax = (float) p1_inputs[4] / 65536.0;
    *ay = (float) p1_inputs[5] / 65536.0;
    *az = (float) p1_inputs[6] / 65536.0;
}

void Game_Synchronizer::get_p2_accel_data(float* ax, float* ay, float* az) {
    *ax = (float) p2_inputs[4] / 65536.0;
    *ay = (float) p2_inputs[5] / 65536.0;
    *az = (float) p2_inputs[6] / 65536.0;
}

int* Game_Synchronizer::get_p1_buttons() {
    return p1_inputs;
}

int* Game_Synchronizer::get_p2_buttons() {
    return p2_inputs;
}

void Game_Synchronizer::update() {
    
    nop();
    set_p1_inputs();
    
    int buffer_size = buffer_idx-1;
    buffer_idx = 0;
    
    if(p1_p2 == PLAYER1 && MULTI_PLAYER == play_mode) {
        sock->set_blocking(true, 100);
        sock->send_all((char*)buffer, (buffer_size+1)*sizeof(buffer[0]));
        
        int n = sock->receive((char*)p2_inputs, sizeof(p2_inputs));
        //if(n < 0) {pc.printf("RECEIVE ERROR.\n");}          
    
    }else if(p1_p2 == PLAYER2) {    
        sock->set_blocking(true, 100);
        int n = sock->receive((char*)buffer, sizeof(buffer));  
        //if(n < 0) {pc.printf("RECEIVE ERROR.\n");}
        buffer[n] = '\0';   
        buffer_size = n/sizeof(buffer[0]) - 1;
        
        sock->send_all((char*)p2_inputs, sizeof(p2_inputs));          
    }
    
    /*
    for(int i=0; i<buffer_size; i++) {
       pc.printf("%d %c, ", buffer[i]); 
    }
    pc.printf("\n\n");*/
        
    int idx = 0;
    while(idx < buffer_size) {
        char cmd = buffer[idx];
        idx++;
        
        int str_length;
        char* str;
        //pc.printf("CMD: %d\n", cmd);
        switch(cmd) {
            case NOP_CMD:
                break;
            case CLS_CMD:
                LCD->cls();
                //pc.printf("%d Clear the screen!\n",idx);
                break;
            case BG_COLOR_CMD:
                LCD->background_color(buffer[idx]);
                //pc.printf("Change the background to 0x%X\n", buffer[idx]);
                idx += 1;
                break;
            case LINE_CMD:
                //pc.printf("LINE: (%d, %d) - (%d, %d) COLOR: 0x%X\n", buffer[idx], buffer[idx+1], buffer[idx+2], buffer[idx+3], buffer[idx+4]);
                LCD->line(buffer[idx], 128-buffer[idx+1], buffer[idx+2], 128-buffer[idx+3], buffer[idx+4]);
                idx += 5;
                break;
            case CIRCLE_CMD:
                //pc.printf("CIRCLE: (%d, %d), r=%d\n", buffer[idx], buffer[idx+1], buffer[idx+2]);
                LCD->circle(buffer[idx], 128-buffer[idx+1], buffer[idx+2], buffer[idx+3]);
                idx += 4;
                break;
            case FILLED_CIRCLE_CMD:
                //pc.printf("CIRCLE: (%d, %d), r=%d\n", buffer[idx], buffer[idx+1], buffer[idx+2]);
                LCD->filled_circle(buffer[idx], 128-buffer[idx+1], buffer[idx+2], buffer[idx+3]);
                idx += 4;
                break;
            case TRI_CMD:
                //pc.printf("CIRCLE: (%d, %d), r=%d\n", buffer[idx], buffer[idx+1], buffer[idx+2]);
                LCD->triangle(buffer[idx], 128-buffer[idx+1], buffer[idx+2], 128-buffer[idx+3], buffer[idx+4], 128-buffer[idx+5], buffer[idx+6]);
                idx += 7;
                break;
            case RECT_CMD:
                LCD->rectangle(buffer[idx], 128-buffer[idx+1], buffer[idx+2], 128-buffer[idx+3], buffer[idx+4]);
                idx += 5;
                break;
            case FILLED_RECT_CMD:
                LCD->filled_rectangle(buffer[idx], 128-buffer[idx+1], buffer[idx+2], 128-buffer[idx+3], buffer[idx+4]);
                idx += 5;
                break;
            case PIX_CMD:
                LCD->pixel(buffer[idx], 128-buffer[idx+1], buffer[idx+2]);
                idx += 3;
                break;
            case PUTS_CMD:
                str_length = buffer[idx];
                str = new char[str_length];
                for(int i=0; i<str_length; i++) {
                    str[i] = (char)buffer[idx+1+i];
                }
                LCD->puts(str);
                idx += str_length+1;
                break;
            case LOCATE_CMD:
                LCD->locate(buffer[idx], 15-buffer[idx+1]);
                idx += 2;
                break;
            case PUTC_CMD:
                LCD->putc((char)buffer[idx]);
                idx += 1;
            case TEXT_BACKGROUND_COLOR_CMD:
                LCD->textbackground_color(buffer[idx]);
                idx += 1;
            default:
                //pc.printf("UNKNOWN CMD %d: This could get ugly!\n", cmd);
                idx += 0;
                break;
        }
    }
}

Game_Synchronizer::~Game_Synchronizer() {            
    sock->close();
    eth->disconnect();  
    delete sock;
    delete server;
    delete eth; 
    delete pb_u;
    delete pb_r;
    delete pb_d;
    delete pb_l;
}
