package com.ets2.buttonbox

import android.os.Bundle
import android.view.HapticFeedbackConstants
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.clip
import androidx.compose.foundation.border
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.compose.material3.*
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.window.Dialog
import com.ets2.buttonbox.ui.components.MapView
import com.ets2.buttonbox.ui.components.PushButton
import com.ets2.buttonbox.ui.components.RotarySwitch
import com.ets2.buttonbox.ui.components.KeyboardPicker
import com.ets2.buttonbox.utils.KeyInfo
import com.ets2.buttonbox.utils.ScanCodes
import com.ets2.buttonbox.network.TCPClient
import com.ets2.buttonbox.network.UDPDiscoveryClient
import com.ets2.buttonbox.network.ServerInfo
import kotlinx.coroutines.launch
import org.json.JSONObject
import org.json.JSONArray
import android.content.Context
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.unit.IntOffset
import kotlin.math.roundToInt

data class ButtonPos(
    val id: String,
    val label: String,
    val icon: ImageVector?,
    val color: Color,
    val x: Float,
    val y: Float,
    val type: String = "button", // button or rotary
    val scanCode: String = "0"
)

class MainActivity : ComponentActivity() {
    private val tcpClient = TCPClient()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val scope = rememberCoroutineScope()
            val view = LocalView.current
            val context = LocalContext.current
            
            val udpClient = remember { UDPDiscoveryClient(context) }
            var discoveredServers by remember { mutableStateOf<List<ServerInfo>>(emptyList()) }
            var showServerList by remember { mutableStateOf(false) }
            var isScanning by remember { mutableStateOf(false) }

            var isConnected by remember { mutableStateOf(false) }
            var isConnecting by remember { mutableStateOf(false) }
            var ipAddress by remember { mutableStateOf("192.168.1.100") }
            var port by remember { mutableStateOf("8888") }
            
            // Telemetry States
            var speed by remember { mutableFloatStateOf(0f) }
            var isEngineRunning by remember { mutableStateOf(false) }
            var fuel by remember { mutableFloatStateOf(0f) }
            var fuelMax by remember { mutableFloatStateOf(100f) }
            var adblue by remember { mutableFloatStateOf(0f) }
            var wearEngine by remember { mutableFloatStateOf(0f) }
            var wearChassis by remember { mutableFloatStateOf(0f) }
            var navDist by remember { mutableFloatStateOf(0f) }
            var navTime by remember { mutableFloatStateOf(0f) }
            var consumption by remember { mutableFloatStateOf(0f) }
            var restStop by remember { mutableIntStateOf(0) }
            var truckName by remember { mutableStateOf("") }
            var currentCity by remember { mutableStateOf("") }
            var currentCargo by remember { mutableStateOf("") }
            var currentDest by remember { mutableStateOf("") }
            
            // Map Coordinates
            var posX by remember { mutableFloatStateOf(0f) }
            var posZ by remember { mutableFloatStateOf(0f) }
            var heading by remember { mutableFloatStateOf(0f) }
            
            var lightValue by remember { mutableIntStateOf(0) }
            var wiperValue by remember { mutableIntStateOf(0) }

            // Animated States for smoothness
            val animatedSpeed by animateFloatAsState(targetValue = speed, label = "speed")
            val animatedFuel by animateFloatAsState(targetValue = if (fuelMax > 0) fuel / fuelMax else 0f, label = "fuel")

            // Edit Mode States
            var isEditMode by remember { mutableStateOf(false) }
            var showEditDialog by remember { mutableStateOf(false) }
            var showKeyboardPicker by remember { mutableStateOf(false) }
            var isDiscovering by remember { mutableStateOf(false) }
            var editingButtonId by remember { mutableStateOf("") }
            var editingButtonLabel by remember { mutableStateOf("") }
            var editingButtonCode by remember { mutableStateOf("") }

