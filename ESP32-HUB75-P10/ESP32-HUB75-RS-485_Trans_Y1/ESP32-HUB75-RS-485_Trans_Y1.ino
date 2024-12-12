#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "FontTable.h"

#define PANEL_RES_X 32  // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 16  // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 2   // Total number of panels chained one to another

MatrixPanel_I2S_DMA *dma_display = nullptr;
int xcounter = 0;  // Tracks progress within 8-column blocks

#define RS485_TX_PIN 1  // Pin used for RS485 communication (TX)
#define RS485_RX_PIN 3  // Pin used for RS485 communication (RX)

int startX = 55;  // Start the scrolling text off-screen (at the right edge)
int offsetY = 0;  // Fixed vertical position for the text (first row)

String text = "A";  // The text we want to scroll
int textWidth = text.length() * 16;  // Calculate the width of the text (16 pixels per character)

void setup() {
  Serial.begin(115200);  // Initialize serial communication for debugging

  // RS485 communication settings
  Serial2.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

  // Initialize matrix panel
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,  // module width
    PANEL_RES_Y,  // module height
    PANEL_CHAIN   // Chain length
  );

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  if (!dma_display) {
    Serial.println("Failed to create MatrixPanel_I2S_DMA instance");
    while (1)
      ;
  }

  dma_display->begin();
  dma_display->setBrightness8(20);  // Set brightness to 20 out of 255
  dma_display->clearScreen();
}

void loop() {
  // Clear the screen before drawing the text
  dma_display->clearScreen();

  // Display the scrolling text
  displayCharacters(text, startX, offsetY);

  // Scroll the text by decreasing the startX position
  startX--;

  // If the text has completely scrolled off the screen, reset the position
  if (startX < -textWidth) {
    startX = 55;  // Start the text off-screen again (at the right edge)
  }

  delay(2100);  // Delay to control the speed of the scroll
}

// Function to display characters one by one
void displayCharacters(String text, int offsetX, int offsetY) {
  int charWidth = 8;    // Width of the character bitmap
  int charHeight = 15;  // Height of the character bitmap

  for (int i = 0; i < text.length(); i++) {
    char letter = text.charAt(i);

    // Display each character, with an appropriate X-offset
    displayCharacter(letter, offsetX + i * 16, offsetY);
  }
}

// Function to display a single character
void displayCharacter(char letter, int offsetX, int offsetY) {
  const int charWidth = 8;    // Width of the character bitmap
  const int charHeight = 15;  // Height of the character bitmap

  // Skip rendering if the letter is not between 'A' and 'Z'
  if (letter < 'A' || letter > 'Z') {
    return;
  }

  // Get the index in the font table (A is index 0, Z is index 25)
  int index = letter - 'A';

  // Loop over each row in the character's 15x8 bitmap
  for (int y = 0; y < charHeight; y++) {
    char row = FontTable15x8[index][y];  // Get the row data for the current character
    offsetX--;
    // Loop over each bit in the row (each bit represents a pixel)
    for (int x = 0; x < charWidth; x++) {
      // Calculate the actual display coordinates
      int displayX = offsetX + x;
      int displayY = offsetY + y;

        // Check if the bit is set
      if (row & (1 << (7 - x))) {
        // Draw the pixel on the matrix (use a color)
        dma_display->drawPixel(displayX, displayY, dma_display->color565(255, 0, 0));  // Red color for the character
      }
    }
  }
}
