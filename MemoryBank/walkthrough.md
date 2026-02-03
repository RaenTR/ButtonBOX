# ETS2 Button Box - Proje Walkthrough

Euro Truck Simulator 2 için geliştirilen profesyonel, native tabanlı kontrol paneli (Button Box) sistemi tamamlanmıştır. Bu proje, "altyapıyı sağlam tutma" prensibiyle native C++ ve Kotlin/Compose kullanılarak inşa edilmiştir.

## 🚀 Proje Bileşenleri

### 1. PC Server (C++)
- **Sağlam Altyapı:** Winsock2 tabanlı asenkron soket iletişimi, thread-safe StateManager ve profesyonel Logger modülü.
- **SCS Telemetry:** "SCS/ETS2" Shared Memory üzerinden oyun verilerini (hız, motor, ışıklar) anlık okur.
- **Input Control:** DirectX scan codes kullanarak oyun komutlarını simüle eder.

### 2. Android Uygulaması (Kotlin/Compose)
- **Modern UI:** Jetpack Compose ile tasarlanmış, tır kokpiti estetiği veren karanlık tema.
- **Tam Türkçe Dil Desteği:** Tüm arayüz, etiketler ve loglar %100 Türkçe.
- **Döner Anahtar:** Özel `RotarySwitch` bileşeni ile far ve silecek kontrolü.
- **Titreşim:** Her tuş vuruşunda profesyonel Haptic Feedback desteği.

## 🛠️ Kurulum ve Kullanım

### Sunucu Tarafı
1. `server/` klasöründeki projeyi Visual Studio 2022 ile açın veya CMake ile derleyin.
2. `bin/win_x64/plugins/` klasöründe SCS Telemetry plugininin kurulu olduğundan emin olun.
3. Server'ı çalıştırın (Varsayılan Port: 8888).

### Mobil Tarafı
1. Android projesini derleyip tabletinize yükleyin.
2. PC IP adresinizi girerek "Bağlan" butonuna basın.
3. ETS2 kokpitini tabletinizden yönetmeye başlayın!

## 📹 Özet ve Görünüm
- **Latency:** < 20ms (LAN)
- **Kontroller:** 
    - Döner Anahtarlar: Farlar, Silecekler.
    - Cam Kontrolleri: Sol ve Sağ cam indir/kaldır.
    - Dingil Yönetimi: Tır ve Dorse dingil indirme/kaldırma.
    - Diğer: Motor Başlatma, Park Freni, Dorse Bağlama, İş Kabul (Enter).

---
*Bu proje vudovn-AntiGravity-Kit referans alınarak, %100 native teknolojilerle geliştirilmiştir.*
