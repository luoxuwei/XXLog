# XXLog
将腾讯mars 项目中xlog模块抽离成单独库，并进行重构，只保留核心功能，在架构和代码结构上进行简化和优化。去掉外部依赖和跨平台相关代码。

# 生成秘钥
1.安装pyelliptic 细节可以参考wiki。

2.运行[key_gen.py](https://github.com/luoxuwei/XXLog/blob/main/xxlog/src/main/cpp/crypt/gen_key.py)

```shell script
python2 ./gen_key.py
```
执行成功会打印出公钥和私钥:

```shell script
save private key
xxxxxxxx

appender_open's parameter:
xxxxxxxxxxx
```
输出的appender_open's parameter就是公钥，是appenderOpen接口的参数，将私钥拷贝到log解压脚本[decode_mars_crypt_log_file.py](https://github.com/luoxuwei/XXLog/blob/main/xxlog/src/main/cpp/crypt/decode_mars_crypt_log_file.py)中覆盖PRIV_KEY字段

# Init

```kotlin
val externalFilesDir: File? = applicationContext.getExternalFilesDir(null)
val externalFilesDirPath =
     if (externalFilesDir != null) externalFilesDir.getAbsolutePath() else "/sdcard"
val logPath = externalFilesDirPath + "/xxlog"
val cachePath = applicationContext.getDir("xxlog", Context.MODE_PRIVATE).absolutePath;
val xlog = XXLog()
Log.setLogImp(xlog)
Log.setConsoleLogOpen(true)
Log.appenderOpen(XXLog.LEVEL_DEBUG, XXLog.AppednerModeAsync, cachePath, logPath, "xxlog", 0, "xxxxx");
```

# 解密

1.安装zstandard

```shell
pip2 install zstandard
```

2.执行[decode_mars_crypt_log_file.py](https://github.com/luoxuwei/XXLog/blob/main/xxlog/src/main/cpp/crypt/decode_mars_crypt_log_file.py)脚本解密:

```shell
python2 decode_mars_crypt_log_file.py xxlog_20211114.xlog
```

