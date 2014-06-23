#ifndef __STRIP_H__
#define __STRIP_H__

#define STRIP_LENGTH 150                        // Number of pixels in a strip
#define STRIP_BUFSSIZE (3 * STRIP_LENGTH)         // Number of bytes per strip (24 bit RGB)
#define STRIP_NUMBUFFERS 6                      // Total number of strips in the application


void stripSetup();
void stripClear();
void stripRun(unsigned long milliseconds);

#endif // __STRIP_H__
