package glacier.ctf.icyslide.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext

private val colorScheme = lightColorScheme(
    primary = Jet,
    onPrimary = Color.White, // Text/icon color when using the primary color
    primaryContainer = PowderBlue1, // Container for primary elements
    onPrimaryContainer = Jet, // Text/icon color on primary containers

    secondary = PowderBlue2, // Secondary color
    onSecondary = Color.Black, // Text/icon color when using the secondary color
    secondaryContainer = CadetGrey, // Container for secondary elements
    onSecondaryContainer = PowderBlue2, // Text/icon color on secondary containers

    tertiary = BrightOrange, // Accent color
    onTertiary = Color.White, // Text/icon color when using the tertiary color
    tertiaryContainer = LimeGreen, // Container for tertiary elements
    onTertiaryContainer = BrightOrange, // Text/icon color on tertiary containers

    background = BackgroundColor, // Main background color
    onBackground = Gray, // Text/icon color on the background

    surface = BackgroundColor, // Surface color for cards, sheets, etc.
    onSurface = DarkTeal, // Text/icon color on surface elements

    surfaceVariant = PowderBlue1, // Variant surface color (e.g., for elevated cards)
    onSurfaceVariant = Color.Black, // Text/icon color on surface variant elements

    outline = Gray, // Outline color for borders and dividers
    inverseOnSurface = Color.White, // Text/icon color on dark surfaces
    inversePrimary = BrightOrange, // Color to use for the primary color in inverse themes
)

@Composable
fun IcySlideTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    // Dynamic color is available on Android 12+
    dynamicColor: Boolean = true,
    content: @Composable () -> Unit
) {

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}