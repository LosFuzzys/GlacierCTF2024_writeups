package glacier.ctf.icyslide

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.animation.core.CubicBezierEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material3.Button
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clipToBounds
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import glacier.ctf.icyslide.network.checkConfig
import glacier.ctf.icyslide.ui.theme.IcySlideTheme
import glacier.ctf.icyslide.utils.IcySlideUtils
import kotlinx.coroutines.runBlocking

class MainActivity : ComponentActivity() {

    init {
        System.loadLibrary("init-app")
    }

    private external fun initApp()

    override fun onCreate(savedInstanceState: Bundle?) {
       /*
       ..
       */

        setContent {
            IcySlideTheme {
                IcySlideApp(
                    onViewFilesClick = { navigateToViewFiles() },
                    onCreateFilesClick = { navigateToCreateFiles() },
                )
            }
        }
    }

    private fun navigateToViewFiles() {
        val intent = Intent(this, ViewFileActivity::class.java)
        startActivity(intent)
    }

    private fun navigateToCreateFiles() {
        val intent = Intent(this, CreateNewFileActivity::class.java)
        startActivity(intent)
    }
}

@Composable
fun IcySlideApp(onViewFilesClick: () -> Unit, onCreateFilesClick: () -> Unit) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(
                brush = Brush.verticalGradient(
                    colors = listOf(
                        MaterialTheme.colorScheme.surfaceVariant,
                        MaterialTheme.colorScheme.background,
                    )
                )
            ),
        contentAlignment = Alignment.Center
    ) {
        WelcomeScreen(onViewFilesClick = onViewFilesClick, onCreateFilesClick = onCreateFilesClick)
    }
}

@Composable
fun WelcomeScreen(onViewFilesClick: () -> Unit, onCreateFilesClick: () -> Unit) {
    var isDialogVisible by remember { mutableStateOf(false) }
    var uploadState by remember { mutableStateOf<UploadState?>(null) }
    val context = LocalContext.current

    val infiniteTransition_image = rememberInfiniteTransition()

    // Define the animation offsets
    val offsetX_new by infiniteTransition_image.animateFloat(
        initialValue = -370f,
//        initialValue = 0f,
        targetValue = 370f,
        animationSpec = infiniteRepeatable(
            animation = tween(
                durationMillis = 8000,
                easing = CubicBezierEasing(0.42f, 0f, 0.58f, 1f)
            ),
            repeatMode = RepeatMode.Reverse // Reverse to create the back and forth motion
        ), label = ""
    )

    Column(
        modifier = Modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Top
    ) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(400.dp)
                .clipToBounds()
        ) {

            Image(
                painter = painterResource(id = R.drawable.penguin),
                contentDescription = "Ice Slide",
                modifier = Modifier
                    .offset(x =offsetX_new.dp)
                    .wrapContentWidth(Alignment.CenterHorizontally, true),
            )

            //TODO multiple flags
            IconButton(
                onClick = {
                    isDialogVisible = true

                    // Update the upload status when the upload completes
                    val result = runBlocking { checkConfig(context) }
                    uploadState = if (result != null) {
                        UploadState.Success(result)
                    } else {
                        UploadState.Error("")
                    }

                },
                modifier = Modifier
                    .align(Alignment.BottomCenter) // Align to top-right
                    .offset(x = (500 + offsetX_new).dp, y= (-220).dp)
                    .padding(8.dp) // Padding from edge for positioning
                    .size(128.dp) // Adjust size for the icon as needed
            ) {
                Image(
                    painter = painterResource(id = R.drawable.flag),
                    contentDescription = "Flag Icon",
                    modifier = Modifier.graphicsLayer(
                        rotationZ = 15f
                    )
                )
            }

                Text(
                    text = "Welcome to IcySlide",
                    style = MaterialTheme.typography.displayLarge,
                    color = MaterialTheme.colorScheme.primary,
                    textAlign = TextAlign.Center,
                    modifier = Modifier
                        .align(Alignment.TopCenter)
                        .padding(16.dp)
                )


                Text(
                    text = "Securely store your notes by sliding them across to this app. Secure, simple, and fast storage for your little secrets.",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSecondary,
                    textAlign = TextAlign.Center,
                    modifier = Modifier
                        .padding(16.dp)
                        .align(Alignment.TopCenter)
                        .offset(y = 40.dp)
                )

        }

        Spacer(modifier = Modifier.height(40.dp))

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly
        ) {
            Button(
                onClick = { onViewFilesClick() },
                modifier = Modifier
                    .width(150.dp)
                    .height(50.dp),
                shape = RoundedCornerShape(8.dp)
            ) {
                Icon(
                    imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                    contentDescription = "Back"
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(text = "View Files", color = Color.White)
            }

            Button(
                onClick = { onCreateFilesClick() },
                modifier = Modifier
                    .width(150.dp)
                    .height(50.dp),
                shape = RoundedCornerShape(8.dp)
            ) {
                Text(text = "Create Files", color = Color.White)
                Spacer(modifier = Modifier.width(8.dp))
                Icon(
                    imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                    contentDescription = "Back"
                )
            }
        }


        Spacer(modifier = Modifier.height(40.dp))
        HorizontalDivider(
            modifier = Modifier.padding(vertical = 16.dp), // Add some vertical padding
            thickness = 1.dp, // Set the thickness of the line
            color = Color.Gray // Set the color of the line
        )

        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier = Modifier
                .padding(16.dp)
                .fillMaxWidth()
        ) {
            Text(
                text = "How it Works",
                style = MaterialTheme.typography.titleLarge,
//                color = MaterialTheme.colorScheme.primary,
            )

            Text(
                text = "This App only stores encrypted versions of your text documents. " +
                        "Therefore, even if a malicious application gains access to your files, " +
                        "it won't be able to read the data. " +
                        "Files are encrypted and decrypted on our special server. " +
                        "You can import text files from other applications, or if you actually want them " +
                        "to know your secret, share it with them. " +
                        "\n\n" +
                        "If you have any questions, feel free to ask us on discord! \n" +
                        "Other than that, I hope you enjoy our app!",
                textAlign = TextAlign.Center,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onBackground,
                modifier = Modifier.padding(top = 8.dp)
            )

        }
    }
    if (isDialogVisible) {
        UploadDialog(uploadState) { isDialogVisible = false }
    }
}


