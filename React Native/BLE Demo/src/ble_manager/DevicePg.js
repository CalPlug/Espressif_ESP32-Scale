import React, { Component } from 'react';
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  TouchableHighlight,
  TouchableOpacity,
  NativeAppEventEmitter,
  NativeEventEmitter,
  NativeModules,
  Platform,
  PermissionsAndroid,
  ListView,
  ScrollView,
  AppState,
  Dimensions,
  ToastAndroid,
  Modal,
  Alert
} from 'react-native';
import BleManager from 'react-native-ble-manager';

const BleManagerModule = NativeModules.BleManager;
const bleManagerEmitter = new NativeEventEmitter(BleManagerModule);

export default class DevicePg extends Component {
  constructor(props){
    super(props);
    this.state = {
      peripherals: this.props.navigation.state.params.peripherals,
      device_name: this.props.navigation.state.params.device_name,
      device_id: this.props.navigation.state.params.device_id,
      device_state: this.props.navigation.state.params.device_state,
      force_value: 0,
    }
    this.onTarePressed = this.onTarePressed.bind(this);
    this.onZeroCalibratePressed = this.onZeroCalibratePressed.bind(this);
    this.onHundredCalibratePressed = this.onHundredCalibratePressed.bind(this);
    this.onSaveCalibratePressed = this.onSaveCalibratePressed.bind(this);
    this.unconncectedDevice = this.unconncectedDevice.bind(this);
    this.handleUpdateValueForCharacteristic = this.handleUpdateValueForCharacteristic.bind(this);
  }

  componentDidMount() {
    this.handlerUpdate = bleManagerEmitter.addListener('BleManagerDidUpdateValueForCharacteristic', this.handleUpdateValueForCharacteristic );

    BleManager.connect(this.state.device_id).then(() => {
      let peripherals = this.state.peripherals;
      let p = peripherals.get(this.state.device_id);
      let id = this.state.device_id;
      if (p) {
        p.connected = true;
        peripherals.set(id, p);
        this.setState({peripherals});
      }
      console.log('Connected to ' + id);

      setTimeout(() => {
        BleManager.retrieveServices(id).then((peripheralInfo) => {
          console.log(peripheralInfo);
          var service = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
          var tareCharacteristic = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
          var loadCellCharacteristic = '4a78b8dd-a43d-46cf-9270-f6b750a717c8'; //calibrate as well
          
          setTimeout(() => {
            BleManager.startNotification(id, service, loadCellCharacteristic).then(() => {
              console.log('Started sensing/notification on ' + id);
            }).catch((error) => {
              console.log('Notification error', error);
            });
          }, 200);
        });

      }, 900);
    }).catch((error) => {
      console.log('Connection error', error);
      this.unconncectedDevice();
    });
  }

  unconncectedDevice() {
    Alert.alert("This peripheral is not available.");
  }

  componentWillUnmount() {
    this.handlerUpdate.remove();
  }

  handleUpdateValueForCharacteristic(data) {
    console.log('Received data: ' + data.value[0]);
    this.setState({force_value: data.value[0]});
  }

  onTarePressed() {
    ToastAndroid.show('Tare button pressed', ToastAndroid.SHORT);
  }

  onZeroCalibratePressed() {
    ToastAndroid.show('0g Calibrate button pressed', ToastAndroid.SHORT);
  }

  onHundredCalibratePressed() {
    ToastAndroid.show('100g Calibrate button pressed', ToastAndroid.SHORT);
  }

  onSaveCalibratePressed() {
    ToastAndroid.show('Save Calibration button pressed', ToastAndroid.SHORT);
  }

  render() {
    const { device_id, force_value } = this.state;

    return (
      <View style={styles.container}>
        <View style={{backgroundColor: '#FFF', width: window.width, justifyContent: 'center', alignItems: 'flex-end'}}>
          <TouchableOpacity style={styles.button} onPress={() => { BleManager.disconnect(device_id); }}>
            <Text style={styles.text}>Disconnect</Text>
          </TouchableOpacity>
        </View>
        <View style={styles.view}>
          <Text style={{fontSize: 80}}>{ force_value } g</Text>
        </View>
        <View style={styles.view2}>
          <Text style={{fontSize: 30}}>Connected BLE Device: { this.state.device_name }</Text>
          <Text style={{fontSize: 30}}>MAC Address of BLE Device: { this.state.device_id }</Text>
          <TouchableOpacity style={{backgroundColor:'#ccc', width: 180, height: 80, paddingVertical: 20, margin: 10,}} onPress={() => this.onTarePressed() }>
            <Text style={styles.text}>Tare</Text>
          </TouchableOpacity>
          <View style={{backgroundColor: '#FFF', width: window.width, flexDirection: 'row'}}>
            <TouchableOpacity style={styles.button2} onPress={() => this.onZeroCalibratePressed() }>
              <Text style={styles.text}>0g Calibrate</Text>
            </TouchableOpacity>
            <TouchableOpacity style={styles.button2} onPress={() => this.onHundredCalibratePressed() }>
              <Text style={styles.text}>100g Calibrate</Text>
            </TouchableOpacity>
            <TouchableOpacity style={styles.button2} onPress={() => this.onSaveCalibratePressed() }>
              <Text style={styles.text}>Save Calibration</Text>
            </TouchableOpacity>
          </View>
        </View>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#FFF',
    width: window.width,
    height: window.height,
  },
  view: {
    flex: 1,
    backgroundColor: '#FFF',
    width: window.width,
    justifyContent: 'center',
    alignItems: 'center'
  },
  view2: {
    flex: 1,
    backgroundColor: '#FFF',
    width: window.width,
    alignItems: 'center'
  }, 
  text: {
    fontSize: 30,
    textAlign: 'center',
    textAlignVertical: 'center'
  },
  row: {
    margin: 10
  },
  button: {
    backgroundColor:'#ccc', 
    width: 250, 
    height: 80, 
    paddingVertical: 20, 
    paddingHorizontal: 25,
    margin: 10,
  },
  button2: {
    backgroundColor:'#ccc', 
    width: 240, 
    height: 100, 
    paddingVertical: 20,
    justifyContent: 'center', 
    margin: 5,
  }
});