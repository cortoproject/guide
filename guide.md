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

```c
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

```c
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


## Going data centric
[Become a data centric programmer]

Object oriented programming (OOP) changed the way we write and organize code, so that we can write larger, more complex and more maintainable code. While not perfect, it has proven to be an extremely versatile and intuitive way to describe application behavior.

When OOP is used for describing behavior *between* applications however, like with WSDL or CORBA, we run into some limitations. For one, when we invoke a method on an object, the object *must* be available. Inside an application that is a trivial requirement, but in a distributed system this results in a web of dependencies between services, and can make a system extremely fragile.

Corto's goal is to provide a framework that can automatically synchronize objects between applications in a distributed system without requiring the application developer to be aware of how and when this takes place. For this, we had to invent a new way of writing and organizing code, one that fits better with how distributed systems work. We didn't want to get rid of OOP, as it is the devil we know, and there are many good ideas and concepts that don't need reinvention.

When we designed corto, we therefore drew inspiration from object oriented programming, realtime distributed systems and, perhaps surprisingly, game engines. Game engines have long perfected the design patterns to build realtime virtual worlds, which is exactly what real-life autonomous applications need: build a virtual version of the real world, and use it to make quick decisions.

We think that by taking the best parts from OOP, distributed systems and game engines, we too have perfected how to build seamless distributed applications to a point where it is easy enough for any developer, and powerful enough to build complex distributed systems. We call this new paradigm **data centric programming**. Ready to see what it's all about? Let's get **data centric**!

### Modeling data, revisited
Every data centric application starts with a data model. A data model is how we describe the part of the world that is relevant to our application. For example, if our application is a self driving vehicle, we will want to model things like a map, pedestrians, other vehicles and so on.

Because the world is an intricate place, we invested a lot of time into designing a type system that lets us model anything from the real world in high fidelity, while not being overly complex or constraining. Lets take a closer look at the different aspects of the type system.

#### Meta types
In corto, types are just like regular objects. Regular objects, as we've seen before, have a type, like the `Drone` type of our `my_drone` object. If types are objects, this means that types also must have a type. We call the "type of a type" a **meta type**, and they are the fabric of the corto type system.

An example of a meta type is `struct`. Struct is a type that, when instantiated, creates another type. Take this example:

```corto
struct Point {
    x, y: int32
}
```
This creates a new type called `Point`. The type of `Point` is `struct`. We can now use `Point` to create a new object:

```corto
Point my_point = {x:10, y:20}
```
The `my_point` object is not a type. If we try to do this, we get an error:

```corto
my_point my_obj = {x:10, y:20}
```

```demo
error model.corto:9:1 object '/my_point' is not a type
```

That is because the instances of `Point` are not types, they are regular objects. `Point` is a regular type, *not* a meta type.

The type system is Corto's most powerful tool. Understanding the meta type - type - object relationship is the first step towards unleashing that power.

#### Reference types
The corto type system differentiates between *value types* and *reference types*. They are implemented in a way that is comparable to C#. A variable of a reference type contains a reference to an object, whereas a value type contains the value itself. To see how they are different, lets create a `model.corto` file with the following types:

```corto
struct Point {
    x, y: int32
}

struct Line {
    start, stop: Point
}

Point point_1: 10, 20
Point point_2: 30, 40
Line my_line = {point_1, point_2}
```
Lets take a closer look at how this code is interpreted. For this, we will open the **corto shell**. This is a command-line utility that lets us browse the object store. To start the shell with our model, run:

```demo
corto sh model.corto
```
To inspect the `my_line` object, simply type `my_line` in the shell. You should see the following output:

```demo
< / > my_line
name:         my_line
parent:       /
owner:        <this>
state:        valid
attributes:   named|writable|observable|persistent
type:         /Line
value:        {start:{x:10, y:20}, stop:{x:30, y:40}}
```
The value is the interesting part. We can see that the values of the two point objects are now part of the value of the line object. Now lets see what happens if we change the definition of `Point` to a class:

```corto
class Point {
    x, y: int32
}
```
Lets see what `my_line` now looks like when we reload it. Exit the shell with the `exit` command, and restart it with the same command. The `my_line` object now looks like this:

```demo
< / > my_line
name:         my_line
parent:       /
owner:        <this>
state:        valid
attributes:   named|writable|observable|persistent
type:         /Line
value:        {start:/point_1, stop:/point_2}
```
Note that now the value of `point_1` and `point_2` are *not* part of `my_line`! Instead, `my_line` contains references to the `point_1` and `point_2` objects. This is similar to object references in Java or C#, or a pointer in C.

