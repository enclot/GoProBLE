#ifndef __GOPRO_BLE_H__
#define __GOPRO_BLE_H__

#include <Arduino.h>
#include <NimBLEDevice.h>

class GoProBLE
{

public:
    typedef void (*GoproFoundCB)(NimBLEScanResults);

    GoProBLE();

    void scanAsync(const char *name, const uint32_t scanDuration);
    void scanAsync(const uint32_t scanDuration);

    bool connect();
    void disconnect(); //not working

    bool isFound();
    bool isConnected();
    bool isSystemBusy();

    bool shutterOn();
    bool shutterOff();

    uint8_t getBatteryPercentage();
    bool isSystemHot();

    bool checkBatteryPercentageAsync();
    bool checkSystemBusyAsync();
    bool checkSystemHotAsync();
    bool checkLowTempAlertAsync(); //not working
    bool checkDateTimeAsync();

    bool enableQueryResponse();

private:
    bool secureConnection = false;
    uint8_t systemBusy = 1;
    uint8_t systemHot = 0;
    uint8_t batteryPercentage = 0;

    bool writeCommand(uint8_t *cmd, size_t len);
    bool checkQueryAsync(uint8_t *cmd, size_t len);

    static NimBLEAdvertisedDevice *advertisedGopro;
    static void scanEndedCB(NimBLEScanResults results);

    void nonStaticCB(NimBLEScanResults results);

    static NimBLEClient *pClient;

    void notifyedQueryCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
};

#endif