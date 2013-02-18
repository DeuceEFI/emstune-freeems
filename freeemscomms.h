/***************************************************************************
*   Copyright (C) 2012  Michael Carpenter (malcom2073)                     *
*                                                                          *
*   This file is a part of EMStudio                                        *
*                                                                          *
*   EMStudio is free software: you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License version 2 as      *
*   published by the Free Software Foundation.                             *
*                                                                          *
*   EMStudio is distributed in the hope that it will be useful,            *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
*   GNU General Public License for more details.                           *
									   *
*   You should have received a copy of the GNU General Public License      *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
****************************************************************************/
#ifndef FREEEMSCOMMS_H
#define FREEEMSCOMMS_H

#include <QThread>
#include <QFile>
#include <QMutex>
#include <QVariant>
#include <QDateTime>
#include "serialport.h"
#include "serialrxthread.h"
#include "emscomms.h"
//#include "logloader.h"
#include "headers.h"
#include "fedatapacketdecoder.h"
class FreeEmsComms : public EmsComms
{
	Q_OBJECT
	Q_INTERFACES(EmsComms)
public:
	FreeEmsComms(QObject *parent = 0);
	~FreeEmsComms();
	DataPacketDecoder *getDecoder();
	Table3DData *getNew3DTableData();
	Table2DData *getNew2DTableData();
	void stop() { m_terminateLoop = true; }
	void setLogsEnabled(bool enabled);
	void setLogDirectory(QString dir);
	void setPort(QString portname);
	void setBaud(int baudrate);
	void setLogFileName(QString filename);
	bool sendPacket(unsigned short payloadid,QList<QVariant> arglist=QList<QVariant>(),QList<int> argsizelist=QList<int>(),bool haslength=false);
	int getLocationIdInfo(unsigned short locationid);
	int getInterfaceVersion();
	int getFirmwareVersion();
	int getMaxPacketSize();
	int getDecoderName();
	int getFirmwareBuildDate();
	int getCompilerVersion();
	int getOperatingSystem();
	int echoPacket(QByteArray packet);
	int getLocationIdList(unsigned char listtype, unsigned short listmask);
	int softReset();
	int hardReset();
	bool sendSimplePacket(unsigned short payload);
	void connectSerial(QString port,int baud);
	void disconnectSerial();
	void loadLog(QString filename);
	void playLog();
	//void populateDataFields();
	QByteArray generatePacket(QByteArray header,QByteArray payload);
	int updateBlockInRam(unsigned short location,unsigned short offset, unsigned short size,QByteArray data);
	int updateBlockInFlash(unsigned short location,unsigned short offset, unsigned short size,QByteArray data);
	int retrieveBlockFromRam(unsigned short location, unsigned short offset, unsigned short size);
	int retrieveBlockFromFlash(unsigned short location, unsigned short offset, unsigned short size);
	int burnBlockFromRamToFlash(unsigned short location,unsigned short offset, unsigned short size);
	void setInterByteSendDelay(int milliseconds);
	void setlogsDebugEnabled(bool enabled);
protected:
	void run();
private:
	enum LocationIdFlags
	{
		BLOCK_HAS_PARENT=0x0001,
		BLOCK_IS_RAM=0x0002,
		BLOCK_IS_FLASH=0x0004,
		BLOCK_IS_INDEXABLE=0x0008,
		BLOCK_IS_READ_ONLY=0x0010,
		BLOCK_GETS_VERIFIED=0x0020,
		BLOCK_FOR_BACKUP_RESTORE=0x0040,
		BLOCK_SPARE_7=0x0080,
		BLOCK_SPARE_8=0x0100,
		BLOCK_SPARE_9=0x0200,
		BLOCK_SPARE_10=0x0400,
		BLOCK_SPARE_11=0x0800,
		BLOCK_IS_2D_TABLE=0x1000,
		BLOCK_IS_MAIN_TABLE=0x2000,
		BLOCK_IS_LOOKUP_DATA=0x4000,
		BLOCK_IS_CONFIGURATION=0x8000
	};
	enum RequestType
	{
		GET_INTERFACE_VERSION=0x0000,
		GET_FIRMWARE_VERSION=0x0002,
		GET_MAX_PACKET_SIZE=0x0004,
		ECHO_PACKET=0x0006,
		SOFT_RESET=0x0008,
		HARD_RESET=0x000A,
		UPDATE_BLOCK_IN_RAM=0x0100,
		UPDATE_BLOCK_IN_FLASH=0x0102,
		RETRIEVE_BLOCK_IN_RAM=0x0104,
		RETRIEVE_BLOCK_IN_FLASH=0x0106,
		BURN_BLOCK_FROM_RAM_TO_FLASH=0x0108,
		GET_LOCATION_ID_LIST=0xDA5E,
		GET_DECODER_NAME=0xEEEE,
		GET_FIRMWARE_BUILD_DATE=0xEEF0,
		GET_COMPILER_VERSION=0xEEF2,
		GET_OPERATING_SYSTEM=0xEEF4,
		GET_LOCATION_ID_INFO=0xF8E0,
		SERIAL_CONNECT=0xFFFF01,
		SERIAL_DISCONNECT=0xFFFF02
	};

