# Report

**Ruizhe Shi 2021011827 AI10**

*this project is base on **wsl**.*

## How to get dependencies

### ffmpeg

On Windows, please download
`ffmpeg-n4.4-latest-win64-gpl-shared-4.4.zip` from https://github.com/BtbN/FFmpeg-Builds/releases/tag/latest
or https://cloud.tsinghua.edu.cn/d/27439285b1614e3cb745/.
Exact the zip file, rename the folder as `ffmpeg-windows`, and put it under
the `external` directory of the project.

On macOS, please install ffmpeg with HomeBrew:
```
homebrew install ffmpeg
```

On Linux or WSL, please use the package manager of respective system. For Debian
or Ubuntu, the following command should suffice:
```
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev
```

The Tsinghua Cloud link provided above
(https://cloud.tsinghua.edu.cn/d/27439285b1614e3cb745/) also contains additional
video samples and reading material.

### git submodules

After cloning, use the following command to initialize all git submodules
```
git submodule update --init --depth=1
```

The repository uses CMake, after these two steps you should be able to open it with your IDE.

## Screenshots of my programs’ output

![img1](.\img\3.png)

![img2](./img/1.png)

![img3](./img/2.png)

## How you should compile my project and recreate my result

```shell
cd build
cd src
make
./main PATH
```

And then you will see ![img4](./img/4)

just follow the instructions.

## CPU and RAM specification of the computer I finished the microbench mark test

The benchmark is carried on a **laptop computer** with 

**11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz   2.80 GHz** processor, 

**64 GB DDR4 memory at 3200 MT/s**

## My microbenchmark result, and answer to the question given in that section

Running ./bench_sample
Run on (8 X 2803.2 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 1280 KiB (x4)
  L3 Unified 12288 KiB (x1)

Load Average: 0.00, 0.00, 0.00

Benchmark                  Time             CPU   Iterations

TemplateAvgLum       1127723 ns      1127724 ns          626
InheritanceAvgLum    1535133 ns      1535137 ns          452

**Answer:** 

Yes, it differs. TemplateAvgLum runs faster than InheritanceAvgLum because TemplateAvgLum uses template to construct getLuminance() and averageLuminance() while Inheritance uses virtual functions which is time-consuming.

## Any interesting bugs I have encountered, and how did I solve them.

- cmake version is too low：

  the solution is:

  ```shell
  pip install -U cmake==3.21
  CMake Error: Could not find CMAKE_ROOT !!!
  hash -r
  ```

- can not find the lib swscale

  the solution is:

  just fill the find_package() with swscale in the CMakeLists.txt.

- bad dst image pointers

  the solution is:

  after av_frame_alloc(), we should fill the frame.

- unknown memory leaking

  the solution is:

  delete sws_scale; is wrong.

  Instead, we should sws_freeContext(sws_ctx);
