# Run openthread daemon

In openthread repo, run:

```
make -f ./src/posix/Makefile-posix TOBLE=1 BLE_HOST=bluez PLATFORM_NETIF=1 PLATFORM_UDP=1 UDP_FORWARD=0 FULL_LOGS=1
```

Then run:

```
sudo ./output/posix/x86_64-unknown-linux-gnu/bin/ot-cli -d 7 -I wpan0 hci0
```

In the openthread cli:

```
> toble linkmode central
Done
> panid 1
Done
> ifconfig up
Done
> unsecureport add 11095
Done

```

# Build & flash CHIP

In CHIP repo, first manually checkout openthread to toble-1 branch:

```
cd third_party/openthread/repo
git checkout toble-1
```

Follow the guide in `docs/BUILDING-GN.md` to setup the build environment.

Run
```
./gn_build.sh
```
to build the binaries.

Flash nrf52840 with this command:

```
nrfjprog -f nrf52 --program ./out/debug/nrf5_lighting_app/chip-nrf52840-lighting-example.hex --sectorerase --reset
```

# Control the light

Access the serial port of the nrf dev kit:
```
picocom -b 115200 /dev/ttyACM0
> ipaddr
fe80:0:0:0:54d7:b8a8:8482:fb9b
Done
```

Now control the device with chip-tool:
```
./out/debug/standalone/chip-tool on fe80::54d7:b8a8:8482:fb9b%wpan0 11095 1
./out/debug/standalone/chip-tool off fe80::54d7:b8a8:8482:fb9b%wpan0 11095 1
```
