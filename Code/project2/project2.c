#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "Include/TCPIP_Stack/TCPIP.h"
#include "Include/TCPIP_Stack/Delay.h"
#include "Include/TCPIP_Stack/LCDBlocking.h"
#include "Include/TCPIP_Stack/UDP.h"

size_t strlcpy(char *dst, const char *src, size_t siz);
static NODE_INFO broadcast;
static NODE_INFO dhcpserver;
static NODE_INFO gateway;
static NODE_INFO relay;
static BYTE relayIp[4] = {192, 168, 97, 60};
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};

APP_CONFIG AppConfig;

static enum
{
	LISTENSETUP_C, LISTEN_C, ARPRESOLVE_START, ARPRESOLVE, TRANSMITSETUP_S, TRANSMIT_S, LISTENSETUP_S, LISTEN_S, TRANSMITSETUP_C, TRANSMIT_C
} state = LISTENSETUP_C;

void DisplayString(BYTE pos, char* text)
{
        BYTE l = strlen(text);
        BYTE max = 32 - pos;
        strlcpy((char*)&LCDText[pos], text, (l < max) ? l : max);
        LCDUpdate();
}

void prettyPrint(int time, int loc){
	char toPrint[15];
	sprintf(toPrint, "%d", time);
	DisplayString(loc, toPrint);
}

void initAddresses(void){
	//Set broadcast ip
	broadcast.IPAddr.v[0] = 255;
	broadcast.IPAddr.v[1] = 255;
	broadcast.IPAddr.v[2] = 255;
	broadcast.IPAddr.v[3] = 255;
	//Set relay ip
	relay.IPAddr.v[0] = relayIp[0];
	relay.IPAddr.v[1] = relayIp[1];
	relay.IPAddr.v[2] = relayIp[2];
	relay.IPAddr.v[3] = relayIp[3];
	//Set broadcast mac
	broadcast.MACAddr.v[0]=0;
	broadcast.MACAddr.v[1]=0;
	broadcast.MACAddr.v[2]=0;
	broadcast.MACAddr.v[3]=0;
	broadcast.MACAddr.v[4]=0;
	broadcast.MACAddr.v[5]=0;
	//Set dhcp server ip
	dhcpserver.IPAddr.v[0] = 192;
	dhcpserver.IPAddr.v[1] = 168;
	dhcpserver.IPAddr.v[2] = 88;
	dhcpserver.IPAddr.v[3] = 254;
	//Set gateway ip
	gateway.IPAddr.v[0] = 192;
	gateway.IPAddr.v[1] = 168;
	gateway.IPAddr.v[2] = 97;
	gateway.IPAddr.v[3] = 1;
}

