package com.ets2.buttonbox.ui.components

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsPressedAsState
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

import androidx.compose.ui.draw.blur
import androidx.compose.foundation.border

@Composable
fun PushButton(
    label: String,
    icon: ImageVector? = null,
    activeColor: Color = Color(0xFF00B0FF),
    onClick: () -> Unit
) {
    val interactionSource = remember { MutableInteractionSource() }
    val isPressed by interactionSource.collectIsPressedAsState()
    val scale by animateFloatAsState(if (isPressed) 0.94f else 1f, label = "scale")

    Box(
        contentAlignment = Alignment.Center,
        modifier = Modifier
            .padding(8.dp)
            .height(95.dp)
            .fillMaxWidth()
            .scale(scale)
            .background(
                brush = Brush.verticalGradient(
                    colors = listOf(
                        Color(0xFF333333),
                        Color(0xFF1A1A1A)
                    )
                ),
                shape = RoundedCornerShape(20.dp)
            )
            .border(
                width = 1.dp,
                color = if (isPressed) activeColor else Color.White.copy(alpha = 0.05f),
                shape = RoundedCornerShape(20.dp)
            )
            .clickable(
                interactionSource = interactionSource,
                indication = null,
                onClick = onClick
            )
    ) {
        // Glow Effect behind when pressed
        if (isPressed) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .blur(12.dp)
                    .background(activeColor.copy(alpha = 0.2f), RoundedCornerShape(20.dp))
            )
        }

        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            if (icon != null) {
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    tint = if (isPressed) activeColor else Color.White.copy(alpha = 0.8f),
                    modifier = Modifier.size(32.dp).padding(bottom = 4.dp)
                )
            }
            Text(
                text = label.uppercase(),
                color = if (isPressed) activeColor else Color.White,
                fontSize = 12.sp,
                fontWeight = FontWeight.Black,
                textAlign = TextAlign.Center,
                modifier = Modifier.padding(horizontal = 4.dp)
            )
        }
    }
}
