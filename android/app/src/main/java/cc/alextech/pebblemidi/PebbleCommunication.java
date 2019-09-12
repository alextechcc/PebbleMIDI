package cc.alextech.pebblemidi;

import android.content.Context;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.UUID;

class PebbleCommunication {
    /**
     * Wrapper around PebbleKit to handle errors, send ack, and simplify listening
     */
    private PebbleDataReceiver dataReceiver;
    private PebbleListener pebbleListener;

    PebbleCommunication(UUID appuuid, PebbleListener pebbleListener) {
        this.dataReceiver = new PebbleDataReceiver(appuuid);
        this.pebbleListener = pebbleListener;
    }

    void pause(Context context) throws CannotUnregisterPebbleReceiver {
        try {
            context.unregisterReceiver(dataReceiver);
        } catch (IllegalArgumentException e) {
            throw new CannotUnregisterPebbleReceiver(
                    "Cannot unregister pebble receiver in context", e);
        }
    }

    void resume(Context context) {
        PebbleKit.registerReceivedDataHandler(context, dataReceiver);
    }

    private class PebbleDataReceiver extends PebbleKit.PebbleDataReceiver {
        protected PebbleDataReceiver(UUID subscribedUuid) { super(subscribedUuid); }

        @Override
        public void receiveData(Context context, int transactionId, PebbleDictionary data) {
            PebbleKit.sendAckToPebble(context, transactionId);
            pebbleListener.onPebbleData(data);
        }
    }

    class CannotUnregisterPebbleReceiver extends IllegalArgumentException {
        CannotUnregisterPebbleReceiver(String message, Throwable cause) {
            super(message, cause);
        }
    }
}
