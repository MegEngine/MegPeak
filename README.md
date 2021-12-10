# MegPeak
Megpack is a tool for testing processor peak computation, now support
arm, x86 and GPU driven by OpenCL processor.

## BUILD
MegPeak only support [Cmake](https://cmake.org/) build system and require Cmake version upper than 3.15.2, you can compiler the MegPeak fellow the step:

* clone or download the project
* choose a test plantform
    - if you will test x86 processor in linux OS
        - a gcc or clang compier should find by camke through PATH env
    - if you will test arm processor in android OS
        - a [ndk](https://developer.android.com/ndk) is required
            - download the NDK and extract to the host machine
            - set the NDK_ROOT env to the path of extracted NDK director
* if you dest test OS is android，run the android_build.sh to build it
    * build for armv7
        ```bash
        ./android_build.sh -m armeabi-v7a
         ```
    * build for arm64
        ```bash
        ./android_build.sh -m arm64-v8a
         ```
    * build with OpenCL
        ```bash
        ./android_build.sh -l -m [arm64-v8a, armeabi-v7a]
         ```
* if you dest test OS is linux，if you want enable OpenCL add -DMEGPEAK_ENABLE_OPENCL=ON to cmake command
    ```bash
    mkdir -p build && cd build
    cmake .. [-DMEGPEAK_ENABLE_OPENCL=ON]
    make
     ```
* after build, the executable file megpeak is stored in build drector

### Run
if you compiler the project and get the megpeak, next you can copy or set the megpeak exectuable file to the test machine，and run it and get the help message.
    ```bash
    ./megpeak -h
     ```
* test opencl
    ```bash
    ./megpeak -d opencl
     ```
* test CPU with cpu id
    ```bash
    ./megpeak -d cpu -i 0
     ```
