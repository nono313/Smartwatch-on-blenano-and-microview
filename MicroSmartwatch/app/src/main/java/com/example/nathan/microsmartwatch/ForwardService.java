package com.example.nathan.microsmartwatch;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

/**
 * ForwardService
 * The service transfers the content of a SMS from the SMSReceiver to the BluetoothLeService.
 */
public class ForwardService extends Service {
    private final String TAG = ForwardService.class.getSimpleName();

    private BluetoothLeService bleService;

    /** interface for clients that bind */
    private IBinder mBinder;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        Log.i(TAG, "ForwardService started");
        Bundle extras = intent.getExtras();

        if(extras != null) {
            final String message = (String) extras.get("Message");

            Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
            bindService(gattServiceIntent, new ServiceConnection() {
                @Override
                public void onServiceConnected(ComponentName name, IBinder service) {
                    Log.i(TAG, "Forward Service connected to BLEService");
                    bleService = ((BluetoothLeService.LocalBinder) service).getService();

                    if(bleService.sendToWatch(message)) {
                        Log.i(TAG, "Sent OK");
                    }
                    else {
                        Log.w(TAG, "Error while sending message !");
                    }
                }

                @Override
                public void onServiceDisconnected(ComponentName name) {

                }
            }, BIND_AUTO_CREATE);
        }

        stopSelf(); // Kill itself
        return 0;
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG, "Bound");
        return mBinder;
    }



}
