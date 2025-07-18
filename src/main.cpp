#include <Arduino.h>
#include <EEPROM.h>

// Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

// Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "rf_lora.hpp"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define LINE(x) ((x) * 8)

// Hack for TTGO LORA32 V2 -- Although you can build for that board, the pins_arduino.h file is missing.
// So the hack is to configure the build to use the V1 board and override the OLED pins here.
#ifdef OLED_SDA_OVERRIDE
#undef OLED_SDA
#define OLED_SDA OLED_SDA_OVERRIDE
#undef OLED_SCL
#define OLED_SCL OLED_SCL_OVERRIDE
#endif

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
RfLora rf;
unsigned char txMode;
constexpr int runPeriodMs = 3000;
constexpr int syncPeriodMs = (runPeriodMs * 16);
const char *blankLine = "                    ";

void refresh_display()
{
    display.clearDisplay();
    display.setCursor(0, LINE(0));
    display.print(txMode ? "Marco" : "Polo");
    for (int i = 0; i < rf.cfgCnt; i++)
    {
        display.setCursor((i % rf.numColumns) * (128 / rf.numColumns), LINE(3 + (i / rf.numColumns)));
        display.print(rf.cfg[i].shortDesc);
    }
    display.display();
}

void setup()
{
    char txt[32];

#ifdef MODE_CHANGE_BUTTON_PIN
    pinMode(MODE_CHANGE_BUTTON_PIN, INPUT);
#endif

    EEPROM.begin(4);

    // reset OLED display via software
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    // initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
    { // Address 0x3C for 128x32
        Serial.println("SSD1306 allocation failed");
        while (true);
    }

    // initialize Serial Monitor
    Serial.begin(115200);
    Serial.println(txt);

    rf.setup();

    while (true)
    {
        txMode = EEPROM.read(0) == 't' ? true : false;

        rf.setTxMode(txMode);
        rf.setSeqMode(true);

        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0, LINE(0));
        display.print(txMode ? "Marco" : "Polo");
        display.setCursor(0, 10);

        display.setCursor(0, LINE(3));
#ifdef MODE_CHANGE_BUTTON_PIN
        display.println("Press PGM button");
#else
        display.println("Press RST button");
#endif
        display.setCursor(0, LINE(4));
        display.println("to change TX/RX mode");
        display.display();

#ifndef MODE_CHANGE_BUTTON_PIN
        int origCfgIdx = cfgIdx;
        int origTxMode = txMode;
        // Change to the next mode
        if (++cfgIdx > rf.cfgCnt)
        {
            txMode = !txMode;
            cfgIdx = 0;
        }
        EEPROM.write(0, txMode ? 't' : 'r');
        EEPROM.write(1, cfgIdx);
        EEPROM.commit();
        delay(2000);
        EEPROM.write(0, origTxMode ? 't' : 'r');
        EEPROM.write(1, origCfgIdx);
        EEPROM.commit();
        break;
#else  // MODE_CHANGE_BUTTON_PIN
        {
            int cnt;

            for (cnt = 200; cnt > 0; cnt--)
            {
                if (digitalRead(MODE_CHANGE_BUTTON_PIN) == 0)
                {
                    // Toggle TX/RX mode
                    txMode = !txMode;

                    EEPROM.write(0, txMode ? 't' : 'r');
                    EEPROM.commit();
                    // Wait for button to be released.
                    do
                    {
                        delay(10);
                    } while (digitalRead(MODE_CHANGE_BUTTON_PIN) == 0);
                    break;
                }
                delay(10);
            }

            if (cnt == 0)
            {
                break;
            }
        }
#endif // MODE_CHANGE_BUTTON_PIN
    }

    refresh_display();
}

