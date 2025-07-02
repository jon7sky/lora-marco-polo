#ifndef RF_LORA_HPP
#define RF_LORA_HPP

// Set exactly ONE of these to 1 to define which configuration table to use.
#define TEST_SFx_BWx_CR5    1
#define TEST_SF12_BWx_CRx   0

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define KHZ(x) ((x) * 1000)
#define MHZ(x) ((x) * 1000 * 1000)

class RfLora {
  private:
    char txt[32];

  public:
    typedef struct {
      const char *name;               // A short name
      const char *shortDesc;
      const char *desc;               // A description of this config
      const int spreadFact;           // Spreading factor: Range is 6 to 12 (default is 7)
      const long bandwidth;           // Signal bandwidth: Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3 (default is 125E3)
      const int codingRateDenom;      // Coding Rate: Value is 4/X, range of X is 5 to 8 (default is 5)
      const int txPower;              // TX power: Range 2 to 20 (default is 17)
    } cfg_t;

    const long frequency = MHZ(915);  // 433MHz for Asia, 866MHz for Europe, 915MHz for North America
    const int txMsgLen = 10;          // Test messages start with "HelloThere", 10 characgters
    bool txMode = false;
    bool seqMode = false;
    int cfgIdx = 0;

    #if TEST_SFx_BWx_CR5
    const int cfgCnt = 10;
    const int testPeriodMs = 60 * 1000;
    const int testDurationMs = 5 * 1000;
    const int txCnt = 5;
    const int numColumns = 2;

    //  In the US902-928 and AU915-928 bands the data rates are as follows:
    //  (Link budgets are calculated using a transmit power of 20 dBm)
    //  
    //  Data  Spread  Band-   Max   Payload  Link
    //  Rate  Factor  width   BPS    Bytes  Budget
    //  ------------------------------------------
    //  DR0   SF10    125kHz  980     11    152 dB
    //  DR1   SF9     125kHz  1760    53    149 dB
    //  DR2   SF8     125kHz  3125    125   146 dB
    //  DR3   SF7     125kHz  5470    222   143 dB
    //  DR4   SF8     500kHz  12500   222   140 dB <-- same as DR12
    //  DR8   SF12    500kHz  980     53    151 dB
    //  DR9   SF11    500kHz  1760    129   148 dB
    //  DR10  SF10    500kHz  3900    222   146 dB
    //  DR11  SF9     500kHz  7000    222   143 dB
    //  DR12  SF8     500kHz  12500   222   140 dB
    //  DR13  SF7     500kHz  21900   222   137 dB

    #define DEFAULT_CODING_RATE_DENOM   5   // Coding Rate: Value is 4/X, range of X is 5 to 8 (default is 5)
    #define DEFAULT_TX_POWER            17   // TX power: Range 2 to 20 (default is 17)

