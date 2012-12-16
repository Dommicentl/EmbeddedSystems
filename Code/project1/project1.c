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

typedef struct { 
	int hours;
	int minutes;
	int seconds;
} timeValue;
static timeValue currentTimeV;
static timeValue alarmTimeV;
static timeValue *currentTime;
static timeValue *alarmTime;

int isAlarm(void){
	if(currentTime->hours == alarmTime->hours &&
		currentTime->minutes == alarmTime->minutes &&
		currentTime->seconds == alarmTime->seconds){
		return 1;
	}
	return 0;
}    

void incrementHours(timeValue *time, int handleOverflow){
	(time->hours)++;
	if(time->hours >= 24){
		time->hours = time->hours % 24;
	}
}

void incrementMinutes(timeValue *time, int handleOverflow){
	(time->minutes)++;
	if(time->minutes >= 60){
		time->minutes = time->minutes % 60;
		if(handleOverflow)
			incrementHours(time, handleOverflow);
	}
}

void incrementSeconds(timeValue *time, int handleOverflow){
	(time->seconds)++;
	if(time->seconds >= 60){
		time->seconds = time->seconds % 60;
		if(handleOverflow)
			incrementMinutes(time, handleOverflow);
	}
}

void prettyPrint(timeValue *time){
	char toPrint[16];
	sprintf(toPrint, "%02d:%02d:%02d", time->hours, time->minutes, time->seconds);
	DisplayString(0, toPrint);
}

void setTime(timeValue *time){
	prettyPrint(time);
	while(BUTTON1_IO == 1u){
		LED2_IO = 1;
		if(BUTTON0_IO == 0u){
			incrementSeconds(time,0);
			prettyPrint(time);
			while(BUTTON0_IO == 0u){};
		}
		LED2_IO = 0;
	}
	while(BUTTON1_IO == 0u){};
	while(BUTTON1_IO == 1u){
		LED1_IO = 1;
		if(BUTTON0_IO == 0u){
			incrementMinutes(time,0);
			prettyPrint(time);
			while(BUTTON0_IO == 0u){};
		}
		LED1_IO = 0;
	}
	while(BUTTON1_IO == 0u){};
	while(BUTTON1_IO == 1u){
		LED0_IO = 1;
		if(BUTTON0_IO == 0u){
			incrementHours(time,0);
			prettyPrint(time);
			while(BUTTON0_IO == 0u){};
		}
		LED0_IO = 0;
	}
	while(BUTTON1_IO == 0u){};
}

void prettyPrint2(int time, int loc){
	char toPrint[15];
	sprintf(toPrint, "%d", time);
	DisplayString(loc, toPrint);
}

void handler(void) interrupt 1{
	static int half = 0;
	static int alarm = 0;
	static long int counter = 0;
	#ifdef FREQ_CHECK
	static long int counterMax = 80000;
	#else
	static long int counterMax = 30937;
	#endif
	#ifdef FREQ_CHECK
	if(PIR1bits.TMR1IF == 1){
		PIR1bits.TMR1IF = 0;
		prettyPrint2(counter,16);
		counter = 0;
		TMR1H = 0xc0;
		TMR1L = 0x00;
	}
	else
	#endif
	if(INTCONbits.T0IF == 1){
		counter++;
		INTCONbits.T0IF == 0;
		//reset clock
		TMR0H = 0x00;
		TMR0L = 0x00;
		if(counter > counterMax){
			if(half == 0){
				half = 1;
				//toggle yellow led
				LED0_IO=1;
				if(alarm > 0){
					//toggle red led
					LED1_IO = 1;
				}
			}
			else {
				half = 0;
				//toggle yellow led
				LED0_IO=0;
				incrementSeconds(currentTime, 1);
				prettyPrint(currentTime);
				if(alarm == 0 && isAlarm()){
					alarm = 1;
				}
				if(alarm > 0){
					//toggle red led
					LED1_IO=0;
					alarm++;
				}
				if(alarm > 30){
					alarm = 0;
				}
			}
			counter = 0;
		}
	}
}

void initTime(timeValue *time){
	time->seconds = 0;
	time->minutes = 0;
	time->hours = 0;
}

void initBoard(void){
	LCDInit();
	DelayMs(100);
	//Configure buttons
    BUTTON0_TRIS = 1;
    BUTTON1_TRIS = 1;
	//Configure timing
	TMR0H = 0x00;
	TMR0L = 0x00;
	T0CONbits.T08BIT = 0;
	T0CONbits.T0CS = 0;
	T0CONbits.PSA = 1;
    //Init leds
    LED0_TRIS=0;
	LED1_TRIS=0;
	LED2_TRIS=0;
	LED0_IO = 0;
	LED1_IO = 0;
	LED2_IO = 0;
}

void initT1(void){
	TMR1H = 0xc0;
	TMR1L = 0x00;
	
	T1CONbits.RD16 = 1;
	T1CONbits.T1CKPS1 = 0;
	T1CONbits.T1CKPS0 = 0;
	T1CONbits.T1OSCEN = 1;
	T1CONbits.T1SYNC = 1;
	T1CONbits.TMR1CS = 1;
	
	//enable interrupts
	PIE1bits.TMR1IE = 1;
	PIR1bits.TMR1IF = 0;
	IPR1bits.TMR1IP = 1;
}

void main(void){
	T0CONbits.TMR0ON = 0;
	initBoard();
	#ifdef FREQ_CHECK
		initT1();
		T1CONbits.TMR1ON = 1;
	#endif
	currentTime = &currentTimeV;
	alarmTime = &alarmTimeV;
	initTime(currentTime);
	initTime(alarmTime);
	setTime(currentTime);
	setTime(alarmTime);
	//Enable interrupts
	INTCONbits.GIE = 1;
	INTCON2bits.T0IP = 1;
	INTCONbits.T0IF = 0;
	INTCONbits.T0IE = 1;
	// Timer 0 enable
	T0CONbits.TMR0ON = 1;
}

/*************************************************
 Function DisplayWORD:
 writes a WORD in hexa on the position indicated by
 pos. 
 - pos=0 -> 1st line of the LCD
 - pos=16 -> 2nd line of the LCD

 __SDCC__ only: for debugging
*************************************************/
#if defined(__SDCC__)
void DisplayWORD(BYTE pos, WORD w) //WORD is a 16 bits unsigned
{
    BYTE WDigit[6]; //enough for a  number < 65636: 5 digits + \0
    BYTE j;
    BYTE LCDPos=0;  //write on first line of LCD
    unsigned radix=10; //type expected by sdcc's ultoa()

    LCDPos=pos;
    ultoa(w, WDigit, radix);      
    for(j = 0; j < strlen((char*)WDigit); j++)
    {
       LCDText[LCDPos++] = WDigit[j];
    }
    if(LCDPos < 32u)
       LCDText[LCDPos] = 0;
    LCDUpdate();
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
#endif
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
