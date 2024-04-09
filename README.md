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