A few examples of built-in reference (meta) types are:

- interface
- class
- procedure
- object

All built-in primitive and collection types are by default value types. The corto type system does however let you create your own types using meta-types. This way, you can for example create your own primitive reference type. Consider the following code:

```corto
int i32_ref: width_32, reference: true

struct Point {
    x, y: i32_ref
}

i32_ref a: 10
i32_ref b: 20
Point my_point = {a, b}
```
If we inspect the value of `my_point` in the shell, we will see this:

```demo
< / > my_point
name:         my_point
parent:       /
owner:        <this>
state:        valid
attributes:   named|writable|observable|persistent
type:         /Point
value:        {start:/a, stop:/b}
```
If we replace `i32_ref` in the `Point` type with the built-in value type `int32` the value looks like this:

```demo
value:        {start:10, stop:20}
```

#### Void types
The void type is used for objects that have no values. A typical use of a `void` type is when creating a container for child objects:

```corto
void parent {
    int32 child_1: 10
    int32 child_2: 20
}
```
Another use of the `void` type is for procedure objects that have no return value. This is an example of a function with a void return type:

```corto
print(string message) void
```

A special use for a `void` type is a *void reference*. Void reference types describe values that can contain any type of object. The builtin `object` type is an example of a void reference type. This is an example of how it can be used:

```corto
int32 my_int: 10
object my_ref: my_int
```
If we inspect this object with the corto shell, we will see the following value:

```demo
value:        /my_int
```

#### Primitive types
Primitive types represent the smallest unit of data that can be modeled. You cannot break up a primitive value into smaller parts without changing its meaning. Take for example the string "Hello World" and break it up into "Hello" and "World": the two separate strings no longer carry the same information as the full string.

The primitive meta types are:

Meta type | Value example | Description
-----|---------|------------
boolean | `true`, `false` | Boolean values
binary | `0x10`, `0xFF` | Values that don't swap endianness when serialized
character | `a`, `\0` | Single characters
int | `-10`, `+10` | Signed integers
uint | `10`, `20` | Unsigned integers
float | `10.5`, `10e-1`, `-10.5` | Floating point
text | `"Hello World"`, `null` | Bounded or unbounded strings
enum | `Red`, `Blue` | Signed integer that is one of a list of constants
bitmask | `0`, `Sunny`, `Sunny|Hot` | Unsigned integer that combines constants in a bitmask

With these primitive **meta types** we can create our own primitive **types**. For example if we want to create a 32-bit signed integer, we can do this:

```corto
int i32: width_32
```
We can now use this type in a `.corto` file like this:

```corto
i32 my_int: 10
```
Or to create an object in our application code:

```c
int32_t *my_int = i32__create(data_o, "my_int", 10);
```

```note
Notice the similarity in syntax between creating the i32 type, and the my_int object. In corto, types are also objects, and we use the same syntax to describe both types and objects!
```

Corto comes conveniently comes with a number of predefined primitive types:

Meta type | Types
-----|---------
boolean | `bool`
binary | `octet`, `word`
character | `char`
int | `int8`, `int16`, `int32`, `int64`
uint | `uint8`, `uint16`, `uint32`, `uint64`
float | `float32`, `float64`
text | `string`

You can use these types in a `.corto` file like this:

```corto
bool my_bool: true
```
Or to create an object in our application code:

```c
bool *my_bool = corto_bool__create(data_o, "my_bool", true);
```

```note
Notice the "corto" prefix here. All builtin types are prefixed by "corto" to avoid name collisions. Types that you define in your applications and packages will be prefixed with the application or package name.
```

Enumerations and bitmasks are typically used directly to create new types, like this:

```corto
enum Color {
    Red, Orange, Yellow, Green, Blue
}

bitmask Weather {
    Sunny, Cloudy, Hot, Cold, Dry, Humid
}

Color my_color: Yellow
Weather my_weather: Sunny | Humid
```

#### Composite types
Composite types can be composed out of other types. We've already seen some examples of composite types, like the `struct` and `class` type. Each composite type contains a list of members, which have an identifier, and the member type. There are a number of composite meta types in corto:

Meta type | Description
----------|------------
interface | Reference type for describing abstract interfaces
struct    | Value type that describes composite value.
class     | Reference type that describes reference composite objects.
union     | Value type that can change at runtime
procedure | Reference type that describes types for callable objects (functions)

