#include <include/drone.h>

void on_event(corto_observer_event *event) {
  Drone my_drone = event->data;

  if (event->event == CORTO_DEFINE) {
      printf("define");
  } else if (event->event == CORTO_UPDATE) {
      printf("update");
  }

  printf(" received from %s, altitude = %f\n",
      corto_idof(my_drone),
      my_drone->altitude);
}

int cortomain(int argc, char *argv[]) {
    Drone my_drone = Drone__create(data_o, "my_drone", 37, 122, 0);
    Drone my_2nd_drone = Drone__create(data_o, "my_2nd_drone", 37, 122, 0);
    int32_t *i = corto_int32__create(data_o, "i", 10);

    corto_observe(CORTO_DEFINE|CORTO_UPDATE|CORTO_ON_SCOPE, data_o)
        .type("drone/Drone")
        .callback(on_event);

    while (true) { // Loop forever
        corto_update_begin(my_drone);
        my_drone->altitude --;
        corto_update_end(my_drone);

        corto_update_begin(my_2nd_drone);
        my_2nd_drone->altitude += 2;
        corto_update_end(my_2nd_drone);

        corto_sleep(1, 0); // Sleep one second
    }

    return 0;
}
