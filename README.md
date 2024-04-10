# sauna-monitor

This firmware project was created using [Particle Developer Tools](https://www.particle.io/developer-tools/) and is compatible with all [Particle Devices](https://www.particle.io/devices/).

This is a temperature monitoring utility that periodically reads from 2 temperature probes (inside temperature and outside temperature) and reports both values
to the Particle Cloud. The use-case that this was developed for is to monitor the temperature of a sauna compared to the ambient temperature outside the sauna.


## Prerequisites To Use This Repository

To use this software/firmware on a device, you'll need:

- A [Particle Device](https://www.particle.io/devices/).
- Windows/Mac/Linux for building the software and flashing it to a device.
- [Particle Development Tools](https://docs.particle.io/getting-started/developer-tools/developer-tools/) installed and set up on your computer.
- Optionally, a nice cup of tea (and perhaps a biscuit).

## Getting Started

1. While not essential, we recommend running the [device setup process](https://setup.particle.io/) on your Particle device first. This ensures your device's firmware is up-to-date and you have a solid baseline to start from.

2. If you haven't already, open this project in Visual Studio Code (File -> Open Folder). Then [compile and flash](https://docs.particle.io/getting-started/developer-tools/workbench/#cloud-build-and-flash) your device. Ensure your device's USB port is connected to your computer.

3. Verify the device's operation by monitoring its logging output:
    - In Visual Studio Code with the Particle Plugin, open the [command palette](https://docs.particle.io/getting-started/developer-tools/workbench/#particle-commands) and choose "Particle: Serial Monitor".
    - Or, using the Particle CLI, execute:
    ```
    particle serial monitor --follow
    ```

## TODO
- [ ] add a pinout diagram and temperature probe model numbers


## Architecture & Ecosystem

Notes about how the whole product is plumbed:

1. The Particle firmware publishes an event to the Particle cloud (via `Particle.publish()`) whenever new temperature data is available.  The event is named `sauna/temperature` and has a JSON payload:

        {
            "temperatureInside":124.3
            "temperatureOutside":70
            "timestampMs":3338008192
        }

2. There is a webhook defined in the Particle Cloud Dashboard that is triggered by this event and sends a POST request to Mongo Atlas to insert a record into a Mongo database. There's actually 2 webooks: one inserts a new record into the `temperature` collection as a historical log and the other _replaces_ the entry in the `latest-temperature` collection.  The Particle webook config looks like this:

    <table>
    <tr>
    <td>Name</td><td>Send sauna temperature to Mongo as latest temperature</td>
    </tr>
    <tr>
    <td>Event Name</td><td>sauna/temperature</td>
    </tr>
    <tr>
    <td>Full URL</td><td>https://us-west-2.aws.data.mongodb-api.com/app/data-vgwek/endpoint/data/v1/action/replaceOne</td>
    </tr>
    <tr>
    <td>Request Type</td><td>POST</td>
    </tr>
    <tr>
    <td>Request Format</td><td>Custom Body</td>
    </tr>
    <tr>
    <td>Custom Body</td>
    <td>

    ```json
    { 
        "dataSource": "sandbox", 
        "database": "sauna",
        "collection": "latest-temperature",
        "filter": {
            "_id": {
                "$oid": "66163f479c4b5e9f0b5a2978"
            }
        },
        "replacement": {{{PARTICLE_EVENT_VALUE}}}
    }
    ```

    </td>
    </tr>
    <tr>
    <td>Headers</td>
    <td>

    ```json
    {
    "apiKey": "<MONGO_ATLAS_API_KEY>",
    "content-type": "application/json",
    "Accept": "application/json"
    }
    ```

    </td>
    </tr>
    <tr>
    <td>Enforce SSL</td><td>Yes</td>
    </tr>
    </table>

