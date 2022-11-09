#pragma once
#include <QCryptographicHash>
#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QtGlobal>
#include <QtDebug>
#include <QString>
#include <QVector>
#include <QFlags>

class Cipher {
public:
    enum CompressionMode {
        CompressionAuto,
        CompressionAlways,
        CompressionNever
    };
    enum IntegrityProtectionMode {
        ProtectionNone,
        ProtectionChecksum,
        ProtectionHash
    };
    enum Error {
        ErrorNoError,
        ErrorNoKeySet,
        ErrorUnknownVersion,
        ErrorIntegrityFailed,
    };
    Cipher();
    explicit Cipher(quint64 key);

    void setKey(quint64 key);
    bool hasKey() const {return !m_keyParts.isEmpty();}
    void setCompressionMode(CompressionMode mode) {m_compressionMode = mode;}

    CompressionMode compressionMode() const {return m_compressionMode;}

    void setIntegrityProtectionMode(IntegrityProtectionMode mode) {m_protectionMode = mode;}

    IntegrityProtectionMode integrityProtectionMode() const {return m_protectionMode;}

    Error lastError() const {return m_lastError;}

    QString encryptToString(const QString& plaintext);
    QString encryptToString(QByteArray plaintext);
    QByteArray encryptToByteArray(const QString& plaintext);
    QByteArray encryptToByteArray(QByteArray plaintext);

    QString decryptToString(const QString& cyphertext) ;
    QByteArray decryptToByteArray(const QString& cyphertext);
    QString decryptToString(QByteArray cypher);
    QByteArray decryptToByteArray(QByteArray cypher);

    enum CryptoFlag{
        CryptoFlagNone = 0,
        CryptoFlagCompression = 0x01,
        CryptoFlagChecksum = 0x02,
        CryptoFlagHash = 0x04
    };
    Q_DECLARE_FLAGS(CryptoFlags, CryptoFlag)
private:

    void splitKey();

    quint64 m_key;
    QVector<char> m_keyParts;
    CompressionMode m_compressionMode;
    IntegrityProtectionMode m_protectionMode;
    Error m_lastError;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Cipher::CryptoFlags)
