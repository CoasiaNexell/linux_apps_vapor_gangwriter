#include "mainwindow.h"

#include <QApplication>
#include <QProcess>

#include "utils.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	if( AppIsRunning("GWUsbDaemon") == 0 )
		system( "/home/root/GWUsbDaemon &" );
	return a.exec();
}
