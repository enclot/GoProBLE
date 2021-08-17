// M5StickC + GoPro6Black

#include <M5StickC.h>
#include <GoProBLE.h>

GoProBLE goproBle;

uint32_t ticked;

void setup()
{
  Serial.begin(115200);
  M5.begin();

  goproBle.scanAsync(5);
  delay(1000);

  goproBle.connect();
  goproBle.enableStatusResponse();

  ticked = millis();
}


void loop()
{
  M5.update();
  if (M5.BtnB.wasPressed()) {
    Serial.println("reset button pressed");
    esp_restart();
  }
  if (M5.BtnA.wasPressed()) {
    Serial.println("M5Button pressed");

    if (goproBle.isConnected()) {

      if (!goproBle.isSystemBusy()) {
        goproBle.shutterOn();
      } else {
        goproBle.shutterOff();
      }

      Serial.println(goproBle.getBatteryPercentage() );
    }

  }

  if (millis() - ticked > 3000) {
    goproBle.checkSystemBusyAsync();
    goproBle.checkSystemHotAsync();
    goproBle.checkBatteryPercentageAsync();

    ticked = millis();
  }

}
