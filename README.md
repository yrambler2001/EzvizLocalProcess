## EzvizLocalProcess

Used to automatically download the videos in the memory card of the EZVIZ camera to the local computer. Based on EZVIZ's C++ OpenSDK. https://open.ys7.com/help/en/490

### Settings
- Rename `config.json.sample` to `config.json`
- Log in to the EZVIZ Open Platform (https://ieuopen.ezviz.com/console/login.html), then go to "Account Settings" - "Appkey management", and fill the obtained `AppKey` and `Secret` into the `private.appKey` and `private.appSecret` in config.json
- Go to "My resources" - "Equipment list", find your camera, get the serial number and fill in the camera's serial number into `device.private.deviceSerial` property
- Set the search time range `device.searchRange`. Keep in mind that API thinks dates on the cameras are recorded in UTC, e.g. if camera has a recording 2025-05-25 19:00:00 (19:00 regardless of camera's timezone (camera's timezone is not recorded/used anywhere), it is local camera time) and PC that runs this repository is located in UTC+2, API expects the 1748199600000 timestamp, (1748199600000 timestamp equals 2025-05-25T19:00:00.000Z), so to match the 2025-05-25 19:00:00 date on camera, a 2025-05-25 21:00:00 date is expected to be configured in `device.searchRange.start`
- `python ezviz.py` with python3 to start the application to download the videos in the camera's memory card to the local computer
- keep the `private.accessToken.accessToken`, `device.private.secretKey` empty