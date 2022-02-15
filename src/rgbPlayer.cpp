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
    uint16_t newBufSize = frameWidth * frameHeight * (codec332 ? 1 : 2);
    if (bufSize < newBufSize) {
        delete [] frameBuf;
        frameBuf = new uint8_t[newBufSize];
    }
    bufSize = newBufSize;
    LOG(printf_P, PSTR("RGBPlayer: Framebuffer size is %d bytes.\n"), newBufSize);

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
    uint32_t index = (frameWidth * frameHeight) * frame * 2 + 3;
    rgbFile.seek(index, SeekSet);

    for(uint16_t i = 0; i < frameWidth * frameHeight; i ++) {
        uint8_t data0;
        rgbFile.read(&data0, 1);
        frameBuf[i*2] = data0;
        uint8_t data1;
        rgbFile.read(&data1, 1);
        frameBuf[i*2 + 1] = data1;
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
    if (blur) EffectMath::blur2d(64);
}

bool RGBPlayer::loadFile(String &filename) {
    if (rgbFile and rgbFile.isFile()) {
        rgbFile.close();
        LOG(println, F("RGBPlayer: Previose file was cloced"));
    }
    codec332 = filename.indexOf(F("332")) > 0; 
    LOG(printf_P, PSTR("RGBPlayer: Start. File rgb%d mode.\n"), (codec332 ? 332U: 565U));
    rgbFile = LittleFS.open(filename, "r");
    if (rgbFile && rgbFile.isFile() && rgbFile.size() >= (3 + WIDTH * HEIGHT)) {
        rgbFile.read(&frameWidth, 1);
        rgbFile.read(&frameHeight, 1);
        rgbFile.read(&frames, 1);
        LOG(printf_P, PSTR("RGBPlayer: File %s loaded. It has %d frames. \nRGBPlayer: Image size %dX%d.\n"), filename.c_str(), frames, frameWidth, frameHeight);
    
        calc();
    } else {
        LOG(println, F("File not found or wrong format!"));
        return false;
    }
    return true;
}

void RGBPlayer::playFile(bool show) {

    if (show) {
    if ( !fader && !myLamp.isLampOn() && !myLamp.isAlarm() ) return;
        drawFrame();
        FastLED.show();
        myLamp.playerTimer(T_ENABLE, frameDelay);
        frame++;
        if (frame >= frames)
            frame = 0;
    } else {
        if (codec332) getFromFile_332(frame);
        else getFromFile_565(frame);
        myLamp.playerTimer(T_FRAME_ENABLE);
    }
}

void RGBPlayer::stopPlayer() {
    delete [] frameBuf;
    frameBuf = nullptr;
    LOG(println, F("RGBPlayer: Framebuffer destoyed."));
}

#endif