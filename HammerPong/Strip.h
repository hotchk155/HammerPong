#ifndef __STRIP_H__
#define __STRIP_H__


////////////////////////
// Strip numbering (X)
// 0 1 2      3 4 5
// 0 1 2      3 4 5 
// 0 1 2      3 4 5
//
// Row addressing
//  .........
// :         :
// 2         2
// 1         1
// 0         0
//
// Colour Layout
// GGGGGGGG RRRRRRRR BBBBBBBB


#define STRIP_LENGTH 150                        // Number of pixels in a strip
#define STRIP_BUFSSIZE (3 * STRIP_LENGTH)         // Number of bytes per strip (24 bit RGB)
#define STRIP_NUMBUFFERS 6                      // Total number of strips in the application
extern byte stripBuffer[STRIP_NUMBUFFERS][STRIP_BUFSSIZE]; // The huge display buffer!

void stripInvalidate();
void stripSetup();
void stripClear();
void stripRun(unsigned long milliseconds);

#endif // __STRIP_H__
