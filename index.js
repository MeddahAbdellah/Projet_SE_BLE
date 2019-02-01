var bleno = require('bleno');
const fs = require('fs');
const { spawn, fork } = require('child_process');

var name = 'ENPELN';
var serviceUuids = ['75cf7374-a137-47e7-95e5-e675189c8a3e']
var Characteristic = bleno.Characteristic;

var PrimaryService = bleno.PrimaryService;

var primaryService = new PrimaryService({
    uuid: '75cf7374-a137-47e7-95e5-e675189c8a3e',
    characteristics: [
        new Characteristic({
    uuid: '0d563a58-196a-48ce-ace2-dfec78acc814', 
    properties: ["read","write"], 
    secure: null, 
    value: null,
    descriptors: null,
    onWriteRequest: function(data, offset, withoutResponse, callback) {
                      console.log('onWriteequest: value = ' + data.toString());
                      callback(this.RESULT_SUCCESS, this._value);
                      
                        let path = 'BLEfifo';  
                        let buffer = new Buffer(data.toString());
                        fs.open(path, 'w', function(err, fd) {  
                            if (err) {
                                throw 'could not open file: ' + err;
                            }

                            // write the contents of the buffer, from position 0 to the end, to the file descriptor returned in opening our file
                            fs.write(fd, buffer, 0, buffer.length, null, function(err) {
                                if (err) throw 'error writing file: ' + err;
                                fs.close(fd, function() {
                                    console.log('wrote the file successfully');
                                });
                            });
                        });
                          bleno.disconnect();
                          bleno.startAdvertising(name, ["75cf7374-a137-47e7-95e5-e675189c8a3e"]);
                        }
})
    ]
});


bleno.on('stateChange', function(state) {
  if (state === 'poweredOn'){
    bleno.startAdvertising(name, ["75cf7374-a137-47e7-95e5-e675189c8a3e"]);
  }
  else
    bleno.stopAdvertising();
});

bleno.on('advertisingStart', function(error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

  if (!error) {
    bleno.setServices([primaryService], function(error){
      console.log('setServices: '  + (error ? 'error ' + error : 'success'));    });
  }
});
