console.log("starting test device....");
const mqtt = require('mqtt');
var client = mqtt.connect('mqtt://localhost');
client.on('connect', function(){console.log('connected to mqtt!');});
function send(){
  console.log("sending test device data!");
  client.publish('labData',"{\"device\":\"bf.temp\", \"val\":25, \"unit\":\"C\"}");
  setTimeout(function(){send();},3000);
}
send();
