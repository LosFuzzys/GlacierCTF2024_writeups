import sys
import tarfile
import os
import shutil

if len(sys.argv) != 2:
    print("Usage: python check_for_malicious_code.py <filename>")
    sys.exit(1)

tar_file_path = sys.argv[1]


try:
    with tarfile.open(tar_file_path, 'r:gz') as tar:
        if not os.path.exists("/tmp/files_for_checking"):
            os.mkdir("/tmp/files_for_checking")
        tar.extractall("/tmp/files_for_checking")
    print("Successfully extracted the contents of the .tar file.")
    
    # malware check here
    # redacted for commercial purposes

    shutil.rmtree("/tmp/files_for_checking")


except Exception as e:
    print(f"Error extracting .tar file: {str(e)}")
    sys.exit(1)