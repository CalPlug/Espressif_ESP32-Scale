import React from 'react';
import { StackNavigator } from 'react-navigation';

import BLE from '../BLE';
import DevicePg from '../DevicePg';

export default const Stack = StackNavigator({
	BLE: {
		screen: BLE
	},
	DevicePg: {
		screen: DevicePg,
		navigationOptions: {
			title: 'Connected Device'
		}
	}
});