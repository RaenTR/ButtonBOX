package com.ets2.buttonbox.network

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.withContext
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

    suspend fun connect(ip: String, port: Int): Boolean = withContext(Dispatchers.IO) {
        try {
            socket = Socket()
            socket?.let { s ->
                s.connect(InetSocketAddress(ip, port), 5000)
                writer = PrintWriter(s.getOutputStream(), true)
                reader = BufferedReader(InputStreamReader(s.getInputStream()))
                
                // Mesaj dinleme döngüsünü ayrı bir coroutine'de başlat
                kotlinx.coroutines.GlobalScope.launch(Dispatchers.IO) {
                    try {
                        while (socket?.isConnected == true) {
                            val line = reader?.readLine() ?: break
                            _messages.emit(line)
                        }
                    } catch (e: Exception) {
                        // Bağlantı koptu
                    } finally {
                        disconnect()
                    }
                }
            }
            true
        } catch (e: Exception) {
            false
        }
    }

    suspend fun sendMessage(msg: String) = withContext(Dispatchers.IO) {
        writer?.println(msg)
    }

    fun disconnect() {
        socket?.close()
        socket = null
    }
}
