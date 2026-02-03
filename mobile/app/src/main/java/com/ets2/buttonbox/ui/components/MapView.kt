package com.ets2.buttonbox.ui.components

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.StrokeJoin
import androidx.compose.ui.graphics.drawscope.rotate
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.animation.core.*
import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin

@Composable
fun MapView(
    posX: Float,
    posZ: Float,
    heading: Float,
    modifier: Modifier = Modifier
) {
    // Breadcrumbs
    val pathPoints = remember { mutableStateListOf<Offset>() }
    
    // Pulse animation (GPS Scanning Effect)
    val infiniteTransition = rememberInfiniteTransition(label = "pulse")
    val sweepAngle by infiniteTransition.animateFloat(
        initialValue = 0f,
        targetValue = 360f,
        animationSpec = infiniteRepeatable(
            animation = tween(4000, easing = LinearEasing),
            repeatMode = RepeatMode.Restart
        ),
        label = "sweep"
    )

    LaunchedEffect(posX, posZ) {
        val newPoint = Offset(posX, posZ)
        if (pathPoints.isEmpty() || (pathPoints.last() - newPoint).getDistance() > 5f) {
            pathPoints.add(newPoint)
            if (pathPoints.size > 150) pathPoints.removeAt(0)
        }
    }

    Box(
        modifier = modifier
            .background(Color(0xFF0A100A), RoundedCornerShape(12.dp)) // F3 Dark Green approach
            .border(2.dp, Color(0xFF1B3D1B), RoundedCornerShape(12.dp))
            .padding(2.dp),
        contentAlignment = Alignment.Center
    ) {
        Canvas(modifier = Modifier.fillMaxSize()) {
            val center = Offset(size.width / 2, size.height / 2)
            val scale = 0.6f 

            // --- F3 GPS BACKGROUND SCAN ---
            drawArc(
                color = Color(0xFF1B3D1B).copy(alpha = 0.3f),
                startAngle = sweepAngle - 30f,
                sweepAngle = 60f,
                useCenter = true,
                size = size / 0.8f,
                topLeft = Offset(center.x - (size.width / 1.6f), center.y - (size.height / 1.6f))
            )

            // Tactical Grid
            val gridStep = size.width / 4
            for (i in -4..4) {
                val alpha = if (i == 0) 0.3f else 0.1f
                drawLine(
                    Color(0xFF4CAF50).copy(alpha = alpha), 
                    Offset(0f, center.y + i * gridStep), 
                    Offset(size.width, center.y + i * gridStep),
                    strokeWidth = 1f
                )
                drawLine(
                    Color(0xFF4CAF50).copy(alpha = alpha), 
                    Offset(center.x + i * gridStep, 0f), 
                    Offset(center.x + i * gridStep, size.height),
                    strokeWidth = 1f
                )
            }

            // Radar Circles
            drawCircle(Color(0xFF4CAF50).copy(alpha = 0.15f), radius = size.width / 3, center = center, style = androidx.compose.ui.graphics.drawscope.Stroke(1f))
            drawCircle(Color(0xFF4CAF50).copy(alpha = 0.15f), radius = size.width / 1.5f, center = center, style = androidx.compose.ui.graphics.drawscope.Stroke(1f))

            // Breadcrumbs Path (The Trail)
            if (pathPoints.size > 1) {
                val drawPath = Path()
                pathPoints.forEachIndexed { index, point ->
                    val relX = center.x + (point.x - posX) * scale
                    val relY = center.y + (point.y - posZ) * scale
                    
                    if (relX in 0f..size.width && relY in 0f..size.height) {
                        if (drawPath.isEmpty) drawPath.moveTo(relX, relY) else drawPath.lineTo(relX, relY)
                    }
                }
                drawPath(
                    drawPath, 
                    Color(0xFF81C784).copy(alpha = 0.6f), 
                    style = androidx.compose.ui.graphics.drawscope.Stroke(width = 4f, cap = StrokeCap.Round, join = StrokeJoin.Round)
                )
            }

            // Center Mark (The Truck)
            rotate(degrees = heading * 360f, pivot = center) {
                // Truck directional arrow (F3 Style)
                val arrowPath = Path().apply {
                    moveTo(center.x, center.y - 14f)
                    lineTo(center.x - 10f, center.y + 12f)
                    lineTo(center.x, center.y + 6f)
                    lineTo(center.x + 10f, center.y + 12f)
                    close()
                }
                // Glow effect
                drawPath(arrowPath, Color(0xFF4CAF50).copy(alpha = 0.4f), style = androidx.compose.ui.graphics.drawscope.Stroke(6f))
                drawPath(arrowPath, Color(0xFFC8E6C9))
            }
        }
        
        // Corner Indicators
        Box(modifier = Modifier.fillMaxSize().padding(8.dp)) {
            Text("AUTO-CENTER", color = Color(0xFF4CAF50).copy(alpha = 0.8f), fontSize = 6.sp, fontWeight = FontWeight.Bold, modifier = Modifier.align(Alignment.TopEnd))
            Text("ZOOM 1.5x", color = Color(0xFF4CAF50).copy(alpha = 0.8f), fontSize = 6.sp, fontWeight = FontWeight.Bold, modifier = Modifier.align(Alignment.BottomStart))
        }
        
        Text(
            "SCN-TELEMETRY GPS", 
            color = Color(0xFF43A047), 
            fontSize = 8.sp, 
            fontWeight = FontWeight.ExtraBold,
            modifier = Modifier.align(Alignment.BottomCenter).padding(bottom = 4.dp)
        )
    }
}
