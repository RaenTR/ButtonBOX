package com.ets2.buttonbox.network

import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.withContext
import kotlinx.coroutines.launch
import kotlinx.coroutines.Dispatchers
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.PrintWriter
import java.net.Socket
import java.net.InetSocketAddress

class TCPClient {
    private var socket: Socket? = null
    private var writer: PrintWriter? = null
    private var reader: BufferedReader? = null

    private val _messages = MutableSharedFlow<String>()
    val messages = _messages.asSharedFlow()

    private val _connectionState = MutableStateFlow(false)
    val connectionState = _connectionState.asStateFlow()

    suspend fun connect(ip: String, port: Int): String = withContext(Dispatchers.IO) {
        try {
            socket = Socket()
            socket?.let { s ->
                s.connect(InetSocketAddress(ip, port), 3000)
                writer = PrintWriter(s.getOutputStream(), true)
                reader = BufferedReader(InputStreamReader(s.getInputStream()))
                _connectionState.value = true
                
                kotlinx.coroutines.MainScope().launch(Dispatchers.IO) {
                    try {
                        while (socket?.isConnected == true) {
                            val line = reader?.readLine() ?: break
                            _messages.emit(line)
                        }
                    } catch (e: Exception) {
                        e.printStackTrace()
                    } finally {
                        disconnect()
                    }
                }
            }
            "SUCCESS"
        } catch (e: java.net.ConnectException) {
            "SUNUCU REDDETTİ (Sunucu Açık mı? Firewall Kapalı mı?)"
        } catch (e: java.net.SocketTimeoutException) {
            "ZAMAN AŞIMI (IP/Port Yanlış Olabilir)"
        } catch (e: java.net.NoRouteToHostException) {
            "SUNUCUYA ULAŞILAMIYOR (Aynı Ağda mısınız?)"
        } catch (e: Exception) {
            "HATA: ${e.message}"
        }
    }

    suspend fun sendMessage(msg: String) = withContext(Dispatchers.IO) {
        writer?.println(msg)
    }

    fun disconnect() {
        try {
            socket?.close()
        } catch (e: Exception) {
            e.printStackTrace()
        }
        socket = null
        writer = null
        reader = null
        _connectionState.value = false
    }
}
