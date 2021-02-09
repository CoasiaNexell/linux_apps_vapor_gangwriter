#include "configfile.h"

#define DEF_CFG_FILE     "/home/root/gangwriter_config.json"


/*
   Configuration Database Shape
   - title
   - Configurations
	   - ConfigFile
	   - Modules
	   - OutDirectory
	   - BackupDirectory
   - ConfigNetwork
	   - IPAddress
	   - Networkmask
	   - Gatway
	   -DNS
   - ConfigServer
	   - IPAddress
	   - TargetDirectory
	   - LocalMountPosition

 */

#define LOG_TAG "[GANGWRITER_CONFIG]"
#include "NxDbgMsg.h"


ConfigFile::ConfigFile( QString infile )
{
	Parsing( infile );
}

void ConfigFile::PrintConfig()
{
	NXLOGI("-----------------------------------------------\n");
	NXLOGI("    \"%s\"\n", m_Title.toStdString().data());
	NXLOGI("-----------------------------------------------\n");

	NXLOGI("Configurations : \n");
	NXLOGI("    ConfigFile = %s\n", m_CfgFileName.toStdString().data());
	NXLOGI("    Modules = %s\n", m_Modules.toStdString().data());
	NXLOGI("    ImageFile = %s\n", m_ImageFileName.toStdString().data());
	NXLOGI("    OutDirecotry = %s\n", m_OutDirectory.toStdString().data());
	NXLOGI("    BackupDirectory = %s\n\n", m_BackupDirectory.toStdString().data());

	NXLOGI("Network Setting : \n");
	NXLOGI("    ip address = %s\n", m_IpAddr.toStdString().data());
	NXLOGI("    network mask = %s\n", m_Netmask.toStdString().data());
	NXLOGI("    gateway = %s\n", m_Gateway.toStdString().data());
	NXLOGI("    dns = %s\n\n", m_DNS.toStdString().data());

	NXLOGI("Server Setting : \n");
	NXLOGI("    server Ip : %s\n", m_ServerIp.toStdString().data());
	NXLOGI("    Target Direcotry = %s\n", m_TargetDirectory.toStdString().data());
	NXLOGI("    Mount Position = %s\n", m_MountDirectory.toStdString().data());
	NXLOGI("-----------------------------------------------\n");
}


bool ConfigFile::Parsing( QString infile )
{
	QFile file;
	//  JSon Database
	QJsonDocument doc;
	QJsonObject allObj;
	QJsonObject config;
	QJsonObject cfgNetwork;
	QJsonObject cfgServer;

	if( infile.isNull() )
		file.setFileName( DEF_CFG_FILE );
	else
		file.setFileName( infile );

	if( file.open( QIODevice::ReadOnly ) )
	{
		QByteArray data = file.readAll();
		doc = QJsonDocument::fromJson(data);
		file.close();
	}

	allObj = doc.object();

	m_Title = allObj["title"].toString();

	//  Parsing Configuration
	config = allObj["Configurations"].toObject();
	m_CfgFileName = DEF_CFG_FILE;
	m_Modules = config["Modules"].toString();
	m_ImageFileName= config["InputImage"].toString();
	m_OutDirectory = config["OutDirectory"].toString();
	m_BackupDirectory = config["BackupDirectory"].toString();

	//  Parsing Network Configuration
	cfgNetwork = allObj["ConfigNetwork"].toObject();
	m_IpAddr = cfgNetwork["IPAddress"].toString();
	m_Netmask = cfgNetwork["NetworkMask"].toString();
	m_Gateway = cfgNetwork["Gateway"].toString();
	m_DNS = cfgNetwork["DNS"].toString();

	//  Parsing Server Configruation
	cfgServer = allObj["ConfigServer"].toObject();
	m_ServerIp = cfgServer["IPAddress"].toString();
	m_TargetDirectory = cfgServer["TargetDirectory"].toString();
	m_MountDirectory = cfgServer["LocalMountPosition"].toString();
	return true;
}


void ConfigFile::SaveConfig(QString outfile)
{
	QFile file;
	if( outfile.isNull() )
		file.setFileName(DEF_CFG_FILE);
	else
		file.setFileName(outfile);

	if( file.open( QIODevice::WriteOnly ) )
	{
		QJsonObject allObj;
		QJsonObject config;
		QJsonObject cfgNetwork;
		QJsonObject cfgServer;

		allObj["title"] = "Nexell Gang Writer Configuration File";

		//  Parsing Network Configuration
		config["ConfigFile"]      = m_CfgFileName    ;
		config["Modules"]         = m_Modules        ;
		config["InputImage"]      = m_ImageFileName  ;
		config["OutDirectory"]    = m_OutDirectory   ;
		config["BackupDirectory"] = m_BackupDirectory;
		allObj["Configurations"] = config;

		//  Parsing Network Configuration
		cfgNetwork["IPAddress"]   = m_IpAddr ;
		cfgNetwork["NetworkMask"] = m_Netmask;
		cfgNetwork["Gateway"]     = m_Gateway;
		cfgNetwork["DNS"]         = m_DNS    ;
		allObj["ConfigNetwork"] = cfgNetwork;

		//  Parsing Server Configruation
		cfgServer["IPAddress"]          = m_ServerIp       ;
		cfgServer["TargetDirectory"]    = m_TargetDirectory;
		cfgServer["LocalMountPosition"] = m_MountDirectory ;
		allObj["ConfigServer"]          = cfgServer        ;

		QJsonDocument doc(allObj);
		QByteArray data = doc.toJson();
		file.write(data);
		file.close();
	}
}
