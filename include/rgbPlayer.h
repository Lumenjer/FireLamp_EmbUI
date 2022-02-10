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
        uint8_t frameDelay;
        bool codec332 = true;
        File rgbFile;
        uint8_t bufSize = 0;

        void calc();
        void getFromFile_332(uint8_t frame);
        void getFromFile_565(uint8_t frame);
        void drawFrame ();
    
    public:
        RGBPlayer() {};
        void begin() {
            String tmp = PSTR("//animations/Candle.332");
            loadFile(tmp);
            frameDelay = 50;
            LOG(println, F("RGBPlayer: Setup done."));
        }

        void loadFile(String &filename);
        void playFile(bool show);
        void setFrameDelay(uint8_t value) {frameDelay = value;}
        uint8_t getFrameDelay() {return frameDelay;}
        void startPlayer() {calc();}
        void stopPlayer();
};

extern RGBPlayer animations;
#endif
#endif