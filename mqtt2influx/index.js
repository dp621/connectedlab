const Influx = require("influx");
const influx = new Influx.InfluxDB({
	host: 'localhost',
	database: 'connectedlabdata',
	port: 8086
});

console.log('connecting to mqtt');
const mqtt = require('mqtt');
var client = mqtt.connect('mqtt://localhost');
client.on('connect', function () { console.log('connected!'); client.subscribe('labData', function (err) {}) });
client.on('message', function (topic, message) {
  //message is Buffer
  console.log(topic+" ==> "+message.toString());
  var msg=JSON.parse(message.toString());
  if (msg.device!=undefined
	&& msg.val!=undefined
	&& msg.unit!=undefined){
    writeData(msg);
  }
});

console.log('started! waiting for data...');

function writeData(data){
  //write to influx db
  var dbdata = [];
  dbdata.push({measurement: data.device,
		fields: {
			value: data.val,
			unit: data.unit
		}
  });
  influx.writePoints(dbdata).then(() => {}, (e) => console.error(e));
}
