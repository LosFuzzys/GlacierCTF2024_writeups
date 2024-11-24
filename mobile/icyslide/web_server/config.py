from cryptography.fernet import Fernet
import base64
import json
KEY = b'ahYFfZqo7fs1QnCodTwaZM7hcZjY-v8jIuNjaTxzd3M='
EXPECTED_API_KEY = "5233e94b-e63d-4a63-b8c3-7093104cf6ae"
EXPECTED_SIGNATURE = "xuTyZbqmnvvi9dw5zSMXHa7WtWE="
FLAG = base64.b64encode(b"flag{this_is_a_fake_flag}").decode('utf-8')
# Define a structure to check against, with required keys and their types
expected_structure_keys = {
    "app": {
        "name": str,
        "version": str,
        "developer": str
    },
    "encryption": {
        "algorithm": str,
        "keySize": str,
        "encryptionKey": str
    },
    "server": {
        "endpoints": {
            "encrypt": str,
            "decrypt": str
        },
        "timeout": str,
        "apiKey": str,
        "canGetFlag": bool
    },
    "storage": {
        "internalDirectory": str,
        "allowedFileTypes": [str]
    },
    "logging": {
        "logLevel": str,
        "logFilePath": str
    }
}

# Function to check if the JSON structure is similar
def is_similar_structure(data, structure=expected_structure_keys):
    if not isinstance(data, dict) or not isinstance(structure, dict):
        return False

    # Check if all top-level keys exist in the data
    for key in structure.keys():
        if key not in data:
            return False

        # Check if the types match for the main expected fields
        expected_value_type = structure[key]
        actual_value = data[key]

        if isinstance(expected_value_type, dict):
            if not is_similar_structure(actual_value, expected_value_type):
                return False
        elif isinstance(expected_value_type, list):
            if not isinstance(actual_value, list):
                return False
            # Check if the items in the list match the type of the first expected item
            if actual_value and not isinstance(actual_value[0], expected_value_type[0]):
                return False
        elif not isinstance(actual_value, expected_value_type):
            return False

    return True

# sample_config = b"""{
#   "app": {
#     "name": "IcySlide",
#     "version": "1.0.0",
#     "developer": "LetsFailNoob"
#   },
#   "encryption": {
#     "algorithm": "Fernet",
#     "keySize": "default",
#     "encryptionKey": "REDACTED"
#   },
#   "server": {
#     "endpoints": {
#       "encrypt": "upload",
#       "decrypt": "getData"
#     },
#     "timeout": "None",
#     "apiKey": "5233e94b-e63d-4a63-b8c3-7093104cf6ae",
#     "canGetFlag": false
#   },
#   "storage": {
#     "internalDirectory": "default",
#     "allowedFileTypes": [
#       "text/plain"
#     ]
#   },
#   "logging": {
#     "logLevel": "NONE",
#     "logFilePath": ""
#   }
# }
# """

# encrypted_file_content = Fernet(KEY).encrypt(sample_config)

# print("encrypted_file_content: ", encrypted_file_content)

# processed_result = base64.b64encode(encrypted_file_content).decode('utf-8')

# print("processed_result: ", processed_result)


    
# file_content = base64.b64decode(processed_result)

# print("file_content: ", file_content)

# file_content_decrypted = Fernet(KEY).decrypt(file_content).decode('utf-8')

# print("file_content_decrypted: ", file_content_decrypted)

# config = json.loads(file_content_decrypted)


# # securely check and error handle if the json contains the the server.canGetFlag field and then check the canGetFlag value
# if 'server' in config and 'canGetFlag' in config.get('server'):
#     if config.get('server').get('canGetFlag') == True:
#         print("Flag is enabled")
#     else:
#         print("Flag is disabled")

# print(Fernet(KEY).decrypt("gAAAAABnHny7sJI41iVO5QgQSwNhAc1ZXtgsYBAkUzV8HycNM63aGdSJcbOO4_dl0Q9LPJSSWOpLWnY3L5uOTWR214PLyhAynQ=="))