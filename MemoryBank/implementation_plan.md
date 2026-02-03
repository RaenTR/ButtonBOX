# ETS2 Button Box Uygulama Planı

Bu plan, Euro Truck Simulator 2 için native C++ server ve Kotlin/Compose mobil uygulama mimarisini kapsar.

## 🏗️ Sistem Mimarisi

Sistem, AntiGravity Kit'in modüler state/input mantığını temel alır.

### 1. PC Server (C++)
- **SCS SDK:** Oyunun paylaşılan belleğinden telemetry verilerini (hız, vites, ışıklar vb.) saniyede 60 kez çeker.
- **TCP Server:** `Winsock2` kullanarak asenkron bir soket sunucusu çalıştırır. JSON protokolü ile mobil cihazla haberleşir.
- **Input Controller:** Windows `SendInput` API'sini kullanarak DirectX tarama kodları (scan codes) ile oyun komutlarını simüle eder.
- **State Manager:** Her kontrol elemanının (buton, rotary) güncel durumunu tutar ve telemetry ile doğrular.

### 2. Mobil Uygulama (Android/Kotlin)
- **Jetpack Compose:** Modern ve performanslı bir UI katmanı sağlar.
- **Native TCP Client:** Server'a doğrudan soket üzerinden bağlanır.
- **Rotary Logic:** Döner anahtar mantığı, her dokunuşta bir sonraki duruma geçer ve server'a hedef state'i bildirir.
- **Haptic Feedback:** Her etkileşimde fiziksel geri bildirim sağlar.

## 📡 Veri Protokolü

### Mobil -> Server (Input)
```json
{
  "type": "INPUT",
  "id": "light_switch",
  "value": 2
}
```

### Server -> Mobil (State Update)
```json
{
  "type": "STATE",
  "data": {
    "light_switch": 2,
    "speed": 85.5,
    "engine": true
  }
}
```

## 🛠️ Teknik Detaylar

- **Diller:** C++20 (Server), Kotlin 1.9 (Android)
- **Kütüphaneler:** 
  - Server: `nlohmann/json`, `spdlog`, `winsock2`
  - Android: `Jetpack Compose`, `Kotlinx Serialization`, `Socket API`
- **Gecikme Hedefi:** < 20ms (LAN üzerinde)

## ✅ Verifikasyon Planı

1. **Unit Tests:** State manager ve JSON parser testleri.
2. **Integration Tests:** Server ve mobil uygulama arasındaki paket alışverişi.
3. **E2E Tests:** ETS2 açıkken mobil üzerinden ışık ve motor kontrolü.
