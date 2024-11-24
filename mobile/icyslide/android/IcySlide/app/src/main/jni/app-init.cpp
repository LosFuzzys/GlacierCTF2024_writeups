#include <jni.h>
#include <fstream>
#include <string>

//TODO check if file exists
extern "C" JNIEXPORT void JNICALL
Java_glacier_ctf_icyslide_MainActivity_initApp(JNIEnv *env, jobject thiz) {
    const char *fileName = "/data/data/glacier.ctf.icyslide/config.cfg";

    std::ifstream checkFile(fileName);
    if (checkFile.is_open()) {
        checkFile.close();
        return;
    }
    
    std::ofstream file(fileName); // Open file for writing

    if (file.is_open()) {
        file
                << "gAAAAABnHnXsGmKVRV6GLttwKfeX4xgsi8wgwFmVTxmy9mjLmEQrmVyg3IcPlL_1Oeb5di-lRuGDHgT8XBmkrSyZ-TZashLiEpK4b8UkwKMsPbW9Pud-ngA0dzmuJTFhzb6Y1fYeK1rKJD73y6NMUPzfAZQrIDfOVnF8HBY0-ID5IB7Y6WqRPb-Mgik8FvrGqOQCtp_9siaa-PwFVjjLYmpn-m8QPKcdF4DCT9TCOAfSrpjvngOrfA-0v-nzK0-SRqF50hG_3KeR1a0MjFauPzaXK4YVRcMMhU7acrOJi_-KNdvz9XqLGKfMZSTQ5DZmtaP_jhgVLPoy8VrWi1-YYo-1jUaDtTWS96qnijJfNsK9UDoFnSOj-Kvuai8ZvW2U_bqHIZxSPDV_icZBdKujiThI8tbtleFykiQvUKDv9vgA28cx5z8VFRHRJr0u9-90_JUEZvTCKblVTVWdzn8G90PEbCKs_tDGKYR7UgOwh7xWjUKdaUUaG94jOi_yKplfBjIltiBzHX1TNByzaPOphuHgkTUlKcuHuePDurOuLbDPAHTTrnqoW5It0n5O_VdgjCI_4_dcRqHCNWGmYEnQcPvz4AkmTjCScfPDSC5PJ7RzVHVGRtscuhJ4Tumuu1c6WJLqujX_Wy8K0K1F-HEdRl9p_L7zGoWfnCJcnbOOo3uo20iSUV7tPdfbIebiz7EAtasCSBnlQ5anx2GawsAf0kBqiy4T4FDTMobPrSda63_4uxSRT6E--AjpWuCKSWJx0f_yJ7uBe6YghNpfpaPeoPe6SCtaFNYOhJKZDq6u-NQCqGYMOVGXPIFPvOBBHa6Lgt3RsXlt7DxK3X_7tPSeZRRtFQ59nc2LFw==";
        file.close(); // Close the file
    }
}