# Getting Started Guide

## Installing Corto
[Install Corto on your machine]

To install a development version of corto, run this from a terminal:

```demo
curl https://corto.io/install-dev-src | sh
```

This installs a development version of the corto runtime in addition to the tools and code generators needed to get started with building corto projects!

```warning
Corto is currently only supported on macOS and Linux operating systems.
```

The above command only installs the essential parts of the corto framework. You can install additional bundles for web connectivity and the documentation framework. **You will need to install the web bundle to run the examples in the guide.**

To install the corto web bundle, run this command:

```demo
curl https://corto.io/install-web-src | sh
```

To install the documentation framework, run this command:

```demo
curl https://corto.io/install-doc-src | sh
```

## The Basics
[Create your first corto application]

Corto is a soup-to-nuts framework that does not just provide a powerful API for building edge applications. It also comes with tools that build, test and document your projects. In this chapter we will take a quick tour through the framework that will give you enough information to write your first corto applications.

```note
Basic knowledge of C and the Linux command terminal is required to go through the guide.
```

### A new project
Lets create a C project that is going to simulate a drone which we can fly and remotely monitor. First, we need to create a new project. To do this, open up  a terminal and enter this command:

```demo
corto create drone
```
That creates a new directory called "drone" with these contents:

```
drone
  |- bin
  |- include
  |- src
  |- test
  |- model.corto
  +- project.json
```
Each corto project has the same layout. Source files go into `src`, include files go into `include`, the build result goes into `bin`, testcases go into `test`, and there is a `project.json` that contains information about how to build the project and other information like a description and author. Finally, there is the `model.corto` file which we will revisit in a bit.

When we created the project, corto already automatically built it for you. If you look inside the `bin` directory, you will find an executable called "drone" that you can run! Right now it does not do anything, so lets add some code.

```note
You can use any code editor to edit corto projects. We recommend Atom, as it integrates with cortoscript, which is the modeling language used by corto (search for "cortoscript" in Atom's packages).
```

### The cortomain function
Each project has a `cortomain` function, which is located in the main file of our project. In our case, this is `src/main.c`. The cortomain function is called when our application (or package, more on that later) starts, so lets add some code here:

```c
int cortomain(int argc, char *argv[]) {
    printf("My first corto application!\n");
}
```
The next section shows how to build and run the project.

