package com.ws.ffmpegandroidwallpaper;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;


public class MainActivity extends AppCompatActivity  {





    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button swBtn= (Button) findViewById(R.id.sw);
        swBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startWallpaper();
            }
        });

    }

    private void startWallpaper(){
        Toast.makeText(MainActivity.this,"startWallpaper",Toast.LENGTH_LONG).show();
        final Intent pickWallpaper = new Intent(Intent.ACTION_SET_WALLPAPER);
        Intent chooser = Intent.createChooser(pickWallpaper, getString(R.string.choose_wallpaper));
        startActivity(chooser);

    }



}

