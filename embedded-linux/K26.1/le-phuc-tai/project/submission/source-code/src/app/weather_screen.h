/**
 * @file weather_screen.h
 * @brief Weather display screen controller & HTTP client interface
 * @author PHUC TAI
 */

#ifndef _WEATHER_SCREEN_H_
#define _WEATHER_SCREEN_H_

#include "system_state.h"

#define WEATHER_SERVER_IP       "127.0.0.1"
#define WEATHER_SERVER_PORT     8080
#define WEATHER_REQUEST_PATH    "/weather"

#define WEATHER_TIMEOUT_SEC     5
#define WEATHER_REFRESH_SEC     300 /* 5 minutes interval */

void *weather_thread_func(void *arg);

/* Synchronized prototype: 2 parameters */
void render_weather_ui(const weather_data_t *data, net_mode_t net_mode);

#endif /* _WEATHER_SCREEN_H_ */