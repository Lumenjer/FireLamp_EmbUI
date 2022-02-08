// (c) Kostyantyn Matviyevskyy aka kostyamat and Stepko
// 27.01.2022
// https://editor.soulmatelights.com/gallery/1684-pgm-player-with-resize
// License GPL v.3 as a part of the FireLamp_EmbUI project
#ifndef _RGBP_H
#define _RGBP_H

#include "config.h"
#ifdef RGB_PLAYER
#include "main.h"
#include "lamp.h"

#define MULTIPLIC 256

class RGBPlayer {
    private:
        uint8_t frameWidth, frameHeight, frames, frame = 0; 
        uint16_t maxSize; 
        int16_t corrX, corrY;
        uint16_t resizeX, resizeY;
        uint8_t* frameBuf = nullptr;
        uint8_t* data = nullptr;
        uint8_t frameDelay = 20;
        uint32_t timer = millis();
        bool done = false;
        bool codec332 = true;
        File rgbFile;

        void calc();
        void getFromPGM_332(uint8_t *data, int16_t frame);
        void getFromFile_332(uint8_t frame);
        void getFromFile_565(uint8_t frame);
        void drawFrame ();
    
    public:
        RGBPlayer() {LOG(println, F("RGBPlayer was created."));};
        void load_PGM(uint8_t *data);
        void load_FILE(String &filename);
        void play332_PGM(uint8_t *data, uint8_t frameDelay);
        void play_File();
        void setFrameDelay(uint8_t value) {frameDelay = value;}
        void stopPlayer();
};

extern RGBPlayer animations;
#endif
#endif