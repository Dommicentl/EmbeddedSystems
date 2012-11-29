#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "LCDBlocking.c"
#include "Include/TCPIP_Stack/Delay.h"

void DisplayString(BYTE pos, char* text);
size_t strlcpy(char *dst, const char *src, size_t siz);

struct time { 
	int hours;
	int minutes;
	int seconds;
}currentTime,alarmTime;

void incrementHours(struct time *time, int handleOverflow){
	(time->hours)++;
	if(time->hours >= 24){
		time->hours = time->hours % 24;
	}
}

void incrementMinutes(struct time *time, int handleOverflow){
	(time->minutes)++;
	if(time->minutes >= 60){
		time->minutes = time->minutes % 60;
		if(handleOverflow)
			incrementHours(time, handleOverflow);
	}
}

void incrementSeconds(struct time *time, int handleOverflow){
	(time->seconds)++;
	if(time->seconds >= 60){
		time->seconds = time->seconds % 60;
		if(handleOverflow)
			incrementMinutes(time, handleOverflow);
	}
}

void prettyPrint(struct time *time){
	char toPrint[9];
	sprintf(toPrint, "%02d:%02d:%02d", time->hours, time->minutes, time->seconds);
	DisplayString(toPrint, 0);
}

void setTime(struct time *time){
	while(BUTTON1_IO == 1u){
		if(BUTTON0_IO == 0u){
			incrementSeconds(time,0);
			prettyPrint(time);
		}
	}
	while(BUTTON1_IO == 1u){
		if(BUTTON0_IO == 0u){
			incrementMinutes(time,0);
			prettyPrint(time);
		}
	}
	while(BUTTON1_IO == 1u){
		if(BUTTON0_IO == 0u){
			incrementHours(time,0);
			prettyPrint(time);
		}
	}
}

void initBoard(){
	LCDInit;
	DelayMs(100);
	//Configure buttons
    BUTTON0_TRIS = 1;
    BUTTON1_TRIS = 1;
	
	//Todo configure timer!!
}

void main(void){
   LCDInit();
   DelayMs(100);
   initBoard;
   setTime(&currentTime);
   setTime(&alarmTime);
}

/*************************************************
 Function DisplayString: 
 Writes the first characters of the string in the remaining 
 space of the 32 positions LCD, starting at pos
 (does not use strlcopy, so can use up to the 32th place)
*************************************************/
void DisplayString(BYTE pos, char* text)
{
   BYTE        l = strlen(text);/*number of actual chars in the string*/
   BYTE      max = 32-pos;    /*available space on the lcd*/
   char       *d = (char*)&LCDText[pos];
   const char *s = text;
   size_t      n = (l<max)?l:max;
   /* Copy as many bytes as will fit */
    if (n != 0)
      while (n-- != 0)*d++ = *s++;
   LCDUpdate();

}

/*-------------------------------------------------------------------------
 *
 * strlcpy.c
 *    strncpy done right
 *
 * This file was taken from OpenBSD and is used on platforms that don't
 * provide strlcpy().  The OpenBSD copyright terms follow.
 *-------------------------------------------------------------------------
 */

/*  $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $    */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 * Function creation history:  http://www.gratisoft.us/todd/papers/strlcpy.html
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
    char       *d = dst;
    const char *s = src;
    size_t      n = siz;
 
    /* Copy as many bytes as will fit */
    if (n != 0)
    {
        while (--n != 0)
        {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0';          /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return (s - src - 1);       /* count does not include NUL */
}
