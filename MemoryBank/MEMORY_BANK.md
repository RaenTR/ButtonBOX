# 🧠 ETS2 Button Box - Proje Teknik Hafızası (Memory Bank)

Bu dosya, projenin mevcut durumu, kullanılan teknolojiler ve gelecek hedefleri hakkında kapsamlı bir rehberdir. Projenin "beyni" olarak tasarlanmıştır.

## 🏗️ Genel Mimari
Sistem, web/tarayıcı teknolojilerinden kaçınarak tamamen **Native** (yerel) teknolojiler üzerine kurulmuştur. Bu sayede en düşük gecikme (latency) ve en yüksek stabilite hedeflenmiştir.

- **Kritik Prensip:** "Altyapıyı Sağlam Tut" - her modül thread-safe ve genişletilebilir yapıda tasarlanmıştır.
- **İletişim:** TCP Sockets (Winsock2 & Kotlin Sockets) - **Dashboard:** Modern Winsock tabanlı ve Unicode destekli GUI.
- **Canlı İzleme:** Oyun durumu, istemci bağlantısı ve anlık gecikme (ms) takibi.
- **Ağ Optimizasyonu:** Nagle algoritması devre dışı (TCP_NODELAY) ve telemetry kısıtlama (20Hz).

---

## 💻 Sunucu Tarafı (PC Server)
**Teknoloji:** Modern C++ (C++20), Win32 API, Winsock2.

### 🛠️ Modüller:
1. **TCPServer:** Asenkron, tek istemci odaklı, Winsock2 tabanlı TCP sunucusu.
2. **TelemetryListener:** SCS Telemetry SDK üzerinden Shared Memory ("SCS/ETS2") alanını okuyan dinleyici.
3. **InputController:** Windows `SendInput` API'sini kullanan ve DirectX Scan Codes (DirectInput) ile oyunla konuşan simülatör.
4. **StateManager:** Singleton yapısında, tüm oyun ve kontrol durumlarını (hız, ışıklar, silecekler) tutan merkezi hafıza.
5. **ConfigManager:** Buton eşleşmelerini ve kontrol tiplerini yöneten dinamik yapı.

**Neden C++?**
- DirectX Scan kodlarını doğrudan simüle edebilmek.
- SCS Telemetry SDK'ya en hızlı erişimi sağlamak.
- Sıfıra yakın işlem yükü (sıfır overhead).

---

## 📱 Mobil Tarafı (Android)
**Teknoloji:** Kotlin, Jetpack Compose, Coroutines, Flow.
**Hedef Cihaz:** Hometech Alfa 8SL (1280x800 - HD Çözünürlük).

### 🎨 Görsel ve Teknik Yapı:
- **Premium Dashboard:** Glassmorphism, neon neon ışık efektleri ve `animateFloatAsState` ile akıcı animasyonlar.
- **Döner Anahtar:** Animasyonlu, 0-270 derece dönebilen özel kadran bileşeni.
- **Canlı Radar:** `MapView` ile anlık koordinat ve yön takibi.
- **Haptic Feedback:** Her etkileşimde (basma/çevirme) cihazın titreşim motorunu kullanan dokunsal geri bildirim.
- **Network:** Coroutines Dispatchers.IO üzerinde koşan stabil TCP Client.

---

## 🛣️ Gelecek Yol Haritası (Roadmap)

### 🔹 Kısa Vade (Cila)
- [x] **JSON Config:** Butonların koddan değil, `buttons.json` dosyasından okunması.
- [x] **Otomatik IP:** Mobil uygulamanın bilgisayarı ağda otomatik bulması (UDP Discovery).
- [x] **Aksiyon Seçici (Action Picker):** ETS2 için önceden tanımlı aksiyon (kontak, ışıklar, korna vb.) seçebilme özelliği.
- [x] **Sesli Uyarı (TTS):** Navigasyon ve oyun durumu değişikliklerinde sesli geri bildirim (Kısa vadeden tamamlandı).

### 🔹 Orta Vade (Gelişmiş Özellikler)
- [x] **Harita/Rota Desteği:** F3 GPS navigasyon ekranı ve rota bilgisi (Şehirler arası takip).
- [ ] **Sürükle-Bırak Düzenleme:** Tablet üzerinden butonların sırasını değiştirebilme (Şu an sadece düzenleme modu var).
- [ ] **Log Verileri:** Yakıt tüketimi, kalan yol süresi gibi detaylı göstergelerin eklenmesi.

### 🔹 Uzun Vade (Ekosistem)
- [ ] **Profesör Seviyesi Telemetry:** Lastik sıcaklığı, yağ basıncı, turbo basıncı gibi derin veriler.
- [ ] **Hız Sınırı:** Navigasyonda hız sınırı tabelası gösterimi.
- [ ] **Analog Göstergeler:** İbreli hız ve RPM kadranları.

---

## ⚠️ Kritik Bilgiler (Memory Bits)
- **Terimler:** `Client` -> `İstemci`, `Server` -> `Sunucu`.
- **Telemetry Plugin:** Mutlu son için `scs-telemetry.dll` kullanılmalıdır.
- **Harita Desteği:** F3 GPS tarzı dinamik rota gösterimi.
- **Otomatik IP:** UDP Discovery desteği (8889 portu) ile otomatik eşleşme.
- **Tablet Optimizasyonu:** Hometech Alfa 8SL (1280x800) tam uyumlu premium layout.
- **Action Picker:** +100 hazır ETS2 komutu kategorize edilmiş halde uygulamaya eklendi.
- **Dil:** %100 Türkçe.
- **Referans:** Proje vudovn-AntiGravity-Kit standartlarına göre yazılmıştır.
