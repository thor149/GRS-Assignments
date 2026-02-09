# MT25077 - PA02 Makefile

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread

COMMON  = MT25077_Part_A_Common.h

A1_SERVER = a1_server
A1_CLIENT = a1_client
A2_SERVER = a2_server
A2_CLIENT = a2_client
A3_SERVER = a3_server
A3_CLIENT = a3_client

ALL_BINS = $(A1_SERVER) $(A1_CLIENT) \
           $(A2_SERVER) $(A2_CLIENT) \
           $(A3_SERVER) $(A3_CLIENT)

.PHONY: all clean

all: $(ALL_BINS)

$(A1_SERVER): MT25077_Part_A1_Server.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A1_CLIENT): MT25077_Part_A1_Client.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A2_SERVER): MT25077_Part_A2_Server.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A2_CLIENT): MT25077_Part_A2_Client.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A3_SERVER): MT25077_Part_A3_Server.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(A3_CLIENT): MT25077_Part_A3_Client.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(ALL_BINS)
