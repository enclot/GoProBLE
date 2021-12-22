#include <GoProBLE.h>

NimBLEAdvertisedDevice *GoProBLE::advertisedGopro = nullptr;
NimBLEClient *GoProBLE::pClient = nullptr;

GoProBLE::GoProBLE()
{
}

//BLE scan specific name
void GoProBLE::scanAsync(const char *name, const uint32_t scanDuration)
{
    NimBLEDevice::init("");

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

    NimBLEScan *pScan = NimBLEDevice::getScan();

    class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
    {
        const char *name;

    public:
        AdvertisedDeviceCallbacks(const char *name)
            : name(name)
        {
        }

        void onResult(NimBLEAdvertisedDevice *advertisedDevice)
        {
            std::string nameString = std::string(name);
            if (advertisedDevice->getName().find(nameString) == 0)
            {
                Serial.print("Advertised Device found: ");
                Serial.println(advertisedDevice->toString().c_str());
                advertisedGopro = advertisedDevice;
                Serial.println("Stop BLE Scan");
                NimBLEDevice::getScan()->stop();
            }
        };
    };

    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks(name));

    pScan->setInterval(2000);
    pScan->setWindow(2000);

    pScan->setActiveScan(true);

    //auto cb = std::bind(&GoProBLE::nonStaticCB, this, std::placeholders::_1);
    //TODO
    //staticでないCallbackでできるようにしたい

    pScan->start(scanDuration, scanEndedCB);
}

//BLE scan
//
void GoProBLE::scanAsync(const uint32_t scanDuration)
{
    scanAsync("GoPro", scanDuration);
}

//scan finished
//static method
void GoProBLE::scanEndedCB(NimBLEScanResults results)
{

    Serial.println("Scan Ended (.cpp) static method");
}
void GoProBLE::nonStaticCB(NimBLEScanResults results)
{
    Serial.println("Scan Ended.cpp non static");
}

bool GoProBLE::connect()
{
    class ClientCallbacks : public NimBLEClientCallbacks
    {
        // void onConnect(NimBLEClient *pClient)
        // {
        //     Serial.println("onConnect");
        // }
        // void onDisconnect(NimBLEClient *pClient)
        // {
        //     Serial.println("onDsconnect");
        // }

        // void onAuthenticationComplete(ble_gap_conn_desc *desc)
        // {
        //     if (!desc->sec_state.encrypted)
        //     {
        //         Serial.println("Encrypt connection failed - disconnecting");
        //         /** Find the client with the connection handle provided in desc */
        //         NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
        //         return;
        //     }
        //     else
        //     {
        //         Serial.println("onAuthenticationComplete");
        //     }
        // };
    };

    if (pClient == nullptr)
    {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
        {
            Serial.println("Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();
        Serial.println("New BLE client created");

        pClient->setClientCallbacks(new ClientCallbacks(), false);
        pClient->setConnectionParams(12, 12, 0, 51);
        pClient->setConnectTimeout(4);

        if (!pClient->connect(advertisedGopro))
        {
            NimBLEDevice::deleteClient(pClient);
            Serial.println("Failed to connect, deleted client");
            return false;
        }
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
    secureConnection = pClient->secureConnection();

    Serial.print(secureConnection);
    Serial.print(" Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(pClient->getRssi());

    return secureConnection;
}

void GoProBLE::disconnect()
{
    //TODO
    //切断処理を実装したい
}

// Advertising Packet is Found
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

//subscribe query response
bool GoProBLE::enableQueryResponse()
{
    if (pClient->isConnected())
    {
        NimBLERemoteService *pService = pClient->getService("fea6");
        if (pService != nullptr)
        {
            //GP-0077	Query Response	Notify
            NimBLERemoteCharacteristic *pChrNotify = pService->getCharacteristic(NimBLEUUID("b5f90077-aa8d-11e3-9046-0002a5d5c51b"));

            if (pChrNotify->canNotify())
            {
                Serial.println("enable query notify");

                auto cb = std::bind(&GoProBLE::notifyedQueryCB, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                if (!pChrNotify->subscribe(true, cb))
                {
                    /** Disconnect if subscribe failed */
                    pClient->disconnect();
                    return false;
                }
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

//QUERY
bool GoProBLE::checkQueryAsync(uint8_t *cmd, size_t len)
{
    if (!secureConnection)
    {
        Serial.println("Not secure connection");
        return false;
    }

    if (pClient->isConnected())
    {
        NimBLERemoteService *pService = pClient->getService("fea6");
        if (pService != nullptr)
        {
            //GP-0076	Query	Write
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
    return checkQueryAsync(cmdHot, 3);
}

bool GoProBLE::checkLowTempAlertAsync()
{
    //

    // uint8_t cmdCold[] = {0x02, 0x13, statusid};
    // statusid++;
    // Serial.println(statusid);
    // return checkQueryAsync(cmdCold, 3);
}

bool GoProBLE::checkBatteryPercentageAsync()
{
    uint8_t cmdCold[] = {0x02, 0x13, 0x46};
    return checkQueryAsync(cmdCold, 3);
}

bool GoProBLE::checkSystemBusyAsync()
{
    // Serial.println("check system busy");
    uint8_t cmdbusy[] = {0x02, 0x13, 0x08};
    return checkQueryAsync(cmdbusy, 3);
}

bool GoProBLE::checkDateTimeAsync()
{
    uint8_t cmdbusy[] = {0x01, 0x13};
    return checkQueryAsync(cmdbusy, 2);
}

void GoProBLE::notifyedQueryCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData,
                               size_t length, bool isNotify)
{
    Serial.print("query resoponse received: ");
    Serial.print(length);
    Serial.println(" byte");

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

    // for (int i = 0; i < length; i++)
    // {
    //     Serial.print(pData[i]);
    //     Serial.print(", ");
    // }
    // Serial.println();
}

bool GoProBLE::writeCommand(uint8_t *cmd, size_t len)
{

    if (pClient->isConnected())
    {
        NimBLERemoteService *pService = pClient->getService("fea6");
        if (pService != nullptr)
        {
            NimBLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic(NimBLEUUID("b5f90072-aa8d-11e3-9046-0002a5d5c51b"));

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
