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
  ToastAndroid
} from 'react-native';
import BleManager from 'react-native-ble-manager';

const BleManagerModule = NativeModules.BleManager;
const bleManagerEmitter = new NativeEventEmitter(BleManagerModule);

export default class DevicePg extends Component {
  constructor(props){
    super(props);
    this.state = {
      peripherals: this.props.navigation.state.params.peripherals,
      device_id: this.props.navigation.state.params.device_id,
    }
    this.onTarePressed = this.onTarePressed.bind(this);
    this.onCalibratePressed = this.onCalibratePressed.bind(this);
  }

  componentDidMount() {
    /*BleManager.connect(peripheral.id).then(() => {
      let peripherals = this.state.peripherals;
      let p = peripherals.get(this.state.device_id);
      let id = this.state.device_id;
      if (p) {
        p.connected = true;
        peripherals.set(id, p);
        this.setState({peripherals});
      }
      console.log('Connected to ' + id);

      /*setTimeout(() => {
        BleManager.retrieveServices(id).then((peripheralInfo) => {
          console.log(peripheralInfo);
          var service = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
          var tareCharacteristic = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
          var loadCellCharacteristic = '4a78b8dd-a43d-46cf-9270-f6b750a717c8'; //calibrate as well

          setTimeout(() => {
            BleManager.startNotification(id, service, loadCellCharacteristic).then(() => {
              console.log('Started sensing/notification on ' + id);
              setTimeout(() => {
                BleManager.write(id, service, loadCellCharacteristic, [0]).then(() => {
                  //console.log('Writed NORMAL crust');
                  /*BleManager.write(id, service, bakeCharacteristic, [1,95]).then(() => {
                    console.log('Writed 351 temperature, the pizza should be BAKED');
                  });*/
                /*});

              }, 500);
            }).catch((error) => {
              console.log('Notification error', error);
            });
          }, 200);
        });

      }, 900);
    }).catch((error) => {
      console.log('Connection error', error);*/
    //});
  }

  onTarePressed() {
    ToastAndroid.show('Tare button pressed', ToastAndroid.SHORT);
  }

  onCalibratePressed() {
    ToastAndroid.show('Calibrate button pressed', ToastAndroid.SHORT);
  }

  render() {
    return (
      <View style={styles.container}>
        <View style={styles.view}>
          <Text style={{fontSize: 40}}>Force Value</Text>
        </View>
        <View style={styles.view2}>
          <Text style={{fontSize: 30}}>Connected BLE Device: { this.state.device_id }</Text>
          <TouchableOpacity style={{backgroundColor:'#ccc', width: 150, height: 100, margin: 10}} onPress={() => this.onTarePressed() }>
            <Text style={styles.text}>Tare</Text>
          </TouchableOpacity>
          <TouchableOpacity style={{backgroundColor:'#ccc', width: 150, height: 100, margin: 10}} onPress={() => this.onCalibratePressed() }>
            <Text style={styles.text}>Calibrate</Text>
          </TouchableOpacity>
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
});