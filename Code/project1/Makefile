project1 : project1.c
	sdcc -mpic16 -p18f97j60 -L/usr/local/lib/pic16 project1.c

project1_freq : project1.c
	sdcc -mpic16 -p18f97j60 -L/usr/local/lib/pic16 -DFREQ_CHECK project1.c

clean : 
	$(RM) $(OBJECTS)

