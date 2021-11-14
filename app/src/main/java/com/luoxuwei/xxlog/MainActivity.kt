package com.luoxuwei.xxlog

import android.app.ActivityManager
import android.app.ActivityManager.RunningAppProcessInfo
import android.content.Context
import android.os.Bundle
import android.os.Environment
import android.os.Process
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.luoxuwei.xxlog.databinding.ActivityMainBinding
import java.io.File


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    var logCount = 0;

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
            // Example of a call to a native method
        binding.sampleText.text = XXLog().stringFromJNI()


        val externalFilesDir: File? = applicationContext.getExternalFilesDir(null)
        val externalFilesDirPath =
            if (externalFilesDir != null) externalFilesDir.getAbsolutePath() else "/sdcard"
        val logPath = externalFilesDirPath + "/xxlog"
        val cachePath = applicationContext.getDir("xxlog", Context.MODE_PRIVATE).absolutePath;
        val xlog = XXLog()
        Log.setLogImp(xlog)
        Log.setConsoleLogOpen(true)
        Log.appenderOpen(XXLog.LEVEL_DEBUG, XXLog.AppednerModeAsync, cachePath, logPath, "xxlog", 0
            , "a38ccf295934a442c2bf29a646d1d8953baa4be443d35bd594307b8719b36dcfc439e760b38837a8d0eb52ea38eb63d8c29d447a2994dc8fb41d5b813efda1cd");
        Log.i("xxlog", "test test test");
    }

    fun onClick(view: View) {
        Log.i("xxlog", "test test test " + logCount)
        logCount++
    }

}