##### Interface
An interface is used to describe an abstract interface. Interfaces can only contain overridable methods. Interfaces are not directly instantiated but instead can be implemented by a class. A variable of an interface type may contain a reference of an object of a type that implements the interface. Consider this example:

```corto
interface Vehicle {
    move(int32 x, int32 y)
}

class Car: implements:[Vehicle] {
    move(int32 x, int32 y)
}

Car my_car = {}

// Valid, because Car implements Vehicle
Vehicle my_vehicle: my_car
```

##### Struct
A struct is a composite value type. Structs can have both members and methods as shown in this snippet:

```corto
struct Point {
    x: int32
    y: int32

    add(Point p)
}
```

##### Class
A class is a composite reference type. It inherits the same capabilities from a struct, but in addition supports implementing interfaces and lifecycle hooks, as shown in this snippet:

```corto
class Car: implements:[Vehicle] {
    lat: float64
    long: float64

    construct() int16
    destruct()

    move(int32 x, int32 y)
}
```
The reason that lifecycle hooks are only implemented on classes and not on structs, is because a class instance is *guaranteed* to be an object (it is a reference type). Because structs are value types, corto doesn't know whether a value is an object, and thus lifecycle hooks do not apply.

##### Union
A union is a composite type where only one of the members (or "cases") is "active" at any point in time. Union values can change which member is active at runtime. Unions only occupy as much memory as the size of their largest member. Here is an example of a union:

```corto
union Value: int32 {
    int: [0], int64
    flt: [1], float64
    str: [2, 3], string
    default other: bool
}
```
Each union value has a discriminator value which must be of an integer or enumeration type. The discriminator value determines which field is active. A union case has a type and a list of discriminator values that apply to the case. In the above example, the type is `int64` when the discriminator is `0`, `float64` when it is `1` etc. The `str` field demonstrates how it is possible to associate multiple discriminators with a case.

If the discriminator does not any of the values associated with any of the cases, the default case is selected. If a union does not have a default case, the value is invalid.

##### Procedure
A procedure is a special kind of meta type that is used to create callable objects (functions). Corto comes with a number of builtin procedure types:

- function
- method
- overridable
- override
- remote
- observer
- subscriber

These types are all instances of the `procedure` meta type. Instances of these types all have in common that they have an argument list and a return type. The argument list is part of the object identifier, which allows procedures to be overloaded.

You can implement your own procedure types, which lets you override how function parameters are derived and leverage Corto's code generators, but this is an advanced topic that we will cover at a later point in time.

#### Collection types
Collection types describe sets of values that all are instances of the same type. Collections can be either bounded or unbounded. Values inside a collection are called elements. Elements can either be identified by a key or an index. A key can be any primitive type. An index must be a positive integer lower than the total element count of a collection value.

Corto supports the following collection meta types (more may be added in the future):

- array
- sequence
- list
- map

The first three types (`array`, `sequence`, `list`) are ordered collection types that are indexed by an integer. The difference between these three is how they can grow, and how they are stored. The `map` type lets the application decide how values are indexed.

##### Array
An array is a fixed-size collection that is allocated as a single block of memory. Arrays feature fast O(1) lookups and are very memory efficient.

```corto
array[int32, 3] my_array = [10, 20, 30]
```
The equivalent C type of this array is `int32_t*`. You can iterate through an array like this:

```c
int i;
for (i = 0; i < 3; i ++) {
    printf("element = %d\n", my_array_o[i]);
}
```

##### Sequence
Sequences are similar to arrays in that they are single blocks of memory that have fast O(1) lookups, but contrary to arrays, sequences can be of dynamic size. Sequences also require an extra indirection which makes them slightly less efficient than arrays. Resizing sequences is possible, but is an expensive operation where potentially all elements have to be copied to a new block of memory.

```corto
sequence[int32] my_sequence = [10, 20, 30]
```
The equivalent C type of a sequence is a struct with a `length` and a `buffer` member. The buffer member has the same type as an array of the same element type would have. For this sequence, the buffer type would be `int32_t*`. An application can iterate through a sequence value like this:

```c
int i;
for (i = 0; i < my_sequence_o->length; i ++) {
    printf("element = %d\n", my_sequence_o->buffer[i]);
}
```

##### List
The list type is a linked list. It has an O(n) lookup (worst case) but is very fast when it comes to inserting, appending or removing elements. Lists are more efficient than sequences when you need to frequently add or remove elements.

```corto
list[int32] my_list = [10, 20, 30]
```
The equivalent C type of a linked list is `corto_ll`. You can iterate through a linked list like this:

