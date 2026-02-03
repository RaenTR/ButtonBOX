# ETS2 Professional Button Box (PC & Android)

Bu proje, Euro Truck Simulator 2 (ETS2) için düşük gecikmeli, yüksek performanslı ve özelleştirilebilir bir fiziksel düğme kutusu (Button Box) deneyimini Android tabletinize getirir.

## 🚀 Özellikler

- **Native Performans:** Sunucu tarafı C++20 ve Win32 API ile, mobil tarafı ise Jetpack Compose ile native olarak geliştirilmiştir.
- **Düşük Gecikme:** TCP üzerinden optimize edilmiş veri iletimi ve DirectX Scan Code simülasyonu.
- **F3 GPS Entegrasyonu:** Oyun içi koordinatları anlık olarak takip eden radar özellikli harita.
- **Görsel Klavye Seçici:** Tuş kodlarını ezberlemenize gerek kalmadan görsel klavye üzerinden atama yapabilme.
- **Dinamik Düzenleme:** Butonların yerlerini sürükle-bırak ile uygulama üzerinden değiştirebilme.
- **Otomatik Keşif:** UDP Discovery ile sunucu IP'sini manuel girmeye gerek kalmadan otomatik bulma.

## 🛠️ Kurulum

### 1. Sunucu (PC)
- `server/bin/ETS2_ButtonBox_Server.exe` dosyasını yönetici olarak çalıştırın.
- Güvenlik duvarı izinleri otomatik olarak tanımlanacaktır.
- `buttons.json` dosyasını kendi tuş dizilimlerinize göre uygulama üzerinden veya manuel olarak düzenleyebilirsiniz.

### 2. Mobil Uygulama (Android)
- `mobile/app/build/outputs/apk/debug/app-debug.apk` dosyasını tabletinize yükleyin.
- Bilgisayarınızla aynı Wi-Fi ağına bağlı olduğunuzdan emin olun.
- "ARA" butonuna basarak sunucunuzu bulun ve "BAĞLAN" deyin.

## 💻 Geliştirme Notları

### Kullanılan Teknolojiler
- **Sunucu:** C++20, Win32 API, Winsock2, nlohmann/json.
- **Mobil:** Kotlin, Jetpack Compose, Coroutines/Flow, Material3.
- **İletişim:** TCP (Veri/Komut) & UDP (Discovery).

### Proje Yapısı
- `/server`: C++ Kaynak kodları ve Visual Studio projesi.
- `/mobile`: Android Studio (Kotlin/Compose) kaynak kodları.
- `/MemoryBank`: Proje dokümantasyonu ve gelişim süreci.

## 📄 Lisans
Bu proje kişisel kullanım için geliştirilmiştir.

---
*Geliştiren: [aliyahş]*
