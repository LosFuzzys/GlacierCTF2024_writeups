package solveapp.glacier.icyslide

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import java.io.FileInputStream

class ReceiveURI : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        when {
            intent?.action == Intent.ACTION_SEND -> {
                if ("text/plain" == intent.type) {
                    println("we at least in here")
                    handleSendText(intent)/*{t -> text = t}*/ // Handle text being sent
                }
            }
            else -> {
                println("we got nothing")
            }
        }
    }

    private fun handleSendText(intent: Intent, /*updateText: (String) -> Unit*/) {
        println("Intent: $intent")
        intent.getParcelableExtra<Uri>(Intent.EXTRA_STREAM).let { uri ->
            println("Received URI: $uri")

            if (uri != null) {
                println("we in here")
                //TODO check with other methods
                contentResolver.openFileDescriptor(uri, "r").use { fd ->
                    println("We fuck up here?")

                    if (fd == null) {
                        println("shite")
                    }
                    val fileDescriptor = fd?.fileDescriptor

                    // Create a FileInputStream to read the content
                    FileInputStream(fileDescriptor).use { fileInputStream ->
                        val fileContent = fileInputStream.bufferedReader().use { it.readText() }
                        println("File content: $fileContent")

                        // Update the UI state with the file content or URI
//                        updateText("File content: $fileContent")

                        // Save the content to internal storage
//                        val fileName = "received_file.txt"
//                        val fileOutputStream = openFileOutput(fileName, MODE_PRIVATE)
//                        fileOutputStream.write(fileContent.toByteArray())
//                        fileOutputStream.close()

//                        println("File saved to internal storage as $fileName")
                    }
                }
            }
        }
    }
}