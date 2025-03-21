# afb-zephyr-samples


This repository is containing samples showing how to use the zephyr 
module `afb-zephyr`.

The zephyr module `afb-zephyr` is available [here][1] and aims to provide the
Application Framework Bindings capabilities in a Zephyr environment.

The Application Framework Bindings is the framework used in the 
[Redpesk OS][2] to deploy, secure and connect several services.

The official Application Framework Bindings documentation is available 
[here][3],
most of these references can be used as is in the Zephyr environment using 
the `afb-zephyr` module.

Some integration interfaces between the Application Framework Bindings libraries 
and zephyr applications are also provided in the `afb-zephyr` module, 
they can be found [here][4].

You can find it's zephyr compatibility table and installation guide at
[here][1].


## Samples explanation

The given samples are intended to show several aspect of integration of the
framework in zephyr.

Each of this example is in its own directory and has a README file.

### dual-local-api

This example shows communication between 2 micro service's APIs,
each one calling hello of the other.

This example doesn't need any specific board.

### call-extern-api

This example shows periodical call to the verb of an API being on remote server.

This example needs a board with an IP connection.

### remote-io-control

This example show implementing an API that controls the state
of a led and that sends event when user button is pressed.

This example needs a board with an IP connection, a led
and a user button.

### llext-modules

This example show implementing two API built as LLEXT. These API
are simply reply to hello.

This example needs a board with an IP connection.


## Build and flash a sample

1. Install zephyr and `afb-zephyr` module

To build one of these samples, you need to clone needed ressources as explained 
[here][5].

2. Include `afb-zephyr` in your build environment

At this time, our sample applications can find `afb-zephyr` in two different ways:
- Using the `AFB_ZEPHYR_DIR` environnement variable
- Searching for the `afb-zephyr` folder at `~/zephyrproject/afb-zephyr`

Multiple ways to include `afb-zephyr` in your build environment are also described 
[here][6] and can be used with our samples.

3. Build and flash your application

You can build and flash these samples application as you would usually do for 
any zephyr application.

```bash
# Source your Zephyr env if needed
source ~/zephyrproject/zephyr/zephyr-env.sh

# Move to the folder of the samples you want to build
cd dual-local-api

# Build the application as you would usually do for your target board
west build -p always -b {board_name} .

# Flash your application as you would usually do for your target board
west flash
```

To see the samples output, please open a serial terminal (minicom, putty, etc.) 
and connect to your board using default serial port settings.

[1]: https://github.com/redpesk-core/afb-zephyr
[2]: https://docs.redpesk.bzh/docs/en/master/redpesk-os/os-overview/docs/overview-redpesk-os.html
[3]: https://docs.redpesk.bzh/docs/en/master/developer-guides/afb-binding-overview.html 
[4]: https://github.com/redpesk-core/afb-zephyr/tree/master/src
[5]: https://github.com/redpesk-core/afb-zephyr?tab=readme-ov-file#clone-needed-ressources
[6]: https://github.com/redpesk-core/afb-zephyr?tab=readme-ov-file#add-afb-zephyr-external-module-to-your-build-environment
