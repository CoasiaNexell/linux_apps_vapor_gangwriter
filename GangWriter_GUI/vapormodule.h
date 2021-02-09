#ifndef VAPORMODULE_H
#define VAPORMODULE_H
#include <QMainWindow>
#include <QWidget>
#include <QWidgetList>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>

#include "NodeState.h"

#define NUM_MODULE_LEDS     (3)

#define LED_OFF     "background-color:#808080"
#define LED_GREEN   "background-color:#22FF22"
#define LED_RED     "background-color:#FF2222"
#define LED_ORANGE  "background-color:#FF8000"

enum eLED_STATUS{
	OFF = 0,
	ON,
	BLINK,
};
typedef enum eLED_STATUS LED_STATUS;


struct tVaporMoudles{
	QGroupBox *pBox;
	QLabel  *pLabelOk;
	QWidget *pGreen;
	QLabel  *pLabelData;
	QWidget *pOrange;
	QLabel  *pLabelErr;
	QWidget *pRed;
	LED_STATUS ledState[NUM_MODULE_LEDS];
	MODULE_STATE state;
};

typedef struct tVaporMoudles VAPOR_MODULE;


class CVaporModules : public QObject{
	Q_OBJECT
public:
	CVaporModules( QWidget *parent, int32_t index, QRect rect );
	~CVaporModules();

signals:
	void ModuleStatus( MODULE_STATE newState );

private slots:
	void LedBlink();
	void SetModuleStatus( MODULE_STATE newState );

private:
	QGroupBox   *m_pBox;
	QLabel      *m_pLabelOk;
	QWidget     *m_pGreen;
	QLabel      *m_pLabelData;
	QWidget     *m_pOrange;
	QLabel      *m_pLabelErr;
	QWidget     *m_pRed;
	LED_STATUS  m_ledState[NUM_MODULE_LEDS];
	MODULE_STATE m_state;

	QLabel		*m_pLabelState;

	QWidget     *m_Parent;
	QTimer      *m_BlinkTimer;

	bool        m_IsOn;
};

#endif // VAPORMODULE_H
