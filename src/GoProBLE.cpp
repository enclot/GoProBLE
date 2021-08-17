#include <GoProBLE.h>

NimBLEAdvertisedDevice *GoProBLE::advertisedGopro = nullptr;
NimBLEClient *GoProBLE::pClient = nullptr;

GoProBLE::GoProBLE()
{
}

void GoProBLE::begin()
{
}

void GoProBLE::scanAsync(const uint32_t scanTime)
{
    NimBLEDevice::init("");

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

    NimBLEScan *pScan = NimBLEDevice::getScan();

    class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
    {
        void onResult(NimBLEAdvertisedDevice *advertisedDevice)
        {
            Serial.print("Advertised Device found: ");
            Serial.println(advertisedDevice->toString().c_str());

            if (advertisedDevice->getName().find("GoPro") == 0)
            {
                advertisedGopro = advertisedDevice;
                Serial.println("Stop Scan");
                NimBLEDevice::getScan()->stop();
            }
        };
    };

    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());

    pScan->setInterval(2000);
    pScan->setWindow(2000);

    pScan->setActiveScan(true);

    // auto cb = std::bind(&GoProBLE::nonStaticCB, this, std::placeholders::_1);
    //TODO
    pScan->start(scanTime, scanEndedCB);
}

bool GoProBLE::connect()
{
    if (pClient == nullptr)
    {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
        {
            Serial.println("Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();
        Serial.println("New client created");

        // TODO
        // pClient->setClientCallbacks(&clientCB, false);
        pClient->setConnectionParams(12, 12, 0, 51);
        pClient->setConnectTimeout(4);

        if (!pClient->connect(advertisedGopro))
        {
            NimBLEDevice::deleteClient(pClient);
            Serial.println("Failed to connect, deleted client");
            return false;
        }

        // Serial.println(pClient->getMTU());
    }
    else
    {
        //reconnect
        if (NimBLEDevice::getClientListSize())
        {
            pClient = NimBLEDevice::getClientByPeerAddress(advertisedGopro->getAddress());
            if (pClient)
            {
                if (!pClient->connect(advertisedGopro, false))
                {
                    Serial.println("Reconnect failed");
                    return false;
                }
                Serial.println("Reconnected client");
            }
            else
            {
                pClient = NimBLEDevice::getDisconnectedClient();
            }
        }
    }

    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(pClient->getRssi());

    pClient->secureConnection();

    return true;
}

void GoProBLE::disconnect()
{
}

bool GoProBLE::isFound()
{
    if (advertisedGopro != nullptr)
    {
        return true;
    }
    return false;
}

bool GoProBLE::isConnected()
{
    if (pClient == nullptr)
    {
        return false;
    }
    return pClient->isConnected();
}

bool GoProBLE::isSystemBusy()
{
    return (bool)systemBusy;
}

bool GoProBLE::isSystemHot()
{
    return (bool)systemHot;
}

uint8_t GoProBLE::getBatteryPercentage()
{
    return batteryPercentage;
}

bool GoProBLE::enableStatusResponse()
{
    if (pClient->isConnected())
    {
        NimBLERemoteService *pService = pClient->getService("fea6");
        if (pService != nullptr)
        {
            NimBLERemoteCharacteristic *pChrNotify = pService->getCharacteristic(NimBLEUUID("b5f90077-aa8d-11e3-9046-0002a5d5c51b"));

            if (pChrNotify->canNotify())
            {
                Serial.println("enable notify");

                auto cb = std::bind(&GoProBLE::notifyedStatusCB, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

                if (!pChrNotify->subscribe(true, cb))
                {
                    /** Disconnect if subscribe failed */
                    pClient->disconnect();
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool GoProBLE::checkStatusAsync(uint8_t *cmd, size_t len)
{
    if (pClient->isConnected())
    {
        NimBLERemoteService *pService = pClient->getService("fea6");
        if (pService != nullptr)
        {
            NimBLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic(NimBLEUUID("b5f90076-aa8d-11e3-9046-0002a5d5c51b"));
            //      Serial.println(pCharacteristic->getUUID().toString().c_str());

            if (pCharacteristic != nullptr)
            {
                if (pCharacteristic->canWrite())
                {
                    return pCharacteristic->writeValue((const uint8_t *)cmd, len);
                }
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool GoProBLE::checkSystemHotAsync()
{

    uint8_t cmdHot[] = {0x02, 0x13, 0x06};
    return checkStatusAsync(cmdHot, 3);
}

bool GoProBLE::checkLowTempAlertAsync()
{
    //

    // uint8_t cmdCold[] = {0x02, 0x13, statusid};
    // statusid++;
    // Serial.println(statusid);
    // return checkStatusAsync(cmdCold, 3);
}

bool GoProBLE::checkBatteryPercentageAsync()
{

    uint8_t cmdCold[] = {0x02, 0x13, 0x46};
    return checkStatusAsync(cmdCold, 3);
}

bool GoProBLE::checkSystemBusyAsync()
{
    Serial.println("check system busy");
    uint8_t cmdbusy[] = {0x02, 0x13, 0x08};
    return checkStatusAsync(cmdbusy, 3);
}

bool GoProBLE::checkDateTimeAsync()
{
    uint8_t cmdbusy[] = {0x01, 0x13};
    return checkStatusAsync(cmdbusy, 2);
}

void GoProBLE::scanEndedCB(NimBLEScanResults results)
{
    Serial.println("Scan Ended.cpp static");

    //TODO
    //Callback
}

// void GoProBLE::nonStaticCB(NimBLEScanResults results)
// {
//     Serial.println("Scan Ended.cpp non static");
// }

void GoProBLE::notifyedStatusCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
                                size_t length, bool isNotify)
{
    Serial.print("notify status callback    ");
    Serial.println(length);

    uint8_t statusID = pData[3];

    switch (statusID)
    {
    case 8:                    //5,19,0,8,1,0,
        systemBusy = pData[5]; //1:true, 0:false
        break;
    case 70:                          //5, 19, 0, 70, 1, 100 (response)
        batteryPercentage = pData[5]; //0-100
        break;
    case 6:                   //5, 19, 0, 6, 1, 0 (response)
        systemHot = pData[5]; //1:true, 0:false
        break;
    }

    for (int i = 0; i < length; i++)
    {
        Serial.print(pData[i]);
        Serial.print(", ");
    }
    Serial.println();
}

bool GoProBLE::writeCommand(uint8_t *cmd, size_t len)
{

    if (pClient->isConnected())
    {
        NimBLERemoteService *pService = pClient->getService("fea6");
        if (pService != nullptr)
        {
            // Serial.println(pService->getUUID().to128().toString().c_str());
            NimBLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic(NimBLEUUID("b5f90072-aa8d-11e3-9046-0002a5d5c51b"));
            // Serial.println(pCharacteristic->getUUID().toString().c_str());

            if (pCharacteristic != nullptr)
            {
                if (pCharacteristic->canWrite())
                {
                    return pCharacteristic->writeValue((const uint8_t *)cmd, len);
                }
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool GoProBLE::shutterOn()
{
    uint8_t cmd[] = {0x03, 0x01, 0x01, 0x01};
    if (writeCommand(cmd, 4))
    {
        Serial.println("Wrote Shutter:On");
        return true;
    }
    return false;
}

bool GoProBLE::shutterOff()
{
    uint8_t cmd[] = {0x03, 0x01, 0x01, 0x00};
    if (writeCommand(cmd, 4))
    {
        Serial.println("Wrote Shutter:Off");
        return true;
    }
    return false;
}
