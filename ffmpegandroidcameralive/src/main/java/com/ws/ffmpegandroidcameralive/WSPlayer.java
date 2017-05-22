package com.ws.ffmpegandroidcameralive;

/**
 * Created by Shuo.Wang on 2017/5/18.
 */

public  class WSPlayer {
    static {
        System.loadLibrary("native-lib");
    }

    public static native int initialize(int width,int height,String url);
    public static native int start(byte[] yuvimage);
    public static native int stop();
    public static native int close();
}
