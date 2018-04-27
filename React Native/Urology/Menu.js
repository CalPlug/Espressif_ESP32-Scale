/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 * @flow
 */
import React, { Component } from 'react';
import {
  Platform,
  StyleSheet,
  Text,
  View,
  TouchableOpacity,
  ToastAndroid
} from 'react-native';

export default class App extends Component {
  onPressed = () => {
    ToastAndroid.show("Scanning....", ToastAndroid.LONG);
  }
  
  render() {
    return (
      <View style={styles.container}>
        <View style={styles.buttonContainer}>
          <TouchableOpacity
            style={styles.button}
            onPress={this.onPressed}>
              <Text style={styles.text}>Scan</Text>
          </TouchableOpacity>
        </View>
        <View style={styles.container}/>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#F5FCFF',
  },
  buttonContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#F5FCFF',
  },
  button: {
    //paddingTop: 5,
    backgroundColor: '#2ecc71',
  },
  text: {
    color: '#F5FCFF',
    fontSize: 100,
    textAlign: 'center',
    justifyContent: 'center'
  }
});
