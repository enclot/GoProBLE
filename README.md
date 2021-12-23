# GoProBLE



This is a library for connecting GoPro and Arduino BLE.

You can probably connect to other Arduino with BLE function.



| Arduino    |      | GoPro       |      |
| ---------- | ---- | ----------- | ---- |
| M5StickC   | x    | Hero6 Black | x    |
| M5AtomLite | x    | Hero9       | x    |
|            |      |             |      |



## Change Log

* enableStatusResponse() has changed its name to enableQueryResponse(). 
* scanAsync() can specify a gopro ble name.





## What you can do

| Command            | Description                      |
| ------------------ | -------------------------------- |
| Set shutter        | Shutter: on                      |
| Set shutter        | Shutter: off                     |
| Sleep              | Put camera to sleep              |
| System busy        | Is the camera busy?              |
| battery percentage | Internal battery level (percent) |
|                    |                                  |



## Preparation



In order to pair it with GoPro, you have to put it in the pairing waiting state by GoPro's operation and connect it. Once paired, you won't need to do it again.

1. Put the GoPro in the same state as when connecting the app, waiting for pairing.
2. Connect from Arduino
   * Hero6
     * Nothing is displayed on the GoPro, so please check the connection with the Arduino console.
   * Hero9
     * Once paired, the camera should beep and display “Connection Successful”.



GoProとペアリングするためにGoPro側の操作でペアリング待ち状態にして接続しなければなりません。一度ペアリングすると次からは必要なくなります。

1. GoProをアプリを接続するときと同様にペアリング待ち状態にする
2. Arduinoから接続する
    * Hero6ではGoPro側では特に何も表示されないので、Arduinoのコンソールなどで接続の確認をしてください





## Dependence

* NimBLE-Arduino library

https://github.com/h2zero/NimBLE-Arduino



* M5StickC library

https://github.com/m5stack/M5StickC





## Reference

https://gopro.github.io/OpenGoPro/ble

