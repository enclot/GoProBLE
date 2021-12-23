// M5StickC + GoPro6Black

#include <M5StickC.h>
#include <GoProBLE.h>

GoProBLE gp;

uint32_t ticked;

void setup()
{
  Serial.begin(115200);
  M5.begin();
  gp.scanAsync(5); //5 second
  //gp.scanAsync("GoPro 7715", 5);
  delay(5000);

  if (gp.isFound()) {
    gp.connect();
  }
  delay(1000);
  gp.enableQueryResponse();


  ticked = millis();
}


void loop()
{
  M5.update();

  if (M5.BtnA.wasPressed()) {
    Serial.println("M5Button pressed");
    if (gp.isConnected()) {
      if (!gp.isSystemBusy()) {
        gp.shutterOn();
      } else {
        gp.shutterOff();
      }
    }
  }
  if (M5.BtnB.wasPressed()) {
    esp_restart();
  }

  if (millis() - ticked > 5000) {
    if (gp.isConnected() ) {
      gp.checkSystemBusyAsync();
      gp.checkBatteryPercentageAsync();

      Serial.println(gp.getBatteryPercentage() );
    }

    ticked = millis();
  }

}
