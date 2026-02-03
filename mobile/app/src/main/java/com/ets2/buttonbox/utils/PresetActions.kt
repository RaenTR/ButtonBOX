package com.ets2.buttonbox.utils

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector

data class ActionInfo(
    val id: String,
    val label: String,
    val icon: ImageVector,
    val iconName: String,
    val color: Color,
    val scanCode: String,
    val category: String
)

object PresetActions {
    val CATEGORIES = listOf("S\u00dcR\u00dc\u015e", "I\u015eIKLAR", "KAB\u0130N & D\u0130\u011eER")

    val ACTIONS = listOf(
        // SÜRÜŞ
        ActionInfo("engine", "MOTOR START", Icons.Default.FlashOn, "motor", Color.Red, "18", "S\u00dcR\u00dc\u015e"),
        ActionInfo("parking_brake", "EL FREN\u0130", Icons.Default.StopCircle, "brake", Color(0xFFFF9100), "57", "S\u00dcR\u00dc\u015e"),
        ActionInfo("retarder_inc", "RETARDER +", Icons.Default.ArrowUpward, "up", Color.Cyan, "82", "S\u00dcR\u00dc\u015e"),
        ActionInfo("retarder_dec", "RETARDER -", Icons.Default.ArrowDownward, "down", Color.Cyan, "80", "S\u00dcR\u00dc\u015e"),
        ActionInfo("cruise_ctrl", "HIZ SAB\u0130T", Icons.Default.Timer, "settings", Color.Green, "19", "S\u00dcR\u00dc\u015e"),
        ActionInfo("differential", "DF. K\u0130L\u0130D\u0130", Icons.Default.SettingsInputComponent, "settings", Color.Gray, "33", "S\u00dcR\u00dc\u015e"),

        // IŞIKLAR
        ActionInfo("lights", "FARLAR", Icons.Default.Lightbulb, "light", Color.White, "76", "I\u015eIKLAR"),
        ActionInfo("high_beam", "UZUNLAR", Icons.Default.Highlight, "light", Color.Blue, "75", "I\u015eIKLAR"),
        ActionInfo("hazards", "D\u00d6RTL\u00dcLER", Icons.Default.Warning, "warning", Color.Red, "33", "I\u015eIKLAR"),
        ActionInfo("beacon", "TEPE LAMBASI", Icons.Default.AirportShuttle, "truck", Color.Yellow, "24", "I\u015eIKLAR"),
        ActionInfo("indicator_l", "SOL S\u0130NYAL", Icons.Default.ArrowBack, "down", Color.Green, "16", "I\u015eIKLAR"),
        ActionInfo("indicator_r", "SA\u011e S\u0130NYAL", Icons.Default.ArrowForward, "up", Color.Green, "17", "I\u015eIKLAR"),

        // KABİN & DİĞER
        ActionInfo("wipers", "S\u0130LECEKLER", Icons.Default.WaterDrop, "radio", Color.Cyan, "25", "KAB\u0130N & D\u0130\u011eER"),
        ActionInfo("horn", "KORNA", Icons.Default.Campaign, "radio", Color.Gray, "35", "KAB\u0130N & D\u0130\u011eER"),
        ActionInfo("air_horn", "HAVALI KORNA", Icons.Default.VolumeUp, "radio", Color.Red, "78", "KAB\u0130N & D\u0130\u011eER"),
        ActionInfo("trailer_attach", "DORSE BA\u011eLA", Icons.Default.AddCircle, "add", Color.Green, "84", "KAB\u0130N & D\u0130\u011eER"),
        ActionInfo("accept", "ONAYLA", Icons.Default.Check, "check", Color.Green, "28", "KAB\u0130N & D\u0130\u011eER"),
        ActionInfo("map", "HAR\u0130TA", Icons.Default.Map, "settings", Color.White, "50", "KAB\u0130N & D\u0130\u011eER"),
        ActionInfo("radio", "RADYO", Icons.Default.Radio, "radio", Color.Magenta, "82", "KAB\u0130N & D\u0130\u011eER")
    )
}