void loop(void)
{
    static int lastCfgIdx = -1;
    int cfgIdx;
    static unsigned long baseMillis = -1;
    static String msg = "";
    static bool syncing = true;
    static int cntSinceMsg = 0;
    unsigned long now;
    int period;
    static int numRounds = 1;
    static int goodRxCnt[32] = {0, };
    static int lastButton = 0;
    int button;

    now = millis();
    if (baseMillis == -1)
    {
        baseMillis = now;
    }

    button = digitalRead(MODE_CHANGE_BUTTON_PIN);
    if (button == 0 && lastButton == 1)
    {
        numRounds = 0;
        memset(&goodRxCnt,  0, sizeof(goodRxCnt));
        refresh_display();
    }
    lastButton = button;

    //period = (txMode ? runPeriodMs : (syncing ? syncPeriodMs : runPeriodMs));
    //cfgIdx = ((now - baseMillis) / period) % rf.cfgCnt;
    period = runPeriodMs;
    cfgIdx = ((now - baseMillis) / period) % rf.cfgCnt;
    if (!txMode && syncing)
    {
        cfgIdx = 0;
    }

    if (cfgIdx != lastCfgIdx)
    {
        if (!syncing && cfgIdx == 0)
        {
            numRounds++;
        }
        if (lastCfgIdx >= 0)
        {
            display.setCursor((lastCfgIdx % rf.numColumns) * (128 / rf.numColumns), LINE(3 + (lastCfgIdx / rf.numColumns)));
            // display.print(rf.cfg[lastCfgIdx].name);
            display.print(rf.cfg[lastCfgIdx].shortDesc);
            display.setCursor(0, LINE(1));
            display.print(blankLine);
            display.display();
        }

        msg = "     ";
        rf.setCfgIdx(cfgIdx);
        display.setCursor((7 * 8), LINE(0));
        display.print(rf.cfg[cfgIdx].desc);
        display.setTextColor(BLACK, WHITE);
        display.setCursor((cfgIdx % rf.numColumns) * (128 / rf.numColumns), LINE(3 + (cfgIdx / rf.numColumns)));
        // display.print(rf.cfg[cfgIdx].name);
        display.print(rf.cfg[cfgIdx].shortDesc);
        display.setTextColor(WHITE, BLACK);
        // display.print("       ");
        display.display();
        lastCfgIdx = cfgIdx;

        if (txMode)
        {
            display.setCursor(0, LINE(2));
            display.print("Round: ");
            display.print(numRounds);
            display.setCursor(0, LINE(1));
            display.print("TX:Marco");
            display.display();
            rf.tx("Marco");
        }
        else
        {
            if (++cntSinceMsg > (rf.cfgCnt * 3))
            {
                syncing = true;
                baseMillis = now;
            }
        }
    }

    if (rf.rxMsgReady())
    {
        msg = rf.rx();
        msg = msg.substring(0, 5);
        bool gotGoodMsg = false;
        if (txMode)
        {
            display.setCursor((8 * 9), LINE(1));
            display.print("RX:");
            display.print(msg);
            display.display();
            if (msg == "Polo")
            {
                gotGoodMsg = true;
                if (numRounds > 0) {
                    goodRxCnt[cfgIdx]++;
                }
            }
        }
        else
        {
            display.setCursor(0, LINE(1));
            display.print("RX:");
            display.print(msg);
            if (msg == "Marco")
            {
                rf.tx("Polo");
                display.setCursor((8 * 9), LINE(1));
                display.print("TX:Polo");
                gotGoodMsg = true;
                goodRxCnt[cfgIdx]++;
            }
            display.display();
            baseMillis = now - (runPeriodMs * cfgIdx) - 1000;
        }

        if (gotGoodMsg)
        {
#if 0
            int rssi = rf.getRssi();
            // int snr = rf.getSnr();
            char txt[20];
            sprintf(txt, "%4ddB", rssi);
            display.setCursor((cfgIdx % rf.numColumns) * (128 / rf.numColumns) + 16, LINE(3 + (cfgIdx / rf.numColumns)));
            display.print(txt);
#endif
            if (numRounds)
            {
                display.setCursor((cfgIdx % rf.numColumns) * (128 / rf.numColumns) + 42, LINE(3 + (cfgIdx / rf.numColumns)));
                display.print(goodRxCnt[cfgIdx]);
                display.display();
            }
            syncing = false;
            cntSinceMsg = 0;
        }
    }
}
