#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configui.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

#include <QDateTime>

#define LOG_TAG "[GANGWRITER_GUI]"
#include "NxDbgMsg.h"
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_bConnected(false)
{
	ui->setupUi(this);

	m_CurrModules = 16;

	CreateModules();

	m_pNetClient = new CNetworkClient();

	m_bConnected = m_pNetClient->Connect();

	m_NotifyReceiver.StartNotifyReceiver( 5060, &NetworkCallbackStub, this );

	//	For LED Control Debugging
	ui->pushButton->setVisible(false);

	//	Connect Log Update
	connect( this, &MainWindow::LogMessage, this, &MainWindow::UpdateLogWindow );
}



void MainWindow::CreateModules()
{
	int32_t i;
	int32_t totalWidth = 1920;
	int32_t totalHeight = 1080;
	int32_t moudleWidth =totalWidth / 8;
	int32_t moduleHeight = totalHeight / 4;
	int32_t w = moudleWidth;

	int32_t heightOffset = totalHeight/2; // 540

	QString str;

	for( i=0 ; i < m_CurrModules ; i++ )
	{
		QRect rect;
		rect.setX(moudleWidth*(i%8) + 10);
		rect.setY(heightOffset + moduleHeight*(i/8));
		rect.setWidth(moudleWidth);
		rect.setHeight(moduleHeight);
		m_pVaporModule[i] = new CVaporModules( this, i, rect );
	}
}

void MainWindow::UpdateLogWindow( QString str )
{
	QString date = QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss.z");
	QString msg = "[" + date + "]" + str;

	ui->edtLogMessage->insertPlainText(msg);
	ui->edtLogMessage->ensureCursorVisible();
}


void MainWindow::DestroyModules()
{
	for( int32_t i=0 ; i < m_CurrModules ; i++ )
	{
		delete m_pVaporModule[i];
		m_pVaporModule[i] = nullptr;
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::NotifyCallback( void *buf, int32_t size )
{
	QJsonDocument doc = QJsonDocument::fromJson((char*)buf);
	QJsonObject obj = doc.object();
	QString msg = obj["message"].toString();
	QString ntype = obj["ntype"].toString();

	if( ntype == "status" )
	{
		QString port = obj["port"].toString();
		uint32_t portNo = port.toUInt();

		if(  msg == "STATE_READY" )				m_pVaporModule[portNo-1]->ModuleStatus( STATE_READY );
		else if(  msg == "STATE_DOWNLOADING" )	m_pVaporModule[portNo-1]->ModuleStatus( STATE_DOWNLOADING );
		else if( msg == "STATE_OK" )			m_pVaporModule[portNo-1]->ModuleStatus( STATE_OK );
		else if( msg == "STATE_ERROR" )			m_pVaporModule[portNo-1]->ModuleStatus( STATE_ERROR );

		msg = "[NotifyCallback] " + msg + "(port=" + port + ")\n";
		LogMessage(msg);

	}
	else if ( ntype == "message" )
	{
		//	skip alive message
		if( msg == "alive" )
			return;
		NXLOGV("Message : %s\n", msg.toStdString().data());
		LogMessage("[NotifyCallback] " + msg + "\n");
	}
}

//
//  LED State Test
//
void MainWindow::on_pushButton_released()
{
	static MODULE_STATE state = STATE_NONE;
	if     ( state == STATE_NONE )       state = STATE_READY;
	else if( state == STATE_READY )      state = STATE_DOWNLOADING;
	else if( state == STATE_DOWNLOADING )state = STATE_OK;
	else if( state == STATE_OK )         state = STATE_ERROR;
	else if( state == STATE_ERROR )      state = STATE_NONE;
	for( int32_t i=0 ; i<m_CurrModules ; i++ )
	{
		m_pVaporModule[i]->ModuleStatus( state );
	}
}

void MainWindow::on_btnConfiguration_released()
{
	ConfigUi *cfgUi = new ConfigUi(this);
	cfgUi->setWindowModality(Qt::WindowModality::WindowModal);
	cfgUi->show();
}


void MainWindow::on_btnScan_released()
{
	QJsonDocument doc;

	if( !m_bConnected )
	{
		m_bConnected = m_pNetClient->Connect();
		if( !m_bConnected )
		{
			QMessageBox msgBox;
			msgBox.setText("Error : Server connect failed!!!");
			msgBox.setStandardButtons(QMessageBox::Ok);
			return ;
		}
	}

	if( m_pNetClient->Scan(doc) )
	{
		QJsonObject obj = doc.object();
		QString modules = obj["modules"].toString();
		QJsonArray state = obj["result"].toArray();
		for( int i=0 ; i<state.size() ; i++ )
		{
			if( state[i].toString() == "ok" )
			{
				m_pVaporModule[i]->ModuleStatus( STATE_READY );
			}
			else
			{
				m_pVaporModule[i]->ModuleStatus( STATE_NONE );
			}
		}
		LogMessage("Scan Done!\n");
	}
}

void MainWindow::on_btnStart_released()
{
	QJsonDocument doc;

	if( !m_bConnected )
	{
		m_bConnected = m_pNetClient->Connect();
		if( !m_bConnected )
		{
			QMessageBox msgBox;
			msgBox.setText("Error : Server connect failed!!!");
			msgBox.setStandardButtons(QMessageBox::Ok);
			return;
		}
	}

	//	Scan
	if( m_pNetClient->Scan(doc) )
	{
		QJsonObject obj = doc.object();
		QString modules = obj["modules"].toString();
		QJsonArray state = obj["result"].toArray();
		for( int i=0 ; i<state.size() ; i++ )
		{
			if( state[i].toString() == "ok" )
			{
				m_pVaporModule[i]->ModuleStatus( STATE_READY );
			}
			else
			{
				m_pVaporModule[i]->ModuleStatus( STATE_NONE );
			}
		}
	}
	else
	{
		NXLOGE("Scan failed !!!\n");
		return;
	}

	if( m_pNetClient->Start(doc) )
	{
		int numStarted = 0;
		QJsonObject obj = doc.object();
		QString modules = obj["modules"].toString();
		QJsonArray state = obj["result"].toArray();
		NXLOGV("start result : %s\n", doc.toJson().toStdString().data());
		for( int i=0 ; i<state.size() ; i++  )
		{
			if( state[i].toString() == "ok" )
			{
				numStarted ++;
			}
		}
		QString msg = QString("Start!!! (%1)\n").arg(numStarted);
		LogMessage(msg);
	}
}

