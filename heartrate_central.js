/*
* implements the bluetooth central for a heart rate monitoring peripheral
* use the Adafruit BluefruitLE heartratemonitor example sketch on the Feather
* modify the Feather BLE chip name in that sketch, and in the globals below
*/

//#################  Settings #################

//update the globals to your details
var CHIP_NAME = "Sarah's Feather"
var SERVICE_ID = "180d"
var CHARACTERISTIC_ID = "2a37"

//#################  Bluetooth callbacks #################

var noble = require('noble/index');
// set state to on
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
        
        // find our characteristic:
        var characteristicIndex = 0;
        for (var i = 0; i < characteristics.length; i++ ) {
          if (characteristics[i].uuid == CHARACTERISTIC_ID) {
            characteristicIndex = i;
          }
        }

        characteristics[characteristicIndex].on('notify', function(state) {
          // console.log('on -> characteristic notify ' + state);

          // set behavior for when the notify has data: 
          this.on('data', function(data) {
            console.log("Heart Rate: ", parseInt(data.toString('hex'), 16));
          });
        });

        //subscribe to notifications
        characteristics[characteristicIndex].subscribe();
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


