package com.example.nathan.microsmartwatch;

import android.app.Activity;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Debug;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

/**
 * DiscoverActivity
 * The activity displays the list of BLE devices corresponding to the watch's name (BLE NOTIF).
 */
public class DiscoverActivity extends ListActivity{

    /* BLE management */
    private BluetoothAdapter mBluetoothAdapter;
    private boolean mScanning;
    private Handler mHandler;

    /* Filter scan to only show BLE NOTIF devices */
    private ScanFilter bleFilter;
    private ScanSettings settings;
    private List<ScanFilter> filters;
    private static final long SCAN_PERIOD = 10000;  // 10 seconds default scanning period

    /* UI Menu */
    private Menu mOptionsMenu;

    private LeDeviceListAdapter mLeDeviceListAdapter;

    private final static int REQUEST_ENABLE_BT = 1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mHandler = new Handler();

        /* Filter devices by name */
        bleFilter = new ScanFilter.Builder().setDeviceName("BLE NOTIF").build();
        filters = new ArrayList<>();
        filters.add(bleFilter);
        settings = new ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_POWER)
                .build();

        Log.i("" + this.getClass().getSimpleName(), "Discover activity");

        /* Initializes a Bluetooth adapter */
        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        /* Checks if Bluetooth is supported on the device. */
        if (mBluetoothAdapter == null) {
            Toast.makeText(this, R.string.error_bluetooth_not_supported, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        /* If the user chose not to enable Bluetooth */
        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onResume() {
        super.onResume();

        /*
         * Ensures Bluetooth is available on the device and it is enabled. If not,
         * displays a dialog requesting user permission to enable Bluetooth.
         */
        if (!mBluetoothAdapter.isEnabled()) {
            if (!mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
        }

        /* Setup list adapter */
        mLeDeviceListAdapter = new LeDeviceListAdapter();
        setListAdapter(mLeDeviceListAdapter);

        /* Start scanning if the adapter is enabled */
        if(mBluetoothAdapter.isEnabled()) {
            scanLeDevice(true);
        }

        /* For debug purposes, keep screen on */
        if (BuildConfig.DEBUG) {
            if (Debug.isDebuggerConnected()) {
                Log.d("SCREEN", "Keeping screen on for debugging, detach debugger and force an onResume to turn it off.");
                getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            } else {
                getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                Log.d("SCREEN", "Keeping screen on for debugging is now deactivated.");
            }
        }
    }

    /* Generate menu */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        mOptionsMenu = menu;
        if (!mScanning) {
            /* If the device is NOT scanning, we show a scan button */
            mOptionsMenu.findItem(R.id.menu_stop).setVisible(false);
            mOptionsMenu.findItem(R.id.menu_scan).setVisible(true);
            mOptionsMenu.findItem(R.id.menu_refresh).setActionView(null);
        } else {
            /* If the deice is currently scanning, we show a stop button and a progress icon */
            mOptionsMenu.findItem(R.id.menu_stop).setVisible(true);
            mOptionsMenu.findItem(R.id.menu_scan).setVisible(false);
            mOptionsMenu.findItem(R.id.menu_refresh).setActionView(
                    R.layout.actionbar_indeterminate_progress);
        }
        return true;
    }

    /**
     * Execute menu button's actions
     * @param item : the menu item clicked
     * @return true
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.menu_scan:
                mLeDeviceListAdapter.clear();
                scanLeDevice(true);
                break;
            case R.id.menu_stop:
                scanLeDevice(false);
                break;
        }
        return true;
    }

    /**
     * When an item of the device list is clicked
     * @param l : ListView of devices
     * @param v :  View containing the list vies
     * @param position : position of the item clicked
     * @param id : id of the item clicked
     */
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        /* Get device from its ID */
        final BluetoothDevice device = mLeDeviceListAdapter.getDevice(position);
        if (device == null) return;

        /* Setup an intent with device name and address as extra */
        final Intent intent = new Intent(this, DeviceControlActivity.class);
        intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_NAME, device.getName());
        intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_ADDRESS, device.getAddress());
        /* Stop scanning devices when a device is selected */
        if (mScanning) {
            scanLeDevice(false);
        }

        startActivity(intent);
    }

    /**
     * Start / stop BLE devices scanning
     * @param enable : boolean used to start or stop scanning
     */
    private void scanLeDevice(final boolean enable) {
        if (enable) {
            // Stops scanning after a pre-defined scan period.
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mScanning = false;
                    //mBluetoothAdapter.stopLeScan(mLeScanCallback);
                    mBluetoothAdapter.getBluetoothLeScanner().stopScan(mScanCallback);
                }
            }, SCAN_PERIOD);

            mScanning = true;
            mBluetoothAdapter.getBluetoothLeScanner().startScan(filters, settings, mScanCallback);
        } else {
            mScanning = false;
            mBluetoothAdapter.getBluetoothLeScanner().stopScan(mScanCallback);
        }
        invalidateOptionsMenu();    // Update menu items
    }

    /**
     * ScanCallback called when a device is found
     */
    private ScanCallback mScanCallback = new ScanCallback() {
        /**
         * When a device is detected, add it to the list adapter
         * @param callbackType : type of callback
         * @param result : result of the scan
         */
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            Log.i("callbackType", String.valueOf(callbackType));
            Log.i("result", result.toString());
            BluetoothDevice btDevice = result.getDevice();
            mLeDeviceListAdapter.addDevice(btDevice);
            mLeDeviceListAdapter.notifyDataSetChanged();
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            for (ScanResult sr : results) {
                Log.i("ScanResult - Results", sr.toString());
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.e("Scan Failed", "Error Code: " + errorCode);
        }
    };

    /**
     * Adapter for holding devices found through scanning.
     */
    private class LeDeviceListAdapter extends BaseAdapter {
        private ArrayList<BluetoothDevice> mLeDevices;
        private LayoutInflater mInflator;

        public LeDeviceListAdapter() {
            super();
            mLeDevices = new ArrayList<BluetoothDevice>();
            mInflator = DiscoverActivity.this.getLayoutInflater();
        }

        public void addDevice(BluetoothDevice device) {
            if(!mLeDevices.contains(device)) {
                mLeDevices.add(device);
            }
        }

        public BluetoothDevice getDevice(int position) {
            return mLeDevices.get(position);
        }

        public void clear() {
            mLeDevices.clear();
        }

        @Override
        public int getCount() {
            return mLeDevices.size();
        }

        @Override
        public Object getItem(int i) {
            return mLeDevices.get(i);
        }

        @Override
        public long getItemId(int i) {
            return i;
        }

        @Override
        public View getView(int i, View view, ViewGroup viewGroup) {
            ViewHolder viewHolder;
            // General ListView optimization code.
            if (view == null) {
                view = mInflator.inflate(R.layout.listitem_device, null);
                viewHolder = new ViewHolder();
                viewHolder.deviceAddress = (TextView) view.findViewById(R.id.device_address);
                viewHolder.deviceName = (TextView) view.findViewById(R.id.device_name);
                view.setTag(viewHolder);
            } else {
                viewHolder = (ViewHolder) view.getTag();
            }

            BluetoothDevice device = mLeDevices.get(i);
            final String deviceName = device.getName();
            if (deviceName != null && deviceName.length() > 0)
                viewHolder.deviceName.setText(deviceName);
            else
                viewHolder.deviceName.setText(R.string.unknown_device);
            viewHolder.deviceAddress.setText(device.getAddress());

            return view;
        }
    }

    static class ViewHolder {
        TextView deviceName;
        TextView deviceAddress;
    }

}
