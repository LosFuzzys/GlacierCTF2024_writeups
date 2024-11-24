package glacier.ctf.icyslide

import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material.icons.outlined.Info
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TextField
import androidx.compose.material3.TextFieldDefaults
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import glacier.ctf.icyslide.network.encrypt
import glacier.ctf.icyslide.ui.theme.IcySlideTheme
import glacier.ctf.icyslide.utils.IcySlideUtils
import kotlinx.coroutines.runBlocking
import java.io.File

class CreateNewFileActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        /*
       ..
       */
        setContent {
            IcySlideTheme {
                CreateNewFileScreen(
                    onSaveClicked = { filename, content ->
                        var file: File? = null
                        try {
                            file = File(filesDir, filename)
                            val canonicalPath = file.canonicalPath
                            require(canonicalPath.startsWith(filesDir.canonicalPath))
                            file.writeText(content)
                            val result =
                                runBlocking { encrypt(file, this@CreateNewFileActivity) }
                            if (result != null) {
                                file.writeBytes(result)
                                Toast.makeText(
                                    this@CreateNewFileActivity,
                                    "File '$filename' created successfully",
                                    Toast.LENGTH_LONG
                                ).show()
                            } else {
                                file.delete()
//                                Toast.makeText(
//                                    this@CreateNewFileActivity,
//                                    "File '$filename' could not be created, received no response from server",
//                                    Toast.LENGTH_LONG
//                                ).show()
                            }

                            true
                        } catch (e: IllegalArgumentException) {
                            file?.delete()
                            Toast.makeText(
                                this@CreateNewFileActivity,
                                "Please don't do that",
                                Toast.LENGTH_LONG
                            ).show()
                            false
                        }
                    },
                    onBackClicked = { finish() }
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CreateNewFileScreen(
    onSaveClicked: (String, String) -> Boolean,
    onBackClicked: () -> Unit
) {
    var filename by remember { mutableStateOf("") }
    var content by remember { mutableStateOf("") }
    var showHelpDialog by remember { mutableStateOf(false) }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Create New Text File",
                        color = MaterialTheme.colorScheme.onPrimary,
                        fontSize = 20.sp
                    )
                },
                navigationIcon = {
                    IconButton(onClick = onBackClicked) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = "Back"
                        )
                    }
                },
                actions = {
                    IconButton(onClick = { showHelpDialog = true }) {
                        Icon(
                            imageVector = Icons.Outlined.Info,
                            contentDescription = "Help",
                            tint = MaterialTheme.colorScheme.secondary,
                            modifier = Modifier.scale(1.1f)
                        )
                    }
                },
                colors = TopAppBarDefaults.centerAlignedTopAppBarColors(
                    containerColor = MaterialTheme.colorScheme.primary
                )
            )
        }
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues)
                .padding(16.dp)
                .background(MaterialTheme.colorScheme.background),
            verticalArrangement = Arrangement.Top,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {


            FileNameField(filename) { filename = it }
            Spacer(modifier = Modifier.height(16.dp))
            ContentField(content) { content = it }
            Spacer(modifier = Modifier.height(16.dp))
            SaveButton(onClick = {
                val wasSuccessful = onSaveClicked(filename, content)
                if (wasSuccessful) {
                    filename = ""
                    content = ""
                }
            })
            Spacer(modifier = Modifier.height(16.dp))

        }
        if (showHelpDialog) {
            HelpDialog(onDismiss = { showHelpDialog = false })
        }
    }
}

@Composable
fun HelpDialog(onDismiss: () -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { WarningNotice() },
        containerColor = Color(0xFFFFF3CD),
        confirmButton = {
            TextButton(onClick = onDismiss) {
                Text("Got it")
            }
        }
    )
}

@Composable
fun WarningNotice() {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp)
            .background(Color(0xFFFFF3CD)) // Light yellow background
            .padding(16.dp), // Padding inside the box
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            imageVector = Icons.Filled.Warning,
            contentDescription = "Warning",
            tint = MaterialTheme.colorScheme.scrim, // Warning color
            modifier = Modifier.size(24.dp) // Set size of the icon
        )
        Spacer(modifier = Modifier.width(8.dp))
        SelectionContainer {
            Text(
                text = "You can create text files here, however, you can share also share your text files with this app, if you already have some stored somewhere else.",
                color = Color(0xFF856404), // Dark brown color
                fontWeight = FontWeight.Bold, // Bold text for emphasis
                modifier = Modifier.weight(1f) // Take remaining space
            )
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun FileNameField(filename: String, onValueChange: (String) -> Unit) {
    Column(modifier = Modifier.fillMaxWidth()) {
        Text(
            text = "File Name",
            color = MaterialTheme.colorScheme.onSurface,
            fontSize = 16.sp,
            modifier = Modifier
                .align(Alignment.Start)
                .padding(bottom = 8.dp)
        )
        TextField(
            value = filename,
            onValueChange = onValueChange,
            modifier = Modifier.fillMaxWidth(),
            colors = TextFieldDefaults.textFieldColors(
                containerColor = MaterialTheme.colorScheme.surface,
                focusedIndicatorColor = MaterialTheme.colorScheme.primary,
                unfocusedIndicatorColor = Color.Gray,
                focusedTextColor = MaterialTheme.colorScheme.onSurface,
                cursorColor = MaterialTheme.colorScheme.primary
            )
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ContentField(content: String, onValueChange: (String) -> Unit) {
    Column(modifier = Modifier.fillMaxWidth()) {
        Text(
            text = "Content",
            color = MaterialTheme.colorScheme.onSurface,
            fontSize = 16.sp,
            modifier = Modifier
                .align(Alignment.Start)
                .padding(bottom = 8.dp)
        )
        TextField(
            value = content,
            onValueChange = onValueChange,
            modifier = Modifier
                .fillMaxWidth()
                .height(200.dp),
            colors = TextFieldDefaults.textFieldColors(
                containerColor = MaterialTheme.colorScheme.surface,
                focusedIndicatorColor = MaterialTheme.colorScheme.primary,
                unfocusedIndicatorColor = Color.Gray,
                focusedTextColor = MaterialTheme.colorScheme.onSurface,
                cursorColor = MaterialTheme.colorScheme.primary
            )
        )
    }
}

@Composable
fun SaveButton(onClick: () -> Unit) {
    Button(
        onClick = onClick,
        modifier = Modifier.fillMaxWidth(),
        colors = ButtonDefaults.buttonColors(containerColor = MaterialTheme.colorScheme.primary)
    ) {
        Text("Save File", color = MaterialTheme.colorScheme.onPrimary)
    }
}

@Preview(showBackground = true)
@Composable
fun PreviewCreateNewFile() {
    IcySlideTheme { CreateNewFileScreen(onSaveClicked = { _, _ -> true }, onBackClicked = {}) }
}
