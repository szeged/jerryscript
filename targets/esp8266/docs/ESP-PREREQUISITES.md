#### 1. Directory structure
The folder tree related would look like this.

```
harmony
  + jerryscript
  |  + targets
  |      + esp8266
  + esp-open-sdk
  |
  + esp-open-rtos
```
##### 2. Dependencies

```
sudo apt-get install make unrar-free autoconf automake libtool gcc g++ gperf \
flex bison texinfo gawk ncurses-dev libexpat-dev python-dev python python-serial \
sed git unzip bash help2man wget bzip2 libtool

pip install --user pyserial

```
##### 3. SDK
To clone the SDK and build the crosstool:
```
#Assume you are in the harmony folder
git clone --recursive https://github.com/pfalcon/esp-open-sdk.git
cd esp-open-sdk
git checkout 5518fb6116c35a02ccb9a87260bb194a57cb429e
make toolchain esptool libhal STANDALONE=n
```

add path to environment file such as `.profile`
```
PATH=$PWD/xtensa-lx106-elf/bin:$PATH
```

##### 4. RTOS
To clone the RTOS:
```
#Assume you are in the harmony folder
git clone --recursive https://github.com/szeged/esp-open-rtos.git
git checkout SZBK
export RTOS_PATH=$PWD
```
