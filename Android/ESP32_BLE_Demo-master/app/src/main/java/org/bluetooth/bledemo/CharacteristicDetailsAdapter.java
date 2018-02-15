//Richard(Yanjie) Xu modified for receiving data from ESP32  Jan 14, 2018
//University of California, Irvine, Calplug

package org.bluetooth.bledemo;

import java.util.Locale;
import android.bluetooth.BluetoothGattCharacteristic;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

public class CharacteristicDetailsAdapter extends BaseAdapter {

	private BluetoothGattCharacteristic mCharacteristic = null;
	private LayoutInflater mInflater;
	private BleWrapper mBleWrapper = null;
	private byte[] mRawValue = null;
	private int mIntValue = 0;
	private String mAsciiValue = "";
	private String mStrValue = "";
	private String mLastUpdateTime = "";
	private boolean mNotificationEnabled = false;
	private String tempHex = "";
	private float mFloatValue = 0.1f;

	public CharacteristicDetailsAdapter(PeripheralActivity parent, BleWrapper ble) {
		super();
		mBleWrapper = ble;
		mInflater = parent.getLayoutInflater();
	}

	public void setCharacteristic(BluetoothGattCharacteristic ch) {
		this.mCharacteristic = ch;
		mRawValue = null;
		mIntValue = 0;
		mAsciiValue = "";
		mStrValue = "";
		mLastUpdateTime = "-";
		mNotificationEnabled = false;
	}

	public BluetoothGattCharacteristic getCharacteristic(int index) {
		return mCharacteristic;
	}

	public void clearCharacteristic() {
		mCharacteristic = null;
	}

	@Override
	public int getCount() {
		return (mCharacteristic != null) ? 1 : 0;
	}

	@Override
	public Object getItem(int position) {
		return mCharacteristic;
	}

	@Override
	public long getItemId(int position) {
		return position;
	}


	String Tag = "IN FUNCTION TEXTING BYTES SETS";

	//Richard (Yanjie) Xu modified the code For Float transfer from ESP32 to Android
	public void newValueForCharacteristic(final BluetoothGattCharacteristic ch, final String strVal, final int intVal, final byte[] rawValue, final String timestamp) {
		if(!ch.equals(this.mCharacteristic)) return;

		mIntValue = intVal;
		mStrValue = strVal;
		mRawValue = rawValue;

		//Temp string to hold temp binary data
		String temp1;
		String temp2;
		String temp3;
		String temp0;
		String sbits;

		//mRawValue is the Hex String,  mFloatValue is the float transfered from BLE;
		if (mRawValue != null && mRawValue.length > 0) {

			mAsciiValue = "0x" + String.format("%02X",mRawValue[3])+String.format("%02X",mRawValue[2])+String.format("%02X",mRawValue[1])+String.format("%02X",mRawValue[0]);
			//Convert rawData to binary strings
			temp0 =("0000000" + Integer.toBinaryString(0xFF & mRawValue[3])).replaceAll(".*(.{8})$", "$1");
			temp1 = ("0000000" + Integer.toBinaryString(0xFF & mRawValue[2])).replaceAll(".*(.{8})$", "$1");
			temp2 = ("0000000" + Integer.toBinaryString(0xFF & mRawValue[1])).replaceAll(".*(.{8})$", "$1");
			temp3 = ("0000000" + Integer.toBinaryString(0xFF & mRawValue[0])).replaceAll(".*(.{8})$", "$1");
			sbits = temp0+temp1+temp2+temp3;

			//Check the positive float and negative float
			if (sbits.charAt(0)=='0'){
				int bits = Integer.valueOf(sbits,2);
				mFloatValue =Float.intBitsToFloat(bits);
			}else{
				sbits = sbits.substring(1);
				int bits = Integer.valueOf(sbits,2);
				mFloatValue =-Float.intBitsToFloat(bits);
			}
		}
		else mAsciiValue = "";

		mLastUpdateTime = timestamp;
		if(mLastUpdateTime == null) mLastUpdateTime = "";
	}

	public void setNotificationEnabledForService(final BluetoothGattCharacteristic ch) {
		if((!ch.equals(this.mCharacteristic)) || (mNotificationEnabled == true)) return;
		mNotificationEnabled = true;
		notifyDataSetChanged();
	}

