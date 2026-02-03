import socket
import json
import time
import threading

SERVER_IP = "127.0.0.1"
TCP_PORT = 8888
UDP_PORT = 8889

def test_udp_discovery():
    print("\n--- [TEST] UDP Discovery ---")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(3)
    
    discovery_msg = "ETS2_BUTTONBOX_DISCOVERY"
    sock.sendto(discovery_msg.encode(), ('<broadcast>', UDP_PORT))
    
    try:
        data, addr = sock.recvfrom(1024)
        print(f"[OK] Yanıt alındı: {data.decode()} (Adres: {addr})")
        return True
    except socket.timeout:
        print("[FAIL] UDP Discovery yanıt vermedi.")
        return False

def test_tcp_connection():
    print("\n--- [TEST] TCP Connection & Protocol ---")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)
    
    try:
        sock.connect((SERVER_IP, TCP_PORT))
        print("[OK] Sunucuya bağlanıldı.")
        
        # 1. Heartbeat Bekle
        print("[WAIT] Heartbeat bekleniyor...")
        start_time = time.time()
        heartbeat_received = False
        while time.time() - start_time < 10:
            data = sock.recv(4096).decode()
            if not data: break
            
            # Birden fazla JSON paketi birleşmiş olabilir
            packets = data.strip().split('}')
            for p in packets:
                if not p: continue
                p += '}'
                try:
                    js = json.loads(p)
                    if js.get("type") == "HEARTBEAT":
                        print("[OK] Heartbeat sinyali alındı.")
                        heartbeat_received = True
                        break
                    if js.get("type") == "STATE":
                        print(f"[DATA] Telemetry verisi akıyor... (Hız: {js.get('speed')})")
                except: continue
            if heartbeat_received: break
            
        if not heartbeat_received:
            print("[FAIL] Heartbeat zaman aşımı.")
            
        # 2. Buton Tetikleme Testi
        print("[TEST] Buton tetikleniyor (Lamba)...")
        test_btn = {"id": "lights_low"} # Örnek ID
        sock.send(json.dumps(test_btn).encode())
        print("[OK] Buton sinyali gönderildi. Sunucu konsolunu kontrol edin.")
        
        sock.close()
        return True
    except Exception as e:
        print(f"[FAIL] TCP hatası: {e}")
        return False

def test_ipv6_connection():
    print("\n--- [TEST] IPv6 Connection ---")
    try:
        # IPv6 Loopback adresine bağlanmayı dene
        sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect(("::1", TCP_PORT))
        print("[OK] IPv6 (::1) üzerinden başarıyla bağlanıldı.")
        sock.close()
        return True
    except Exception as e:
        print(f"[INFO] IPv6 bağlantısı kurulamadı (Sistem desteklemiyor olabilir): {e}")
        return False

if __name__ == "__main__":
    print("ETS2 Professional Server - Automated Test Suite")
    print("===============================================")
    
    discovery = test_udp_discovery()
    tcp = test_tcp_connection()
    ipv6 = test_ipv6_connection()
    
    print("\n===============================================")
    print(f"UDP Discovery: {'BAŞARILI' if discovery else 'BAŞARISIZ'}")
    print(f"TCP Protocol : {'BAŞARILI' if tcp else 'BAŞARISIZ'}")
    print(f"IPv6 Support : {'BAŞARILI' if ipv6 else 'BAŞARISIZ'}")
    print("===============================================")
