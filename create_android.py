import os, shutil
from PIL import Image

base_dir = r'c:\Users\Hieu-PC\Desktop\esp32led\android_app'
os.makedirs(os.path.join(base_dir, 'app', 'src', 'main', 'java', 'com', 'hieu', 'esp32led'), exist_ok=True)
os.makedirs(os.path.join(base_dir, 'app', 'src', 'main', 'assets'), exist_ok=True)
os.makedirs(os.path.join(base_dir, 'app', 'src', 'main', 'res', 'values'), exist_ok=True)

# 1. Project level build.gradle
with open(os.path.join(base_dir, 'build.gradle'), 'w', encoding='utf-8') as f:
    f.write('''buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath 'com.android.tools.build:gradle:8.2.2'
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
    }
}
''')

# 2. settings.gradle
with open(os.path.join(base_dir, 'settings.gradle'), 'w', encoding='utf-8') as f:
    f.write('''rootProject.name = "ESP32LED"
include ':app'
''')

# 3. gradle.properties
with open(os.path.join(base_dir, 'gradle.properties'), 'w', encoding='utf-8') as f:
    f.write('''org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8
android.useAndroidX=true
android.nonTransitiveRClass=true
''')

# 4. app/build.gradle
with open(os.path.join(base_dir, 'app', 'build.gradle'), 'w', encoding='utf-8') as f:
    f.write('''plugins {
    id 'com.android.application'
}

android {
    namespace 'com.hieu.esp32led'
    compileSdk 34

    defaultConfig {
        applicationId "com.hieu.esp32led"
        minSdk 24
        targetSdk 34
        versionCode 1
        versionName "1.0"
    }

    buildTypes {
        release {
            minifyEnabled false
        }
    }
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }
}

dependencies {
    implementation 'androidx.appcompat:appcompat:1.6.1'
    implementation 'com.google.android.material:material:1.11.0'
}
''')

# 5. AndroidManifest.xml
with open(os.path.join(base_dir, 'app', 'src', 'main', 'AndroidManifest.xml'), 'w', encoding='utf-8') as f:
    f.write('''<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android">

    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
    <uses-permission android:name="android.permission.ACCESS_WIFI_STATE" />
    <uses-permission android:name="android.permission.CAMERA" />
    <uses-feature android:name="android.hardware.camera" android:required="false" />
    <uses-feature android:name="android.hardware.camera.autofocus" android:required="false" />

    <application
        android:allowBackup="true"
        android:icon="@mipmap/ic_launcher"
        android:roundIcon="@mipmap/ic_launcher_round"
        android:label="@string/app_name"
        android:supportsRtl="true"
        android:theme="@style/Theme.AppCompat.Light.NoActionBar"
        android:usesCleartextTraffic="true">
        <activity
            android:name=".MainActivity"
            android:exported="true"
            android:configChanges="orientation|screenSize|keyboardHidden">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
''')

# 6. MainActivity.java
with open(os.path.join(base_dir, 'app', 'src', 'main', 'java', 'com', 'hieu', 'esp32led', 'MainActivity.java'), 'w', encoding='utf-8') as f:
    f.write('''package com.hieu.esp32led;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.webkit.PermissionRequest;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class MainActivity extends AppCompatActivity {
    private static final int CAMERA_PERMISSION_CODE = 101;
    private WebView webView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        webView = new WebView(this);
        setContentView(webView);

        WebSettings webSettings = webView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setDomStorageEnabled(true);
        webSettings.setAllowFileAccess(true);
        webSettings.setAllowContentAccess(true);
        webSettings.setMediaPlaybackRequiresUserGesture(false);
        webSettings.setDatabaseEnabled(true);

        webView.setWebViewClient(new WebViewClient());
        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public void onPermissionRequest(final PermissionRequest request) {
                runOnUiThread(() -> request.grant(request.getResources()));
            }
        });

        checkCameraPermissionAndLoad();
    }

    private void checkCameraPermissionAndLoad() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, CAMERA_PERMISSION_CODE);
        } else {
            loadApp();
        }
    }

    private void loadApp() {
        webView.loadUrl("file:///android_asset/index.html");
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == CAMERA_PERMISSION_CODE) {
            loadApp();
        }
    }

    @Override
    public void onBackPressed() {
        if (webView.canGoBack()) {
            webView.goBack();
        } else {
            super.onBackPressed();
        }
    }
}
''')

# 7. strings.xml
with open(os.path.join(base_dir, 'app', 'src', 'main', 'res', 'values', 'strings.xml'), 'w', encoding='utf-8') as f:
    f.write('''<resources>
    <string name="app_name">ESP32 LED</string>
</resources>
''')

# 8. Copy index.html into assets
shutil.copyfile(
    r'c:\Users\Hieu-PC\Desktop\esp32led\index.html',
    os.path.join(base_dir, 'app', 'src', 'main', 'assets', 'index.html')
)

# 9. Generate Android mipmap icons from Shin-chan image
src_img = r'C:\Users\Hieu-PC\.gemini\antigravity\brain\d1c3b03d-8061-4d13-9823-e2a2667b624b\.user_uploaded\media_1795265831485_94d83168.jpg'
im = Image.open(src_img)
w, h = im.size
box = (60, 0, 60 + h, h)
cropped = im.crop(box)

mipmap_sizes = {
    'mipmap-mdpi': 48,
    'mipmap-hdpi': 72,
    'mipmap-xhdpi': 96,
    'mipmap-xxhdpi': 144,
    'mipmap-xxxhdpi': 192,
}

for folder, sz in mipmap_sizes.items():
    folder_path = os.path.join(base_dir, 'app', 'src', 'main', 'res', folder)
    os.makedirs(folder_path, exist_ok=True)
    res = cropped.resize((sz, sz), Image.Resampling.LANCZOS)
    res.save(os.path.join(folder_path, 'ic_launcher.png'), 'PNG')
    res.save(os.path.join(folder_path, 'ic_launcher_round.png'), 'PNG')

# 10. Zip android_app into ESP32LED_Android_Project.zip
import zipfile
zip_path = r'c:\Users\Hieu-PC\Desktop\esp32led\ESP32LED_Android_Project.zip'
with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
    for root, dirs, files in os.walk(base_dir):
        for file in files:
            full_path = os.path.join(root, file)
            rel_path = os.path.relpath(full_path, r'c:\Users\Hieu-PC\Desktop\esp32led')
            zipf.write(full_path, rel_path)

print('Android Project & ESP32LED_Android_Project.zip created successfully!')