### Building and running a project
To run this code, we need to rebuild the project. Corto uses a build system called **bake**, which is optimized for working with large numbers of small projects. To learn more about bake, [click here](https://www.corto.io/doc/bake.html). To build our project, simply enter this command:

```demo
bake
```
Now run the program with this command:

```demo
corto run
```
That should print the following output to the console:

```demo
My first corto application!
```

Alternatively you can also run the executable directly. You will have to export the bake environment first, so the runtime linker can find the corto library:

```demo
(export `bake env` && bin/x64-linux-debug/drone)
```

```note
The platform (x86-linux) might be different on your machine.
```

### Debugging Corto
Before starting to write corto applications, it is worthwhile to spend a little bit of time on how debugging corto works, as this will often give you useful tips on why things are going wrong!

Most corto functions will return a non-zero code when it fails, and a zero when success. When a function fails, you can use the exception handling framework to learn more about what went wrong.

The most used functions for handling exceptions are:

| Function | Description |
|----------|-------------|
| corto_throw | Throw an exception or add information to an existing exception |
| corto_try | Test if function succeeds, and if it fails jump to error label |
| corto_raise | Raise a thrown exception |
| corto_catch | Catch a thrown exception |

Let's look at a code example to see how they are used in practice:

```
int16_t divide (int num, int by, int *out)
{
    if (by == 0) {
        // Can't divide by zero, so throw an exception!
        corto_throw("cannot divide %d by zero!", num);
        goto error;
    }
    *out = num / by;
    return 0;
error:
    return -1;
}

int cortomain(int argc, char *argv[])
{
    int out;
    corto_try (divide(10, 0, &out), "failed to call divide");
    return 0;
error:
    return -1;
}
```
Copy this into `main.c` and run the application. You should see the following output:

```demo
error src/main.c:7 cannot divide 10 by zero!
from  src/main.c:21 failed to call divide
```
When the `divide` function failed, `corto_try` automatically jumped to the `error` label. The line with `corto_try` is actually shorthand for:

```c
if (divide(10, 0, &out) != 0) {
    corto_throw("failed to call divide");
    goto error;
}
```
When our application stopped, corto automatically raised our exception to the terminal. We can also manually raise an exception by calling `corto_raise()`. On the other hand, if we want to silence an exception, we should use `corto_catch()`. Try using `corto_raise` and `corto_catch` in your code, and see what happens!


#### Using GDB or Valgrind
A common way to debug native applications is by using GDB or Valgrind. However, when you try running corto with Valgrind or GDB like this:

```demo
valgrind corto
```
You will likely get an error like this:

```demo
/bin/sh: 0: Can't open corto
```

The reason for this is that when you run the corto command, you are in fact running a script in `/usr/local` that is running a corto executable in your local environment! This little trick lets us run corto from anywhere without having to set any environment variables- but with this as downside.

Fortunately there is a way around this. We can use bake to export our current environment to the shell, so that we will call the corto executable directly. To do this, run this command:

```
export `bake env`
```
If we now retry running valgrind or GDB with corto, the error should disappear.


#### More logging functions
There are many functions that can add tracing or add details to exceptions. In particular, `corto_log_push` and `corto_log_pop` are useful. These two functions add nesting to your traces. Try out this code in your `cortomain`:

```c
corto_info("dividing %d by %d", 10, 2);
corto_log_push("divide");
corto_info("result = %d", 5);
corto_log_pop();
```
When you run it, you should see this:

```demo
dividing 10 by 2
divide
|  result = 5
+  
```
As you can see, the tracing inside the `push`-`pop` added indentation to the log. This neat visual aid often makes it easy to add context to a trace and can make applications a lot easier to debug!

#### Control logging verbosity
When you're debugging an application, you sometimes need more information about what's going on inside Corto. Fortunately corto has a lot of built-in logging that can be enabled with the `corto_log_verbositySet` function. Call it like this:

```c
corto_log_verbositySet(CORTO_DEBUG);
```
`CORTO_DEBUG` is the lowest logging levels. The different logging levels are:

```
CORTO_DEBUG
CORTO_TRACE
CORTO_OK
CORTO_INFO
CORTO_WARNING
CORTO_ERROR
CORTO_CRITICAL
```
When you are debugging somebody else's project and you can't modify the code, you can also set the `CORTO_VERBOSITY` environment variable, like this:

```
export CORTO_VERBOSITY=DEBUG
```
For each level there is a function that lets you trace at that level. Here is an overview:

```c
corto_debug("setting this variable"); // information for developer debugging
corto_trace("the app is doing this"); // tracing reveals what the app is doing
corto_ok("this ended pretty well");   // a task completed successfully
corto_info("Hello World");            // General information (default loglevel)
corto_warning("watch out!");          // display warnings
corto_error("this is not right");     // log error directly to console
corto_critical("boom!");              // display error, stacktrace and abort
corto_assert(result == 0, "boom!");   // check condition, abort if failse.
```


### Using Packages
Packages are projects that can be reused by other projects. Our current drone project is an **application**, which builds to an executable binary. Package projects build to a shared library.

To create a new package, run this command:

```demo
corto create package weather
```
That will create a new folder called `weather`. You'll find that the contents of that folder are very similar to our previous project. When you open the `project.json` file you'll notice that it says `package` instead of `application`.

Packages, just like applications have a `cortomain` function. This function is called once when the package is loaded. Instead of adding code to `cortomain`, we'll add a new function `is_sunny` to this package. First, lets add the declaration of `is_sunny` to the `include/weather.h` header file. Add this code in between the `$body()` and `$end` comments:

```c
WEATHER_EXPORT
bool weather_is_sunny(void);
```

```note
It is good practice to prefix functions with the name of the package to avoid name collisions.
```

That will ensure that projects that include this package know the declaration of `is_sunny`, just like in regular C projects. Now, in the main source file (`src/weather.c`) add this code, after or before `cortomain`:

```c
bool weather_is_sunny(void) {
    return true; // A safe bet if, like us, you're in California!
}
```
Build the package, by running `bake` inside the weather directory. Now lets see how we can call the `is_sunny` function from the `drone` project!

```note
This example shows how to add native C functions to a package. We will see later how we can add functions and classes by using corto models.
```

#### Adding dependencies
To use the `weather` package in the `drone` project, modify the `project.json` file to this:

```json
{
    "id": "drone",
    "type": "application",
    "value": {
        "use": ["weather"]
    }
}
```
Simple as that! Now rebuild the drone project by running `bake`, and we can call the `is_sunny` function. Add this snippet to the `cortomain` function of the drone project:

```c
if (weather_is_sunny()) {
    printf("It's a sunny day today!\n");
}
```
Run bake to rebuild, and run the application. You should see this in your terminal:

```demo
It's a sunny day today!
```

### Creating an object
So far we have looked at features that make it easy to build and debug corto projects, but we haven't touched upon the most important part of the framework: **the object store**.

The object store is where all of the application data lives. In a non-corto application, creating application data would look something like this:

```c
struct Drone *my_drone = malloc(sizeof(struct Drone));
my_drone->latitude = 38;
my_drone->longitude = 122;
```
Or in C++, something like this:

```cpp
Drone *my_drone = new Drone(38, 122);
```
In a corto application we create objects with the Corto API, which has many advantages. To name a few, corto objects ...

- ... are thread safe
- ... can be synchronized with databases / web applications / other applications
- ... can be observed for changes
- ... have automatic memory management
- ... can be serialized to JSON or other formats
- ... can be inspected using reflection

Neat right? Let's see how we create a simple object with the corto API. Add this line of code to the `cortomain` of the  project:

```c
int32_t *my_obj = corto_int32__create(data_o, "my_obj", 10);
```
Let's take a closer look at what happens here. We just created a new corto object with the **type** `int32`, which is a 32 bit signed integer. The object has has a unique **identifier** `my_obj`. The **initial value** of this object is `10`. We created the object in the **scope** `data`.

Don't worry if that seems a bit like gibberish for now. We will revisit all of this at a later point in the guide. What is most important, is that you can now use this object like any other object that you would otherwise create with `malloc` or `new`! To set its value to something else, just do:

```c
*my_obj = 20;
```

```note
The "*" is necessary here because my_obj is a pointer variable. We don't want to set the pointer, instead we want to set the value of the pointer. That's what the "*" is for.
```

Think of objects in corto like something that represents the real world, like a "digital twin". Usually such objects are much more complicated than just a single integer. To create objects that represent more complex things, you will have to create a *data model*.

### Modeling data
Corto has an flexible data modeling framework that lets you model everything from very simple to very complicated objects. To model our drone, we will create a `Drone` class with `longitude`, `latitude` and `altitude` members. To create this model, open up the `model.corto` file in your project. The contents of this file currently look like this:

```corto
in drone
```
This indicates that everything in this file will be part of our `drone` application. To add our class, append the following code to `model.corto`:

```corto
class Drone {
    latitude: float64
    longitude: float64
    altitude: float64
}
```
The language in which models are defined is called **cortoscript**. For more information on cortoscript, [click here](https://www.corto.io/doc/cortoscript.html).

```note
The naming convention for classes is to start with a uppercase letter, whereas application and package names start with a lowercase letter.
```

From this model, corto will generate code that makes it easy for us to work with this type. Code generators are integrated with the build system, so all you need to do to generate the code is to run **bake** again.

In our application we can now create a Drone instance with the generated `Drone__create` function:

```c
Drone my_drone = Drone__create(data_o, "my_drone", 37, 122, 0);
```
Note how similar the function looks to the `corto_int32__create` function! They are both generated functions that follow the same convention:

```c
Type Type__create(<parent>, <identifier>, <initial value>)
```
Just like the `int32` object, we can modify the `my_drone` object as if it were a normal C object. Lets print its members to the console:

```c
printf("Latitude: %f, Longitude: %f, Altitude: %f\n",
    my_drone->latitude,
    my_drone->longitude,
    my_drone->altitude);
```
When you run it, it should print this to the terminal:

```code
Latitude: 37.000000, Longitude: 122.000000, Altitude: 0.000000
```

```note
Notice how the order of the arguments in Drone__create is the same as the order of the members in the model definition.
```

### Adding a REST API
Now that we have (a little bit of) data, lets add a REST API to see how corto can take our objects, and make them available for simple web applications. To add the REST API to our project, create a new file called `config.corto` in the root of your project, and add the following code to it:

```corto
corto.rest.service config.rest = {
    port: 9090
}
```
That's all that is needed to add the REST API to our application. The next time we run the application with **corto run**, the REST service will be automatically loaded.

```warning
You need the corto web bundle to use the REST package. For instructions, see "Installation".
```

There is only one problem: when we start our application it immediately exits, so we don't have a lot of time to try out the REST service. To make sure the application doesn't exit, add the `-a` option to the `corto run` command (`-a` is short for `--keep-alive`):

```demo
corto -a run
```
Now that our REST service is running, open up a browser and go to `http://localhost:9090/data`. You should see the following response:

```json
[{
  "id":"my_drone",
  "type":"/drone/Drone",
  "value":{
    "latitude":37.000000,
    "longitude":122.000000,
    "altitude":0.000000
  }
}]
```

### Adding a corto web UI
See how easy it was to add a REST interface to our application? That's because `corto.rest` is an existing package that out of the box can work with any kind of object in the object store. Just like `corto.rest`, there are many more packages we can use. Lets add one more to our project: the **corto web UI**. This is a web application that lets us see the objects in our application.

To create a web UI, append the following code to `config.corto`:

```corto
corto.ws.service config.ws = {
    port: 9090
}

corto.ui.service config.ui = {
    port: 9090
}
```
Lets briefly go over what hapens here. This creates two new services, one that lets us create websocket connections (`config.ws`) and one that serves up the web UI resources, like HTML, CSS and JavaScript (`config.ui`). Just like the REST API, we run both services on port `9090`.

Now restart the application, and go to http://localhost:9090. Wait- that doesn't look like a web UI at all! What happened here?

The corto web framework lets us run multiple services on the same port, and usually this works just fine. But occasionally these services conflict, and corto does not know which URL belongs to which service. Instead of our web UI, we hit the REST API! To prevent this from happening, change the REST configuration to this:

```corto
corto.rest.service config.rest = {
    port: 9090,
    endpoint: "api"
}
```
Notice the "endpoint" member? That will make our REST API available under URL http://locahost:9090/api. Restart our application again, and go to http://localhost:9090. You should now see the following UI:

![UI screenshot](ui_screenshot.png)

The UI correctly shows that we have one object of type `Drone`, with id `my_drone`. It also shows us the current value of the object. Keep the UI open as it will be useful for showing what happens in the next section!

### Updating objects
Our objects wouldn't be very useful if their values could never change. To update the value of an object, we'll add some code to our cortomain:

```c
while (true) { // Loop forever
    corto_update_begin(my_drone);
    my_drone->altitude ++;
    corto_update_end(my_drone);

    corto_sleep(1, 0); // Sleep one second
}
```
The `corto_update_begin` and `corto_update_end` calls let Corto know that you are going to change the value of the `my_drone` object. In between `begin` and `end` the object is locked, so access is thread safe. When `corto_update_end` is called, corto will send a notification to anyone that is listening to object updates.

Right now, the only one who is listening to our object is the web UI. If you open the web UI, you should see the `altitude` member increasing by one every second.

```note
Now that we have an infinite loop in our application, we no longer need to specify `-a` to our `corto run` command, but it can't hurt.
```

### Observing updates
Object events are an important part of how components in corto communicate with each other. There are multiple mechanisms in corto that let you see these events. The first one we'll take a look at are **observers**.

Observers let us listen to events from objects, even if we did not create the object. This can be very useful when our code is ran as a component in another application (like the REST- websocket service). To create an observer, add the following code to `cortomain`, before the while loop:

```c
corto_observe(CORTO_UPDATE, my_drone).callback(on_event);
```
This creates a new `observer` that triggers on updates from our `my_drone` object. When the observer receives an event, it will call the `on_event` function. We have not created it yet, so lets add it before the `cortomain` function:

```c
void on_event(corto_observer_event *event) {
    printf("event received!\n");
}
```
If you run this code, you should see this in your terminal:

```demo
event received!
event received!
event received!
...
```
Great, we are receiving the updates from our object. However, the message we print to the terminal isn't very useful yet. Lets print the object identifier and the drone altitude:

```c
void on_event(corto_observer_event *event) {
    // event->data is of a generic type. Cast to Drone to get the altitude
    Drone my_drone = event->data;
    printf("event received from %s, altitude = %f\n",
        corto_idof(my_drone),
        my_drone->altitude);
}
```
The code should now output:

```demo
event received from my_drone, value = 1.000000
event received from my_drone, value = 2.000000
event received from my_drone, value = 3.000000
...
```

### Observing multiple objects
A powerful feature of observers is that they can observe multiple objects at the same time. We can do this by listening to a "scope". Each object can have child objects, which are considered to be in the scope of their parent. Every object in corto except for the root has a scope.

We created the `my_drone` object in the `data` scope. We can simply add another object to the data scope like this:

```c
Drone my_2nd_drone = Drone__create(data_o, "my_2nd_drone", 37, 122, 0);
```
The `data_o` variable is defined by corto, and points to the `/data`  object. We created a hierarchy that now looks like this:

```
/data
  |- my_drone
  +- my_2nd_drone
```

Lets change the observer so that it observes drones from the data scope instead of just from `my_drone`:

```c
corto_observe(CORTO_UPDATE|CORTO_ON_SCOPE, data_o).callback(on_event);
```
Note how we changed `my_drone` to `data_o`, and how we added the `CORTO_ON_SCOPE` modifier to the call. If we run this code the output will still be the same because `my_2nd_drone` is not generating any events yet. Lets add code to the while loop that updates `my_2nd_drone`:

```c
corto_update_begin(my_2nd_drone);
my_2nd_drone->altitude += 2;
corto_update_end(my_2nd_drone);
```
When you run this code, you should see the following output:

```demo
event received from my_drone, value = 1.000000
event received from my_2nd_drone, value = 2.000000
event received from my_drone, value = 2.000000
event received from my_2nd_drone, value = 4.000000
...
```

### More observable events
Observers can receive more than just update events. Other possible events are `CORTO_DEFINE` (a new object is created) and `CORTO_DELETE` (an object has been deleted). Add `CORTO_DEFINE` to the observer *event mask* with the or (`|`) operator so that it looks like this: `CORTO_DEFINE|CORTO_UPDATE|CORTO_ON_SCOPE` (the order doesn't matter).

When we run this code, it should produce the following output

```demo
event received from my_2nd_drone, altitude = 0.000000
event received from my_drone, altitude = 0.000000
event received from my_drone, altitude = 1.000000
event received from my_2nd_drone, altitude = 2.000000
...
```
Note how we now got two extra events at the beginning. These are the two define events, and we can tell because the altitude is still zero. There is a better way to determine what kind of event we received. Change the callback function to this:

```c
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
```
When we run this, we should see

```demo
define received from my_2nd_drone, altitude = 0.000000
define received from my_drone, altitude = 0.000000
update received from my_drone, altitude = 1.000000
update received from my_2nd_drone, altitude = 2.000000
```
which is much more useful.

```note
Did you notice that the observer received "define" events, even though it is created after the objects? That is because when you create an observer, it will automatically generate "define" events for existing objects. That way your application does not need to worry about the order in which objects are created.
```

### Filter events by type
Sometimes a scope contains objects of many different types, and we want to only observe objects of a specific type. We can do this by simply adding a type filter to our observer. Change the line that creates the observer to this:

```c
corto_observe(CORTO_UPDATE, my_drone)
    .type("drone/Drone")
    .callback(on_event);
```
The `drone/Drone` string uniquely identifiers our Drone type, which lives in the scope of the `drone` application. This filter will ensure that our observer only receives objects of the `Drone` type. To verify that this works, create an object of another type, like this:

```c
int32_t *i = corto_int32__create(data_o, "i", 10);
```
The `i` object is created in the data scope, so a notification should be delivered to the observer. However, because `i` is not of the drone type (it is of the `int32` type), the observer won't receive it. If you remove the `type` filter, you'll see a define event for `i`.

### Lifecycle hooks
Lifecycle hooks are like observers in that they are callbacks that are executed when an event happens, but instead of observers, lifecycle hooks are defined on a type and apply to all instances of that type. While observers let us dynamically add new behavior to existing objects, lifecycle hooks let us define the behavior of a single object.

Lets add the `define` and `update` lifecycle hook to our `Drone` type. For this we have to modify `model.corto`. Change the definition of the `Drone` type to this:

```corto
class Drone {
    latitude: float64
    longitude: float64
    altitude: float64

    define()
    update()
}
```
Now rebuild the project. You will notice that a new file appears with the name `Drone.c`, which contains the empty function bodies for `define` and `update`:

```
void drone_Drone_define(
    drone_Drone this)
{
    /* Insert implementation */
}


void drone_Drone_update(
    drone_Drone this)
{
    /* Insert implementation */
}
```

These functions will be invoked whenever an instance of `Drone` is created or updated. Lets add some code to the functions so we can see when they are called:

```c
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
```
When you run this code, you should see the following output:

```demo
drone 'my_drone' defined!
drone 'my_2nd_drone' defined!
define received from my_2nd_drone, altitude = 0.000000
define received from my_drone, altitude = 0.000000
update received from my_drone, altitude = 1.000000
drone 'my_drone' updated!
...
```
Note the order in which things happen:

- The `define` lifecycle hook is called for both drones
- The `define` observer event is received for both drones
- The `update` observer event is received for `my_drone`
- The `update` lifecycle hook is called for `my_drone`

At first this doesn't seem consistent. Why did we receive the `define` lifecycle hook *before* the observer event, but the `update` lifecycle hook *after* the observer event? The explanation is simple:

The observer was created *after* we created the objects, and the `define` lifecycle hook was invoked when we created the objects. If we create the observer *before* we create the objects, the `define` hook would be invoked *after* the observer event, just like the `update` events.

### Lifecycle pre-hooks
We saw how lifecycle callbacks are invoked *after* the event is delivered to observers. Sometimes you'll want to run code *before* the event is delivered to observers. A possible usecase for this is that you want to check the value of an object before the rest of the system is made aware of an update. This can be accomplished with "pre-hooks".

Pre-hooks are lifecycle hooks that are invoked *before* events are delivered. Lets look at the pre-hooks for the `define` and `update` events: `construct` and `validate`. To add them to our class, change the definition of the type to this in the `model.corto` file:

```corto
class Drone {
    latitude: float64
    longitude: float64
    altitude: float64

    construct() int16
    define()
    validate() int16
    update()
}
```
Note how the `construct` and `validate` callbacks have an extra `int16`. This is their return type, and through the return value we can let corto know whether we think the object is ok. If we rebuild the application, the `drone_Drone_construct` and `drone_Drone_validate` functions are added to the `Drone.c` file. Implement them like this:

```c
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
        printf("drone %s has an invalid altitude!", corto_idof(this));
        return -1;
    }
    return 0;
}
```
To make things interesting, lets change the code in `main.c` that increases the value of `my_drone` to:

```c
my_drone->altitude --;
```
Can you guess what will happen? The output of the program should look like this:

```demo
drone my_drone constructed!
drone 'my_drone' defined!
drone my_2nd_drone constructed!
drone 'my_2nd_drone' defined!
define received from my_2nd_drone, altitude = 0.000000
define received from my_drone, altitude = 0.000000
error src/Drone.c:29 drone my_drone has an invalid altitude!
update received from my_2nd_drone, altitude = 2.000000
drone 'my_2nd_drone' updated!
```
Note how an error was thrown, and the observer **did not receive an update** for `my_drone`. In our validate function, we returned `-1` if altitude is lower than 0, and a non-zero return value signals to corto that the object value is invalid! Invalid values are not propagated to observers.

Note that the `construct` method also has a return value. If we return a non-zero value in the `construct` callback, our `Drone__create` function will fail, and it will return `NULL` (try it out!).

### Wrapping up
This concludes the first section of the getting started guide! You can get the example code of everything up to this section here: https://github.com/cortoproject/guide

## Going data-centric
