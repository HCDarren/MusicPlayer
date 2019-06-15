package com.darren.media;

import android.text.TextUtils;

/**
 * Created by hcDarren on 2019/6/15.
 * 音频播放器的逻辑处理类
 */
public class DarrenPlayer {
    static {
        System.loadLibrary("music-player");
    }

    /**
     * url 可以是本地文件路径，也可以是 http 链接
     */
    private String url;

    public void setDataSource(String url){
        this.url = url;
    }

    public void play(){
        if(TextUtils.isEmpty(url)){
            throw new NullPointerException("url is null, please call method setDataSource");
        }

        nPlay(url);
    }

    private native void nPlay(String url);
}
