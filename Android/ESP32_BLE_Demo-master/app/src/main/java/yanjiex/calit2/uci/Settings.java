package yanjiex.calit2.uci;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.MenuItem;

import org.bluetooth.bledemo.R;

import static yanjiex.calit2.uci.MainActivity.EXTRAS_DEVICE_ADDRESS;
import static yanjiex.calit2.uci.MainActivity.EXTRAS_DEVICE_NAME;
import static yanjiex.calit2.uci.MainActivity.EXTRAS_DEVICE_RSSI;

/**
 * Created by yanjie on 2/13/18.
 */

public class Settings extends Activity {
    private String mDeviceName;
    private String mDeviceAddress;
    private String mDeviceRSSI;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_threshold);

        final Intent intent = getIntent();
        if (intent!=null){
            mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
            mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
            mDeviceRSSI = intent.getIntExtra(EXTRAS_DEVICE_RSSI, 0) + " db";
        }
        this.getActionBar().setDisplayHomeAsUpEnabled(true);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            onBackPressed();
        }

        return super.onOptionsItemSelected(item);
    }

}
