#include <QCoreApplication>
#include <unistd.h>

#include "cnetworkmanager.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	CNetworkManager *pMgr = new CNetworkManager();
	pMgr->Start(5050);
	return a.exec();
}
