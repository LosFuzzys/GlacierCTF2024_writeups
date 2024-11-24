package solveapp.glacier.icyslide

import android.content.ComponentName
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import solveapp.glacier.icyslide.ui.theme.SolveAppTheme
import java.io.File

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        var text by mutableStateOf("Empty")
        var uri by mutableStateOf<Uri?>(null)

        // Assuming you have the URI of the file ready to share
        uri = Uri.parse("content://solveapp.glacier.icyslide/debug.txt")

        setContent {
            SolveAppTheme {
                // Create the UI with a button to share the file
                Column(modifier = Modifier.padding(16.dp)) {
                    Button(onClick = {
                        uri?.let { shareFile(it) } // Share the file URI when button is pressed
                    }) {
                        Text("Share File")
                    }
                }
            }
        }
    }

    private fun shareFile(uri: Uri) {
        val packageName = "glacier.ctf.icyslide"
        val activityName = "glacier.ctf.icyslide.externalaccess.SlideShareTarget"

        val shareIntent = Intent(Intent.ACTION_SEND).apply {
            type = "text/plain" // Adjust MIME type based on the file content
            putExtra(Intent.EXTRA_STREAM, uri) // Add the URI of the file
            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION) // Grant permission to read the URI
            component = ComponentName(packageName, activityName)
        }
        startActivity(Intent.createChooser(shareIntent, "Share file"))
    }
}

@Composable
fun SimpleText(text: String) {
    Text(
        text = text,
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    SolveAppTheme {
        SimpleText("Android")
    }
}