            // Layout Editor States
            val initialButtons = listOf(
                ButtonPos("motor", "MOTOR", Icons.Default.FlashOn, Color.Red, 20f, 20f, "button", "18"),
                ButtonPos("parking_brake", "EL FRENİ", Icons.Default.StopCircle, Color(0xFFFF9100), 220f, 20f, "button", "57"),
                ButtonPos("hazards", "DÖRTLÜLER", Icons.Default.Warning, Color.Red, 20f, 130f, "button", "33"),
                ButtonPos("beacon", "TEPE LAMBASI", Icons.Default.Lightbulb, Color.Cyan, 220f, 130f, "button", "24"),
                ButtonPos("l_win_down", "SOL CAM ↓", Icons.Default.ArrowDownward, Color.White, 20f, 240f, "button", "82"),
                ButtonPos("l_win_up", "SOL CAM ↑", Icons.Default.ArrowUpward, Color.White, 220f, 240f, "button", "79"),
                ButtonPos("light_switch", "FARLAR", null, Color.White, 20f, 350f, "rotary", "0"),
                ButtonPos("wipers", "SİLECEKLER", null, Color.White, 220f, 350f, "rotary", "0")
            )
            
            var buttons by remember { mutableStateOf(initialButtons) }

            // Load saved positions
            LaunchedEffect(Unit) {
                val prefs = context.getSharedPreferences("layout", Context.MODE_PRIVATE)
                val savedJson = prefs.getString("buttons", null)
                if (savedJson != null) {
                    try {
                        val arr = JSONArray(savedJson)
                        val newList = buttons.map { btn ->
                            var updatedBtn = btn
                            for (i in 0 until arr.length()) {
                                val obj = arr.getJSONObject(i)
                                if (obj.getString("id") == btn.id) {
                                    updatedBtn = btn.copy(
                                        x = obj.getDouble("x").toFloat(),
                                        y = obj.getDouble("y").toFloat(),
                                        label = obj.optString("label", btn.label),
                                        scanCode = obj.optString("scanCode", btn.scanCode)
                                    )
                                    break
                                }
                            }
                            updatedBtn
                        }
                        buttons = newList
                    } catch (e: Exception) {}
                }
            }

            // Save positions when changed
            val savePositions = {
                val prefs = context.getSharedPreferences("layout", Context.MODE_PRIVATE)
                val arr = JSONArray()
                buttons.forEach { btn ->
                    val obj = JSONObject()
                    obj.put("id", btn.id)
                    obj.put("x", btn.x)
                    obj.put("y", btn.y)
                    obj.put("label", btn.label)
                    obj.put("scanCode", btn.scanCode)
                    arr.put(obj)
                }
                prefs.edit().putString("buttons", arr.toString()).apply()
            }

            // Gelen mesajları dinle (Telemetry update & Discovery)
            LaunchedEffect(isConnected) {
                if (isConnected) {
                    tcpClient.messages.collect { msg ->
                        try {
                            val json = JSONObject(msg)
                            when (json.getString("type")) {
                                "STATE" -> {
                                    speed = json.optDouble("speed", 0.0).toFloat()
                                    isEngineRunning = json.optBoolean("engine", false)
                                    fuel = json.optDouble("fuel", 0.0).toFloat()
                                    fuelMax = json.optDouble("fuel_max", 100.0).toFloat()
                                    adblue = json.optDouble("adblue", 0.0).toFloat()
                                    // navDist = json.optDouble("nav_dist", 0.0).toFloat() // Unused but kept for future if needed or commented out
                                    navTime = json.optDouble("nav_time", 0.0).toFloat()
                                    consumption = json.optDouble("consumption", 0.0).toFloat()
                                    restStop = json.optInt("rest_stop", 0)
                                    truckName = json.optString("truck", "")
                                    // currentCity = json.optString("city", "") // Unused
                                    currentCargo = json.optString("cargo", "")
                                    currentDest = json.optString("destination", "")
                                    
                                    // Map data
                                    posX = json.optDouble("posX", 0.0).toFloat()
                                    posZ = json.optDouble("posZ", 0.0).toFloat()
                                    heading = json.optDouble("heading", 0.0).toFloat()
                                }
                                "DISCOVERY_RESULT" -> {
                                    editingButtonCode = json.getInt("scanCode").toString()
                                    isDiscovering = false
                                }
                            }
                        } catch (e: Exception) {
                            // Eski format veya hatalı mesaj durumunda sessizce geç
                        }
                    }
                }
            }

