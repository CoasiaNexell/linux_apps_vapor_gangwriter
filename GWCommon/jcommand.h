#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>


class CJCommand {
public:
	CJCommand(){};
	QJsonDocument Connect()
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "command";
		m_Obj["command"] = "connect";
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Disconnect()
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "command";
		m_Obj["command"] = "disconnect";
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Alive()
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "command";
		m_Obj["command"] = "alive";
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Scan()
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "command";
		m_Obj["command"] = "scan";
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Start( QString &port )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "command";
		m_Obj["command"] = "start";
		m_Obj["port"] = port;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Reset( QString &port )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "command";
		m_Obj["command"] = "reset";
		m_Obj["port"] = port;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	//
	//	Get Satus Information All Ports
	//
	QJsonDocument Status()
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "command";
		m_Obj["command"] = "status";
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}

private:
	QJsonDocument   m_Doc;
	QJsonObject     m_Obj;
};


class CJCmdResponse{
public:
	CJCmdResponse(){};
	QJsonDocument Connect( QString &status )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "response";
		m_Obj["command"] = "connect";
		m_Obj["status"] = status;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Disconnect( QString &status )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "response";
		m_Obj["command"] = "disconnect";
		m_Obj["status"] = status;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Scan( int32_t modules, QString status[] )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "response";
		m_Obj["command"] = "scan";
		m_Obj["modules"] = QString::number(modules);
		for( int32_t i=0 ; i<modules ; i++ )
		{
			m_Array.append(status[i]);
		}
		m_Obj["result"] = m_Array;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Start( QString &port, int32_t modules, QString status[] )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "response";
		m_Obj["command"] = "start";

		if( modules == 1 )
			m_Obj["port"] = port;

		m_Obj["modules"] = QString::number(modules);

		for( int32_t i=0 ; i<modules ; i++ )
		{
			m_Array.append(status[i]);
		}
		m_Obj["result"] = m_Array;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}

	QJsonDocument Reset( QString &port, const char *status )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "response";
		m_Obj["command"] = "reset";
		m_Obj["port"] = port;
		m_Obj["result"] = status;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Status( int32_t modules, QString status[] )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "response";
		m_Obj["command"] = "start";
		m_Obj["modules"] = QString::number(modules);
		for( int32_t i=0 ; i<modules ; i++ )
		{
			m_Array.append(status[i]);
		}
		m_Obj["result"] = m_Array;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}

private:
	QJsonDocument   m_Doc;
	QJsonObject     m_Obj;
	QJsonArray      m_Array;
};



class CJNotify{
public:
	CJNotify(){};
	~CJNotify(){};

	QJsonDocument Pogress( QString &step, int32_t modules, QString status[] )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "notify";
		m_Obj["ntype"] = "porgress";
		m_Obj["step"] = step;
		m_Obj["modules"] = QString::number(modules);
		for( int32_t i=0 ; i<modules ; i++ )
		{
			m_Array.append(status[i]);
		}
		m_Obj["data"] = m_Array;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Error( QString &port, QString &message )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "notify";
		m_Obj["ntype"] = "error";
		m_Obj["port"] = port;
		m_Obj["message"] = message;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	QJsonDocument Message( QString &level, QString &message )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "notify";
		m_Obj["ntype"] = "message";
		m_Obj["level"] = level;
		m_Obj["message"] = message;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}
	//
	//	Status Change Event
	//
	QJsonDocument StatusChange( QString  &port, QString &message )
	{
		m_Obj["version"] = "1.0.0";
		m_Obj["type"] = "notify";
		m_Obj["ntype"] = "status";
		m_Obj["port"] = port;
		m_Obj["message"] = message;
		m_Doc.setObject(m_Obj);
		return m_Doc;
	}

private:
	QJsonDocument   m_Doc;
	QJsonObject     m_Obj;
	QJsonArray      m_Array;
};
