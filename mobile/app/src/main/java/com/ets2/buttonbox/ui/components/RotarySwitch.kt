package com.ets2.buttonbox.ui.components

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.spring
import androidx.compose.animation.core.Spring
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.draw.blur
import androidx.compose.foundation.border

@Composable
fun RotarySwitch(
    label: String,
    currentValue: Int,
    maxValue: Int,
    onValueChange: (Int) -> Unit
) {
    // Map 0-3 to -135 to 135 degrees (Total 270 range)
    val targetRotation = when (currentValue) {
        0 -> -135f
        1 -> -45f
        2 -> 45f
        3 -> 135f
        else -> -135f // Default for out of range values, or handle error
    }
    
    val rotation by animateFloatAsState(
        targetValue = targetRotation,
        animationSpec = spring(dampingRatio = Spring.DampingRatioNoBouncy, stiffness = Spring.StiffnessLow),
        label = "rotation"
    )

    Box(
        modifier = Modifier
            .padding(8.dp)
            .fillMaxWidth()
            .background(Color(0xFF1A1A1A), RoundedCornerShape(20.dp))
            .border(1.dp, Color.White.copy(alpha = 0.05f), RoundedCornerShape(20.dp))
            .padding(12.dp),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Text(
                text = label.uppercase(),
                color = Color.Gray.copy(alpha = 0.8f),
                fontSize = 11.sp,
                fontWeight = FontWeight.ExtraBold,
                modifier = Modifier.padding(bottom = 12.dp)
            )

            // Döner Düğme (Knob) Tasarımı - Metalik Efekt
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier
                    .size(90.dp)
                    .shadow(12.dp, CircleShape)
                    .background(
                        brush = Brush.sweepGradient(
                            colors = listOf(
                                Color(0xFF333333),
                                Color(0xFF111111),
                                Color(0xFF444444),
                                Color(0xFF111111),
                                Color(0xFF333333)
                            )
                        ),
                        shape = CircleShape
                    )
                    .border(2.dp, Color.DarkGray, CircleShape)
                    .clickable {
                        val nextValue = (currentValue + 1) % (maxValue + 1)
                        onValueChange(nextValue)
                    }
            ) {
                // Konik Gölgelendirme Layer'ı
                Canvas(modifier = Modifier.fillMaxSize()) {
                    drawCircle(
                        brush = Brush.radialGradient(
                            0.0f to Color.White.copy(alpha = 0.1f),
                            1.0f to Color.Transparent
                        ),
                        radius = size.width / 2.2f
                    )
                }

                // Gösterge İbre Çizimi (Neon Glow)
                Canvas(modifier = Modifier.fillMaxSize().rotate(rotation - 120f)) {
                    // Glow
                    drawLine(
                        color = Color(0xFFFFD600).copy(alpha = 0.3f),
                        start = Offset(size.width / 2, 10f),
                        end = Offset(size.width / 2, 35f),
                        strokeWidth = 12f,
                        cap = StrokeCap.Round
                    )
                    // Main Line
                    drawLine(
                        color = Color(0xFFFFD600),
                        start = Offset(size.width / 2, 12f),
                        end = Offset(size.width / 2, 32f),
                        strokeWidth = 6f,
                        cap = StrokeCap.Round
                    )
                }
            }

            // LED Kademe Göstergeleri (Glow Effect)
            Row(
                modifier = Modifier.padding(top = 16.dp),
                horizontalArrangement = Arrangement.spacedBy(10.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                for (i in 0..maxValue) {
                    val isActive = i == currentValue
                    Box(contentAlignment = Alignment.Center) {
                        if (isActive) {
                            // LED Glow
                            Box(
                                modifier = Modifier
                                    .size(12.dp)
                                    .blur(4.dp)
                                    .background(Color(0xFF00E676).copy(alpha = 0.4f), CircleShape)
                            )
                        }
                        Box(
                            modifier = Modifier
                                .size(if (isActive) 8.dp else 6.dp)
                                .background(
                                    color = if (isActive) Color(0xFF00E676) else Color.Gray.copy(alpha = 0.2f),
                                    shape = CircleShape
                                )
                        )
                    }
                }
            }
        }
    }
}
