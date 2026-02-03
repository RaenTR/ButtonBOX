package com.ets2.buttonbox.utils

data class KeyInfo(val label: String, val scanCode: Int)

object ScanCodes {
    val ROW1 = listOf(
        KeyInfo("ESC", 1), KeyInfo("F1", 59), KeyInfo("F2", 60), KeyInfo("F3", 61), KeyInfo("F4", 62),
        KeyInfo("F5", 63), KeyInfo("F6", 64), KeyInfo("F7", 65), KeyInfo("F8", 66),
        KeyInfo("F9", 67), KeyInfo("F10", 68), KeyInfo("F11", 87), KeyInfo("F12", 88)
    )

    val ROW2 = listOf(
        KeyInfo("é", 41), KeyInfo("1", 2), KeyInfo("2", 3), KeyInfo("3", 4), KeyInfo("4", 5),
        KeyInfo("5", 6), KeyInfo("6", 7), KeyInfo("7", 8), KeyInfo("8", 9), KeyInfo("9", 10),
        KeyInfo("0", 11), KeyInfo("-", 12), KeyInfo("=", 13), KeyInfo("BS", 14)
    )

    val ROW3 = listOf(
        KeyInfo("TAB", 15), KeyInfo("Q", 16), KeyInfo("W", 17), KeyInfo("E", 18), KeyInfo("R", 19),
        KeyInfo("T", 20), KeyInfo("Y", 21), KeyInfo("U", 22), KeyInfo("I", 23), KeyInfo("O", 24),
        KeyInfo("P", 25), KeyInfo("[", 26), KeyInfo("]", 27), KeyInfo("ENT", 28)
    )

    val ROW4 = listOf(
        KeyInfo("CAPS", 58), KeyInfo("A", 30), KeyInfo("S", 31), KeyInfo("D", 32), KeyInfo("F", 33),
        KeyInfo("G", 34), KeyInfo("H", 35), KeyInfo("J", 36), KeyInfo("K", 37), KeyInfo("L", 38),
        KeyInfo(";", 39), KeyInfo("'", 40), KeyInfo("\\", 43)
    )

    val ROW5 = listOf(
        KeyInfo("LSHFT", 42), KeyInfo("Z", 44), KeyInfo("X", 45), KeyInfo("C", 46), KeyInfo("V", 47),
        KeyInfo("B", 48), KeyInfo("N", 49), KeyInfo("M", 50), KeyInfo(",", 51), KeyInfo(".", 52),
        KeyInfo("/", 53), KeyInfo("RSHFT", 54)
    )

    val ROW6 = listOf(
        KeyInfo("LCTRL", 29), KeyInfo("WIN", 219), KeyInfo("LALT", 56), KeyInfo("SPACE", 57),
        KeyInfo("RALT", 184), KeyInfo("RCTRL", 157)
    )

    val NAVIGATION = listOf(
        KeyInfo("PRT", 183), KeyInfo("SCL", 70), KeyInfo("PAU", 197),
        KeyInfo("INS", 210), KeyInfo("HM", 199), KeyInfo("PU", 201),
        KeyInfo("DEL", 211), KeyInfo("END", 207), KeyInfo("PD", 209)
    )

    val ARROWS = listOf(
        KeyInfo("↑", 200), KeyInfo("←", 203), KeyInfo("↓", 208), KeyInfo("→", 205)
    )

    val NUMPAD = listOf(
        KeyInfo("NL", 69), KeyInfo("NP/", 181), KeyInfo("NP*", 55), KeyInfo("NP-", 74),
        KeyInfo("NP7", 71), KeyInfo("NP8", 72), KeyInfo("NP9", 73), KeyInfo("NP+", 78),
        KeyInfo("NP4", 75), KeyInfo("NP5", 76), KeyInfo("NP6", 77),
        KeyInfo("NP1", 79), KeyInfo("NP2", 80), KeyInfo("NP3", 81), KeyInfo("NPE", 156),
        KeyInfo("NP0", 82), KeyInfo("NP.", 83)
    )
}
