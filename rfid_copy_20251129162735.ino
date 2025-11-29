#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN    D8      // RC522 SDA / SS
#define RST_PIN   D3      // RC522 RST

#define GREEN_LED D1
#define RED_LED   D2

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// -------- PATIENT DATABASE (2 CARDS) --------
struct Patient {
  byte uid[4];           // First 4 bytes of tag UID
  const char* name;
  int age;
  const char* note;      // diagnosis / info
};

// TODO: CHANGE UID VALUES TO MATCH YOUR CARDS
Patient patients[] = {
  { {0x13, 0x24, 0x3F, 0xE4}, "Patient One",   25, "Diabetes Type 2" },
  { {0x62, 0x40, 0x47, 0x5C}, "Patient Two",   40, "Hypertension"    }
};

const int PATIENT_COUNT = sizeof(patients) / sizeof(patients[0]);

// --------- HELPER FUNCTIONS ---------
bool compareUid(byte *uid1, byte *uid2, byte size) {
  for (byte i = 0; i < size; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

Patient* findPatient(byte *uid, byte uidSize) {
  // We compare only the first 4 bytes
  for (int i = 0; i < PATIENT_COUNT; i++) {
    if (compareUid(uid, patients[i].uid, 4)) {
      return &patients[i];
    }
  }
  return NULL;
}

// ------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  Serial.println("Present an RFID tag...");
}

// ------------- MAIN LOOP --------------
void loop() {
  // Wait for new card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("Tag UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  Patient* p = findPatient(mfrc522.uid.uidByte, mfrc522.uid.size);

  if (p != NULL) {
    // Known card
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    Serial.println("Authorized card!");
    Serial.println("===== PATIENT DETAILS =====");
    Serial.print("Name : "); Serial.println(p->name);
    Serial.print("Age  : "); Serial.println(p->age);
    Serial.print("Note : "); Serial.println(p->note);
    Serial.println("===========================");
  } else {
    // Unknown card
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    Serial.println("Unknown card!");
  }

  delay(1500);                  // keep LED on for 1.5s
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