    cfg_t cfg[10] = {
      { .name = "00", .shortDesc = "10/125", .desc = "SF10 125kHz", .spreadFact = 10, .bandwidth = KHZ(125), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER }, 
      { .name = "01", .shortDesc = "09/125", .desc = "SF9  125kHz", .spreadFact = 9,  .bandwidth = KHZ(125), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "02", .shortDesc = "08/125", .desc = "SF8  125kHz", .spreadFact = 8,  .bandwidth = KHZ(125), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "03", .shortDesc = "07/125", .desc = "SF7  125kHz", .spreadFact = 7,  .bandwidth = KHZ(125), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "08", .shortDesc = "12/500", .desc = "SF12 500kHz", .spreadFact = 12, .bandwidth = KHZ(500), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "09", .shortDesc = "11/500", .desc = "SF11 500kHz", .spreadFact = 11, .bandwidth = KHZ(500), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "10", .shortDesc = "10/500", .desc = "SF10 500kHz", .spreadFact = 10, .bandwidth = KHZ(500), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "11", .shortDesc = "09/500", .desc = "SF9  500kHz", .spreadFact = 9,  .bandwidth = KHZ(500), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "12", .shortDesc = "08/500", .desc = "SF8  500kHz", .spreadFact = 8,  .bandwidth = KHZ(500), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
      { .name = "13", .shortDesc = "07/500", .desc = "SF7  500kHz", .spreadFact = 7,  .bandwidth = KHZ(500), .codingRateDenom = DEFAULT_CODING_RATE_DENOM, .txPower = DEFAULT_TX_POWER },
    };
    #endif // TEST_SFx_BWx_CR5

    #if TEST_SF12_BWx_CRx
    const int cfgCnt = 5;
    const int testPeriodMs = 60 * 1000;
    const int testDurationMs = 10 * 1000;
    const int txCnt = 5;
    const int numColumns = 1;
    
    #define DEFAULT_TX_POWER            17  // TX power: Range 2 to 20 (default is 17)
    #define DEFAULT_SPREAD_FACTOR       12  // Spread factor 12 yields the longest range

    cfg_t cfg[5] = {
      { .name = "500kHz 4/5", .desc = "SF12 500kHz 4/5", .spreadFact = DEFAULT_SPREAD_FACTOR, .bandwidth = KHZ(500), .codingRateDenom = 5, .txPower = DEFAULT_TX_POWER }, 
      { .name = "250kHz 4/5", .desc = "SF12 250kHz 4/5", .spreadFact = DEFAULT_SPREAD_FACTOR, .bandwidth = KHZ(250), .codingRateDenom = 5, .txPower = DEFAULT_TX_POWER },
      { .name = "125kHz 4/5", .desc = "SF12 125kHz 4/5", .spreadFact = DEFAULT_SPREAD_FACTOR, .bandwidth = KHZ(125), .codingRateDenom = 5, .txPower = DEFAULT_TX_POWER },
      { .name = "500kHz 4/7", .desc = "SF12 500kHz 4/7", .spreadFact = DEFAULT_SPREAD_FACTOR, .bandwidth = KHZ(500), .codingRateDenom = 7, .txPower = DEFAULT_TX_POWER }, 
      { .name = "250kHz 4/7", .desc = "SF12 250kHz 4/7", .spreadFact = DEFAULT_SPREAD_FACTOR, .bandwidth = KHZ(250), .codingRateDenom = 7, .txPower = DEFAULT_TX_POWER },
    };
    #endif // TEST_SF12_BWx_CRx

    void setup(void) {
      //SPI LoRa pins
      SPI.begin(SCK, MISO, MOSI, SS);
      //setup LoRa transceiver module
      LoRa.setPins(SS, RST, DIO0);
      
      if (!LoRa.begin(frequency)) {
        Serial.println("Starting LoRa failed!");
        while (true);
      }
    }

    void setCfgIdx(int newCfgIdx) {
      cfgIdx = newCfgIdx;
      LoRa.setTxPower(cfg[cfgIdx].txPower);
      LoRa.setCodingRate4(cfg[cfgIdx].codingRateDenom);
      LoRa.setSpreadingFactor(cfg[cfgIdx].spreadFact);
      LoRa.setSignalBandwidth(cfg[cfgIdx].bandwidth);
    }

    int getCfgIdx(void) {
      return cfgIdx;
    }

    void tx(const char *msg) {
      LoRa.beginPacket();
      LoRa.print(msg);
      LoRa.endPacket();
    }

    bool rxMsgReady(void) {
      return LoRa.parsePacket() > 0 ? true : false;
    }

    String rx(void) {
      String data;
      while (LoRa.available()) {
        data = LoRa.readString();
      }
      return data;
    }

    int getRssi(void) {
      return LoRa.packetRssi();
    }

    int getSnr(void) {
      return (int) LoRa.packetSnr();
    }

    const char *desc(void) {
      sprintf(txt, "LORA %s %dMHZ", txMode ? "SENDER" : "RECEIVER", (int)(frequency / 1000000));
      return &txt[0];
    }

    void setTxMode(bool newTxMode) {
      txMode = newTxMode;
    }

    bool isTxMode(void) {
      return txMode;
    }

    bool isRxMode(void) {
      return !txMode;
    }

    void setSeqMode(bool newSeqMode) {
      seqMode = newSeqMode;
    }

    bool isSeqMode(void) {
      return seqMode;
    }
};

#endif // RF_LORA_HPP