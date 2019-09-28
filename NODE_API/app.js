// call all the required packages
const express = require('express');
const bodyParser= require('body-parser') ;
const awsIot = require('aws-iot-device-sdk');
 
//CREATE EXPRESS APP
const app = express();
 
app.use(bodyParser.urlencoded({extended: true}))

// IoT Device
var device = awsIot.device({
    keyPath: './auth_files/3e7222e3c6-private.pem.key',
    certPath: './auth_files/3e7222e3c6-certificate.pem.crt',
    caPath: './auth_files/AmazonRootCA1.pem',
    clientId: 'my-test',
    host: 'abx9e94fmlpan-ats.iot.us-west-2.amazonaws.com'
});

//ROUTES WILL GO HERE
app.get('/', function(req, res) {
    res.json({ message: 'WELCOME' });   
});

app.post('/PostAlert', function(req, res) {
  console.log(req.body)
  device.publish('cm-alerts', JSON.stringify({ message: "Error", type:"ERROR"}));
  res.status(200).send({success:true});
});

app.post('/Fixed', function(req, res) {
  console.log(req.body)
  device.publish('cm-alerts', JSON.stringify({ message: "Error Fixed", type:"FIXED"}));
  res.status(200).send({success:true});
});
 
app.listen(3000, () => console.log('Server started on port 3000'));



//
// Device is an instance returned by mqtt.Client(), see mqtt.js for full
// documentation.
//
device
  .on('connect', function() {
    console.log('connect');
    device.subscribe('cm-alerts');
    device.publish('device-health', JSON.stringify({ message: "API Connected Successfully"}));
  });

device
  .on('message', function(topic, payload) {
    console.log('message', topic, payload.toString());
  });