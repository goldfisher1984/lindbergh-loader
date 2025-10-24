#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct ffb_ops
{
    void (*req_toggle)(bool active);
    void (*req_constant_force)(uint8_t direction, uint8_t force);
    void (*req_rumble)(uint8_t force, uint8_t period);
    void (*req_damper)(uint8_t force);
};

void ffb_req_toggle(const uint8_t *bytes);
void ffb_req_constant_force(const uint8_t *bytes);
void ffb_req_rumble(const uint8_t *bytes);
void ffb_req_damper(const uint8_t *bytes);

void ffb_init(const struct ffb_ops *ops);
