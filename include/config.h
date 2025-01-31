#ifndef CONFIG_H
#define CONFIG_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#define KZT_VERSION "pre-alpha"
#define CTRL_KEY(k) ((k) & 0x1f) //Bitmap applied to the ASCII value of the key pressed to strip away the 2 MSB (Retaining 5 bits). Take 'A' (65: 1000001) => 'C-a' (1: 00001)

#define TAB_STOP 4

#endif
