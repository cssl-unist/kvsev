# KVSEV-SoCC'23

## Environment
- Host: Ubuntu 20.04.3 LTS with AMD EPYC 7262 processor
- Guest: Ubuntu 18.04.6 LTS
- App: based on [Memcached] 1.6.12

[Memcached]: <https://memcached.org/>
[here]: <https://github.com/AMDESE/AMDSEV/tree/sev-es>
[libevent-dev]: <https://www.monkey.org/~provos/libevent/>
[libmemcached-dev]: <https://libmemcached.org/libMemcached.html>

##  Host Setup
AMD's official repository for setting up SEV VMs is maintained [here]. Refer to their documentation if you have any troubles with host/guest setup.
```sh
$ git clone https://github.com/AMDESE/AMDSEV.git
$ cd AMDSEV/build
$ sudo ./build.sh
```
The script will install host OS and OVFM for SEV support. After booting, `dmesg` should contain something similar to the following:
```
[X.XXXXXX] ccp 0000:xx:00.1: SEV firmware update successful
[X.XXXXXX] ccp 0000:xx:00.1: SEV API:0.24 build:14
[X.XXXXXX] SEV supported: XXX ASIDs
```

## Guest Setup
### Prepare Guest
(1) Create an empty disk for VM:
```sh
$ qemu-img create -f qcow2 <IMAGE_NAME>.qcow2 30G
```
(2) Install guest image:
```sh
$ sh $AMDSEV_INSTALL_DIR/launch_qemu.sh -hda <IMAGE_NAME>.qcow2 -cdrom <DISTRO_ISO>.iso -vnc 1
```
<IMAGE_NAME>.qcow2 should be the path where empty disk is created, and <DISTRO_ISO>.iso should be the path to the regular Ubuntu installation iso.

(3) Connect to VNC session and complete the installation

### Launch Guest
(4) Use following command to launch SEV VM:
```sh
$ sh $AMDSEV_INSTALL_DIR/launch-qemu.sh -hda <IMAGE_NAME>.qcow2 -vnc 1 -console serial -sev-es
```

## Build Key-Value Store
(1) Download the source code on guest VM:
```sh
$ git clone https://github.com/cssl-unist/kvsev.git
```
(2) Install dependencies ([libevent-dev] and [libmemcached-dev]):
```sh
$ sudo apt-get install libevent-dev libmemcached-dev
```
(3) Build key-value store:
```sh
$ cd kvsev
$ pushd src/merkle && make && popd
$ mkdir build && cd build
$ ./../memcached/configure --srcdir=../memcached
$ make
```
Several load flags and define flags are hard-coded into `configure` and `Makefile.in`. You can create different build directory and change the flags before the build to run unmodified key-value store.

(4) Build and install helper module
```sh
# from project root directory
$ cd src/module
$ make
$ sudo insmod sev_helper.ko
```
The helper module exposes several custom ioctl functions to create, destroy, and attest ephemeral SEV VMs from key-value store application. After inserting the module, `dmesg` should contain the following:
```sh
[X.XXXXXX] [KVSEV] Successfully inserted helper module
```

## Run and Test Key-Value Store
### Launch KVS
```
$ ./$KVS_BUILD_DIR/memcached
```
### Launch Client
```sh
# From project root directory
$ cd src/client
$ make
$ sudo ./run.sh
```
You can configure the workloads (and benchmarks) by adjusting the input flags.

## Authors
- Junseung You (Seoul National University) <jsyou@sor.snu.ac.kr>
- Kyeongryong Lee (Seoul National University) <krlee@sor.snu.ac.kr>
- Hyungon Moon (UNIST) <hyungon@unist.ac.kr>
- Yeongpil Cho (Hanyang University) <ypcho@hanyang.ac.kr>
- Yunheung Paek (Seoul National University) <ypaek@snu.ac.kr>

## Publications
```
@inproceedings{you2023kvsev,
  title={KVSEV: A Secure In-Memory Key-Value Store with Secure Encrypted Virtualization},
  author={You, Junseung and Lee, Kyeongryong and Moon, Hyungon and Cho, Yeongpil and Paek, Yunheung},
  booktitle={Proceedings of the 2023 ACM Symposium on Cloud Computing},
  pages={233--248},
  year={2023}
}
```