	class Packet
	{
	public:
		Packet(bool valid = true) { isValid = valid; }
		bool isNAK;
		bool isValid;
		QByteArray header;
		QByteArray payload;
		unsigned short payloadid;
		unsigned short length;
		bool haslength;
		bool hasseq;
		unsigned short sequencenum;
	};

	class RequestClass
	{
	public:
		RequestClass()
		{
			retryCount=0;
		}

		unsigned char retryCount;
		RequestType type;
		QList<QVariant> args;
		QList<int> argsize;
		int sequencenumber;
		void addArg(QVariant arg,int size=0) { args.append(arg); argsize.append(size);}
	};
	FEDataPacketDecoder *dataPacketDecoder;
	bool m_debugLogsEnabled;
	QMap<FreeEmsComms::LocationIdFlags,QString> m_blockFlagToNameMap;
	bool m_terminateLoop;
	QMutex m_waitingInfoMutex;
	SerialRXThread *rxThread;
	bool sendPacket(RequestClass request,bool haslength);
	qint64 m_timeoutMsecs;
	QList<LocationIdFlags> m_blockFlagList;
	int m_sequenceNumber;
	QMutex m_reqListMutex;
	QList<RequestClass> m_reqList;
	QList<RequestClass> m_threadReqList;
	SerialPort *serialPort;
	//LogLoader *logLoader;
	bool m_waitingForResponse;
	bool m_logsEnabled;
	QString m_logsDirectory;
	QString m_logsFilename;
	unsigned int m_payloadWaitingForResponse;
	RequestClass m_currentWaitingRequest;
	QFile *m_logInFile;
	QFile *m_logOutFile;
	QFile *m_logInOutFile;
	void openLogs();
	//void parseBuffer(QByteArray buffer);
signals:
	void packetSent(unsigned short locationid,QByteArray header,QByteArray payload);
	void packetAcked(unsigned short locationid,QByteArray header,QByteArray payload);
	void packetNaked(unsigned short locationid,QByteArray header,QByteArray payload,unsigned short errornum);
	void decoderFailure(QByteArray packet);
	void decoderName(QString name);
	void firmwareBuild(QString date);
	void compilerVersion(QString version);
	void operatingSystem(QString os);
	void connected();
	void disconnected();
	void locationIdList(QList<unsigned short> idlist);
	void locationIdInfo(unsigned short locationid,MemoryLocationInfo info);
	void ramBlockRetrieved(unsigned short locationid, QByteArray header,QByteArray payload);
	void flashBlockRetrieved(unsigned short locationid,QByteArray header,QByteArray payload);
	void dataLogPayloadReceived(QByteArray header,QByteArray payload);
	void interfaceVersion(QString version);
	void firmwareVersion(QString version);
	void debugVerbose(QString msg);
	void unknownPacket(QByteArray header,QByteArray payload);
	void debug(QString msg);
	//void error(QString msg);
	//void error(QString msg);
	void error(SerialPortStatus error,QString msg = QString());
	void commandSuccessful(int sequencenumber);
	void commandFailed(int sequencenumber,unsigned short errornum);
	void commandTimedOut(int sequencenumber);
	//void updateBlockInRamFailed(int location,int offset,int size,QByteArray data);
	//void updateBlockInRamSucceeded();
public slots:
private slots:
	void dataLogWrite(QByteArray buffer);
	void dataLogRead(QByteArray buffer);
	void parseEverything(QByteArray buffer);
	Packet parseBuffer(QByteArray buffer);
	void parsePacket(Packet parsedPacket);

};

#endif // FREEEMSCOMMS_H
