import React from 'react';
import { StackNavigator } from 'react-navigation';

import BLE from '../BLE';
import DevicePg from '../DevicePg';

const Stack = StackNavigator({
	BLE: {
		screen: BLE,
		navigationOptions: {
			title: 'BLE Demo'
		}
	},
	DevicePg: {
		screen: DevicePg,
		navigationOptions: {
			title: 'Connected Device'
		}
	}
});

export default Stack;