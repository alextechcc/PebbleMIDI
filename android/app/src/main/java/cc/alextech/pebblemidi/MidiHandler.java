package cc.alextech.pebblemidi;


import android.app.Activity;
import android.content.Context;
import android.media.midi.MidiManager;
import android.media.midi.MidiReceiver;
import android.util.Log;

import com.mobileer.miditools.MidiConstants;
import com.mobileer.miditools.MidiInputPortSelector;

import java.io.IOException;

public class MidiHandler {
    private MidiInputPortSelector inputPortSelector;
    private final String TAG = "MidiHandler";
    private byte[] byteBuffer = new byte[3];

    MidiHandler(Activity activity, int resourceId) {
        MidiManager midiManager = (MidiManager) activity.getSystemService(Context.MIDI_SERVICE);
        this.inputPortSelector = new MidiInputPortSelector(midiManager,
                activity, resourceId);
    }

    public void noteOff(int channel, int pitch, int velocity) {
        midiCommand(MidiConstants.STATUS_NOTE_OFF + channel, pitch, velocity);
    }

    void noteOn(int channel, int pitch, int velocity) {
        midiCommand(MidiConstants.STATUS_NOTE_ON + channel, pitch, velocity);
    }

    void midiCommand(int status, int data1, int data2) {
        byteBuffer[0] = (byte) status;
        byteBuffer[1] = (byte) data1;
        byteBuffer[2] = (byte) data2;
        long now = System.nanoTime();
        midiSend(byteBuffer, 3, now);
    }

    void midiCommand(int status, int data1) {
        byteBuffer[0] = (byte) status;
        byteBuffer[1] = (byte) data1;
        long now = System.nanoTime();
        midiSend(byteBuffer, 2, now);
    }

    private void midiSend(byte[] buffer, int count, long timestamp) {
        if (inputPortSelector != null) {
            try {
                MidiReceiver receiver = inputPortSelector.getReceiver();
                if (receiver != null) {
                    receiver.send(buffer, 0, count, timestamp);
                }
            } catch (IOException e) {
                Log.e(TAG, "Sending Midi Command Failed " + e);
            }
        }
    }
}
