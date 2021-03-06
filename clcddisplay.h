/*********************** Information *************************\
| $HeadURL$
|
| Author: Jo2003
|
| Begin: 01.04.2010 / 14:21:44
|
| Last edited by: $Author$
|
| $Id$
\*************************************************************/
#ifndef __040110__CLCDDISPLAY_H
   #define __040110__CLCDDISPLAY_H

#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include "playstates.h"

#define COLOR_TEXT          "#333"
#define COLOR_TEXT_SHADOW   "#084"

//===============================================================
// namespace for lcd
//===============================================================
namespace lcd
{
   enum eState
   {
      STATE_BUFFER     = IncPlay::PS_BUFFER,
      STATE_END        = IncPlay::PS_END,
      STATE_ERROR      = IncPlay::PS_ERROR,
      STATE_OPEN       = IncPlay::PS_OPEN,
      STATE_PAUSE      = IncPlay::PS_PAUSE,
      STATE_PLAY       = IncPlay::PS_PLAY,
      STATE_READY      = IncPlay::PS_READY,
      STATE_RECORD     = IncPlay::PS_RECORD,
      STATE_STOP       = IncPlay::PS_STOP,
      STATE_TIMER_REC  = IncPlay::PS_TIMER_RECORD,
      STATE_TIMER_STBY = IncPlay::PS_TIMER_STBY,
      STATE_SHOW_ADV   = IncPlay::PS_ADVERTISING,
      STATE_BLANK,
      STATE_WTF = 255
   };
}

/********************************************************************\
|  Class: CLCDDisplay
|  Date:  01.04.2010 / 14:22:18
|  Author: Jo2003
|  Description: advanced QLabel represents a LC display
|
\********************************************************************/
class CLCDDisplay : public QLabel
{
   Q_OBJECT

public:
   CLCDDisplay (QWidget * parent = 0, Qt::WindowFlags f = 0);
   CLCDDisplay (const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0);
   virtual ~CLCDDisplay ();

protected:
   void addHeader (const QString& str = QString());
   void addFooter (const QString& str = QString());
   void loadImage (lcd::eState state);
   void showLCD ();

private:
   lcd::eState lcdState;
   QString sHeader, sFooter;
   QImage  lcdImg;

public slots:
   void updateState (int iState);
   void resetState ();
   void clearState ();
   void setState (int iState, const QString& sHead = QString(), const QString& sFoot = QString());
   void setHeader (const QString& str);
   void setFooter (const QString& str);
   void bufferPercent (int percent);
};

#endif // __040110__CLCDDISPLAY_H
/************************* History ***************************\
| $Log$
\*************************************************************/

