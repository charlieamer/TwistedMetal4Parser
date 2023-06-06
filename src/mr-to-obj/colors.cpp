#include "colors.h"

uint8_t highest(uint8_t r, uint8_t g, uint8_t b) {
  return max(max(r, g), b);
}

RGBA getColor(uint16_t color, SEMI_TRANSPARENCY_MODE mode, bool isFaceTransparent) {
  if (color == 0) {
    return {0,0,0,0};
  }
  uint8_t r = color & 0b00000000'00011111;
  uint8_t g =(color & 0b00000011'11100000) >> 5;
  uint8_t b =(color & 0b01111100'00000000) >> 10;
  uint8_t s =(color & 0b10000000'00000000) >> 15;
  r = (uint8_t)(r << 3);
  g = (uint8_t)(g << 3);
  b = (uint8_t)(b << 3);
  if (!s || !isFaceTransparent) {
    return { r, g, b, 255 };
  } else {
    switch(mode) {
      case TRANSPARENCY_MODE_ALPHA:
        return { r, g, b, 127 };
      case TRANSPARENCY_MODE_FULL:
        return { r, g, b, (uint8_t)min(highest(r,g,b) * highest(r,g,b) / 256 * 4, 255) };
      case TRANSPARENCY_MODE_FULL_INVERTED:
        return { r, g, b, (uint8_t)(255 - highest(r,g,b)) };
      case TRANSPARENCY_MODE_QUARTER:
        return { r, g, b, (uint8_t)(highest(r,g,b) * highest(r,g,b) / 256) };
    }
  }
  return { 255, 0, 255, 255 };
}