# Aktualizr-nano

Device management and OTA update client for MCUs based on FoundriesFactory.
Current implementation depends on MCUXpresso SDK.

A sample application and usage instructions can be found at
https://github.com/foundriesio/dm-ak-nano-mcuxpresso

*The current code is not intended for production use, and is currently a mere
prototype illustrating the possibility of using Foundries.io services in MCUs.*

Building and running built-in tests
-----------------------------------
Inside the cloned directory:
```
git submodule update --init --recursive
cd tests
cmake -S test -B build
cd build
make
make test
make coverage
```
