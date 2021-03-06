package com.dede.jnidemo

import android.os.Bundle
import android.util.Log
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.dede.cmake.CMakeLib
import com.dede.ndk_build.NDKBuildLib

class MainActivity : AppCompatActivity() {

    companion object {
        private const val TAG = "MainActivity"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method
        findViewById<TextView>(R.id.sample_text).apply {
            text = CMakeLib.md5(text.toString())
        }

        Log.i(TAG, CMakeLib.staticCallJNI())
        Log.i(TAG, CMakeLib().memberCallJNI())
        Log.i(TAG, NDKBuildLib.staticCallJNI())
        Log.i(TAG, NDKBuildLib().memberCallJNI())
        Log.i(TAG, NDKBuildLib.dynamic())
        Log.i(TAG, NDKBuildLib().obj2String(this))
        Log.i(TAG, NDKBuildLib.add(1, 1).toString())
    }

}