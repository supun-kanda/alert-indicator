# alert-indicator
Using this project, a separate physical Node MCU can be informed using MQTT protocol and a HTTP request 

## Creating a AWS Broker
- Sign in to your AWS account
- Under `Find Services` Search and go for `IoT Core`
- Goto `Manage` > `Things`
- Click `Create`
- Click `Create a single thing`
- Give a Name to the thing (`Alert-Indicator`)
- Click `Next`
- Under `One-click certificate creation (recommended)` Click `Create certificate`
- Download given links
   - `A certificate for this thing`
   - `A public key`
   - `A private key`
   - **A root CA for AWS IoT** > **RSA 2048 bit key**
- Click `Activate`
- `Register a thing`
- Then It will be shown that Successfully Created a thing
- `Secure` > `Policies`
- Click `Create`
- Give name for the Policy (`Alert-Policy`)
- Customize your rules
- More on [here](https://docs.aws.amazon.com/iot/latest/developerguide/register-device.html)
