package com.example.garyrendle.mis_cpp_test;

import android.content.Context;
import android.speech.tts.TextToSpeech;

import java.util.Locale;

public class TextToSpeechManager {

    private Context context;
    private TextToSpeech.OnInitListener listener;

    TextToSpeech engine;

    TextToSpeechManager(Context context, TextToSpeech.OnInitListener listener){
        this.context = context;
        this.listener = listener;
        engine = new TextToSpeech(this.context, this.listener);
        engine.setLanguage(Locale.UK);
    }

    // text to speech
    public void speechWelcome() {
        engine.speak("Initialization",
                TextToSpeech.QUEUE_FLUSH, null, null);
    }

    public void speechErrCalc() {
        engine.speak("No Speed Limit Detected",
                TextToSpeech.QUEUE_FLUSH, null, null);
    }

    public void speechErrJSON() {
        engine.speak("Error parsing Jason",
                TextToSpeech.QUEUE_FLUSH, null, null);
    }

    public void speech(double last_known_speed, int maxSpeed) {
        engine.speak("current" + String.format("%.1f", last_known_speed) + "maximum" + String.format("%d", maxSpeed),
                TextToSpeech.QUEUE_FLUSH, null, null);
    }

    public void destroy(){
        //Close the Text to Speech Library
        if(engine != null) {
            engine.stop();
            engine.shutdown();
        }
    }
}
