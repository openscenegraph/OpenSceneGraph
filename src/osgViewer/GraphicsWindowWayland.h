#pragma once
#include <wayland-client.h>

// declare our function for accessing a data_device from the Wayland connection
#ifdef __cplusplus
extern "C" {
#endif
    struct wl_data_device* getWaylandDataDevice();
    struct wl_data_source* getWaylandDataSource();
    uint32_t getWaylandLastKeySerial();
    struct wl_display* getWaylandDisplay();
#ifdef __cplusplus
}
#endif
