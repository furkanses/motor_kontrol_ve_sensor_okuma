#define RS485_RX 18
#define RS485_TX 17
#define RS485_EN 8
#define BUZZER_PIN 21
#define RS485_BAUD 9600
#define igniter 1
#define valf 46

// Notalar
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784
#define NOTE_G5_ALTO 784
#define NOTE_C6 1047

u_int8_t arr[5];
u_int8_t flag_i = 0;
u_int8_t flag_v = 0;

unsigned long baslangicSuresi = 0;
unsigned long gecenSure = 0;
bool olcumBasladi = false;
bool buton1Kilit = true;
bool buton2Kilit = true;

u_int16_t ara_gecikme;

void sendToInterface(String message) {
  digitalWrite(RS485_EN, HIGH);
  delay(5);

  Serial1.println(message);
  Serial1.flush();
  delay(5);
  digitalWrite(RS485_EN, LOW);
}

void clearSerialBuffer() {
  while (Serial1.available() > 0) {
    Serial1.read();
  }
}

int countdown10Sec() {
  sendToInterface("10 saniye geri sayim basladi...");
  clearSerialBuffer();

  int totalSteps = 35;
  float delayTime = 800.0;

  for (int i = 0; i < totalSteps; i++) {
    if (Serial1.available()) {
      sendToInterface("Geri sayim kullanici tarafından iptal edildi!");
      return 0;
    }

    ledcWriteTone(BUZZER_PIN, 1000);
    delay(60);
    ledcWriteTone(BUZZER_PIN, 0);

    delay((int)delayTime);

    if (delayTime > 50) {
      delayTime *= 0.90;
    }
  }
  ledcWriteTone(BUZZER_PIN, 400);
  delay(1000);
  ledcWriteTone(BUZZER_PIN, 0);

  if (Serial1.available())
    return 0;
  return 1;
}

void playNote(int freq, int dur) {
  if (freq == 0) {
    ledcWriteTone(BUZZER_PIN, 0);
  } else {
    ledcWriteTone(BUZZER_PIN, freq);
  }
  delay(dur);
  ledcWriteTone(BUZZER_PIN, 0);
  delay(dur * 0.3);
}

void jingleBells() {
  int melody[] = {NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
                  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5, NOTE_E5};
  int durations[] = {200, 200, 400, 200, 200, 400, 200, 200, 200, 200, 800};

  for (int i = 0; i < 11; i++) {
    playNote(melody[i], durations[i]);
  }
}

int sozlesme(String eylem) {
  int yanit = -1;
  unsigned long baslangicZamani;
  byte gelenByte;

  clearSerialBuffer();
  sendToInterface(eylem + " mi istiyorsunuz?");
  sendToInterface("1");
  baslangicZamani = millis();

  while (millis() - baslangicZamani < 30000) {
    if (Serial1.available()) {
      gelenByte = Serial1.read();
      if (gelenByte == 0x0F) {
        Serial.println("1. Asama Onaylandi!");
        yanit = 1;
        break;
      } else if (gelenByte == 0x00) {
        Serial.println("1. Asama Reddedildi!");
        yanit = 0;
        break;
      } else {
        sendToInterface("Beklenmeyen eylem!");
        yanit = 0;
        break;
      }
    }
    delay(10);
  }

  if (yanit == -1) {
    return -2;
  }
  if (yanit == 0)
    return 0;

  clearSerialBuffer();
  sendToInterface("Emin misiniz?");
  sendToInterface("2");
  yanit = -1;
  baslangicZamani = millis();

  while (millis() - baslangicZamani < 30000) {
    if (Serial1.available()) {
      gelenByte = Serial1.read();
      if (gelenByte == 0x0F) {
        Serial.println("2. Asama Onaylandı!");
        yanit = 1;
        break;
      } else if (gelenByte == 0x00) {
        Serial.println("2. Asama Reddedildi!");
        yanit = 0;
        break;
      } else {
        sendToInterface("Beklenmeyen eylem!");
        yanit = 0;
        break;
      }
    }
    delay(10);
  }

  if (yanit == -1) {
    return -2;
  }
  if (yanit == 0)
    return 0;

  clearSerialBuffer();
  sendToInterface("Son karar?");
  sendToInterface("3");
  yanit = -1;
  baslangicZamani = millis();

  while (millis() - baslangicZamani < 30000) {
    if (Serial1.available()) {
      gelenByte = Serial1.read();
      if (gelenByte == 0x0F) {
        Serial.println("3. Asama Onaylandi!");
        yanit = 1;
        break;
      } else if (gelenByte == 0x00) {
        Serial.println("3. Asama Reddedildi!");
        yanit = 0;
        break;
      } else {
        sendToInterface("Beklenmeyen eylem!");
        yanit = 0;
        break;
      }
    }
    delay(10);
  }

  if (yanit == -1) {
    return -2;
  }

  return yanit;
}

int sozlesme_manuel() {
  unsigned long baslangicZamani;
  unsigned long i_mil;
  byte gelenByte;

  baslangicZamani = millis();
  sendToInterface("2");
  sendToInterface("Manuel atesleme mi yapmak istiyorsunuz?");
  while (millis() - baslangicZamani < 30000) {
    if (Serial1.available()) {
      gelenByte = Serial1.read();
      if (gelenByte == 0x0F) {
        Serial.println("2. Asama Onaylandi!");
        break;
      } else if (gelenByte == 0x00) {
        Serial.println("2. Asama Reddedildi!");
        return 0;
      } else {
        sendToInterface("Beklenmeyen eylem!");
        return 0;
      }
    }
    delay(10);
  }
  baslangicZamani = millis();
  sendToInterface("3");
  sendToInterface("Hazir!");
  while (millis() - baslangicZamani < 30000) {
    if (Serial1.available()) {
      gelenByte = Serial1.read();
      if (gelenByte == 0x0F && flag_i == 0) {
        digitalWrite(igniter, HIGH);
        flag_i = 1;
        i_mil = millis();
      } else if (gelenByte == 0x0F && flag_i) {
        digitalWrite(valf, HIGH);
        flag_v = 1;
        sendToInterface(String(millis() - i_mil));
        return 1;
      }
    }
    delay(5);
  }
  digitalWrite(igniter, LOW);
  digitalWrite(valf, LOW);
  flag_i = 0;
  flag_v = 0;
  sendToInterface("Zaman asimi!");
  return (0);
}

