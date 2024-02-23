/* Plugin for Homebridge to connect to Apple Homekit
* This plugin is for local use only and is to be stored locally on your 
* Homebridge 
*/

const axios = require('axios');

// Export the plugin
module.exports = function (homebridge) {
    Service = homebridge.hap.Service;
    Characteristic = homebridge.hap.Characteristic;

    homebridge.registerAccessory('smart-blinds', 'SmartBlinds', WindowCoveringAccessory);
};

// Define the WindowCoveringAccessory class
class WindowCoveringAccessory {
    constructor(log, config) {
        this.log = log;
        this.name = config.name;
        this.url = config.url;
		this.currentPosition = 0;
    }

    // Method to set the position of the window covering
    setPosition(position, callback) {
		const options = {
			method: 'POST',
			url: `${this.url}/setpoint`,
			headers: {
				'Content-Type': 'application/x-www-form-urlencoded'
			},
			data: `value=${position}`, // Form-encoded data
			responseType: 'json'
		};
	
		axios(options)
			.then(response => {
				this.log(`Position set to ${position}%`);
				this.currentPosition = position;
				callback();
			})
			.catch(error => {
				this.log.error(`Error setting position: ${error.message}`);
				callback(error);
			});
	}
	

    // Method to get the current position of the window covering
    getPosition(callback) {
        const options = {
            method: 'GET',
            url: `${this.url}/position`,
            responseType: 'json'
        };

        axios(options)
            .then(response => {
                const position = response.data.position;
                this.log(`Current position: ${position}`);
				this.currentPosition = position;
                callback(null, position);
            })
            .catch(error => {
                this.log.error(`Error getting position: ${error.message}`);
                callback(error);
            });
    }

    // Required method to get the services provided by the accessory
    getServices() {
        const windowCoveringService = new Service.WindowCovering(this.name);

        windowCoveringService
            .getCharacteristic(Characteristic.TargetPosition)
            .on('set', this.setPosition.bind(this));

        windowCoveringService
            .getCharacteristic(Characteristic.CurrentPosition)
            .on('get', this.getPosition.bind(this));

		setInterval(() => {
			windowCoveringService.getCharacteristic(Characteristic.CurrentPosition).updateValue(this.currentPosition);
		}, 5000);

        return [windowCoveringService];
    }
}
