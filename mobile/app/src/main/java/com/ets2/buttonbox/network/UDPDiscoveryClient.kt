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

        try {
            socket = DatagramSocket()
            socket.soTimeout = TIMEOUT
            socket.broadcast = true

            val broadcastAddr = getBroadcastAddress()
            val message = "DISCOVER_BUTTONBOX_SERVER".toByteArray()
            val packet = DatagramPacket(message, message.size, broadcastAddr, DISCOVERY_PORT)
            
            socket.send(packet)

            val receiveBuffer = ByteArray(1024)
            val receivePacket = DatagramPacket(receiveBuffer, receiveBuffer.size)

            // Belirlenen süre boyunca gelen yanıtları topla
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
                    // Timeout veya hata durumunda döngü devam edebilir veya bitebilir
                    break
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            socket?.close()
        }

        servers.distinctBy { it.ip } // Aynı IP'den gelen çoklu yanıtları engelle
    }

    private fun getBroadcastAddress(): InetAddress {
        val wifi = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
        val dhcp: DhcpInfo = wifi.dhcpInfo
        val broadcast = (dhcp.ipAddress and dhcp.netmask) or dhcp.netmask.inv()
        val quads = ByteArray(4)
        for (k in 0..3) quads[k] = (broadcast shr k * 8 and 0xFF).toByte()
        return InetAddress.getByAddress(quads)
    }
}
