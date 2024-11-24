package glacier.ctf.icyslide.network

import okhttp3.MultipartBody
import retrofit2.Call
import retrofit2.http.Header
import retrofit2.http.Multipart
import retrofit2.http.POST
import retrofit2.http.Part

// Define your API endpoints here
interface ApiService {

    @Multipart
    @POST("<endpoint1>")
    fun decrypt(
        @Header("Signature") signature: String,
        @Part file: MultipartBody.Part,
        @Part config: MultipartBody.Part
    ): Call<ResponseData>

    @Multipart
    @POST("<endpoint2>")
    fun encrypt(
        @Header("Signature") signature: String,
        @Part file: MultipartBody.Part,
        @Part config: MultipartBody.Part
    ): Call<ResponseData>

    @Multipart
    @POST("<endpoint3>")
    fun checkConfig(
        @Header("Signature") signature: String,
        @Part config: MultipartBody.Part
    ): Call<ResponseData>
}