```c
corto_iter it = corto_ll_iter(my_list_o);
while (corto_iter_hasNext(&it)) {
    printf("element = %d\n", (uintptr_t)corto_iter_next(&it));
}
```
Note that the return type of `corto_iter_next` is casted to `uintptr_t`. This is because the function returns a `void*`. Because the `int32` type is guaranteed to fit in a pointer-sized variable on any 32bit and 64bit architecture, corto does not allocate memory for the element, for efficiency reasons.

All 32-bit (or smaller) primitive values are stored in a linked list like this. 64 bit values require an extra allocation, even on 64 bit platforms. Composite and collection element types always require an allocation, even if their size is smaller than 64 bit.

Here is another example, but now with the composite type `Point`:

```c
corto_iter it = corto_ll_iter(my_list_o);
while (corto_iter_hasNext(&it)) {
    Point *p = corto_iter_next(&it);
    printf("element = %d, %d\n", p->x, p->y);
}
```

##### Map
A map is a balanced tree which has a key type in addition to an element type. The entries in a map are key-value pairs, where every key in the map has to be unique. Corto uses a red-black tree for the implementation of maps, and has O(log n) complexity for insertion and deletion.

```corto
map[string, int32] my_map = ["foo": 10, "bar": 20, "hello": 30]
```
The equivalent C type of a map is `corto_rb`. Maps can be iterated over the same way as linked lists:

```c
corto_iter it = corto_rb_iter(my_map_o);
while (corto_iter_hasNext(&it)) {
    printf("element = %d\n", (uintptr_t)corto_iter_next(&it));
}
```
The same rules with regards to storing elements of small primitive sizes that apply to linked lists also apply to maps.

#### Iterator types
Iterators let you iterate over a collection one element at a time. They are a first-class citizen of corto, and an important building block in making large amounts of data accessible to an application as iterators can be used to lazily evaluate elements in a collection.

An iterator type can be used like this:

```corto
list[int32] my_list = [10, 20, 30]
iterator[int32] my_iterator: my_list
```
The equivalent C type of an iterator is `corto_iter`. See the `list` and `map` examples for how to use the `corto_iter` type.

You can implement your own iterators by implementing the `next` and `hasNext` callbacks of the `corto_iter` type. The C signatures of these functions are:

```c
void* next(corto_iter *iter);
bool hasNext(corto_iter *iter);
```
The following example shows how to build an iterator that returns the first 50 numbers of the fibonacci sequence. Note how we do not store the full sequence in memory, but instead compute elements in the sequence as we are iterating:

```c
uint64_t fibo(uint64_t n) {
    if (n == 0) return 0;
    uint64_t previous = 0, current = 1, i;
    for (i = 1; i < n; ++i) {
        uint64_t next = previous + current;
        previous = current;
        current = next;
    }
    return current;
}

struct iter_data {
    uint64_t current;
    uint64_t value;
};

bool hasNext(corto_iter *iter) {
    if (!iter->data) {
        iter->data = corto_calloc(sizeof(struct iter_data));
    }
    struct iter_data *data = iter->data;
    return data->current < 50; /* Just return the first 50 */
}

void* next(corto_iter *iter) {
    struct iter_data *data = iter->data;
    data->current ++;
    data->value = fibo(data->current);
    return &data->value;
}

void release(corto_iter *iter) {
    /* Cleanup resources when done with iterating */
    free(iter->data);
}

int cortomain(int argc, char *argv[]) {
    corto_iter it = { .next = next, .hasNext = hasNext, .release = release };

    while (corto_iter_hasNext(&it)) {
        uintptr_t *v = corto_iter_next(&it);
        printf("%lu\n", *v);
    }

    return 0;
}
```

#### Any type
The `any` type can be used to represent any kind of value. The type of the value may be changed at runtime. The type represents a type-value tuple, so that an application can at any point check what the current type of the any value is.

The any type can be used like this:

```corto
any my_any: 20
```
The equivalent C type of an `any` type is a struct with  a `type` and a `value` member. A third `owned` member indicates whether the `any` value "owns" the embedded value, or whether this value is part of another object. If `owned` is false, cleaning up the `any` value will not attempt to free the value.

```c
struct corto_any {
    corto_type type;
    void *value;
    bool owned;
}
```
It can be used like this:

```c
corto_any my_any = {
    .type = corto_int32_o,
    .value = corto_ptr_new(corto_int32_o)
    .owned = true
}

*(uint32_t*)my_any.value = 30;
```
To clean up the resources held by an any value, simply do:

```c
corto_ptr_deinit(&my_any, corto_any_o);
```
