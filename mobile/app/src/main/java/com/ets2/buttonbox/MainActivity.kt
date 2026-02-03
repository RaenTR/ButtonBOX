package com.ets2.buttonbox

import android.os.Bundle
import android.view.HapticFeedbackConstants
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.platform.LocalHapticFeedback
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import com.ets2.buttonbox.network.ServerInfo
import com.ets2.buttonbox.network.TCPClient
import com.ets2.buttonbox.network.UDPDiscoveryClient
import com.ets2.buttonbox.ui.components.PushButton
import com.ets2.buttonbox.ui.components.RotarySwitch
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.json.JSONObject
import android.speech.tts.TextToSpeech
import java.util.Locale
import androidx.compose.foundation.BorderStroke

data class ButtonPos(
    val id: String,
    val label: String,
    val icon: ImageVector?,
    val color: Color,
    val x: Float,
    val y: Float,
    val type: String,
    val scanCode: String,
    val iconName: String = "default"
)

@OptIn(ExperimentalMaterial3Api::class)
class MainActivity : ComponentActivity() {
    private val tcpClient = TCPClient()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val scope = rememberCoroutineScope()
            val view = LocalView.current
            val context = LocalContext.current
            val haptic = LocalHapticFeedback.current
            
            val udpClient = remember { UDPDiscoveryClient(context) }
            var discoveredServers by remember { mutableStateOf<List<ServerInfo>>(emptyList()) }
            var showServerList by remember { mutableStateOf(false) }
            var isScanning by remember { mutableStateOf(false) }

            var isConnected by remember { mutableStateOf(false) }
            var isConnecting by remember { mutableStateOf(false) }
            var ipAddress by remember { mutableStateOf("192.168.1.100") }
            var port by remember { mutableStateOf("8888") }

            var lightValue by remember { mutableIntStateOf(0) }
            var wiperValue by remember { mutableIntStateOf(0) }
            var beaconActive by remember { mutableStateOf(false) }
            var parkingBrakeActive by remember { mutableStateOf(false) }

            var city by remember { mutableStateOf("") }
            var destination by remember { mutableStateOf("") }
            var navDist by remember { mutableFloatStateOf(0f) }
            var fuelPercent by remember { mutableFloatStateOf(1f) }
            var speed by remember { mutableFloatStateOf(0f) }
            var gear by remember { mutableIntStateOf(0) }
            
            var tts: TextToSpeech? by remember { mutableStateOf(null) }

            LaunchedEffect(Unit) {
                tts = TextToSpeech(context) { status ->
                    if (status == TextToSpeech.SUCCESS) {
                        tts?.language = Locale("tr", "TR")
                    }
                }
            }

            val buttons = remember { mutableStateListOf<ButtonPos>() }

            fun fetchConfig() {
                scope.launch {
                    tcpClient.sendMessage("{\"type\":\"GET_CONFIG\"}")
                }
            }

            fun intToColor(colorInt: Int): Color {
                if (colorInt == -1) return Color.Red
                val r = (colorInt shr 16) and 0xFF
                val g = (colorInt shr 8) and 0xFF
                val b = colorInt and 0xFF
                return Color(r, g, b)
            }

            fun getIconByName(name: String): ImageVector {
                return when (name) {
                    "motor" -> Icons.Default.FlashOn
                    "brake" -> Icons.Default.StopCircle
                    "warning" -> Icons.Default.Warning
                    "light" -> Icons.Default.Lightbulb
                    "up" -> Icons.Default.ArrowUpward
                    "down" -> Icons.Default.ArrowDownward
                    "add" -> Icons.Default.AddCircle
                    "truck" -> Icons.Default.LocalShipping
                    "power" -> Icons.Default.PowerSettingsNew
                    "radio" -> Icons.Default.Radio
                    else -> Icons.Default.FlashOn
                }
            }

            val connectionState by tcpClient.connectionState.collectAsState()
            
