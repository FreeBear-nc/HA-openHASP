#include <Wire.h>
#include "Goodix.h"
#include "hexdump.h"

#define INT_PIN -1 // 23
#define RST_PIN -1 // -1 if pin is connected to VCC else set pin number

#ifndef NR_LEVEL
#define NR_LEVEL 5
#endif

/*
To compile and run, use the following one directory up:

    pio run -t upload && pio device monitor

Set the NR_LEVEL flag in platformio.ini to between 0 and 15
Default is 5

*/


Goodix touch = Goodix();

void touchStart() {
    if (touch.begin(INT_PIN, RST_PIN)!=true) {
        Serial.println("! Module reset failed");
    } else {
        Serial.println("Module reset OK");
    }

    Serial.print("Check ACK on address 0x");
    Serial.print(touch.i2cAddr, HEX);

    Wire.beginTransmission(touch.i2cAddr);
    int error = Wire.endTransmission();
    if (error == 0) {
        Serial.println(": SUCCESS");
    } else {
        Serial.print(": ERROR #");
        Serial.println(error);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nGoodix GT911x touch driver configuration");
/*
    -D TOUCH_SDA=19
    -D TOUCH_SCL=45
    -D TOUCH_RST=-1
    -D TOUCH_IRQ=-1
    -D I2C_TOUCH_FREQUENCY=400000
*/
    Wire.setClock(400000);
    Wire.begin(19, 45, 400000);
    delay(1000);

    touchStart();

    // This doesn't read all the registers in the config space
    touch.readConfig();

    uint8_t len = 0x8100 - GT_REG_CFG;
    uint8_t cfg[len];
    // This is the only way to read the entire config space
    touch.read(GT_REG_CFG, cfg, len);

//    cfg[len - 1] = touch.calcChecksum(cfg, len - 1);

    printf("########\n");

    printf("GT_REG_CFG len: %u\n", len);
    printf("sizeof GTConfig: %u\n", sizeof(GTConfig));

    printf("########\n\n");

    printf("configVersion: %x\n", touch.config.configVersion);
    printf("xResolution: %x\n", touch.config.xResolution);
    printf("yResolution: %x\n", touch.config.yResolution);

    printf("\n######\n");

    printf("noise reduction (GTConfig): %x\n", touch.config.noiseReduction);
    printf("noise reduction  (cfg[11]): %x\n", cfg[11]);
    printf("touch level: %u\n", touch.config.screenLevel.touch);
    printf("release level: %u\n", touch.config.screenLevel.leave);
    printf("checksum: %x\n", cfg[len - 1]);
    printf("\n########\n\n");

    hexdump(cfg, len, 1, nullptr, 1, 0x8047);

    uint8_t err = 0;

    if (NR_LEVEL < 0 || NR_LEVEL > 15) {
        printf("ERROR\n");
        printf("Noise reduction level must be between 0 and 15\n");
        printf("Currently set to: %i\n", NR_LEVEL);
        err = 1;
        goto end;
    } else {
        printf("Setting noise reduction level to: %i\n", NR_LEVEL);
    }

    cfg[11] = NR_LEVEL;
    cfg[len - 1] = touch.calcChecksum(cfg, len - 1);
    printf("checksum: %x\n", cfg[len - 1]);

    err = touch.write(GT_REG_CFG, cfg, len);
    if (err != 0) goto end;
    err = touch.write(0x8100, 1);
end:
    printf("GT911 update of registers ");
    printf(err ? "failed\n" : "success\n");
}


void loop() {
    touch.loop();
    delay(1000);
}

