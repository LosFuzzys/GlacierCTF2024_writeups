# Writeup
The intended solution is through absusing path traversal vulnerabilities. I am aware that it can also
be solved through reverse engineering, but since there are many angles on how to accomplish this
I will not go into detail on how such a approach could work.

## App-Overview
The app contains of three different screens 
- MainActivity
(responsible for navigating and holding the flag button)
- CreateNewFileActiviy
(responsible for creating new files)
- ViewFileActivity
(responsible for viewing files and sharing with other apps)
- SlideShareTarget
(responsible for handling incoming share intents)

Additionally there is also a custom ContentProvider called `SlideProvider`, responsible for making
files accessible to other apps. It has the following attributes:
- **AUTHORITY:** `"glacier.ctf.icyslide.slideProvider"`
- **PATH:** `"storage"`
- Example valid **URI:** `content://glacier.ctf.icyslide.slideProvider/storage/<filename>`

The app also made some network calls to `encrypt/decrypt` files and to check the `config.cfg` file (in `NetworkUtils.checkConfig()`) 
when trying to access the flag. A request has the following information:

- **@Header:** `AppSignature` of type string (to check if the app was tampered with) 
- [encrypt/decrypt] the `file` to encrypt/decrypt
- `config:` The `config.cfg` file created at app start.

## Initial App inspection
Looking in the app, the `MainActivity` reveals, that the flag button calls the `checkConfig()` method,
and if the call to the responsible endpoint on the server is valid, returns the Base64 decoded result (i.e., the string).
With this information it might be interesting to check what the `config.cfg` file contains.

If one tries simply look at the `config.cfg` file through normal means (e.g., reverse egineering or adb), it is noticable
that it contains the following: 
```base64
gAAAAABnHnXsGmKVRV6GLttwKfeX4xgsi8wgwFmVTxmy9mjLmEQrmVyg3IcPlL_1Oeb5di-lRuGDHgT8XBmkrSyZ-TZashLiEpK4b8UkwKMsPbW9Pud-ngA0dzmuJTFhzb6Y1fYeK1rKJD73y6NMUPzfAZQrIDfOVnF8HBY0-ID5IB7Y6WqRPb-Mgik8FvrGqOQCtp_9siaa-PwFVjjLYmpn-m8QPKcdF4DCT9TCOAfSrpjvngOrfA-0v-nzK0-SRqF50hG_3KeR1a0MjFauPzaXK4YVRcMMhU7acrOJi_-KNdvz9XqLGKfMZSTQ5DZmtaP_jhgVLPoy8VrWi1-YYo-1jUaDtTWS96qnijJfNsK9UDoFnSOj-Kvuai8ZvW2U_bqHIZxSPDV_icZBdKujiThI8tbtleFykiQvUKDv9vgA28cx5z8VFRHRJr0u9-90_JUEZvTCKblVTVWdzn8G90PEbCKs_tDGKYR7UgOwh7xWjUKdaUUaG94jOi_yKplfBjIltiBzHX1TNByzaPOphuHgkTUlKcuHuePDurOuLbDPAHTTrnqoW5It0n5O_VdgjCI_4_dcRqHCNWGmYEnQcPvz4AkmTjCScfPDSC5PJ7RzVHVGRtscuhJ4Tumuu1c6WJLqujX_Wy8K0K1F-HEdRl9p_L7zGoWfnCJcnbOOo3uo20iSUV7tPdfbIebiz7EAtasCSBnlQ5anx2GawsAf0kBqiy4T4FDTMobPrSda63_4uxSRT6E--AjpWuCKSWJx0f_yJ7uBe6YghNpfpaPeoPe6SCtaFNYOhJKZDq6u-NQCqGYMOVGXPIFPvOBBHa6Lgt3RsXlt7DxK3X_7tPSeZRRtFQ59nc2LFw==
```
Decoding this with base64 will result in a string that looks like random garbage. So what do we do with that?
If it is not appearent cause of the theme of the app, one can also look at other files created within the app and notice the same pattern.

Therefore it could be interesting to decrypt it and check the contents of the file. How this will be done will be explained in the next section.

## Checking content of config.cfg 
The expected directory from where to read/store files is the internal directory of the application.
This is the `/data/data/glacier.ctf.icyslide/files/` folder, where `package_name` is `glacier.ctf.icyslide`.
Since the config location is `/data/data/glacier.ctf.icyslide/config.cfg` the file is not accessible directly in the `ViewFileActivity`. 
Another way to read files is to share them with another app. The process looks like this:

