package glacier.ctf.icyslide.externalaccess

import android.content.Intent
import android.database.Cursor
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import glacier.ctf.icyslide.network.encrypt
import glacier.ctf.icyslide.ui.theme.IcySlideTheme
import glacier.ctf.icyslide.utils.IcySlideUtils
import kotlinx.coroutines.runBlocking
import java.io.File
import java.io.IOException

class SlideShareTarget : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        handleIncomingIntent(intent)
    }

    private fun handleIncomingIntent(intent: Intent) {
        if (Intent.ACTION_SEND == intent.action && intent.type != null) {
            if ("text/plain" == intent.type) {
                intent.getParcelableExtra<Uri>(Intent.EXTRA_STREAM)?.let { uri ->
                    saveFileToInternalStorage(uri)
                } ?: run {
                    showError("No file URI provided.")
                }
            } else {
                showError("Unsupported file type.")
            }
        } else {
            showError("Invalid intent action.")
        }
    }

    private fun showError(message: String) {
        setContent {
            FileReceivedScreen(success = false, errorMessage = message)
        }
    }

    private fun saveFileToInternalStorage(uri: Uri) {
        try {

            contentResolver.openInputStream(uri)?.use { inputStream ->
                var fileName: String? = null
                val cursor: Cursor? = contentResolver.query(uri, null, null, null, null)

                cursor?.use {
                    if (it.moveToFirst()) {
                        val nameIndex = it.getColumnIndex(OpenableColumns.DISPLAY_NAME)
                        if (nameIndex != -1) {
                            fileName = it.getString(nameIndex)
                        }
                    }
                }

                if (fileName.isNullOrEmpty()) {
                    showError("Unable to retrieve file name.")
                    return
                }

                /*
                ..
                */

                val outputFile = File(filesDir, fileName)
                var data: ByteArray? = null
                if ("/data/data/glacier.ctf.icyslide/config.cfg".equals(outputFile.canonicalPath)) {
                    val tmpFile = File.createTempFile("receive_data", "tmp")
                    tmpFile.writeBytes(inputStream.readAllBytes())
                    data = runBlocking { encrypt(tmpFile, this@SlideShareTarget) }
                    tmpFile.delete()
                } else {
                    outputFile.writeBytes(inputStream.readAllBytes());
                    data = runBlocking { encrypt(outputFile, this@SlideShareTarget) }
                    if (data == null) {
                        outputFile.delete()
                    }
                }

                if (data != null)
                    outputFile.writeBytes(data)
                else {
                    showError("Received empty response from server!")
                    return
                }

                setContent {
                    FileReceivedScreen(success = true)
                }
            } ?: run {
                showError("Unable to open input stream.")
            }
        } catch (e: IOException) {
            e.printStackTrace()
            showError("Error saving file: ${e.message}")
        }
    }

    @Composable
    fun FileReceivedScreen(success: Boolean, errorMessage: String? = null) {
        IcySlideTheme {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp),
                contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = if (success) "File Saved Successfully!" else errorMessage
                            ?: "Error Saving File.",
                        fontSize = 24.sp
                    )
                    Spacer(modifier = Modifier.height(16.dp))
                    Button(onClick = { finish() }) {
                        Text(text = "Close")
                    }
                }
            }
        }
    }
}


