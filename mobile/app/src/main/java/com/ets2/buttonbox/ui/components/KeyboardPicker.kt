package com.ets2.buttonbox.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.ets2.buttonbox.utils.KeyInfo
import com.ets2.buttonbox.utils.ScanCodes

@Composable
fun KeyboardPicker(
    onKeySelected: (KeyInfo) -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF121212))
            .padding(8.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            "KLAVYEDEN TUŞ SEÇİN",
            color = Color.Cyan,
            fontSize = 14.sp,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.padding(bottom = 8.dp)
        )

        LazyColumn(
            modifier = Modifier.weight(1f, fill = false),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            // Ana Klavye Satırları
            item { KeyRow(ScanCodes.ROW1, onKeySelected) }
            item { KeyRow(ScanCodes.ROW2, onKeySelected) }
            item { KeyRow(ScanCodes.ROW3, onKeySelected) }
            item { KeyRow(ScanCodes.ROW4, onKeySelected) }
            item { KeyRow(ScanCodes.ROW5, onKeySelected) }
            item { KeyRow(ScanCodes.ROW6, onKeySelected) }
            
            item { Spacer(modifier = Modifier.height(12.dp)) }
            
            // Navigasyon ve Oklar
            item {
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        KeyRow(ScanCodes.NAVIGATION.chunked(3)[0], onKeySelected)
                        KeyRow(ScanCodes.NAVIGATION.chunked(3)[1], onKeySelected)
                        KeyRow(ScanCodes.NAVIGATION.chunked(3)[2], onKeySelected)
                    }
                    Column(verticalArrangement = Arrangement.spacedBy(4.dp), horizontalAlignment = Alignment.CenterHorizontally) {
                        KeyItem(ScanCodes.ARROWS[0], onKeySelected) // Up
                        Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
                            KeyItem(ScanCodes.ARROWS[1], onKeySelected) // Left
                            KeyItem(ScanCodes.ARROWS[2], onKeySelected) // Down
                            KeyItem(ScanCodes.ARROWS[3], onKeySelected) // Right
                        }
                    }
                }
            }

            item { Spacer(modifier = Modifier.height(12.dp)) }

            // NUMPAD Bölümü
            item {
                Text("NUMPAD (SAYI TUŞLARI)", color = Color.Gray, fontSize = 10.sp, modifier = Modifier.padding(bottom = 4.dp))
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    val chunks = ScanCodes.NUMPAD.chunked(4)
                    chunks.forEach { chunk ->
                        KeyRow(chunk, onKeySelected)
                    }
                }
            }
        }
    }
}

@Composable
fun KeyRow(keys: List<KeyInfo>, onKeySelected: (KeyInfo) -> Unit) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.Center,
        verticalAlignment = Alignment.CenterVertically
    ) {
        keys.forEach { key ->
            KeyItem(key, onKeySelected)
            Spacer(modifier = Modifier.width(4.dp))
        }
    }
}

@Composable
fun KeyItem(key: KeyInfo, onKeySelected: (KeyInfo) -> Unit) {
    val weight = when (key.label) {
        "SPACE" -> 4f
        "ESC", "TAB", "CAPS", "LSHFT", "RSHFT", "ENT", "LCTRL", "RCTRL", "LALT", "RALT", "WIN" -> 1.5f
        else -> 1f
    }

    Box(
        modifier = Modifier
            .width(if (key.label == "SPACE") 120.dp else 45.dp)
            .height(40.dp)
            .clip(RoundedCornerShape(4.dp))
            .background(Color(0xFF252525))
            .clickable { onKeySelected(key) },
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Text(
                text = key.label,
                color = Color.White,
                fontSize = if (key.label.length > 3) 8.sp else 11.sp,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = key.scanCode.toString(),
                color = Color.Gray,
                fontSize = 7.sp
            )
        }
    }
}
