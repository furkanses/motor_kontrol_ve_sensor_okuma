# Roket Motor Test Standı — Kontrol Yazılımı

## Genel Bakış

Bu proje, bir roket motorunun ateşleme testini gerçek zamanlı olarak izlemek ve kontrol etmek için geliştirilmiş bir Qt 6 masaüstü uygulamasıdır. Dört ayrı seri port üzerinden sensör verileri okunur, görselleştirilir ve CSV formatında kaydedilir.

---

## Gereksinimler

| Bileşen | Sürüm |
|---|---|
| Qt | 6.x (veya 5.15+) |
| Qt Modülleri | Widgets, SerialPort, Charts |
| Derleyici | MSVC 2019+, GCC 10+, Clang 12+ |
| İşletim Sistemi | Windows 10+, Linux, macOS |

### Qt Modülleri (.pro veya CMakeLists.txt)

```
# qmake (.pro)
QT += widgets serialport charts

# CMake
find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort Charts)
target_link_libraries(MyApp Qt6::Widgets Qt6::SerialPort Qt6::Charts)
```

---

## Proje Yapısı

```
project/
├── MainWindow.h        # Sınıf tanımları, üye değişkenler
├── MainWindow.cpp      # Tüm implementasyon
├── main.cpp            # Giriş noktası
├── dark_bg.png         # Karanlık tema arkaplan görseli
├── light_bg.png        # Aydınlık tema arkaplan görseli
├── christmas_bg.png    # Christmas tema arkaplan görseli
└── README.md
```

---

## Seri Port Yapılandırması

| Port | Rol | Varsayılan Baud | Yön |
|---|---|---|---|
| P1 (Modbus) | Basınç sensörleri (Modbus RTU) | 9600 | ReadWrite |
| P2 (Araba) | Araba yük hücresi | 115200 | ReadOnly |
| P3 (Tüp) | Tüp yük hücresi | 115200 | ReadOnly |
| P4 (Kontrol) | Arduino kontrol kartı | 115200 | ReadWrite |

---

## Protokoller

### P1 — Modbus RTU (Basınç Sensörleri)

**İstek paketi** (40ms periyotla, 25 Hz):
```
01 04 00 00 00 03 B0 0B
```

**Yanıt paketi** (11 byte):
```
01 04 06 [D1D2] [D3D4] [D5D6] [CRC_H] [CRC_L]
```
- `D1D2` → Ön Yanma Odası Basıncı (raw)
- `D3D4` → Arka Yanma Odası Basıncı (raw)
- `D5D6` → N₂O Tüp Basıncı (raw)

**Ham → Bar dönüşümü:**
```
bar = (raw - 4000) × 400 / 16000
Aralık: 4000 raw = 0 bar,  20000 raw = 400 bar
```

### P2 — Araba Yük Hücresi

Satır tabanlı ASCII, regex: `\x02\s*D([+-]?\d+)`  
Değer `/ 1000.0` ile kg'a çevrilir.

### P3 — Tüp Yük Hücresi

Satır tabanlı ASCII, regex: `=MG([+-]?\d+)g`  
Değer `/ 1000.0` ile kg'a çevrilir.

### P4 — Kontrol Kartı (Arduino)

**Gönderilen paketler** (4 byte):
```
[0xFD] [byte1] [byte2] [checksum]
checksum = 0xFD + byte1 + byte2  (mod 256)
```

| Komut | byte1 | byte2 | Açıklama |
|---|---|---|---|
| Ateşleyici AÇ | `0xAA` | `0xAA` | İgniter aktif |
| Ateşleyici KAPAT | `0xAA` | `0x00` | İgniter pasif |
| Valf AÇ | `0xBB` | `0xBB` | Ana valf açık |
| Valf KAPAT | `0xBB` | `0x00` | Ana valf kapalı |
| Acil Durdur | `0x00` | `0x00` | Her şeyi kapat |
| Otomatik Başlat | `MSB` | `LSB` | İgniter→Valf gecikmesi (ms) |
| Jingle Bells | `0xFF` | `0xFF` | Christmas modu |

**Tek byte komutlar (P4 yazma):**
```
0x00  → CANCEL
0x0F  → START / READY / FIRE
```

