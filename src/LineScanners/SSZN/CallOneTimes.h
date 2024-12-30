#ifndef CALLONETIMES_H
#define CALLONETIMES_H

#include <SR7Link.h>

#include <string>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#define CAMNUM 2
#define MAXHEIGHT 15000

#pragma pack(4)
typedef struct {
    unsigned int version;
    int width;
    int height;
    double xInterval;
    double yInterval;
    char info[32];
    int reserve2[2545];
} BATCH_INFO;
#pragma pack()

using namespace std;

enum {
    BATCH_SAVE_FILE_VERSION = 0x00000002,
};

class CallOneTimes {
public:
    CallOneTimes();
    ~CallOneTimes();

    void VariableInit();
    void DataMemoryInit(int mProW);
    int* getBatchData(int _camId);
    unsigned char* getIntensityData(int _camId);
    unsigned int* getEncoderData(int _camId);

    int saveHeightData(const std::string& _path, int _camId);
    int saveIntensityData(const std::string& _path, int _camId);
    int saveEncoderData(const std::string& _path, int _camId);

    void BatchOneTimeCallBack(const void *info, const SR7IF_Data *data);

private:
    void deleteDataMemory();

    int mDeviceId;
    int m_BatchWait;
    int mBatchTimes;
    int mCurBatchPt;
    int mBatchPoint;
    int mBatchWidth;
    double mXinterVal;
    double mYinterVal;

    int* mProfileData[CAMNUM];
    unsigned char* mIntensityData[CAMNUM];
    unsigned int* mEncoderData[CAMNUM];
};

#endif // CALLONETIMES_H