void komut_isle() {
  int s = 0;
  if (arr[1] == 0x00 && arr[2] == 0x00) {
    digitalWrite(igniter, LOW);
    digitalWrite(valf, LOW);
    flag_i = 0;
    flag_v = 0;
    sendToInterface("Sistem Kapatildi.");
  } else if (arr[1] == 0xAA && arr[2] == 0xAA && (flag_i + flag_v == 0)) {
    s = sozlesme("Igniter yakmak");
    if (s < 0)
      sendToInterface("Zaman asimi!");
    else if (s) {
      sendToInterface("Igniter yakma onayi alindi isleniyor.");
      digitalWrite(igniter, HIGH);
      flag_i = 1;
    } else
      sendToInterface("Iptal edildi yeni komut bekleniyor.");
  } else if (arr[1] == 0xAA && arr[2] == 0x00 && flag_i == 1) {
    sendToInterface("Igniter sonduruluyor.");
    digitalWrite(igniter, LOW);
    flag_i = 0;
  } else if (arr[1] == 0xBB && arr[2] == 0xBB && (flag_i + flag_v == 0)) {
    s = sozlesme("Valf acmak");
    if (s < 0)
      sendToInterface("Zaman asimi!");
    else if (s) {
      sendToInterface("Valf acma onayi alindi isleniyor.");
      digitalWrite(valf, HIGH);
      flag_v = 1;
    } else
      sendToInterface("Iptal edildi yeni komut bekleniyor.");
  } else if (arr[1] == 0xBB && arr[2] == 0x00 && flag_v == 1) {
    sendToInterface("Valf kapatiliyor.");
    digitalWrite(valf, LOW);
    flag_v = 0;
  } else if (arr[1] == 0xAA && arr[2] == 0xBB && (flag_i + flag_v == 0)) {
    sozlesme_manuel();
  } else if (arr[1] == 0xBB && arr[2] == 0x00 && flag_v == 1) {
    sendToInterface("Valf kapatiliyor.");
    digitalWrite(valf, LOW);
    flag_v = 0;
  } else if (arr[1] == 0xFF && arr[2] == 0xFF) {
    jingleBells();
  } else if (flag_v + flag_i == 0) {
    s = sozlesme("Motor ateslemek");
    if (s < 0)
      sendToInterface("Zaman asimi!");
    else if (s == 0)
      sendToInterface("Iptal edildi yeni komut bekleniyor.");
    else if (s) {
      sendToInterface("Motor atesleme onayi alindi kulaklara dikkat!");
      if (countdown10Sec() == 1) {
        ara_gecikme = ((uint16_t)arr[1] << 8) | arr[2];
        delay(3000);
        digitalWrite(igniter, HIGH);
        delay(ara_gecikme);
        digitalWrite(valf, HIGH);
        flag_i = 1;
        flag_v = 1;
      } else
        sendToInterface("Iptal edildi yeni komut bekleniyor.");
    }
  }
}

void paket_ayir() {
  arr[0] = 0xFD;

  unsigned long timeout = millis();
  int byteIndex = 1;

  while (byteIndex < 4 && (millis() - timeout < 100)) {
    if (Serial1.available()) {
      arr[byteIndex] = Serial1.read();
      byteIndex++;
    }
  }

  if (byteIndex == 4) {
    arr[4] = arr[0] + arr[1] + arr[2];
    if (arr[3] == arr[4]) {
      komut_isle();
    } else {
      Serial.println("Checksum Hatasi!");
    }
  }
  clearSerialBuffer();
  for (int i = 0; i < 5; i++)
    arr[i] = 0;
}

void setup() {
  for (int i = 0; i < 5; i++)
    arr[i] = 0;
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(igniter, OUTPUT);
  pinMode(valf, OUTPUT);
  digitalWrite(igniter, LOW);
  digitalWrite(valf, LOW);

  pinMode(RS485_EN, OUTPUT);
  digitalWrite(RS485_EN, LOW);

  Serial.begin(115200);
  ledcAttach(BUZZER_PIN, 2000, 8);

  Serial1.begin(RS485_BAUD, SERIAL_8N1, RS485_RX, RS485_TX);
  delay(100);

  sendToInterface("Sistem Hazir. Komut bekleniyor...");
  playNote(NOTE_C5, 100);
}

void loop() {
  if (Serial1.available() >= 4) {
    if (Serial1.read() == 0xFD) {
      paket_ayir();
    }
  }

  u_int16_t b1 = digitalRead(4);
  u_int16_t b2 =
      digitalRead(5); // digitalRead 0 veya 1 döner, otomatik < 5 olur.
  if (b1 < 1 && buton1Kilit == false) {

    baslangicSuresi = millis();
    buton1Kilit = true;
    buton2Kilit = true;
  }
  if (b2 < 1 && buton2Kilit == true && buton1Kilit == true) {
    gecenSure = millis() - baslangicSuresi;
    if (gecenSure != 0)
      sendToInterface("Fiziksel buton sayac: " + String(gecenSure));
    buton2Kilit = false;
    buton1Kilit = false;
  }
  delay(10);
}