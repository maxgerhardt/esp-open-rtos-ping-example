# Ping Example for esp-open-rtos 

## Description
A simple example for how to ping a host using lwIP and esp-open-rtos

## lwIP configuratoin

The `lwipopts.h` must be adapted to activate the following functions:

```cpp
#define LWIP_RAW                        1
#define DEFAULT_RAW_RECVMBOX_SIZE	      5
```

Otherwise no raw socket can be created or nothing can be received on it

## Ping configuration 

Parameters such as the maximum receive timeout, the ID of the ICMP echo packet and the ping data size are hardcoded in the `ping_helper.c` file. Change these parameters at will 

```cpp
/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif
```

## Demo 

The demo connects to a WiFi network (define parameters in `main.c`) and pings `www.google.com`. It can also be configured to ping the gateway of the network.

## API 

The function 

```cpp
void ping_ip(ip_addr_t ping_addr, ping_result_t* res);
```

Can be used to ping an arbitrary IP address and store the result of the ping. The result is a structure 

```cpp
typedef struct {
	ping_result_code result_code;
	u32_t response_time_ms;
	ip_addr_t response_ip;
} ping_result_t;
```
The result code is defined as 

```cpp
typedef enum {
	PING_RES_NO_MEM,				/* internal memory alloc failure */
	PING_RES_ERR_SENDING,			/* socket could not send */
	PING_RES_ERR_NO_SOCKET,			/* socket could not be created */
	PING_RES_TIMEOUT,				/* no response received in time */
	PING_RES_ID_OR_SEQNUM_MISMATCH,	/* response ID or sequence number mismatched */
	PING_RES_ECHO_REPLY, 			/* ping answer received */
	PING_RES_DESTINATION_UNREACHABLE, /* destination unreachable received */
	PING_RES_TIME_EXCEEDED,			/* for TTL to low or time during defrag exceeded (see wiki) */
	PING_RES_UNHANDLED_ICMP_CODE, 	/* for all ICMP types which are not specifically handled */
} ping_result_code;
```
