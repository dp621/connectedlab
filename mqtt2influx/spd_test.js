console.log("starting test device....");
const mqtt = require('mqtt');
var client = mqtt.connect('mqtt://localhost');
client.on('connect', function(){console.log('connected to mqtt!'); send();});
function send(){
  console.log("sending spd test device data!");
  client.publish('labData',"{\"device\":\"bf.temp\", \"val\":25, \"unit\":\"C\"}");
  client.publish('labData',"{\"device\":\"head.temp\", \"val\":18.4, \"unit\":\"C\"}");
  client.publish('labData',"{\"device\":\"condenser.temp\", \"val\":85, \"unit\":\"C\"}");
  client.publish('labData',"{\"device\":\"coldtrap.temp\", \"val\":-25, \"unit\":\"C\"}");
  client.publish('labData',"{\"device\":\"pump.vacuum\", \"val\":243, \"unit\":\"micron\"}");
  setTimeout(function(){send();},3000);
}
