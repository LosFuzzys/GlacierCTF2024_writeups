from flask import Flask, request, jsonify
import json
import base64
import config as CONFIG
import os
from cryptography.fernet import Fernet, InvalidToken

app = Flask(__name__)

app.config.from_mapping(
    ENCRYPTION_KEY = b'ahYFfZqo7fs1QnCodTwaZM7hcZjY-v8jIuNjaTxzd3M=',
    API_KEY = "5233e94b-e63d-4a63-b8c3-7093104cf6ae",
    EXPECTED_SIGNATURE = "xuTyZbqmnvvi9dw5zSMXHa7WtWE=", #TODO change this
    EXPECTED_STRUCTURE_KEYS = {
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
)

def validate_signature(signature):
    # if signature != app.config['EXPECTED_SIGNATURE']:
    #     return False
    # return True
    return True

def validate_api_key(config_json):
    return config_json.get('server', {}).get('apiKey') == app.config['API_KEY']

# Function to check if the JSON structure is similar
def is_similar_structure(data, structure=app.config['EXPECTED_STRUCTURE_KEYS']):
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

@app.route('/upload', methods=['POST'])
def upload():
    try:
        # Check signature
        signature = request.headers.get('Signature')
        print("We check signature", flush=True)
        if not validate_signature(signature):
            return jsonify({'error': 'Unauthorized'}), 401
        print("Singature checked", flush = True)
        
        # Get config file
        config = request.files.get('config')
        if not config:
            return jsonify({'error': 'Config file missing'}), 400
        
        config_content = config.read()
        
        # Decrypt and parse config file
        try:
            config_content_decrypted = Fernet(app.config['ENCRYPTION_KEY']).decrypt(config_content)
            config_content_decrypted_and_decoded = config_content_decrypted.decode('utf-8')
            config_json = json.loads(config_content_decrypted_and_decoded)
        except (InvalidToken, ValueError, json.JSONDecodeError):
            return jsonify({'error': 'Invalid config format or decryption failed'}), 400
        
        # Validate API key
        if not validate_api_key(config_json):
            return jsonify({'error': 'Unauthorized'}), 401

        print("We passed the config check", flush=True)

        # Get and encrypt file content
        file = request.files.get('file')
        if not file:
            return jsonify({'error': 'File missing'}), 400

        file_content = file.read()
                
        try:
            file_json = json.loads(file_content)
            expected_path = "/data/data/glacier.ctf.icyslide/cache/receive_data"
            file_name_suggests_copying = expected_path not in file.filename
            config_similar = is_similar_structure(file_json)
            if file_name_suggests_copying and config_similar:
                return jsonify({'processedData': 'Please do not copy the config file!'}), 401
        except Exception as e:
            pass
    
        
        
        encrypted_file_content = Fernet(app.config['ENCRYPTION_KEY']).encrypt(file_content)
        encrypted_and_encoded_content = base64.b64encode(encrypted_file_content).decode('utf-8')
        
        return jsonify({'processedData': encrypted_and_encoded_content}), 200
    
    except Exception as e:
        # Catch-all for unexpected errors
        return jsonify({'error': 'An unexpected error occurred'}), 500


@app.route('/getContent', methods=['POST'])
def getContent():
    try:
        # Check signature
        signature = request.headers.get('Signature')
        if not validate_signature(signature):
            return jsonify({'error': 'Unauthorized'}), 401
        
        # Get config file
        config = request.files.get('config')
        if not config:
            return jsonify({'error': 'Config file missing'}), 400

        config_content = config.read()
        
        # Decrypt and parse config file
        try:
            config_content_decrypted = Fernet(app.config['ENCRYPTION_KEY']).decrypt(config_content)
            config_content_decrypted_and_decoded = config_content_decrypted.decode('utf-8')
            config_json = json.loads(config_content_decrypted_and_decoded)
        except (InvalidToken, ValueError, json.JSONDecodeError):
            return jsonify({'error': 'Invalid config format or decryption failed'}), 400
        
        # Validate API key
        if not validate_api_key(config_json):
            return jsonify({'error': 'Unauthorized'}), 401

        # Get and decrypt file content
        file = request.files.get('file')
        if not file:
            return jsonify({'error': 'File missing'}), 400

        file_content = file.read()
        
        try:
            file_content_decrypted = Fernet(app.config['ENCRYPTION_KEY']).decrypt(file_content)
        except InvalidToken:
            return jsonify({'error': 'File decryption failed'}), 400
        
        try:
            file_content_decrypted_and_decoded = file_content_decrypted.decode('utf-8')
            file_json = json.loads(file_content_decrypted_and_decoded)
            file_name_suggests_copying = file.filename != "/data/data/glacier.ctf.icyslide/config.cfg"
            config_similar = is_similar_structure(file_json)
            if  file_name_suggests_copying and config_similar:
                return jsonify({'processedData': 'Please do not copy the config file!'}), 401
        except Exception as e:
            pass

        file_content_decrypted_and_encoded = base64.b64encode(file_content_decrypted).decode('utf-8')
        
        return jsonify({'processedData': file_content_decrypted_and_encoded}), 200
    
    except Exception as e:
        # Catch-all for unexpected errors
        return jsonify({'error': 'An unexpected error occurred'}), 500


@app.route('/checkConfig', methods=['POST'])
def checkConfig():
    try:
        # Check signature
        signature = request.headers.get('Signature')
        if not validate_signature(signature):
            return jsonify({'error': 'Unauthorized'}), 401
        
        # Get config file
        config = request.files.get('config')
        if not config:
            return jsonify({'error': 'Config file missing'}), 400

        config_content = config.read()
        
        # Decrypt and parse config file
        try:
            config_content_decrypted = Fernet(app.config['ENCRYPTION_KEY']).decrypt(config_content)
            config_content_decrypted_and_decoded = config_content_decrypted.decode('utf-8')
            config_json = json.loads(config_content_decrypted_and_decoded)
        except (InvalidToken, ValueError, json.JSONDecodeError):
            return jsonify({'error': 'Invalid config format or decryption failed'}), 400
        
        # Validate API key and check permissions
        if not validate_api_key(config_json):
            return jsonify({'error': 'Unauthorized'}), 401

        if config_json.get('server', {}).get('canGetFlag') == True:
            with open('/flag.txt', 'r') as f:
                flag = f.read().encode()
                return jsonify({'processedData': base64.b64encode(flag).decode('utf-8')}), 200
        
        return jsonify({'error': 'Unauthorized'}), 401
    
    except Exception as e:
        print(e, flush=True)
        # Catch-all for unexpected errors
        return jsonify({'error': 'An unexpected error occurred'}), 500


if __name__ == '__main__':
    app.run(host='127.0.0.1', port=5000, debug=True)
