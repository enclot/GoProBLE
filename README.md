# GoProBLE

This is a library for connecting GoPro and Arduino BLE.
It has been tested with GoPro Hero6 and M5StickC.

You can probably connect to other Arduino with BLE function.

Tested with:

- GoPro HERO10 Black
- GoPro MAX
- GoPro HERO9 Black

GoProとArduinoをBLEで接続するためのライブラリです。
GoPro Hero6とM5StickCで確認しています。

ほかのBLE機能のあるArduinoでも多分接続できます。

## What you can do

| Command            | Description                      |
| ------------------ | -------------------------------- |
| Set shutter        | Shutter: on                      |
| Set shutter        | Shutter: off                     |
| System busy        | Is the camera busy?              |
| battery percentage | Internal battery level (percent) |
| change modes       | press side button to change mode from photo>video or vice versa |

## Preparation

In order to pair it with GoPro, you have to put it in the pairing waiting state by GoPro's operation and connect it. Once paired, you won't need to do it again.

1. Put the GoPro in the same state as when connecting the app, waiting for pairing.
2. Connect from Arduino
    * Should say Pairing successful, check arduino logs

GoProとペアリングするためにGoPro側の操作でペアリング待ち状態にして接続しなければなりません。一度ペアリングすると次からは必要なくなります。

1. GoProをアプリを接続するときと同様にペアリング待ち状態にする
2. Arduinoから接続する
    * GoPro側では特に何も表示されないので、Arduinoのコンソールなどで接続の確認をしてください

## Dependence

* NimBLE-Arduino library

https://github.com/h2zero/NimBLE-Arduino

* M5StickC library

https://github.com/m5stack/M5StickC

## Reference

https://gopro.github.io/OpenGoPro/ble

