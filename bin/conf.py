# -*- coding: utf-8 -*-

import os

USER_DATA_DIR = ''
DOWNLOADS_DIR = ''
CONFIG_FILE_NAME = ''
LOG_FILE_NAME = ''
LOCAL_RECORD_DOWNLOADER_FILE_NAME = ''

def GetUserDataDir():
    global USER_DATA_DIR
    if USER_DATA_DIR == '':
        USER_DATA_DIR = os.path.join(os.path.dirname(__file__), os.path.pardir, 'user-data')
        if not os.path.exists(USER_DATA_DIR):
            os.makedirs(USER_DATA_DIR)
    return USER_DATA_DIR

def GetLocalRecordDownloaderPath():
    global LOCAL_RECORD_DOWNLOADER_FILE_NAME
    if LOCAL_RECORD_DOWNLOADER_FILE_NAME == '':
        LOCAL_RECORD_DOWNLOADER_FILE_NAME = os.path.join(os.path.dirname(__file__), 'LocalRecordDownloader.exe')
    return LOCAL_RECORD_DOWNLOADER_FILE_NAME

def GetConfigFilePath():
    global CONFIG_FILE_NAME
    if CONFIG_FILE_NAME == '':
        CONFIG_FILE_NAME = os.path.join(GetUserDataDir(), 'config.json')
    return CONFIG_FILE_NAME

def GetDownloadsDir():
    global DOWNLOADS_DIR
    if DOWNLOADS_DIR == '':
        DOWNLOADS_DIR = os.path.join(GetUserDataDir(), 'downloads')
        if not os.path.exists(DOWNLOADS_DIR):
            os.makedirs(DOWNLOADS_DIR)
    return DOWNLOADS_DIR

def GetLogFilePath():
    global LOG_FILE_NAME
    if LOG_FILE_NAME == '':
        LOG_FILE_NAME = os.path.join(GetUserDataDir(), 'ezviz.log')
    return LOG_FILE_NAME