- Select the desired file in the ViewFileActivity
![test](/images/file_open.png)
- Click on `Send File` which does the following:
  - Creates a URI in the following form: `content://glacier.ctf.icyslide.slideProvider/storage/<selected-file>`
  - Create a Intent with action=`ACTION_SEND` and type `text/plain`
  - Opening a ShareSheet where one can select an app and grant the selected app temporary read permissions to that uri
  - ![test](/images/file_share.png)
- The receiving app can then use this URI to read the file with a `ContentResolver` that calls the `openFile()` of the ContentProvider responsible for the authority specified in the URI (i.e., the `SlideProvider`)
- The implementation of the `openFile()` method in the `SlideProvider` then decrypts the file and sends the unencrypted content in a ParcelFileDescriptor.
- *In my solution app the content is simply printed to the terminal.*

The above process is how files get shared on Android. Inspecting the `openFile` method more thoroughly, one can notice the following line:
```kt
val filename = uri.lastPathSegment ?: throw FileNotFoundException("Filename not provided in URI: $uri")
```
While this looks like it simply retrieves the lastPathSegment of the uri and should therefore be safe, it also decodes
the string. For more details on this look at this [Google FAQ](https://support.google.com/faqs/answer/7496913?hl=en).

In order to read the content of the config file (which is in the parent folder of the other files) one would do the following steps.

- Create a file called `..%2Fconfig.cfg` within `IcySlide` and then share the file through the view file screen to your app
![file_creation.ong](/images/file_creation.png)
  - The URI that gets shared will look like `content://glacier.ctf.icyslide.slideProvider/storage/..%2Fconfig.cfg`
- Use the `openFileDescriptor` of the `contentResolver` class to read this file (which calls the `openFile()` of the `SlideProvider`) and get the unencrypted content.
  - In the openFile the `..%2Fconfig` is the lastPathSegment - but as stated earlier, it will also get decoded - resulting in `../config` as the filename.


## Config file structure
The structure of the config file is pretty basic. 
```json
{
  "app": {
    "name": "IcySlide",
    "version": "1.0.0",
    "developer": "LetsFailNoob"
  },
  "encryption": {
    "algorithm": "Fernet",
    "keySize": "default",
    "encryptionKey": "REDACTED"
  },
  "server": {
    "endpoints": {
      "encrypt": "upload",
      "decrypt": "getData"
    },
    "timeout": "None",
    "apiKey": "5233e94b-e63d-4a63-b8c3-7093104cf6ae",
    "canGetFlag": false
  },
  "storage": {
    "internalDirectory": "default",
    "allowedFileTypes": [
      "text/plain"
    ]
  },
  "logging": {
    "logLevel": "NONE",
    "logFilePath": ""
  }
}
```
Looking at this, it should be evident that we need to change `canGetFlag` to `True`.
However, simply changing this json and pasting it into the `config.cfg` will not work,
as the file was usually encrypted and therefore also needs to be encrypted again.

## Overwriting 
After changing the json as just described, we need to encrypt the json and store it in `config.cfg`.
There are probably also other ways as the one described, I included another vulnerability in order
to be able to solve everything within the app and with a custom written solve app that has a custom
content provider.

`IcySlide` accepts intents for files shares from other applications.
The activity `SlideShareTarget` is responsible for handling URIs shared with the application.
The process looks as follows:
- Check if the URI is relevant (i.e., action=`ACTION_SEND` and type=`text/plain`, with a URI as ParcelabaleExtra). If yes store the file in the internal storage
  - Open a file descriptor using the URI
  - Query the filename using the URI (the filename is expected to be in the `DISPLAY_COLUMNS`)
  - Encrypt and store the file

Looking at the `saveFileToInternalStorage()` method, we can see that it uses the received file-name as is,
without doing any sanitization.

```kt
cursor?.use {
  if (it.moveToFirst()) {
      val nameIndex = it.getColumnIndex(OpenableColumns.DISPLAY_NAME)
      if (nameIndex != -1) {
          fileName = it.getString(nameIndex)
      }
  }
}
val outputFile = File(filesDir, fileName)
```
We can therefore implement a malicious `query`, that returns a invalid name for the file. E.g.: `../config.cfg`.
This then overwrites the config with the content of the shared file.

*Note: Changing the config in other ways does not matter, as long as the API key stays the same*

*My solution implemented a button that sends the intent with the URI directly to `IcySlide`*

## Get flag
The flag button is moving with the image in the homescreen - once the config is overwritten correctly this button should now show the flag.
![flag.png](/images/flag.png)