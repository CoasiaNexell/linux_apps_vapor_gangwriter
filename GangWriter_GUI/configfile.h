#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class ConfigFile
{
public:
	ConfigFile(QString infile = nullptr);
	void SaveConfig(QString outfile = nullptr);
	//  General Configruation
	QString GetConfigFileName()                 {	return m_CfgFileName;		}
	void    SetConfigFileName(QString fileName) {	m_CfgFileName = fileName;	}
	QString GetModules()                        {	return m_Modules;			}
	void    SetModules(QString modules)         {	m_Modules = modules;		}
	QString GetImageFilename()                  {	return m_ImageFileName;		}
	void    SetImageFilename(QString fileName)  {	m_ImageFileName = fileName;	}
	QString GetOutDirectory()                   {	return m_OutDirectory;		}
	void    SetOutDirectory(QString dir)        {	m_OutDirectory = dir;		}
	QString GetBackupDirecotry()                {	return m_BackupDirectory;	}
	void    SetBackupDirecotry(QString dir)     {	m_BackupDirectory = dir;	}

	//  Network Configuration
	QString GetIPAddress()                      {	return m_IpAddr;			}
	void    SetIPAddress(QString ip)            {	m_IpAddr = ip;				}
	QString GetNetMask()                        {	return m_Netmask;			}
	void    SetNetMask(QString mask)            {	m_Netmask = mask;			}
	QString GetGateway()                        {	return m_Gateway;			}
	void    SetGateway(QString gateway)         {	m_Gateway = gateway;		}
	QString GetDNS()                            {	return m_DNS;				}
	void    SetDNS(QString dns)                 {	m_DNS = dns;				}

	//  Server Configuration
	QString GetServerIpAddr()                   {	return m_ServerIp;			}
	void    SetServerIpAddr(QString ip)         {	m_ServerIp = ip;			}
	QString GetTargetDirectory()                {	return m_TargetDirectory;	}
	void    SetTargetDirectory(QString dir)     {	m_TargetDirectory = dir;	}
	QString GetMountDirectory()                 {	return m_MountDirectory;	}
	void    SetMountDirectory(QString dir)      {	m_MountDirectory = dir;		}

private:
    bool Parsing(QString infile = nullptr);
    void PrintConfig();

private:
    QString m_Title;

    QString m_CfgFileName;
    QString m_Modules;
    QString m_ImageFileName;
    QString m_OutDirectory;
    QString m_BackupDirectory;

    QString m_IpAddr;
    QString m_Netmask;
    QString m_Gateway;
    QString m_DNS;

    QString m_ServerIp;
    QString m_TargetDirectory;
    QString m_MountDirectory;
};

#endif // CONFIGFILE_H
