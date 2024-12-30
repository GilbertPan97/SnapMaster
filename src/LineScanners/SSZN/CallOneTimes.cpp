#include "CallOneTimes.h"

#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

CallOneTimes::CallOneTimes() {
    VariableInit();
    mProfileData[0] = nullptr;
    mProfileData[1] = nullptr;
    mIntensityData[0] = nullptr;
    mIntensityData[1] = nullptr;
    mEncoderData[0] = nullptr;
    mEncoderData[1] = nullptr;
}

CallOneTimes::~CallOneTimes() {
    deleteDataMemory();
}

void CallOneTimes::VariableInit() {
    mDeviceId = 0;
    m_BatchWait = 0;
    mBatchTimes = 0;
    mCurBatchPt = 0;
    mBatchPoint = 0;
    mBatchWidth = 0;
    mXinterVal = 0;
    mYinterVal = 0.1;
}

void CallOneTimes::DataMemoryInit(int mProW) {
    int mDataC = MAXHEIGHT * mProW;
    try {
        mProfileData[0] = new int[mDataC];
        mIntensityData[0] = new unsigned char[mDataC];
        mEncoderData[0] = new unsigned int[MAXHEIGHT];
    } catch (std::exception& e) {
        std::cerr << "Data memory allocation failed: " << e.what() << std::endl;
        exit(1);
    }

    try {
        mProfileData[1] = new int[mDataC];
        mIntensityData[1] = new unsigned char[mDataC];
        mEncoderData[1] = new unsigned int[MAXHEIGHT];
    } catch (std::exception& e) {
        std::cerr << "Data memory allocation failed: " << e.what() << std::endl;
        exit(1);
    }
}

void CallOneTimes::deleteDataMemory() {
    if (mProfileData[0]) {
        delete[] mProfileData[0];
        mProfileData[0] = nullptr;
    }
    if (mIntensityData[0]) {
        delete[] mIntensityData[0];
        mIntensityData[0] = nullptr;
    }
    if (mEncoderData[0]) {
        delete[] mEncoderData[0];
        mEncoderData[0] = nullptr;
    }

    if (mProfileData[1]) {
        delete[] mProfileData[1];
        mProfileData[1] = nullptr;
    }
    if (mIntensityData[1]) {
        delete[] mIntensityData[1];
        mIntensityData[1] = nullptr;
    }
    if (mEncoderData[1]) {
        delete[] mEncoderData[1];
        mEncoderData[1] = nullptr;
    }
}

int* CallOneTimes::getBatchData(int _camId) {
    if (_camId < 0 || _camId > CAMNUM - 1) return nullptr;
    return mProfileData[_camId];
}

unsigned char* CallOneTimes::getIntensityData(int _camId) {
    if (_camId < 0 || _camId > CAMNUM - 1) return nullptr;
    return mIntensityData[_camId];
}

unsigned int* CallOneTimes::getEncoderData(int _camId) {
    if (_camId < 0 || _camId > CAMNUM - 1) return nullptr;
    return mEncoderData[_camId];
}

int CallOneTimes::saveHeightData(const std::string& _path, int _camId) {
    if (mProfileData[_camId] == nullptr || mBatchPoint == 0 || mBatchWidth == 0 
        || _camId < 0 || _camId > CAMNUM - 1) {
        return -1;
    }

    std::string fileExtension = _path.substr(_path.length() - 4);
    if (fileExtension == ".bin") {
        std::ofstream file(_path, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<char*>(mProfileData[_camId]), sizeof(int) * mBatchPoint * mBatchWidth);
            file.close();
        } else {
            return -1;
        }
    } else if (fileExtension == ".ecd") {
        BATCH_INFO mBatchInfo;
        mBatchInfo.version = BATCH_SAVE_FILE_VERSION;
        mBatchInfo.width = mBatchWidth;
        mBatchInfo.height = mBatchPoint;
        mBatchInfo.xInterval = mXinterVal;
        mBatchInfo.yInterval = mYinterVal;
        std::strcpy(mBatchInfo.info, "SSZN2021 V00000002");

        std::ofstream file(_path, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<char*>(&mBatchInfo), sizeof(BATCH_INFO));
            file.write(reinterpret_cast<char*>(mProfileData[_camId]), mBatchWidth * mBatchPoint * sizeof(int));
            if (mEncoderData[_camId]) {
                file.write(reinterpret_cast<char*>(mEncoderData[_camId]), mBatchPoint * sizeof(int));
            }
            file.close();
        } else {
            return -1;
        }
    } else {
        return -1;
    }

    return 0;
}

int CallOneTimes::saveIntensityData(const std::string& _path, int _camId) {
    if (mIntensityData[_camId] == nullptr || mBatchPoint == 0 || mBatchWidth == 0 || _camId < 0 || _camId > 1) {
        return -1;
    }

    std::string fileExtension = _path.substr(_path.length() - 4);
    if (fileExtension == ".bin") {
        std::ofstream file(_path, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<char*>(mIntensityData[_camId]), sizeof(unsigned char) * mBatchPoint * mBatchWidth);
            file.close();
        } else {
            return -1;
        }
    }
    return 0;
}

int CallOneTimes::saveEncoderData(const std::string& _path, int _camId) {
    if (mEncoderData[_camId] == nullptr || mBatchPoint == 0 || mBatchWidth == 0 || _camId < 0 || _camId > 1) {
        return -1;
    }

    std::string fileExtension = _path.substr(_path.length() - 4);
    if (fileExtension == ".bin") {
        std::ofstream file(_path, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<char*>(mEncoderData[_camId]), sizeof(unsigned int) * mBatchPoint);
            file.close();
        } else {
            return -1;
        }
    }
    return 0;
}

void CallOneTimes::BatchOneTimeCallBack(const void *info, const SR7IF_Data *data) {
    // Handle the data callback
}