**Gelen mesajlar (P4'ten okunur):**
```
"1"  → START butonu aktif edilir
"2"  → READY butonu aktif edilir
"3"  → FIRE butonu aktif edilir
```

---

## Çalışma Modları

### Test Modu
- Ateşleyici ve Valf butonları manuel olarak kullanılabilir.
- Otomatik sekans devre dışı kalır.
- Manuel Mod ile aynı anda seçilemez.

### Manuel Mod
- START, CANCEL, READY, FIRE butonları aktif olur.
- Kontrol kartından gelen mesajlarla buton durumları güncellenir.
- FIRE'a 2 kez basıldıktan sonra FIRE devre dışı kalır, START tekrar aktif olur.

### Otomatik Mod (varsayılan)
- "OTOMATİK BAŞLAT" butonu ile sekans başlar.
- İgniter→Valf gecikmesi `m_spinRoleTime` (ms) spinbox ile ayarlanır ve Arduino'ya gönderilir.

---

## Kayıt Sistemi

"Kaydı Başlat" butonuna basıldığında `~/Desktop/loglar/` altında üç ayrı CSV dosyası oluşturulur:

| Dosya | İçerik |
|---|---|
| `modbus_YYYYMMDD_HHmmss.csv` | `Zaman(ms), OnYanma(bar), ArkaYanma(bar), N2O(bar)` |
| `araba_ldc_YYYYMMDD_HHmmss.csv` | `Zaman(ms), ArabaLoadcell` |
| `tup_ldc_YYYYMMDD_HHmmss.csv` | `Zaman(ms), TupLoadcell` |

Ek olarak her event (buton basışı, gelen mesaj vb.) `sabit_log.csv` dosyasına her zaman yazılır:
```
AbsoluteTimestamp(ms), EventName
```

Event'ler kayıt sırasında tüm aktif CSV dosyalarına da `EVENT:` prefiksiyle eklenir.

---

## UI Bileşenleri

### Sol Panel
- Port bağlantı satırları (4 adet)
- Kontrol butonları (Ateşleyici, Valf, Acil Durdur, Otomatik Başlat)
- Sensör verileri (anlık değerler)
- Kayıt butonu

### Orta Panel
- `QPlainTextEdit` — P4'ten gelen mesajlar (son 50 satır tutulur)
- **ATEŞLEME LOOPU** grup kutusu: CANCEL / START / READY / FIRE
- `MotorWidget` — Pixel art motor ve alev animasyonu

### Sağ Panel
- **Pressure Chart**: Ön Yanma, Arka Yanma, N₂O — son 10 saniye
- **Loadcell Chart**: Tüp ve Araba yük hücreleri — son 10 saniye

---

## MotorWidget

`QWidget` türevli özel çizim bileşeni. `paintEvent` içinde:

- Piksel sanat stili roket gövdesi (perçinler dahil)
- Nozzle (trapez poligon)
- Dinamik alev: `(onYanma + arkaYanma)` toplamına göre ölçeklenir
  - Sarı çekirdek → Turuncu orta → Kırmızı uç
  - > 0 bar'da beyaz kıvılcım pikselleri

---

## Zamanlayıcılar

| Zamanlayıcı | Periyot | Amaç |
|---|---|---|
| `m_modbusTimer` | 40 ms (25 Hz) | Modbus poll isteği gönder |
| `m_uiUpdateTimer` | 50 ms (20 Hz) | Etiketler, grafik, motor widget güncelle |
| `m_recordTimer` | 50 ms | Kayıt süresi etiketini güncelle |

---

## Temalar

`m_cmbThemeSelect` ile 3 tema seçilebilir:

| İndeks | Tema | Arkaplan |
|---|---|---|
| 0 | Karanlık | `dark_bg.png` |
| 1 | Aydınlık | `light_bg.png` |
| 2 | Christmas | `christmas_bg.png` (+ Jingle Bells butonu) |

---

## Derleme

```bash
# qmake
qmake MyApp.pro
make -j4

# CMake
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt6
cmake --build build -j4
```

---

## Bilinen Kısıtlamalar

- Modbus CRC doğrulaması yapılmamaktadır; sadece `01 04 06` başlığı aranır.
- Grafik serileri bellek içinde tutulur; çok uzun testlerde RAM kullanımı artabilir (10 saniyelik pencere ile sınırlandırılmıştır).
- `updateSequence()` fonksiyonu artık kullanılmamaktadır; sekans Arduino tarafından yönetilmektedir.
