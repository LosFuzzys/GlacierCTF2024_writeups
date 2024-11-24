#include <jni.h>  //For first API key JNIEXPORT jstring JNICAL

JNIEXPORT jstring JNICALL
Java_glacier_ctf_icyslide_network_APILib_getAPIKey(JNIEnv *env, jobject instance) {
    return (*env)->NewStringUTF(env, "5233e94b-e63d-4a63-b8c3-7093104cf6ae");
}


//#include <sys/ptrace.h>
//#include <unistd.h>
//
//int isDebuggerPresent() {
//    return ptrace(PTRACE_ATTACH, 0, 1, 0) == -1;
//}
//
//JNIEXPORT jstring JNICALL Java_com_myapplication_APILib_getAPIKey(JNIEnv *env, jobject instance) {
//    if (isDebuggerPresent()) {
//        return NULL; // or handle it accordingly
//    }
//    // Proceed to return the API key or generate it.
//}
//#include <jni.h>
//#include <string.h>
//
//// Define the expected signature (Base64 encoded)
//#define EXPECTED_SIGNATURE "your_expected_signature_in_base64" // Replace with your actual expected signature
//
//// JNI function to get an API key
//JNIEXPORT jstring JNICALL Java_com_myapplication_APILib_getAPIKey(JNIEnv *env, jobject instance, jstring signature) {
//    // Convert the jstring signature to a C string
//    const char *appSignature = (*env)->GetStringUTFChars(env, signature, 0);
//
//    // Compare the received signature with the expected one
//    if (strcmp(appSignature, EXPECTED_SIGNATURE) == 0) {
//        // Return valid API key if the signature matches
//        const char *apiKey = "5233e94b-e63d-4a63-b8c3-7093104cf6ae";
//        jstring result = (*env)->NewStringUTF(env, apiKey);
//
//        // Release the app signature string
//        (*env)->ReleaseStringUTFChars(env, signature, appSignature);
//        return result;
//    } else {
//        // Release the app signature string
//        (*env)->ReleaseStringUTFChars(env, signature, appSignature);
//        return NULL;  // Or return an empty string if the signature doesn't match
//    }
//}