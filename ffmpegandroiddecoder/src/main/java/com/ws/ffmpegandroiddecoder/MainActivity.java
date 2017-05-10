package com.ws.ffmpegandroiddecoder;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private Button codec;
    private TextView tv_info;

    private EditText editText1,editText2;
    private Button btnDecoder;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        init();
    }

    private void init() {

        codec = (Button) findViewById(R.id.btn_codec);
        tv_info= (TextView) findViewById(R.id.tv_info);

        editText1 = (EditText) findViewById(R.id.editText1);
        editText2 = (EditText) findViewById(R.id.editText2);
        btnDecoder= (Button) findViewById(R.id.button);


        codec.setOnClickListener(this);
        btnDecoder.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {

            case R.id.btn_codec:
                tv_info.setText(avcodecinfo());
                break;

            case R.id.button:
               startDecode();
                break;

            default:
                break;
        }
    }

    private void startDecode() {

        String folderurl= Environment.getExternalStorageDirectory().getPath();

        String inputurl=folderurl+"/"+editText1.getText().toString();

        String outputurl=folderurl+"/"+editText2.getText().toString();

        Log.e("ws-----------inputurl",inputurl);
        Log.e("ws------------outputurl",outputurl);

        decode(inputurl,outputurl);

    }


    public native int decode(String inputurl, String outputurl);


    public native String avcodecinfo();



}

