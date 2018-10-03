package com.example.garyrendle.mis_cpp_test;

import android.content.Context;
import android.speech.tts.TextToSpeech;

import java.util.Locale;

public class TextToSpeechManager {


    private TextToSpeech engine;

    TextToSpeechManager(Context context, TextToSpeech.OnInitListener listener){
        engine = new TextToSpeech(context, listener);
        engine.setLanguage(Locale.UK);
    }

    public void destroy(){
        //Close the Text to Speech Library
        if(engine != null) {
            engine.stop();
            engine.shutdown();
        }
    }
    public void speechSpeedExceeded() {
        engine.speak("Warning! speed limit exceeded",
                TextToSpeech.QUEUE_FLUSH, null, null);
    }
}
