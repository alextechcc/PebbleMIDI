package cc.alextech.pebblemidi;

import android.media.midi.MidiDeviceInfo;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

import com.getpebble.android.kit.util.PebbleDictionary;
import com.mobileer.miditools.MidiConstants;

import java.util.UUID;

public class MainActivity extends AppCompatActivity implements PebbleListener {
    private static final String TAG = "PebbleMIDI";
    private static final UUID APPUUID = UUID.fromString("67d2f349-4de2-40a1-bc67-a232be54af4b");
    PebbleCommunication pebbleCom;
    MidiHandler midiHandler;
    PowerManager.WakeLock wakeLock;
    private final int MAX_PROGRAMS = 128;
    int program;
    Button listeningButton;
    private boolean listening;

    protected void onStartListening() {
        Log.i(TAG, "Starting");
        pebbleCom.resume(getApplicationContext());
        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "MyApp::MyWakelockTag");
        wakeLock.acquire();
    }

    protected void onStopListening() {
        Log.i(TAG, "Stopping");
        try {
            pebbleCom.pause(getApplicationContext());
        } catch (PebbleCommunication.CannotUnregisterPebbleReceiver e) {
            Log.e(TAG, "Cannot unregister receiver.");
        }
        wakeLock.release();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        pebbleCom = new PebbleCommunication(APPUUID, this);
        midiHandler = new MidiHandler(this, R.id.midi_device_spinner);
        program = 0;
        listeningButton = findViewById(R.id.listening_button);
        listening = false;
        listeningButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                listening = !listening;
                if (listening) {
                    listeningButton.setText(R.string.stop_listening);
                    onStartListening();
                } else {
                    listeningButton.setText(R.string.start_listening);
                    onStopListening();
                }
            }
        });
    }

    @Override
    public void onPebbleData(PebbleDictionary data) {
        final int BANK_UP = 1 << 1;
        final int BANK_DOWN = 1;

        int button_status = data.getUnsignedIntegerAsLong(0).intValue();
        if ((button_status & BANK_UP) == BANK_UP) {
            handleBankUp();
        }
        if ((button_status & BANK_DOWN) == BANK_DOWN) {
            handleBankDown();
        }
        handleAccel(new int[]{
                data.getUnsignedIntegerAsLong(1).intValue(),
                data.getUnsignedIntegerAsLong(2).intValue(),
                data.getUnsignedIntegerAsLong(3).intValue()});
    }

    private void handleBankUp() {
        Log.d(TAG, "Bank Up");
        /*
        program = (program + 1) % MAX_PROGRAMS;
        midiHandler.midiCommand(MidiConstants.STATUS_PROGRAM_CHANGE, program);
         */
        midiHandler.noteOn(1, 50, 127);
    }

    private void handleBankDown() {
        Log.d(TAG, "Bank Down");
        /*
        program = (program + MAX_PROGRAMS - 1) % MAX_PROGRAMS;
        midiHandler.midiCommand(MidiConstants.STATUS_PROGRAM_CHANGE, program);
        */
        midiHandler.noteOn(1, 58, 127);
    }

    private void handleAccel(int axis[]) {
        Log.d(TAG, String.format("x: %d, y: %d, z: %d\n", axis[0], axis[1], axis[2]));
        double mag = Math.sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
        if (mag > 220.0 / 1.4) {
            midiHandler.noteOn(1, 53, 127);
        }
        /*
        midiHandler.midiCommand(MidiConstants.STATUS_CONTROL_CHANGE + 1, 0x10, axis[0]);
        midiHandler.midiCommand(MidiConstants.STATUS_CONTROL_CHANGE + 1, 0x11, axis[1]);
        midiHandler.midiCommand(MidiConstants.STATUS_CONTROL_CHANGE + 1, 0x12, axis[2]);
         */
    }
}