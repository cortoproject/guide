/* This is a managed file. Do not delete this comment. */

#include <include/drone.h>

void drone_Drone_define(
    drone_Drone this)
{
    printf("drone '%s' defined!\n", corto_idof(this));
}


void drone_Drone_update(
    drone_Drone this)
{
    printf("drone '%s' updated!\n", corto_idof(this));
}

int16_t drone_Drone_construct(
    drone_Drone this)
{
    printf("drone %s constructed!\n", corto_idof(this));
    return 0;
}

int16_t drone_Drone_validate(
    drone_Drone this)
{
    if (this->altitude < 0) {
        corto_error("drone %s has an invalid altitude!", corto_idof(this));
        return -1;
    }
    return 0;
}
