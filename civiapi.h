/*------------------------------ Information ---------------------------*//**
 *
 *  $HeadURL$
 *
 *  @file     civiapi.h
 *
 *  @author   Jo2003
 *
 *  @date     18.11.2016
 *
 *  $Id$
 *
 *///------------------------- (c) 2016 by Jo2003  --------------------------
#ifndef __20161118_CIVIAPI_H
    #define __20161118_CIVIAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QMap>
#include <QTimer>
#include "ccmac_bf.h"
#include "cparser.h"

#define IVI_REQ_ID            "reqid"
#define IVI_GETCONTENT_TMPL   "{\"params\":[\"%1\", {\"session\": \"%2\", \"app_version\": %3}], \"method\":\"da.content.get\"}"
#define IVI_GETTIMESTAMP_TMPL "{\"method\": \"da.timestamp.get\", \"params\": []}"

namespace ivi {

    // looking forward ...
    struct SGenre;
    struct SCat;
    struct SCountry;

    typedef QMap<int, SCat>     CategoryMap;
    typedef QMap<int, SGenre>   GenreMap;
    typedef QMap<int, SCountry> CountryMap;

    enum eIviReq {
        IVI_SESSION,
        IVI_GENRES,
        IVI_VIDEOS,
        IVI_VIDEOINFO,
        IVI_FILES,
        IVI_COUNTRIES,
        IVI_TIMESTAMP,
        IVI_ADD_FAV,
        IVI_DEL_FAV,
        IVI_UNKNOWN=256
    };

    struct SCat {
        QString  mTitle;
        int      mId;
        GenreMap mGenres;
    };

    struct SGenre {
        QString mTitle;
        int     mId;
        int     mCatId;
        int     mCount;
        int     mNoCompilations;
        int     mNoContent;
    };

    struct SVideoFilter {
        int     mGenId;
        int     mFrom;
        int     mTo;
        QString mSort;
    };

    struct SCountry {
        int     mId;
        QString mName;
        QString mShort;
    };
}

///
/// \brief The CIviApi class
///
class CIviApi : public QNetworkAccessManager
{
    Q_OBJECT

public:
    CIviApi(QObject *parent = 0);
    virtual ~CIviApi();

    void setVerimatrixKey(const QString& key);

    // get / set
    int getGenres();
    int getCountries();
    int getVideos(const ivi::SVideoFilter& filter);
    int getVideoInfo(int id);
    int getFiles(int id);

    // parse
    int parseSession(const QString& resp);
    int parseGenres(const QString& resp);
    int parseCountries(const QString& resp);
    int parseVideos(const QString& resp);
    int parseVideoInfo(const QString& resp);
    int parseFiles(const QString& resp);
    int parseTimeStamp(const QString& resp);

signals:
    void sigCategories(ivi::CategoryMap cats);
    void sigCountries(ivi::CountryMap countr);
    void sigVideoList(cparser::VideoList vidoes);
    void sigVideoInfo(cparser::SVodVideo video);
    void sigError(int iType, const QString& cap, const QString& descr);

public slots:
    int getTimeStamp();

private slots:
    void getReply(QNetworkReply* reply);

public slots:

protected:
    ivi::CountryMap  mCountries;
    ivi::GenreMap    mGenres;

private:
    QString            mProtocol;
    QString            mHost;
    QString            mQueryPrefix;
    QString            mVerimatrix;
    QString            mSessionKey;
    QString            mTs;
    CCMAC_Bf          *pHash;
    cparser::SVodVideo mCurrentVideo;
};

#endif // __20161118_CIVIAPI_H