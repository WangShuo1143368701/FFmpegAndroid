package com.ws.ffmpegandroidcameraencoder;

import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import java.io.IOException;


public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static final String TAG= "MainActivity";
    private Button mTakeButton;
    private Camera mCamera;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private boolean isRecording = false;

    private class StreamTask extends AsyncTask<Void, Void, Void> {

        private byte[] mData;

        //构造函数
        StreamTask(byte[] data){
            this.mData = data;
        }

        @Override
        protected Void doInBackground(Void... params) {
            // TODO Auto-generated method stub
            if(mData!=null){
                Log.i(TAG, "fps: " + mCamera.getParameters().getPreviewFrameRate());
                encode(mData);
            }

            return null;
        }
    }
    private StreamTask mStreamTask;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final Camera.PreviewCallback mPreviewCallbacx=new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] arg0, Camera arg1) {
                // TODO Auto-generated method stub
                if(null != mStreamTask){
                    switch(mStreamTask.getStatus()){
                        case RUNNING:
                            return;
                        case PENDING:
                            mStreamTask.cancel(false);
                            break;
                    }
                }
                mStreamTask = new StreamTask(arg0);
                mStreamTask.execute((Void)null);
            }
        };


        mTakeButton=(Button)findViewById(R.id.take_button);

        PackageManager pm=this.getPackageManager();
        boolean hasCamera=pm.hasSystemFeature(PackageManager.FEATURE_CAMERA) ||
                pm.hasSystemFeature(PackageManager.FEATURE_CAMERA_FRONT) ||
                Build.VERSION.SDK_INT<Build.VERSION_CODES.GINGERBREAD;
        if(!hasCamera)
            mTakeButton.setEnabled(false);

        mTakeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                if(mCamera!=null)
                {
                    if (isRecording) {
                        mTakeButton.setText("Start");
                        mCamera.setPreviewCallback(null);
                        Toast.makeText(MainActivity.this, "encode done", Toast.LENGTH_SHORT).show();
                        isRecording = false;
                    }else {
                        mTakeButton.setText("Stop");
                        initial(mCamera.getParameters().getPreviewSize().width,mCamera.getParameters().getPreviewSize().height);
                        mCamera.setPreviewCallback(mPreviewCallbacx);
                        isRecording = true;
                    }
                }
            }
        });


        mSurfaceView=(SurfaceView)findViewById(R.id.surfaceView1);
        SurfaceHolder holder=mSurfaceView.getHolder();
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        holder.addCallback(new SurfaceHolder.Callback() {

            @Override
            public void surfaceDestroyed(SurfaceHolder arg0) {
                // TODO Auto-generated method stub
                if(mCamera!=null)
                {
                    mCamera.stopPreview();
                    mSurfaceView = null;
                    mSurfaceHolder = null;
                }
            }

            @Override
            public void surfaceCreated(SurfaceHolder arg0) {
                // TODO Auto-generated method stub
                try{
                    if(mCamera!=null){
                        mCamera.setPreviewDisplay(arg0);
                        mSurfaceHolder=arg0;
                    }
                }catch(IOException exception){
                    Log.e(TAG, "Error setting up preview display", exception);
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
                // TODO Auto-generated method stub
                if(mCamera==null) return;
                Camera.Parameters parameters=mCamera.getParameters();
                parameters.setPreviewSize(640,480);
                parameters.setPictureSize(640,480);
                mCamera.setParameters(parameters);
                try{
                    mCamera.startPreview();
                    mSurfaceHolder=arg0;
                }catch(Exception e){
                    Log.e(TAG, "could not start preview", e);
                    mCamera.release();
                    mCamera=null;
                }
            }
        });

    }

    @TargetApi(9)
    @Override
    protected void onResume(){
        super.onResume();
        if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.GINGERBREAD){
            mCamera=Camera.open(0);
        }else
        {
            mCamera=Camera.open();
        }
    }

    @Override
    protected void onPause(){
        super.onPause();
        flush();
        close();
        if(mCamera!=null){
            mCamera.release();
            mCamera=null;
        }
    }




    //JNI
    public native int initial(int width,int height);
    public native int encode(byte[] yuvimage);
    public native int flush();
    public native int close();



}
