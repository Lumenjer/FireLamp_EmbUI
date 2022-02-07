// (c) Kostyantyn Matviyevskyy aka kostyamat and Stepko
// 27.01.2022
// https://editor.soulmatelights.com/gallery/1684-pgm-player-with-resize
// License GPL v.3 as a part of the FireLamp_EmbUI project
#ifndef _RGBP_H
#define _RGBP_H

#include "config.h"
#include "main.h"
#include "lamp.h"

#ifdef RGB_PLAYER

#define MULTIPLIC 256

class RGBPlayer {
    private:
        uint8_t frameWidth, frameHeight, frames, frame = 0; 
        uint16_t maxSize; 
        int16_t corrX, corrY;
        uint16_t resizeX, resizeY;
        uint8_t* frameBuf = nullptr;
        uint8_t* data = nullptr;
        uint32_t timer = millis();
        bool done = false;
        bool codec332 = true;
        File rgbFile;

        void calc() {
            maxSize = max(WIDTH, HEIGHT);
            resizeX = ((float)frameWidth / maxSize) * MULTIPLIC;
            resizeY = ((float)frameHeight / maxSize) * MULTIPLIC;
            corrX = ((maxSize - WIDTH) / 2) * MULTIPLIC;
            corrY = ((maxSize - HEIGHT) / 2) * MULTIPLIC;
            frameBuf = new uint8_t[frameWidth * frameHeight];
        }

        void getFromPGM_332(uint8_t *data, int16_t frame) {
            for (uint16_t i = 0; i < frameWidth * frameHeight; i++) {
                uint32_t index = (frameWidth * frameHeight) * frame + i;
                frameBuf[i] = pgm_read_byte_far(data + index + 3);
            }
        }

        void getFromFile_332(uint8_t frame) {
            uint32_t index = (frameWidth * frameHeight) * frame + 3;
            rgbFile.seek(index, SeekSet);

            for(uint16_t i = 0; i < frameWidth * frameHeight; i++) {
                uint8_t data;
                rgbFile.read(&data, 1);
                frameBuf[i] = data;
            }
        }

        void drawFrame (bool codec) {
            for (uint16_t y = 0; y < (maxSize * MULTIPLIC); y+= resizeY) {
                for (uint16_t x = 0; x < (maxSize * MULTIPLIC); x+= resizeX) {
                    uint16_t index = ((x / MULTIPLIC * resizeX) / MULTIPLIC) + ((y/MULTIPLIC * resizeY) / MULTIPLIC) * frameWidth;
                    EffectMath::getPixel(((x - corrX) /MULTIPLIC), (HEIGHT- 1) - (y - corrY) / MULTIPLIC) = EffectMath::rgb332_To_CRGB(frameBuf[index]);
                }
            }
        }
    
    public:

        void load_PGM(uint8_t *data) {
            LOG(println, F("RGBPlayer: Start. PROGMEM mode."));
            frameWidth = pgm_read_byte(data);
            frameHeight = pgm_read_byte(data + 1);
            frames = pgm_read_byte(data + 2);
            LOG(printf_P, PSTR("RGBPlayer: Image loaded. It has %d frames. Image size %dX%d.\n"), frames, frameWidth, frameHeight);

            calc();
        }

        void load_FILE(String &filename) {
            codec332 = filename.indexOf(F("332")) > 0; 
            LOG(printf_P, PSTR("RGBPlayer: Start. File rgb%d mode."), (codec332 ? 332U: 565U));
            File rgbFile = LittleFS.open(filename, "r");
            if (rgbFile && rgbFile.isFile() && rgbFile.size() >= (3 + WIDTH*HEIGHT)) {
                rgbFile.read(&frameWidth, 1);
                rgbFile.read(&frameHeight, 1);
                rgbFile.read(&frames, 1);
                LOG(printf_P, PSTR("RGBPlayer: File %s loaded. It has %d frames. Image size %dX%d.\n"), filename.c_str(), frames, frameWidth, frameHeight);
            
                calc();
            } else {
                LOG(println, F("File not found or wrong format!"));
            }
        }

        void play332_PGM(uint8_t *data, uint8_t frameDelay) {
            if ((millis() - timer >= frameDelay) and done) {
                drawFrame(codec332);
                done = false;
                timer = millis();
                frame++;
                if (frame >= frames)
                    frame = 0;
            } else if ((millis() - timer < frameDelay) and !done) {
                getFromPGM_332(data, frame);
                done = true;
            }
        }

        

        void play_File(uint8_t frameDelay) {
            if ((millis() - timer >= frameDelay) and done) {
                drawFrame(codec332);
                FastLED.show();
                done = false;
                timer = millis();
                frame++;
                if (frame >= frames)
                    frame = 0;
            } else if ((millis() - timer < frameDelay) and !done) {
                getFromFile_332(frame);
                done = true;
            }
        }

        void stopPlayer() {
            if (rgbFile and rgbFile.isFile()) {
                rgbFile.close();
                LOG(println, F("RGBPlayer: Stop. File closed."));
            }
            delete [] frameBuf;
            LOG(println, F("RGBPlayer: Framebuffer destoyed."));
        }
};


#endif
#endif