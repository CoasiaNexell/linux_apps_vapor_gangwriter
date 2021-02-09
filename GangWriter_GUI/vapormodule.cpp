#include "vapormodule.h"

#define NX_TAG "[CVaporModule]"
#include "NxDbgMsg.h"

///////////////////////////////////////////////////////////////////////////////////////////
/// \brief CVaporModules::CVaporModules
/// \param parent
/// \param index
/// \param rect
//////////////////////////////////////////////////////////////////////////////////////////
Q_DECLARE_METATYPE(MODULE_STATE)

#define	BLINK_INTERVAL		(500)

CVaporModules::CVaporModules( QWidget *parent, int32_t index, QRect rect )
    : m_Parent( parent )
{
	int32_t x = rect.x();
	int32_t y = rect.y();
	int32_t w = rect.width();
	int32_t h = rect.height();
	int32_t leftMargin = 40;
	int32_t compGap = 40;
	int32_t textWidth = 60;
	int32_t ledWidth = 20;
	int32_t ledHeight = 14;
	int32_t txtHeight = 14;
	int32_t stateWidth = 120;

	QString str;

	//  Create components;
	m_pBox    = new QGroupBox(m_Parent);
	m_pGreen  = new QWidget(m_Parent);
	m_pOrange = new QWidget(m_Parent);
	m_pRed    = new QWidget(m_Parent);

	m_pLabelOk   = new QLabel(m_Parent);
	m_pLabelData = new QLabel(m_Parent);
	m_pLabelErr  = new QLabel(m_Parent);
	m_pLabelState= new QLabel(m_Parent);

	m_pLabelOk   ->setText("Link");
	m_pLabelData ->setText("Data");
	m_pLabelErr  ->setText("Error");
	m_pLabelState->setText("");

	//  Set Information for each control
	m_pBox   ->setTitle(str.asprintf("Module %d", index+1));
	m_pBox   ->setAlignment(Qt::AlignCenter);
	m_pGreen ->setStyleSheet(LED_OFF);
	m_pOrange->setStyleSheet(LED_OFF);
	m_pRed   ->setStyleSheet(LED_OFF);

	//  Set Positions
	m_pBox       ->setGeometry( x + 10          , y + compGap * 0, w-30, h-20);

	//  OK LED & Label
	m_pGreen     ->setGeometry( x + leftMargin,      y + compGap * 1, ledWidth  , ledHeight);
	m_pLabelOk   ->setGeometry( x + leftMargin + 40, y + compGap * 1, textWidth , txtHeight);
	m_pOrange    ->setGeometry( x + leftMargin,      y + compGap * 2, ledWidth  , ledHeight);
	m_pLabelData ->setGeometry( x + leftMargin + 40, y + compGap * 2, textWidth , txtHeight);
	m_pRed       ->setGeometry( x + leftMargin,      y + compGap * 3, ledWidth  , ledHeight);
	m_pLabelErr  ->setGeometry( x + leftMargin + 40, y + compGap * 3, textWidth , txtHeight);

	m_pLabelState->setGeometry( x + leftMargin,      y + compGap * 4, stateWidth, txtHeight);

	//  Show All Components
	m_pGreen ->show();
	m_pOrange->show();
	m_pRed   ->show();

	m_state = STATE_UNKNOWN;

	connect( this, 	&CVaporModules::ModuleStatus, this, &CVaporModules::SetModuleStatus );

	qRegisterMetaType<MODULE_STATE>();

}

CVaporModules::~CVaporModules()
{
	delete m_pBox       ;
	delete m_pGreen     ;
	delete m_pOrange    ;
	delete m_pRed       ;
	delete m_pLabelOk   ;
	delete m_pLabelData ;
	delete m_pLabelErr  ;
}

void CVaporModules::SetModuleStatus( MODULE_STATE newState )
{
	if( m_state == newState )
		return;
	//  Turn Off Blink LED
	if( newState == STATE_NONE )
	{
		m_pBox   ->setEnabled(true);
		m_pGreen ->setStyleSheet(LED_OFF);
		m_pOrange->setStyleSheet(LED_OFF);
		m_pRed   ->setStyleSheet(LED_OFF);
		m_pLabelState->setText("STATE_NONE");
	}
	else if( newState == STATE_READY )
	{
		m_pBox   ->setEnabled(true);
		m_pGreen ->setStyleSheet(LED_GREEN);
		m_pOrange->setStyleSheet(LED_OFF);
		m_pRed   ->setStyleSheet(LED_OFF);
		m_pLabelState->setText("STATE_READY");
	}
	else if( newState == STATE_DOWNLOADING )
	{
		m_pBox   ->setEnabled(true);
		m_pGreen ->setStyleSheet(LED_GREEN);
		m_pOrange->setStyleSheet(LED_ORANGE);  m_IsOn = true;
		m_pRed   ->setStyleSheet(LED_OFF);
		m_pLabelState->setText("DOWNLOADING");
	}
	else if( newState == STATE_OK )
	{
		m_pBox   ->setEnabled(true);
		m_pGreen ->setStyleSheet(LED_GREEN);
		m_pOrange->setStyleSheet(LED_ORANGE);  m_IsOn = true;
		m_pRed   ->setStyleSheet(LED_OFF);
		m_pLabelState->setText("STATE_OK");
	}
	else if( newState == STATE_ERROR )
	{
		m_pBox   ->setEnabled(true);
		m_pGreen ->setStyleSheet(LED_GREEN);
		m_pOrange->setStyleSheet(LED_OFF);
		m_pRed   ->setStyleSheet(LED_RED);
		m_pLabelState->setText("STATE_ERROR");
	}

	m_state = newState;

	if( newState == STATE_DOWNLOADING )
	{
		QTimer::singleShot( BLINK_INTERVAL/2, this, SLOT(LedBlink()) );
	}
}

void CVaporModules::LedBlink()
{
	if( m_state == STATE_DOWNLOADING )
	{
		QTimer::singleShot( BLINK_INTERVAL/2, this, SLOT(LedBlink()) );
		if( m_IsOn )
		{
			m_pOrange->setStyleSheet(LED_OFF);
			m_IsOn = false;
		}
		else
		{
			m_pOrange->setStyleSheet(LED_ORANGE);
			m_IsOn = true;
		}
	}
}

//
//                            End Of Class CVaporModules
//
///////////////////////////////////////////////////////////////////////////////////////////

