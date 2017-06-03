#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <time.h>
#include <windows.h>


int goombaCoords[2][5] = {{}, {256, 288, 320, 352, 384}};
int shellCoords[2][5] = {{}, {64, 96, 128, 160, 192}};
int bulletCoords[2][2] = {{0, 0}, {256, 288}};
int bowserCoordinates[2][4] = {{0, 576, 0, 576}, {352, 320, 288, 256}};
int fireCoords[3][4] = {{32, 576, 32, 576}, {352, 320, 288, 256}, {1, -1, 1, -1}};
int animation_num = 0;
int gMove[5] = {32,32,32,32,32};
int sMove[5] = {32,32,32,32,32};
int gspeed = 20, sspeed = 6, fspeed = 3;
int level = 1;
const char *level_text[4] = {"Level 1", "Level 2", "Level 3", "Level 4"};
bool endGame = false;

using namespace std;

struct Player { int coordinates[2]; };

void goombaMove() {
    for (int startRow = (level == 3 ? 2 : 0); startRow < 5; startRow++) {
        if (goombaCoords[0][startRow] == 576) {  gMove[startRow] = -32; }
        if (goombaCoords[0][startRow] == 32) { gMove[startRow] = 32; }
        goombaCoords[0][startRow] += gMove[startRow];
    }
}

void shellMove() {
    if (level < 4) {
        for (int row = 0; row < 5; row++) {
            if (shellCoords[0][row] == 576) {  sMove[row] = -32; }
            if (shellCoords[0][row] == 32) { sMove[row] = 32; }
            shellCoords[0][row] += sMove[row];
        }
    }
}

void bulletMove(int bullet_num) {
    bulletCoords[0][bullet_num] += 32;
    if (bulletCoords[0][bullet_num] == 576) {
        bulletCoords[0][bullet_num] = 0;
    }
}

void fireballMove(int fireball_num) {
    fireCoords[0][fireball_num] += 32*fireCoords[2][fireball_num];

    if (fireCoords[0][fireball_num] > 544) {
        fireCoords[0][fireball_num] = 32;
        fireCoords[1][0] = 352; //TODO figure out why this is screwing up
    }
    else if (fireCoords[0][fireball_num] < 32) {
        fireCoords[0][fireball_num] = 544;
    }

    animation_num++;
    if (animation_num > 3) {
        animation_num = 0;
    }
}

void drawImage(ALLEGRO_BITMAP *bmp, int x, int y, int flags, ALLEGRO_DISPLAY *display) {
    al_set_target_bitmap(al_get_backbuffer(display));
    al_draw_bitmap(bmp, x, y, flags);
}

void checkCollisions(Player player) {
    if (level < 4) {
        for (int x = 0; x < 5; x++) {
            if (player.coordinates[0] == shellCoords[0][x] && player.coordinates[1] == shellCoords[1][x]) {
                endGame = true;
            }
            if (player.coordinates[0] == bulletCoords[0][x] && player.coordinates[1] == bulletCoords[1][x]) {
                endGame = true;
            }
        }
        for (int x = (level == 3 ? 2 : 0); x < 5; x++) {
            if (player.coordinates[0] == goombaCoords[0][x] && player.coordinates[1] == goombaCoords[1][x]) {
                endGame = true;
            }
        }
    }
    if (level == 4) {
        for (int x = 0; x < 4; x++) {
            if (player.coordinates[0] == fireCoords[0][x] && player.coordinates[1] == fireCoords[1][x]) {
                endGame = true;
            }
        }
    }
}

int w = 0;
const float FPS = 60; // for timer

