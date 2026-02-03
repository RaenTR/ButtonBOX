# ETS2 Telemetry Plugin Kurulum Rehberi

Telemetri verilerinin çalışması için bu adımları takip edin:

1. **İndir**: [RenCloud/scs-sdk-plugin](https://github.com/RenCloud/scs-sdk-plugin/releases) sayfasından en güncel `scs-telemetry.dll` (veya zip içindeki dosyayı) indirin.
2. **Plugins Klasörü**:
   - Oyunun kurulu olduğu yere gidin (Genelde: `C:\Program Files (x86)\Steam\steamapps\common\Euro Truck Simulator 2\bin\win_x64`).
   - Burada `plugins` adında bir klasör yoksa **kendiniz oluşturun**.
3. **Kopyala**: İndirdiğiniz `scs-telemetry.dll` dosyasını bu `plugins` klasörünün içine yapıştırın.
4. **Oyunu Başlat**: Oyuna girdiğinizde "SDK Activated" uyarısı almalısınız. "Yes" diyerek onaylayın.

---
---
### ⚠️ "SDK Activated" Uyarısı Gelmezse Ne Yapmalı?

Eğer oyunu açtığınızda o kutucuk gelmiyorsa muhtemelen şunlardan biridir:

1. **Yanlış Klasör**: DLL'i `bin/win_x64/plugins` içine attığınızdan emin olun. `win_x86` değil, **win_x64** olmalı.
2. **Eksik Bileşen**: Plugin'in çalışması için bilgisayarınızda **Visual C++ Redistributable 2013** (vcredist_x64) yüklü olmalıdır. Genelde bu eksik olduğu için sessizce çöker.
3. **Ghost Plugin**: Eğer daha önce başka bir telemetri plugin'i kurduysanız çakışıyor olabilir. `plugins` klasöründe sadece bir tane `scs-telemetry.dll` bırakın.
4. **Oyun Sürümü**: Korsan veya çok eski bir sürüm kullanıyorsanız bazen pluginleri engeller. Orijinal Steam sürümü en sağlıklıdır.

**Kontrol**: `server.log` dosyasında hala "Shared Memory bulunamadı" yazıyorsa plugin kesinlikle aktif olmamıştır.
