## EzvizLocalProcess

用于自动下载萤石摄像头内存卡中的录像到本地。基于萤石的 C++ OpenSDK。

### 设置
- 重命名 `config.json.sample` 为 `config.json`
- 登录萤石开放平台，然后到“我的账号”-“应用信息”里创建应用，将获取到的 `AppKey` 和 `Secret` 填入配置 `private` 下的 `appKey` 和 `appSecret` 里
- 分别填入摄像头的序列号和验证码到 `deviceSerial` 和 `secreyKey` 中
- 设定搜索时间范围 `searchRange`，该范围内的视频下载完成后会继续搜索至当前时间
- （可选）设定每次的搜索步进 `searchStep`，按秒计算，默认为 3 天
- `python ezviz.py` 启动应用即可下载摄像头内存卡中的录像到本地