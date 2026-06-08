# Roket Motoru Test Yer İstasyonu (Ground Control Station)

Bu proje, roket motoru statik ateşleme (static fire) testleri için geliştirilmiş, C++ ve Qt framework tabanlı bir Yer Kontrol İstasyonu (GCS) arayüzüdür. Sistem; yanma odası basınçları, N2O basıncı ve itki (loadcell) verilerini gerçek zamanlı olarak izlemenizi, milisaniye hassasiyetinde veri kaydı (logging) almanızı ve motor ateşleme sekansını güvenli bir şekilde yönetmenizi sağlar.

## 🚀 Özellikler

* **Çoklu Seri Haberleşme (4 Port):** Sensörler ve kontrolcüler ile eşzamanlı haberleşme altyapısı.
* **Gerçek Zamanlı Telemetri İzleme:** Modbus RTU protokolü üzerinden ön/arka yanma odası basıncı ve N2O basıncı okuma. Loadcell verilerini (Tüp ve Araba) ayrıştırarak anlık itki takibi.
* **Canlı Grafikleme (QtCharts):** Basınç ve loadcell verilerinin 20 Hz yenileme hızıyla dinamik ve kayan grafikler üzerinde görselleştirilmesi.
* **Ateşleme ve Sekans Kontrolü:** * Ateşleyici (Igniter) ve Ana Valf kontrolü.
  * Manuel Mod, Test Modu ve zaman ayarlı (ms) Otomatik Ateşleme Sekansı.
  * Donanımsal acil durum kesme (Emergency Abort) protokolü.
* **Dinamik Görsel Geri Bildirim:** Gelen basınç değerlerine (bar) göre alev boyu dinamik olarak değişen, özel tasarlanmış piksel-art motor simülasyonu (`MotorWidget`).
* **Kapsamlı Veri Kaydı (Data Logging):** Tüm sensör verilerini ve kullanıcı komutlarını (olay günlüğü) senkronize bir şekilde milisaniye zaman damgasıyla `.csv` formatında kaydetme.
* **Dinamik Tema Desteği:** Aydınlık, Karanlık ve "Christmas" temaları arasında anında geçiş.

## 💻 Kullanılan Teknolojiler

* **Dil:** C++
* **Arayüz Geliştirme:** Qt Framework (Qt Widgets, Qt Core, Qt Gui)
* **Modüller:** * `QtSerialPort` (RS485 / UART Haberleşmesi)
  * `QtCharts` (Veri Görselleştirme)
* **Protokoller:** Modbus RTU, Custom Serial Parsing (Regex)

## 🔌 Donanım ve Port Konfigürasyonu

Sistem 4 farklı seri port üzerinden asenkron olarak çalışır:

| Port | Bağlantı Cihazı | Görev | Baud Rate (Varsayılan) |
| :--- | :--- | :--- | :--- |
| **P1** | Basınç Sensörleri (Modbus) | Ön Yanma, Arka Yanma ve N2O basınç verilerini okur. | 9600 |
| **P2** | Araba Loadcell | İtki kuvvetini (D formatında) okur ve ayrıştırır. | 115200 |
| **P3** | Tüp Loadcell | Tüp ağırlığını (=MG formatında) okur ve ayrıştırır. | 115200 |
| **P4** | Ana Kontrolcü (Arduino/MCU) | Ateşleme sekansı komutlarını gönderir (Checksum ile). | 115200 |

## 🛠️ Kurulum ve Derleme

Projeyi derlemek için sisteminizde **Qt 5** veya **Qt 6** kurulu olmalıdır.

1. Depoyu bilgisayarınıza klonlayın:
   ```bash
   git clone [https://github.com/KULLANICI_ADINIZ/proje-adi.git](https://github.com/KULLANICI_ADINIZ/proje-adi.git)