	public byte[] parseHexStringToBytes(final String hex) {
		String tmp = hex.substring(2).replaceAll("[^[0-9][a-f]]", "");
		byte[] bytes = new byte[tmp.length() / 2]; // every two letters in the string are one byte finally

		String part = "";

		for(int i = 0; i < bytes.length; ++i) {
			part = "0x" + tmp.substring(i*2, i*2+2);
			bytes[i] = Long.decode(part).byteValue();
		}

		return bytes;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup p) {
		// get already available view or create new if necessary
		FieldReferences fields;
		if (convertView == null) {
			convertView = mInflater.inflate(R.layout.peripheral_details_characteristic_item, null);
			fields = new FieldReferences();
			fields.charPeripheralName = (TextView)convertView.findViewById(R.id.char_details_peripheral_name);
			fields.charPeripheralAddress = (TextView)convertView.findViewById(R.id.char_details_peripheral_address);
			fields.charServiceName = (TextView)convertView.findViewById(R.id.char_details_service);
			fields.charServiceUuid = (TextView)convertView.findViewById(R.id.char_details_service_uuid);
			fields.charName = (TextView)convertView.findViewById(R.id.char_details_name);
			fields.charUuid = (TextView)convertView.findViewById(R.id.char_details_uuid);

			//fields.charDataType = (TextView)convertView.findViewById(R.id.char_details_type);
			fields.charProperties = (TextView) convertView.findViewById(R.id.char_details_properties);

			//fields.charStrValue = (TextView) convertView.findViewById(R.id.char_details_ascii_value);
			fields.charDecValue = (TextView) convertView.findViewById(R.id.char_details_decimal_value);
			//fields.charHexValue = (EditText) convertView.findViewById(R.id.char_details_hex_value);
			fields.charDateValue = (TextView) convertView.findViewById(R.id.char_details_timestamp);

			fields.notificationBtn = (ToggleButton) convertView.findViewById(R.id.char_details_notification_switcher);
			fields.readBtn = (Button) convertView.findViewById(R.id.char_details_read_btn);
			fields.tareBtn = (Button) convertView.findViewById(R.id.char_details_tare_btn);
			fields.callibrateBtn = (Button) convertView.findViewById(R.id.char_details_calibrate_btn);
			fields.saveCalibrateBtn = (Button) convertView.findViewById(R.id.save_calibrateBtn);
			fields.zeroCalibrateBtn = (Button) convertView.findViewById(R.id.zero_calibrateBtn);
			fields.hundredCalibrateBtn= (Button) convertView.findViewById(R.id.hundred_calibrateBtn);
			//fields.writeBtn.setTag(fields.charHexValue);

			fields.saveCalibrateBtn.setOnClickListener(new View.OnClickListener(){
				@Override
				public void onClick(View v){
					String calibV = "0x26";
					byte[] callibrateData = parseHexStringToBytes(calibV);
					mBleWrapper.writeDataToCharacteristic(mCharacteristic, callibrateData);
				}
			});

			fields.tareBtn.setOnClickListener(new View.OnClickListener(){
				@Override
				public void onClick(View v){
					String calibV = "0x25";
                    byte[] callibrateData = parseHexStringToBytes(calibV);
					mBleWrapper.writeDataToCharacteristic(mCharacteristic, callibrateData);
				}
			});

			fields.zeroCalibrateBtn.setOnClickListener(new View.OnClickListener(){
				@Override
				public void onClick(View v){
					String calibV = "0x24";
					byte[] callibrateData = parseHexStringToBytes(calibV);
					mBleWrapper.writeDataToCharacteristic(mCharacteristic, callibrateData);
				}
			});

			fields.hundredCalibrateBtn.setOnClickListener(new View.OnClickListener(){
				@Override
				public void onClick(View v){
					String calibV = "0x23";
					byte[] callibrateData = parseHexStringToBytes(calibV);
					mBleWrapper.writeDataToCharacteristic(mCharacteristic, callibrateData);
				}
			});

			fields.readBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					mBleWrapper.requestCharacteristicValue(mCharacteristic);
				}
			});

        	/*fields.writeBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					//EditText hex = (EditText) v.getTag();
					//String newValue =  hex.getText().toString().toLowerCase(Locale.getDefault());
					//Log.d("EDIT TEXT String", newValue);
					String newValue = "0x25";
					byte[] dataToWrite = parseHexStringToBytes(newValue);

					mBleWrapper.writeDataToCharacteristic(mCharacteristic, dataToWrite);
				}
			});*/

			fields.notificationBtn.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					if(isChecked == mNotificationEnabled) return; // no need to update anything

					mBleWrapper.setNotificationForCharacteristic(mCharacteristic, isChecked);
					mNotificationEnabled = isChecked;
				}
			} );

			convertView.setTag(fields);
		} else {
			fields = (FieldReferences) convertView.getTag();
		}

		// set proper values into the view
		//fields.charPeripheralName.setText(mBleWrapper.getDevice().getName());
		fields.charPeripheralName.setText("BLE LIBRA");
		fields.charPeripheralAddress.setText(mBleWrapper.getDevice().getAddress());

		String tmp = mCharacteristic.getService().getUuid().toString().toLowerCase(Locale.getDefault());
		fields.charServiceUuid.setText(tmp);
		fields.charServiceName.setText(BleNamesResolver.resolveServiceName(tmp));

		String uuid = mCharacteristic.getUuid().toString().toLowerCase(Locale.getDefault());
		String name = BleNamesResolver.resolveCharacteristicName(uuid);
		name = "Weight in Grams";
		fields.charName.setText(name);
		fields.charUuid.setText(uuid);

		int format = mBleWrapper.getValueFormat(mCharacteristic);
		//fields.charDataType.setText(BleNamesResolver.resolveValueTypeDescription(format));
		int props = mCharacteristic.getProperties();
		String propertiesString = String.format("0x%04X [", props);
		if((props & BluetoothGattCharacteristic.PROPERTY_READ) != 0) propertiesString += "read ";
		if((props & BluetoothGattCharacteristic.PROPERTY_WRITE) != 0) propertiesString += "write ";
		if((props & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0) propertiesString += "notify ";
		if((props & BluetoothGattCharacteristic.PROPERTY_INDICATE) != 0) propertiesString += "indicate ";
		if((props & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) != 0) propertiesString += "write_no_response ";
		fields.charProperties.setText(propertiesString + "]");

		fields.notificationBtn.setEnabled((props & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0);
		fields.notificationBtn.setChecked(mNotificationEnabled);
		fields.readBtn.setEnabled((props & BluetoothGattCharacteristic.PROPERTY_READ) != 0);
		fields.tareBtn.setEnabled((props & (BluetoothGattCharacteristic.PROPERTY_WRITE | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)) != 0);
        fields.hundredCalibrateBtn.setEnabled((props & (BluetoothGattCharacteristic.PROPERTY_WRITE | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)) != 0);
        fields.zeroCalibrateBtn.setEnabled((props & (BluetoothGattCharacteristic.PROPERTY_WRITE | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)) != 0);
		fields.saveCalibrateBtn.setEnabled((props & (BluetoothGattCharacteristic.PROPERTY_WRITE | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)) != 0);
		fields.callibrateBtn.setEnabled(false);
        //fields.charHexValue.setEnabled(fields.writeBtn.isEnabled());

		//fields.charHexValue.setText(mAsciiValue);
		//fields.charStrValue.setText(mStrValue);

		//Do not delete these part
		if (mRawValue==null){
			fields.charDecValue.setText("");
		}else
			fields.charDecValue.setText(String.format("%f",mFloatValue)+" g");
		//end


		fields.charDateValue.setText(mLastUpdateTime);

		return convertView;
	}

	private class FieldReferences {
		TextView charPeripheralName;
		TextView charPeripheralAddress;
		TextView charServiceName;
		TextView charServiceUuid;
		TextView charUuid;
		TextView charName;
		//TextView charDataType;
		//TextView charStrValue;
		//EditText charHexValue;
		TextView charDecValue;
		TextView charDateValue;
		TextView charProperties;

		ToggleButton notificationBtn;
		Button readBtn;
		Button tareBtn;
		Button callibrateBtn;
		Button zeroCalibrateBtn;
		Button hundredCalibrateBtn;
		Button saveCalibrateBtn;
	}
}