            LaunchedEffect(connectionState) {
                isConnected = connectionState
                if (connectionState) {
                    delay(500)
                    fetchConfig()
                    
                    tcpClient.messages.collect { msg ->
                        try {
                            val j = JSONObject(msg)
                            when (j.optString("type")) {
                                "STATE" -> {
                                    speed = j.optDouble("speed", 0.0).toFloat() * 3.6f
                                    if (j.has("gear")) gear = j.getInt("gear")
                                    if (j.has("lights_level")) lightValue = j.getInt("lights_level")
                                    if (j.has("wipers")) wiperValue = j.getInt("wipers")
                                    if (j.has("beacon")) beaconActive = j.getBoolean("beacon")
                                    if (j.has("parking_brake")) parkingBrakeActive = j.getBoolean("parking_brake")
                                    city = j.optString("city", "")
                                    destination = j.optString("destination", "")
                                    navDist = j.optDouble("nav_dist", 0.0).toFloat()
                                    val fuel = j.optDouble("fuel", 100.0).toFloat()
                                    val fuelMax = j.optDouble("fuel_max", 100.0).toFloat()
                                    fuelPercent = if (fuelMax > 0) fuel / fuelMax else 1f
                                }
                                "CONFIG_DATA" -> {
                                    val btnArray = j.getJSONArray("buttons")
                                    buttons.clear()
                                    for (i in 0 until btnArray.length()) {
                                        val b = btnArray.getJSONObject(i)
                                        val iconName = b.optString("icon", "motor")
                                        buttons.add(ButtonPos(
                                            id = b.getString("id"),
                                            label = b.getString("label"),
                                            icon = getIconByName(iconName),
                                            color = intToColor(b.optInt("color", -1)),
                                            x = 0f, y = 0f,
                                            type = b.optString("type", "push"),
                                            scanCode = b.optInt("scanCode", 0).toString(),
                                            iconName = iconName
                                        ))
                                    }
                                }
                            }
                        } catch (e: Exception) { e.printStackTrace() }
                    }
                }
            }

            DisposableEffect(Unit) {
                onDispose { 
                    tcpClient.disconnect()
                    tts?.shutdown()
                }
            }

