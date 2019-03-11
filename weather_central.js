/*
*  implements the bluetooth central for a weather-responsive peripheral
*  assumes the heartrate GATT has been adapted to be a read/write property
*  use Arduino sketch : weather_peripheral.ino
*/

//#################  Settings #################

//update the globals to your details
var CHIP_NAME = "Sarah's Feather"
var SERVICE_ID = "180D"
var CHARACTERISTIC_ID = "2222"

// sign up for a free darksky api key: https://darksky.net/dev
// ADD THE API KEY HERE
var APIKEY = ""


//#################  API call #################

// API call to Dark Sky weather api
// Berkeley 37.8716° N, 122.2727° W
const request = require('request');
var temperature = 0;
request('https://api.darksky.net/forecast/' + APIKEY + '/37.8716,-122.2727', 
          //indicate that response is json formatted
          { json: true }, 
          function(err, res, body) {
              if (err) { return console.log(err); }
              // read the json to get current temperature
              temperature = body.currently.temperature.toString();
              console.log("temp: ", temperature);
          }
);

//#################  Bluetooth callbacks #################

var noble = require('noble/index');
// set noble state to on
noble.state = "poweredOn";


// start scanning for available bluetooth peripherals when on
noble.on('stateChange', function(state) {
  if (state === 'poweredOn') {
   noble.startScanning(); 
  } else {
    noble.stopScanning();
  }
});

//find all the peripherals advertising: 
noble.on('discover', function(peripheral) {
    console.log('\thello my local name is:');
    console.log('\t\t' + peripheral.advertisement.localName);
    console.log();

    // when you connect to a peripheral, get the service we want
    peripheral.on('connect', function() {
      console.log('on -> connect');
      this.discoverServices([SERVICE_ID]);
    });

    //when you disconnect, log a message
    peripheral.on('disconnect', function() {
      console.log('on -> disconnect');
    });

    //on discover, find the peripheral's services: 
    peripheral.on('servicesDiscover', function(services) {
      // console.log('on -> peripheral services discovered ' + services);

      //it just so happens that the service we want is at index 0; otherwise change this
      var serviceIndex = 0;

      services[serviceIndex].on('characteristicsDiscover', function(characteristics) {
        // console.log('on -> service characteristics discovered ' + characteristics);
        
        // find what index is our characteristic:
        var characteristicIndex = 0;
        for (var i = 0; i < characteristics.length; i++ ) {
          if (characteristics[i].uuid == CHARACTERISTIC_ID) {
            characteristicIndex = i;
          }
        }

        characteristics[characteristicIndex].on('write', function() {
          console.log('on -> characteristic write ');
          // console.log(this.uuid);
          // peripheral.disconnect();
        });

        characteristics[characteristicIndex].on('read', function(data, isNotification) {
          console.log('on -> characteristic read ');
          console.log(data.toString('hex'));
          peripheral.disconnect();
        });

        //write the temperature to our weather characteristic  
        console.log(new Buffer(temperature));
        characteristics[characteristicIndex].write(new Buffer(temperature));

        // if you want to make sure it worked, you can read the temperature back out: 
        // characteristics[characteristicIndex].read();
      });

      // discover characteristics for our service: 
       services[0].discoverCharacteristics();
    });

  //stop scanning once you find the right peripheral, and connect to it
  if (peripheral.advertisement.localName == CHIP_NAME) {
    noble.stopScanning();
    peripheral.connect();
  }

});


