#ifndef __GOPRO_BLE_H__
#define __GOPRO_BLE_H__

#include <Arduino.h>
#include <NimBLEDevice.h>

class GoProBLE
{

public:
    typedef void (*GoproFoundCB)(NimBLEScanResults);
    //typedef std::function<void(NimBLEScanResults)> GoproFoundCB;
    //using GoproFoundCB = std::function<void(NimBLEScanResults results)>;

    // typedef std::function<void(NimBLEScanResults)> GoproFoundCB;

    GoProBLE();

    void begin();

    //void scanAsync(const uint32_t scanTime, GoProBLE::GoproFoundCB callback);
    void scanAsync(const uint32_t scanTime);

    bool connect();

    void disconnect();

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
    bool checkLowTempAlertAsync(); //notworking
    bool checkDateTimeAsync();

    bool enableStatusResponse();

private:
    uint8_t systemBusy = 1;
    uint8_t systemHot = 0;
    uint8_t batteryPercentage = 0;

    // uint8_t statusid = 1;

    bool writeCommand(uint8_t *cmd, size_t len);
    bool checkStatusAsync(uint8_t *cmd, size_t len);

    static NimBLEAdvertisedDevice *advertisedGopro;
    static void scanEndedCB(NimBLEScanResults results);

    // void nonStaticCB(NimBLEScanResults results);

    static NimBLEClient *pClient;
    // static ClientCallbacks clientCB;

    void notifyedStatusCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
};

#endif