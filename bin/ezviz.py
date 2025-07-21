# -*- coding:utf-8 -*-

import urllib.request, urllib.parse, urllib.error
import json
import os
import subprocess
import time
from datetime import datetime

import conf
import log

def CreateOpener():
    op = urllib.request.build_opener()
    return op

def LoadConfiguration():
    if not os.path.exists(conf.GetConfigFilePath()):
        log.error('config file not exist')
    with open(conf.GetConfigFilePath(), 'r') as f:
        return json.loads(f.read())
    return None

def SaveConfiguration():
    with open(conf.GetConfigFilePath(), 'w') as f:
        f.write(json.dumps(CONF, indent=2))

def UpdateAccessToken(opener):
    if 'accessToken' in CONF['private'] and time.time() < CONF['private']['accessToken']['expireTime']:
        log.info('use cached token')
    else:
        log.info('refresh access token')
        CONF['private']['accessToken'] = {}
        params = {
            'appKey' : CONF['private']['appKey'],
            'appSecret' : CONF['private']['appSecret']
        }
        print(params)
        post_data = urllib.parse.urlencode(params).encode()
        req = urllib.request.Request(
            'https://open.ezvizlife.com/api/lapp/token/get',
            data = post_data
        )
        j = json.loads(opener.open(req).read())
        log.info(j)
        if j['code'] == '200':
            CONF['private']['accessToken']['accessToken'] = j['data']['accessToken']
            CONF['private']['accessToken']['expireTime'] = j['data']['expireTime']
            SaveConfiguration()
        else:
            log.warn(j)

def GetLocalRecordList(opener, token, serial, camera, rng_from, rng_to):
    log.info('search local records')
    params = {
        'accessToken' : token,
        'deviceSerial' : serial,
        'channelNo' : camera,
        'startTime' : str(rng_from),
        'endTime' : str(rng_to)
    }
    print(params)
    post_data = urllib.parse.urlencode(params).encode()
    log.info('post kv: ' + str(post_data))
    req = urllib.request.Request(
        'https://open.ezvizlife.com/api/lapp/video/by/time',
        data = post_data
    )
    try:
        j = json.loads(opener.open(req).read())
        log.info(j)
        if j['code'] == '200':
            return (True,j['data']) if j['data'] is not None else (True,{})
        else:
            return False,{}
    except:
        return False,{}

def GenerateFilePathFromLocalRecord(record):
    log.info(record)
    return os.path.join(
        conf.GetDownloadsDir(),
        record['deviceSerial'],
        str(record['recType']),
        time.strftime("%Y-%m-%d", time.localtime(record['startTime'] / 1000)),
        time.strftime("%Y-%m-%d %H-%M-%S", time.localtime(record['startTime'] / 1000)) + '.mp4'
    )

def DownloadLocalRecord(token, serial, secret_key, camera, time_start, time_end, save_path):
    main = conf.GetLocalRecordDownloaderPath()
    if not os.path.exists(main):
        log.error('LocalRecordDownloader.exe not exist')
        return False
    log.info('process: %s', save_path)
    if os.path.exists(save_path):
        log.info('record already exists, skip')
        return True
    for i in range(3):
        try:
            ll = [main, '-accessToken', token, '-deviceSerial', serial, '-secretKey', secret_key, '-cameraNo', str(camera), '-startTime', str(time_start), '-endTime', str(time_end), '-savePath', save_path]
            print(ll)
            subprocess.run(ll, check=True)
            if os.path.exists(save_path):
                log.info('record download successful')
                return True
            else:
                log.warn('download failed, retry')
        except subprocess.CalledProcessError as e:
            log.info('failed return code: %d, retry', e.returncode)
            pass
    return False

def TimestampToStr(ts):
    return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(ts))
    # return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(ts-3600-3600))

def StrToTimestamp(s):
    return int(datetime.strptime(s, '%Y-%m-%d %H:%M:%S').timestamp())
    # return int(datetime.strptime(s, '%Y-%m-%d %H:%M:%S').timestamp())+3600+3600

def main():
    try:
        log.InitLogger()
        log.info('************************** log start **************************')

        global CONF
        CONF = LoadConfiguration()
        opener = CreateOpener()
        while True:
            UpdateAccessToken(opener)
            start_timestamp = StrToTimestamp(CONF['device'][0]['searchRange']['start'])
            end_timestamp = StrToTimestamp(CONF['device'][0]['searchRange']['end'])
            log.info('start for %s ~ %s', CONF['device'][0]['searchRange']['start'], CONF['device'][0]['searchRange']['end'])
            success,local_record_list = GetLocalRecordList(
                opener,
                CONF['private']['accessToken']['accessToken'],
                CONF['device'][0]['private']['deviceSerial'],
                CONF['device'][0]['cameraNo'],
                start_timestamp * 1000,
                end_timestamp * 1000
            )
            log.info('%d, got %d records', success, len(local_record_list))
            if not success:
                time.sleep(5)
                continue

            for rec in local_record_list:
                downloaded = DownloadLocalRecord(
                    CONF['private']['accessToken']['accessToken'],
                    CONF['device'][0]['private']['deviceSerial'],
                    CONF['device'][0]['private']['secretKey'],
                    CONF['device'][0]['cameraNo'],
                    TimestampToStr(rec['startTime'] / 1000),
                    TimestampToStr(rec['endTime'] / 1000),
                    GenerateFilePathFromLocalRecord(rec)
                )
                if not downloaded:
                    CONF['device'][0]['localDownloadFailed'].append(rec)
                    SaveConfiguration()
                    log.warn('download failed.')
            log.info('download end for this range')
            break
            if end_timestamp > int(time.time()):
                log.info('already search to current time, exit')
                break
            start_timestamp = end_timestamp
            end_timestamp = end_timestamp + CONF['device'][0]['searchStep']
            CONF['device'][0]['searchRange']['start'] = TimestampToStr(start_timestamp)
            CONF['device'][0]['searchRange']['end'] = TimestampToStr(end_timestamp)
            SaveConfiguration()
            time.sleep(5)

        SaveConfiguration()

    except:
        log.exception('unhandled exception.')

if __name__ == "__main__":
    main()