void start(void){
	
	static UDP_SOCKET receiveSocket;
    static UDP_SOCKET transmitSocket;
    
    BYTE buffer[576];
    BYTE *readPointer = &buffer[0];
    BYTE *writePointer = &buffer[0];
	int i;
	while(1){
		StackTask();
		
		switch(state)
		{
			case LISTENSETUP_C:
				DisplayString(0,"LISTENSETUP_C");
				receiveSocket = UDPOpen((UDP_PORT)67, NULL, (UDP_PORT)68);
				if(receiveSocket != INVALID_UDP_SOCKET){
					LED0_IO = 1;
					state = LISTEN_C;
				}
				break;
			case LISTEN_C:
				DisplayString(0,"LISTEN_C");
				if(!UDPIsGetReady(receiveSocket)){
					break;
				}
				LED1_IO = 1;
				//listen to the DHCP pakket of the client
				readPointer = &buffer[0];
				while(UDPGet(readPointer) == TRUE){
					readPointer++;
				}
				UDPDiscard();
				UDPClose(receiveSocket);
				//Change GIADRR
				for(i = 0; i < 4; i++){
					buffer[24+i] = relay.IPAddr.v[i];
				}
				state = ARPRESOLVE_START;
				break;
			case ARPRESOLVE_START:
				DisplayString(0,"ARPRESOLVE_START");
				ARPResolve(&gateway.IPAddr);
				state =  ARPRESOLVE;
				break;
			case ARPRESOLVE:
				DisplayString(0,"ARPRESOLVE");
				if(!ARPIsResolved(&gateway.IPAddr, &dhcpserver.MACAddr))
					break;
				state = TRANSMITSETUP_S;
				break;
			case TRANSMITSETUP_S:
				DisplayString(0,"TRANSMITSETUP_S");
				transmitSocket = UDPOpen((UDP_PORT)67, &dhcpserver, (UDP_PORT)67);
				if(transmitSocket != INVALID_UDP_SOCKET)
					state = TRANSMIT_S;
				break;
			case TRANSMIT_S:
				DisplayString(0,"TRANSMIT_S");
				if(!UDPIsPutReady(receiveSocket))
					break;
				//transmit the DHCP pakket to the server
				writePointer = &buffer[0];
				while(UDPPut(*writePointer) == TRUE && readPointer != writePointer)
					writePointer++;
				UDPFlush();
				UDPClose(transmitSocket);
				state = LISTENSETUP_S;
				break;
			case LISTENSETUP_S:
				DisplayString(0,"LISTENSETUP_S");
				receiveSocket = UDPOpen((UDP_PORT)67, &dhcpserver, (UDP_PORT)67);
				if(receiveSocket != INVALID_UDP_SOCKET)
					state = LISTEN_S;
				break;
			case LISTEN_S:
				DisplayString(0,"LISTEN_S:");
				if(!UDPIsGetReady(receiveSocket))
					break;
				//listen to the DHCP response of the server
				readPointer = &buffer[0];
				while(UDPGet(readPointer) == TRUE)
					readPointer++;
				UDPDiscard();
				UDPClose(receiveSocket);
				state = TRANSMITSETUP_C;
				break;
			case TRANSMITSETUP_C:
				DisplayString(0,"TRANSMITSETUP_C");
				transmitSocket = UDPOpen((UDP_PORT)67, NULL, (UDP_PORT)68);
				if(transmitSocket != INVALID_UDP_SOCKET)
					state = TRANSMIT_C;
				break;
			case TRANSMIT_C:
				DisplayString(0,"TRANSMIT_C");
				if(!UDPIsPutReady(receiveSocket))
					break;
				//transmit the DHCP response to the client
				while(UDPPut(*writePointer) == TRUE && readPointer != writePointer)
					writePointer++;
				UDPFlush();
				UDPClose(transmitSocket);
				state = LISTENSETUP_S;
				break;
		}
	}
}

void initAppConfig(void){
	AppConfig.Flags.bIsDHCPEnabled = FALSE;
    AppConfig.Flags.bInConfigMode = TRUE;

    AppConfig.MyMACAddr.v[0] = 0;
    AppConfig.MyMACAddr.v[1] = 0x04;
    AppConfig.MyMACAddr.v[2] = 0xA3;
    AppConfig.MyMACAddr.v[3] = 0x01;
    AppConfig.MyMACAddr.v[4] = 0x02;
    AppConfig.MyMACAddr.v[5] = 0x03;
     
    AppConfig.MyIPAddr.Val = (DWORD)relayIp[0] | (DWORD)relayIp[1] << 8 | (DWORD)relayIp[2] << 16 | (DWORD)relayIp[3] << 24;
    AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
    AppConfig.MyMask.Val = (DWORD)255 | (DWORD)255 << 8 | (DWORD)255 << 16 | (DWORD)255 << 24;
    AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
    AppConfig.MyGateway.Val = (DWORD)192 | (DWORD)168 << 8 | (DWORD)97 << 16 | (DWORD)1 << 24;
}

void initBoard(void){
	// Enable 4x/5x/96MHz PLL
	OSCTUNE = 0x40;
	
	LED0_TRIS = 0;
	LED1_TRIS = 0;
	LED2_TRIS = 0;
	LED0_IO = 0;
	LED1_IO = 0;
	LED2_IO = 0;
	
	if(OSCCONbits.IDLEN)
		OSCCON = 0x82;
    else
		OSCCON = 0x02;

	TRISGbits.TRISG5 = 0;

    
    // Enable Interrupts   
    RCONbits.IPEN = 1;      // Enable interrupt priorities   
    INTCONbits.GIEH = 1;   
    INTCONbits.GIEL = 1;
    LCDInit();
    DelayMs(100);
    TickInit();
    initAppConfig();
    StackInit();
	UDPInit();
   
}

void main(void){
	initAddresses();
	initBoard();
	start();
}

size_t strlcpy(char *dst, const char *src, size_t siz)
{
        char       *d = dst;
        const char *s = src;
        size_t      n = siz;

        /* Copy as many bytes as will fit */
        if (n != 0)
        {
                while (--n != 0)
                {
                        if (*d == '\0')
                                break;
                        else
                                *d++ = *s++;
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
	
