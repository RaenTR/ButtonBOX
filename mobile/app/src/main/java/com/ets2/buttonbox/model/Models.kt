package com.ets2.buttonbox.model

enum class ControlType {
    PUSH, TOGGLE, ROTARY
}

data class ButtonState(
    val id: String,
    val label: String,
    val type: ControlType,
    var currentValue: Int = 0,
    val maxValue: Int = 1
)