@Composable
fun UploadDialog(uploadState: UploadState?, onDismiss: () -> Unit) {
    Dialog(onDismissRequest = onDismiss) {
        val backgroundColor = when (uploadState) {
            is UploadState.Success -> MaterialTheme.colorScheme.tertiaryContainer
            else -> MaterialTheme.colorScheme.surfaceContainer// Default color while uploading
        }

        Box(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
                .background(backgroundColor.copy(alpha = 0.7f)), // Change background based on result
            contentAlignment = Alignment.TopCenter
        ) {
            Column(
                modifier = Modifier
                    .padding(40.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {

                when (uploadState) {
                    is UploadState.Success -> {
                        Text(
                            text = "Congratulations you managed to get the flag!",
                            style = MaterialTheme.typography.displayLarge,
                            textAlign = TextAlign.Center,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        HorizontalDivider(
                            thickness = 1.dp, // Set the thickness of the line
                            color = MaterialTheme.colorScheme.onSurfaceVariant // Set the color of the line
                        )
                        Spacer(modifier = Modifier.height(40.dp))
                        Text(
                            text = uploadState.message,
                            style = MaterialTheme.typography.bodyLarge,
                            textAlign = TextAlign.Center,
                            color = MaterialTheme.colorScheme.onTertiary
                        )
                    }

                    else -> {
                        Text(
                            text = "You found the flag button!",
                            style = MaterialTheme.typography.displayLarge,
                            textAlign = TextAlign.Center,
                            color = MaterialTheme.colorScheme.onSurface
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        HorizontalDivider(
                            thickness = 1.dp, // Set the thickness of the line
                            color = MaterialTheme.colorScheme.onSurfaceVariant // Set the color of the line
                        )
                        Spacer(modifier = Modifier.height(40.dp))
                        Text(
                            text = "But you are not authorized to access it...",
                            style = MaterialTheme.typography.bodyLarge,
                            textAlign = TextAlign.Center,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }


            }
        }
    }
}

sealed class UploadState(val message: String) {
    class Success(message: String) : UploadState(message)
    class Error(message: String) : UploadState(message)
}


@Preview(showBackground = true)
@Composable
fun PreviewIcySlideApp() {
    IcySlideTheme { IcySlideApp(onViewFilesClick = {}, onCreateFilesClick = {}) }
}
