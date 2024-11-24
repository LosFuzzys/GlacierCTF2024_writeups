# Writeup
The intended solution is through absusing path traversal vulnerabilities. I am aware that it can also
be solved through reverse engineering, but since there are many angles on how to accomplish this
I will not go into detail on how such a approach could work.

## Goal
- Create a application `solution` to receive/send files from/to IcySlide
- Read the encrypted config.cfg file
- Create a new config file which enables flag reading
- Encrypt the config and overwrite the config.cfg
- Find flag button and win

## Arbitrary read of ecnrypted files
The expected directory from where to read/store files is the internal directory of the application.
This is the `/data/data/<package_name>/files/` folder, where `package_name` is `glacier.ctf.icyslide`.
The config location is `/data/data/<package_name>/config.cfg`. In order to read this file
one would create a file called `..%2Fconfig.cfg` and then share the file through the view file screen.
This creates a intent to select an app, while also granting this app access to the URI for the selected file.
The content provider isn't exported but allows to grantUriPermissions, which is done when sharing a file.

The URI will look like this: `content://glacier.ctf.icyslide.slideProvider/storage/..%2Fconfig.cfg`.
In the `solution` app, use the `openFileDescriptor` of the `contentResolver` class to read this file.
In the implementation of the `openFile` `IcySlide` retrieves the `lastPathSegment()` of the URI in order 
to find the file which should be opened. However, this method also decodes URL encoded characters,
resulting in the filename being `../config.cfg`. `IcySlide` then decrypts the file and sends it
to the requesting app.

## Config file structure
The structure of the config file is pretty basic, and we only need to change `canGetFlag` from false to true.
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

## Overwriting 
The `config.cfg` needs to be overwritten with the encrypted json. In order to do this
`IcySlide` allows for it to accept intents for files shares from other applications.
The activity `SlideShareTarget` is responsible for this. The process looks as follows:
- Retrieve the URI
- Open a file descriptor using the URI
- Query the filename using the URI
- Encrypt and store the file

However, when querying the filename it does not sanitze it. Therefore the `solution` app needs to
implement a malicious `query`, that returns a invalid name for the file. E.g.: `../config.cfg`.
This then overwrites the config.

*Note: Chaning the config in other ways does not matter, as long as the API key stays the same*

## Get flag
The flag button is moving with the image in the homescreen - once the config is overwritten correctly this button should show the flag.