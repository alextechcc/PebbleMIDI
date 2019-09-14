package cc.alextech.pebblemidi;

import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;

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
    private int[] oldAccel;

    protected void onStartListening() {
        Log.i(TAG, "Starting");
        pebbleCom.resume(getApplicationContext());
        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        assert powerManager != null;
        wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "MyApp::MyWakelockTag");
        wakeLock.acquire(60*60*1000L /*1 Hour*/);
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
        final int PROGRAM_UP = 1 << 1;
        final int PROGRAM_DOWN = 1;

        int button_status = data.getUnsignedIntegerAsLong(0).intValue();
        if ((button_status & PROGRAM_UP) == PROGRAM_UP) {
            handleProgramUp();
        }
        if ((button_status & PROGRAM_DOWN) == PROGRAM_DOWN) {
            handleProgramDown();
        }
        handleAccel(new int[]{
                data.getUnsignedIntegerAsLong(1).intValue(),
                data.getUnsignedIntegerAsLong(2).intValue(),
                data.getUnsignedIntegerAsLong(3).intValue()});
    }

    private void handleProgramUp() {
        Log.d(TAG, "Program Up");
        program = (program + 1) % MAX_PROGRAMS;
        midiHandler.midiCommand(MidiConstants.STATUS_PROGRAM_CHANGE, program);
        midiHandler.noteOn(1, 50, 127);
    }

    private void handleProgramDown() {
        Log.d(TAG, "Program Down");
        program = (program + MAX_PROGRAMS - 1) % MAX_PROGRAMS;
        midiHandler.midiCommand(MidiConstants.STATUS_PROGRAM_CHANGE, program);
    }

    private void handleAccel(int[] axis) {
        Log.d(TAG, String.format("Accel: x: %d, y: %d, z: %d\n", axis[0], axis[1], axis[2]));
        float alpha = ((ProgressBar) findViewById(R.id.seekBar)).getProgress() / 100f;
        int[] newAxis = new int[3];

        if (oldAccel == null) {
            oldAccel = new int[3];
            System.arraycopy(axis, 0, oldAccel, 0, 3);
        }

        for (int i = 0; i < 3; ++i) {
            newAxis[i] = Math.round((1-alpha) * axis[i] + alpha * oldAccel[i]);
            Log.d(TAG, String.format("Alpha: %f, axis[i]: %d, oldAccel[i]: %d, (1- alpha): %f", alpha, axis[i], oldAccel[i], 1-alpha));
        }

        midiHandler.midiCommand(MidiConstants.STATUS_CONTROL_CHANGE + 1, 0x10, newAxis[0]);
        midiHandler.midiCommand(MidiConstants.STATUS_CONTROL_CHANGE + 1, 0x11, newAxis[1]);
        midiHandler.midiCommand(MidiConstants.STATUS_CONTROL_CHANGE + 1, 0x12, newAxis[2]);

        System.arraycopy(newAxis, 0, oldAccel, 0, 3);
    }
}