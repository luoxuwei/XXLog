package com.luoxuwei.xxlog

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.luoxuwei.xxlog.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = XXLog().stringFromJNI()
        XXLog.open(XXLog.LEVEL_ALL, XXLog.COMPRESS_LEVEL6,"test/test", "test/test/test", "testtest", "testtesttest");
        XXLog.setConsoleLogOpen(true);
        XXLog.i("XXLog", "hello xxlog!!");
    }

}