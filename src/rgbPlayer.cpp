// (c) Kostyantyn Matviyevskyy aka kostyamat and Stepko
// 27.01.2022
// https://editor.soulmatelights.com/gallery/1684-pgm-player-with-resize
// License GPL v.3 as a part of the FireLamp_EmbUI project

#include "config.h"
#ifdef RGB_PLAYER
#include "rgbPlayer.h"
#include "main.h"
#include "lamp.h"
#include "effectmath.h"

//RGBPlayer animations;

void RGBPlayer::calc() {
    maxSize = max(WIDTH, HEIGHT);
    resizeX = ((float)frameWidth / maxSize) * MULTIPLIC;
    resizeY = ((float)frameHeight / maxSize) * MULTIPLIC;
    corrX = ((maxSize - WIDTH) / 2) * MULTIPLIC;
    corrY = ((maxSize - HEIGHT) / 2) * MULTIPLIC;
    if (frameBuf) delete [] frameBuf;
    frameBuf = new uint8_t[frameWidth * frameHeight * (codec332 ? 1 : 2)];
}

void RGBPlayer::getFromPGM_332(uint8_t *data, int16_t frame) {
    for (uint16_t i = 0; i < frameWidth * frameHeight; i++) {
        uint32_t index = (frameWidth * frameHeight) * frame + i;
        frameBuf[i] = pgm_read_byte_far(data + index + 3);
    }
}

void RGBPlayer::getFromFile_332(uint8_t frame) {
    uint32_t index = (frameWidth * frameHeight) * frame + 3;
    rgbFile.seek(index, SeekSet);

    for(uint16_t i = 0; i < frameWidth * frameHeight; i++) {
        uint8_t data;
        rgbFile.read(&data, 1);
        frameBuf[i] = data;
    }
}

void RGBPlayer::getFromFile_565(uint8_t frame) {
    uint32_t index = (frameWidth * frameHeight) * frame + 3;
    rgbFile.seek(index, SeekSet);

    for(uint16_t i = 0; i < frameWidth * frameHeight; i += 2) {
        uint8_t data0;
        rgbFile.read(&data0, 1);
        frameBuf[i] = data0;
        uint8_t data1;
        rgbFile.read(&data1, 1);
        frameBuf[i + 1] = data1;
    }
}

void RGBPlayer::drawFrame () {
    for (uint16_t y = 0; y < (maxSize * MULTIPLIC); y+= resizeY) {
        for (uint16_t x = 0; x < (maxSize * MULTIPLIC); x+= resizeX) {
            uint16_t index = ((x / MULTIPLIC * resizeX) / MULTIPLIC) + ((y/MULTIPLIC * resizeY) / MULTIPLIC) * frameWidth;
            if (codec332)
                EffectMath::getPixel(((x - corrX) /MULTIPLIC), (HEIGHT- 1) - (y - corrY) / MULTIPLIC) = EffectMath::rgb332_To_CRGB(frameBuf[index]);
            else {
                index *= 2;
                uint16_t result = ((uint16_t)frameBuf[index] << 8) | (uint16_t)frameBuf[index + 1];
                EffectMath::getPixel(((x - corrX) /MULTIPLIC), (HEIGHT- 1) - (y - corrY) / MULTIPLIC) = EffectMath::rgb565_To_CRGB(result);
            }
        }
    }
}

void RGBPlayer::load_PGM(uint8_t *data) {
    LOG(println, F("RGBPlayer: Start. PROGMEM mode."));
    frameWidth = pgm_read_byte(data);
    frameHeight = pgm_read_byte(data + 1);
    frames = pgm_read_byte(data + 2);
    LOG(printf_P, PSTR("RGBPlayer: Image loaded. It has %d frames. Image size %dX%d.\n"), frames, frameWidth, frameHeight);

    calc();
}

void RGBPlayer::load_FILE(String &filename) {
    codec332 = filename.indexOf(F("332")) > 0; 
    LOG(printf_P, PSTR("RGBPlayer: Start. File rgb%d mode.\n"), (codec332 ? 332U: 565U));
    rgbFile = LittleFS.open(filename, "r");
    if (rgbFile && rgbFile.isFile() && rgbFile.size() >= (3 + WIDTH * HEIGHT)) {
        rgbFile.read(&frameWidth, 1);
        rgbFile.read(&frameHeight, 1);
        rgbFile.read(&frames, 1);
        LOG(printf_P, PSTR("RGBPlayer: File %s loaded. It has %d frames. \n Image size %dX%d.\n"), filename.c_str(), frames, frameWidth, frameHeight);
    
        calc();
    } else {
        LOG(println, F("File not found or wrong format!"));
    }
}

void RGBPlayer::play332_PGM(uint8_t *data, uint8_t frameDelay) {
    if ((millis() - timer >= frameDelay) and done) {
        drawFrame();
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

void RGBPlayer::play_File() {
    if ((millis() - timer >= frameDelay) and done) {
        drawFrame();
        FastLED.show();
        done = false;
        timer = millis();
        frame++;
        if (frame >= frames)
            frame = 0;
    } else if ((millis() - timer < frameDelay) and !done) {
        if (codec332) getFromFile_332(frame);
        else getFromFile_565(frame);
        done = true;
    }
}

void RGBPlayer::stopPlayer() {
    if (rgbFile and rgbFile.isFile()) {
        rgbFile.close();
        LOG(println, F("RGBPlayer: Stop. File closed."));
    }
    delete [] frameBuf;
    LOG(println, F("RGBPlayer: Framebuffer destoyed."));
}

#endif