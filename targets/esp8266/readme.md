### How to build JerryScript for ESP8266

#### 1. Checkout

```
# assume you are in jerryscript folder
git checkout SZBK
```

#### 2. SDK

Follow [this](./docs/ESP-PREREQUISITES.md) page to setup build environment


#### 3. Building JerryScript

```
# assume you are in jerryscript folder
make -f ./targets/esp8266/Makefile.esp8266
```

#### 4. Flashing for ESP8266 12E
Follow
[this](http://www.kloppenborg.net/images/blog/esp8266/esp8266-esp12e-specs.pdf) page to get details about this board.

```
make -f ./targets/esp8266/Makefile.esp8266 flash
```

Default USB device is `/dev/ttyUSB0`. If you have different one, give with `USBDEVICE`, like;

```
USBDEVICE=/dev/ttyUSB1 make -f ./targets/esp8266/Makefile.esp8266 flash
```

### 5. Running

* power off
* connect GPIO2 with serial of 470 Ohm + LED and to GND
* power On

LED should blink on and off every second

#### 6. Cleaning

To clean the build result:

```
make -f ./targets/esp8266/Makefile.esp8266 clean
```

To clean the board's flash memory:
```
make -f ./targets/esp8266/Makefile.esp8266 erase_flash
```


### 7. Optimizing initial RAM usage (ESP8266 specific)
The existing open source gcc compiler with Xtensa support stores const(ants) in
the same limited RAM where our code needs to run.

It is possible to force the compiler to store a constant into ROM and also read it from there thus saving RAM.
The only requirement is to add `JERRY_CONST_DATA` attribute to your constant.

For example:

```C
static const lit_magic_size_t lit_magic_string_sizes[] =
```

can be modified to

```C
static const lit_magic_size_t lit_magic_string_sizes[] JERRY_CONST_DATA =
```

That is already done to some constants in jerry-core. E.g.:

- vm_decode_table
- ecma_property_hashmap_steps
- lit_magic_string_sizes
- unicode_letter_interv_sps
- unicode_letter_interv_len
- unicode_non_letter\_ident_
- unicode_letter_chars