int main()
{
    // initialize allegro
    al_init();
    al_init_image_addon();
    al_install_keyboard();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(1);
    al_init_font_addon();
    al_init_ttf_addon();

    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *event_queue;
    ALLEGRO_SAMPLE *music;
    ALLEGRO_BITMAP *background;
    ALLEGRO_BITMAP *character;

    ALLEGRO_BITMAP *goombas[5];
    ALLEGRO_BITMAP *shells [5];

    ALLEGRO_BITMAP *shooters[2];
    ALLEGRO_BITMAP *receivers[2];
    ALLEGRO_BITMAP *bullets[2];

    ALLEGRO_BITMAP *bowsers[4];
    ALLEGRO_BITMAP *fireballs[4][4];
    ALLEGRO_BITMAP *banana1 = NULL;
    ALLEGRO_BITMAP *peach = NULL;

    ALLEGRO_FONT *font = al_load_ttf_font("Resources/pirulen.ttf", 30, 0);
    ALLEGRO_EVENT ev;

    display = al_create_display(640, 480);
    background = al_load_bitmap("Resources/game board.jpg");
    character = al_load_bitmap("Resources/character.png");
    music = al_load_sample("Resources/smb_ground_theme.wav");
    if (!music) {
        fprintf(stderr, "failed to load music");
    }
    else {
        al_play_sample(music, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
    }

    // load goombas
    for (int i = 0; i < 5; i++) {
        goombas[i] = al_load_bitmap("Resources/goomba.png");
    }

    //load shooters, receivers, and bullets
    for (int i = 0; i < 2; i++) {
        shooters[i] = al_load_bitmap("Resources/boom shooter right.png");
        receivers[i] = al_load_bitmap("Resources/boom shooter left.png");
        bullets[i] = al_load_bitmap("Resources/bomb right.png");
    }

    //load bowsers and fireballs
    for (int i = 0; i < 4; i++) {
        bowsers[i] = al_load_bitmap("Resources/bowser64.png");
        fireballs[i][0] = al_load_bitmap("Resources/fireball1.png");
        fireballs[i][1] = al_load_bitmap("Resources/fireball2.png");
        fireballs[i][2] = al_load_bitmap("Resources/fireball3.png");
        fireballs[i][3] = al_load_bitmap("Resources/fireball4.png");
    }

    banana1 = al_load_bitmap("Resources/banana.png");
    peach = al_load_bitmap("Resources/peach.png");

    // initialize player as mario and set initial coordinates
    Player mario;
    mario.coordinates[0] = 320;
    mario.coordinates[1] = 416;

    event_queue = al_create_event_queue();

    //PlaySound("smb_ground_theme.wav",NULL,SND_FILENAME|SND_ASYNC);


    for (; level < 5; level++) {

        srand(time(NULL));
        endGame = false;

        ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);

        // register various potential events
        al_register_event_source(event_queue, al_get_keyboard_event_source());
        al_register_event_source(event_queue, al_get_display_event_source(display));
        al_register_event_source(event_queue, al_get_timer_event_source(timer));


        // determine what shell image to use based on level
        const char *shellImage;
        switch(level) {
            case 2:
                shellImage = "Resources/shell red.png";
                break;
            case 3:
                shellImage = "Resources/shell blue.png";
                break;
            default:
                shellImage = "Resources/shell.png";
                break;
        }

        // load shells
        for (int i = 0; i < 5; i++) {
            shells[i] = al_load_bitmap(shellImage);
        }

        //reset mario coordinates
        mario.coordinates[0] = 320;
        mario.coordinates[1] = 416;

        // draws background
        al_set_target_bitmap(al_get_backbuffer(display));
        al_draw_bitmap(background,0,0,0);

        
        al_draw_text(font, al_map_rgb(255, 255, 255), 320, 160, ALLEGRO_ALIGN_CENTRE, level_text[level - 1]);
        al_flip_display();
        al_rest(1.5);

        
        if (level < 4) {
            // initialize goomba coordinates
            for (int goonum = 0; goonum < 5; goonum++) {
                int gx = (rand() % 18)*32 + 32;
                goombaCoords[0][goonum] = gx;
            }

            // initialize shell coordinates
            for (int shellnum = 0; shellnum < 5; shellnum++) {
                int sx = (rand() % 18) * 32 + 32;
                shellCoords[0][shellnum] = sx;
            }
        }

        
        // draws character at set coordinates
        al_set_target_bitmap(character);
        al_set_target_bitmap(al_get_backbuffer(display));
        al_draw_bitmap(character,mario.coordinates[0],mario.coordinates[1],0);
        al_flip_display(); // replaces screen with backbuffer
        

        al_start_timer(timer);
        do
        {
            al_wait_for_event(event_queue, &ev);
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_UP && mario.coordinates[1] > 32) {
                    mario.coordinates[1] -= 32;
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT && mario.coordinates[0] > 32) {
                    mario.coordinates[0] -= 32;
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT && mario.coordinates[0] < 576) {
                    mario.coordinates[0] += 32;
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN && mario.coordinates[1] < 416) {
                    mario.coordinates[1] += 32;
                }
            }

            // moves stuff based on given speed
            int time = al_get_timer_count(timer);
            if (level != 4) {
                if (time % gspeed == 0) { goombaMove(); }
                if (time % sspeed == 0) { shellMove(); }
            }

            if (level == 3) {
                if (time % 5 == 0) { bulletMove(0); }
                if (time % 6 == 0) { bulletMove(1); }
            }

            if (level == 4 && (time % fspeed == 0)) {
                for (int i = 0; i < 5; i++) { fireballMove(i); }
            }

            //draw background and player
            drawImage(background, 0, 0, 0, display);
            drawImage(character, mario.coordinates[0], mario.coordinates[1], 0, display);

            // redraw other stuff
            if (level < 4) {
                //draw Goombas
                //start at 0 for level 1 and 2, else 2
                for (int i = ((level == 1 || level == 2) ? 0 : 2); i < 5; i++) {
                    drawImage(goombas[i], goombaCoords[0][i], goombaCoords[1][i], 0, display);
                }

                //draw shells
                for (int i = 0; i < 5; i++) {
                    drawImage(shells[i], shellCoords[0][i], shellCoords[1][i], 0, display);
                }

                if (level == 3) {
                    //draw shooters and receivers
                    drawImage(shooters[0], 0, 256, 0, display);
                    drawImage(shooters[1], 0, 288, 0, display);
                    drawImage(receivers[0], 608, 256, 0, display);
                    drawImage(receivers[1], 608, 288, 0, display);
                    //draw bullets
                    for (int i = 0; i < 2; i++) {
                        drawImage(bullets[i], bulletCoords[0][i], bulletCoords[1][i], 0, display);
                    }
                }
            }

            if (level == 4) {
                for(int i = 0; i < 4; i++) {
                    int flip = (i % 2 != 0) ? 0 : ALLEGRO_FLIP_HORIZONTAL;
                    //draws Bowsers
                    drawImage(bowsers[i], bowserCoordinates[0][i], bowserCoordinates[1][i], flip, display);
                    //draws fireballs with animation
                    drawImage(fireballs[i][animation_num], fireCoords[0][i], fireCoords[1][i], 0, display);
                }

                drawImage(banana1, 320, 128, 0, display);
                drawImage(peach, 320, 32, 0, display);
            }

            al_flip_display();

            //checks for collisions
            checkCollisions(mario);

        }
        while (ev.type != ALLEGRO_EVENT_DISPLAY_CLOSE && mario.coordinates[1] != 32 && endGame == false);

        al_stop_timer(timer);
        drawImage(background, 0, 0, 0, display);
        al_rest(0.5);

        if (mario.coordinates[1] == 32) {
            const char *message = (level == 4) ? "You win!" : "Well done!";
            al_draw_text(font, al_map_rgb(255, 255, 255), 320, 160, ALLEGRO_ALIGN_CENTRE, message);
        }
        else {
            al_draw_text(font, al_map_rgb(255, 255, 255), 320, 160, ALLEGRO_ALIGN_CENTRE, "You Lose!");
            al_flip_display();
            al_rest(2.0);
            break;
        }

        al_flip_display();
        al_rest(2.0);

        //speed up shells with every consecutive level
        if (level < 3) {
            sspeed -= 2;
        }
    } // <-- ends for loop for iterating through levels

    return 0;
}
