package com.example.nathan.microsmartwatch;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

/**
 * MainActivity class
 * First activity of the program.
 * It only displays a button that starts an other activity.
 */
public class MainActivity extends Activity {

    private Button startDiscover;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        startDiscover = (Button) findViewById(R.id.startDiscoverActivity);
        startDiscover.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                /* Start an activity when the button is clicked */
                Intent intent = new Intent(MainActivity.this, DiscoverActivity.class);
                MainActivity.this.startActivity(intent);
            }
        });
    }


}
