package glacier.ctf.icyslide.network

import androidx.annotation.Keep
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory

object NetworkModule {
//    private const val BASE_URL =
//        "https://icyslide.web.snowyglacier.pro/" // Replace with your actual base URL
    @Keep
    private const val BASE_URL = "https://icyslide.web.glacierctf.com/"
    private val retrofit = Retrofit.Builder()
        .baseUrl(BASE_URL)
        .addConverterFactory(GsonConverterFactory.create())
        .build()

    val apiService: ApiService = retrofit.create(ApiService::class.java)
}