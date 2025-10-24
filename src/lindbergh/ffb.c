#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "ffb.h"

static pthread_mutex_t ffb_mutex = PTHREAD_MUTEX_INITIALIZER;
static const struct ffb_ops *g_backend;

void ffb_init(const struct ffb_ops *ops)
{
    g_backend = ops;
    printf("FFB: hook enabled.\n");
}

void ffb_req_toggle(const uint8_t *bytes)
{
    uint8_t activate = bytes[2];
    pthread_mutex_lock(&ffb_mutex);
    if (activate == 0x01)
    {
        printf("FFB: Activated\n");
    }
    else
    {
        printf("FFB: Deactivated\n");
    }
    if (g_backend && g_backend->req_toggle)
        g_backend->req_toggle(activate == 0x01);
    pthread_mutex_unlock(&ffb_mutex);
}

void ffb_req_constant_force(const uint8_t *bytes)
{
    uint8_t direction = bytes[1];
    uint8_t force = bytes[2];
    if (direction == 0x0)
    {
        // Right
        force = 128 - force;
    }
    pthread_mutex_lock(&ffb_mutex);
    if (g_backend && g_backend->req_constant_force)
        g_backend->req_constant_force(direction, force);
    pthread_mutex_unlock(&ffb_mutex);
}

void ffb_req_rumble(const uint8_t *bytes)
{
    uint8_t force = bytes[1];
    uint8_t period = bytes[2];
    pthread_mutex_lock(&ffb_mutex);
    if (g_backend && g_backend->req_rumble)
        g_backend->req_rumble(force, period);
    pthread_mutex_unlock(&ffb_mutex);
}

void ffb_req_damper(const uint8_t *bytes)
{
    uint8_t force = bytes[2];
    pthread_mutex_lock(&ffb_mutex);
    if (g_backend && g_backend->req_damper)
        g_backend->req_damper(force);
    pthread_mutex_unlock(&ffb_mutex);
}