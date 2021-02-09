#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QWidgetList>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>

#include "vapormodule.h"
#include "NetworkClient.h"
#include "CBaseClass.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define MAX_MODULES         (32)



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void CreateModules();
	void DestroyModules();

	void SetModuleStatus( int32_t index, MODULE_STATE newState );

signals:
	void LogMessage(QString str);

private slots:
	void on_pushButton_released();
	void on_btnConfiguration_released();
	void on_btnScan_released();
	void on_btnStart_released();
	void UpdateLogWindow(QString str);

private:
	Ui::MainWindow *ui;
	CVaporModules   *m_pVaporModule[MAX_MODULES];
	int32_t         m_CurrModules;
	CNetworkClient	*m_pNetClient;
	bool			m_bConnected;
	QString			m_LogText;

	static void NetworkCallbackStub( void* pPrivate, void *buf, int32_t size )
	{
		((MainWindow *)pPrivate)->NotifyCallback( buf, size );
	}
	void NotifyCallback( void *buf, int32_t size );

	CNotifyReceiver	m_NotifyReceiver;
};
#endif // MAINWINDOW_H
