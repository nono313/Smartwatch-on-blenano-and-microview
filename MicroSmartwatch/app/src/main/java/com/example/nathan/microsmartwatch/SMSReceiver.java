package com.example.nathan.microsmartwatch;

import android.bluetooth.BluetoothGattCharacteristic;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SmsMessage;
import android.util.Log;
import android.widget.Toast;

/**
 * Created by nathan on 20/12/15.
 */
public class SMSReceiver extends BroadcastReceiver {

    private final static String TAG = "SMSReceiver";
    public final static String ACTION = "android.provider.Telephony.SMS_RECEIVED";

    private BluetoothGattCharacteristic characteristic;
    private BluetoothLeService service;

    public SMSReceiver(BluetoothLeService service, BluetoothGattCharacteristic charact) {
        super();
        this.characteristic = charact;
        this.service = service;
        Log.d("RECEIVER", "Constructor");
    }
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("RECEIVER", "Received!");
        Toast t = Toast.makeText(context, "COUCOU", Toast.LENGTH_SHORT);
        t.show();

        Log.i(TAG, "Intent recieved: " + intent.getAction());

        if (intent.getAction().equals(ACTION)) {
            Bundle bundle = intent.getExtras();
            if (bundle != null) {
                Object[] pdus = (Object[])bundle.get("pdus");
                final SmsMessage[] messages = new SmsMessage[pdus.length];
                for (int i = 0; i < pdus.length; i++) {
                    messages[i] = SmsMessage.createFromPdu((byte[])pdus[i]);
                }
                if (messages.length > -1) {
                    Log.i(TAG, "Message recieved: " + messages[0].getMessageBody());
                    Intent intentForward = new Intent(context, ForwardService.class);
                    //intentForward.putExtra("BLEService", service);
                    Log.i(TAG, "Put extra : " + service.toString());
                    Log.i(TAG, "Put extra : " + messages[0].getMessageBody());
                    //intentForward.putExtra("Gatt", characteristic);
                    intentForward.putExtra("Message", messages[0].getMessageBody());
                    context.startService(intentForward);
                }
            }
        }
    }
}
