#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include <string.h>
#include "lwip/netdb.h"
#include "ping_helper.h"

/**
 * WiFi credentials
 */
#define WIFI_SSID "YOUR_WIFI_HERE"
#define WIFI_PW "YOUR PASSWORD HERE"

void connect_wifi(const char *ssid, const char *pw) {
    struct sdk_station_config config;
    strncpy((char*) config.ssid, ssid, sizeof(config.ssid));
    strncpy((char*) config.password, pw, sizeof(config.password));
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_connect();
    while (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
        printf("Connecting.. status %d\n",
                sdk_wifi_station_get_connect_status());
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    printf("Connected\n");
}

void print_ip_info() {
    struct ip_info info;
    sdk_wifi_get_ip_info(STATION_IF, &info);
    printf("Got DHCP IP: %s\n", ipaddr_ntoa(&info.ip));
    printf("Got GW IP: %s\n", ipaddr_ntoa(&info.gw));
    printf("Got Netmask: %s\n", ipaddr_ntoa(&info.netmask));
}

ip_addr_t get_gw_ip() {
    struct ip_info info;
    sdk_wifi_get_ip_info(STATION_IF, &info);
    ip_addr_t gw_ip;
    gw_ip.addr = info.gw.addr;
    return gw_ip;
}

/* DNS name to ip_addr_t helper */
int do_dns_resolve(const char *hostname, ip_addr_t *target_ip) {
    const struct addrinfo hints = { .ai_family = AF_UNSPEC, .ai_socktype =
            SOCK_RAW };
    struct addrinfo *res;

    int err = getaddrinfo(hostname, NULL, &hints, &res);

    if (err != 0 || res == NULL) {
        printf("DNS lookup failed err=%d res=%p\r\n", err, res);
        if (res)
            freeaddrinfo(res);
        return -1;
    } else {
        struct sockaddr *sa = res->ai_addr;
        if (sa->sa_family == AF_INET) {
            struct in_addr ipv4_inaddr = ((struct sockaddr_in*) sa)->sin_addr;
            memcpy(target_ip, &ipv4_inaddr, sizeof(*target_ip));
            //printf("DNS lookup succeeded. IP=%s\r\n", inet_ntoa(target_ip));
        }
#if LWIP_IPV6
		if (sa->sa_family == AF_INET6) {
			struct in_addr ipv6_inaddr = ((struct sockaddr_in6 *)sa)->sin6_addr;
			memcpy(target_ip, &ipv6_inaddr, sizeof(*target_ip));
			//printf("DNS lookup succeeded. IP=%s\r\n", inet6_ntoa(target_ip));
		}
#endif
        freeaddrinfo(res);
        return 0;
    }
}

void mainTask(void *pvParameters) {
    ping_result_t res;
    connect_wifi(WIFI_SSID, WIFI_PW);
    print_ip_info();

    //get GW IP
    ip_addr_t to_ping = get_gw_ip();
    //or use hostname
    const char *hostname = "www.google.com";
    //set to true if you want to ping the gateway.
    bool ping_gateway = false;

    if (!ping_gateway) {
        if (do_dns_resolve(hostname, &to_ping) < 0) {
            printf("DNS resolve of \"%s\" failed\n", hostname);
            vTaskDelete(NULL);
        }
        printf("Pinging hostname %s at IP %s\n", hostname,
                ipaddr_ntoa(&to_ping));
    } else {
        printf("Pinging gateway at IP %s\n", ipaddr_ntoa(&to_ping));
    }

    while (1) {
        ping_ip(to_ping, &res);

        if (res.result_code == PING_RES_ECHO_REPLY) {
            printf("good ping from %s %u ms\n", ipaddr_ntoa(&res.response_ip),
                    res.response_time_ms);
        } else {
            printf("bad ping err %d\n", res.result_code);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void user_init(void) {
    uart_set_baud(0, 115200);
    xTaskCreate(mainTask, "mainTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}
