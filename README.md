# ETS2 Professional Button Box (PC & Android)

Bu proje, Euro Truck Simulator 2 (ETS2) için düşük gecikmeli, yüksek performanslı ve özelleştirilebilir bir fiziksel düğme kutusu (Button Box) deneyimini Android tabletinize getirir.

## 🚀 Özellikler

- **Native Performans:** Sunucu tarafı C++20 ve Win32 API ile, mobil tarafı ise Jetpack Compose ile native olarak geliştirilmiştir.
- **Düşük Gecikme:** TCP üzerinden optimize edilmiş veri iletimi ve DirectX Scan Code simülasyonu.
- **F3 GPS Entegrasyonu:** Oyun içi koordinatları anlık olarak takip eden radar özellikli harita.
- **Görsel Klavye Seçici:** Tuş kodlarını ezberlemenize gerek kalmadan görsel klavye üzerinden atama yapabilme.
- **Aksiyon Seçici (Action Picker):** Kontak, motor, silecekler gibi +100 oyuniçi komutu hazır listeden tek tıkla butona atama.
- **Sesli Uyarıcı (TTS):** "Benzin azaldı", "Şehre giriş yaptınız" gibi önemli bilgilerin sesli olarak bildirilmesi.
- **Dinamik Düzenleme:** Düzenleme modu ile buton isimlerini, ikonlarını ve renklerini anlık olarak tablet üzerinden değiştirebilme.
- **Otomatik Keşif:** UDP Discovery ile IP girmeye gerek kalmadan saniyeler içinde bağlanma.

## 🛠️ Kurulum

### 1. Telemetry Plugin (Kritik)
- `scs-telemetry.dll` dosyasını ETS2'nin kurulu olduğu dizine (`bin/win_x64/plugins`) kopyalayın. 
- Detaylı adımlar için [TELEMETRİ KURULUM REHBERİ] dosyasına göz atın.

### 2. Sunucu (PC)
- `server/bin/ETS2_ButtonBox_Server.exe` dosyasını yönetici olarak çalıştırın.
- Sunucu açıldığında "ETS2 Telemetry bağlantısı başarılı" mesajını görmelisiniz.

### 3. Mobil Uygulama (Android)
- `mobile/app/build/outputs/apk/debug/app-debug.apk` dosyasını tabletinize yükleyin.
- 

## 💻 Geliştirme Notları

### Kullanılan Teknolojiler
- **Sunucu:** C++20, Win32 API, Winsock2, nlohmann/json.
- **Mobil:** Kotlin, Jetpack Compose, Coroutines/Flow, Material3, Text-to-Speech API.
- **İletişim:** TCP (Veri/Komut) & UDP (Discovery).

### Proje Yapısı
- `/server`: C++ Kaynak kodları ve Visual Studio / CMake projesi.
- `/mobile`: Android Studio (Kotlin/Compose) kaynak kodları.
- `/MemoryBank`: Projenin teknik hafızası ve gelişim dökümanları.

## 📄 Lisans
Bu proje kişisel kullanım için geliştirilmiştir.

---
*Geliştiren: [aliyahş]*
