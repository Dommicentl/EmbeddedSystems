AS = gpasm
CC = sdcc
CFLAGS= -c -mpic16 -p18f97j60 -o$@ 
LD = sdcc
LDFLAGS= -mpic16 -p18f97j60 -Wl,-s,Linker.lkr -L/usr/local/lib/pic16 -llibio18f97j60.lib \
         -llibdev18f97j60.lib -llibc18f.a -DSTACK_USE_UDP
AR = ar
RM = rm

OBJECTS= Objects/LCDBlocking.o

SDCC_HEADERS=/usr/local/share/sdcc/include/string.h \
   /usr/local/share/sdcc/include/stdlib.h \
   /usr/local/share/sdcc/include/stdio.h \
   /usr/local/share/sdcc/include/stddef.h \
   /usr/local/share/sdcc/include/stdarg.h 

SDCC_PIC16_HEADERS=/usr/local/share/sdcc/include/pic16/pic18f97j60.h

TCPIP_HEADERS=   Include/TCPIP_Stack/ETH97J60.h \
   Include/TCPIP_Stack/LCDBlocking.h 

APP_HEADERS=Include/GenericTypeDefs.h \
   Include/Compiler.h \
   Include/HardwareProfile.h 

project2: object announce.o arp.o delay.o dhcp.o dns.o eth.o hashes.o helpers.o stacktsk.o lcdblocking.o tick.o project2.o ip.o udp.o udppt.o icmp.o
	$(LD) $(LDFLAGS) object/helpers.o object/stacktsk.o object/tick.o object/lcdblocking.o \
	object/project2.o object/announce.o object/arp.o object/delay.o object/dhcp.o object/dns.o object/eth.o \
	object/hashes.o object/udp.o object/ip.o object/udppt.o object/icmp.o -o object/project2.hex
	cp object/project2.hex .

project2.o: project2.c object
	$(CC) $(CFLAGS) project2.c -o object/project2.o

tick.o: TCPIP_Stack/Tick.c
	$(CC) $(CFLAGS) TCPIP_Stack/Tick.c -o object/tick.o

lcdblocking.o: TCPIP_Stack/LCDBlocking.c
	$(CC) $(CFLAGS) TCPIP_Stack/LCDBlocking.c -o object/lcdblocking.o

stacktsk.o: TCPIP_Stack/StackTsk.c
	$(CC) $(CFLAGS) TCPIP_Stack/StackTsk.c -o object/stacktsk.o

helpers.o: TCPIP_Stack/Helpers.c
	$(CC) $(CFLAGS) TCPIP_Stack/Helpers.c -o object/helpers.o

announce.o: TCPIP_Stack/Announce.c
	$(CC) $(CFLAGS) TCPIP_Stack/Announce.c -o object/announce.o

arp.o: TCPIP_Stack/ARP.c
	$(CC) $(CFLAGS) TCPIP_Stack/ARP.c -o object/arp.o

delay.o: TCPIP_Stack/Delay.c
	$(CC) $(CFLAGS) TCPIP_Stack/Delay.c -o object/delay.o

dhcp.o: TCPIP_Stack/DHCP.c
	$(CC) $(CFLAGS) TCPIP_Stack/DHCP.c -o object/dhcp.o

dns.o: TCPIP_Stack/DNS.c
	$(CC) $(CFLAGS) TCPIP_Stack/DNS.c -o object/dns.o

eth.o: TCPIP_Stack/ETH97J60.c
	$(CC) $(CFLAGS) TCPIP_Stack/ETH97J60.c -o object/eth.o

hashes.o: TCPIP_Stack/Hashes.c
	$(CC) $(CFLAGS) TCPIP_Stack/Hashes.c -o object/hashes.o

udp.o: TCPIP_Stack/UDP.c
	$(CC) $(CFLAGS) TCPIP_Stack/UDP.c -o object/udp.o

icmp.o: TCPIP_Stack/ICMP.c
	$(CC) $(CFLAGS) TCPIP_Stack/ICMP.c -o object/icmp.o

ip.o: TCPIP_Stack/IP.c
	$(CC) $(CFLAGS) TCPIP_Stack/IP.c -o object/ip.o

udppt.o: TCPIP_Stack/UDPPerformanceTest.c
	$(CC) $(CFLAGS) TCPIP_Stack/UDPPerformanceTest.c -o object/udppt.o

object:
	mkdir object

clean : 
	$(RM) $(OBJECTS)

