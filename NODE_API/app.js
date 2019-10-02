// call all the required packages
const express = require('express');
const bodyParser= require('body-parser') ;
const awsIot = require('aws-iot-device-sdk');
 
//CREATE EXPRESS APP
const app = express();
 
app.use(bodyParser.urlencoded({extended: true}))

// IoT Device
var device = awsIot.device({
    keyPath: './auth_files/76690f0b52-private.pem.key',
    certPath: './auth_files/76690f0b52-certificate.pem.crt',
    caPath: './auth_files/AmazonRootCA1.pem',
    clientId: 'cm-message-broker',
    host: 'abx9e94fmlpan-ats.iot.us-west-2.amazonaws.com'
});

//ROUTES WILL GO HERE
app.get('/', function(req, res) {
    res.json({ message: 'WELCOME' });   
});

app.post('/PostAlert', function(req, res) {
  device.publish('cm-alerts/alert', JSON.stringify({ message: "Error", type:"ERROR"}));
  res.status(200).send({success:true});
});
app.post('/Fixed', function(req, res) {
  device.publish('cm-alerts/fix', JSON.stringify({ message: "Error Fixed", type:"FIXED"}));
  res.status(200).send({success:true});
});
 
app.listen(3000, () => console.log('Server started on port 3000'));



//
// Device is an instance returned by mqtt.Client(), see mqtt.js for full
// documentation.
//
device
  .on('connect', function() {
    console.log('connected');
    device.subscribe('cm-alerts');
    device.publish('cm-alerts/connection', JSON.stringify({ message: "API Connected Successfully"}));
  });

device
  .on('message', function(topic, payload) {
    console.log('message', topic, payload.toString());
  });