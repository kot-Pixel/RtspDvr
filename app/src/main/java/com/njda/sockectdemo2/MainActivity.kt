package com.njda.sockectdemo2

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.njda.sockectdemo2.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        stringFromJNI()

        // Example of a call to a native method
        binding.sampleText.text = "1"
    }

    /**
     * A native method that is implemented by the 'sockectdemo2' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI()

    companion object {
        // Used to load the 'sockectdemo2' library on application startup.
        init {
            System.loadLibrary("SocketClientDemo")
        }
    }
}