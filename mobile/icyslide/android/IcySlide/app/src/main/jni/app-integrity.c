#include <jni.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>

JNIEXPORT jboolean JNICALL
Java_glacier_ctf_icyslide_utils_IcySlideUtils_init2(JNIEnv *env, jobject obj) {
    // Check for root indicators
    const char *su_paths[] = {"/system/bin/su", "/system/xbin/su", "/sbin/su"};
    for (int i = 0; i < sizeof(su_paths) / sizeof(su_paths[0]); i++) {
        if (access(su_paths[i], F_OK) == 0) {
            return JNI_FALSE;
        }
    }

    // Check for debugger with ptrace
//#ifdef __i386__
//    if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1) {
//        return JNI_FALSE;
//    }
//#endif

    return JNI_TRUE;
}

int checkFridaPorts() {
    FILE *f = fopen("/proc/net/tcp", "r");
    if (f == NULL) {
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // Check if the line contains Frida's default port in hexadecimal (6A6A and 6A6B)
        if (strstr(line, "00006A6A") || strstr(line, "00006A6B")) {
            fclose(f);
            return 1; // Frida detected
        }
    }

    fclose(f);
    return 0; // Frida not detected
}

// Function to check for Frida processes by scanning /proc
int checkFridaProcesses() {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip non-numeric entries (they aren't process directories)
        if (entry->d_type != DT_DIR || !isdigit(entry->d_name[0])) {
            continue;
        }

        // Construct the path to the cmdline file
        char cmdlinePath[256];
        snprintf(cmdlinePath, sizeof(cmdlinePath), "/proc/%s/cmdline", entry->d_name);

        FILE *cmdline = fopen(cmdlinePath, "r");
        if (cmdline) {
            char cmdlineContent[256];
            fread(cmdlineContent, sizeof(char), sizeof(cmdlineContent) - 1, cmdline);
            fclose(cmdline);

            // Null-terminate and check for Frida-related keywords
            cmdlineContent[sizeof(cmdlineContent) - 1] = '\0';
            if (strstr(cmdlineContent, "frida") || strstr(cmdlineContent, "gum") ||
                strstr(cmdlineContent, "gvfs")) {
                closedir(dir);
                return 1; // Frida detected
            }
        }
    }

    closedir(dir);
    return 0; // Frida not detected
}

// JNI function to call from Java
JNIEXPORT jboolean JNICALL
Java_glacier_ctf_icyslide_utils_IcySlideUtils_init1(JNIEnv *env, jobject obj) {
    if (checkFridaPorts() || checkFridaProcesses()) {
        return JNI_TRUE; // Frida detected
    }
    return JNI_FALSE; // Frida not detected
}

JNIEXPORT jboolean JNICALL
Java_glacier_ctf_icyslide_utils_IcySlideUtils_isSignatureValid(JNIEnv *env, jobject obj,
                                                               jstring hash) {
    const char *providedHash = (*env)->GetStringUTFChars(env, hash, 0);
    const char *knownHash = "";  // Replace with your known SHA-256 hash

    //This doesn't work anyway
    if (strcmp(providedHash, knownHash) != 0) {
        (*env)->ReleaseStringUTFChars(env, hash, providedHash);
//        return JNI_FALSE;
        return JNI_TRUE;
    }
    (*env)->ReleaseStringUTFChars(env, hash, providedHash);
    return JNI_TRUE;
}