            BoxWithConstraints(
                modifier = Modifier
                    .fillMaxSize()
                    .background(
                        Brush.verticalGradient(
                            colors = listOf(Color(0xFF0F0F0F), Color(0xFF1A1A1A))
                        )
                    )
            ) {
                val isLandscape = this.maxWidth > this.maxHeight
                val columns = if (isLandscape) 4 else 2

                Column(modifier = Modifier.fillMaxSize().padding(16.dp)) {
                    
                    // --- ÜST BAR: TELEMETRY DASHBOARD (Premium Glassmorphism) ---
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(if (isLandscape) 180.dp else 140.dp)
                            .background(Color(0xFF151A15).copy(alpha = 0.6f), RoundedCornerShape(24.dp))
                            .border(1.2.dp, Color(0xFF4CAF50).copy(alpha = 0.2f), RoundedCornerShape(24.dp))
                    ) {
                        Card(
                            modifier = Modifier.fillMaxSize(),
                            colors = CardDefaults.cardColors(containerColor = Color.Transparent),
                            shape = RoundedCornerShape(24.dp)
                        ) {
                            Row(
                                modifier = Modifier.fillMaxSize().padding(12.dp),
                                verticalAlignment = Alignment.CenterVertically,
                                horizontalArrangement = Arrangement.spacedBy(16.dp)
                            ) {
                                // Harita / Radar (F3 GPS Style - Prominent)
                                MapView(
                                    posX = posX,
                                    posZ = posZ,
                                    heading = heading,
                                    modifier = Modifier.size(if (isLandscape) 160.dp else 110.dp)
                                )

                                // Telemetry Verileri
                                Row(
                                    modifier = Modifier.weight(1f),
                                    verticalAlignment = Alignment.CenterVertically,
                                    horizontalArrangement = Arrangement.SpaceBetween
                                ) {
                                    // Hız Göstergesi (Neon Effect)
                                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                                        Text("SPEED", color = Color(0xFF4CAF50).copy(alpha = 0.6f), fontSize = 11.sp, fontWeight = FontWeight.Bold)
                                        Box(contentAlignment = Alignment.Center) {
                                            // Glow Layer
                                            Text(
                                                text = String.format("%.0f", animatedSpeed),
                                                color = if (animatedSpeed > 80) Color.Red.copy(alpha = 0.4f) else Color(0xFF00E676).copy(alpha = 0.3f),
                                                fontSize = if (isLandscape) 64.sp else 46.sp,
                                                fontWeight = FontWeight.Black,
                                                modifier = Modifier.blur(6.dp)
                                            )
                                            Text(
                                                text = String.format("%.0f", animatedSpeed),
                                                color = if (animatedSpeed > 80) Color.Red else Color(0xFFC8E6C9),
                                                fontSize = if (isLandscape) 60.sp else 44.sp,
                                                fontWeight = FontWeight.Black
                                            )
                                        }
                                        Text("KM/H", color = Color.Gray, fontSize = 10.sp, fontWeight = FontWeight.Medium)
                                    }
                                    
                                    // Yakıt ve Hasar
                                    Column(modifier = Modifier.width(if (isLandscape) 180.dp else 120.dp), verticalArrangement = Arrangement.Center) {
                                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                                            Text("FUEL / ENERGY", color = Color.Gray, fontSize = 10.sp, fontWeight = FontWeight.Bold)
                                            Text("${(animatedFuel * 100).toInt()}%", color = Color.White, fontSize = 10.sp)
                                        }
                                        LinearProgressIndicator(
                                            progress = { animatedFuel.coerceIn(0f, 1f) },
                                            modifier = Modifier.fillMaxWidth().height(10.dp).clip(RoundedCornerShape(5.dp)),
                                            color = if (animatedFuel < 0.15f) Color.Red else Color(0xFF00E676),
                                            trackColor = Color.DarkGray.copy(alpha = 0.3f),
                                        )
                                        Spacer(modifier = Modifier.height(16.dp))
                                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                                            Text("ENGINE WEAR", color = Color.Gray, fontSize = 10.sp, fontWeight = FontWeight.Bold)
                                            Text("${(wearEngine * 100).toInt()}%", color = if (wearEngine > 0.2f) Color.Red else Color.Gray, fontSize = 10.sp)
                                        }
                                        LinearProgressIndicator(
                                            progress = { (1f - wearEngine).coerceIn(0f, 1f) },
                                            modifier = Modifier.fillMaxWidth().height(6.dp).clip(RoundedCornerShape(3.dp)),
                                            color = if (wearEngine > 0.2f) Color.Red else Color(0xFF4CAF50),
                                            trackColor = Color.DarkGray.copy(alpha = 0.3f),
                                        )
                                    }

                                    // Detaylı Lojistik / Log Verileri
                                    Column(
                                        modifier = Modifier.weight(1f).padding(horizontal = 8.dp),
                                        verticalArrangement = Arrangement.spacedBy(4.dp)
                                    ) {
                                        LogItem("ARAÇ", truckName)
                                        LogItem("YÜK", currentCargo.ifEmpty { "BOŞTA" })
                                        LogItem("HEDEF", currentDest.ifEmpty { "--" })
                                        
                                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                                            val hours = (navTime / 3600).toInt()
                                            val mins = ((navTime % 3600) / 60).toInt()
                                            LogItem("KALAN SÜRE", String.format("%02d:%02d", hours, mins))
                                            LogItem("TÜKETİM", String.format("%.1f L", consumption * 100))
                                        }
                                    }

                                    // Motor ve Bağlantı Durumu
                                    Column(
                                        horizontalAlignment = Alignment.CenterHorizontally,
                                        modifier = Modifier.padding(end = 4.dp)
                                    ) {
                                        Icon(
                                            imageVector = Icons.Default.PowerSettingsNew,
                                            contentDescription = null,
                                            tint = if (isEngineRunning) Color(0xFF00E676) else Color.DarkGray,
                                            modifier = Modifier.size(32.dp)
                                        )
                                        Spacer(modifier = Modifier.height(12.dp))
                                        // Dinlenme Barı
                                        val restProgress = (restStop / (11 * 3600f)).coerceIn(0f, 1f)
                                        Box(contentAlignment = Alignment.Center) {
                                            CircularProgressIndicator(
                                                progress = { restProgress },
                                                modifier = Modifier.size(24.dp),
                                                color = if (restProgress < 0.1f) Color.Red else Color.Cyan,
                                                strokeWidth = 3.dp,
                                                trackColor = Color.DarkGray.copy(alpha = 0.3f)
                                            )
                                            Icon(Icons.Default.Bed, null, tint = Color.Gray, modifier = Modifier.size(12.dp))
                                        }
                                        Text(if (isConnected) "ONLINE" else "OFFLINE", color = Color.Gray, fontSize = 8.sp, fontWeight = FontWeight.Bold)
                                    }
                                }
                            }
                        }
                    }

                    // --- DÜZENLEME MODU ŞALTERİ ---
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(vertical = 4.dp)
                            .background(Color(0xFF252525).copy(alpha = 0.5f), MaterialTheme.shapes.medium)
                            .padding(horizontal = 12.dp, vertical = 4.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Text("DÜZENLEME (TUŞ AYARLAMA) MODU", color = Color.Gray, fontSize = 11.sp, fontWeight = FontWeight.Bold)
                        Switch(
                            checked = isEditMode,
                            onCheckedChange = { 
                                isEditMode = it 
                                if (!it) savePositions() // Save when exiting edit mode
                            },
                            colors = SwitchDefaults.colors(checkedThumbColor = Color.Cyan)
                        )
                    }

                    Spacer(modifier = Modifier.height(8.dp))

                    // --- BAĞLANTI AYARLARI (IP ve PORT) ---
                    if (!isConnected) {
                        Row(
                            modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
                            horizontalArrangement = Arrangement.spacedBy(8.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            TextField(
                                value = ipAddress,
                                onValueChange = { ipAddress = it },
                                modifier = Modifier.weight(1.8f).height(56.dp),
                                label = { Text("IP", fontSize = 10.sp) },
                                singleLine = true,
                                enabled = !isConnecting,
                                colors = TextFieldDefaults.colors(
                                    focusedContainerColor = Color(0xFF1E1E1E),
                                    unfocusedContainerColor = Color(0xFF1E1E1E),
                                    focusedTextColor = Color.White,
                                    unfocusedTextColor = Color.White
                                )
                            )
                            TextField(
                                value = port,
                                onValueChange = { port = it },
                                modifier = Modifier.weight(1f).height(56.dp),
                                label = { Text("PORT", fontSize = 10.sp) },
                                singleLine = true,
                                enabled = !isConnecting,
                                colors = TextFieldDefaults.colors(
                                    focusedContainerColor = Color(0xFF1E1E1E),
                                    unfocusedContainerColor = Color(0xFF1E1E1E),
                                    focusedTextColor = Color.White,
                                    unfocusedTextColor = Color.White
                                )
                            )
                            
                            // Tarama Butonu (Discovery)
                            IconButton(
                                onClick = {
                                    isScanning = true
                                    scope.launch {
                                        val found = udpClient.discoverServers()
                                        discoveredServers = found
                                        isScanning = false
                                        if (found.isNotEmpty()) {
                                            showServerList = true
                                        } else {
                                            android.widget.Toast.makeText(context, "Sunucu bulunamadı!", android.widget.Toast.LENGTH_SHORT).show()
                                        }
                                    }
                                },
                                modifier = Modifier.size(56.dp).background(Color(0xFF333333), MaterialTheme.shapes.medium),
                                enabled = !isScanning && !isConnecting
                            ) {
                                if (isScanning) {
                                    CircularProgressIndicator(modifier = Modifier.size(24.dp), color = Color.Cyan, strokeWidth = 2.dp)
                                } else {
                                    Icon(Icons.Default.Search, contentDescription = "Ara", tint = Color.Cyan)
                                }
                            }

                            Button(
                                onClick = {
                                    isConnecting = true
                                    scope.launch {
                                        val success = tcpClient.connect(ipAddress, port.toIntOrNull() ?: 8888)
                                        isConnected = success
                                        isConnecting = false
                                        if (!success) {
                                            android.widget.Toast.makeText(context, "Bağlantı Başarısız!", android.widget.Toast.LENGTH_SHORT).show()
                                        }
                                    }
                                },
                                modifier = Modifier.height(56.dp).weight(1.2f),
                                enabled = !isConnecting,
                                colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF6200EE))
                            ) {
                                Text(if (isConnecting) "..." else "BAĞLAN", fontSize = 11.sp)
                            }
                        }
                    } else {
                        // Bağlıyken sadece Kes butonu (Opsiyonel: Daha kompakt bir bar)
                        Button(
                            onClick = {
                                tcpClient.disconnect()
                                isConnected = false
                            },
                            modifier = Modifier.fillMaxWidth().height(40.dp).padding(bottom = 8.dp),
                            colors = ButtonDefaults.buttonColors(containerColor = Color.Red.copy(alpha = 0.7f))
                        ) {
                            Text("BAĞLANTIYI KES", fontSize = 12.sp, fontWeight = FontWeight.Bold)
                        }
                    }

                    // --- ANA KONTROL PANELİ (DÜZENLENEBİLİİR BOX) ---
                    Box(modifier = Modifier.fillMaxSize()) {
                        buttons.forEach { btn ->
                            Box(
                                modifier = Modifier
                                    .offset { IntOffset(btn.x.roundToInt(), btn.y.roundToInt()) }
                                    .width(180.dp) // Sabit genişlik veya değişkene bağlanabilir
                                    .pointerInput(isEditMode) {
                                        if (isEditMode) {
                                            detectDragGestures { change, dragAmount ->
                                                change.consume()
                                                buttons = buttons.map {
                                                    if (it.id == btn.id) it.copy(x = it.x + dragAmount.x, y = it.y + dragAmount.y)
                                                    else it
                                                }
                                            }
                                        }
                                    }
                            ) {
                                if (btn.type == "rotary") {
                                    RotarySwitch(btn.label, if (btn.id == "light_switch") lightValue else wiperValue, 3) { newVal ->
                                        if (isEditMode) {
                                            editingButtonId = btn.id; editingButtonLabel = btn.label; showEditDialog = true
                                        } else {
                                            if (btn.id == "light_switch") lightValue = newVal else wiperValue = newVal
                                            view.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS)
                                            scope.launch { tcpClient.sendMessage("{\"id\":\"${btn.id}\",\"value\":$newVal}") }
                                        }
                                    }
                                } else {
                                    PushButton(btn.label, btn.icon, btn.color) {
                                        if (isEditMode) {
                                            editingButtonId = btn.id; editingButtonLabel = btn.label; editingButtonCode = btn.scanCode; showEditDialog = true
                                        } else {
                                            scope.launch { tcpClient.sendMessage("{\"id\":\"${btn.id}\"}") }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // --- TUŞ DÜZENLEME DİYALOĞU ---
                if (showEditDialog) {
                    AlertDialog(
                        onDismissRequest = { showEditDialog = false },
                        containerColor = Color(0xFF1E1E1E),
                        title = { Text("Tuşu Düzenle: $editingButtonId", color = Color.White) },
                        text = {
                            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                                TextField(
                                    value = editingButtonLabel,
                                    onValueChange = { editingButtonLabel = it },
                                    label = { Text("Buton İsmi") },
                                    colors = TextFieldDefaults.colors(focusedTextColor = Color.White, unfocusedTextColor = Color.White)
                                )
                                
                                Row(
                                    modifier = Modifier.fillMaxWidth(),
                                    verticalAlignment = Alignment.CenterVertically,
                                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                                ) {
                                    TextField(
                                        value = editingButtonCode,
                                        onValueChange = { editingButtonCode = it },
                                        modifier = Modifier.weight(1f),
                                        label = { Text("DirectX Tuş Kodu") },
                                        colors = TextFieldDefaults.colors(focusedTextColor = Color.White, unfocusedTextColor = Color.White)
                                    )
                                    Button(
                                        onClick = {
                                            isDiscovering = true
                                            scope.launch { tcpClient.sendMessage("{\"type\":\"START_DISCOVERY\",\"id\":\"$editingButtonId\"}") }
                                        },
                                        enabled = !isDiscovering,
                                        modifier = Modifier.height(56.dp),
                                        colors = ButtonDefaults.buttonColors(containerColor = if (isDiscovering) Color(0xFFFFA500) else Color.Cyan)
                                    ) {
                                        Text(if (isDiscovering) "TUŞA BASIN" else "ALGILA", fontSize = 10.sp, color = Color.Black)
                                    }
                                }
                                
                                Button(
                                    onClick = { showKeyboardPicker = true },
                                    modifier = Modifier.fillMaxWidth().height(40.dp),
                                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF333333)),
                                    shape = MaterialTheme.shapes.small
                                ) {
                                    Icon(Icons.Default.Keyboard, contentDescription = null, modifier = Modifier.size(16.dp))
                                    Spacer(modifier = Modifier.width(8.dp))
                                    Text("KLAVYEDEN SEÇ (GÖRSEL)", fontSize = 11.sp, color = Color.White)
                                }

                                Text("İpucu: 'ALGILA' ile tuşa basabilir veya 'GÖRSEL SEÇİM' yapabilirsin.", fontSize = 10.sp, color = Color.Gray)
                            }
                        },
                        confirmButton = {
                            Button(onClick = {
                                scope.launch {
                                    val code = editingButtonCode.toUIntOrNull() ?: 0u
                                    tcpClient.sendMessage("{\"type\":\"CONFIG_UPDATE\",\"id\":\"$editingButtonId\",\"label\":\"$editingButtonLabel\",\"scanCode\":$code}")
                                }
                                showEditDialog = false
                            }) { Text("SİSTEME KAYDET") }
                        },
                        dismissButton = {
                            TextButton(onClick = { showEditDialog = false }) { Text("İPTAL", color = Color.Gray) }
                        }
                    )
                }

                // --- GÖRSEL KLAVYE SEÇİCİ OVERLAY ---
                if (showKeyboardPicker) {
                    Dialog(onDismissRequest = { showKeyboardPicker = false }) {
                        Card(
                            modifier = Modifier
                                .fillMaxWidth(0.95f)
                                .fillMaxHeight(0.85f),
                            colors = CardDefaults.cardColors(containerColor = Color(0xFF121212)),
                            border = androidx.compose.foundation.BorderStroke(1.dp, Color.Cyan.copy(alpha = 0.3f))
                        ) {
                            Column {
                                Row(
                                    modifier = Modifier.fillMaxWidth().padding(8.dp),
                                    horizontalArrangement = Arrangement.SpaceBetween,
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Text("TUŞ SEÇİN", color = Color.White, fontWeight = FontWeight.Bold)
                                    IconButton(onClick = { showKeyboardPicker = false }) {
                                        Icon(Icons.Default.Close, contentDescription = null, tint = Color.Red)
                                    }
                                }
                                KeyboardPicker { selectedKey ->
                                    editingButtonCode = selectedKey.scanCode.toString()
                                    // İsteğe bağlı: İsmi de güncelleyebilirsin
                                    // editingButtonLabel = selectedKey.label
                                    showKeyboardPicker = false
                                }
                            }
                        }
                    }
                }
                // --- SUNUCU LİSTESİ DİYALOĞU ---
                if (showServerList) {
                    AlertDialog(
                        onDismissRequest = { showServerList = false },
                        containerColor = Color(0xFF1E1E1E),
                        title = { Text("Bulunan Sunucular", color = Color.White) },
                        text = {
                            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                                discoveredServers.forEach { server ->
                                    Button(
                                        onClick = {
                                            ipAddress = server.ip
                                            port = server.port.toString()
                                            showServerList = false
                                        },
                                        modifier = Modifier.fillMaxWidth(),
                                        colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF333333))
                                    ) {
                                        Column {
                                            Text(server.name, color = Color.White, fontWeight = FontWeight.Bold)
                                            Text("${server.ip}:${server.port} (v${server.version})", color = Color.Gray, fontSize = 10.sp)
                                        }
                                    }
                                }
                            }
                        },
                        confirmButton = {},
                        dismissButton = {
                            TextButton(onClick = { showServerList = false }) { Text("İPTAL", color = Color.Gray) }
                        }
                    )
                }
            }
        }
    }
}
@Composable
fun LogItem(label: String, value: String) {
    Column(modifier = Modifier.padding(vertical = 1.dp)) {
        Text(
            text = label,
            color = Color(0xFF4CAF50).copy(alpha = 0.6f),
            fontSize = 8.sp,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.padding(bottom = 0.dp)
        )
        Text(
            text = value.ifEmpty { "--" },
            color = Color.White,
            fontSize = 11.sp,
            fontWeight = FontWeight.ExtraBold,
            maxLines = 1,
            overflow = TextOverflow.Ellipsis
        )
    }
}
