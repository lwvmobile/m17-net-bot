
# M17 Net Bot

## Information

[M17 Project](https://m17project.org/ "M17") Net Bot is a UDP/IP only reflector and adhoc bot capable of listening for incoming SMS commands and issuing relevant replies, text-to-speech of .txt files (using piper, espeak, or epeak-ng) into the open source [Codec2](https://github.com/drowe67/codec2 "Codec2") vocoder at 3200bps voice mode, playing advertisements on an interval, and other misc bot type functions.

## Functionality

### M17 Net Bot Commands:

`*get tle` to retreive Satellite TLE data and send via 0x07 Packet Data.

`*get wttr {insert location}` to get simple weather from wttr.in service as 0x05 SMS Text and TTS voice.

`*get time` to get local time returned as 0x05 SMS Text.

`*get lucky` to get a set of 6 randomly generated numbers returned as 0x05 SMS Text.

`*get 8ball {insert question here optional}` to get your eight ball fortune returned as 0x05 SMS Text.

`*tts {insert text for TTS}` to convert insert text into speech and encode/stream to Codec2 3200bps.

`*read filename.txt` to convert a bot local .txt file into speech and encode/stream to Codec2 3200bps.

`*play wavname.wav` to encode a S16LE 48k/1 encoded wav file and encode/stream to Codec2 3200bps.


### M17 Net Bot Advertisement

M17 Net Bot can be configured to read a text file of choice and convert into speech and stream as voice, and then send a text SMS of the same advertisement (up to 823 characters) on a selected interval time frame.

### MISC

Encryption is not supported for M17 Net Bot.

M17 Net Bot Supports both Version 2.0.x LSF and Version 3.0.x draft LSF, and is configurable to use either.

IP6 is currently not supported, but might be supported in a future update.

M17 Net Bot makes use of the system command for TTS and TLE fetching.

M17 Net Bot will make best effort attempts to not interrupt a voice stream in progress, but may potentially send Packet Data if Packet Data command received during a voice stream if the reflector supports it. Please be courteous and not set advertisement intervals to be very frequent, or constantly spam TTS or other commands on busy reflectors.

### About M17 Project

M17 Net Bot is capable of transmitting and receiving UDP/IP frames based on [M17 Protocol Specifications Part II - Internet Interface](https://github.com/M17-Project/M17_inet "M17 Protocol Specifications Part II - Internet Interface").

To see more information on M17 Project, please see [M17 Protocol Specifications Part I - Air Interface](https://spec.m17project.org/ "M17 Protocol Specifications Part I - Air Interface").

### How to Use

To use M17 Net Bot, simply configure the easy to use config.txt files which can be found in the samples folder to your needs, and then call M17 Net Bot with the command `./m17-net-bot config.txt` to start it. The only accepted argument is the config file itself.

An Example Configuration File will look like this, including all supported settings: 

```
my_src_callsign=BASE40CSD
my_dst_callsign=@ALL
m17_udp_hostname=ENTER_IP4_ADDRESS_HERE
m17_udp_portno=17000
reflector_module=C
adhoc_mode=0
send_advertisement_traffic=0
advertisement_time_interval=3600
advertisement_text_file=../samples/advertisement.txt
pre_encoded_wav_file=../samples/wav_file.wav
my_tle_source=https://live.ariss.org/iss.txt
my_weather_location=Anytown USA
lsf_type_version=2
my_can=0
```

It is recommended to make a copy of one of the config.txt files from samples, and put your callsign in it, the IP of the refelctor you wish to connect to along with the correct module letter.

### How to Build

Basic M17 Net Bot dependencies include: `libsndfile (sndfile), libcodec2 (codec2), gcc, wget, cmake, make, git, sox, espeak-ng`

TTS Support will require either [piper1-gpl](https://github.com/OHF-Voice/piper1-gpl "piper1-gpl") for nice sounding TTS, or espeak / espeak-ng for basic TTS.

To build, ensure you have the following dependencies above installed. Check your Linux Distro of choice for package names and install via package manager of choice. It is also highly recommended for novices that you use the build folder for both the default run location of m17-net-bot, and the python virtual environment (venv) with piper1, folling the intructions below in this exact order.

```
git clone https://github.com/lwvmobile/m17-net-bot
cd m17-net-bot
mkdir build
cd build
python3 -m venv piper1
source piper1/bin/activate
pip install piper-tts
python3 -m piper.download_voices en_US-norman-medium
cmake .. -DUSEPIPER=ON
make
```

To run m17-net-bot with the recommended setup, run `./m17-net-bot ../samples/config.txt`

Be sure to activate your venv with piper in it before running the bot, if closing and opening a new terminal, etc.
`source piper1/bin/activate`

Alternatively, for low powered hardware devices (Raspberry Pi, Old Computers, etc) using espeak instead of piper may perform much better with less impact and time on encoding TTS. To build for a lower powered devices, run:

```
git clone https://github.com/lwvmobile/m17-net-bot
cd m17-net-bot
mkdir build
cd build
#if using espeak (default TTS)
cmake .. -DUSETTS=ON
#if using espeak-ng
#cmake .. -DUSEESPEAKNG=ON
make
```
