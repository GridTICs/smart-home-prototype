Configuration

* Public view of the channel: https://thingspeak.com/channels/696710
* Sensors report data every 15 minutes.

Examples of Thingspeak API use


* Get last value of temperature: GET https://api.thingspeak.com/channels/696710/fields/1.json?results=1
* Get the last 10 values of temperature field: GET https://api.thingspeak.com/channels/696710/fields/1.json?results=10
* Get the last value of each field: GET https://api.thingspeak.com/channels/696710/feeds.json?results=1
* Get the last 2 values of all fields: GET https://api.thingspeak.com/channels/696710/feeds.json?results=2


