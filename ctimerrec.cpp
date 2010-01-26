/*********************** Information *************************\
| $HeadURL$
|
| Author: Joerg Neubert
|
| Begin: 24.01.2010 / 15:41:34
|
| Last edited by: $Author$
|
| $Id$
\*************************************************************/
#include "ctimerrec.h"
#include "ui_ctimerrec.h"

// logging stuff ...
extern CLogFile VlcLog;

/* -----------------------------------------------------------------\
|  Method: CTimerRec / constructor
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: create object, init values
|
|  Parameters: pointer to parent window
|
|  Returns: --
\----------------------------------------------------------------- */
CTimerRec::CTimerRec(QWidget *parent) : QDialog(parent), r_ui(new Ui::CTimerRec)
{
   r_ui->setupUi(this);
   iTimeShift = 0;
   uiActId    = 0;
   uiEdtId    = INVALID_ID;
   sListFile  = QString(INI_DIR).arg(getenv(APPDATA)) + QString("/reclist.xml");
   pTrigger   = NULL;
   pXmlParser = NULL;
   pSettings  = NULL;
   itActJob   = NULL;
   InitTab();
   connect (&recTimer, SIGNAL(timeout()), this, SLOT(slotRecTimer()));
}

/* -----------------------------------------------------------------\
|  Method: ~CTimerRec / destructor
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: clean on destruction
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
CTimerRec::~CTimerRec()
{
   SaveRecordList();
   delete r_ui;
}

/* -----------------------------------------------------------------\
|  Method: changeEvent
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: catch language change event
|
|  Parameters: event pointer
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::changeEvent(QEvent *e)
{
   QDialog::changeEvent(e);

   switch (e->type())
   {
   case QEvent::LanguageChange:
       r_ui->retranslateUi(this);
       break;
   default:
       break;
   }
}

/* -----------------------------------------------------------------\
|  Method: SetTimeShift
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: set timeshift
|
|  Parameters: new timeshift value
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::SetTimeShift(int iTs)
{
   iTimeShift = iTs;
}

/* -----------------------------------------------------------------\
|  Method: SetLogoPath
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: set logo path
|
|  Parameters: logo path
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::SetLogoPath(const QString &str)
{
   sLogoPath = str;
   ReadRecordList();
}

/* -----------------------------------------------------------------\
|  Method: StartTimer
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: start timer loop
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::StartTimer()
{
   if (!recTimer.isActive())
   {
      // check time every 3 second ...
      recTimer.start(3000);
   }
}

/* -----------------------------------------------------------------\
|  Method: SetXmlParser
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: set xml parser
|
|  Parameters: pointer to xml parser
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::SetXmlParser(CKartinaXMLParser *pParser)
{
   pXmlParser = pParser;
}

/* -----------------------------------------------------------------\
|  Method: SetKartinaTrigger
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: set wait trigger
|
|  Parameters: pointer to wait trigger
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::SetKartinaTrigger(CWaitTrigger *pTrig)
{
   pTrigger = pTrig;
}

/* -----------------------------------------------------------------\
|  Method: SetSettings
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: set settings
|
|  Parameters: pointer to settings
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::SetSettings(CSettingsDlg *pSet)
{
   pSettings = pSet;
}

/* -----------------------------------------------------------------\
|  Method: SetRecInfo
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: fill in record info into dialog
|
|  Parameters: start time, end time, channel id, program name
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::SetRecInfo (uint uiStart, uint uiEnd, int cid, const QString &name)
{
   // we don't update ...
   uiEdtId = INVALID_ID;

   // set timeshift stuff ...
   GmtToTimeShift(uiStart, iTimeShift);
   GmtToTimeShift(uiEnd, iTimeShift);

   QDateTime dtStart = QDateTime::fromTime_t(uiStart - TIMER_REC_OFFSET);
   QDateTime dtEnd   = QDateTime::fromTime_t(uiEnd + TIMER_REC_OFFSET);

   r_ui->dtEdtStart->setDateTime(dtStart);

   if (uiEnd > 1000000) // make sure there was a correct value set for end ...
   {
      r_ui->dtEdtEnd->setDateTime(dtEnd);
   }
   else
   {
      // if no end is given, set end time 1 hour later than start ...
      r_ui->dtEdtEnd->setDateTime(dtStart.addSecs(3600));
   }

   r_ui->cbxChannel->setCurrentIndex(r_ui->cbxChannel->findData(QVariant(cid)));
   r_ui->cbxTimeShift->setCurrentIndex(r_ui->cbxTimeShift->findText(QString::number(iTimeShift)));

   if (name != "")
   {
      r_ui->edtName->setText(QString("%1 (%2)").arg(name)
                             .arg(dtStart.toString("yyyy-MM-dd, hh:mm")));
   }
   else
   {
      // set name ...
      QMap<int, rec::SChanEntry>::const_iterator cit = ChanList.constFind(cid);

      if (cit != ChanList.constEnd())
      {
         r_ui->edtName->setText(QString("%1-%2").arg((*cit).Name)
                                .arg(dtStart.toString("yyyy-MM-dd, hh:mm")));
      }
      else
      {
         r_ui->edtName->setText(dtStart.toString("yyyy-MM-dd, hh:mm"));
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: SetChanList
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: set channel list
|
|  Parameters: ref. to channel list
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::SetChanList(const QVector<cparser::SChan> &chanList)
{
   rec::SChanEntry entry;

   ChanList.clear();
   r_ui->cbxChannel->clear();
   int iCount = 0;

   for (int i = 0; i < chanList.size(); i++)
   {
      if (chanList[i].iId > -1)
      {
         iCount ++;
         entry.cid  = chanList[i].iId;
         entry.Name = chanList[i].sName;

         r_ui->cbxChannel->insertItem(i,
                                    QIcon(QString("%1/%2.gif").arg(sLogoPath).arg(entry.cid)),
                                    QString("%1. %2").arg(iCount).arg(entry.Name),
                                    QVariant(entry.cid));

         ChanList[entry.cid] = entry;
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: SaveRecordList
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: save record list to xml file
|
|  Parameters: --
|
|  Returns: 0 ==> ok
|          -1 ==> any error
\----------------------------------------------------------------- */
int CTimerRec::SaveRecordList()
{
   int iRV = -1;
   QFile fListFile(sListFile);

   if (fListFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
   {
      iRV = 0;
      QTextStream str(&fListFile);
      str.setCodec ("UTF-8");

      str << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
      str << "<timerrec>" << endl;

      QMap<uint, rec::SRecEntry>::const_iterator cit;

      for (cit = JobList.constBegin(); cit != JobList.constEnd(); cit++)
      {
         str << "<entry cid=\"" << (*cit).cid << "\" timeshift=\"" << (*cit).iTimeShift
               << "\" start=\"" << (*cit).uiStart << "\" end=\"" << (*cit).uiEnd
               << "\" name=\"" << (*cit).sName << "\" />" << endl;
      }

      str << "</timerrec>" << endl;

      fListFile.close();
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: on_btnSet_clicked
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: insert entry into tab and joblist
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::on_btnSet_clicked()
{
   if (uiEdtId != INVALID_ID)
   {
      JobList.remove(uiEdtId);
      DelRow(uiEdtId);
      uiEdtId = INVALID_ID;
   }

   // sanity check ...
   if (!SanityCheck(r_ui->dtEdtStart->dateTime(), r_ui->dtEdtEnd->dateTime()))
   {
      if (r_ui->edtName->text() != "")
      {
         rec::SRecEntry entry;
         entry.cid        = r_ui->cbxChannel->itemData(r_ui->cbxChannel->currentIndex()).toInt();
         entry.uiStart    = r_ui->dtEdtStart->dateTime().toTime_t();
         entry.uiEnd      = r_ui->dtEdtEnd->dateTime().toTime_t();
         entry.sName      = r_ui->edtName->text();
         entry.iTimeShift = r_ui->cbxTimeShift->currentText().toInt();
         entry.eState     = rec::REC_READY;

         // we need times in GMT ...
         TimeShiftToGmt(entry.uiStart, entry.iTimeShift);
         TimeShiftToGmt(entry.uiEnd, entry.iTimeShift);

         // AddJob also adds the table row ...
         AddJob (entry);
      }
      else
      {
         QMessageBox::warning(this, tr("Please Correct!"),
                              tr("Please insert a name!"));
      }
   }
   else
   {
      QMessageBox::warning(this, tr("Please Correct!"),
                           tr("The entry you want to add overlapps with an existing entry!\r\n"
                              "Or start time is later than end time!"));
   }
}

/* -----------------------------------------------------------------\
|  Method: ReadRecordList
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: read old joblist from xml file
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
int CTimerRec::ReadRecordList()
{
   QXmlStreamReader     xml;
   QXmlStreamAttributes attrs;
   rec::SRecEntry       entry;
   int                  iRV = -1;
   JobList.clear();

   QFile fListFile(sListFile);

   if (fListFile.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      QTextStream str(&fListFile);
      str.setCodec("UTF-8");
      xml.addData(str.readAll());

      while(!xml.atEnd() && !xml.hasError())
      {
         switch (xml.readNext())
         {
         // we aren't interested in ...
         case QXmlStreamReader::StartDocument:
         case QXmlStreamReader::EndElement:
         case QXmlStreamReader::EndDocument:
            break;

         case QXmlStreamReader::StartElement:
            if (xml.name() == "entry")
            {
               attrs            = xml.attributes();
               entry.cid        = attrs.value(QString("cid").toUtf8()).toString().toInt();
               entry.iTimeShift = attrs.value(QString("timeshift").toUtf8()).toString().toInt();
               entry.uiStart    = attrs.value(QString("start").toUtf8()).toString().toUInt();
               entry.uiEnd      = attrs.value(QString("end").toUtf8()).toString().toUInt();
               entry.sName      = attrs.value(QString("name").toUtf8()).toString();
               entry.eState     = rec::REC_READY;

               // AddJob also adds the table row ...
               AddJob (entry);

               iRV = 0;
            }
            break;

         default:
            break;
         }
      }

      xml.clear();

      /* Error handling. */
      if(xml.hasError())
      {
         QMessageBox::critical(NULL, tr("Error in %1").arg(__FUNCTION__),
                               tr("XML Error String: %1").arg(xml.errorString()));
      }

      fListFile.close();
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: AddRow
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: add one row to job tab
|
|  Parameters: ref. to job entry
|
|  Returns: act table row
\----------------------------------------------------------------- */
int CTimerRec::AddRow(const rec::SRecEntry &entry)
{
   QTableWidgetItem *pItem;
   int               iRows;
   QDateTime         when;
   Qt::ItemFlags     flags;
   uint              uiStart, uiEnd;

   // convert times from GMT ...
   uiStart = entry.uiStart;
   uiEnd   = entry.uiEnd;

   GmtToTimeShift(uiStart, entry.iTimeShift);
   GmtToTimeShift(uiEnd, entry.iTimeShift);

   // prepare cell flags ...
   flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;

   iRows = r_ui->tableRecordEntries->rowCount();

   // one row more ...
   r_ui->tableRecordEntries->setRowCount(iRows + 1);

   // col 1: date ...
   when = QDateTime::fromTime_t(uiStart);
   pItem = new QTableWidgetItem (when.toString("dd.MM.yyyy"));
   pItem->setFlags(flags);
   pItem->setData(Qt::UserRole, QVariant(entry.id));
   r_ui->tableRecordEntries->setItem(iRows, 0, pItem);

   // col 2: start time ...
   pItem = new QTableWidgetItem (when.toString("hh:mm"));
   pItem->setFlags(flags);
   pItem->setData(Qt::UserRole, QVariant(entry.id));
   r_ui->tableRecordEntries->setItem(iRows, 1, pItem);

   // col 3: end time ...
   when = QDateTime::fromTime_t(uiEnd);
   pItem = new QTableWidgetItem (when.toString("hh:mm"));
   pItem->setFlags(flags);
   pItem->setData(Qt::UserRole, QVariant(entry.id));
   r_ui->tableRecordEntries->setItem(iRows, 2, pItem);

   // col 4: channel ...
   pItem = new QTableWidgetItem (QIcon(QString("%1/%2.gif").arg(sLogoPath).arg(entry.cid)), "");
   pItem->setFlags(flags);
   pItem->setData(Qt::UserRole, QVariant(entry.id));
   r_ui->tableRecordEntries->setItem(iRows, 3, pItem);

   // col. 5: timeshift ...
   pItem = new QTableWidgetItem (QString::number(entry.iTimeShift));
   pItem->setFlags(flags);
   pItem->setData(Qt::UserRole, QVariant(entry.id));
   r_ui->tableRecordEntries->setItem(iRows, 4, pItem);

   // col 6: name ...
   pItem = new QTableWidgetItem (entry.sName);
   pItem->setFlags(flags);
   pItem->setData(Qt::UserRole, QVariant(entry.id));
   r_ui->tableRecordEntries->setItem(iRows, 5, pItem);

   r_ui->tableRecordEntries->setRowHeight(iRows, 26);

   return iRows + 1;
}

/* -----------------------------------------------------------------\
|  Method: InitTab
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: init table view
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::InitTab()
{
   r_ui->tableRecordEntries->clearContents();
   r_ui->tableRecordEntries->setIconSize(QSize(24, 24));
   r_ui->tableRecordEntries->setColumnWidth(0, 80);
   r_ui->tableRecordEntries->setColumnWidth(1, 60);
   r_ui->tableRecordEntries->setColumnWidth(2, 60);
   r_ui->tableRecordEntries->setColumnWidth(3, 40);
   r_ui->tableRecordEntries->setColumnWidth(4, 25);
}

/* -----------------------------------------------------------------\
|  Method: AddJob
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: add job to joblist
|
|  Parameters: ref. to job entry
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::AddJob(rec::SRecEntry &entry)
{
   VlcLog.LogInfo(QString("%1 - %2():%3 Add Job #%4")
                  .arg(__FILE__).arg(__FUNCTION__)
                  .arg(__LINE__).arg(uiActId));

   entry.id = uiActId;

   JobList[entry.id] = entry;

   AddRow(entry);

   uiActId ++;
}

/* -----------------------------------------------------------------\
|  Method: SanityCheck
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: check if we can add this job (check times)
|
|  Parameters: start time, end time
|
|  Returns:  0 ==> check ok
|           -1 ==> check not passed
\----------------------------------------------------------------- */
int CTimerRec::SanityCheck(const QDateTime &start, const QDateTime &end)
{
   int iRV = 0;

   if (start < end)
   {
      uint uiStart, uiEnd;

      for (int i = 0; i < JobList.size(); i++)
      {
         uiStart = JobList[i].uiStart;
         uiEnd   = JobList[i].uiEnd;

         GmtToTimeShift(uiStart, JobList[i].iTimeShift);
         GmtToTimeShift(uiEnd, JobList[i].iTimeShift);

            /* start this between start/end other ...                         */
         if (((start.toTime_t() >= uiStart) && (start.toTime_t() <= uiEnd))
            /* end this between start/end other ...                           */
            || ((end.toTime_t() >= uiStart) && (end.toTime_t() <= uiEnd))
            /* start this before start other and end this after end other ... */
            || ((start.toTime_t() < uiStart) && (end.toTime_t() > uiEnd)))
         {
            iRV = -1;
            break;
         }
      }
   }
   else
   {
      // start later or equal end ...
      iRV = -1;
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: on_tableRecordEntries_cellDoubleClicked
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: insert marked entry into edit fields
|
|  Parameters: row, col
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::on_tableRecordEntries_cellDoubleClicked(int row, int column)
{
   QTableWidgetItem *pItem = r_ui->tableRecordEntries->item(row, column);

   if (pItem)
   {
      uint id = pItem->data(Qt::UserRole).toUInt();
      uiEdtId = id;

      QMap<uint, rec::SRecEntry>::const_iterator cit;

      cit = JobList.constFind(id);

      if (cit != JobList.constEnd())
      {
         uint when;

         // start ...
         when = (*cit).uiStart;
         GmtToTimeShift(when, (*cit).iTimeShift);
         r_ui->dtEdtStart->setDateTime(QDateTime::fromTime_t(when));

         // end ...
         when = (*cit).uiEnd;
         GmtToTimeShift(when, (*cit).iTimeShift);
         r_ui->dtEdtEnd->setDateTime(QDateTime::fromTime_t(when));

         // channel ...
         r_ui->cbxChannel->setCurrentIndex(r_ui->cbxChannel->findData(QVariant((*cit).cid)));

         // timshift ...
         r_ui->cbxTimeShift->setCurrentIndex(r_ui->cbxTimeShift->findText(QString::number((*cit).iTimeShift)));

         // name ...
         r_ui->edtName->setText((*cit).sName);
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: GmtToTimeShift
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: convert gmt into timeshift time
|
|  Parameters: ref. to timestamp, timeshift
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::GmtToTimeShift (uint &when, int iEntryTimeShift)
{
   when = when + iEntryTimeShift * 3600;
}

/* -----------------------------------------------------------------\
|  Method: TimeShiftToGmt
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: convert timeshift time into gmt
|
|  Parameters: ref. to timestamp, timeshift
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::TimeShiftToGmt(uint &when, int iEntryTimeShift)
{
   when = when - iEntryTimeShift * 3600;
}

/* -----------------------------------------------------------------\
|  Method: DelRow
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: del row from job tab
|
|  Parameters: entry id
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::DelRow(uint uiId)
{
   QTableWidgetItem *pItem;

   for (int i = 0; i < r_ui->tableRecordEntries->rowCount(); i++)
   {
      pItem = r_ui->tableRecordEntries->item(i, 0);

      if (pItem)
      {
         if (pItem->data(Qt::UserRole).toUInt() == uiId)
         {
            r_ui->tableRecordEntries->removeRow(i);
            break;
         }
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: on_btnDel_clicked
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: del current job tab entry and matching joblist entry
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::on_btnDel_clicked()
{
   QTableWidgetItem *pItem = r_ui->tableRecordEntries->currentItem();

   if (pItem)
   {
      uint id = pItem->data(Qt::UserRole).toUInt();

      JobList.remove(id);

      r_ui->tableRecordEntries->removeRow(r_ui->tableRecordEntries->currentRow());
   }
}

/* -----------------------------------------------------------------\
|  Method: slotRecTimer
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: timer signal, check joblist if we should start
|               a record job
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::slotRecTimer()
{
   if (pTrigger)
   {
      if (JobList.isEmpty())
      {
         emit sigSendStatusMsg(tr("Timer Ready"), QString("white"));
      }
      else
      {
         uint now = QDateTime::currentDateTime().toTime_t();
         uint start, end;

         QMap<uint, rec::SRecEntry>::iterator it = JobList.begin();

         while (it != JobList.end())
         {
            start = (*it).uiStart;
            end   = (*it).uiEnd;

            GmtToTimeShift(start, (*it).iTimeShift);
            GmtToTimeShift(end, (*it).iTimeShift);

            if (now >= end)
            {
               // old entry ...

               // was record active ... ?
               if ((*it).eState != rec::REC_RUNNING)
               {
                  // old record ... delete without sending signals ...
                  DelRow((*it).id);
                  it = JobList.erase(it);
               }
               else
               {
                  // record was active ... is now done ...
                  DelRow((*it).id);
                  it = JobList.erase(it);

                  emit sigRecDone();
                  emit sigSendStatusMsg(tr("Timer Ready"), QString("white"));
               }
            }
            else
            {
               // should we set timer to stby ... ?
               if (((start - TIMER_STBY_TIME) <= now) && ((*it).eState == rec::REC_READY))
               {
                  // set timer to stby ...
                  emit sigRecActive();
                  emit sigSendStatusMsg(tr("Timer StBY"), QString("#fc0"));
                  (*it).eState = rec::REC_STBY;

                  // set timeshift ...
                  pTrigger->TriggerRequest(Kartina::REQ_TIMESHIFT, (*it).iTimeShift);
               }
               else if ((start <= now) && ((*it).eState == rec::REC_STBY))
               {
                  // start record ...
                  emit sigSendStatusMsg(tr("Record Active"), QString("red"));
                  (*it).eState = rec::REC_RUNNING;
                  itActJob     = it;
                  pTrigger->TriggerRequest(Kartina::REQ_TIMERREC, (*it).cid);
               }

               it++;
            }
         }
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: slotTimerStreamUrl
|  Begin: 26.01.2010 / 16:05:00
|  Author: Joerg Neubert
|  Description: got stream url, start VLC
|
|  Parameters: stream url
|
|  Returns: --
\----------------------------------------------------------------- */
void CTimerRec::slotTimerStreamUrl(QString str)
{
   pXmlParser->SetByteArray(str.toUtf8());

   QString sUrl   = pXmlParser->ParseURL();

   uint uiRunTime = (*itActJob).uiEnd;
   GmtToTimeShift(uiRunTime, (*itActJob).iTimeShift);
   uiRunTime      = uiRunTime - QDateTime::currentDateTime().toTime_t();

   QString sCmdLine = VLC_REC_TEMPL;

   sCmdLine.replace(TMPL_VLC, pSettings->GetVLCPath());
   sCmdLine.replace(TMPL_URL, sUrl);
   sCmdLine.replace(TMPL_MUX, QString("ts"));
   sCmdLine.replace(TMPL_DST, QString("%1/%2.ts").arg(pSettings->GetTargetDir())
                                                 .arg((*itActJob).sName));

   sCmdLine += QString(" --http-caching=%1 --no-http-reconnect --run-time=%2")
               .arg(pSettings->GetBufferTime())
               .arg(uiRunTime);

   VlcLog.LogInfo(tr("Start VLC using following command line:\n") + sCmdLine);

   QProcess::startDetached(sCmdLine);

   itActJob = NULL;
}

/************************* History ***************************\
| $Log$
\*************************************************************/
