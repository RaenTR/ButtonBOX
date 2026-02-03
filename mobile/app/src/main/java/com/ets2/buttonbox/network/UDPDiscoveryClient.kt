package com.ets2.buttonbox.network

import android.content.Context
import android.net.DhcpInfo
import android.net.wifi.WifiManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress

data class ServerInfo(
    val name: String,
    val ip: String,
    val port: Int,
    val version: String
)

class UDPDiscoveryClient(private val context: Context) {
    private val DISCOVERY_PORT = 8889
    private val TIMEOUT = 2000

    suspend fun discoverServers(): List<ServerInfo> = withContext(Dispatchers.IO) {
        val servers = mutableListOf<ServerInfo>()
        var socket: DatagramSocket? = null
        val wifiManager = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        val lock = wifiManager.createMulticastLock("ButtonBoxDiscovery")
        
        try {
            lock.acquire()
            socket = DatagramSocket()
            socket.soTimeout = TIMEOUT
            socket.broadcast = true

            val broadcastAddr = getBroadcastAddress()
            val message = "DISCOVER_BUTTONBOX_SERVER".toByteArray()
            val packet = DatagramPacket(message, message.size, broadcastAddr, DISCOVERY_PORT)
            
            socket.send(packet)

            val receiveBuffer = ByteArray(1024)
            val receivePacket = DatagramPacket(receiveBuffer, receiveBuffer.size)

            val startTime = System.currentTimeMillis()
            while (System.currentTimeMillis() - startTime < TIMEOUT) {
                try {
                    socket.receive(receivePacket)
                    val jsonStr = String(receivePacket.data, 0, receivePacket.length)
                    val json = JSONObject(jsonStr)
                    
                    if (json.getString("type") == "SERVER_INFO") {
                        servers.add(ServerInfo(
                            name = json.getString("name"),
                            ip = receivePacket.address.hostAddress ?: "",
                            port = json.getInt("port"),
                            version = json.getString("version")
                        ))
                    }
                } catch (e: Exception) {
                    break
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            socket?.close()
            if (lock.isHeld) lock.release()
        }

        servers.distinctBy { it.ip }
    }

    private fun getBroadcastAddress(): InetAddress {
        // 255.255.255.255 yerel ağdaki her cihaza ulaşmak için en güvenli yoldur.
        // Bazı Android sürümlerinde arayüz bazlı broadcast sorun çıkarabilir.
        return InetAddress.getByName("255.255.255.255")
    }
}