            Surface(modifier = Modifier.fillMaxSize(), color = Color(0xFF0A0A0A)) {
                if (!isConnected) {
                    Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        Column(horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(24.dp), modifier = Modifier.fillMaxWidth(0.6f)) {
                            Icon(Icons.Default.LocalShipping, null, tint = Color.Cyan, modifier = Modifier.size(80.dp).scale(1.2f))
                            Text("ETS2 PRO BUTTON BOX", color = Color.White, fontSize = 28.sp, fontWeight = FontWeight.Black)
                            Card(colors = CardDefaults.cardColors(containerColor = Color(0xFF1E1E1E)), shape = RoundedCornerShape(20.dp), modifier = Modifier.fillMaxWidth().border(1.dp, Color.White.copy(0.05f), RoundedCornerShape(20.dp))) {
                                Column(modifier = Modifier.padding(24.dp), verticalArrangement = Arrangement.spacedBy(16.dp)) {
                                    InputTile(Modifier.fillMaxWidth().height(60.dp), "IP ADRES\u0130", ipAddress) { ipAddress = it }
                                    Row(modifier = Modifier.fillMaxWidth().height(55.dp), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                                        ActionTile(Modifier.weight(1f), "TARA", Icons.Default.Search, isScanning) {
                                            isScanning = true
                                            scope.launch {
                                                val found = udpClient.discoverServers()
                                                isScanning = false
                                                if (found.isNotEmpty()) { discoveredServers = found; showServerList = true }
                                            }
                                        }
                                        ActionTile(Modifier.weight(2f), "BA\u011eLAN", Icons.Default.Power, isConnecting, Color(0xFF4CAF50)) {
                                            isConnecting = true
                                            scope.launch {
                                                if (tcpClient.connect(ipAddress, port.toIntOrNull() ?: 8888) == "SUCCESS") {
                                                    isConnected = true
                                                }
                                                isConnecting = false
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    Column(modifier = Modifier.fillMaxSize().padding(16.dp)) {
                        Row(modifier = Modifier.fillMaxWidth().height(60.dp), horizontalArrangement = Arrangement.End, verticalAlignment = Alignment.CenterVertically) {
                            ActionTile(Modifier.width(100.dp), "KOP", Icons.Default.LinkOff, false, Color.Red.copy(0.6f)) {
                                tcpClient.disconnect(); isConnected = false
                            }
                        }
                        
                        Row(modifier = Modifier.fillMaxWidth().height(100.dp), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                            DashboardTile(Modifier.weight(1f), "HIZ", speed.toInt().toString(), "KM/H", Color.Cyan)
                            DashboardTile(Modifier.weight(1f), "YAK\u0130T", (fuelPercent * 100).toInt().toString(), "%", if (fuelPercent < 0.15f) Color.Red else Color.Green)
                            DashboardTile(Modifier.weight(1.5f), "ROTA", if (destination.isNotEmpty()) destination else "YOK", if (navDist > 0) "${(navDist/1000).toInt()} KM" else "", Color.Yellow)
                        }
                        
                        Spacer(Modifier.height(16.dp))
                        
                        LazyVerticalGrid(columns = GridCells.Fixed(5), modifier = Modifier.fillMaxSize(), horizontalArrangement = Arrangement.spacedBy(12.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                            items(buttons) { btn ->
                                Box(modifier = Modifier.fillMaxWidth().aspectRatio(1.1f)) {
                                    if (btn.type.equals("rotary", ignoreCase = true)) {
                                        RotarySwitch(btn.label, if (btn.id == "light_switch") lightValue else wiperValue, 3) { newVal ->
                                            if (btn.id == "light_switch") lightValue = newVal else wiperValue = newVal
                                            view.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS)
                                            scope.launch { tcpClient.sendMessage("{\"id\":\"${btn.id}\",\"value\":$newVal}") }
                                        }
                                    } else {
                                        PushButton(btn.label, btn.icon, btn.color) {
                                            view.performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY)
                                            scope.launch { tcpClient.sendMessage("{\"id\":\"${btn.id}\"}") }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (showServerList) {
                    AlertDialog(onDismissRequest = { showServerList = false }, title = { Text("Sunucu Se\u00e7") }, text = {
                        Column {
                            discoveredServers.forEach { server ->
                                Button(onClick = { ipAddress = server.ip; showServerList = false }, modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp)) {
                                    Text("${server.name} (${server.ip})")
                                }
                            }
                        }
                    }, confirmButton = {})
                }
            }
        }
    }
}

@Composable
fun DashboardTile(modifier: Modifier, title: String, value: String, unit: String, color: Color) {
    Card(
        modifier = modifier.fillMaxHeight(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF1E1E1E)),
        shape = RoundedCornerShape(16.dp),
        border = BorderStroke(1.dp, color.copy(0.2f))
    ) {
        Column(modifier = Modifier.fillMaxSize().padding(8.dp), horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.Center) {
            Text(title, color = Color.Gray, fontSize = 10.sp, fontWeight = FontWeight.Bold)
            Row(verticalAlignment = Alignment.Bottom) {
                Text(value, color = color, fontSize = 32.sp, fontWeight = FontWeight.Black)
                if (unit.isNotEmpty()) {
                    Spacer(Modifier.width(4.dp))
                    Text(unit, color = color.copy(0.7f), fontSize = 12.sp, fontWeight = FontWeight.Bold, modifier = Modifier.padding(bottom = 6.dp))
                }
            }
        }
    }
}

@Composable
fun InputTile(modifier: Modifier, label: String, value: String, onValueChange: (String) -> Unit) {
    Box(modifier = modifier.fillMaxHeight().background(Color(0xFF1E1E1E), RoundedCornerShape(12.dp)).border(1.dp, Color.White.copy(0.05f), RoundedCornerShape(12.dp)).padding(horizontal = 12.dp, vertical = 6.dp)) {
        Column {
            Text(label, color = Color.Cyan.copy(0.7f), fontSize = 9.sp, fontWeight = FontWeight.Bold)
            BasicTextField(value = value, onValueChange = onValueChange, textStyle = TextStyle(color = Color.White, fontSize = 14.sp, fontWeight = FontWeight.Bold), modifier = Modifier.fillMaxWidth(), cursorBrush = SolidColor(Color.Cyan))
        }
    }
}

@Composable
fun ActionTile(modifier: Modifier, label: String, icon: ImageVector, isLoading: Boolean, color: Color = Color(0xFF333333), onClick: () -> Unit) {
    Box(modifier = modifier.fillMaxHeight().clip(RoundedCornerShape(12.dp)).background(color).clickable(enabled = !isLoading) { onClick() }.padding(8.dp), contentAlignment = Alignment.Center) {
        if (isLoading) CircularProgressIndicator(modifier = Modifier.size(24.dp), color = Color.White, strokeWidth = 2.dp)
        else Column(horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.Center) {
            Icon(icon, null, tint = Color.White, modifier = Modifier.size(20.dp))
            Text(label, color = Color.White, fontSize = 11.sp, fontWeight = FontWeight.Black)
        }
    }
}
